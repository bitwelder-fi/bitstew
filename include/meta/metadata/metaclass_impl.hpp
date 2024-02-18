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

#include <array>
#include <utils/utility.hpp>
#include <meta/metadata/metaclass.hpp>

namespace meta
{

namespace detail
{

template <class MetaRegistrars, class DeclaredClass, class... SuperClasses>
class META_TEMPLATE_API MetaClassImpl : public MetaClass
{
    using SelfType = MetaClassImpl<MetaRegistrars, DeclaredClass, SuperClasses...>;

protected:
    static constexpr auto arity = sizeof... (SuperClasses);

    struct META_API MetaClassDescriptor : DescriptorInterface
    {
        explicit MetaClassDescriptor(std::string_view metaClassName) :
            DescriptorInterface(metaClassName)
        {
        }

        MetaObjectPtr create(std::string_view name) const override
        {
            if constexpr (!std::is_abstract_v<DeclaredClass> && std::is_base_of_v<MetaObject, DeclaredClass>)
            {
                return DeclaredClass::create(name);
            }
            else
            {
                return {};
            }
        }

        VisitResult visitSuper(Visitor visitor) const final
        {
            auto result = VisitResult::Continue;

            auto predicate = [&visitor, &result](const MetaClass* metaClass)
            {
                if (result == VisitResult::Abort)
                {
                    return;
                }
                result = metaClass->visit(visitor);
            };
            utils::for_each_arg(predicate, SuperClasses::getStaticMetaClass()...);

            return result;
        }

        bool isAbstract() const final
        {
            return std::is_abstract_v<DeclaredClass>;
        }

        bool isExtension() const final
        {
            return std::is_base_of_v<ObjectExtension, DeclaredClass>;
        }
    };

public:
    explicit MetaClassImpl(std::string_view metaClassName) :
        MetaClass(std::make_unique<MetaClassDescriptor>(metaClassName))
    {
        static_assert(std::is_base_of_v<Registrars, MetaRegistrars>, "MetaRegistrars template argument must derive from meta::Registrars.");
        MetaRegistrars registrars;
        registrars.apply(*this);
        this->m_descriptor->sealed = true;
    }
};


} // detail

} // meta
