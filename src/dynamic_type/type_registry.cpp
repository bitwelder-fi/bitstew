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

#include <stew/core/assert.hpp>
#include <stew/dynamic_type/bad_variable_exception.hpp>
#include <stew/dynamic_type/type_converter.hpp>
#include <stew/dynamic_type/type_registry.hpp>

#include <stew/standalone/utility/concepts.hpp>

#include <any>

namespace stew
{

// Default type converters.
namespace
{

template <typename T>
concept std_byte = std::is_same_v<T, std::byte>;

template <typename T>
concept stoi_number = std::is_same_v<T, char> && std::is_same_v<T, unsigned char> &&
                      std::is_same_v<T, short> && std::is_same_v<T, unsigned short> &&
                      std::is_same_v<T, int> && std::is_same_v<T, unsigned int>;

template <typename T>
concept stol_number = std::is_same_v<T, long>;

template <typename T>
concept stoll_number = std::is_same_v<T, long long>;

template <typename T>
concept stoul_number = std::is_same_v<T, unsigned long>;

template <typename T>
concept stoull_number = std::is_same_v<T, unsigned long long>;


template <typename From, typename To>
    requires std::convertible_to<From, To>
struct ExplicitConvertible : TypeConverter::VTable
{
    static TypeInfo _target()
    {
        return TypeInfo(typeid(To));
    }
    static std::any _convert(std::any value)
    {
        auto source = std::any_cast<From>(value);
        return static_cast<To>(source);
    }

    ExplicitConvertible()
    {
        this->target = &_target;
        this->convert = &_convert;
    }
};

template <typename From, typename To>
void registerAtomicConverter(TypeRegistry& self)
{
    self.registerTypeConverter(typeid(From), ExplicitConvertible<From, To>());
    self.registerTypeConverter(typeid(To), ExplicitConvertible<To, From>());
}


template <typename From>
struct ToByte : TypeConverter::VTable
{
    static TypeInfo _target()
    {
        return TypeInfo(typeid(std::byte));
    }
    static std::any _convert(std::any value)
    {
        using ByteType = std::underlying_type_t<std::byte>;

        auto source = std::any_cast<From>(value);
        return static_cast<std::byte>(static_cast<ByteType>(source));
    }

    ToByte()
    {
        this->target = &_target;
        this->convert = &_convert;
    }
};

template <typename To>
struct FromByte : TypeConverter::VTable
{
    static TypeInfo _target()
    {
        return TypeInfo(typeid(To));
    }
    static std::any _convert(std::any value)
    {
        using ByteType = std::underlying_type_t<std::byte>;

        auto source = static_cast<ByteType>(std::any_cast<std::byte>(value));
        return static_cast<To>(source);
    }

    FromByte()
    {
        this->target = &_target;
        this->convert = &_convert;
    }
};

template <typename T>
void registerByteConverter(TypeRegistry& self)
{
    self.registerTypeConverter(typeid(T), ToByte<T>());
    self.registerTypeConverter(typeid(std::byte), FromByte<T>());
}


template <typename From>
struct ToString : TypeConverter::VTable
{
    static TypeInfo _target()
    {
        return TypeInfo(typeid(std::string));
    }
    static std::any _convert(std::any value)
    {
        if constexpr (std_byte<From>)
        {
            using ByteType = std::underlying_type_t<std::byte>;

            auto source = static_cast<ByteType>(std::any_cast<std::byte>(value));
            return std::to_string(source);
        }
        else
        {
            auto source = std::any_cast<From>(value);
            return std::to_string(source);
        }
    }

    ToString()
    {
        this->target = &_target;
        this->convert = &_convert;
    }
};

template <typename To, typename From>
    requires std_string<From> || std_string_view<From>
struct FromString : TypeConverter::VTable
{
    static TypeInfo _target()
    {
        return TypeInfo(typeid(To));
    }
    static std::any _convert(std::any value)
    {
        auto source = std::any_cast<From>(value);

        if constexpr (std_byte<To>)
        {
            using ByteType = std::underlying_type_t<std::byte>;

            auto ivalue = std::stoi(std::string(source));
            return static_cast<std::byte>(static_cast<ByteType>(ivalue));
        }
        else if constexpr (stoi_number<To>)
        {
            return std::stoi(std::string(source));
        }
        else if constexpr (stol_number<To>)
        {
            return std::stol(std::string(source));
        }
        else if constexpr (stoll_number<To>)
        {
            return std::stoll(std::string(source));
        }
        else if constexpr (stol_number<To>)
        {
            return std::stoul(std::string(source));
        }
        else if constexpr (stol_number<To>)
        {
            return std::stoull(std::string(source));
        }

        throw BadConverterException(typeid(From), typeid(To));
    }

    FromString()
    {
        this->target = &_target;
        this->convert = &_convert;
    }
};

template <typename T>
void registerStringConverter(TypeRegistry& self)
{
    self.registerTypeConverter(typeid(T), ToString<T>());
    self.registerTypeConverter(typeid(std::string), FromString<T, std::string>());
    self.registerTypeConverter(typeid(std::string_view), FromString<T, std::string_view>());
}

}


TypeRegistry& TypeRegistry::instance()
{
    static TypeRegistry registry;

    if (!registry.m_isInitialized)
    {
        registry.initialize();
    }

    return registry;
}

void TypeRegistry::uninitialize()
{
    m_registry.clear();
    m_isInitialized = false;
}

void TypeRegistry::initialize()
{
    registerType(typeid(bool));
    registerType(typeid(char));
    registerType(typeid(unsigned char));
    registerType(typeid(std::byte));
    registerType(typeid(short));
    registerType(typeid(unsigned short));
    registerType(typeid(int));
    registerType(typeid(unsigned int));
    registerType(typeid(long));
    registerType(typeid(unsigned long));
    registerType(typeid(long long));
    registerType(typeid(unsigned long long));
    registerType(typeid(float));
    registerType(typeid(double));
    registerType(typeid(std::string));
    registerType(typeid(std::string_view));

    // bool
    registerAtomicConverter<bool, char>(*this);
    registerAtomicConverter<bool, unsigned char>(*this);
    registerAtomicConverter<bool, short>(*this);
    registerAtomicConverter<bool, unsigned short>(*this);
    registerAtomicConverter<bool, int>(*this);
    registerAtomicConverter<bool, unsigned int>(*this);
    registerAtomicConverter<bool, long>(*this);
    registerAtomicConverter<bool, unsigned long>(*this);
    registerAtomicConverter<bool, long long>(*this);
    registerAtomicConverter<bool, unsigned long long>(*this);
    registerAtomicConverter<bool, float>(*this);
    registerAtomicConverter<bool, double>(*this);
    // char
    registerAtomicConverter<char, unsigned char>(*this);
    registerAtomicConverter<char, short>(*this);
    registerAtomicConverter<char, unsigned short>(*this);
    registerAtomicConverter<char, int>(*this);
    registerAtomicConverter<char, unsigned int>(*this);
    registerAtomicConverter<char, long>(*this);
    registerAtomicConverter<char, unsigned long>(*this);
    registerAtomicConverter<char, long long>(*this);
    registerAtomicConverter<char, unsigned long long>(*this);
    registerAtomicConverter<char, float>(*this);
    registerAtomicConverter<char, double>(*this);
    // uchar
    registerAtomicConverter<unsigned char, short>(*this);
    registerAtomicConverter<unsigned char, unsigned short>(*this);
    registerAtomicConverter<unsigned char, int>(*this);
    registerAtomicConverter<unsigned char, unsigned int>(*this);
    registerAtomicConverter<unsigned char, long>(*this);
    registerAtomicConverter<unsigned char, unsigned long>(*this);
    registerAtomicConverter<unsigned char, long long>(*this);
    registerAtomicConverter<unsigned char, unsigned long long>(*this);
    registerAtomicConverter<unsigned char, float>(*this);
    registerAtomicConverter<unsigned char, double>(*this);
    // short
    registerAtomicConverter<short, unsigned short>(*this);
    registerAtomicConverter<short, int>(*this);
    registerAtomicConverter<short, unsigned int>(*this);
    registerAtomicConverter<short, long>(*this);
    registerAtomicConverter<short, unsigned long>(*this);
    registerAtomicConverter<short, long long>(*this);
    registerAtomicConverter<short, unsigned long long>(*this);
    registerAtomicConverter<short, float>(*this);
    registerAtomicConverter<short, double>(*this);
    // ushort
    registerAtomicConverter<unsigned short, int>(*this);
    registerAtomicConverter<unsigned short, unsigned int>(*this);
    registerAtomicConverter<unsigned short, long>(*this);
    registerAtomicConverter<unsigned short, unsigned long>(*this);
    registerAtomicConverter<unsigned short, long long>(*this);
    registerAtomicConverter<unsigned short, unsigned long long>(*this);
    registerAtomicConverter<unsigned short, float>(*this);
    registerAtomicConverter<unsigned short, double>(*this);
    // int
    registerAtomicConverter<int, unsigned int>(*this);
    registerAtomicConverter<int, long>(*this);
    registerAtomicConverter<int, unsigned long>(*this);
    registerAtomicConverter<int, long long>(*this);
    registerAtomicConverter<int, unsigned long long>(*this);
    registerAtomicConverter<int, float>(*this);
    registerAtomicConverter<int, double>(*this);
    // uint
    registerAtomicConverter<unsigned int, long>(*this);
    registerAtomicConverter<unsigned int, unsigned long>(*this);
    registerAtomicConverter<unsigned int, long long>(*this);
    registerAtomicConverter<unsigned int, unsigned long long>(*this);
    registerAtomicConverter<unsigned int, float>(*this);
    registerAtomicConverter<unsigned int, double>(*this);
    // long
    registerAtomicConverter<long, unsigned long>(*this);
    registerAtomicConverter<long, long long>(*this);
    registerAtomicConverter<long, unsigned long long>(*this);
    registerAtomicConverter<long, float>(*this);
    registerAtomicConverter<long, double>(*this);
    // ulong
    registerAtomicConverter<unsigned long, long long>(*this);
    registerAtomicConverter<unsigned long, unsigned long long>(*this);
    registerAtomicConverter<unsigned long, float>(*this);
    registerAtomicConverter<unsigned long, double>(*this);
    // llong
    registerAtomicConverter<long long, unsigned long long>(*this);
    registerAtomicConverter<long long, float>(*this);
    registerAtomicConverter<long long, double>(*this);
    // ullong
    registerAtomicConverter<unsigned long long, float>(*this);
    registerAtomicConverter<unsigned long long, double>(*this);
    // float
    registerAtomicConverter<float, double>(*this);

    // std::byte
    registerByteConverter<bool>(*this);
    registerByteConverter<char>(*this);
    registerByteConverter<unsigned char>(*this);
    registerByteConverter<short>(*this);
    registerByteConverter<unsigned short>(*this);
    registerByteConverter<int>(*this);
    registerByteConverter<unsigned int>(*this);
    registerByteConverter<long>(*this);
    registerByteConverter<unsigned long>(*this);
    registerByteConverter<long long>(*this);
    registerByteConverter<unsigned long long>(*this);
    registerByteConverter<float>(*this);
    registerByteConverter<double>(*this);

    // std::string
    registerStringConverter<bool>(*this);
    registerStringConverter<char>(*this);
    registerStringConverter<unsigned char>(*this);
    registerStringConverter<short>(*this);
    registerStringConverter<unsigned short>(*this);
    registerStringConverter<int>(*this);
    registerStringConverter<unsigned int>(*this);
    registerStringConverter<long>(*this);
    registerStringConverter<unsigned long>(*this);
    registerStringConverter<long long>(*this);
    registerStringConverter<unsigned long long>(*this);
    registerStringConverter<float>(*this);
    registerStringConverter<double>(*this);
    registerStringConverter<std::byte>(*this);

    // String to number
    m_isInitialized = true;
}

void TypeRegistry::registerType(const TypeInfo& type)
{
    const auto tindex = std::type_index(type);
    auto it = m_registry.find(tindex);
    abortIfFailWithMessage(it == m_registry.end(), "Type " << type.getName() << " already registered.");

    m_registry.insert(std::make_pair(tindex, ConverterMap()));
}

void TypeRegistry::registerTypeConverter(const TypeInfo& type, TypeConverter converter)
{
    const auto sourceIndex = std::type_index(type);
    auto it = m_registry.find(sourceIndex);
    abortIfFailWithMessage(it != m_registry.end(), "Type " << type.getName() << " is not registered.");

    const auto target = converter.target();
    const auto targetIndex = std::type_index(target);
    auto cit = it->second.find(targetIndex);
    abortIfFailWithMessage(cit == it->second.end(), "Converter " << type.getName() << " -> " << target.getName() << " already registered.");
    it->second.insert(std::make_pair(targetIndex, std::move(converter)));
}

TypeConverter TypeRegistry::findConverter(const TypeInfo& source, const TypeInfo& target)
{
    const auto sourceIndex = std::type_index(source);
    if (auto it = m_registry.find(sourceIndex); it != m_registry.end())
    {
        const auto targetIndex = std::type_index(target);
        if (auto cit = it->second.find(targetIndex); cit != it->second.end())
        {
            return cit->second;
        }
    }

    return {};
}

}
