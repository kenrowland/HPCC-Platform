/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2017 HPCC Systems®.

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

#include "EnvironmentMgr.hpp"
#include "ConfigExceptions.hpp"
#include "XMLEnvironmentMgr.hpp"


EnvironmentMgr *getEnvironmentMgrInstance(const std::string &envType)
{
    EnvironmentMgr *pEnvMgr = NULL;
    if (envType == "XML")
    {
        //std::shared_ptr<ConfigParser> pCfgParser = std::make_shared<XSDConfigParser>(configPath, m_pConfig);
        pEnvMgr = new XMLEnvironmentMgr();
    }
    return pEnvMgr;
}


EnvironmentMgr::EnvironmentMgr() :
    m_key(0)
{
    m_pConfig = std::make_shared<ConfigItem>("root");  // make the root
}


bool EnvironmentMgr::loadConfig(const std::string &configPath, const std::string &masterConfigFile,  const std::vector<std::string> &cfgParms)  // todo: add a status object here for return
{
    bool rc = false;
    Status status;
    if (createParser(configPath, masterConfigFile, cfgParms))
    {
        rc = m_pConfigParser->parseEnvironmentConfig(configPath, masterConfigFile, cfgParms, status);
        if (rc)
        {
            m_pConfig->processUniqueAttributeValueSets();  // really a pre-post processing requirement
            m_pConfig->postProcessConfig();
        }
    }
    return rc;
}


bool EnvironmentMgr::loadEnvironment(const std::string &filename)
{
    std::ifstream in;
    std::string fpath = filename;
    
    in.open(fpath);
    if (in.is_open())
    {
        doLoadEnvironment(in);
    }
    return true;
}


void EnvironmentMgr::saveEnvironment(const std::string &filename, Status &status)
{
    std::ofstream out;

    out.open(filename);
    if (out.is_open())
    {
        save(out);
    }
}


void EnvironmentMgr::addPath(const std::shared_ptr<EnvironmentNode> pNode)
{
    auto retVal = m_nodeIds.insert({pNode->getId(), pNode });
    if (!retVal.second)
    {
        throw (ParseException("Attempted to insert duplicate path name " + pNode->getId() + " for node "));
    }
}


std::shared_ptr<EnvironmentNode> EnvironmentMgr::getEnvironmentNode(const std::string &nodeId)
{
    std::shared_ptr<EnvironmentNode> pNode;
    auto pathIt = m_nodeIds.find(nodeId);
    if (pathIt != m_nodeIds.end())
        pNode = pathIt->second;
    return pNode;
}



std::shared_ptr<EnvironmentNode> EnvironmentMgr::addNewEnvironmentNode(const std::string &parentNodeId, const std::string &elementType, Status &status)
{
    std::shared_ptr<EnvironmentNode> pNewNode;
    std::shared_ptr<EnvironmentNode> pParentNode = getEnvironmentNode(parentNodeId);
    if (pParentNode)
    {
        std::shared_ptr<ConfigItem> pNewCfgItem;
        std::vector<std::shared_ptr<ConfigItem>> insertableItems = pParentNode->getInsertableItems();  // configured items under the parent
        for (auto it = insertableItems.begin(); it != insertableItems.end(); ++it)
        {
            if ((*it)->getItemType() == elementType)
            {
                pNewNode = addNewEnvironmentNode(pParentNode, *it, status);
                break;
            }
        }
    }
    else
    {
        status.addStatusMsg(statusMsg::error, parentNodeId, "", "", "Unable to find indicated parent node");
    }
    return pNewNode;
}


std::shared_ptr<EnvironmentNode> EnvironmentMgr::addNewEnvironmentNode(const std::shared_ptr<EnvironmentNode> &pParentNode, const std::shared_ptr<ConfigItem> &pNewCfgItem, Status &status)
{
    std::shared_ptr<EnvironmentNode> pNewNode;
    
    //
    // Create the new node and add it to the parent
    pNewNode = std::make_shared<EnvironmentNode>(pNewCfgItem, pNewCfgItem->getName(), pParentNode);
    pNewNode->setId(getUniqueKey());
    pNewNode->addMissingAttributesFromConfig();
    pParentNode->addChild(pNewNode);
    addPath(pNewNode);

    //
    // Should we give it a name?
    if (pNewCfgItem->getNamePrefix() != "" && pNewNode->hasAttribute("name"))
    {
        std::string prefix = pNewCfgItem->getNamePrefix();
        std::string newName;
        std::vector<std::string> curNames = pNewNode->getAllFieldValues("name");
        size_t count = curNames.size();
        for (size_t n = 1; n <= count + 1; ++n)
        {
            newName = prefix + std::to_string(n);
            bool found = false;
            for (auto it = curNames.begin(); it != curNames.end() && !found; ++it)
            {
                if ((*it) == newName)
                    found = true;
            }

            if (!found)
            {
                pNewNode->setAttributeValue("name", newName, status);
                break;
            }
        }
    }

    //
    // Look through the children and add any that are necessary
    auto cfgChildren = pNewCfgItem->getChildren();
    for (auto childIt = cfgChildren.begin(); childIt != cfgChildren.end(); ++childIt)
    {
        unsigned numReq = (*childIt)->getMinInstances();
        for (int i=0; i<numReq; ++i)
        {
            addNewEnvironmentNode(pNewNode, *childIt, status);
        }
    }
    
    return pNewNode;
}


bool EnvironmentMgr::removeEnvironmentNode(const std::string &nodeId, Status &status)
{
    bool rc = false;
    std::shared_ptr<EnvironmentNode> pNode = getEnvironmentNode(nodeId);
    
    if (pNode)
    {
        std::shared_ptr<EnvironmentNode> pParentNode = pNode->getParent();
        if (pParentNode->removeChild(pNode))
        {
            m_nodeIds.erase(nodeId);
        }
        else
        {
            status.addStatusMsg(statusMsg::error, nodeId, "", "", "Unable to remove the node");
        }
    }
    else
    {
        status.addStatusMsg(statusMsg::error, nodeId, "", "", "Unable to find indicated node");
    }

    return rc;
}


std::string EnvironmentMgr::getUniqueKey()
{
    return std::to_string(m_key++);
}


void EnvironmentMgr::validate(Status &status) const
{
    if (m_pRootNode)
    {
        m_pRootNode->validate(status, true);
    }
}