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


#include "XSDAttribute.hpp"


void XSDAttribute::parse(const pt::ptree &attr)
{
    bool typeSet = false;




    //
    // Process the attribute's attributes 
    const pt::ptree &attributes = attr.get_child("<xmlattr>", pt::ptree());
    for (auto attrIt=attributes.begin(); attrIt!=attributes.end(); ++attrIt)
    {
        //
        // if type is present as an attribute, the the type shall have been previously defined, or shall be a supported base type.
        if (attrIt->first == "type")
        {
            m_pType = XSDType::getTypeInstance(attrIt->second.get_value<std::string>());
        }



        //std::cout << "attr = " << attrIt->first.data() << " second = " << attrIt->second.get_value<std::string>() << std::endl;
    }



}