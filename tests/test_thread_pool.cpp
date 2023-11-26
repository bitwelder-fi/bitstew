/*
 * Copyright (C) 2023 bitWelder
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see
 * <http://www.gnu.org/licenses/>
 */

#include "test_utils.hpp"

#include <gtest/gtest.h>
#include <meta/meta.hpp>
#include <meta/library_config.hpp>
#include <meta/tasks/task.hpp>
#include <meta/tasks/task_scheduler.hpp>

#include <queue>
#include <string>

using namespace meta;

namespace
{

using SecureInt = Atomic<size_t>;

class Job : public Task
{
public:
    explicit Job(SecureInt& jobCount) :
        m_jobCount(jobCount)
    {
    }

    void setStatus(Status status)
    {
        Task::setStatus(status);
    }

protected:
    void runOverride() override
    {
        ++m_jobCount;
    }

    SecureInt& m_jobCount;
};

class RescheduledJob : public Task
{
    TaskScheduler* m_scheduler = nullptr;
public:

    explicit RescheduledJob(TaskScheduler* scheduler) :
        m_scheduler(scheduler)
    {
    }

    void push(std::string_view text)
    {
        if (isStopped())
        {
            std::cout << "Task stopped\n";
            return;
        }

        {
            GuardLock lock(m_lock);
            m_queue.push(std::string(text));
        }
        m_signal.notify_one();
        const auto status = getStatus();
        if (status == Status::Deferred || status == Status::Stopped)
        {
            m_scheduler->tryQueueTask(shared_from_this());
        }
    }

protected:
    void runOverride() override
    {
        UniqueLock lock(m_lock);
        auto condition = [this]()
        {
            return !this->m_queue.empty() || isStopped();
        };
        m_signal.wait(lock, condition);
        while (!m_queue.empty())
        {
            auto text = m_queue.front();
            m_queue.pop();
            std::cout << "Queued log: " << text << std::endl;
        }
    }

    void stopOverride() override
    {
        m_signal.notify_all();
    }

    Mutex m_lock;
    ConditionVariable m_signal;
    std::queue<std::string> m_queue;
};

class QueuedJob : public Job
{
public:
    explicit QueuedJob(SecureInt& jobCount) :
        Job(jobCount)
    {
    }

    void push(std::string string)
    {
        if (isStopped())
        {
            return;
        }
        {
            GuardLock lock(m_lock);
            m_queue.push(string);
        }
        m_signal.notify_one();
    }

protected:
    void runOverride() override
    {
        ++m_jobCount;
        while (!isStopped())
        {
            UniqueLock lock(m_lock);
            auto condition = [this]()
            {
                return !this->m_queue.empty() || isStopped() || m_taskScheduler->isStopSignalled();
            };
            m_signal.wait(lock, condition);
            if (m_queue.empty())
            {
                if (m_taskScheduler->isStopSignalled())
                {
                    break;
                }
                continue;
            }
            auto text = m_queue.front();
            m_queue.pop();
            std::cout << "Queued log: " << text << std::endl;
        }
    }

    void stopOverride() override
    {
        m_signal.notify_all();
    }

    Mutex m_lock;
    ConditionVariable m_signal;
    std::queue<std::string> m_queue;
};

class ThreadPoolTest : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        auto arguments = meta::LibraryArguments();
        arguments.taskScheduler.createThreadPool = false;
        meta::Domain::instance().initialize(arguments);
    }
    static void TearDownTestSuite()
    {
        meta::Domain::instance().uninitialize();
    }

protected:
    std::unique_ptr<TaskScheduler> taskScheduler;

    void SetUp() override
    {
        taskScheduler = std::make_unique<meta::TaskScheduler>(Thread::hardware_concurrency());
        if (!taskScheduler->isRunning())
        {
            taskScheduler->start();
        }
    }

    void TearDown() override
    {
        if (taskScheduler)
        {
            taskScheduler->stop();
        }
        taskScheduler.reset();
    }

    template <class JobType>
    struct ScenarioBase
    {
        std::vector<TaskPtr> jobs;
        std::vector<TaskFuture> futures;
        std::shared_ptr<JobType> operator[](int index)
        {
            return std::static_pointer_cast<JobType>(jobs[index]);
        }
    };

    template <typename JobType>
    struct QueuedTaskScenario : ScenarioBase<JobType>
    {
        SecureInt jobCount = 0u;
        ThreadPoolTest& test;

        explicit QueuedTaskScenario(ThreadPoolTest& test, size_t tasks) :
            test(test)
        {
            this->jobs.reserve(tasks);
            while (tasks-- != 0u)
            {
                this->jobs.push_back(std::make_shared<JobType>(jobCount));
            }
            this->futures = test.taskScheduler->tryQueueTasks(this->jobs);
            ThisThread::sleep_for(std::chrono::milliseconds(1));
        }

    };

    template <class JobType>
    struct ReschedulingTaskSchenario : ScenarioBase<JobType>
    {
        ThreadPoolTest& test;

        explicit ReschedulingTaskSchenario(ThreadPoolTest& test, size_t taskCount) :
            test(test)
        {
            this->jobs.reserve(taskCount);
            while (taskCount-- != 0u)
            {
                this->jobs.push_back(std::make_shared<JobType>(test.taskScheduler.get()));
            }
        }
    };
};

}

TEST(Tasks, testJob)
{
    SecureInt jobCount = 0u;
    Job job(jobCount);
    job.setStatus(Job::Status::Scheduled);
    job.run();
    EXPECT_EQ(Job::Status::Stopped, job.getStatus());
    EXPECT_EQ(1u, jobCount);
}

TEST_F(ThreadPoolTest, testAddJobs)
{
    constexpr auto maxJobs = 50u;
    QueuedTaskScenario<Job> scenario(*this, maxJobs);
    ASSERT_EQ(scenario.jobCount, maxJobs);

    size_t jobCount = 0u;
    for (auto& future : scenario.futures)
    {
        ++jobCount;
        future.wait();
    }    
    EXPECT_EQ(jobCount, maxJobs);
    EXPECT_FALSE(taskScheduler->isBusy());
}

TEST_F(ThreadPoolTest, testAddQueuedJobs)
{
    SKIP_IF_NOT_MULTI_THREADED;
    QueuedTaskScenario<QueuedJob> scenario(*this, 1u);
    EXPECT_EQ(scenario.jobCount, 1u);

    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Second test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Third test string");
    taskScheduler->schedule();
    EXPECT_EQ(taskScheduler->getThreadCount() - 1u, taskScheduler->getIdleCount());
    EXPECT_TRUE(taskScheduler->isBusy());
}

TEST_F(ThreadPoolTest, stressTestExclusiveJobs)
{
    SKIP_IF_NOT_MULTI_THREADED;
    QueuedTaskScenario<QueuedJob> scenario(*this, taskScheduler->getThreadCount());
    EXPECT_EQ(scenario.jobCount, taskScheduler->getThreadCount());

    // taskScheduler->schedule(std::chrono::milliseconds(100));
    EXPECT_EQ(0u, taskScheduler->getIdleCount());
    EXPECT_TRUE(taskScheduler->isBusy());

}

TEST_F(ThreadPoolTest, reschedulingTask)
{
    ReschedulingTaskSchenario<RescheduledJob> scenario(*this, 1u);
    scenario[0]->push("1st string");
    scenario[0]->push("2nd string");
    scenario[0]->push("3rd string");
    scenario[0]->push("4th string");
    // std::cout << taskScheduler->getTaskCount() << std::endl;
    taskScheduler->schedule();
}
