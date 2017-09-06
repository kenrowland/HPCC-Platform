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

#include "XMLEnvironmentMgr.hpp"
#include "ConfigItemValueSet.hpp"


bool XMLEnvironmentMgr::load(std::istream &in)
{
	pt::ptree envTree;

	pt::read_xml(in, envTree, pt::xml_parser::no_comments);
	auto rootIt = envTree.begin();

	//
	// Start at root, these better match!
	std::string rootName = rootIt->first;
	//std::shared_ptr<EnvironmentNode> pRootNode;
	if (rootName == m_pConfig->getName())
	{
		m_pRootNode = std::make_shared<EnvironmentNode>(m_pConfig, rootName);
		addPath(getUniqueKey(), m_pRootNode);
		parse(rootIt->second, m_pConfig, m_pRootNode);
	}

	return true;
}


void XMLEnvironmentMgr::parse(const pt::ptree &envTree, const std::shared_ptr<ConfigItem> &pConfigItem, std::shared_ptr<EnvironmentNode> &pEnvNode)
{

	//
	// Find elements in environment tree cooresponding to this config item, then parse each
	for (auto it = envTree.begin(); it != envTree.end(); ++it)
	{
		std::string elemName = it->first;

		//
		// First see if there are attributes for this element (<xmlattr> === <element attr1="xx" attr2="yy" ...></element>  The attr1 and attr2 are in this)
		if (elemName == "<xmlattr>")
		{
			for (auto attrIt = it->second.begin(); attrIt != it->second.end(); ++attrIt)
			{
				std::shared_ptr<EnvValue> pEnvValue = std::make_shared<EnvValue>();   // this is where we would use a variant
				std::shared_ptr<CfgValue> pCfgValue = pConfigItem->getAttribute(attrIt->first);
				pEnvValue->setCfgValue(pCfgValue);
				pEnvValue->setValue(attrIt->second.get_value<std::string>());
				//pEnvNode->addInt(4, 5);
				pEnvNode->addValue(attrIt->first, pEnvValue);
			}
		}
		else
		{
			std::shared_ptr<ConfigItem> pEnvConfig = pConfigItem->getChild<ConfigItem>(elemName);
			if (pEnvConfig)
			{
				std::shared_ptr<EnvironmentNode> pElementNode = std::make_shared<EnvironmentNode>(pEnvConfig, elemName, pEnvNode);
				addPath(getUniqueKey(), pElementNode);
				parse(it->second, pEnvConfig, pElementNode);
				pEnvNode->addSubNode(pElementNode);
			}
			else
			{
				// this is where we need to generate a raw environment node that just passes thru environment stuff
			}
		}
	}
}