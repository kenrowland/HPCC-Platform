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
#include "XSDTypeParser.hpp"

namespace pt = boost::property_tree;

bool XSDConfigParser::doParse(const std::string &envFilename)
{

    pt::ptree xsdTree;

    try
    {
        std::string fpath = m_basePath + envFilename;
        pt::read_xml(fpath, xsdTree);

        //
        // Proccess include files first
        auto schemaIt = xsdTree.find("xs:schema");
        const pt::ptree &keys = schemaIt->second.get_child("", pt::ptree());
        for (auto it = keys.begin(); it != keys.end(); ++it)
        {
            if (it->first == "xs:include")
            {
                std::string schema = it->second.get("<xmlattr>.schemaLocation", "not found");
                parseXSD(schema);
                // std::cout << "Parsing found include: " << schema << std::endl;
            }
        }
    }
    catch (std::exception &e)
    {
        throw(ParseException("Input configuration file is not valid"));
    }

    return true;
}

bool XSDConfigParser::parseXSD(const std::string &filename, bool isComponent)
{
    bool rc = true;
    pt::ptree xsdTree;
    std::string fpath = m_basePath + filename;

    try
    {
        pt::read_xml(fpath, xsdTree);
    }
    catch (std::exception &e)
    {
        //throw(ParseException("Input configuration file is not valid"));
        rc = false;
    }

    if (!rc)
        return false;


    //
    // skip ahead to the meat
    auto schemaIt = xsdTree.find("xs:schema");

    //
    // process the XSD
    const pt::ptree &keys = schemaIt->second.get_child("", pt::ptree());
    for (auto it = keys.begin(); it != keys.end(); ++it)
    {
        if (it->first == "xs:include")
        {
            std::string schemaFile = it->second.get("<xmlattr>.schemaLocation", "not found");
            bool isComponentInclude = it->second.get("<xmlattr>.hpcc:component", false);
            if (isComponentInclude && isComponent)
                throw(ParseException("A component include is not allowe with a component"));

            if (!parseXSD(schemaFile))
                throw(ParseException("Error parsing included schema"));
        }
        else if (it->first == "xs:simpleType")
        {
            std::shared_ptr<XSDTypeParser> pTypeParser = std::make_shared<XSDTypeParser>();
            std::shared_ptr<CfgType> pType = pTypeParser->parseSimpleType(it->second);
            m_pConfig->addType(pType);
        }
        else if (it->first == "xs:complexType")
        {
            if (isComponent)
                parseComponent(it->second);
        }
        else if (it->first == "xs:attributeGroup")
        {

        }
    }

    return rc;
}


bool XSDConfigParser::parseComponent(const pt::ptree &componentTree)
{
    //
    // Get component info
    std::string name = componentTree.get("<xmlattr>.name", "missing");

    //auto schemaIt = xsdTree.find("xs:sequence");
    const pt::ptree &keys = componentTree.get_child("xs:sequence.xs:element", pt::ptree());
    std::string elementName = keys.get("<xmlattr>.name", "noname");
    // for (auto it=keys.begin(); it!=keys.end(); ++it)
    // {
        
    // }

    int i = 4;

}


bool XSDConfigParser::parseAttributeGroup(const pt::ptree &attributeTree)
{
    std::string groupName = "unknown";
    try
    {
        groupName = attributeTree.get<std::string>("<xmlattr>.name");
        for (auto attrIt=attributeTree.begin(); attrIt!=attributeTree.end(); ++attrIt)
        {
            if (attrIt->first == "xs:attribute")
            {
                //
                // Get required attributes, these will throw if not found
                std::string name = attrIt->second.get<std::string>("<xmlattr>.name");
                std::string typeName = attrIt->second.get<std::string>("<xmlattr>.type");

                std::shared_ptr<CfgValue> pValue = std::make_shared<CfgValue>(name);
                std::shared_ptr<CfgType> pType = m_pConfig->getType(typeName);
                pValue->setType(pType);

                 //<xs:attribute name="computer" type="nodeName" use="required" hpcc:mirror="//Computer[@name]"/>

            }
        }

    }
    catch(std::exception &e)
    {
        std::string msg = "There was an error parsing attributeGroup: " + groupName;
        throw(ParseException(msg));

    }
    

}