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

struct Connection;
class MetaObject;
class Object;
class ExecutableExtension;
class SignalExtension;

using ConnectionPtr = std::shared_ptr<Connection>;
using MetaObjectPtr = std::shared_ptr<MetaObject>;
using ObjectPtr = std::shared_ptr<Object>;
using ObjectWeakPtr = std::weak_ptr<Object>;
using ExecutableExtensionPtr = std::shared_ptr<ExecutableExtension>;
using ExecutableExtensionWeakPtr = std::weak_ptr<ExecutableExtension>;
using SignalExtensionPtr = std::shared_ptr<SignalExtension>;

}

#endif // META_FORWARDS_HPP
