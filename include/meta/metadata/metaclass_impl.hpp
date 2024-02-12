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

#include <array>
#include <meta/metadata/metaclass.hpp>

namespace meta
{

namespace detail
{

template <class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API StubMetaClass : public MetaClass
{
protected:
    static constexpr auto arity = sizeof... (SuperClasses);

    struct META_API StubDescriptor : DescriptorInterface
    {
        explicit StubDescriptor(std::string_view name) :
            DescriptorInterface(name)
        {
            sealed = true;
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

        const MetaClass* getBaseClass(std::size_t index) const final
        {
            if constexpr (arity)
            {
                auto superMetas = std::array<const MetaClass*, arity>({{SuperClasses::getStaticMetaClass()...}});
                return superMetas[index];
            }
            else
            {
                return {};
            }
        }
        std::size_t getBaseClassCount() const final
        {
            if constexpr (arity)
            {
                return arity;
            }
            else
            {
                return 0u;
            }
        }
        bool hasSuperClass(const MetaClass& metaClass) const final
        {
            if constexpr (arity)
            {
                auto superMetas = std::array<const MetaClass*, arity>({{SuperClasses::getStaticMetaClass()...}});
                for (auto& meta : superMetas)
                {
                    if (meta->isDerivedFrom(metaClass))
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        bool isAbstract() const final
        {
            return std::is_abstract_v<DeclaredClass>;
        }

        bool isExtension() const final
        {
            return std::is_base_of_v<ObjectExtension, DeclaredClass>;
        }

        bool isMetaClassOf(const MetaObject& object) const final
        {
            auto address = dynamic_cast<const DeclaredClass*>(&object);
            return address != nullptr;
        }
    };

    explicit StubMetaClass(DescriptorPtr descriptor) :
        MetaClass(std::move(descriptor))
    {
    }

public:
    explicit StubMetaClass() :
        StubMetaClass(std::make_unique<StubDescriptor>(""))
    {        
    }
};

template <const char* MetaClassName, class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API MetaClassImpl : public StubMetaClass<DeclaredClass, SuperClasses...>
{
    using BaseMetaClass = StubMetaClass<DeclaredClass, SuperClasses...>;

protected:
    struct META_API MetaDescriptor : BaseMetaClass::StubDescriptor
    {
        explicit MetaDescriptor(std::string_view name) :
            BaseMetaClass::StubDescriptor(name)
        {
        }
    };

public:
    explicit MetaClassImpl() :
        BaseMetaClass(std::make_unique<MetaDescriptor>(MetaClassName))
    {
    }
};

} // detail

} // meta
