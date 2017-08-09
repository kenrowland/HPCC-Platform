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

class ConfigItemValueSet;

class ConfigItem 
{
    public:

        ConfigItem(const std::string &name, std::shared_ptr<ConfigItem> &pParent, const std::string &className="root") : 
            m_className(className), m_name(name), m_pParent(pParent), m_displayName(name), 
            m_minInstances(1), m_maxInstances(1)  { };
        virtual ~ConfigItem() { };

        virtual const std::string &getClassName() const { return m_className; };
        virtual const std::string &getName() const { return m_name; };
        virtual void setName(const std::string &name) { m_name = name; };
        
        //virtual void addValueSetType(const std::vector<std::shared_ptr<CfgValue>> &set, const std::string &setName);
        virtual const std::vector<std::shared_ptr<CfgValue>> &getValueSetType(const std::string &setName) const;
        virtual void setDisplayName(const std::string &displayName) { m_displayName = displayName; };
        virtual void setMinInstances(int num) { m_minInstances = num; };
        virtual const int getMinInstances() { return m_minInstances; };
        virtual void setMaxInstances(int num) { m_maxInstances = num; };
        virtual const int getMaxInstances() { return m_maxInstances; };

        virtual void addType(const std::shared_ptr<CfgType> &pType);
        virtual const std::shared_ptr<CfgType> &getType(const std::string &typeName) const;
        virtual void addValueToValueSetType(const std::shared_ptr<CfgValue> &pValue, const std::string &setName);
        virtual void addConfigValue(const std::shared_ptr<CfgValue> &pValue) { m_configValues.push_back(pValue); };
        virtual void addConfigValue(const std::vector<std::shared_ptr<CfgValue>> &configValues);


    protected:

        // some kind of category map parent/child Software->ESP->[components]
        ConfigItem() { };

        std::string m_name;
        std::string m_displayName;
        std::string m_className;
        std::vector<std::shared_ptr<ConfigItem>> m_children;
        std::weak_ptr<ConfigItem> m_pParent;

        std::map<std::string, std::shared_ptr<CfgType>> m_pTypes;
        std::map<std::string, std::vector<std::shared_ptr<CfgValue>>> m_valueSetTypes;   // in XSD terms, an attribute group that can be referenced
        std::vector<std::shared_ptr<CfgValue>> m_configValues;                           // this item's config values (like attributes in XML terms)



        std::vector<std::shared_ptr<ConfigItemValueSet>> m_valueSets;   // for a component this is the set of attributes by element

        int m_minInstances;
        int m_maxInstances;
        

    private:

        
        

};


#endif // _CONFIG2_CONFIGITEM_HPP_
