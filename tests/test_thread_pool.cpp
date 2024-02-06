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
#include <meta/safe_queue.hpp>

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


class TestJob : public meta::Job
{
public:
    explicit TestJob(OutputPtr, SecureInt& jobCount) :
        m_jobCount(jobCount)
    {
    }

    void setStatus(Status status)
    {
        meta::Job::setStatus(status);
    }

    void run() override
    {
        ++m_jobCount;
    }

protected:
    SecureInt& m_jobCount;
};

class RescheduledJob : public meta::Job
{
    meta::ThreadPool* m_scheduler = nullptr;
    OutputPtr m_out;

public:
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

        m_queue.push(std::string(text));
        m_signal.notify_one();
        const auto status = getStatus();
        if (status == Status::Deferred)
        {
            m_scheduler->pushJob(shared_from_this());
        }
    }

protected:
    void run() override
    {
        std::unique_lock<std::mutex> lock(m_lock);
        auto condition = [this]()
        {
            return !this->m_queue.isEmpty() || isStopped();
        };
        m_signal.wait(lock, condition);
        for (;;)
        {
            auto text = m_queue.pop();
            if (text.empty())
            {
                break;
            }
            m_out->write(text);
        }
    }

    void stopOverride() override
    {
        m_signal.notify_all();
    }

    std::mutex m_lock;
    std::condition_variable m_signal;
    meta::SharedQueue<std::string> m_queue;
};

class QueuedJob : public TestJob
{
    OutputPtr m_out;
public:
    explicit QueuedJob(OutputPtr out, SecureInt& jobCount) :
        TestJob(out, jobCount),
        m_out(out)
    {
    }

    void push(std::string string)
    {
        if (isStopped())
        {
            return;
        }

        m_queue.push(string);
        m_signal.notify_all();
    }

protected:
    void run() override
    {
        ++m_jobCount;
        while (!isStopped())
        {
            std::unique_lock<std::mutex> lock(m_lock);
            auto condition = [this]()
            {
                return isStopped() || !this->m_queue.isEmpty();
            };
            m_signal.wait(lock, condition);
            for (;;)
            {
                auto text = m_queue.pop();
                if (text.empty())
                {
                    break;
                }
                m_out->write(text);
            }
        }
    }

    void stopOverride() override
    {
        m_signal.notify_all();
    }

    std::mutex m_lock;
    std::condition_variable m_signal;
    meta::SharedQueue<std::string> m_queue;
};

class TaskSchedulerTest : public ::testing::Test
{
protected:
    std::unique_ptr<meta::ThreadPool> threadPool;
    OutputPtr m_output;

    void SetUp() override
    {
        threadPool = std::make_unique<meta::ThreadPool>(std::thread::hardware_concurrency());
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
        std::vector<meta::JobPtr> jobs;

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
            test.threadPool->pushMultipleJobs(this->jobs);
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

    std::size_t jobCount = 0u;
    for (auto& job : scenario.jobs)
    {
        ++jobCount;
        job->wait();
    }    
    EXPECT_EQ(jobCount, maxJobs);
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
    ReschedulingTaskSchenario<RescheduledJob> scenario(*this, 1u);
    scenario[0]->push("1st string");
    scenario[0]->push("2nd string");
    scenario[0]->push("3rd string");
    scenario[0]->push("4th string");
    threadPool->schedule();
    scenario[0]->wait();

    EXPECT_EQ(4u, m_output->getBuffer().size());
}
