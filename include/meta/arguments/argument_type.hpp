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

#ifndef META_ARGUMENT_TYPE_HPP
#define META_ARGUMENT_TYPE_HPP

#include <meta/meta_api.hpp>
#include <utils/function_traits.hpp>

#include <any>
#include <vector>

namespace meta
{

/// ArgumentData stores an argument passed on slot invocation.
class META_API ArgumentData : public std::any
{
public:
    /// Creates a default argument data, with no data stored.
    ArgumentData() = default;

    /// Creates an argument data with a value.
    /// \tparam T The type of the value passed as argument.
    /// \param value The value to store.
    template <class T>
    ArgumentData(T value)
        : std::any(value)
    {
    }

    /// Cast operator, returns the data stored by an ArgumentData instance.
    /// \tparam T The type of the casted value.
    /// \return The value stored.
    /// \throws Throws std::bad_any_cast if the type to cast to is not the type the data is stored.
    template <class T>
    operator T() const;
};

template <typename T>
bool operator==(const T& lhs, const ArgumentData& rhs)
{
    return lhs == static_cast<const T&>(rhs);
}
template <typename T>
bool operator!=(const T& lhs, const ArgumentData& rhs)
{
    return lhs != static_cast<const T&>(rhs);
}

/// PackagedArguments packages arguments for meta method or meta signal invocation.
struct META_API PackagedArguments
{
    /// The argument container;
    using Container = std::vector<ArgumentData>;
    /// The iterator of the argument container.
    using Iterator = Container::const_iterator;

    /// Default constructor.
    explicit PackagedArguments() = default;
    /// Move constructor.
    PackagedArguments(PackagedArguments&& other);
    /// Move operator.
    PackagedArguments& operator=(PackagedArguments&& other);
    /// Swaps this packaged arguments with \a other.
    void swap(PackagedArguments& other);

    DISABLE_COPY(PackagedArguments);

    /// Creates an argument pack with the \a arguments.
    /// \tparam Arguments Variadic number of arguments to pack.
    /// \param arguments The variadic argument values to pack.
    template <typename... Arguments>
    PackagedArguments(Arguments&&... arguments);

    /// Creates packaged arguments from a subset of an other packaged arguments.
    PackagedArguments(Iterator begin, Iterator end);

    /// Catenates two packaged arguments.
    PackagedArguments& operator+=(const PackagedArguments& rhs);

    /// Returns the argument data at index.
    /// \param index The index of the argument.
    /// \return The value of the argument at index.
    ArgumentData get(size_t index) const;

    /// Returns the size of the pack.
    size_t getSize() const;

    /// Returns whether the pack is empty.
    bool isEmpty() const;

    /// Returns an iterator to the beginning of the pack.
    Iterator begin() const
    {
        return m_pack.begin();
    }
    /// Returns an iterator to the end of the pack.
    Iterator end() const
    {
        return m_pack.end();
    }

    /// Returns the value of an argument at a given \a index.
    /// \tparam T The argument type to get. The argument type must be identical to the type the
    ///           type of the stored value.
    /// \param index The index of the argument.
    /// \return The value of the argument at index.
    template <typename T>
    T get(size_t index) const;

    /// Converts the argument package into a tuple, using the signature of a function.
    /// \tparam FunctionSignature The function signature to use when converting the arguments.
    /// \return The tuple prepared with the arguments ready to invoke a callable.
    /// \throws std::bad_any_cast when the argument types of the signature do not match the type
    ///         of the argument value stored in the package.
    template <class FunctionSignature>
    auto toTuple(FunctionSignature) const;

    /// Converts the argument package into a tuple, using the signature of a method.
    /// \tparam MethodSignature The method signature to use when converting the arguments.
    /// \param instance The instance of the class with the method to add as first element of the tuple.
    /// \return The tuple prepared with the arguments ready to invoke a callable.
    /// \throws std::bad_any_cast when the argument types of the signature do not match the type
    ///         of the argument value stored in the package.
    template <class TClass, class TRet, class... TArgs>
    auto toTuple(TRet (TClass::*signature)(TArgs...)) const;

private:
    std::vector<ArgumentData> m_pack;
};


/// Invokes a \a function with the given \a arguments.
/// \tparam Function The function signature.
/// \param function The function to invoke.
/// \param arguments The packaged arguments with which to invoke the function.
/// \return The return value of the function. If the function is void type, returns an undefined value.
template <typename Function>
auto invoke(Function function, const PackagedArguments& arguments);

} // namespace meta

#include <meta/arguments/argument_type_impl.hpp>

#endif // META_ARGUMENT_TYPE_HPP
