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

#ifndef META_FORWARDS_HPP
#define META_FORWARDS_HPP

#include <memory>

namespace meta
{

class Invokable;
class Object;
class ObjectExtension;

using InvokablePtr = std::shared_ptr<Invokable>;
using ObjectPtr = std::shared_ptr<Object>;
using ObjectWeakPtr = std::weak_ptr<Object>;
using ObjectExtensionPtr = std::shared_ptr<ObjectExtension>;
using ObjectExtensionWeakPtr = std::shared_ptr<ObjectExtension>;

}

#endif // META_FORWARDS_HPP
