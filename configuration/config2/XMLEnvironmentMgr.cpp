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
#include "XSDConfigParser.hpp"


bool XMLEnvironmentMgr::createParser(const std::vector<std::string> &cfgParms)
{
    m_pConfigParser = std::make_shared<XSDConfigParser>(m_configPath, m_pConfig);
    return true;
}


bool XMLEnvironmentMgr::load(std::istream &in)
{
	pt::ptree envTree;

	pt::read_xml(in, envTree, pt::xml_parser::trim_whitespace | pt::xml_parser::no_comments);
	auto rootIt = envTree.begin();

	//
	// Start at root, these better match!
	std::string rootName = rootIt->first;
	//std::shared_ptr<EnvironmentNode> pRootNode;
	if (rootName == m_pConfig->getName())
	{
		m_pRootNode = std::make_shared<EnvironmentNode>(m_pConfig, rootName);
		m_pRootNode->setPath(".");
		addPath(m_pRootNode);
		parse(rootIt->second, m_pConfig, m_pRootNode);
	}

	return true;
}


void XMLEnvironmentMgr::save(std::ostream &out)
{
	pt::ptree envTree;

	serialize(envTree, m_pRootNode);

	pt::write_xml(out, envTree);
}


void XMLEnvironmentMgr::parse(const pt::ptree &envTree, const std::shared_ptr<ConfigItem> &pConfigItem, std::shared_ptr<EnvironmentNode> &pEnvNode)
{
	
	std::string value;
	try
	{
		value = envTree.get<std::string>("");
		if (value != "")
		{
			std::shared_ptr<CfgValue> pCfgValue;
			if (pConfigItem)
				pCfgValue = pConfigItem->getItemCfgValue();
			std::shared_ptr<EnvValue> pEnvValue = std::make_shared<EnvValue>(pEnvNode, "");  // node's value has no name
			pEnvValue->setCfgValue(pCfgValue);
			pEnvValue->setValue(value);
			pEnvNode->setNodeEnvValue(pEnvValue);
		}
	}
	catch (...)
	{
		// do nothing
	}

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
				std::shared_ptr<CfgValue> pCfgValue;
				std::shared_ptr<EnvValue> pEnvValue = std::make_shared<EnvValue>(pEnvNode, attrIt->first);   // this is where we would use a variant
				if (pConfigItem)
					pCfgValue = pConfigItem->getAttribute(attrIt->first);
				if (!pCfgValue)
				{
					pEnvNode->addStatus(NodeStatus::warning, "Attribute " + attrIt->first + " not defined in configuration schema, unable to validate value.");
				}
				pEnvValue->setCfgValue(pCfgValue);
				pEnvValue->setValue(attrIt->second.get_value<std::string>());
				pEnvNode->addAttribute(attrIt->first, pEnvValue);
			}
		}
		else
		{
			std::string typeName = it->second.get("<xmlattr>.buildSet", "");
			std::shared_ptr<ConfigItem> pEnvConfig;
			if (typeName != "")
			{
				if (pConfigItem)
					pEnvConfig = pConfigItem->getChild(typeName);

				if (!pEnvConfig)
				{
					pEnvNode->addStatus(NodeStatus::warning, "Specified schema " + typeName + " was not found");
				}
			}
			else
			{
				if (pConfigItem)
					pEnvConfig = pConfigItem->getChild(elemName);
			}

			//std::string value;
			//try
			//{
			//	value = it->second.get<std::string>("");
			//	if (value != "")
			//	{
			//		std::shared_ptr<CfgValue> pCfgValue;
			//		if (pConfigItem)
			//			pCfgValue = pConfigItem->getItemCfgValue();
			//		std::shared_ptr<EnvValue> pEnvValue = std::make_shared<EnvValue>("");  // node's value has no name
			//		pEnvValue->setCfgValue(pCfgValue);
			//		pEnvValue->setValue(value);
			//		pEnvNode->setNodeEnvValue(pEnvValue);
			//	}
			//}
			//catch (...)
			//{
			//	// do nothing
			//}

			
			//if (pEnvConfig)
			{
				std::shared_ptr<EnvironmentNode> pElementNode = std::make_shared<EnvironmentNode>(pEnvConfig, elemName, pEnvNode);
				pElementNode->setPath(getUniqueKey());
				addPath(pElementNode);
				parse(it->second, pEnvConfig, pElementNode);
				pEnvNode->addChild(pElementNode);
			}
			//else
			//{
				// this is where we need to generate a raw environment node that just passes thru environment stuff
			//}
		}
	}
}


void XMLEnvironmentMgr::serialize(pt::ptree &envTree, std::shared_ptr<EnvironmentNode> &pEnvNode) const
{
	std::vector<std::shared_ptr<EnvValue>> attributes = pEnvNode->getAttributes();
	for (auto attrIt = attributes.begin(); attrIt != attributes.end(); ++attrIt)
	{
		envTree.put("<xmlattr>." + (*attrIt)->getName(), (*attrIt)->getValue());
	}

	std::shared_ptr<EnvValue> pNodeValue = pEnvNode->getNodeEnvValue();
	if (pNodeValue)
	{
		envTree.put_value(pNodeValue->getValue()); 
	}

	std::vector<std::shared_ptr<EnvironmentNode>> children = pEnvNode->getChildren();
	for (auto childIt = children.begin(); childIt != children.end(); ++childIt)
	{
		pt::ptree nodeTree;
		serialize(nodeTree, *childIt);
		envTree.add_child((*childIt)->getName(), nodeTree);
	}
}