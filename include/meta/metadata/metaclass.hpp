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

#ifndef META_METACLASS_HPP
#define META_METACLASS_HPP

#include <meta/meta_api.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace meta
{

class Callable;
class MetaObject;
using MetaObjectPtr = std::shared_ptr<MetaObject>;


class META_API Metaclass
{
public:
    /// Destructor.
    virtual ~Metaclass() = default;

    /// Creates an object of the class to which the meta class is connected.
    /// \param name The name of the meta object created.
    MetaObjectPtr create(std::string_view name) const
    {
        return m_descriptor->create(name);
    }

    /// Cast the created meta object to the given class type.
    template <class ClassType>
    std::shared_ptr<ClassType> create(std::string_view name) const
    {
        return std::dynamic_pointer_cast<ClassType>(create(name));
    }

    /// Returns the meta class of the base class at index.
    /// \param index The base class index.
    /// \return The meta class of the base class at index.
    const Metaclass* getBaseClass(size_t index) const
    {
        return m_descriptor->getBaseClass(index);
    }

    /// Returns the number of base classes with meta data.
    /// \return The number of base classes with meta data.
    size_t getBaseClassCount() const
    {
        return m_descriptor->getBaseClassCount();
    }

    /// Returns whether the class to which the meta class is connected is abstract.
    /// \return If the class to which the meta class is connected is abstract, returns \e true,
    ///         otherwise \e false.
    bool isAbstract() const
    {
        return m_descriptor->isAbstract();
    }

    /// Returns whether this meta class is the meta class of the meta object.
    /// \param The meta object to check.
    /// \return If this meta class is the meta class of the meta object, returns \e true, otherwise
    ///         \e false.
    bool isMetaClassOf(const MetaObject& object) const
    {
        return m_descriptor->isMetaClassOf(object);
    }

    /// Returns whether this MetaClass is or is derived from the \a metaClass.
    /// \param metaClass The metaClass instance to check.
    /// \return If this MetaClass is or is derived from the metaClass, returns \e true, otherwise
    ///         \e false.
    bool isDerivedFrom(const Metaclass& metaClass) const;

    template <class TDerivedClass>
    bool isDerivedFromClass() const
    {
        return m_descriptor->hasSuperClass(TDerivedClass::staticMetaclass);
    }

    /// Adds a callable to the meta class. The callable is either a method of the class, or an attached
    /// function, or a lambda. The method fails if the meta class already has a callable with the same
    /// name.
    /// \param callable The callable object to add.
    /// \return If the method is added with success, returns \e true, otherwise \e false. On failure,
    ///         and tracing is enabled, an error log is printed.
    /// \see Callable
    bool addMethod(Callable& callable);

    /// Tries to find a callable with the given name.
    /// \param name The name of the callable to find.
    /// \return The callable with the name, or \e nullptr if no callable with the name is found.
    /// \see Callable
    Callable* findMethod(std::string_view name);

protected:
    struct META_API DescriptorInterface
    {
        virtual ~DescriptorInterface() = default;
        virtual MetaObjectPtr create(std::string_view /*name*/) const
        {
            return {};
        }
        virtual const Metaclass* getBaseClass(size_t /*index*/) const
        {
            return {};
        }
        virtual size_t getBaseClassCount() const
        {
            return 0u;
        }
        virtual bool isAbstract() const = 0;
        virtual bool isMetaClassOf(const MetaObject& object) const = 0;
        /// Checks whether a \a metaClass is a super metaclass of this metaclass.
        virtual bool hasSuperClass(const Metaclass& /*metaClass*/) const
        {
            return false;
        }
    };
    using DescriptorPtr = std::unique_ptr<DescriptorInterface>;

    /// Constructor. Creates a metaclass with a base class and name.
    explicit Metaclass(std::string_view name, DescriptorPtr descriptor) :
        m_name(name),
        m_descriptor(std::move(descriptor))
    {
    }

    /// The callable container type.
    using CallableContainer = std::unordered_map<std::string_view, Callable*>;

    /// The name of the meta class.
    std::string m_name;
    /// The callable container of the meta class.
    CallableContainer m_callables;
    DescriptorPtr m_descriptor;
};


namespace detail
{

template <class DeclaredClass>
class META_TEMPLATE_API BaseMetaclass : public Metaclass
{
    struct META_API Descriptor : DescriptorInterface
    {
        MetaObjectPtr create(std::string_view name) const override
        {
            if constexpr (std::is_abstract_v<DeclaredClass>)
            {
                return {};
            }
            else
            {
                return DeclaredClass::create(name);
            }
        }

        bool isAbstract() const override
        {
            return std::is_abstract_v<DeclaredClass>;
        }
        bool isMetaClassOf(const MetaObject& object) const override
        {
            auto address = dynamic_cast<const DeclaredClass*>(&object);
            return address != nullptr;
        }
    };

public:
    explicit BaseMetaclass(std::string_view name) :
        Metaclass(name, std::make_unique<Descriptor>())
    {
    }
};

template <class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API StaticMetaclass : public Metaclass
{
    struct META_API Descriptor : DescriptorInterface
    {
        using SuperClassContainer = std::array<const Metaclass*, sizeof...(SuperClasses)>;
        SuperClassContainer m_superClasses;

        explicit Descriptor() :
            m_superClasses({{SuperClasses::staticMetaclass...}})
        {
        }
        MetaObjectPtr create(std::string_view name) const override
        {
            if constexpr (std::is_abstract_v<DeclaredClass>)
            {
                return {};
            }
            else
            {
                return DeclaredClass::create(name);
            }
        }

        const Metaclass* getBaseClass(size_t index) const final
        {
            return m_superClasses.at(index);
        }
        size_t getBaseClassCount() const final
        {
            return m_superClasses.size();
        }

        bool hasSuperClass(const Metaclass& metaClass) const final
        {
            for (auto& meta : m_superClasses)
            {
                if (meta->isDerivedFrom(metaClass))
                {
                    return true;
                }
            }

            return false;
        }

        bool isAbstract() const override
        {
            return std::is_abstract_v<DeclaredClass>;
        }
        bool isMetaClassOf(const MetaObject& object) const override
        {
            auto address = dynamic_cast<const DeclaredClass*>(&object);
            return address != nullptr;
        }
    };

public:
    explicit StaticMetaclass(std::string_view name) :
        Metaclass(name, std::make_unique<Descriptor>())
    {
    }
};

}



/// Defines the metaclass of an associated class.
class META_API MetaClass
{
public:
    /// Creates an object of the class to which the meta class is connected.
    /// \param name The name of the meta object created.
    virtual MetaObjectPtr create(std::string_view name) const = 0;

    /// Cast the created meta object to the given class type.
    template <class ClassType>
    std::shared_ptr<ClassType> create(std::string_view name) const
    {
        return std::dynamic_pointer_cast<ClassType>(create(name));
    }

    /// Returns the meta class of the base class at index.
    /// \param index The base class index.
    /// \return The meta class of the base class at index.
    virtual const MetaClass* getBaseClass(size_t index) const = 0;

    /// Returns the number of base classes with meta data.
    /// \return The number of base classes with meta data.
    virtual size_t getBaseClassCount() const = 0;

    /// Returns whether the class to which the meta class is connected is abstract.
    /// \return If the class to which the meta class is connected is abstract, returns \e true,
    ///         otherwise \e false.
    virtual bool isAbstract() const = 0;

    /// Returns whether this meta class is the meta class of the meta object.
    /// \param The meta object to check.
    /// \return If this meta class is the meta class of the meta object, returns \e true, otherwise
    ///         \e false.
    virtual bool isMetaClassOf(const MetaObject& object) const = 0;

    /// Returns whether this MetaClass is or is derived from the \a metaClass.
    /// \param metaClass The metaClass instance to check.
    /// \return If this MetaClass is or is derived from the metaClass, returns \e true, otherwise
    ///         \e false.
    bool isDerivedFrom(const MetaClass& metaClass) const;

    /// Adds a callable to the meta class. The callable is either a method of the class, or an attached
    /// function, or a lambda. The method fails if the meta class already has a callable with the same
    /// name.
    /// \param callable The callable object to add.
    /// \return If the method is added with success, returns \e true, otherwise \e false. On failure,
    ///         and tracing is enabled, an error log is printed.
    /// \see Callable
    bool addMethod(Callable& callable);

    /// Tries to find a callable with the given name.
    /// \param name The name of the callable to find.
    /// \return The callable with the name, or \e nullptr if no callable with the name is found.
    /// \see Callable
    Callable* findMethod(std::string_view name);

protected:
    /// Constructor. Creates a metaclass with a base class and name.
    explicit MetaClass() = default;
    /// Destructor.
    virtual ~MetaClass();

    /// Checks whether a \a metaClass is a super metac class of this meta class.
    virtual bool hasSuperClass(const MetaClass& /*metaClass*/) const
    {
        return false;
    }

    /// The callable container type.
    using CallableContainer = std::unordered_map<std::string_view, Callable*>;

    /// The name of the meta class.
    std::string m_name;
    /// The callable container of the meta class.
    CallableContainer m_callables;
};

/// Meta class implementation with no super classes.
template <class DeclaredMetaClass, class DeclaredClass>
class META_TEMPLATE_API CoreMetaClassImpl : public MetaClass
{
public:
    CoreMetaClassImpl() {}
    const MetaClass* getBaseClass(size_t) const override
    {
        return {};
    }
    size_t getBaseClassCount() const override
    {
        return 0u;
    }
    bool isAbstract() const final
    {
        return std::is_abstract_v<DeclaredClass>;
    }
    bool isMetaClassOf(const MetaObject& object) const final
    {
        auto address = dynamic_cast<const DeclaredClass*>(&object);
        return address != nullptr;
    }

    template <class TDerivedClass>
    bool isDerivedFromClass() const
    {
        auto metaClass = TDerivedClass::getStaticMetaClass();
        return hasSuperClass(*metaClass);
    }
};

namespace detail
{

/// Meta class implementation for abstract classes, with base classes that have no metadata.
template <class DeclaredMetaClass, class DeclaredClass>
class META_TEMPLATE_API AbstractBaseMetaClass : public CoreMetaClassImpl<DeclaredMetaClass, DeclaredClass>
{
public:
    MetaObjectPtr create(std::string_view) const override
    {
        return {};
    }
};

/// Meta class implementation for non-abstract classes, with base classes that have no metadata.
template <class DeclaredMetaClass, class DeclaredClass>
class META_TEMPLATE_API BaseMetaClass : public CoreMetaClassImpl<DeclaredMetaClass, DeclaredClass>
{
public:
    MetaObjectPtr create(std::string_view name) const override
    {
        return DeclaredClass::create(name);
    }
};


/// Meta class implementation with one or more superclasses that have metadata.
template <class DeclaredMetaClass, class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API AbstractStaticMetaClass : public AbstractBaseMetaClass<DeclaredMetaClass, DeclaredClass>
{
    using SuperClassContainer = std::array<const MetaClass*, sizeof...(SuperClasses)>;
    SuperClassContainer m_superClasses;

public:
    AbstractStaticMetaClass() :
        m_superClasses({{SuperClasses::getStaticMetaClass()...}})
    {
    }

    const MetaClass* getBaseClass(size_t index) const final
    {
        return m_superClasses.at(index);
    }
    size_t getBaseClassCount() const final
    {
        return m_superClasses.size();
    }

protected:
    bool hasSuperClass(const MetaClass& metaClass) const final
    {
        for (auto& meta : m_superClasses)
        {
            if (meta->isDerivedFrom(metaClass))
            {
                return true;
            }
        }

        return false;
    }
};

template <class DeclaredMetaClass, class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API StaticMetaClass : public AbstractStaticMetaClass<DeclaredMetaClass, DeclaredClass, SuperClasses...>
{
public:
    MetaObjectPtr create(std::string_view name) const override
    {
        return DeclaredClass::create(name);
    }
};

} // namespace detail

} // namespace meta

/// Defines the metadata of an abstract base class. Use this macro when the class does not derive from
/// classes with metadata.
#define AbstractBaseMetaData(DeclaredClass)         \
struct StaticMetaClass;                             \
static const StaticMetaClass* getStaticMetaClass()  \
{                                                   \
    static StaticMetaClass metaClass;               \
    return &metaClass;                              \
}                                                   \
struct META_API StaticMetaClass : public meta::detail::AbstractBaseMetaClass<StaticMetaClass, DeclaredClass>

/// Defines the metadata of a base class. Use this macro when the class does not derive from classes
/// with metadata.
#define BaseMetaData(DeclaredClass)                 \
struct StaticMetaClass;                             \
static const StaticMetaClass* getStaticMetaClass()  \
{                                                   \
    static StaticMetaClass metaClass;               \
    return &metaClass;                              \
}                                                   \
struct META_API StaticMetaClass : public meta::detail::BaseMetaClass<StaticMetaClass, DeclaredClass>

/// Defines a metadata of an abstract class. The first argument must be the class for which you declare
/// the meta data. The rest of the arguments should refer to the base classes, preferrably in the
/// order of their declaration.
#define AbstractMetaData(...)                       \
struct StaticMetaClass;                             \
static const StaticMetaClass* getStaticMetaClass()  \
{                                                   \
    static StaticMetaClass metaClass;               \
    return &metaClass;                              \
}                                                   \
struct META_API StaticMetaClass : public meta::detail::AbstractStaticMetaClass<StaticMetaClass, __VA_ARGS__>

/// Defines a metadata of a class. The first argument must be the class for which you declare the meta
/// data. The rest of the arguments should refer to the base classes, preferrably in the order of
/// their declaration.
#define MetaData(...)                               \
struct StaticMetaClass;                             \
static const StaticMetaClass* getStaticMetaClass()  \
{                                                   \
    static StaticMetaClass metaClass;               \
    return &metaClass;                              \
}                                                   \
struct META_API StaticMetaClass : public meta::detail::StaticMetaClass<StaticMetaClass, __VA_ARGS__>


#endif // META_METACLASS_HPP
