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
#include "CfgStringLimits.hpp"
#include "CfgIntegerLimits.hpp"

#include "XSDComponentParser.hpp"
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
            parseXSD(schemaFile);
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
        std::string value = tree.get<std::string>(attrName);
    }
    catch (std::exception &e)
    {
        if (throwIfNotPresent)
            throw(ParseException("Missint attribute " + attrName + "."));
    }
    return defaultVal;
}


void XSDConfigParser::parseSimpleType(const pt::ptree &typeTree)
{
    std::string typeName = getXSDAttributeValue(typeTree, "<xmlattr>.name");

    std::shared_ptr<CfgType> pCfgType = std::make_shared<CfgType>(typeName);
    auto restriction = typeTree.find("xs:restriction");
    if (restriction != typeTree.not_found())
    {
        std::string baseType = getXSDAttributeValue(restriction->second, "<xmlattr>.base");
        std::shared_ptr<CfgLimits> pLimits = std::make_shared<CfgLimits>();

        if (baseType == "xs:string")
            pLimits = std::make_shared<CfgStringLimits>();
        else if (baseType == "xs:integer")
            pLimits = std::make_shared<CfgIntegerLimits>();
        else
        {
            std::string msg = "Unsupported base type(" + baseType + ")";
            throw(ParseException(msg));
        }

        if (!restriction->second.empty())
        {
            pt::ptree restrictTree = restriction->second.get_child("", pt::ptree());
            for (auto it = restrictTree.begin(); it != restrictTree.end(); ++it)
            {
                int value;
                try
                {
                    value = it->second.get<int>("<xmlattr>.value");
                }
                catch (std::exception &e)
                {
                    std::string msg = "Invalid value found for restriction(" + it->first + ")";
                    throw(ParseException(msg));
                }

                if (it->first == "xs:minLength")
                    pLimits->setMinLength(value);
                else if (it->first == "xs:maxLength")
                    pLimits->setMaxLength(value);
                else if (it->first == "xs:length")
                    pLimits->setLength(value);
                else if (it->first == "xs:minInclusive")
                    pLimits->setMinInclusive(value);
                else if (it->first == "xs:maxInclusive")
                    pLimits->setMaxInclusive(value);
                else if (it->first == "xs:minExclusive")
                    pLimits->setMinExclusive(value);
                else if (it->first == "xs:maxExclusive")
                    pLimits->setMaxExclusive(value);
                else if (it->first == "xs:pattern")
                    pLimits->addPattern(it->second.get("<xmlattr>.value", "0"));
                else
                {
                    std::string msg = "Unsupported restriction(" + it->first + ") found while parsing type(" + typeName + ")";
                    throw(new ParseException(msg));
                }
            }
        }

        pCfgType->setLimits(pLimits);
        m_pConfig->addType(pCfgType);
    }
}



void XSDConfigParser::parseAttributeGroup(const pt::ptree &attributeTree)
{
    m_curXSDElementParent = "xs:attributeGroup";
    m_curXSDElementName = getXSDAttributeValue(attributeTree, "<xmlattr>.name");
    parseXSD(attributeTree.get_child("", pt::ptree()));  // parse the elements in the attribute group

    // std::string groupName = m_curXSDElementName;
    // std::vector<std::shared_ptr<CfgValue>> valueSet;
    // try
    // {
    //     groupName = attributeTree.get<std::string>("<xmlattr>.name");
    //     const pt::ptree &keys = attributeTree.get_child("", pt::ptree());
    //     for (auto attrIt = keys.begin(); attrIt != keys.end(); ++attrIt)
    //     {
    //         if (attrIt->first == "xs:attribute")
    //         {
    //             std::shared_ptr<CfgValue> pValue = parseAttribute(attriIt->second);
    //             valueSet.push_back(pValue);
    //         }
    //     }
    //     m_pConfig->addNamedValueSet(valueSet, groupName);
    // }
    // catch (std::exception &e)
    // {
    //     std::string msg = "There was an error parsing attributeGroup: " + groupName;
    //     throw(ParseException(msg));
    // }
}



void XSDConfigParser::parseComplexType(const pt::ptree &typeTree)
{
    std::string typeName = getXSDAttributeValue(typeTree, "<xmlattr>.name", false, "");

    if (typeName != "")
    {
        std::string className = typeTree.get("<xmlattr>.hpcc:class", "");
        std::string catName = typeTree.get("<xmlattr>.hpcc:category", "");
        std::string displayName = typeTree.get("<xmlattr>.hpcc:displayName", "");

        if (className == "component")
        {
            std::shared_ptr<ConfigItemComponent> pComponent = std::make_shared<ConfigItemComponent>("", m_pConfig);  // don't know component name yet, will be in a later xs:element tag
            pComponent->setDisplayName(displayName);
            std::shared_ptr<XSDComponentParser> pComponentXSDParaser = std::make_shared<XSDComponentParser>(m_basePath, m_pConfig);
            pComponentXSDParaser->parseXSD(typeTree.get_child("", pt::ptree()));
        }
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
    // not sure yet, might be an error or could be category like sw/programs/hw etc. turns into category all bsed on hpcc:class
}


void XSDConfigParser::parseAttribute(const pt::ptree &attr) const
{
    std::string typeName = getXSDAttributeValue(attr, "<xmlattr>.type");
    std::string attrName = getXSDAttributeValue(attr, "<xmlattr>.name");
    std::shared_ptr<CfgValue> pCfgValue = std::make_shared<CfgValue>(attrName);
    pCfgValue->setType(m_pConfig->getType(typeName));
    // add other stuff later such as hpcc:mirror, hpcc:dependson, hpcc:presentif

    if (m_curXSDElementParent == "xs:attributeGroup")
    {
        m_pConfig->addValueToValueSetType(pCfgValue, m_curXSDElementName);
    }
    else
    {
        m_pConfig->addConfigValue(pCfgValue);
    }
}
