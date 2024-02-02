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
#include <meta/tasks/job.hpp>
#include <meta/tasks/thread_pool.hpp>

#include <string>

namespace
{

using SecureInt = meta::Atomic<std::size_t>;

class Output
{
    std::vector<std::string> buffer;
public:
    void write(std::string_view text)
    {
        buffer.emplace_back(std::string(text));
    }

    const std::vector<std::string>& getBuffer() const
    {
        return buffer;
    }
};
using OutputPtr = std::shared_ptr<Output>;


class Job : public meta::Job
{
public:
    explicit Job(OutputPtr, SecureInt& jobCount) :
        m_jobCount(jobCount)
    {
    }

    void setStatus(Status status)
    {
        meta::Job::setStatus(status);
    }

protected:
    void runOverride() override
    {
        ++m_jobCount;
    }

    SecureInt& m_jobCount;
};

class RescheduledJob : public meta::Job
{
    meta::ThreadPool* m_scheduler = nullptr;
    OutputPtr m_out;
public:

    meta::TaskCompletionWatchObject m_jobWatch;

    explicit RescheduledJob(meta::ThreadPool* scheduler, OutputPtr out) :
        m_scheduler(scheduler),
        m_out(out)
    {
    }

    void push(std::string_view text)
    {
        if (isStopped())
        {
            return;
        }

        {
            meta::GuardLock lock(m_lock);
            m_queue.push(std::string(text));
        }
        m_signal.notify_one();
        const auto status = getStatus();
        if (status == Status::Deferred || status == Status::Stopped)
        {
            m_jobWatch = m_scheduler->pushJob(shared_from_this());
        }
    }

protected:
    void runOverride() override
    {
        meta::UniqueLock lock(m_lock);
        auto condition = [this]()
        {
            return !this->m_queue.empty() || isStopped();
        };
        m_signal.wait(lock, condition);
        while (!m_queue.empty())
        {
            auto text = m_queue.front();
            m_queue.pop();
            m_out->write(text);
        }
    }

    void stopOverride() override
    {
        m_signal.notify_all();
    }

    meta::Mutex m_lock;
    meta::ConditionVariable m_signal;
    std::queue<std::string> m_queue;
};

class QueuedJob : public Job
{
    OutputPtr m_out;
public:
    explicit QueuedJob(OutputPtr out, SecureInt& jobCount) :
        Job(out, jobCount),
        m_out(out)
    {
    }

    void push(std::string string)
    {
        if (isStopped())
        {
            return;
        }
        {
            meta::GuardLock lock(m_lock);
            m_queue.push(string);
        }
        m_signal.notify_all();
    }

protected:
    void runOverride() override
    {
        ++m_jobCount;
        while (!isStopped())
        {
            meta::UniqueLock lock(m_lock);
            auto condition = [this]()
            {
                return !this->m_queue.empty() || isStopped() || m_threadPool->isStopSignalled();
            };
            m_signal.wait(lock, condition);
            if (m_queue.empty())
            {
                if (m_threadPool->isStopSignalled())
                {
                    break;
                }
                continue;
            }
            while (!m_queue.empty())
            {
                auto text = m_queue.front();
                m_queue.pop();
                m_out->write(text);
            }
        }
    }

    void stopOverride() override
    {
        m_signal.notify_all();
    }

    meta::Mutex m_lock;
    meta::ConditionVariable m_signal;
    std::queue<std::string> m_queue;
};

class TaskSchedulerTest : public ::testing::Test
{
protected:
    std::unique_ptr<meta::ThreadPool> taskScheduler;
    OutputPtr m_output;

    void SetUp() override
    {
        taskScheduler = std::make_unique<meta::ThreadPool>(meta::Thread::hardware_concurrency());
        if (!taskScheduler->isRunning())
        {
            taskScheduler->start();
        }

        m_output = std::make_shared<Output>();
    }

    void TearDown() override
    {
        if (taskScheduler)
        {
            taskScheduler->stop();
        }
        taskScheduler.reset();
        m_output.reset();
    }

    template <class JobType>
    struct ScenarioBase
    {
        std::vector<meta::TaskPtr> jobs;
        std::vector<meta::TaskCompletionWatchObject> futures;

        std::shared_ptr<JobType> operator[](int index)
        {
            return std::static_pointer_cast<JobType>(jobs[index]);
        }
    };

    template <typename JobType>
    struct QueuedTaskScenario : ScenarioBase<JobType>
    {
        SecureInt jobCount = 0u;
        TaskSchedulerTest& test;

        explicit QueuedTaskScenario(TaskSchedulerTest& test, std::size_t tasks) :
            test(test)
        {
            this->jobs.reserve(tasks);
            while (tasks-- != 0u)
            {
                this->jobs.push_back(std::make_shared<JobType>(test.m_output, jobCount));
            }
            this->futures = test.taskScheduler->pushMultipleJobs(this->jobs);
            test.taskScheduler->schedule(std::chrono::milliseconds(1));
        }

    };

    template <class JobType>
    struct ReschedulingTaskSchenario : ScenarioBase<JobType>
    {
        TaskSchedulerTest& test;

        explicit ReschedulingTaskSchenario(TaskSchedulerTest& test, std::size_t taskCount) :
            test(test)
        {
            this->jobs.reserve(taskCount);
            while (taskCount-- != 0u)
            {
                this->jobs.push_back(std::make_shared<JobType>(test.taskScheduler.get(), test.m_output));
            }
        }
    };
};

}

TEST(Tasks, testJob)
{
    SecureInt jobCount = 0u;
    OutputPtr out = std::make_shared<Output>();
    Job job(out, jobCount);
    job.setStatus(Job::Status::Scheduled);
    job.run();
    EXPECT_EQ(Job::Status::Stopped, job.getStatus());
    EXPECT_EQ(1u, jobCount);
}

TEST_F(TaskSchedulerTest, testAddJobs)
{
    constexpr auto maxJobs = 50u;
    QueuedTaskScenario<Job> scenario(*this, maxJobs);
    ASSERT_EQ(scenario.jobCount, maxJobs);

    std::size_t jobCount = 0u;
    for (auto& future : scenario.futures)
    {
        ++jobCount;
        future.wait();
    }    
    EXPECT_EQ(jobCount, maxJobs);
    EXPECT_FALSE(taskScheduler->isBusy());
}

TEST_F(TaskSchedulerTest, testAddQueuedJobs)
{
    SKIP_IF_NOT_MULTI_THREADED;
    QueuedTaskScenario<QueuedJob> scenario(*this, 1u);
    EXPECT_EQ(scenario.jobCount, 1u);

    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Second test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Third test string");
    taskScheduler->schedule(std::chrono::milliseconds(10));
    EXPECT_EQ(taskScheduler->getThreadCount() - 1u, taskScheduler->getIdleCount());
    EXPECT_TRUE(taskScheduler->isBusy());
    EXPECT_EQ(m_output->getBuffer().size(), 3u);
}

TEST_F(TaskSchedulerTest, stressTestExclusiveJobs)
{
    SKIP_IF_NOT_MULTI_THREADED;
    QueuedTaskScenario<QueuedJob> scenario(*this, taskScheduler->getThreadCount());
    EXPECT_EQ(scenario.jobCount, taskScheduler->getThreadCount());

    // taskScheduler->schedule(std::chrono::milliseconds(100));
    EXPECT_EQ(0u, taskScheduler->getIdleCount());
    EXPECT_TRUE(taskScheduler->isBusy());

}

TEST_F(TaskSchedulerTest, reschedulingTask)
{
    ReschedulingTaskSchenario<RescheduledJob> scenario(*this, 1u);
    scenario[0]->push("1st string");
    scenario[0]->push("2nd string");
    scenario[0]->push("3rd string");
    scenario[0]->push("4th string");
    taskScheduler->schedule();
    scenario[0]->m_jobWatch.wait();

    EXPECT_EQ(4u, m_output->getBuffer().size());
}
