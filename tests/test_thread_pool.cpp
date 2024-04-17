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
#include <stew/stew.hpp>
#include <stew/library_config.hpp>
#include <stew/tasks/job.hpp>
#include <stew/tasks/thread_pool.hpp>
#include <stew/safe_queue.hpp>

#include <atomic>
#include <string>

namespace
{

using SecureInt = std::atomic_size_t;

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


class TestJob : public stew::Job
{
public:
    explicit TestJob(OutputPtr, SecureInt& jobCount) :
        m_jobCount(jobCount)
    {
    }

    void setStatus(Status status)
    {
        stew::Job::setStatus(status);
    }

    void run() override
    {
        ++m_jobCount;
    }

protected:
    SecureInt& m_jobCount;
};

class ReusableJob : public stew::Job
{
    stew::ThreadPool* m_scheduler = nullptr;
    OutputPtr m_out;

public:
    std::atomic_size_t rescheduleCount = 0u;
    explicit ReusableJob(stew::ThreadPool* scheduler, OutputPtr out) :
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

        auto successfulSchedule = false;
        auto self = shared_from_this();
        while (!m_queue.tryPush(std::string(text)))
        {
            successfulSchedule |= m_scheduler->tryScheduleJob(self);
        }
        if (!successfulSchedule)
        {
            m_scheduler->tryScheduleJob(self);
        }
    }

protected:
    void run() override
    {
        ++rescheduleCount;
        for (auto text = m_queue.tryPop(); !text.empty(); text = m_queue.tryPop())
        {
            m_out->write(text);
        }
    }

    void onCompleted() override
    {
        if (m_queue.wasEmpty())
        {
            return;
        }
        m_scheduler->tryScheduleJob(shared_from_this());
    }

    stew::CircularBuffer<std::string> m_queue;
};


class QueuedJob : public TestJob, public stew::SharedQueue<std::string, QueuedJob>
{
    OutputPtr m_out;
    std::condition_variable m_signal;

public:
    explicit QueuedJob(OutputPtr out, SecureInt& jobCount) :
        TestJob(out, jobCount),
        stew::SharedQueue<std::string, QueuedJob>(*this),
        m_out(out)
    {
    }

    // notifier functions
    void notifyOne()
    {
        m_signal.notify_one();
    }

    void wait(std::unique_lock<std::mutex>& lock)
    {
        auto condition = [this]()
        {
            return isStopped() || !nolock_isEmpty();
        };
        m_signal.wait(lock, condition);
    }

protected:
    void run() override
    {
        ++m_jobCount;
        while (!isStopped())
        {
            auto processor = [this](std::string text)
            {
                if (text.empty())
                {
                    return false;
                }
                m_out->write(text);
                return true;
            };
            forEach(processor);
        }
    }

    void stopOverride() override
    {
        m_signal.notify_all();
    }
};

class TaskSchedulerTest : public ::testing::Test
{
protected:
    std::unique_ptr<stew::ThreadPool> threadPool;
    OutputPtr m_output;

    void SetUp() override
    {
        threadPool = std::make_unique<stew::ThreadPool>(std::thread::hardware_concurrency());
        if (!threadPool->isRunning())
        {
            threadPool->start();
        }

        m_output = std::make_shared<Output>();
    }

    void TearDown() override
    {
        if (threadPool)
        {
            threadPool->stop();
        }
        threadPool.reset();
        m_output.reset();
    }

    template <class JobType>
    struct ScenarioBase
    {
        std::vector<stew::JobPtr> jobs;

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
            test.threadPool->tryScheduleJobs(this->jobs);
            test.threadPool->schedule(std::chrono::milliseconds(1));
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
                this->jobs.push_back(std::make_shared<JobType>(test.threadPool.get(), test.m_output));
            }
        }
    };
};

}

TEST_F(TaskSchedulerTest, testAddJobs)
{
    constexpr auto maxJobs = 50u;
    QueuedTaskScenario<TestJob> scenario(*this, maxJobs);
    ASSERT_EQ(scenario.jobCount, maxJobs);

    for (auto& job : scenario.jobs)
    {
        job->wait();
    }
    EXPECT_FALSE(threadPool->isBusy());
}

TEST_F(TaskSchedulerTest, testAddQueuedJobs)
{
    QueuedTaskScenario<QueuedJob> scenario(*this, 1u);
    EXPECT_EQ(scenario.jobCount, 1u);

    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Second test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Third test string");
    threadPool->schedule(std::chrono::milliseconds(10));
    EXPECT_EQ(threadPool->getThreadCount() - 1u, threadPool->getIdleCount());
    EXPECT_TRUE(threadPool->isBusy());
    EXPECT_EQ(m_output->getBuffer().size(), 3u);
}

TEST_F(TaskSchedulerTest, stressTestExclusiveJobs)
{
    QueuedTaskScenario<QueuedJob> scenario(*this, threadPool->getThreadCount());
    EXPECT_EQ(scenario.jobCount, threadPool->getThreadCount());

    // threadPool->schedule(std::chrono::milliseconds(100));
    EXPECT_EQ(0u, threadPool->getIdleCount());
    EXPECT_TRUE(threadPool->isBusy());

}

TEST_F(TaskSchedulerTest, reschedulingTask)
{
    ReschedulingTaskSchenario<ReusableJob> scenario(*this, 1u);
    scenario[0]->push("1st string");
    scenario[0]->push("2nd string");
    scenario[0]->push("3rd string");
    scenario[0]->push("4th string");
    threadPool->schedule();
    scenario[0]->wait();

    EXPECT_EQ(4u, m_output->getBuffer().size());
    EXPECT_GE(scenario[0]->rescheduleCount, 1u);
}

TEST_F(TaskSchedulerTest, stressTestReschedulingTask)
{
    constexpr auto stressCount = 1000;
    ReschedulingTaskSchenario<ReusableJob> scenario(*this, 1u);

    for (auto i = 0; i < stressCount; ++i)
    {
        scenario[0]->push("example text to get printed with a reusable job");
    }
    threadPool->schedule();
    scenario[0]->wait();
    EXPECT_EQ(stressCount, m_output->getBuffer().size());
    EXPECT_GE(scenario[0]->rescheduleCount, 1u);
    // std::cerr << "reschedule count = " << scenario[0]->rescheduleCount << std::endl;
}
