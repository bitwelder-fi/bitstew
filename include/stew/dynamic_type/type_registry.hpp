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

#ifndef STEW_TYPE_REGISTER_HPP
#define STEW_TYPE_REGISTER_HPP

#include <stew/stew_api.hpp>
#include <stew/dynamic_type/type_info.hpp>
#include <stew/dynamic_type/type_converter.hpp>

#include <typeindex>
#include <unordered_map>

namespace stew
{

class STEW_API TypeRegistry
{
    explicit TypeRegistry() = default;
    ~TypeRegistry() = default;

public:
    static TypeRegistry& instance();
    void uninitialize();

    void registerType(const TypeInfo& type);
    void registerTypeConverter(const TypeInfo& type, TypeConverter converter);
    TypeConverter findConverter(const TypeInfo& source, const TypeInfo& target);

private:
    using ConverterMap = std::unordered_map<std::type_index, TypeConverter>;
    using TypeMap = std::unordered_map<std::type_index, ConverterMap>;

    void initialize();

    TypeMap m_registry;
    bool m_isInitialized = false;
};

}

#endif // STEW_CONVERTER_HPP
