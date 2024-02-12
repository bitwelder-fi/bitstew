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

/// Defines the metaclass of an associated class.
class META_API MetaClass
{
    DISABLE_COPY(MetaClass);
    DISABLE_MOVE(MetaClass);

public:
    /// Container with the meta classes of the object extensions added to a meta class.
    using MetaExtensionContainer = std::unordered_map<std::string_view, const MetaClass*>;
    /// The iterator of the meta extensions.
    using MetaExtensionIterator = MetaExtensionContainer::const_iterator;

    /// Registers an object extension metaclass to a static meta class.
    struct META_API MetaExtensionRegistrar
    {
        explicit MetaExtensionRegistrar(MetaClass& self, const MetaClass& extensionMeta, std::string_view name = std::string_view());
    };

    struct META_API EnableDynamic
    {
        template <class TMetaClass>
        explicit EnableDynamic(TMetaClass& self)
        {
            self.m_descriptor->getDynamic = []()
            {
                static TMetaClass dynamic;
                dynamic.m_descriptor->sealed = false;
                return &dynamic;
            };
        }
    };

    /// Destructor.
    virtual ~MetaClass() = default;

    /// Creates an object of the class to which the meta class is connected.
    /// \param name The name of the meta object created.
    template <class ClassType = MetaObject>
    std::shared_ptr<ClassType> create(std::string_view name) const
    {
        abortIfFail(m_descriptor);
        auto result = std::dynamic_pointer_cast<ClassType>(m_descriptor->create(name));
        if constexpr (std::is_base_of_v<Object, ClassType>)
        {
            initializeInstance(result);
        }
        return result;
    }

    /// Returns whether the meta class is sealed.
    /// \return If the meta class is sealed, returns \e true, otherwise \e false.
    bool isSealed() const;

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

    /// \name Meta extensions
    /// \{
    /// Meta extensions are meta classes of object extensions you can add to the meta class of an Object.
    /// When you create an instance of an object through a meta class, Meta automatically attaches
    /// the object extensions to the created instance.

    /// Adds the meta class of an object extension to this meta class. The method fails if either this
    /// or the meta class of the object extension is invalid, the meta class is not a meta class of
    /// an object extension, it is already registered, or the name specified is not a valid metaname,
    /// or there is an object extension meta class registered with the name.
    /// \param extensionMeta The meta class of the object extension to add.
    /// \param name Optional, the metaname under which to register the meta class. You must specify
    ///        a metaname for the stub meta classes.
    void addMetaExtension(const MetaClass& extensionMeta, std::string_view name = std::string_view());

    /// Tries to add the meta class of a registered object extension to this meta class. The meta class
    /// must be registered to Meta library under the metaname passed as argument.
    /// \param metaName The metaname of teh meta class to register as extension.
    /// \return If the meta class of the object extension was registered with success, returns \e true,
    ///         otherwise \e false.
    bool tryAddExtension(std::string_view metaName);

    /// Finds the object extension meta class registered under a given name.
    /// \param name The metaname of the object extension meta class to find.
    /// \return On success returns the meta class of the object extension, or \e nullptr on failure.
    const MetaClass* findMetaExtension(std::string_view name) const;

    /// Returns the begin iterator of the object extensions meta class register.
    inline MetaExtensionIterator beginExtensions() const
    {
        abortIfFail(m_descriptor);
        return m_descriptor->extensions.begin();
    }

    /// Returns the end iterator of the object extensions meta class register.
    inline MetaExtensionIterator endExtensions() const
    {
        abortIfFail(m_descriptor);
        return m_descriptor->extensions.end();
    }
    /// \}

    MetaClass* getDynamicMetaClass() const
    {
        return m_descriptor->getDynamic ? m_descriptor->getDynamic() : nullptr;
    }

protected:
    /// The descriptor of the metaclass.
    struct META_API DescriptorInterface
    {
        /// The container with the meta extensions.
        MetaExtensionContainer extensions;
        /// The name of the meta class.
        std::string name;
        /// The dynamic meta class callback.
        std::function<MetaClass*()> getDynamic;
        /// Whether the metaclass is sealed.
        bool sealed = false;

        explicit DescriptorInterface(std::string_view name) :
            name(name)
        {
        }
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
        virtual bool isExtension() const = 0;
        virtual bool isMetaClassOf(const MetaObject& object) const = 0;
    };
    using DescriptorPtr = std::unique_ptr<DescriptorInterface>;

    /// Constructor. Creates a metaclass with a descriptor.
    explicit MetaClass(DescriptorPtr descriptor) :
        m_descriptor(std::move(descriptor))
    {
    }

    /// The descriptor of the metaclass.
    DescriptorPtr m_descriptor;
    friend class ObjectFactory;

private:
    void initializeInstance(ObjectPtr instance) const;
};

} // namespace meta

#include <meta/metadata/metaclass_impl.hpp>


/// Defines the metadata of a class. The first argument is the name of the metaclass, followed by the
/// class for which you declare the meta data. The rest of the arguments should refer to the base
/// classes, preferrably in the order of their declaration.
#define META_CLASS(ClassName, ...)                                              \
struct MetaClassType;                                                           \
static const meta::MetaClass* getStaticMetaClass()                              \
{                                                                               \
    static MetaClassType metaClass;                                             \
    return &metaClass;                                                          \
}                                                                               \
static inline constexpr char __MetaName[]{ClassName};                           \
using MetaClassTypeBase = meta::detail::MetaClassImpl<__MetaName, __VA_ARGS__>; \
struct META_API MetaClassType : MetaClassTypeBase


#define STUB_META_CLASS(DeclaredClass, ...)                                         \
struct MetaClassType;                                                               \
static const meta::MetaClass* getStaticMetaClass()                                  \
{                                                                                   \
    static MetaClassType metaClass;                                                 \
    return &metaClass;                                                              \
}                                                                                   \
using MetaClassTypeBase = meta::detail::StubMetaClass<DeclaredClass, __VA_ARGS__>;  \
struct META_API MetaClassType : MetaClassTypeBase

/// Declares the meta class to have its dynamic copy.
#define DYNAMIC_META_CLASS() EnableDynamic _d{*this}

#define META_EXTENSION(Extension) \
MetaExtensionRegistrar __##Extension {*this, *(Extension::getStaticMetaClass())}

#define META_NAMED_EXTENSION(Extension, Name) \
MetaExtensionRegistrar __##Extension {*this, *(Extension::getStaticMetaClass()), Name}

#endif // META_METACLASS_HPP
