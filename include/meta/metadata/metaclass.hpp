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

#ifndef META_METACLASS_HPP
#define META_METACLASS_HPP

#include <assert.hpp>
#include <meta/arguments/packaged_arguments.hpp>
#include <meta/forwards.hpp>
#include <meta/meta_api.hpp>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace meta
{

/// Defines the metaclass of an associated class. The meta-class provides type inspection mechanism
/// for your classes. You can use the meta information of a class to provide dynamic behavior through
/// scripting.
///
/// Use META_CLASS() macro in the public body of your class to declare the meta-class. The macro declares
/// methods for both static and dynamic meta-class. The name of the meta-class identifies the class
/// in interoperability between scripting and native code.
///
/// Use STATIC_META_CLASS() macro in classes which do not derive from MetaObject. These classes only
/// have static meta data.
///
/// Use AUTO_META_CLASS() macro declare meta classes whose meta name gets generated automatically at
/// compile time. The name of the meta-class is generated from the RTTI name of the type. Use these
/// macros when you don't need to know the meta-class name in your application.
///
/// Use META_EXTENSION() macro to add the meta-class of object extensions. Use the macro inside the
/// body of the meta-class declaration. Example:
/// \code
///
/// class RunMe : public meta::ObjectExtension
/// {
/// public:
///     META_CLASS("runMe", RunMe, meta::ObjectExtension)
///     {
///     };
///
///     static auto create(stdL::string_view)
///     {
///         return std::make_shared<RunMe>();
///     }
/// protected:
///     explicit RunMe() :
///         meta::ObjectExtension(getStaticMetaClass()->getname())
///     {
///     }
/// };
/// class MyObject : public meta::Object
/// {
/// public:
///     META_CLASS("MyObject", MyObject, meta::Object)
///     {
///         META_EXTENSION(RunMe);
///     };
/// };
/// \endcode
///
class META_API MetaClass
{
    DISABLE_COPY(MetaClass);
    DISABLE_MOVE(MetaClass);

public:
    /// Container with the meta classes of the object extensions added to a meta-class.
    using MetaExtensionContainer = std::unordered_map<std::string_view, const MetaClass*>;
    /// The iterator of the meta extensions.
    using MetaExtensionIterator = MetaExtensionContainer::const_iterator;

    /// Destructor.
    virtual ~MetaClass() = default;

    /// Creates an object of the class to which the meta-class is connected.
    /// \param name The name of the meta object created.
    MetaObjectPtr create(std::string_view name) const;

    template <typename T>
    std::shared_ptr<T> create(std::string_view name) const
    {
        return std::dynamic_pointer_cast<T>(MetaClass::create(name));
    }

    /// Returns whether the meta-class is sealed.
    /// \return If the meta-class is sealed, returns \e true, otherwise \e false.
    bool isSealed() const;

    /// Returns the name of the metaclass.
    std::string_view getName() const;

    /// Returns whether the class to which the meta-class is connected is abstract.
    /// \return If the class to which the meta-class is connected is abstract, returns \e true,
    ///         otherwise \e false.
    bool isAbstract() const;

    /// Returns whether this MetaClass is or is derived from the \a metaClass.
    /// \param metaClass The metaClass instance to check.
    /// \return If this MetaClass is or is derived from the metaClass, returns \e true, otherwise
    ///         \e false.
    bool isDerivedFrom(const MetaClass& metaClass) const;

    template <class TDerivedClass>
    bool isDerivedFromClass() const
    {
        return isDerivedFrom(*TDerivedClass::getStaticMetaClass());
    }

    /// \name Meta-class visiting.
    /// \{
    /// Meta-class visiting allows you to walk through the meta-class and its super meta-classes for
    /// various purposes.

    /// The visit result.
    enum class VisitResult
    {
        /// Abort the visiting.
        Abort,
        /// Continue visiting.
        Continue
    };
    /// The visitor function type.
    using Visitor = std::function<VisitResult(const MetaClass*)>;

    /// Visits the meta-class and its super meta-classes.
    /// \param visitor The visitor function.
    /// \return The visit result.
    VisitResult visit(Visitor visitor) const;

    /// Visits the super meta-classes of a meta-class.
    /// \param visitor The visitor function.
    /// \return The visit result.
    VisitResult visitSuper(Visitor visitor) const;
    /// \}

    /// \name Meta extensions
    /// \{
    /// Meta extensions are meta classes of object extensions you can add to the meta-class of an Object.
    /// When you create an instance of an object through a meta-class, Meta automatically attaches
    /// the object extensions to the created instance.

    /// Adds the meta-class of an object extension to this meta-class. The method fails if either this
    /// or the meta-class of the object extension is invalid, the meta-class is not a meta-class of
    /// an object extension, or there is an object extension meta-class registered with the same name.
    /// \param extensionMeta The meta-class of the object extension to add.
    void addMetaExtension(const MetaClass* extensionMeta);

    template <class ClassType>
    void addMetaExtension()
    {
        addMetaExtension(ClassType::getStaticMetaClass());
    }

    /// Tries to add the meta-class of a registered object extension to this meta-class. The meta-class
    /// must be registered to Meta library under the meta-name passed as argument.
    /// \param metaName The meta-name of teh meta-class to register as extension.
    /// \return If the meta-class of the object extension was registered with success, returns \e true,
    ///         otherwise \e false.
    bool tryAddExtension(std::string_view metaName);

    /// Tries to find the object extension meta-class registered under a given name. The look-up includes
    /// the super meta-classes too.
    /// \param name The meta-name of the object extension meta-class to find.
    /// \return On success returns the meta-class of the object extension, or \e nullptr on failure.
    const MetaClass* findMetaExtension(std::string_view name) const;

    /// Returns the begin iterator of the object extensions meta-class register.
    MetaExtensionIterator beginExtensions() const;

    /// Returns the end iterator of the object extensions meta-class register.
    MetaExtensionIterator endExtensions() const;
    /// \}

protected:
    /// The descriptor of the metaclass.
    struct META_API DescriptorInterface
    {
        /// The container with the meta extensions.
        MetaExtensionContainer extensions;
        /// The name of the meta-class.
        const std::string name;
        /// Whether the metaclass is sealed.
        bool sealed = false;

        explicit DescriptorInterface(std::string_view metaClassName) :
            name(metaClassName)
        {
        }
        virtual ~DescriptorInterface() = default;
        virtual MetaObjectPtr create(std::string_view /*name*/) const
        {
            return {};
        }
        virtual VisitResult visitSuper(Visitor visitor) const = 0;
        virtual bool isAbstract() const = 0;
        virtual bool isExtension() const = 0;
    };
    using DescriptorPtr = std::unique_ptr<DescriptorInterface>;

    /// Constructor. Creates a metaclass with a descriptor.
    explicit MetaClass(DescriptorPtr descriptor) :
        m_descriptor(std::move(descriptor))
    {
    }

    /// The descriptor of the metaclass.
    DescriptorPtr m_descriptor;

private:
    void initializeInstance(ObjectPtr instance) const;
};

/// The registrars of a meta-class. The registrars are used in metaclass definitions within the body
/// of the meta-class. These register additional metadata to the meta-class. Use the following macros
/// to define meta-class registrars:
/// -# META_EXTENSION() to add an object extension.
struct META_API Registrars
{
    /// Registrar for a meta object extension.
    struct META_API Extension
    {
        /// Adds the registrar for the meta extension.
        explicit Extension(Registrars& self, const MetaClass* extension);
    };

    /// Applies the registrars on the meta-class.
    void apply(MetaClass& metaClass);

private:
    /// The registrar function.
    using Registrar = std::function<void(meta::MetaClass&)>;
    std::vector<Registrar> m_registrars;
};

} // namespace meta

#include <meta/metadata/metaclass_impl.hpp>


/// Defines the static metadata of a class. The first argument is the name of the metaclass, followed
/// by the class for which you declare the meta data. The rest of the arguments should refer to the
/// base classes, preferrably in the order of the derivate definition. The metadata has no dynamic
/// meta-class. Use this macro to declare the static meta-class of both classes which derive from
/// MetaObject and for those which don't.
#define STATIC_META_CLASS(ClassName, ...)                                       \
struct StaticMetaClass;                                                         \
static const meta::MetaClass* getStaticMetaClass()                              \
{                                                                               \
static StaticMetaClass metaClass(ClassName);                                    \
    return &metaClass;                                                          \
}                                                                               \
struct _Traits_;                                                                \
using MetaClassTypeBase = meta::detail::MetaClassImpl<_Traits_, __VA_ARGS__>;   \
struct META_API StaticMetaClass : MetaClassTypeBase                             \
{                                                                               \
    StaticMetaClass(std::string_view name) : MetaClassTypeBase(name) {}         \
};                                                                              \
struct _Traits_ : meta::Registrars


/// Defines the metadata of a class. The first argument is the name of the metaclass, followed by the
/// class for which you declare the meta data. The rest of the arguments should refer to the base
/// classes, preferrably in the order of the derivate definition.
#define META_CLASS(ClassName, ...)                                              \
struct StaticMetaClass;                                                         \
static const meta::MetaClass* getStaticMetaClass()                              \
{                                                                               \
static StaticMetaClass metaClass(ClassName);                                    \
    return &metaClass;                                                          \
}                                                                               \
virtual const meta::MetaClass* getDynamicMetaClass() const override             \
{                                                                               \
    return getStaticMetaClass();                                                \
}                                                                               \
struct _Traits_;                                                                \
using MetaClassTypeBase = meta::detail::MetaClassImpl<_Traits_, __VA_ARGS__>;   \
struct META_API StaticMetaClass : MetaClassTypeBase                             \
{                                                                               \
    StaticMetaClass(std::string_view name) : MetaClassTypeBase(name) {}         \
};                                                                              \
struct _Traits_ : meta::Registrars


/// Defines the metadata of a class with automatic meta-name generated from the TypeHint type. The
/// second argument is the class for which you declare the meta data. The rest of the arguments should
/// refer to the base classes, preferrably in the order of the derivate definition.
#define AUTO_META_CLASS(TypeHint, ...)                                          \
struct StaticMetaClass;                                                         \
static const meta::MetaClass* getStaticMetaClass()                              \
{                                                                               \
    static auto metaName = ensureValidMetaName(Argument::Type(typeid(TypeHint)).getName());\
    static StaticMetaClass metaClass(metaName);                                 \
    return &metaClass;                                                          \
}                                                                               \
virtual const meta::MetaClass* getDynamicMetaClass() const override             \
{                                                                               \
    return getStaticMetaClass();                                                \
}                                                                               \
struct _Traits_;                                                                \
using MetaClassTypeBase = meta::detail::MetaClassImpl<_Traits_, __VA_ARGS__>;   \
struct META_API StaticMetaClass : MetaClassTypeBase                             \
{                                                                               \
    StaticMetaClass(std::string_view name) : MetaClassTypeBase(name) {}         \
};                                                                              \
struct _Traits_ : meta::Registrars

#define META_EXTENSION(ExtensionType) \
meta::Registrars::Extension __##ExtensionType {*this, ExtensionType::getStaticMetaClass()}

#endif // META_METACLASS_HPP
