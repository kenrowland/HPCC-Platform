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


#include "XSDTypeParser.hpp"
#include "CfgStringLimits.hpp"
#include "CfgIntegerLimits.hpp"
#include <iostream>



std::shared_ptr<CfgType> XSDTypeParser::parseSimpleType(const pt::ptree &typeTree)
{
    std::string typeName = typeTree.get("<xmlattr>.name", "not found");
    std::shared_ptr<CfgType> pCfgType = std::make_shared<CfgType>(typeName);

    auto restriction = typeTree.find("xs:restriction");
    // auto it = xsdTree.find("xs:schema");
    if (restriction != typeTree.not_found())
    {
        std::string baseType = restriction->second.get("<xmlattr>.base", "fixme");
        
        std::shared_ptr<CfgLimits> pLimits = std::make_shared<CfgLimits>();

        if (baseType == "xs:string")
        {
            pLimits = std::make_shared<CfgStringLimits>();
            
        }
        else if (baseType == "xs:integer")
        {
            pLimits = std::make_shared<CfgIntegerLimits>();
        }


        if (!restriction->second.empty())
        {
            parseRestrictions(pLimits, restriction->second.get_child("", pt::ptree()));
        }

        pCfgType->setLimits(pLimits);

        pCfgType->isValueValid(4);

        


        //std::shared_ptr<XSDLimits> pLimits = XSDLimits::getLimitsInstance(baseType, restriction->second);
        //s_typeLimits[typeName] = pLimits;

        // auto xx = restriction->second.get_child("", pt::ptree());
        // for (auto it=xx.begin(); it!=xx.end(); ++it)
        // {
        //     std::cout << "restriction = " << it->first << std::endl;
        // }
        // int j = 7;
    }

    int i = 4;
    return pCfgType;
}


void XSDTypeParser::parseRestrictions(std::shared_ptr<CfgLimits> &pLimits, const pt::ptree &tree)
{
    for (auto it=tree.begin(); it!=tree.end(); ++it)
    {
        int value = stoi(it->second.get("<xmlattr>.value", "0"));
        if (it->first == "xs:minLength")
        {
             pLimits->setMinLength(value);
        }
        else if (it->first == "xs:maxLength")
        {
            pLimits->setMaxLength(value);
        }
        else if (it->first == "xs:length")
        {
            pLimits->setLength(value);
        }
        else if (it->first == "xs:minInclusive")
        {
            pLimits->setMinInclusive(value);
        }
        else if (it->first == "xs:maxInclusive")
        {
            pLimits->setMaxInclusive(value);
        }
        else if (it->first == "xs:minExclusive")
        {
            pLimits->setMinExclusive(value);
        }
        else if (it->first == "xs:maxExclusive")
        {
            pLimits->setMaxExclusive(value);
        }
        else if (it->first == "xs:pattern")
        {
            pLimits->addPattern(it->second.get("<xmlattr>.value", "0"));
        }

        //m_pEnumeration = make_shared<XSDEnumeration<std::string>>();

        //std::cout << "restriction = " << it->first << std::endl;
    }
}
