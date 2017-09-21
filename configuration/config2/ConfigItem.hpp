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


// this should be replaced with a generic environment item class
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>


//
// hpcc:class - what the item is
//              component - represents something that is configurable, addable, deletable, etc
//              category  - represents a hierarchial grouping in the configuration. A real deliniation (Hardware, Software)
//
// hpcc:category - Meaning differs based on class
//                 component - value is used to group together components into logical functional groups (esp services, logging agents, etc.) 
//                             does not represent a real division, a logical division



namespace pt = boost::property_tree;


class ConfigItemValueSet;


class ConfigItem 
{
    public:

    public:

        ConfigItem(const std::string &name, const std::string &className="category", std::shared_ptr<ConfigItem> pParent=nullptr) : 
            m_className(className), m_name(name), m_pParent(pParent), m_displayName(name), 
            m_minInstances(1), m_maxInstances(1), m_isConfigurable(false), m_version(-1)  { }
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

		void setVersion(int version) { m_version = version;  }
		int getVersion() const { return m_version; }

        virtual void addConfigType(const std::shared_ptr<ConfigItem> &pItem, const std::string &typeName);
        virtual const std::shared_ptr<ConfigItem> &getConfigType(const std::string &name) const;

        virtual void addChild(const std::shared_ptr<ConfigItem> &pItem) { m_children[pItem->getName()] = pItem; }
		virtual void addChild(const std::shared_ptr<ConfigItem> &pItem, const std::string &name) { m_children[name] = pItem; }
        virtual std::vector<std::shared_ptr<ConfigItem>> getChildren() const;
		template<typename T> std::shared_ptr<T> getChild(const std::string &name) const;
        virtual void setItemCfgValue(const std::shared_ptr<CfgValue> &pValue) { m_pValue = pValue; }
        virtual std::shared_ptr<CfgValue> getItemCfgValue() const { return m_pValue; }
		virtual void addAttribute(const std::shared_ptr<CfgValue> &pCfgValue);
		virtual void addAttribute(const std::vector<std::shared_ptr<CfgValue>> &attributes);
		virtual std::shared_ptr<CfgValue> getAttribute(const std::string &name) const;

        virtual bool addUniqueName(const std::string keyName);
        virtual void addKey(const std::string &keyName, const std::string &elementName, const std::string &attributeName);
        virtual void addKeyRef(const std::string &keyName, const std::string &elementName, const std::string &attributeName);

		//virtual void addEnvironmentInstance(const std::shared_ptr<EnvInstanceBase> &pInstance) { m_envInstances.push_back(pInstance); }

		bool isConfigurable() const { return m_isConfigurable; }


    protected:

       
        // some kind of category map parent/child Software->ESP->[components]
        ConfigItem() { };

        std::string m_name;
        std::string m_displayName;
        std::string m_className;
        std::string m_category;  // used for further subdividing to the user
		bool m_isConfigurable;
        std::map<std::string, std::shared_ptr<ConfigItem>> m_children; 
        std::shared_ptr<CfgValue> m_pValue;   // value for this item (think of it as the VALUE for an element <xx attr= att1=>VALUE</xx>)
		std::map<std::string, std::shared_ptr<CfgValue>> m_attributes;   // attributes for this item (thin in xml terms <m_name attr1="val" attr2="val" .../> where attrN is in this vector
        std::set<std::string> m_keys;   // generic set of key values for use by any component to prevent duplicat operations
        std::weak_ptr<ConfigItem> m_pParent;

        std::map<std::string, std::shared_ptr<CfgType>> m_types;
        std::map<std::string, std::shared_ptr<ConfigItem>> m_configTypes;                // reusable types
        
        std::map<std::string, std::shared_ptr<CfgValue>> m_keyDefs;

        int m_minInstances;
        int m_maxInstances;
		int m_version;
        

    private:

        
        

};


template<typename T> std::shared_ptr<T> ConfigItem::getChild(const std::string &name) const
{
	std::shared_ptr<T> pItem;
	auto it = m_children.find(name);
	if (it != m_children.end())
		pItem = std::dynamic_pointer_cast<T>(it->second);
	return pItem;
}

#endif // _CONFIG2_CONFIGITEM_HPP_
