# Meta
A library of metaprogramming

This is a fresh remake of MOX, with different comncepts, based on elements from C++20.

*under construction*

# The library

**Meta** library is a utility for you to provide a bridge between scripting and your native C++ code. The library consists of a set of lose-coupled components, and utilities. The core components are grouped in a singleton, the [Library](./include/meta/meta.hpp), which you must initialize at your application startup, and respectively uninitialize on application tear down.

```cpp
#include <meta/meta.hpp>

int main(...)
{
    // Initialize the meta library with the default configuration:
    // - thread count set to the maximum thread capability of the HW
    // - log level set to Debug.
    meta::Library::instance().initialize();

    // Your code comes here.

    // Tear down meta library.
    meta::Library::instance().uninitialize();
}
```

# Library components

## Packaging arguments

As stated, **Meta** is a bridge between scripting and your native C++ code. For this, variadic arguments you use in your C++ application must be packaged into a dynamic form. [Argument](./include/meta/arguments/argument.hpp) and [PackagedArguments](./include/meta/arguments/packaged_arguments.hpp) provide this mechanism for you to package the arguments for dynamic transportation, as well as to unpack those into an invokable tuple.

## Meta data, meta class

Next to the dynamic argument packaging, an essential contributor for the scripting is the stringified meta data. Adopting the metadata to your classes allows you to expose those to scripting access. Each class can have a meta class, where each meta class has its unique name. To expose those to scripting, you must register the meta class to the [ObjectFactory](./include/meta/metadata/factory.hpp) of the meta Library.

To declare the static mata-class for a class which derives from [Object](./include/meta/object.hpp):
```cpp
class MyClass : public meta::Object
{
public:
    META_CLASS("MyClass", MyClass, Object)
    {
    };

    // Declare the object factory function.
    static auto create(std::string_view objectName)
    {
        // Do a two-phased initialization.
        auto result = std::shared_ptr<MyClass>(new MyClass(objectName));
        // Note: provide your own initialization if required.
        result->initialize();
        return result;
    }

protected:
    explicit MyClass(std::string_view objectName) :
        meta::MetaObject(objectName)
    {
    }

    void initialize()
    {
        // Don't forget to call the base class initialization before you do your own!
        meta::Object::initialize();

        // Continue initializing your object.
    }
};
```

To declare the static mata-class for a class which does not derive from [MetaObject](./include/meta/metadata/meta_object.hpp):
```cpp
class MyClass
{
public:
    STATIC_META_CLASS("MyClass", MyClass)
    {
    };
};
```
Note that **Meta** can only instantiate classes which derive from
- [meta::MetaObject](./include/meta/metadata/meta_object.hpp) (if those have no meta extensions).
- [meta::Object](./include/meta/object.hpp) (if those must have meta extensions).

See [MetaClass](./include/meta/metadata/metaclass.hpp) for further documentation and examples.

## Thread pool

`std::async()` documentation states that its implementation may or may not be pooled. **meta** comes with a thread pool implementation, which queues jobs whose state you can observe during the execution time of the job. You can create [Job](./include/meta/tasks/job.hpp)s which can re-schedule themselves on completion, or create jobs with queues, where you can also use one of the safe queues provided by the library.

To run a job with thread pool, use `meta::async()` or using the thread pool of the library.

You can configure the number of threads of the pool at the library initialization phase.

## Safe queues

**Meta** provides two types of thread safe queues:
- [CircularBuffer<>](./include/meta/safe_queue.hpp#CircularBuffer) - a thread-safe non-locking buffer of finite number of elements.
- [SharedQueue<>](./include/meta/safe_queue.hpp#SharedQueue) - a thread-safe locking buffer of infinite number of elements.

You can use these queues in your jobs to implement reschedulable jobs (preferably with `CircularBuffer`), or locking queues (with `SharedQueue`).

## Logging

You can enable tracing at build time, by turning `CONFIG_ENABLE_LOGS` build flag on. The log level gets configured at the library initialization time. The [Tracer](./include/meta/log/trace.hpp) is a self-rescheduling job of the **Meta** library, which the logging system component uses to print logs.

You can install multiple log printers to **Meta**, and each log printer can have a chain of log decorators. **Meta** provides the following printers and decorators:
- [TracePrinter](./include/meta/log/trace_printer.hpp#TracePrinter) - the log printer interface.
- [ConsoleOut](./include/meta/log/trace_printer.hpp#ConsoleOut) - the log printer to print to `std::cout`.

and the following decorators:
- [PrinterDecoratorInterface](./include/meta/log/trace.hpp#PrinterDecoratorInterface) - the printer decorator interface.
- [LogLevelDecorator](./include/meta/log/trace.hpp#LogLevelDecorator) - decorates the log with readable log level.
- [ThreadIdDecorator](./include/meta/log/trace.hpp#ThreadIdDecorator) - decorates the log with the thread ID.
- [FunctionDecorator](./include/meta/log/trace.hpp#FunctionDecorator) - decorates the log with function name.
- [FileLineDecorator](./include/meta/log/trace.hpp#FileLineDecorator) - decorates the log with file and line number.
- [MessageDecorator](./include/meta/log/trace.hpp#MessageDecorator) - decorates the log with the log message.

To trace logs, use the dedicated macros for the log level:
- `META_LOG_FATAL(text)` to log fatal errors.
- `META_LOG_ERROR(text)` to log recoverable errors.
- `META_LOG_WARNING(text)` to log warnings.
- `META_LOG_INFO(text)` to log info traces.
- `META_LOG_DEBUG(text)` to log debug traces.

## Object, Object Extensions

[Object](./include/meta/object.hpp)s provide extendable functionality to your classes, with meta-data. You can dynalically attach extensions to them, where the extensions are derived from [ObjectExtension](./include/meta/object_extensions/object_extension.hpp). Both Object and ObjectExtension derive from [MetaObject](./include/meta/metadata/meta_object.hpp), which enables the dynamic meta-class queries from an instance.

Though Object Extensions provide dynamic functionality, those can still be associated with objects statically. Signals defined on a class, for instance, extend the class at compile time, and they are added to the instance of the class even if those were not created through the `ObjectFactory`.

You can invoke an object extension either through the `Object::invoke()` method, or through the [meta::invoke()]((./include/meta/object.hpp#invoke)) function.

**Meta** provides the following ready-made extensions:
- [Invokable](./include/meta/object_extensions/invokable.hpp) - to declare an invokable extension for a method, a function, or a lambda.
- [Signal](./include/meta/object_extensions/signal.hpp) - to declare a free signal, or a signal extension for your class.

### Meta-extensions

Instances are created from an Object derived class through the `create()` static function. Objects which define meta-data can register extensions to their meta-class, which when created with the `ObjectFactory`, or through the meta-class factory itself, will add the object extensions to the factory created instance automatically. Whilst the same `create()` static function is called by the MetaClass, only meta-class can add meta-extension created extensiomns to the instance.
Example:
```cpp
class MyObject : public meta::Object
{
public:
    // Declare an invokable to the getName() method.
    DECLARE_INVOKABLE(GetName, "getName', &Object::getName);

    // The meta-class.
    META_CLASS("MyObject", MyObject, meta::Object)
    {
        META_EXTENSION(GetName);
    };

    static auto create(std::string_view name)
    {
        return std::shared_ptr<MyObject>(new MyObject(name));
    }

protected:
    explicit MyObject(std::string_view name) :
        meta::Object(name)
    {
    }
};

// Later in your code:
auto object = MyObject::create("noFactory");
if (!object->findExtension("getName"))
{
    std::cout << "Non-factory object, should have no getName extension" << std::endl;
}

// Factory object.
auto object = MyObject::getStaticMetaClass()->create("factory");
if (object->findExtension("getName"))
{
    std::cout << "Factory object, has getName extension" << std::endl;
}
```

You can query whether an instabnce is factory-created instance by calling the `Object::getFactory()` method.

You can add meta-extensions to static meta-classes at compile time only. These meta-classes are sealed. To extend a meta-class at runtime, your class must provide an un-sealed static meta-class, and register that meta-class to the `ObjectFactory`.
Example of a class which provides an un-sealed static meta-class in addition to a meta-class:
```cpp
class DynamicObject : public metaObject
{
public:
    // The static meta-class of this class.
    STATIC_META_CLASS("DynamicObject", DynamicObject, meta::Object)
    {
    };

    // The un-sealed static meta-class of this class.
    class UnsealedMetaClass : public DynamicObject::StaticMetaClass
    {
    public:
        explicit UnsealedMetaClass(std::string_view className) :
            StaticMetaClass(className)
        {
            m_descriptor->sealed = false;
        }
    };

    // DynamicObject needs an override of the dynamic meta-class, to provide the proper meta-class.
    const meta::MetaClass* getDynamicMetaClass() const override
    {
        auto metaClass = getFactory();
        if (!metaClass)
        {
            metaClass = getStaticMetaClass();
        }
        return metaClass;
    }

    // Returns the un-sealed statci meta class.
    static meta::MetaClass* getUnsealedMetaClass()
    {
        static UnsealedMetaClass unsealedMetaClass("DynamicUnsealedObject");
        return &unsealedMetaClass;
    }

    static auto create(std::string_view name)
    {
        return std::shared_ptr<Object>(new DynamicObject(name));
    }

protected:
    explicit DynamicObject(std::string_view name) :
        ExtendedObject(name)
    {
    }
};
```

### Object Extensions: Invokable

Invokables are object extensions built to execute a signle wrapped function, method or a lambda on an object. Invokable extensions have their meta-class name generated from the signature of the wrapped callable. Should this be inconvenient, subclass the Invokable<> template and define a more readable meta-name for the meta-class of your extension.

For convenience, **Meta** provides two macros which enables you to define the invokable type and its meta-name:
- Use the `DECLARE_INVOKABLE()` macro to declare a human readable name for a function.
- Use the `DECLARE_INVOKABLE_OVERRIDE()` macro to declare a human readable name for an override of a function.

### Object Extensions: Signal

Signals are object extensions which implement the observer-observant pattern in **Meta**. They connect a source object with a set of targets through connection objects. These connected targets then get executed when the signal gets triggered.

Whilst signals are dedicated to work through Objects, they can be used as individual, so called "orphan" signals. You can register the meta-data of any type of signal to the `ObjectFactory`, and then add those instances to objects runtime.

To define a signal:
```cpp
using VoidSignal = meta::Signal<void()>;
```

You can put this signal in your class:
```cpp
class MyObject : public meta::Object
{
public:
    VoidSignal sigVoid("sigVoid");

    META_CLASS("MyObjecty", MyObject, meta::Object)
    {
        META_EXTENSION(VoidSignal);
    };

    static auto create(std::string_view name)
    {
        return std::shared_ptr<MyObject>(new MyObject(name));
    }

protected:
    explicit MyObject(std::string_view name) :
        meta::Object(name)
    {
    }
};
```

Note that you only have to add the signal as meta-extension to you rmeta-class if you want to expose the signal to scripting. Both `MyObject::create()` and `MyObject::getStaticMetaClass()->create()` will add the signal extension to the object created. And remember to register the VoidSignal to the `ObjectFactory`.

#### Connect signals to slots (aka other object extensions)

To connect a signal to a slot, simply call the `connect()` method of the signal:
```cpp
VoidSignal sigVoid("sigVoid");

auto lambda = []() { };
using LambdaSlot = meta::Invokable<decltype(lambda), lambda>;
auto slot = LambdaSlot::create("lambda");
auto connection = sigVoid.connect(slot);
```

You can use the connection token to track the conection, or to disconnect.

#### Disconnecting from signals

To disconnect a connection from the signal above:
```cpp
sigVoid.disconnect(*connection);
```

However, you may not always have the connection toklen at your disposal. There may be cases when your object extension is descroyed while teh connection is still alive. Or the opposite case, when the signal gets destroyed while the slots are still alive.

To disconnect a slot from all the signals it is connected, call `ObjectExtension::disconnectTarget()`. This call will disconnect all the connections where the slot is set as target. The opposite is handled by the `SignalExtension`, which is the core of the **Meta** signals.
```cpp
auto selfDisconnect = [](meta::ObjectExtension* self)
{
    // The self argument is the slot object itself.
    self->disconnectTarget();
};
using SelfDisconnect = meta::Invokable<decltype(lambda), lambda>;
auto slot = SelfDisconnect::create("lambda");
sigVoid.connect(slot);
// The triger disconnects the lambda, so a consecutive trigger will not invoke it.
if (sigVoid.trigger() == 1)
{
    // The lambda was triggered.
}
if (sigVoid.trigger() == 0)
{
    // There were no more slots connected to the signal.
}
```
**Note** that the connections hold a weak reference to the source and the target of the connection. You must ensure that both of these extensions are kept alive. Expired references to either the source and the target renders the connection invalid.

You can connect slots in an activated slot. Those slots will only be executed after the current triger is over.

## Properties
*NEXT*

## Scripting
*Future, consider LUA*

# What's next

- Property types, meta properties
- Property bindigs, binding expressions
