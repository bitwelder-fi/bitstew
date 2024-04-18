[TOC]

# Guarded Containers

Guarded containers are wrapers around the usual standard library containers. They guard the containers agains desctructive operations where recursive application logic requires insertion, adding or removing elements of the container. To ensure the content is properly guarded, lock the container using [LockView](./view.hpp#LockView).

## Creating containers

When creating guarded containers, you must set the invalid element for the container. Guarded containers set this value on erase or clear operations when the container is guard-locked.

The template argument of a guarded container gets the container type to guard. The value type gets deducted from the container type to know the type of the invalid element.

The following example declares a vector of signed integers, and initializes the container with the minimum value of the integer as invalid element.

```cpp
using GuardedIntVector = containers::GraudedSequenceContainer<std::vector<int>>;
auto container = GuardedIntVector(std::numeric_limits<int>::min());
```

## Container size

Guarded containers have two types of sizes: 

* The content size is the number of valid elements.
* The effective size is the total container size, which includes both valid and invalid elements.

Both of these sizes are less than the capacity of the container.

## Iterators and Views

Guarded containers use custom iterators, which wrap the original container iterator and skip the invalid elements when incremented or decremented.

A view is a range within which you can operate. However, to safely operate in a view, you must guard-lock the view.

## Guard-locking

When you guard-lock a container, that creates a locked view of the actual container. This means that the size of the locked view cannot grow, but can shrink. You can add, insert or remove elements from the container within or outside the guarded view. On a container that is not guard-locked, every operation gets executed straight on the container. However, only a subset of operations is allowed on a guard-locked container:

* You can add elements to a guarded container the same way as to a non-guarded one.
* You can only insert elements outside of the guarded area of a container. Inserting elements in the guarded view fails.
* You can erase elements inside or outside of the guarded view.

Use iterators to access elements of a guarded container.



# Safe queues

**Stew** provides two types of thread safe queues:

- [CircularBuffer<>](./safe_queue.hpp#CircularBuffer) - a thread-safe non-locking buffer of finite number of elements.
- [SharedQueue<>](./safe_queue.hpp#SharedQueue) - a thread-safe locking buffer of infinite number of elements.

You can use these queues in your jobs to implement reschedulable jobs (preferably with `CircularBuffer`), or locking queues (with `SharedQueue`).
