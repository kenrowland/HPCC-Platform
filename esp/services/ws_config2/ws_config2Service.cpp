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
    std::string id = req.getId();
    std::shared_ptr<EnvironmentNode> pNode = m_pEnvMgr->getEnvironmentNode(id);

    //
    // If the ID was bad, just return an error
    if (!pNode)
    {
        resp.setInputId(id.c_str());
        resp.updateStatus().setError(true);
        IArrayOf<IEspstatusMsgType> msgs;
        Owned<IEspstatusMsgType> pStatusMsg = createstatusMsgType();
        pStatusMsg->setNodeId(id.c_str());
        pStatusMsg->setMsg("The input node ID is not a valid node in the environment");
        msgs.append(*pStatusMsg.getLink());
        resp.updateStatus().setStatus(msgs);
        return false;
    }


    IArrayOf<IEspelementType> elements;
    
    //
    // Fill out the response headdre stuff
    resp.setInputId(id.c_str());

    //
    // Status object to hold all status for this node
    Status nodeStatus;

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
    std::map<std::string, std::shared_ptr<ConfigItem>> nodeConfigChildren = pNodeConfigItem->getChildren();
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
            pElement->setIdList(ids);
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
            pElement->setIdList(ids);
            pElement->setNumChildren(0);
            elements.append(*pElement.getLink());
        }
    }
    resp.setChildren(elements); 

    return true;  
}


bool Cws_config2Ex::onsetValues(IEspContext &context, IEspSetValuesRequest &req, IEspGetNodeResponse &resp)
{
    bool rc;
    std::string id = req.getId();
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
        resp.setInputId(id.c_str());
        status.addStatusMsg(statusMsg::error, id.c_str(), "", "", "The input node ID is not a valid not in the environment");
        rc = false;
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
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

    //
    // finalize the responsse
    resp.setInputId(id.c_str());
    resp.updateStatus().setStatus(msgs); 
    resp.updateStatus().setError(status.isError());
   
    return rc;
}


//bool Cws_config2Ex::mockInterface(const std::string &path, IEspGetNodeResponse &resp)
//{
//    return true;
//}
//     IArrayOf<IEspelementType> elements;
//     IArrayOf<IEspattributeType> attributes;

//     if (path == ".")
//     {
//         Owned<IEspelementType> pElement = createelementType();
//         pElement->updateItemInfo().setName("hw");
//         pElement->updateItemInfo().setDisplayName("Hardware Configuration");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("category");
//         pElement->setPath(".hw");
//         pElement->updateDoc().setTooltip("Defines the hardware configuration for the cluster");
//         pElement->setNumChildren(5);
//         pElement->setNumAllowedInstances(0);
//         pElement->setNumRequiredInstances(0);
//         elements.append(*pElement.getLink());

//         pElement.setown(createelementType());
//         pElement->updateItemInfo().setName("sw");
//         pElement->updateItemInfo().setDisplayName("Software Configuration");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("category");
//         pElement->setPath(".sw");
//         pElement->updateDoc().setTooltip("Defines the software configuration for the cluster");
//         pElement->setNumChildren(4);
//         pElement->setNumAllowedInstances(0);
//         pElement->setNumRequiredInstances(0);
//         elements.append(*pElement.getLink());
//     }
//     else if (path == ".sw")
//     {
//         Owned<IEspelementType> pElement = createelementType();
//         pElement->updateItemInfo().setName("esp");
//         pElement->updateItemInfo().setDisplayName("ESP Processes");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("instanceSet");
//         pElement->setPath(".sw.esp");
//         pElement->updateDoc().setTooltip("Defines ESP processes that run on various systems in the cluster");
//         pElement->setNumChildren(2);
//         pElement->setNumAllowedInstances(0);
//         pElement->setNumRequiredInstances(1);
//         elements.append(*pElement.getLink());

//         pElement.setown(createelementType());
//         pElement->updateItemInfo().setName("dropzones");
//         pElement->updateItemInfo().setDisplayName("Dropzones");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("instanceSet");
//         pElement->setPath(".sw.dropzones");
//         pElement->updateDoc().setTooltip("Define dropzones for file transfer activities");
//         pElement->setNumChildren(2);
//         pElement->setNumAllowedInstances(0);
//         pElement->setNumRequiredInstances(0);
//         elements.append(*pElement.getLink());

//         pElement.setown(createelementType());
//         pElement->updateItemInfo().setName("loggingagents");
//         pElement->updateItemInfo().setDisplayName("Logging Agents");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("instanceSet");
//         pElement->setPath(".sw.loggingagents");
//         pElement->updateDoc().setTooltip("Define logging agents for use across ESP and other services");
//         pElement->setNumChildren(0);
//         pElement->setNumAllowedInstances(0);
//         pElement->setNumRequiredInstances(0);
//         elements.append(*pElement.getLink());

//         pElement.setown(createelementType());
//         pElement->updateItemInfo().setName("xyzservice");
//         pElement->updateItemInfo().setDisplayName("XYZ Service");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("component");
//         pElement->setPath(".sw.xyzservice");
//         pElement->updateDoc().setTooltip("Configure XYZ service");
//         pElement->setNumChildren(3);
//         pElement->setNumAllowedInstances(0);
//         pElement->setNumRequiredInstances(1);
//         elements.append(*pElement.getLink());        

//     }
//     else if (path == ".sw.xyzservice")
//     {
//         Owned<IEspelementType> pElement = createelementType();

//         pElement->updateItemInfo().setName("otherstuff");
//         pElement->updateItemInfo().setDisplayName("Other Stuff");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("valueSet");
//         pElement->setPath(".sw.xyzservice.otherstuff");
//         pElement->updateDoc().setTooltip("Some other settings");
//         pElement->setNumChildren(4);
//         elements.append(*pElement.getLink());

//         pElement.setown(createelementType());
//         pElement->updateItemInfo().setName("instances");
//         pElement->updateItemInfo().setDisplayName("System Bindings");
//         pElement->updateItemInfo().setStatus("ok");
//         pElement->setClass("instanceSet");
//         pElement->setPath(".sw.xyzservice.instances");
//         pElement->updateDoc().setTooltip("Computer bindings for the xyz service (5 allowed)");
//         pElement->setNumChildren(0);  // current number defined (must compare with numXXXInstances below to see if can create or if any required)
//         pElement->setNumAllowedInstances(5);
//         pElement->setNumRequiredInstances(1);
//         elements.append(*pElement.getLink());

//         Owned<IEspattributeType> pAttribute = createattributeType();
//         pAttribute->updateItemInfo().setName("username");
//         pAttribute->updateItemInfo().setDisplayName("Username");
//         pAttribute->updateItemInfo().setStatus("ok");
//         pAttribute->updateDoc().setTooltip("Username for all operations");
//         pAttribute->updateType().setName("string");
//         pAttribute->updateType().updateLimits().setMin(5);
//         pAttribute->updateType().updateLimits().setMax(15);
//         StringArray excludeList;
//         excludeList.append("username");
//         pAttribute->updateType().updateLimits().setDisallowList(excludeList);
//         pAttribute->setCurrentValue("bobtheuser");
//         pAttribute->setDefaultValue("username");
//         attributes.append(*pAttribute.getLink());

//         pAttribute.setown(createattributeType());
//         pAttribute->updateItemInfo().setName("password");
//         pAttribute->updateItemInfo().setDisplayName("Password");
//         pAttribute->updateItemInfo().setStatus("ok");
//         pAttribute->updateDoc().setTooltip("User's password");
//         pAttribute->updateType().setName("string");
//         pAttribute->updateType().updateLimits().setMin(5);
//         pAttribute->updateType().updateLimits().setMax(15);
//         StringArray modifiers;
//         modifiers.append("mask");
//         modifiers.append("verify");
//         pAttribute->updateType().updateLimits().setDisallowList(excludeList);
//         attributes.append(*pAttribute.getLink());

//         pAttribute.setown(createattributeType());
//         pAttribute->updateItemInfo().setName("maxopenfiles");
//         pAttribute->updateItemInfo().setDisplayName("Max Open Files");
//         pAttribute->updateItemInfo().setStatus("ok");
//         pAttribute->updateDoc().setTooltip("Password for the user");
//         pAttribute->updateType().setName("integer");
//         pAttribute->updateType().updateLimits().setMin(1);
//         pAttribute->updateType().updateLimits().setMax(20);
//         pAttribute->setCurrentValue("6");
//         pAttribute->setDefaultValue("3");
//         attributes.append(*pAttribute.getLink());



//     }
    // else if (path == ".sw.xyzservice.attributes")
    // {
    //     Owned<IEspelementType> pElement = createelementType();
    //     pElement->updateItemInfo().setName("username");
    //     pElement->updateItemInfo().setDisplayName("Username");
    //     pElement->setClass("value");
    //     pElement->updateDoc().setTooltip("Username for all operations");
    //     pElement->updateElementData().updateType().setName("string");
    //     pElement->updateElementData().updateType().updateLimits().setMin(5);
    //     pElement->updateElementData().updateType().updateLimits().setMax(15);
    //     StringArray excludeList;
    //     excludeList.append("username");
    //     pElement->updateElementData().updateType().updateLimits().setDisallowList(excludeList);
    //     pElement->updateElementData().setCurrentValue("bobtheuser");
    //     pElement->updateElementData().setDefaultValue("username");
    //     elements.append(*pElement.getLink());

    //     pElement.setown(createelementType());
    //     pElement->updateItemInfo().setName("password");
    //     pElement->updateItemInfo().setDisplayName("Password");
    //     pElement->setClass("value");
    //     pElement->updateDoc().setTooltip("Password for the user");
    //     pElement->updateElementData().updateType().setName("string");
    //     pElement->updateElementData().updateType().updateLimits().setMin(5);
    //     pElement->updateElementData().updateType().updateLimits().setMax(15);
    //     StringArray modifiers;
    //     modifiers.append("password");
    //     pElement->updateElementData().updateType().setModifiers(modifiers);
    //     pElement->updateElementData().setCurrentValue("mypassword");
    //     pElement->updateElementData().setDefaultValue("");
    //     elements.append(*pElement.getLink());

    //     pElement.setown(createelementType());
    //     pElement->updateItemInfo().setName("maxopenfiles");
    //     pElement->updateItemInfo().setDisplayName("Max Open Files");
    //     pElement->setClass("value");
    //     pElement->updateDoc().setTooltip("Password for the user");
    //     pElement->updateElementData().updateType().setName("integer");
    //     pElement->updateElementData().updateType().updateLimits().setMin(1);
    //     pElement->updateElementData().updateType().updateLimits().setMax(20);
    //     pElement->updateElementData().setCurrentValue("6");
    //     pElement->updateElementData().setDefaultValue("3");
    //     elements.append(*pElement.getLink());


    // }


    // just add the elements we made
//     resp.setChildren(elements); 
//     resp.setAttributes(attributes); 
//     resp.setStatus("ok");

//     return(true);
// }


