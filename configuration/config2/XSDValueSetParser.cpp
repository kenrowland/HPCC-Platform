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

#include "XSDValueSetParser.hpp"
#include "ConfigExceptions.hpp"

namespace pt = boost::property_tree;

void XSDValueSetParser::parseXSD(const pt::ptree &valueSetTree)
{
    for (auto it = valueSetTree.begin(); it != valueSetTree.end(); ++it)
    {
        //
        // Element parent (a type in realilty) and the element name help figure out how to process the XSD schema element
        std::string elemType = it->first;
        if (it->first == "xs:attributeGroup")
        {
            parseAttributeGroup(it->second);
        }
        else if (it->first == "xs:attribute")
        {
            parseAttribute(it->second);
        }
        // this is where elements in element would be hangled,probably with allocting new valueSet with this one as the parent.
    }
}


void XSDValueSetParser::parseAttributeGroup(const pt::ptree &attributeTree)
{
    //
    // Only support an attribute reference. The ref value is a type.
    std::string groupRefName = getXSDAttributeValue(attributeTree, "<xmlattr>.ref");
    std::shared_ptr<ConfigItemValueSet> pValueSet = std::dynamic_pointer_cast<ConfigItemValueSet>(m_pConfig->getConfigType(groupRefName));
    if (pValueSet)
    {
        m_pValueSet->addCfgValue(pValueSet);
    }
}


void XSDValueSetParser::parseAttribute(const pt::ptree &attr)
{
    std::string attrName = getXSDAttributeValue(attr, "<xmlattr>.name");
    std::string use = attr.get("<xmlattr>.use", "required");
    std::shared_ptr<CfgValue> pCfgValue = std::make_shared<CfgValue>(attrName);
    pCfgValue->setDisplayName(attr.get("<xmlattr>.hpcc:displayName", attrName));
    pCfgValue->setRequired(use == "required");
    pCfgValue->setForceOutput(attr.get("<xmlattr>.forceOutput", true));
    pCfgValue->setTooltip(attr.get("<xmlattr>.hpcc:tooltip", ""));
    pCfgValue->setReadOnly(attr.get("<xmlattr>.hpcc:readOnly", "false") == "true");
    pCfgValue->setHidden(attr.get("<xmlattr>.hpcc:hidden", "false") == "true");

    std::string modList = attr.get("<xmlattr>.modifiers", "");
    if (modList.length())
    {
        pCfgValue->setModifiers(split(modList, ","));
    }

	try
	{
		std::string defaultValue = getXSDAttributeValue(attr, "<xmlattr>.default");
		pCfgValue->setDefault(defaultValue);
	}
	catch (ParseException &e)
	{
		// don't do anything, no default found
	}


    std::string typeName = attr.get("<xmlattr>.type", "");
    if (typeName != "")
    {
        pCfgValue->setType(m_pConfig->getType(typeName));
    }
    else
    {
        std::shared_ptr<CfgType> pCfgType = getCfgType(attr.get_child("xs:simpleType", pt::ptree()), false);
        if (!pCfgType->isValid())
        {
            throw(new ParseException("Attribute " + attrName + " does not have a valid type"));
        }
        pCfgValue->setType(pCfgType);
    }
    m_pValueSet->addCfgValue(pCfgValue);
}


std::vector<std::string> XSDValueSetParser::split(const std::string  &input, const std::string  &delim)
{
    size_t  start = 0, end = 0, delimLen = delim.length();
    std::vector<std::string> list;

    while (end != std::string::npos)
    {
        end = input.find(delim, start);
        list.push_back(input.substr(start, (end == std::string::npos) ? std::string::npos : end - start));

        // If at end, use start=maxSize.  Else use start=end+delimiter.
        start = ((end > (std::string::npos - delimLen)) ? std::string::npos : end + delimLen); 
    }
    return list;
}