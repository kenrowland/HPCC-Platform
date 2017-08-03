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


#ifndef _CONFIG2_CONFIGITEM_HPP_
#define _CONFIG2_CONFIGITEM_HPP_

#include <string>
#include <memory>
#include <map>
#include "CfgType.hpp"
#include "CfgValue.hpp"



class ConfigItem 
{
    public:

        ConfigItem(const std::string &name, const std::string &className, std::shared<ConfigItem> &pParent) : m_name(name), m_className(className), m_pParent(pParent)  { };
        virtual ~ConfigItem() { };

        void addType(const std::shared_ptr<CfgType> &pType);
        const std::shared_ptr<CfgType> &getType(const std::string &typeName) const;
        const std::vector<std::shared_ptr<CfgValue>> &getNamedValueSet(const std::string &setName) const;


    protected:

        // some kind of category map parent/child Software->ESP->[components]
        ConfigItem() { };

        std::string m_class;
        std::vector<std::shared_ptr<ConfigItem>> m_children;
        std::weak_ptr<ConfigItem> m_pParent;
        std::map<std::string, std::shared_ptr<CfgType>> m_pTypes;
        std::map<std::string, std::vector<std::shared_ptr<CfgValue>>> m_namedValueSets;   // in XSD terms, an attribute group that can be referenced
        

    private:

        
        

};


#endif // _CONFIG2_CONFIGITEM_HPP_
