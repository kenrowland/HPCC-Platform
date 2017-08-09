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
#include "ConfigExceptions.hpp"

namespace pt = boost::property_tree;


void XSDComponentParser::parseElement(const pt::ptree &elemTree)
{
    std::string elementName = elemTree.get("<xmlattr>.name", "");
    std::string refName = elemTree.get("<xmlattr>.ref", "");

    if (elementName != "")
    {
        //
        // If we don't have a component name yet, this element is for the component (likely defined by a complexType)
        if (m_pConfig->getName() == "")
        {
            // m_pConfig->setName(elementName);
            // std::shared_ptr<ConfigValueSet> pValueSet = make_shared<ConfigItemValueSet>("attributes", m_pConfig); // holds the attributes for the component level element
            // m_pAttributeValueSet->setDisplayName("Attributes");
            // const pt::ptree &keys = typeTree.get_child("", pt::ptree());
            // parseXSD(keys);  // go parse the rest of the component
        }
        //
        // Since we have a name, we are now taking keys for the component. An element is a valueset
        else
        {

            // // use an xsd element parser here
            // std::shared_ptr<ConfigValueSet> pValueSet = make_shared<ConfigItemValueSet>(elementName, m_pConfig); 
            // std::string displayName = elemTree.get("<xmlattr>.hpcc:displayName", m_name);
            // pValueSet->setDisplayName(displayName);
            // pValueSet->setMinInstances(elemTree.get("<xmlattr>.minOccurs", 1);
            // std::string maxStr = elemTree.get("<xmlattr>.maxOccurs", "1");
            // pValueSet->setMaxInstances((maxStr == "unbounded") ? -1 : stoi(maxStr));
            // const pt::ptree &keys = typeTree.get_child("", pt::ptree());
            // pValueSet->parseXSD(keys);
            

        }
    }
    else if (refName != "")
    {
        // we need to find the referenced complex type, it's the component def
    }
    else
    {
        std::string msg = "Element missing name or ref. One is required";
        throw(ParseException(msg));
    }
}

