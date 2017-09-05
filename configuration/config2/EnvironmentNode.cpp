#include "EnvironmentNode.hpp"

void EnvironmentNode::addSubNode(const std::string &nodeName, std::shared_ptr<EnvironmentNode> pNode)
{
	m_subNodes.insert(std::make_pair(nodeName, pNode));
}



void EnvironmentNode::addValue(const std::string &name, std::shared_ptr<EnvValue> pValue)
{
	auto retValue = m_values.insert({ name, pValue });
}
