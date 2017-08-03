/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2015 HPCC Systems®.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################## */

#include "ConfigItem.hpp"
#include "ConfigExceptions.hpp"


void ConfigItem::addType(const std::shared_ptr<CfgType> &pType)
{
    m_pTypes[pType->getName()] = pType;
}


const std::shared_ptr<CfgType> &ConfigItem::getType(const std::string &typeName) const
{
    auto it = m_pTypes.find(typeName);
    if (it != m_pTypes.end())
    {
        return it->second;
    }
    else
    {
        std::shared_ptr<ConfigItem> pParent = m_pParent.lock();
        if (pParent)
        {
            return pParent->getType(typeName);
        }
        std::string msg = "Unable to find type: " + typeName;
        throw(new ParseException(msg));
    }
    
}


const std::vector<std::shared_ptr<CfgValue>> &ConfigItem::getNamedValueSet(const std::string &setName) const\
{
    auto it = m_namedValueSets.find(setName);
    if (it != m_namedValueSets.end())
    {
        return it->second;
    }
    else
    {
        std::shared_ptr<ConfigItem> pParent = m_pParent.lock();
        if (pParent)
        {
            return pParent->getNamedValueSet(setName);
        }
        std::string msg = "Unable to find named value set: " + setName;
        throw(new ParseException(msg));
    }


    
}