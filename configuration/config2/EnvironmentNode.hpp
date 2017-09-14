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

#ifndef _CONFIG2_ENVIRONMENTNODE_HPP_
#define _CONFIG2_ENVIRONMENTNODE_HPP_

#include <memory>
#include <map>
#include "ConfigItem.hpp"
#include "EnvValue.hpp"
#include "CfgValue.hpp"

class EnvironmentNode //: std::enable_shared_from_this<EnvironmentNode>
{
	public:

		EnvironmentNode(const std::shared_ptr<ConfigItem> &pCfgItem, const std::string &elemName, const std::shared_ptr<EnvironmentNode> &pParent = nullptr) : 
			m_pConfigItem(pCfgItem), m_elementName(elemName), m_pParent(pParent) { }
		~EnvironmentNode() { }
		const std::string &getNodeName() const { return m_elementName;  }
		void addSubNode(std::shared_ptr<EnvironmentNode> pNode);
		std::vector<std::shared_ptr<EnvironmentNode>> getElements() const;
		void addValue(const std::string &name, std::shared_ptr<EnvValue> pValue);
		std::map<std::string, std::shared_ptr<EnvValue>> getValues() const { return m_values; }
		bool hasValues() const { return m_values.size() != 0; }
		void setPath(const std::string &path) { m_path = path; } 
		const std::string &getPath() const { return m_path;  }


	protected:

		std::string m_elementName;   // may also be stored in m_values, so keep synched
		std::shared_ptr<ConfigItem> m_pConfigItem;
		std::weak_ptr<EnvironmentNode> m_pParent;
		std::multimap<std::string, std::shared_ptr<EnvironmentNode>> m_subNodes;
		std::shared_ptr<CfgValue> m_nodeValue;
		std::map<std::string, std::shared_ptr<EnvValue>> m_values;
		std::string m_path;
};


#endif