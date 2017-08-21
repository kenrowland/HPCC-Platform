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

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
// //#include <boost/property_tree/json_parser.hpp>
// #include <boost/foreach.hpp>
// #include <string>
// #include <set>
#include <exception>
// #include <iostream>
// #include <vector>
#include "XSDConfigParser.hpp"
#include "ConfigExceptions.hpp"
#include "CfgValue.hpp"
//#include "XSDTypeParser.hpp"
// #include "CfgStringLimits.hpp"
// #include "CfgIntegerLimits.hpp"

#include "XSDComponentParser.hpp"
#include "XSDValueSetParser.hpp"
#include "ConfigItemComponent.hpp"

namespace pt = boost::property_tree;

bool XSDConfigParser::doParse(const std::string &envFilename)
{
    bool rc = true;
    try
    {
        parseXSD(envFilename);
    }
    catch (ParseException &e)
    {
        rc = false;
    }
    // pt::ptree xsdTree;

    // try
    // {
    //     std::string fpath = m_basePath + envFilename;
    //     pt::read_xml(fpath, xsdTree);

    //     //
    //     // // Proccess include files first
    //     // auto schemaIt = xsdTree.find("xs:schema");
    //     // const pt::ptree &keys = schemaIt->second.get_child("", pt::ptree());
    //     parseXSD(xsdTree);
    //     // for (auto it = keys.begin(); it != keys.end(); ++it)
    //     // {
    //     //     if (it->first == "xs:include")
    //     //     {
    //     //         std::string schema = it->second.get("<xmlattr>.schemaLocation", "not found");
    //     //         parseXSD(schema);
    //     //         // std::cout << "Parsing found include: " << schema << std::endl;
    //     //     }
    //     // }
    // }
    // catch (std::exception &e)
    // {
    //     throw(ParseException("Input configuration file is not valid"));
    // }

    return rc;
}


void XSDConfigParser::parseXSD(const std::string &filename)
{
    try
    {
        pt::ptree xsdTree;
        std::string fpath = m_basePath + filename;
        pt::read_xml(fpath, xsdTree);
        auto schemaIt = xsdTree.find("xs:schema");
        const pt::ptree &keys = schemaIt->second.get_child("", pt::ptree());
        parseXSD(keys);
    }
    catch (std::exception &e)
    {
        throw(ParseException("Input configuration file is not valid"));
    }
}


void XSDConfigParser::parseXSD(const pt::ptree &keys)
{
    for (auto it = keys.begin(); it != keys.end(); ++it)
    {
        //
        // Element parent (a type in realilty) and the element name help figure out how to process the XSD schema element
        std::string elemType = it->first;
        if (it->first == "xs:include")
        {
            std::string schemaFile = getXSDAttributeValue(it->second, "<xmlattr>.schemaLocation");
            if (m_pConfig->addKey(schemaFile))
            {
                parseXSD(schemaFile);
            }
        }
        else if (it->first == "xs:simpleType")
        {
            parseSimpleType(it->second);
        }
        else if (it->first == "xs:complexType")
        {
            parseComplexType(it->second);
        }
        else if (it->first == "xs:attributeGroup")
        {
            parseAttributeGroup(it->second);
        }
        else if (it->first == "xs:attribute")
        {
            parseAttribute(it->second);
        }
        else if (it->first == "xs:sequence")
        {
            parseXSD(it->second.get_child("", pt::ptree()));
        }
        else if (it->first == "xs:element")
        {
            parseElement(it->second);
        }
    }
}


std::string XSDConfigParser::getXSDAttributeValue(const pt::ptree &tree, const std::string &attrName, bool throwIfNotPresent, const std::string &defaultVal) const
{
    std::string value = defaultVal;
    try
    {
        value = tree.get<std::string>(attrName);
    }
    catch (std::exception &e)
    {
        if (throwIfNotPresent)
            throw(ParseException("Missing attribute " + attrName + "."));
    }
    return value;
}


void XSDConfigParser::parseSimpleType(const pt::ptree &typeTree)
{
    std::shared_ptr<CfgType> pCfgType = getCfgType(typeTree, true);
    m_pConfig->addType(pCfgType);
}


void XSDConfigParser::parseAttributeGroup(const pt::ptree &attributeTree)
{
    std::string groupName = getXSDAttributeValue(attributeTree, "<xmlattr>.name");  // only a named attributeGroup is supported

    std::shared_ptr<ConfigItemValueSet> pValueSet = std::make_shared<ConfigItemValueSet>(groupName, m_pConfig); 
    std::shared_ptr<ConfigItem> pConfigItem = std::dynamic_pointer_cast<ConfigItem>(pValueSet);
    std::shared_ptr<XSDValueSetParser> pXSDValueSetParaser = std::make_shared<XSDValueSetParser>(m_basePath, pConfigItem);
    pXSDValueSetParaser->parseXSD(attributeTree.get_child("", pt::ptree())); 
    m_pConfig->addConfigType(pValueSet);
}



void XSDConfigParser::parseComplexType(const pt::ptree &typeTree)
{
    std::string complexTypeName = getXSDAttributeValue(typeTree, "<xmlattr>.name", false, "");
    std::string className = typeTree.get("<xmlattr>.hpcc:class", "");
    std::string catName = typeTree.get("<xmlattr>.hpcc:category", "");
    std::string displayName = typeTree.get("<xmlattr>.hpcc:displayName", "");

    if (complexTypeName != "" || className != "")
    {
        if (className == "component")
        {
            std::shared_ptr<ConfigItemComponent> pComponent = std::make_shared<ConfigItemComponent>(complexTypeName, m_pConfig); 
            pComponent->setCategory(catName);
            pComponent->setDisplayName(displayName);
            pt::ptree componentTree = typeTree.get_child("", pt::ptree());
            if (!componentTree.empty())
            {
                std::shared_ptr<ConfigItem> pConfigItem = std::dynamic_pointer_cast<ConfigItem>(pComponent);
                std::shared_ptr<XSDComponentParser> pComponentXSDParaser = std::make_shared<XSDComponentParser>(m_basePath, pConfigItem);
                pComponentXSDParaser->parseXSD(componentTree); 
                m_pConfig->addConfigType(pComponent);
            }
            else
            {
                throw(new ParseException("Component definition empty: " + displayName));
            }
        }
        // todo: else throw, not recognized complexType class
    }

    //
    // Just a complexType delimiter, ignore and parse the children
    else
    {
        parseXSD(typeTree.get_child("", pt::ptree()));
    }
}


void XSDConfigParser::parseElement(const pt::ptree &elemTree)
{
    std::string elementName = elemTree.get("<xmlattr>.name", "");
    std::string className = elemTree.get("<xmlattr>.hpcc:class", "");
    std::string catName = elemTree.get("<xmlattr>.hpcc:category", "");
    std::string displayName = elemTree.get("<xmlattr>.hpcc:displayName", "");
    std::string refName = elemTree.get("<xmlattr>.ref", "");
    std::string typeName = elemTree.get("<xmlattr>.type", "");

    if (className == "component")
    {
        std::shared_ptr<ConfigItemComponent> pComponent = std::make_shared<ConfigItemComponent>(elementName, m_pConfig);  
        pComponent->setCategory(catName);
        pComponent->setDisplayName(displayName);
        pComponent->setMinInstances(elemTree.get("<xmlattr>.minOccurs", 1));
        pComponent->setMaxInstances(elemTree.get("<xmlattr>.maxOccurs", "1"));
        std::shared_ptr<ConfigItem> pConfigItem = std::dynamic_pointer_cast<ConfigItem>(pComponent);
        std::shared_ptr<XSDComponentParser> pComponentXSDParaser = std::make_shared<XSDComponentParser>(m_basePath, pConfigItem);
        pComponentXSDParaser->parseXSD(elemTree.get_child("xs:complexType", pt::ptree())); 
        m_pConfig->addChild(pComponent);
    }
    else if (className == "category")
    {
        std::shared_ptr<ConfigItem> pConfigItem = std::make_shared<ConfigItem>(catName, "category", m_pConfig);
        std::shared_ptr<XSDConfigParser> pXSDParaser = std::make_shared<XSDConfigParser>(m_basePath, pConfigItem);
        pXSDParaser->parseXSD(elemTree.get_child("", pt::ptree()));
        m_pConfig->addChild(pConfigItem);
    }
    else if (elementName != "")
    {
        //
        // If this element has a type, then it will contain data. Pull the type 
        // elemTree.get_child("xs:complexType", pt::ptree()) not empty
        // typeName != ""
        // then give m_pConfig a value (cfgvalue)
        if (typeName != "")
        {
            std::shared_ptr<CfgValue> pCfgValue = std::make_shared<CfgValue>("");  // a no name value becuase it belongs to the element
            pCfgValue->setType(m_pConfig->getType(typeName));
            m_pConfig->setItemCfgValue(pCfgValue);
        }
        else 
        {
            std::shared_ptr<CfgType> pType = getCfgType(elemTree.get_child("xs:simpleType", pt::ptree()), false);
            if (pType->isValid())
            {
                std::shared_ptr<CfgValue> pCfgValue = std::make_shared<CfgValue>("");  // a no name value becuase it belongs to the element
                pCfgValue->setType(pType);
                m_pConfig->setItemCfgValue(pCfgValue);
            }
            else 
            {
                throw(new ParseException("Invalid type for element " + elementName));
            }
        }

        
        std::shared_ptr<ConfigItemValueSet> pValueSet = std::make_shared<ConfigItemValueSet>(elementName, m_pConfig);  
        pValueSet->setDisplayName(displayName);
        pValueSet->setMinInstances(elemTree.get("<xmlattr>.minOccurs", 1));
        pValueSet->setMaxInstances(elemTree.get("<xmlattr>.maxOccurs", "1"));
        std::shared_ptr<ConfigItem> pConfigItem = std::dynamic_pointer_cast<ConfigItem>(pValueSet);
        std::shared_ptr<XSDValueSetParser> pValueSetXSDParaser = std::make_shared<XSDValueSetParser>(m_basePath, pConfigItem);
        pValueSetXSDParaser->parseXSD(elemTree.get_child("xs:complexType", pt::ptree())); 
        m_pConfig->addChild(pValueSet);
    }
    else if (refName != "")
    {
        std::shared_ptr<ConfigItem> pConfigItem = m_pConfig->getConfigType(refName);
        std::shared_ptr<ConfigItemComponent> pComponent = std::dynamic_pointer_cast<ConfigItemComponent>(pConfigItem);
        if (pComponent)
        {
             std::shared_ptr<ConfigItemComponent> pComponentCopy = std::make_shared<ConfigItemComponent>(*pComponent);
             m_pConfig->addChild(pComponentCopy);
        }
        else
        {
            throw(new ParseException("Element reference is not a compoenent: " + refName));
        }
    }
    else
    {
        throw(new ParseException("Only component elements are supported at the top level"));
    }
}


std::shared_ptr<CfgType> XSDConfigParser::getCfgType(const pt::ptree &typeTree, bool nameRequired)
{
    std::string typeName = getXSDAttributeValue(typeTree, "<xmlattr>.name", nameRequired, "");

    std::shared_ptr<CfgType> pCfgType = std::make_shared<CfgType>(typeName);
    std::shared_ptr<CfgLimits> pLimits = std::make_shared<CfgLimits>();
    auto restriction = typeTree.find("xs:restriction");
    if (restriction != typeTree.not_found())
    {
        std::string baseType = getXSDAttributeValue(restriction->second, "<xmlattr>.base");
        pLimits = m_pConfig->getStandardTypeLimits(baseType);
        if (!pLimits)
        {
            std::string msg = "Unsupported base type(" + baseType + ")";
            throw(ParseException(msg));
        }

        if (!restriction->second.empty())
        {
            pt::ptree restrictTree = restriction->second.get_child("", pt::ptree());
            for (auto it = restrictTree.begin(); it != restrictTree.end(); ++it)
            {
                try
                {
                    std::string restrictionType = it->first;
                    if (restrictionType == "xs:minLength")
                        pLimits->setMinLength(it->second.get<int>("<xmlattr>.value"));
                    else if (restrictionType == "xs:maxLength")
                        pLimits->setMaxLength(it->second.get<int>("<xmlattr>.value"));
                    else if (restrictionType == "xs:length")
                        pLimits->setLength(it->second.get<int>("<xmlattr>.value"));
                    else if (restrictionType == "xs:minInclusive")
                        pLimits->setMinInclusive(it->second.get<int>("<xmlattr>.value"));
                    else if (restrictionType == "xs:maxInclusive")
                        pLimits->setMaxInclusive(it->second.get<int>("<xmlattr>.value"));
                    else if (restrictionType == "xs:minExclusive")
                        pLimits->setMinExclusive(it->second.get<int>("<xmlattr>.value"));
                    else if (restrictionType == "xs:maxExclusive")
                        pLimits->setMaxExclusive(it->second.get<int>("<xmlattr>.value"));
                    else if (restrictionType == "xs:pattern")
                        pLimits->addPattern(it->second.get("<xmlattr>.value", "0"));
                    else if (restrictionType == "xs:enumeration")
                        pLimits->addAllowedValue(it->second.get("<xmlattr>.value", "badbadbad"), it->second.get("<xmlattr>.hpcc:description", ""));
                    else if (restrictionType.find("xs:") != std::string::npos)
                    {
                        std::string msg = "Unsupported restriction(" + it->first + ") found while parsing type(" + typeName + ")";
                        throw(new ParseException(msg));
                    }
                }
                catch (std::exception &e)
                {
                    std::string msg = "Invalid value found for restriction(" + it->first + ")";
                    throw(ParseException(msg));
                }
            }
        }
    }

    pCfgType->setLimits(pLimits);
    return pCfgType;
}



void XSDConfigParser::parseAttribute(const pt::ptree &attr)
{
    // std::string typeName = getXSDAttributeValue(attr, "<xmlattr>.type");
    // std::string attrName = getXSDAttributeValue(attr, "<xmlattr>.name");
    // std::shared_ptr<CfgValue> pCfgValue = std::make_shared<CfgValue>(attrName);
    // pCfgValue->setType(m_pConfig->getType(typeName));
    // // add other stuff later such as hpcc:mirror, hpcc:dependson, hpcc:presentif

    //     m_pConfig->addConfigValue(pCfgValue);
    // }
}