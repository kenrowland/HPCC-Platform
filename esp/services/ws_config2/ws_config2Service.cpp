#include "ws_config2Service.hpp"
//#include "ConfiguratorAPI.hpp"
#include "jstring.hpp"
#include "ConfigItem.hpp"


Cws_config2Ex::Cws_config2Ex()
{
    //CONFIGURATOR_API::initialize();
    
    //std::shared_ptr<ConfigItem> pConfig = std::make_shared<ConfigItem>("root");
    //ConfigParser *pCfgParser = new XSDConfigParser("", pConfig);
    //m_envMgr.setConfig(pConfig);

    m_pEnvMgr = getEnvironmentMgrInstance("XML", "/opt/HPCCSystems/componentfiles/config2xml/");
    std::vector<std::string> cfgParms;
    cfgParms.push_back("newenv.xsd");
    cfgParms.push_back("buildset.xml");
    m_pEnvMgr->loadConfig(cfgParms);
    m_pEnvMgr->loadEnvironment("environment.xml");

}


Cws_config2Ex::~Cws_config2Ex()
{
}


bool Cws_config2Ex::ongetNode(IEspContext &context, IEspGetNodeRequest &req, IEspGetNodeResponse &resp)
{
    std::string id = req.getNodeId();
    std::shared_ptr<EnvironmentNode> pNode = m_pEnvMgr->getEnvironmentNode(id);
    Status status;
    bool rc;

    if (pNode)
    {
        IArrayOf<IEspelementType> elements;

        //
        // Handle the attributes
        IArrayOf<IEspattributeType> nodeAttributes;
        if (pNode->hasAttributes())
        {
            std::vector<std::shared_ptr<EnvValue>> attributes = pNode->getAttributes();
            for (auto it=attributes.begin(); it!=attributes.end(); ++it)
            {
                std::shared_ptr<EnvValue> pAttr = *it;
                Owned<IEspattributeType> pAttribute = createattributeType();
                
                const std::shared_ptr<CfgValue> &pCfgValue = pAttr->getCfgValue();
                pAttribute->updateItemInfo().setName(pAttr->getName().c_str());
                pAttribute->updateItemInfo().setDisplayName(pCfgValue->getDisplayName().c_str());
                pAttribute->updateDoc().setTooltip(pCfgValue->getTooltip().c_str());
                
                const std::shared_ptr<CfgType> &pType = pCfgValue->getType();
                std::shared_ptr<CfgLimits> &pLimits = pType->getLimits();
                pAttribute->updateType().setName(pType->getName().c_str());
                pAttribute->updateType().updateLimits().setMin(pType->getLimits()->getMin());
                pAttribute->updateType().updateLimits().setMax(pType->getLimits()->getMin());
                
                
                StringArray excludeList;
                //excludeList.append("username");
                pAttribute->updateType().updateLimits().setDisallowList(excludeList);

                if (pType->isEnumerated())
                {
                    IArrayOf<IEspchoiceType> choices;
                    const std::vector<AllowedValue> &allowedValues = pType->getAllowedValues();
                    for (auto valueIt=allowedValues.begin(); valueIt!=allowedValues.end(); ++valueIt)
                    {
                        Owned<IEspchoiceType> pChoice = createchoiceType();
                        pChoice->setName((*valueIt).m_value.c_str());
                        pChoice->setDesc((*valueIt).m_description.c_str());
                        choices.append(*pChoice.getLink());
                    }
                    pAttribute->updateType().updateLimits().setChoiceList(choices);
                }

                pAttribute->setCurrentValue(pAttr->getValue().c_str());
                pAttribute->setDefaultValue("");
                
                
                nodeAttributes.append(*pAttribute.getLink());
            }
        }
        resp.setAttributes(nodeAttributes); 

        //
        // Now the children
        const std::shared_ptr<ConfigItem> &pNodeConfigItem = pNode->getConfigItem();
        std::multimap<std::string, std::shared_ptr<ConfigItem>> nodeConfigChildren = pNodeConfigItem->getChildren();
        if (pNode->hasChildren())
        {
            std::map<std::string, std::vector<std::shared_ptr<EnvironmentNode>>> children = pNode->getChildrenByName();
            for (auto it=children.begin(); it!=children.end(); ++it)
            {
                const std::vector<std::shared_ptr<EnvironmentNode>> &nodes = it->second;
                const std::shared_ptr<ConfigItem> pConfigItem = nodes[0]->getConfigItem();
                Owned<IEspelementType> pElement = createelementType();
                pElement->updateItemInfo().setName(it->first.c_str());
                pElement->updateItemInfo().setDisplayName(pConfigItem->getDisplayName().c_str());
                pElement->setClass(pConfigItem->getClassName().c_str());
                pElement->setCategory(pConfigItem->getCategory().c_str());
                pElement->setNumAllowedInstances(pConfigItem->getMaxInstances());
                pElement->setNumRequiredInstances(pConfigItem->getMinInstances());
                pElement->updateDoc().setTooltip("");
                

                StringArray ids;
                int numChildren = 0;
                for (auto nodeIt=nodes.begin(); nodeIt!=nodes.end(); ++nodeIt)
                {
                    ids.append((*nodeIt)->getId().c_str());
                    numChildren += (*nodeIt)->getNumChildren();
                }
                pElement->setNumChildren(numChildren);
                pElement->setNodeIdList(ids);
                elements.append(*pElement.getLink());

                //
                // Remove the node we just added from the node's configuration children because at least one is configured. Note
                // that a previous iteration of this loop may have already removed it in the case where there is more than one
                // occurance of a node in the environment for a config item
                nodeConfigChildren.erase(it->first);
            }

            //
            // Now add any configuration elements that don't have instantiations in the environment. These all get empty id lists
            // indicating that they don't exist yet, but can be added
            for (auto it=nodeConfigChildren.begin(); it!=nodeConfigChildren.end(); ++it)
            {
                std::shared_ptr<ConfigItem> pConfigItem = it->second;
                Owned<IEspelementType> pElement = createelementType();
                pElement->updateItemInfo().setName(pConfigItem->getName().c_str());
                pElement->updateItemInfo().setDisplayName(pConfigItem->getDisplayName().c_str());
                pElement->setClass(pConfigItem->getClassName().c_str());
                pElement->setCategory(pConfigItem->getCategory().c_str());
                pElement->setNumAllowedInstances(pConfigItem->getMaxInstances());
                pElement->setNumRequiredInstances(pConfigItem->getMinInstances());
                pElement->updateDoc().setTooltip("");
                
                StringArray ids;
                pElement->setNodeIdList(ids);
                pElement->setNumChildren(0);
                elements.append(*pElement.getLink());
            }
        }
        resp.setChildren(elements); 
        rc = true;
    }
    else
    {
        status.addStatusMsg(statusMsg::error, id.c_str(), "", "", "The input node ID is not a valid not in the environment");
        rc = false;
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status);
    resp.updateStatus().setStatus(msgs); 
    resp.updateStatus().setError(status.isError());

    //
    // Finalize the response
    resp.setNodeId(id.c_str());

    return rc;  
}


bool Cws_config2Ex::onsetValues(IEspContext &context, IEspSetValuesRequest &req, IEspSetValuesResponse &resp)
{
    bool rc;
    std::string id = req.getNodeId();
    std::shared_ptr<EnvironmentNode> pNode = m_pEnvMgr->getEnvironmentNode(id);
    Status status;
    if (pNode)
    {
        bool forceCreate = req.getForceCreate();
        bool allowInvalid = req.getAllowInvalid();
        IArrayOf<IConstattributeValueType> &attrbuteValues = req.getAttributeValues();
        std::vector<ValueDef> values;
        ForEachItemIn(i, attrbuteValues)
        {
            IConstattributeValueType& attrVal = attrbuteValues.item(i);
            ValueDef value;
            value.name = attrVal.getName();
            value.value = attrVal.getValue();
            values.push_back(value);
        }
        pNode->setAttributeValues(values, status, allowInvalid, forceCreate);
        rc = true;
    }
    else
    {
        status.addStatusMsg(statusMsg::error, id.c_str(), "", "", "The input node ID is not a valid not in the environment");
        rc = false;
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status);
    resp.updateStatus().setStatus(msgs); 
    resp.updateStatus().setError(status.isError());

    //
    // finalize the responsse
    resp.setNodeId(id.c_str());
   
    return rc;
}

bool Cws_config2Ex::ongetParents(IEspContext &context, IEspGetParentsRequest &req, IEspGetParentsResponse &resp)
{
    std::string id = req.getNodeId();
    std::shared_ptr<EnvironmentNode> pNode = m_pEnvMgr->getEnvironmentNode(id);
    Status status;
    bool rc;

    if (pNode)
    {
        StringArray ids;
        while (pNode)
        {
            pNode = pNode->getParent();
            if (pNode)
            {
                ids.append(pNode->getId().c_str());
            }
        }
        resp.setParentIdList(ids);
        rc = true;
    }
    else
    {
        status.addStatusMsg(statusMsg::error, id.c_str(), "", "", "The input node ID is not a valid not in the environment");
        rc = false;
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status);
    resp.updateStatus().setStatus(msgs); 
    resp.updateStatus().setError(status.isError());

    //
    // finalize the responsse
    resp.setNodeId(id.c_str()); 
    
    return rc;
}

void Cws_config2Ex::buildStatusMessageObject(IArrayOf<IEspstatusMsgType> &msgs, const Status &status) const
{
    std::vector<statusMsg> statusMsgs = status.getMessages();
    for (auto msgIt=statusMsgs.begin(); msgIt!=statusMsgs.end(); ++msgIt)
    {
        Owned<IEspstatusMsgType> pStatusMsg = createstatusMsgType();
        pStatusMsg->setNodeId((*msgIt).nodeId.c_str());
        pStatusMsg->setMsg((*msgIt).msg.c_str());
        pStatusMsg->setMsgLevel(status.getStatusTypeString((*msgIt).msgLevel).c_str());
        pStatusMsg->setRefNodeId((*msgIt).referNodeId.c_str());
        pStatusMsg->setAttribute((*msgIt).attribute.c_str());
        msgs.append(*pStatusMsg.getLink());
    }
}