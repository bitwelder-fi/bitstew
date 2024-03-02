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

#include <memory>
#include <vector>

#include <meta/detail/packaged_arguments.hpp>

namespace meta
{

/// Stub call context.
struct META_API CallContext {};
using CallContextPtr = std::shared_ptr<CallContext>;

/// PackagedArguments packages arguments for meta method or meta signal invocation.
struct META_API PackagedArguments
{
    /// The argument container;
    using Container = std::vector<Argument>;
    /// The iterator of the argument container.
    using Iterator = Container::const_iterator;

    /// Default constructor.
    explicit PackagedArguments();

    /// Creates an argument pack with the \a arguments.
    /// \tparam Arguments Variadic number of arguments to pack.
    /// \param arguments The variadic argument values to pack.
    template <typename... Arguments>
    explicit PackagedArguments(Arguments&&... arguments);

    /// Move constructor.
    PackagedArguments(PackagedArguments&& other);
    /// Move operator.
    PackagedArguments& operator=(PackagedArguments&& other);
    /// Swaps this packaged arguments with \a other.
    void swap(PackagedArguments& other);

    /// Copy constructor. Copies of a pack share the same content. a Deep copy is executed when the
    /// copies change the content.
    PackagedArguments(const PackagedArguments& other);
    /// Copy operator.
    PackagedArguments& operator=(const PackagedArguments& other);

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
        return addBack(std::forward<T>(rhs));
    }

    /// Appends \a package arguments to this.
    /// \param package The packaged arguments to append to this package.
    /// \return Returns this packaged arguments object.
    PackagedArguments& cat(const PackagedArguments& package);

    /// Prepends \a package arguments to this.
    /// \param package The packaged arguments to prepend to this package.
    /// \return Returns this packaged arguments object.
    PackagedArguments& prepend(const PackagedArguments& package);

    /// Adds an argument to the end of the packaged arguments.
    /// \param argument The argument to add.
    /// \return Returns this packaged arguments object.
    PackagedArguments& addBack(Argument value);

    /// Adds an argument to the front of the packaged arguments.
    /// \param argument The argument to prepend.
    /// \return Returns this packaged arguments object.
    PackagedArguments& addFront(Argument value);

    /// Returns the argument data at index.
    /// \param index The index of the argument.
    /// \return The value of the argument at index.
    Argument get(std::size_t index) const;

    /// Returns the size of the pack.
    std::size_t getSize() const;

    /// Returns whether the pack is empty.
    bool isEmpty() const;

    CallContextPtr getContext() const
    {
        return m_descriptor->callContext;
    }

    /// Returns an iterator to the beginning of the pack.
    Iterator begin() const
    {
        return m_descriptor->pack.begin();
    }
    /// Returns an iterator to the end of the pack.
    Iterator end() const
    {
        return m_descriptor->pack.end();
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
    void deepCopyIfRequired();
    struct META_API Descriptor
    {
        Container pack;
        CallContextPtr callContext;

        template <typename... Arguments>
        explicit Descriptor(Arguments&&... args) :
            pack{std::forward<Arguments>(args)...}
        {
        }

        explicit Descriptor() = default;
        std::shared_ptr<Descriptor> clone();
    };
    std::shared_ptr<Descriptor> m_descriptor;

    friend bool operator==(const PackagedArguments& lhs, const PackagedArguments& rhs);
    friend bool operator!=(const PackagedArguments& lhs, const PackagedArguments& rhs);
    };

META_API bool operator==(const PackagedArguments& lhs, const PackagedArguments& rhs);
META_API bool operator!=(const PackagedArguments& lhs, const PackagedArguments& rhs);

template <typename... Arguments>
PackagedArguments::PackagedArguments(Arguments&&... arguments) :
    m_descriptor(std::make_shared<Descriptor>(std::forward<Arguments>(arguments)...))
{
}

template <typename T>
T PackagedArguments::get(std::size_t index) const
{
    return m_descriptor->pack.at(index);
}


template <class FunctionSignature>
auto PackagedArguments::toTuple() const
{
    constexpr auto N = traits::function_traits<FunctionSignature>::signature::arity;
    return detail::PackToTuple<FunctionSignature, PackagedArguments>::template convert<N, N>(*this);
}

} // namespace meta

#endif // META_PACKAGED_ARGUMENTS_HPP
