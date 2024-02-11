/*
 * Copyright (C) 2024 bitWelder
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

#ifndef META_JOB_HPP
#define META_JOB_HPP

#include <meta/meta_api.hpp>
#include <meta/tasks/thread_pool.hpp>

#include <memory>

namespace meta
{

class Job;
using JobPtr = std::shared_ptr<Job>;

/// Represents a job executed by the thread pool. Derive from this to write jobs or queues.
///
/// To create a simple job:
/// \code
/// class SimpleJob : public Job
/// {
/// protected:
///     //...
///     void run() override
///     {
///         // put your code here.
///     }
/// };
/// \endcode
///
/// \name Processing queues
///
/// Depending on the queue type, you can use a different setup with a Job to process the queue.
/// Processing a circular buffer does not necessarily require to reserve a thread, whilst processing
/// a shared queue most likely would be more efficient when the queue resides in a steady thread.
///
/// A job which proceses a circular buffer would need to reschedule itself, so that it can properly
/// consume the buffer content. You reschedule the job by overriding the onCompleted() method, and
/// try to schedule it if the buffer has some content.
/// \code
/// class Buffer : public Job
/// {
///     // A thread-safe circular buffer of 10 elements.
///     CircularBuffer<std::string, 10u> m_buffer;
///
/// public:.
///     // Pushes the text into the buffer.
///     void push(std::string text)
///     {
///         // Try to push the job into the buffer. If the push fails, try to schedule the job,
///         // so that it can process the buffer.
///         auto scheduled = false;
///         while (!m_buffer.tryPush(text))
///         {
///             scheduled != async(shared_from_this());
///         }
///         // If it was a successful push, or there was a successful push after an unsuccessful one,
///         // schedule itself for processing.
///         if (!scheduled)
///         {
///             async(shared_from_this());
///         }
///     }
///
/// protected:
///     //...
///     void run() override
///     {
///         // Process the buffer.
///         for (auto text = m_buffer.tryPop(); !text.empty(); text = m_buffer.tryPop())
///         {
///             // Do somethign with the text.
///         }
///     }
///
///     // Override Job::onCompleted() and check if the buffer already has some data added to process.
///     // If there is data to process, reschedule.
///     void onCompleted() override
///     {
///         if (m_buffer.wasEmpty())
///         {
///             return;
///         }
///         // Reschedule.
///         async(shared_from_this());
///     }
/// };
/// \endcode
///
/// To create a job which processes a shared queue, put a loop to process the queue. This loops are
/// long running tasks, which lock the job to the thread which runs the job. Locked threads must check
/// in their loop whether they got stopped by the thread pool, and bail out the loop if they were.
///
/// The following example uses a SharedQueue, and sets itself as the notifier type for the queue. For
/// this, the SharedQueue requires the noitifier a notifyOne() and a wait() method. The push and pop
/// methods are provided by SharedQueue.
/// \code
/// class Queue : public Job, public SharedQueue<std::string, Queue>
/// {
///     // The Queue is the notifier for the shared queue.
///     std::condition_variable m_signal;
///
/// public:
///     // The notifier for the SharedQueue.
///     void notifyOne()
///     {
///         m_signal.notify_one();
///     }
///     // The wait for the SharedQueue.
///     void wait(std::unique_lock<std::mutex>& lock)
///     {
///         auto condition = [this]()
///         {
///             return isStopped() || !nolock_isEmpty();
///         };
///         m_signal.wait(lock, condition);
///     }
///
/// protected:
///     //...
///     void run() override
///     {
///         while (!isStopped())
///         {
///             auto processor = [this](auto data)
///             {
///                 if (data.empty())
///                 {
///                     return false;
///                 }
///                 // Process the data.
///                 // ...
///                 return true;
///             };
///             forEach(processor);
///         }
///     }
///
///     // Override Job::stopOverride() to signal the condition variable when the thread pool stops
///     // the running jobs.
///     void stopOverride() override
///     {
///         // Notifies the wait in the run() method to wake up and check the conditiom.
///         m_signal.notify_all();
///     }
/// };
/// \endcode
class META_API Job : public ThreadPool::BaseJob
{
public:
    /// The status of the job.
    enum class Status
    {
        /// The job is deferred.
        Deferred,
        /// The job is queued for excecution.
        Queued,
        /// The job is running.
        Running,
        /// The job is completed its duties.
        Completed,
        /// The job is stopped.
        Stopped
    };

    /// Destructor.
    virtual ~Job();

    /// Returns the status of the job.
    /// \return The status of the job.
    Status getStatus() const;

    /// Stops the job. Call this to schedule the downwinding of a job. To make this effective, you
    /// must check the stopped state of the job.
    void stop();

    /// Returns whether the stop got signalled on the job.
    bool isStopped() const;

    /// Returns whether the job is queued or running.
    bool isBusy() const;

    /// Waits for the job to complete.
    void wait();

    /// Overloads enable_shared_from_this
    JobPtr shared_from_this()
    {
        return std::static_pointer_cast<Job>(std::enable_shared_from_this<ThreadPool::BaseJob>::shared_from_this());
    }
protected:
    /// Constructor.
    explicit Job();

    /// Implement BaseJob interface.
    bool canQueue() const override;
    void queue() final;
    void schedule() final;
    void complete() final;


    /// The main function of the job. Override this to implement job specific logic.
    virtual void run() = 0;

    /// Override this to implement job specific stop logic.
    virtual void stopOverride(){}

    /// Called by the thread pool when the job gets queued.
    virtual void onQueued(){}
    /// Called by the the thread pool when the job gets completed.
    virtual void onCompleted(){}

    /// Sets the status of the job.
    /// \param status The status to set.
    void setStatus(Status status);

private:
    struct Descriptor;
    std::unique_ptr<Descriptor> descriptor;
};


/// Executes the job asynchronously. To wait for the job completion, call Job::wait() methods.
/// \param job The job to execute.
/// \return If the job got scheduled with success, returns \e true, otherwise \e false.
bool META_API async(JobPtr job);

}

#endif // META_JOB_HPP

