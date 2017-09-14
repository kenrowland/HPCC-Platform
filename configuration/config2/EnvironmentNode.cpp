#include "EnvironmentNode.hpp"

void EnvironmentNode::addSubNode(std::shared_ptr<EnvironmentNode> pNode)
{
	m_subNodes.insert(std::make_pair(pNode->getNodeName(), pNode));
}



void EnvironmentNode::addValue(const std::string &name, std::shared_ptr<EnvValue> pValue)
{
	auto retValue = m_values.insert(std::make_pair(name, pValue));
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