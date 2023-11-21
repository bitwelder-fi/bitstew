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

#ifndef META_API_HPP
#define META_API_HPP

#include <preprocessor.hpp>

#ifdef CONFIG_LIBRARY
#   define META_API             EXPORT_API
#   define META_TEMPLATE_API    TEMPLATE_EXPORT_API
#else
#   define META_API             IMPORT_API
#   define META_TEMPLATE_API    TEMPLATE_IMPORT_API
#endif

#endif // META_API_HPP
