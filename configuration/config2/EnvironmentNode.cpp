#include "EnvironmentNode.hpp"

void EnvironmentNode::addSubNode(std::shared_ptr<EnvironmentNode> pNode)
{
	m_subNodes.insert(std::make_pair(pNode->getNodeName(), pNode));
}


void EnvironmentNode::addAttribute(const std::string &name, std::shared_ptr<EnvValue> pValue)
{
	auto retValue = m_attributes.insert(std::make_pair(name, pValue));
	// todo: add check to make sure no duplicate attributes, use retValue to see if value was inserted or not
}


std::vector<std::shared_ptr<EnvironmentNode>> EnvironmentNode::getElements() const
{
	std::vector<std::shared_ptr<EnvironmentNode>> childNodes;
	for (auto nodeIt = m_subNodes.begin(); nodeIt != m_subNodes.end(); ++nodeIt)
	{
		childNodes.push_back(nodeIt->second);
	}
	return childNodes;
}


std::vector<std::shared_ptr<EnvValue>> EnvironmentNode::getAttributes() const
{
	std::vector<std::shared_ptr<EnvValue>> attributes;

	for (auto attrIt = m_attributes.begin(); attrIt != m_attributes.end(); ++attrIt)
	{
		attributes.push_back(attrIt->second);
	}
	return attributes;
}


bool EnvironmentNode::setAttributeValue(const std::string &name, const std::string &value, bool force)
{
	bool rc = false;
	auto it = m_attributes.find(name);
	if (it != m_attributes.end())
	{
		rc = it->second->setValue(value, force);
	}

	//
	// This is a non-defined attribute. If force is true, create a new non-config backed attribute
	else if (force)
	{
		std::shared_ptr<EnvValue> pEnvValue = std::make_shared<EnvValue>(name);
		addStatus(NodeStatus::warning, "Attribute " + name + " not defined in configuration schema, unable to validate value.");
		pEnvValue->setValue(value);
		addAttribute(name, pEnvValue);
		rc = false;
	}
	else
	{
		// todo: a message here that value does not exist and force was not set?
		rc = false;
	}

	return rc;
}


bool EnvironmentNode::setValue(const std::string &value, bool force)
{
	bool rc = false;

	if (m_pNodeValue)
	{
		rc = m_pNodeValue->setValue(value, force);
	}
	else
	{
		std::shared_ptr<CfgValue> pCfgValue;
		
		m_pNodeValue = std::make_shared<EnvValue>("");  // node's value has no name
		if (m_pConfigItem)
			m_pNodeValue->setCfgValue(m_pConfigItem->getItemCfgValue());
		rc = m_pNodeValue->setValue(value, force);
	}
	return rc;
}