

#include "OperationDeleteNode.hpp"
#include "EnvironmentMgr.hpp"
#include "TemplateExecutionException.hpp"

void OperationDeleteNode::doExecute(std::shared_ptr<EnvironmentMgr> pEnvMgr, std::shared_ptr<Variables> pVariables, const std::string &nodeId)
{
    std::string deleteNodeId = pVariables->doValueSubstitution(nodeId);
    pEnvMgr->removeEnvironmentNode(deleteNodeId);

    //
    // If any node IDs found, go delete them
    if (!m_parentNodeIds.empty())
    {
        for (auto &parentNodeId: m_parentNodeIds)
        {

        }
    }
    else if (m_throwOnEmpty)
    {
        throw TemplateExecutionException("No nodes selected for deletion");
    }
}
