#include "EnvironmentNode.hpp"

void EnvironmentNode::addChild(std::shared_ptr<EnvironmentNode> pNode)
{
	m_children.insert(std::make_pair(pNode->getName(), pNode));
}


void EnvironmentNode::addAttribute(const std::string &name, std::shared_ptr<EnvValue> pValue)
{
	auto retValue = m_attributes.insert(std::make_pair(name, pValue));
	// todo: add check to make sure no duplicate attributes, use retValue to see if value was inserted or not
}


std::vector<std::shared_ptr<EnvironmentNode>> EnvironmentNode::getChildren(const std::string &name) const
{
	std::vector<std::shared_ptr<EnvironmentNode>> childNodes;
	if (name == "") {
		for (auto nodeIt = m_children.begin(); nodeIt != m_children.end(); ++nodeIt)
		{
			childNodes.push_back(nodeIt->second);
		}
	}
	else
	{
		auto rangeIt = m_children.equal_range(name);
		for (auto it = rangeIt.first; it != rangeIt.second; ++it)
		{
			childNodes.push_back(it->second);
		}
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
		std::shared_ptr<EnvValue> pEnvValue = std::make_shared<EnvValue>(shared_from_this(), name);
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


std::string EnvironmentNode::getAttributeValue(const std::string &name) const
{
	std::string value;
	std::shared_ptr<EnvValue> pAttribute = getAttribute(name);
	if (pAttribute)
		value = pAttribute->getValue();
	return value;
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
		
		m_pNodeValue = std::make_shared<EnvValue>(shared_from_this(), "");  // node's value has no name
		if (m_pConfigItem)
			m_pNodeValue->setCfgValue(m_pConfigItem->getItemCfgValue());
		rc = m_pNodeValue->setValue(value, force);
	}
	return rc;
}


bool EnvironmentNode::validate()
{
	//
	// Check node value
	if (m_pNodeValue)
	{
		m_pNodeValue->checkCurrentValue();
	}

	//
	// Check any attributes
	for (auto attrIt = m_attributes.begin(); attrIt != m_attributes.end(); ++attrIt)
	{
		attrIt->second->checkCurrentValue();
	}

	//
	// Now check all children
	for (auto childIt = m_children.begin(); childIt != m_children.end(); ++childIt)
	{
		childIt->second->validate();
	}

	return true;
}


std::vector<std::string> EnvironmentNode::getAllFieldValues(const std::string &fieldName) const
{
	std::vector<std::string> values;
	std::shared_ptr<EnvironmentNode> pParentNode = m_pParent.lock();
	if (pParentNode)
	{
		std::vector<std::shared_ptr<EnvironmentNode>> nodes = pParentNode->getChildren(m_name);
		for (auto it = nodes.begin(); it != nodes.end(); ++it)
		{
			values.push_back((*it)->getAttributeValue(fieldName));
		}
	}
	return values;
}


const std::shared_ptr<EnvValue> EnvironmentNode::getAttribute(const std::string &name) const
{
	std::shared_ptr<EnvValue> pValue;
	auto it = m_attributes.find(name);
	if (it != m_attributes.end())
	{
		pValue = it->second;
	}
	return pValue;
}