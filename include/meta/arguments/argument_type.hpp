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
#include <memory>
#include <string>
#include <vector>

namespace meta
{

class META_API BadArgument : public std::exception
{
public:
    BadArgument(const std::type_info& actualType, const std::type_info& expectedType) noexcept;
    BadArgument(const BadArgument&) noexcept;
    ~BadArgument() noexcept = default;
    const char* what() const noexcept override;

private:
    std::unique_ptr<char, void(*)(void*)> message;
};

struct META_API ArgumentType
{
    ArgumentType(const std::type_info& tinfo) :
        type(tinfo)
    {
    }
    std::string getName() const;

private:
    const std::type_info& type;
};

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
    ArgumentData(T value) :
        std::any(value),
        m_isConst(std::is_const_v<T>)
    {
    }

    ArgumentType getType() const;

    /// Cast operator, returns the data stored by an ArgumentData instance.
    /// \tparam T The type of the casted value.
    /// \return The value stored.
    /// \throws Throws std::bad_any_cast if the type to cast to is not the type the data is stored.
    template <class T>
    operator T() const;

private:
    bool m_isConst = false;
};

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
    explicit PackagedArguments(Arguments&&... arguments);

    /// Creates packaged arguments from a subset of an other packaged arguments.
    explicit PackagedArguments(Iterator begin, Iterator end);

    /// Catenates two packaged arguments.
    /// \rhs The argument package to append.
    /// \return The argument package.
    PackagedArguments& operator+=(const PackagedArguments& rhs);

    /// Adds an argument to a packaged arguments.
    /// \rhs The argument to add.
    /// \return The argument package.
    PackagedArguments& operator+=(ArgumentData rhs);

    /// Appends \a package arguments to this.
    /// \param package The packaged arguiments to append to this package.
    /// \return Returns this packaged arguments object.
    PackagedArguments& append(const PackagedArguments& package);

    /// Returns the argument data at index.
    /// \param index The index of the argument.
    /// \return The value of the argument at index.
    ArgumentData get(std::size_t index) const;

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
    std::vector<ArgumentData> m_pack;
};

} // namespace meta

#include <meta/arguments/argument_type_impl.hpp>

#endif // META_ARGUMENT_TYPE_HPP
