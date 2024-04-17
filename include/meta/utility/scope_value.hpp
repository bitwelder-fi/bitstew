/*
 * Copyright (C) 2017-2019 bitWelder
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

#ifndef UTILS_SCOPE_VALUE_HPP
#define UTILS_SCOPE_VALUE_HPP

namespace utils
{

/// The template flips a variable value to a temporary value for the lifetime of the class.
/// When the class is destroyed, it restores the value of the variable at the class creation
/// time.
template <typename T>
class ScopeValue
{
public:
    explicit ScopeValue(T& variable, T value)
        : m_variable(variable)
        , m_previousValue(m_variable)
    {
        m_variable = value;
    }
    ~ScopeValue()
    {
        m_variable = m_previousValue;
    }
private:
    T& m_variable;
    T m_previousValue;
};

} // namespace utils

#endif // UTILS_SCOPE_VALUE_HPP
