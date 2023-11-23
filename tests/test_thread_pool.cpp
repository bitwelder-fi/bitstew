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

#include <gtest/gtest.h>
#include <meta/meta.hpp>
#include <meta/library_config.hpp>
#include <meta/thread_pool/thread_pool.hpp>

#include <string>

using namespace meta::thread_pool;

namespace
{

using SecureInt = std::atomic<size_t>;

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
        m_jobCount++;
    }

    SecureInt& m_jobCount;
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
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_queue.push(string);
        }
        m_signal.notify_one();
    }

    void notifyTaskQueued() override
    {
    }

protected:
    void runOverride() override
    {
        m_jobCount++;
        while (!isStopped())
        {
            std::unique_lock<std::mutex> lock(m_lock);
            auto condition = [this]()
            {
                return !this->m_queue.empty() || isStopped();
            };
            m_signal.wait(lock, condition);
            if (m_queue.empty())
            {
                continue;
            }
            auto text = m_queue.front();
            m_queue.pop();
            std::cout << "Queued log: " << text << std::endl;
        }
    }

    void stopOverride() override
    {
        m_signal.notify_one();
    }

    std::mutex m_lock;
    std::condition_variable m_signal;
    std::queue<std::string> m_queue;
};

class ThreadPoolTest : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        auto arguments = meta::LibraryArguments();
        arguments.threadPool.createThreadPool = false;
        meta::Domain::instance().initialize(arguments);
    }
    static void TearDownTestSuite()
    {
        meta::Domain::instance().uninitialize();
    }

protected:
    std::unique_ptr<ThreadPool> threadPool;

    void SetUp() override
    {
        threadPool = std::make_unique<meta::thread_pool::ThreadPool>(std::thread::hardware_concurrency());
        if (!threadPool->isRunning())
        {
            threadPool->start();
        }
    }

    void TearDown() override
    {
        if (threadPool)
        {
            threadPool->stop();
        }
        threadPool.reset();
    }

    template <typename JobType>
    struct QueuedTaskScenario
    {
        SecureInt jobCount = 0u;
        ThreadPoolTest& test;
        std::vector<TaskPtr> jobs;
        std::vector<TaskFuture> futures;

        explicit QueuedTaskScenario(ThreadPoolTest& test, size_t tasks) :
            test(test)
        {
            jobs.reserve(tasks);
            while (tasks-- != 0u)
            {
                jobs.push_back(std::make_shared<JobType>(jobCount));
            }
            futures = test.threadPool->addTasks(jobs);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    };
};

class ThreadPoolStressTest : public ThreadPoolTest
{
protected:
};

}

TEST(Tasks, testJob)
{
    SecureInt jobCount = 0u;
    Job job(jobCount);
    job.setStatus(Job::Status::Queued);
    job.run();
    EXPECT_EQ(Job::Status::Stopped, job.getStatus());
    EXPECT_EQ(1u, jobCount);
}

TEST_F(ThreadPoolTest, testAddJobs)
{
    constexpr auto maxJobs = 50u;
    QueuedTaskScenario<Job> scenario(*this, maxJobs);
    EXPECT_EQ(scenario.jobCount, maxJobs);

    size_t jobCount = 0u;
    for (auto& future : scenario.futures)
    {
        ++jobCount;
        future.wait();
    }    
    EXPECT_EQ(jobCount, maxJobs);
    EXPECT_FALSE(threadPool->isBusy());
}

TEST_F(ThreadPoolTest, testAddQueuedJobs)
{
    QueuedTaskScenario<QueuedJob> scenario(*this, 1u);
    EXPECT_EQ(scenario.jobCount, 1u);

    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Second test string");
    std::static_pointer_cast<QueuedJob>(scenario.jobs.back())->push("Third test string");
    std::this_thread::yield();
    EXPECT_EQ(threadPool->getThreadCount() - 1u, threadPool->getIdleCount());
    EXPECT_TRUE(threadPool->isBusy());
}

TEST_F(ThreadPoolTest, stressTestExclusiveJobs)
{
    QueuedTaskScenario<QueuedJob> scenario(*this, threadPool->getThreadCount());
    EXPECT_EQ(scenario.jobCount, threadPool->getThreadCount());

    EXPECT_EQ(0u, threadPool->getIdleCount());
    EXPECT_TRUE(threadPool->isBusy());
}
