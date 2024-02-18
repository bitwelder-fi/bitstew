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

#ifndef META_METAOBJECT_HPP
#define META_METAOBJECT_HPP

#include <meta/meta.hpp>
#include <meta/metadata/metaclass.hpp>

namespace meta
{

/// The base class of objects whith metadata.
class META_API MetaObject
{
    friend class MetaClass;

public:
    /// Destructor.
    virtual ~MetaObject();

    /// Returns the name of the meta-object.
    /// \return The name of the meta-object.
    std::string_view getName() const
    {
        return m_name;
    }

    STATIC_META_CLASS("meta.MetaObject", MetaObject)
    {
    };

    /// Returns the dynamic meta-class of the meta object.
    virtual const MetaClass* getDynamicMetaClass() const
    {
        return getStaticMetaClass();
    }

    static MetaObjectPtr create(std::string_view name)
    {
        return MetaObjectPtr(new MetaObject(name));
    }

    /// Returns the factory meta-class which created the object. By default this is the dynamic meta-class.
    /// \return The factory meta-class which created the object. If the object was not creasted by
    ///         a meta-class, returns nullptr;
    const MetaClass* getFactory() const
    {
        return m_factory;
    }

protected:
    /// Constructor. Fails if the meta-name passed as argument is invalid.
    explicit MetaObject(std::string_view metaName);

    /// Second phase initializer of the object.
    void initialize()
    {
    }

private:
    /// Holds the meta-class which created the object. The value of \e nullptr means the object was
    /// not created through meta-class.
    MetaClass* m_factory = nullptr;
    /// The meta-name of the meta-object.
    std::string m_name;
};

}

#endif
