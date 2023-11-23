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

#ifndef META_HPP
#define META_HPP

#include <pimpl.hpp>
#include <preprocessor.hpp>
#include <meta/meta_api.hpp>

namespace meta
{

namespace thread_pool
{
class ThreadPool;
}

struct LibraryArguments;
class MetaLibraryPrivate;

/// The domain holds the meta library elements.
class META_API Domain
{
public:
    /// Returns the instance of the meta library domain.
    static Domain& instance();

    /// Initializes the meta library domain.
    /// \param arguments The library comain initialization arguments.
    void initialize(const LibraryArguments& arguments);

    /// Uninitialize the meta library domain.
    void uninitialize();

    /// Returns the thread pool of the library.
    /// \return The thread pool of the library, or nullptr, if the library is iniotialized without
    ///         a thread pool.
    thread_pool::ThreadPool* threadPool() const;

private:
    explicit Domain();
    ~Domain();

    DECLARE_PRIVATE_PTR(MetaLibraryPrivate)
};

} // namespace meta

#endif // META_HPP
