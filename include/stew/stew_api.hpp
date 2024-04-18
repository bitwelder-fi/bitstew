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

#ifndef STEW_API_HPP
#define STEW_API_HPP

#include <stew/core/preprocessor.hpp>

#ifdef CONFIG_LIBRARY
#   define STEW_API             EXPORT_API
#   define STEW_TEMPLATE_API    TEMPLATE_EXPORT_API
#else
#   define STEW_API             IMPORT_API
#   define STEW_TEMPLATE_API    TEMPLATE_IMPORT_API
#endif

#endif // STEW_API_HPP
