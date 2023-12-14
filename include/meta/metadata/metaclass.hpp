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
#include <meta/metadata/callable.hpp>
#include <assert.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace meta
{

class MetaObject;
using MetaObjectPtr = std::shared_ptr<MetaObject>;


/// Defines the metaclass of an associated class.
class META_API MetaClass
{
    DISABLE_COPY(MetaClass);

public:
    /// The metamethod of a metaclass.
    class META_API MetaMethod : public Callable
    {
    public:
        /// Constructor.
        template<class Function>
        explicit MetaMethod(MetaClass& metaClass, std::string_view name, Function function) :
            Callable(name, function)
        {
            metaClass.addMethod(*this);
        }
    };

    /// Destructor.
    virtual ~MetaClass() = default;

    /// Creates an object of the class to which the meta class is connected.
    /// \param name The name of the meta object created.
    MetaObjectPtr create(std::string_view name) const;

    /// Cast the created meta object to the given class type.
    template <class ClassType>
    std::shared_ptr<ClassType> create(std::string_view name) const
    {
        return std::dynamic_pointer_cast<ClassType>(create(name));
    }

    /// Returns the name of the metaclass.
    std::string_view getName() const;

    /// Returns the meta class of the base class at index.
    /// \param index The base class index.
    /// \return The meta class of the base class at index.
    const MetaClass* getBaseClass(std::size_t index) const;

    /// Returns the number of base classes with meta data.
    /// \return The number of base classes with meta data.
    std::size_t getBaseClassCount() const;

    /// Returns whether the class to which the meta class is connected is abstract.
    /// \return If the class to which the meta class is connected is abstract, returns \e true,
    ///         otherwise \e false.
    bool isAbstract() const;

    /// Returns whether this meta class is the meta class of the meta object.
    /// \param The meta object to check.
    /// \return If this meta class is the meta class of the meta object, returns \e true, otherwise
    ///         \e false.
    bool isMetaClassOf(const MetaObject& object) const;

    /// Returns whether this MetaClass is or is derived from the \a metaClass.
    /// \param metaClass The metaClass instance to check.
    /// \return If this MetaClass is or is derived from the metaClass, returns \e true, otherwise
    ///         \e false.
    bool isDerivedFrom(const MetaClass& metaClass) const;

    template <class TDerivedClass>
    bool isDerivedFromClass() const
    {
        abortIfFail(m_descriptor);
        return m_descriptor->hasSuperClass(*TDerivedClass::getStaticMetaClass());
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
    Callable* findMethod(std::string_view name) const;

protected:
    /// The descriptor of the metaclass.
    struct META_API DescriptorInterface
    {
        /// The callable container type.
        using CallableContainer = std::unordered_map<std::string, Callable*>;

        /// The name of the meta class.
        std::string name;
        /// The callable container of the meta class.
        CallableContainer callables;
        /// Whether the metaclass is sealed.
        bool sealed = false;

        virtual ~DescriptorInterface() = default;
        virtual MetaObjectPtr create(std::string_view /*name*/) const
        {
            return {};
        }
        virtual const MetaClass* getBaseClass(std::size_t /*index*/) const
        {
            return {};
        }
        virtual std::size_t getBaseClassCount() const
        {
            return 0u;
        }
        virtual bool hasSuperClass(const MetaClass& /*metaClass*/) const
        {
            return false;
        }
        virtual bool isAbstract() const = 0;
        virtual bool isMetaClassOf(const MetaObject& object) const = 0;
    };
    using DescriptorPtr = std::unique_ptr<DescriptorInterface>;

    /// Constructor. Creates a metaclass with a name and descriptor.
    explicit MetaClass(std::string_view name, DescriptorPtr descriptor) :
        m_descriptor(std::move(descriptor))
    {
        m_descriptor->name = name;
    }

    /// The descriptor of the metaclass.
    DescriptorPtr m_descriptor;
};


/// Tests the string passed as argument against metaname correctness. A metaname can contain numeric
/// and alphanumeric characters, dots, columns, dashes and underscores.
/// \param text The text to check.
/// \return If teh text is a valid metaname, returns \e true, otherwise \e false.
META_API bool testMetaName(std::string_view text);

} // namespace meta

#include <meta/metadata/metaclass_impl.hpp>


/// Defines the metadata of a class. The first argument is the name of the metaclass, followed by the
/// class for which you declare the meta data. The rest of the arguments should refer to the base
/// classes, preferrably in the order of their declaration.
#define META_CLASS(ClassName, ...)                                              \
struct MetaClassType;                                                           \
using MetaClassTypePtr = const MetaClassType*;                                  \
static const meta::MetaClass* getStaticMetaClass()                              \
{                                                                               \
    static MetaClassType metaClass;                                             \
    return &metaClass;                                                          \
}                                                                               \
static inline constexpr char __MetaName[]{ClassName};                           \
using MetaClassTypeBase = meta::detail::MetaClassImpl<__MetaName, __VA_ARGS__>; \
struct META_API MetaClassType : MetaClassTypeBase

/// Defines a metamethod to a method of a class for a metaclass.
#define META_METHOD(Class, Method) \
MetaMethod _##Method{*this, #Method, &Class::Method}

#endif // META_METACLASS_HPP
