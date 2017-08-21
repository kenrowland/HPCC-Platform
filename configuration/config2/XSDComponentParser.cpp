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


#include "XSDComponentParser.hpp"
#include "XSDValueSetParser.hpp"
#include "ConfigExceptions.hpp"

namespace pt = boost::property_tree;


// void XSDComponentParser::parseXSD(const pt::ptree &compTree)
// {
//     //
//     // Allocate a valueset to hold the attributes, then parse them
//     std::shared_ptr<ConfigItemValueSet> pAttributeValueSet = std::make_shared<ConfigItemValueSet>("attributes", m_pConfig); // holds the attributes for the component level element 
//     pAttributeValueSet->setDisplayName("Attributes");
//     std::shared_ptr<ConfigItem> pConfigItem = std::dynamic_pointer_cast<ConfigItem>(pAttributeValueSet);
//     std::shared_ptr<XSDValueSetParser> pXSDValueSetParaser = std::make_shared<XSDValueSetParser>(m_basePath, pConfigItem);
//     pXSDValueSetParaser->parseXSD(compTree);
//     m_pConfig->addChild(pAttributeValueSet);

//     //
//     // Now parse the sequence section (these are sub keys for the component)
//     XSDConfigParser::parseXSD(compTree.get_child("xs:sequence", pt::ptree()));
// }



void XSDComponentParser::parseXSD(const pt::ptree &compTree)
{
    bool foundComponentDef = false;

    //
    // First time through look for attributeGroups that can be defined and for existence of a sequenc element that actually defines the component
    for (auto it = compTree.begin(); it != compTree.end(); ++it)
    {
        //
        // Element parent (a type in realilty) and the element name help figure out how to process the XSD schema element
        std::string elemType = it->first;
        if (elemType == "xs:attributeGroup")
        {
            parseAttributeGroup(it->second);
        }
        else if (elemType == "xs:sequence")
        {
            foundComponentDef = true;
        }
    }


    if (foundComponentDef)
    {
        pt::ptree elemTree = compTree.get_child("xs:sequence.xs:element", pt::ptree());
        if (!elemTree.empty())
        {
            std::string componentName = getXSDAttributeValue(elemTree, "<xmlattr>.name");
            m_pConfig->setName(componentName);
            m_pConfig->setMinInstances(elemTree.get("<xmlattr>.minOccurs", 1));
            m_pConfig->setMaxInstances(elemTree.get("<xmlattr>.maxOccurs", "1"));

            //
            // Allocate a value set to hold the component attributes, parse them and save
            std::shared_ptr<ConfigItemValueSet> pAttributeValueSet = std::make_shared<ConfigItemValueSet>("attributes", m_pConfig); // holds the attributes for the component level element 
            pAttributeValueSet->setDisplayName("Attributes");
            std::shared_ptr<ConfigItem> pConfigItem = std::dynamic_pointer_cast<ConfigItem>(pAttributeValueSet);
            std::shared_ptr<XSDValueSetParser> pXSDValueSetParaser = std::make_shared<XSDValueSetParser>(m_basePath, pConfigItem);
            pt::ptree keys = elemTree.get_child("xs:complexType", pt::ptree());
            pXSDValueSetParaser->parseXSD(keys);
            m_pConfig->addChild(pAttributeValueSet);

            //
            // Now parse the sequence section (these are sub keys for the component)
            XSDConfigParser::parseXSD(elemTree.get_child("xs:complexType.xs:sequence", pt::ptree()));
        }
        else
        {
            throw(new ParseException("Missing element section for component."));
        }
    }
}
