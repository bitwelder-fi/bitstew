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

#ifndef STEW_TYPE_OPERATORS_HPP
#define STEW_TYPE_OPERATORS_HPP

#include <stew/stew_api.hpp>
#include <stew/dynamic_type/type_info.hpp>

#include <any>

namespace stew
{

/// %TypeOperators defines the operators on a type.
///
/// Not all operations are available for each type. Operations which are not available throw an
/// UndefinedOperator exception.
class STEW_API TypeOperators
{
public:
    struct STEW_API VTable
    {
        /// Arythmentic operations.
        std::any (*add)(const std::any& lhs, const std::any& rhs) = nullptr;
        std::any (*sub)(const std::any& lhs, const std::any& rhs) = nullptr;
        std::any (*mul)(const std::any& lhs, const std::any& rhs) = nullptr;
        std::any (*div)(const std::any& lhs, const std::any& rhs) = nullptr;

        /// Logical operations.
        bool (*land)(const std::any& lhs, const std::any& rhs) = nullptr;
        bool (*lor)(const std::any& lhs, const std::any& rhs) = nullptr;
        std::any (*lnot)(const std::any& rhs) = nullptr;
        bool (*eq)(const std::any& lhs, const std::any& rhs) = nullptr;
        bool (*less)(const std::any& lhs, const std::any& rhs) = nullptr;
        bool (*leq)(const std::any& lhs, const std::any& rhs) = nullptr;
        bool (*gt)(const std::any& lhs, const std::any& rhs) = nullptr;
        bool (*geq)(const std::any& lhs, const std::any& rhs) = nullptr;

        /// Bitwise operations.
        std::any (*bw_and)(const std::any& lhs, const std::any& rhs) = nullptr;
        std::any (*bw_or)(const std::any& lhs, const std::any& rhs) = nullptr;
        std::any (*bw_xor)(const std::any& lhs, const std::any& rhs) = nullptr;
        std::any (*bw_not)(const std::any& rhs) = nullptr;
        std::any (*bw_shl)(const std::any& value, std::size_t count) = nullptr;
        std::any (*bw_shr)(const std::any& value, std::size_t count) = nullptr;

        /// Pointer operations.
        void* (*ptr)(const std::any&) = nullptr;
        const void* (*cptr)(const std::any&) = nullptr;
    };

    TypeOperators() = default;

    TypeOperators(VTable vTable);

    ~TypeOperators() = default;

    std::any add(const std::any& lhs, const std::any& rhs);
    std::any sub(const std::any& lhs, const std::any& rhs);
    std::any mul(const std::any& lhs, const std::any& rhs);
    std::any div(const std::any& lhs, const std::any& rhs);

    /// Logical operations.
    bool land(const std::any& lhs, const std::any& rhs);
    bool lor(const std::any& lhs, const std::any& rhs);
    std::any lnot(const std::any& rhs);
    bool eq(const std::any& lhs, const std::any& rhs);
    bool less(const std::any& lhs, const std::any& rhs);
    bool leq(const std::any& lhs, const std::any& rhs);
    bool gt(const std::any& lhs, const std::any& rhs);
    bool geq(const std::any& lhs, const std::any& rhs);

    /// Bitwise operations.
    std::any bw_and(const std::any& lhs, const std::any& rhs);
    std::any bw_or(const std::any& lhs, const std::any& rhs);
    std::any bw_xor(const std::any& lhs, const std::any& rhs);
    std::any bw_not(const std::any& rhs);
    std::any bw_shl(const std::any& value, std::size_t count);
    std::any bw_shr(const std::any& value, std::size_t count);

    /// Pointer operations.
    void* ptr(const std::any&);
    const void* cptr(const std::any&);

private:
    VTable v_table;
};

}

#endif // STEW_CONVERTER_HPP
