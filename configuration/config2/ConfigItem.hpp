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
#include <set>
#include <map>
#include "CfgType.hpp"
#include "CfgValue.hpp"


class ConfigItemValueSet;


class ConfigItem 
{
    public:

        ConfigItem(const std::string &name, const std::string &className="category", std::shared_ptr<ConfigItem> pParent=nullptr) : 
            m_className(className), m_name(name), m_pParent(pParent), m_displayName(name), 
            m_minInstances(1), m_maxInstances(1)  { }
        virtual ~ConfigItem() { }

        virtual const std::string &getClassName() const { return m_className; }

        virtual const std::string &getName() const { return m_name; }
        virtual void setName(const std::string &name) { m_name = name; }      

        virtual void setDisplayName(const std::string &displayName) { m_displayName = displayName; }
        virtual const std::string &getDisplayName() const { return m_displayName; }

        virtual void setCategory(const std::string &category) { m_category = category; }
        virtual const std::string &getCategory() const { return m_category; }

        void setMinInstances(int num) { m_minInstances = num; }
        int getMinInstances() const { return m_minInstances; }
        
        void setMaxInstances(int num) { m_maxInstances = num; }
        void setMaxInstances(const std::string &max) { m_maxInstances = (max == "unbounded") ? -1 : stoi(max); }
        int getMaxInstances() const { return m_maxInstances; }

        virtual void addType(const std::shared_ptr<CfgType> &pType);
        virtual const std::shared_ptr<CfgType> getType(const std::string &typeName);
        std::shared_ptr<CfgLimits> getStandardTypeLimits(const std::string &typeName) const;

        // virtual void addValueToValueSetType(const std::shared_ptr<CfgValue> &pValue, const std::string &setName);
        // virtual void addConfigValueSetType(const std::shared_ptr<ConfigItemValueSet> &pValueSet);
        // virtual void addConfigValue(const std::shared_ptr<CfgValue> &pValue) { m_configValues.push_back(pValue); };
        // virtual void addConfigValue(const std::vector<std::shared_ptr<CfgValue>> &configValues);

        virtual void addConfigType(const std::shared_ptr<ConfigItem> &pItem);  
        virtual const std::shared_ptr<ConfigItem> &getConfigType(const std::string &name) const;

        virtual void addChild(const std::shared_ptr<ConfigItem> &pItem) { m_children.push_back(pItem); }
        virtual void setItemCfgValue(const std::shared_ptr<CfgValue> &pValue) { m_pValue = pValue; }
        virtual std::shared_ptr<CfgValue> getItemCfgValue() const { return m_pValue; }

        virtual bool addKey(const std::string keyName);


    protected:

        // some kind of category map parent/child Software->ESP->[components]
        ConfigItem() { };

        std::string m_name;
        std::string m_displayName;
        std::string m_className;
        std::string m_category;  // used for further subdividing to the user
        std::vector<std::shared_ptr<ConfigItem>> m_children;
        std::shared_ptr<CfgValue> m_pValue;   // value for this itme (think of it as the value for an element <xx attr= att1=>value</xx>)
        std::set<std::string> m_keys;   // generic set of key values for use by any component to prevent duplicat operations
        std::weak_ptr<ConfigItem> m_pParent;

        std::map<std::string, std::shared_ptr<CfgType>> m_types;
        std::map<std::string, std::shared_ptr<ConfigItem>> m_configTypes;                // reusable types
        //std::map<std::string, std::shared_ptr<ConfigItemValueSet>> m_valueSetTypes;      // in XSD terms, an attribute group that can be referenced
        
        //std::vector<std::shared_ptr<ConfigItemValueSet>> m_valueSets;   // for a component this is the set of attributes by element

        int m_minInstances;
        int m_maxInstances;
        

    private:

        
        

};


#endif // _CONFIG2_CONFIGITEM_HPP_
