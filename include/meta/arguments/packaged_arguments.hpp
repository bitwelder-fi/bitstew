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

#ifndef META_PACKAGED_ARGUMENTS_HPP
#define META_PACKAGED_ARGUMENTS_HPP

#include <meta/meta_api.hpp>
#include <meta/arguments/argument.hpp>
#include <utils/function_traits.hpp>

#include <array>
#include <vector>

#include <meta/detail/packaged_arguments.hpp>

namespace meta
{

/// PackagedArguments packages arguments for meta method or meta signal invocation.
struct META_API PackagedArguments
{
    /// The argument container;
    using Container = std::vector<Argument>;
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

    template <typename... Arguments>
    PackagedArguments(std::initializer_list<Arguments...> ilist)
    {
        std::array<Argument, sizeof... (Arguments)> aa = {ilist};
        m_pack.reserve(aa.size());
        m_pack.insert(m_pack.end(), aa.begin(), aa.end());
    }

    /// Creates packaged arguments from a subset of an other packaged arguments.
    explicit PackagedArguments(Iterator begin, Iterator end);

    /// Catenates two packaged arguments.
    /// \rhs The argument package to append.
    /// \return The argument package.
    PackagedArguments& operator+=(const PackagedArguments& rhs);

    /// Adds an argument to a packaged arguments.
    /// \rhs The argument to add.
    /// \return The argument package.
    PackagedArguments& operator+=(Argument rhs);

    /// Adds an argument to a packaged arguments.
    /// \tparam T The argument type to add.
    /// \rhs The argument to add.
    /// \return The argument package.
    template <typename T>
    PackagedArguments& operator+=(T&& rhs)
    {
        *this += Argument(std::forward<T>(rhs));
        return *this;
    }

    /// Appends \a package arguments to this.
    /// \param package The packaged arguiments to append to this package.
    /// \return Returns this packaged arguments object.
    PackagedArguments& append(const PackagedArguments& package);

    /// Returns the argument data at index.
    /// \param index The index of the argument.
    /// \return The value of the argument at index.
    Argument get(std::size_t index) const;

    /// Returns the size of the pack.
    std::size_t getSize() const;

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
    /// \throws Throws std::bad_any_cast if the type to cast to is not the type the data is stored.
    template <typename T>
    T get(std::size_t index) const;

    /// Converts the argument package into a tuple, using the signature of a function. If the function
    /// is a member function, the first packaged argument must be the object to which the function
    /// member belongs.
    /// \tparam FunctionSignature The function signature to use when converting the arguments.
    /// \return The tuple prepared with the arguments ready to invoke a callable.
    /// \throws std::bad_any_cast when the argument types of the signature and the packaged arguments
    ///         mismatch.
    template <class FunctionSignature>
    auto toTuple() const;

private:
    Container m_pack;
};

template <typename... Arguments>
PackagedArguments::PackagedArguments(Arguments&&... arguments)
{
    std::array<Argument, sizeof... (Arguments)> aa = {{Argument(arguments)...}};
    m_pack.reserve(aa.size());
    m_pack.insert(m_pack.end(), aa.begin(), aa.end());
}

template <typename T>
T PackagedArguments::get(std::size_t index) const
{
    return m_pack.at(index);
}


template <class FunctionSignature>
auto PackagedArguments::toTuple() const
{
    constexpr auto N = traits::function_traits<FunctionSignature>::signature::arity;
    return detail::PackToTuple<FunctionSignature, PackagedArguments>::template convert<N, N>(*this);
}

} // namespace meta

#endif // META_PACKAGED_ARGUMENTS_HPP
