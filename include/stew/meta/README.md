[TOC]

# Meta data, meta class

Next to the dynamic argument packaging, an essential contributor for the scripting is the stringified meta data. Adopting the metadata to your classes allows you to expose those to scripting access. Each class can have a meta class, where each meta class has its unique name. To expose those to scripting, you must register the meta class to the [ObjectFactory](./metadata/factory.hpp) of the meta Library.

To declare the static mata-class for a class which derives from [Object](./object.hpp):

```cpp
class MyClass : public stew::Object
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
        stew::MetaObject(objectName)
    {
    }

    void initialize()
    {
        // Don't forget to call the base class initialization before you do your own!
        stew::Object::initialize();

        // Continue initializing your object.
    }
};
```

To declare the static mata-class for a class which does not derive from [MetaObject](./metadata/meta_object.hpp):

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

- [stew::MetaObject](./metadata/meta_object.hpp) (if those have no meta extensions).
- [stew::Object](./object.hpp) (if those must have meta extensions).

See [MetaClass](./metadata/metaclass.hpp) for further documentation and examples.

# Object, Object Extensions

[Object](./object.hpp)s provide extendable functionality to your classes, with meta-data. You can dynamically attach extensions to them, where the extensions are derived from [ObjectExtension](./object_extensions/object_extension.hpp). Both Object and ObjectExtension derive from [MetaObject](./metadata/meta_object.hpp), which enables the dynamic meta-class queries from an instance.

Though Object Extensions provide dynamic functionality, those can still be associated with objects statically. Signals defined on a class, for instance, extend the class at compile time, and they are added to the instance of the class even if those were not created through the `ObjectFactory`.

You can invoke an object extension either through the `Object::invoke()` method, or through the [stew::invoke()]((./object.hpp#invoke)) function.

**Meta** provides the following ready-made extensions:

- [Invokable](./object_extensions/invokable.hpp) - to declare an invokable extension for a method, a function, or a lambda.
- [Signal](./object_extensions/signal.hpp) - to declare a free signal, or a signal extension for your class.

## Meta-extensions

Instances are created from an Object derived class through the `create()` static function. Objects which define meta-data can register extensions to their meta-class, which when created with the `ObjectFactory`, or through the meta-class factory itself, will add the object extensions to the factory created instance automatically. Whilst the same `create()` static function is called by the MetaClass, only meta-class can add meta-extension created extensions to the instance.
Example:

```cpp
class MyObject : public stew::Object
{
public:
    // Declare an invokable to the getName() method.
    DECLARE_INVOKABLE(GetName, "getName', &Object::getName);

    // The meta-class.
    META_CLASS("MyObject", MyObject, stew::Object)
    {
        META_EXTENSION(GetName);
    };

    static auto create(std::string_view name)
    {
        auto object = std::shared_ptr<MyObject>(new MyObject(name));
        object->initialize();
        return object;
    }

protected:
    explicit MyObject(std::string_view name) :
        stew::Object(name)
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
class DynamicObject : public stew::MetaObject
{
public:
    // The static meta-class of this class.
    STATIC_META_CLASS("DynamicObject", DynamicObject, stew::Object)
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
    const stew::MetaClass* getDynamicMetaClass() const override
    {
        auto metaClass = getFactory();
        if (!metaClass)
        {
            metaClass = getStaticMetaClass();
        }
        return metaClass;
    }

    // Returns the un-sealed statci meta class.
    static stew::MetaClass* getUnsealedMetaClass()
    {
        static UnsealedMetaClass unsealedMetaClass("DynamicUnsealedObject");
        return &unsealedMetaClass;
    }

    static auto create(std::string_view name)
    {
        auto object = std::shared_ptr<DynamicObject>(new DynamicObject(name));
        object->initialize();
        return object;
    }

protected:
    explicit DynamicObject(std::string_view name) :
        ExtendedObject(name)
    {
    }
};
```

## Object Extensions: Invokable

Invokables are object extensions built to execute a signle wrapped function, method or a lambda on an object. Invokable extensions have their meta-class name generated from the signature of the wrapped callable. Should this be inconvenient, subclass the Invokable<> template and define a more readable meta-name for the meta-class of your extension.

For convenience, **Meta** provides two macros which enables you to define the invokable type and its meta-name:

- Use the `DECLARE_INVOKABLE()` macro to declare a human readable name for a function.
- Use the `DECLARE_INVOKABLE_OVERRIDE()` macro to declare a human readable name for an override of a function.

## Object Extensions: Signal

Signals are object extensions which implement the observer-observant pattern in **Meta**. They connect a source object with a set of targets through connection objects. These connected targets then get executed when the signal gets triggered.

Whilst signals are dedicated to work through Objects, they can be used as individual, so called "orphan" signals. You can register the meta-data of any type of signal to the `ObjectFactory`, and then add those instances to objects runtime.

To define a signal:

```cpp
using VoidSignal = stew::Signal<void()>;
```

You can put this signal in your class:

```cpp
class MyObject : public stew::Object
{
public:
    VoidSignal sigVoid("sigVoid");

    META_CLASS("MyObjecty", MyObject, stew::Object)
    {
        META_EXTENSION(VoidSignal);
    };

    static auto create(std::string_view name)
    {
        auto object = std::shared_ptr<MyObject>(new MyObject(name));
        object->initialize();
        return object;
    }

protected:
    explicit MyObject(std::string_view name) :
        stew::Object(name)
    {
    }
};
```

Note that you only have to add the signal as meta-extension to you rmeta-class if you want to expose the signal to scripting. Both `MyObject::create()` and `MyObject::getStaticMetaClass()->create()` will add the signal extension to the object created. And remember to register the VoidSignal to the `ObjectFactory`.

### Connect signals to slots (aka other object extensions)

To connect a signal to a slot, simply call the `connect()` method of the signal:

```cpp
VoidSignal sigVoid("sigVoid");

auto lambda = []() { };
using LambdaSlot = stew::Invokable<decltype(lambda), lambda>;
auto slot = LambdaSlot::create("lambda");
auto connection = sigVoid.connect(slot);
```

You can use the connection token to track the connection, or to disconnect.

### Disconnecting from signals

To disconnect a connection from the signal above:

```cpp
sigVoid.disconnect(*connection);
```

However, you may not always have the connection token at your disposal. There may be cases when your object extension is destroyed while the connection is still alive. Or the opposite case, when the signal gets destroyed while the slots are still alive.

To disconnect a slot from all the signals it is connected, call `ObjectExtension::disconnectTarget()`. This call will disconnect all the connections where the slot is set as target. The opposite is handled by the `SignalExtension`, which is the core of the **Meta** signals.

```cpp
auto selfDisconnect = [](stew::ObjectExtension* self)
{
    // The self argument is the slot object itself.
    self->disconnectTarget();
};
using SelfDisconnect = stew::Invokable<decltype(lambda), lambda>;
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

You can connect slots in an activated slot. Those slots will only be executed after the current triger is over. The following example connects a new slot to the signal which activated the slot.

```cpp
class MyObject : public stew::Object
{
public:
    VoidSignal sigVoid{*this, "sigVoid"};

    // Connect itself to the signal which triggers this extension. This doubles the connections at 
    // each trigger of the signal.
    void connectInSlot(stew::CallContextPtr context)
    {
        auto connection = std::static_pointer_cast<stew::Connection>(context);

        // Create an object extension with a different name, so that it gets successfully added to the object.
        auto innerSlot = ConnectInSlot::create(generateName());
        addExtension(innerSlot);

        // Connect to the signal of the connection.
        connection->getSource<stew::SignalExtension>()->connect(innerSlot);
    }

    DECLARE_INVOKABLE(ConnectInSlot, "connectInSlot", &Object::connectInSlot);

    META_CLASS("Object", Object, stew::Object)
    {
        META_EXTENSION(VoidSignal);
        META_EXTENSION(ConnectInSlot);
    };

    static auto create(std::string_view name)
    {
        auto object = std::shared_ptr<MyObject>(new MyObject(name));
        object->initialize();
        return object;
    }

protected:
    explicit Object(std::string_view name) :
        stew::Object(name)
    {
    }

    // Generates a name using the time since epoch.
    static std::string generateName()
    {
        std::stringstream ss;
        ss << "slot_" << std::chrono::steady_clock().now().time_since_epoch().count();
        return ss.str();
    }
};

// Connect the `connectInSlot` extension to the signal, then trigger.
auto object = MyObject::create("test");
object->sigVoid.connect("connectInSlot");
object->sigVoid.trigger();
```

# Properties

*NEXT*