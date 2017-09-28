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
#include "CfgStringLimits.hpp"
#include "CfgIntegerLimits.hpp"
#include <algorithm>

void ConfigItem::addType(const std::shared_ptr<CfgType> &pType)
{
    m_types[pType->getName()] = pType;
}


const std::shared_ptr<CfgType> ConfigItem::getType(const std::string &typeName)
{
    auto it = m_types.find(typeName);
    if (it != m_types.end())
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
        else
        {
            //
            // We are at the root and did not find the type. Inspect the type to see if it is a builtin type and add it if needed
            std::shared_ptr<CfgLimits> pLimits = getStandardTypeLimits(typeName);
            if (pLimits)
            {
                std::shared_ptr<CfgType> pCfgType = std::make_shared<CfgType>(typeName);
                pCfgType->setLimits(pLimits);
                addType(pCfgType);
                return pCfgType;
            }
        }    
    }

    //
    // Did not find the type
    std::string msg = "Unable to find type: " + typeName;
    throw(new ParseException(msg));
}


std::shared_ptr<CfgLimits> ConfigItem::getStandardTypeLimits(const std::string &baseType) const
{
    std::shared_ptr<CfgLimits> pLimits;
	if (baseType == "xs:string" || baseType == "xs:token")
	{
		pLimits = std::make_shared<CfgStringLimits>();
	}
	else if (baseType == "xs:integer" || baseType == "xs:int")
	{
		pLimits = std::make_shared<CfgIntegerLimits>();
	}
    else if (baseType == "xs:nonNegativeInteger")
    {
        pLimits = std::make_shared<CfgIntegerLimits>();
        pLimits->setMinInclusive(0);
    }
    else if (baseType == "xs:positiveInteger")
    {
        pLimits = std::make_shared<CfgIntegerLimits>();
        pLimits->setMinInclusive(1);
    }
	else if (baseType == "xs:boolean")
	{
		pLimits = std::make_shared<CfgStringLimits>(); 
		pLimits->addAllowedValue("true");
		pLimits->addAllowedValue("false");
	}
	
    return pLimits;
}


bool ConfigItem::addUniqueName(const std::string keyName)
{
    auto result = m_keys.insert(keyName);
    return result.second;
}


void ConfigItem::addConfigType(const std::shared_ptr<ConfigItem> &pItem, const std::string &typeName)
{
    auto it = m_configTypes.find(typeName);
    if (it == m_configTypes.end())
    {
        m_configTypes[typeName] = pItem;
    }
    else
    {
        throw(new ParseException("Duplicate config type found: " + pItem->getName()));
    }
}


std::shared_ptr<ConfigItem> ConfigItem::getConfigType(const std::string &name, bool throwIfNotPresent) const
{
    std::shared_ptr<ConfigItem> pItem;
    auto it = m_configTypes.find(name);
    if (it != m_configTypes.end())
    {
        return it->second;
    }
    else
    {
        std::shared_ptr<ConfigItem> pParent = m_pParent.lock();
        if (pParent)
        {
            return pParent->getConfigType(name);
        }
    }
    //
    // Did not find the type
    if (throwIfNotPresent)
    {
        std::string msg = "Unable to find config type: " + name;
        throw(new ParseException(msg));
    }
    return pItem;
}


void ConfigItem::addAttribute(const std::shared_ptr<CfgValue> &pCfgValue)
{
	auto retVal = m_attributes.insert({ pCfgValue->getName(), pCfgValue });
	if (!retVal.second)
	{
		throw(new ParseException("Duplicate attribute (" + pCfgValue->getName() + ") found for element " + m_name));
	}
}


void ConfigItem::addAttribute(const std::vector<std::shared_ptr<CfgValue>> &attributes)
{
	for (auto it = attributes.begin(); it != attributes.end(); ++it)
		addAttribute((*it));
}


std::shared_ptr<CfgValue> ConfigItem::getAttribute(const std::string &name) const
{
	std::shared_ptr<CfgValue> pCfgValue;
	auto it = m_attributes.find(name);
	if (it != m_attributes.end())
		pCfgValue = it->second;
	return pCfgValue;
}


void ConfigItem::addKey(const std::string &keyName, const std::string &elementName, const std::string &attributeName)
{
    std::shared_ptr<ConfigItem> pCfgItem = getChild(elementName);  // todo: validate pCfgItem
    std::shared_ptr<CfgValue> pAttribute = pCfgItem->getAttribute(attributeName);  
    if (pAttribute)
    {
        pAttribute->setKey(true);
        auto result = m_keyDefs.insert({ keyName, pAttribute });
        if (!result.second)
        {
            throw(new ParseException("Duplicate key (" + keyName + ") found for element " + m_name));
        }
    }
}


void ConfigItem::addKeyRef(const std::string &keyName, const std::string &elementName, const std::string &attributeName)
{
    auto keyIt = m_keyDefs.find(keyName);
    if (keyIt != m_keyDefs.end())
    {
        std::shared_ptr<CfgValue> pKeyRefAttribute = keyIt->second;
        std::shared_ptr<ConfigItem> pCfgItem = getChild(elementName);   // todo: validate pCfgItem
        std::shared_ptr<CfgValue> pAttribute = pCfgItem->getAttribute(attributeName); 
        if (pAttribute)
        {
            pAttribute->setKeyRef(pKeyRefAttribute);
        }
    }
}



std::vector<std::shared_ptr<ConfigItem>> ConfigItem::getChildren() const
{
    std::vector<std::shared_ptr<ConfigItem>> children;
    
    for (auto it = m_children.begin(); it != m_children.end(); ++it)
        children.push_back(it->second);

    return children;
}

std::shared_ptr<ConfigItem> ConfigItem::getChild(const std::string &name) const
{
	std::shared_ptr<ConfigItem> pItem;
	auto it = m_children.find(name);
	if (it != m_children.end())
		return it->second;
	return pItem;
}