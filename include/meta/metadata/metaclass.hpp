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

namespace meta
{

class MetaObject;
using MetaObjectPtr = std::shared_ptr<MetaObject>;

/// Defines the metaclass of an associated class.
class META_API MetaClass
{
public:

    virtual MetaObjectPtr create(std::string_view name) = 0;
    template <class ClassType>
    std::shared_ptr<ClassType> create(std::string_view name)
    {
        return std::dynamic_pointer_cast<ClassType>(create(name));
    }

    virtual const MetaClass* getBaseClass(size_t index) const = 0;
    virtual size_t getBaseClassCount() const = 0;
    virtual bool isAbstract() const = 0;
    virtual bool isMetaClassOf(const MetaObject&) const = 0;
    bool isDerivedFrom(const MetaClass& metaClass) const;

protected:
    /// Constructor. Creates a metaclass with a base class and name.
    explicit MetaClass() = default;
    /// Destructor.
    virtual ~MetaClass();

    virtual bool hasSuperClass(const MetaClass& metaClass) const = 0;

    std::string m_name;
};

namespace data
{

/// Meta class implementation.
template <class DeclaredMetaClass, class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API MetaClassImpl : public MetaClass
{
    using SuperClassContainer = std::array<const MetaClass*, sizeof...(SuperClasses)>;
    SuperClassContainer m_superClasses;

public:
    MetaClassImpl() :
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
class META_TEMPLATE_API StaticMetaClass : public MetaClassImpl<DeclaredMetaClass, DeclaredClass, SuperClasses...>
{
public:
    MetaObjectPtr create(std::string_view name) override
    {
        return DeclaredClass::create(name);
    }
};

/// Meta Class for abstract meta objects.
template <class DeclaredMetaClass, class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API AbstractStaticMetaClass : public MetaClassImpl<DeclaredMetaClass, DeclaredClass, SuperClasses...>
{
public:
    MetaObjectPtr create(std::string_view) override
    {
        return {};
    }
};

} // namespace data

} // namespace meta

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
struct META_API StaticMetaClass : public meta::data::StaticMetaClass<StaticMetaClass, __VA_ARGS__>


/// Defines a metadata of a class. The first argument must be the class for which you declare the meta
/// data. The rest of the arguments should refer to the base classes, preferrably in the order of
/// their declaration.
#define AbstractMetaData(...)                       \
struct StaticMetaClass;                             \
static const StaticMetaClass* getStaticMetaClass()  \
{                                                   \
    static StaticMetaClass metaClass;               \
    return &metaClass;                              \
}                                                   \
struct META_API StaticMetaClass : public meta::data::AbstractStaticMetaClass<StaticMetaClass, __VA_ARGS__>

#endif // META_METACLASS_HPP
