#include "ws_config2Service.hpp"
//#include "ConfiguratorAPI.hpp"
#include "jstring.hpp"
#include "ConfigItem.hpp"
#include "ws_config2Session.hpp"


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


bool Cws_config2Ex::onopenSession(IEspContext &context, IEspOpenSessionRequest &req, IEspOpenSessionResponse &resp) { return true; }
bool Cws_config2Ex::oncloseSession(IEspContext &context, IEspCloseSessionRequest &req, IEspPassFailResponse &resp) { return true; }
bool Cws_config2Ex::onsetEnvironmentConfig(IEspContext &context, IEspSetEnvironmentConfigRequest &req, IEspPassFailResponse &resp) { return true; }
bool Cws_config2Ex::ongetEnvironmentFileList(IEspContext &context, IEspGetEnvironmentListRequest &req, IEspGetEnvironmentListResponse &resp) { return true; }

bool Cws_config2Ex::ongetNode(IEspContext &context, IEspGetNodeRequest &req, IEspGetNodeResponse &resp)
{
    std::string id = req.getNodeId();
    std::shared_ptr<EnvironmentNode> pNode = m_pEnvMgr->getEnvironmentNode(id);
    Status status;
    bool rc;

    if (pNode)
    {
        const std::shared_ptr<ConfigItem> &pNodeConfigItem = pNode->getConfigItem();
        std::string nodeDisplayName = pNodeConfigItem->getDisplayName();
        resp.setNodeName(nodeDisplayName.c_str());
        IArrayOf<IEspelementType> elements;

        //
        // Handle the attributes
        IArrayOf<IEspattributeType> nodeAttributes;
        if (pNode->hasAttributes())
        {
            std::vector<std::shared_ptr<EnvironmentValue>> attributes = pNode->getAttributes();
            for (auto it=attributes.begin(); it!=attributes.end(); ++it)
            {
                std::shared_ptr<EnvironmentValue> pAttr = *it;
                Owned<IEspattributeType> pAttribute = createattributeType();
                
                const std::shared_ptr<ConfigValue> &pCfgValue = pAttr->getCfgValue();
                std::string attributeName = pAttr->getName();
                pAttribute->setName(pCfgValue->getDisplayName().c_str());
                if (attributeName == "name" && pAttr->isValueSet())
                {
                    nodeDisplayName = pAttr->getValue();
                }

                pAttribute->updateDoc().setTooltip(pCfgValue->getTooltip().c_str());
                
                const std::shared_ptr<ConfigValueType> &pType = pCfgValue->getType();
                std::shared_ptr<ConfigTypeLimits> &pLimits = pType->getLimits();
                pAttribute->updateType().setName(pType->getName().c_str());
                pAttribute->updateType().updateLimits().setMin(pType->getLimits()->getMin());
                pAttribute->updateType().updateLimits().setMax(pType->getLimits()->getMin());
                
                
                StringArray excludeList;
                //excludeList.append("username");
                pAttribute->updateType().updateLimits().setDisallowList(excludeList);

                //
                // when processing keyref not in a component, or anywhere really, use the xpath and attribute to find all
                // matches. to each add the keyref to the named attribute.
                // Then, isEnumerated, probably passing either the envionment node (pNode) or the attribute (pAttr), needs to
                // look and see if there is a keyref. If so, it is an enumerated list, use the attribute to get the list
                // of valid values (see the envValue validation method)

                std::vector<AllowedValue> allowedValues = pCfgValue->getAllowedValues(pAttr.get());
                if (!allowedValues.empty())
                {
                    IArrayOf<IEspchoiceType> choices;
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
                pAttribute->setDefaultValue(pCfgValue->getDefaultValue().c_str());
                pAttribute->setDefaultValid(pCfgValue->hasDefaultValue());
                
                
                nodeAttributes.append(*pAttribute.getLink());
            }
        }
        resp.setAttributes(nodeAttributes); 

        //
        // Now the children. Get all the possible configured children so that we will know if any are not
        // instantiated.
        //std::vector<std::shared_ptr<ConfigItem>> nodeConfigChildren = pNode->getConfigItem()->getChildren();
        if (pNode->hasChildren())
        {
            std::vector<std::shared_ptr<EnvironmentNode>> children = pNode->getChildren();
            for (auto it=children.begin(); it!=children.end(); ++it)
            {
                std::shared_ptr<EnvironmentNode> pNode = *it;
                const std::shared_ptr<ConfigItem> pConfigItem = pNode->getConfigItem();
                Owned<IEspelementType> pElement = createelementType();
                pElement->setName(pConfigItem->getDisplayName().c_str());
                pElement->setElementType(pConfigItem->getItemType().c_str());
                pElement->setClass(pConfigItem->getClassName().c_str());
                pElement->setCategory(pConfigItem->getCategory().c_str());
                pElement->setNumAllowedInstances(pConfigItem->getMaxInstances());
                pElement->setNumRequiredInstances(pConfigItem->getMinInstances());
                pElement->updateDoc().setTooltip("");
                pElement->setNodeId(pNode->getId().c_str());
                pElement->setNumChildren(pNode->getNumChildren());
                elements.append(*pElement.getLink());
            }

            
        }
        resp.setChildren(elements); 

        //
        // Build a list of items that can be inserted under this node
        IArrayOf<IEspnewElementType> newElements;
        std::vector<std::shared_ptr<ConfigItem>> insertableList = pNode->getInsertableItems();
        for (auto it=insertableList.begin(); it!=insertableList.end(); ++it)
        {
            std::shared_ptr<ConfigItem> pConfigItem = *it;
            Owned<IEspnewElementType> pNewElement = createnewElementType();
            pNewElement->setName(pConfigItem->getDisplayName().c_str());
            pNewElement->setElementType(pConfigItem->getItemType().c_str());
            pNewElement->setClass(pConfigItem->getClassName().c_str());
            pNewElement->setCategory(pConfigItem->getCategory().c_str());
            pNewElement->setIsRequired(pConfigItem->isRequired());
            newElements.append(*pNewElement.getLink());
        }
        resp.setNewElements(newElements);


        if (pNodeConfigItem->isItemValueDefined())
        {
            resp.setNodeValueDefined(true);  

            const std::shared_ptr<ConfigValue> &pNodeCfgValue = pNodeConfigItem->getItemCfgValue();
            const std::shared_ptr<ConfigValueType> &pType = pNodeCfgValue->getType();
            resp.updateValue().updateType().setName(pType->getName().c_str());
            resp.updateValue().updateType().updateLimits().setMin(pType->getLimits()->getMin());
            resp.updateValue().updateType().updateLimits().setMax(pType->getLimits()->getMin());

            if (pNode->isNodeValueSet())
            {
                const std::shared_ptr<EnvironmentValue> &pNodeValue = pNode->getNodeEnvValue();
                resp.setNodeValueSet(true);
                resp.updateValue().setCurrentValue(pNodeValue->getValue().c_str());
            }
       }

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