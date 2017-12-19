#include "ws_config2Service.hpp"
#include "jfile.hpp"
#include "ConfigItem.hpp"

static const std::string CFG2_MASTER_CONFIG_FILE = "newenv.xsd";
static const std::string CFG2_CONFIG_DIR = COMPONENTFILES_DIR  "/config2xml/";
static const std::string CFG2_SOURCE_DIR = CFG2_CONFIG_DIR;
//static const std::string CFG2_SOURCE_DIR = CONFIG_SOURCE_DIR;

Cws_config2Ex::Cws_config2Ex()
{
    m_sessionKey = 0;  // for now and default to 0 in esdl defs 74523;  // some number
    //CONFIGURATOR_API::initialize();
    
    //std::shared_ptr<ConfigItem> pConfig = std::make_shared<ConfigItem>("root");
    //ConfigParser *pCfgParser = new XSDConfigParser("", pConfig);
    //m_envMgr.setConfig(pConfig);

    //ConfigMgrSession session("XML");
    //std::vector<std::string> cfgParms;
    //cfgParms.push_back("/opt/HPCCSystems/componentfiles/config2xml/");
    //cfgParms.push_back("newenv.xsd");
    //cfgParms.push_back("buildset.xml");
    //session.initializeSession(cfgParms);
    //session.loadEnvironment("/opt/HPCCSystems/componentfiles/config2xml/environment.xml");

    //m_pEnvMgr = session.m_pEnvMgr;

    /*m_pEnvMgr = getEnvironmentMgrInstance("XML");
    std::vector<std::string> cfgParms;
    cfgParms.push_back("newenv.xsd");
    cfgParms.push_back("/opt/HPCCSystems/componentfiles/config2xml/");
    cfgParms.push_back("buildset.xml");
    m_pEnvMgr->loadConfig(cfgParms);
    m_pEnvMgr->loadEnvironment("/opt/HPCCSystems/componentfiles/config2xml/environment.xml");*/

}


Cws_config2Ex::~Cws_config2Ex()
{
}


bool Cws_config2Ex::onopenSession(IEspContext &context, IEspOpenSessionRequest &req, IEspOpenSessionResponse &resp) 
{
    bool loaded = false;
    ConfigMgrSession newSession;

    std::string inputMasterFile = req.getMasterConfigFile();
    std::string inputConfigPath = req.getConfigPath();
    std::string inputSourcePath = req.getSourcePath();
    newSession.masterConfigFile = (inputMasterFile != "") ? req.getMasterConfigFile() : CFG2_MASTER_CONFIG_FILE;
    newSession.configType = req.getType();
    newSession.username = req.getUsername();
    newSession.configPath = (inputConfigPath != "") ? req.getConfigPath() : CFG2_CONFIG_DIR;
    newSession.sourcePath = (inputSourcePath != "") ? req.getSourcePath() : CFG2_SOURCE_DIR;
    newSession.activePath = req.getActivePath();


    std::vector<std::string> cfgParms;
    cfgParms.push_back("buildset.xml");
    if (newSession.initializeSession(cfgParms))
    {
        if (newSession.loadEnvironment("/opt/HPCCSystems/componentfiles/config2xml/environment.xml"))
        {
            std::string sessionId = std::to_string(m_sessionKey);
            resp.setSessionId(sessionId.c_str());
            resp.setError(false);
            m_sessions[sessionId] = newSession;
            m_sessionKey++;
        }
    }
    m_pEnvMgr = newSession.m_pEnvMgr;   // for cenvenience as the code is migrated to support sessions

    return true; 
}


bool Cws_config2Ex::oncloseSession(IEspContext &context, IEspCloseSessionRequest &req, IEspPassFailResponse &resp) { return true; }


bool Cws_config2Ex::ongetEnvironmentFileList(IEspContext &context, IEspGetEnvironmentListRequest &req, IEspGetEnvironmentListResponse &resp) 
{
    std::string sessionId = req.getSessionId();
    const ConfigMgrSession *pSession = getConfigSession(sessionId);
    Status status;

    bool rc = true;

    if (pSession)
    {
        IArrayOf<IEspenvironmentFileType> environmentFiles;
        Owned<IFile> pDir = createIFile(CFG2_SOURCE_DIR.c_str());
        if (pDir->exists())
        {
            Owned<IDirectoryIterator> it = pDir->directoryFiles(NULL, false, true);
            ForEach(*it)
            {
                StringBuffer filename;
                it->getName(filename);

                String str(filename.toLowerCase());
                if (str.endsWith(pSession->getEnvironmentFileExtension().c_str()))
                {            
                    Owned<IEspenvironmentFileType> pEnvFile = createenvironmentFileType();
                    pEnvFile->setFilename(filename.str());
                    environmentFiles.append(*pEnvFile.getLink());
                }
            }
            resp.setEnvironmentFiles(environmentFiles); 
        }
    }
    return rc;
}


bool Cws_config2Ex::onopenEnvironmentFile(IEspContext &context, IEspOpenEnvironmentFileRequest &req, IEspPassFailResponse &resp)
{
    return true;
}


bool Cws_config2Ex::onsaveEnvironmentFile(IEspContext &context, IEspSaveEnvironmentFileRequest &req, IEspPassFailResponse &resp)
{
    return true;
}


bool Cws_config2Ex::onenableEnvironmentChanges(IEspContext &context, IEspEnableChangesRequest &req, IEspPassFailResponse &resp)
{
    return true;
}


bool Cws_config2Ex::ongetNode(IEspContext &context, IEspNodeRequest &req, IEspGetNodeResponse &resp)
{
    std::string sessionId = req.getSessionId();
    std::string id = req.getNodeId();
    const ConfigMgrSession *pSession = getConfigSession(sessionId);
    Status status;

    bool rc;

    if (pSession)
    {
        EnvironmentMgr *pEnvMgr = pSession->m_pEnvMgr;
        std::shared_ptr<EnvironmentNode> pNode = pEnvMgr->getEnvironmentNode(id);
        
        if (pNode)
        {
            getNodelInfo(pNode, resp);
            
        }
        else
        {
            status.addStatusMsg(statusMsg::error, id.c_str(), "", "", "The input node ID is not a valid not in the environment");
            rc = false;
        }
    }
    else
    {
        status.addStatusMsg(statusMsg::error, id.c_str(), "", "", "The session ID is not valid");
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


bool Cws_config2Ex::oninsertNode(IEspContext &context, IEspInsertNodeRequest &req, IEspGetNodeResponse &resp)
{

    std::string sessionId = req.getSessionId();
    const ConfigMgrSession *pSession = getConfigSession(sessionId);
    Status status;

    bool rc = true;

    if (pSession)
    {
        std::string parentNodeId = req.getParentNodeId();
        std::shared_ptr<EnvironmentNode> pNode = pSession->m_pEnvMgr->getEnvironmentNode(parentNodeId);
        
        if (pNode)
        { 
            std::shared_ptr<EnvironmentNode> pNewNode = pSession->m_pEnvMgr->addNewEnvironmentNode(parentNodeId, req.getElementType(), status);
            if (pNewNode)
            {
                getNodelInfo(pNewNode, resp);
                resp.setNodeId(pNewNode->getId().c_str());
            }
        }
        else
        {
            status.addStatusMsg(statusMsg::error, sessionId.c_str(), "", "", "The input parent node ID is not a valid not in the environment");
            rc = false;
        }
    }
    else
    {
        status.addStatusMsg(statusMsg::error, sessionId.c_str(), "", "", "The session ID is not valid");
    }


    return true;
}


bool Cws_config2Ex::onremoveNode(IEspContext &context, IEspNodeRequest &req, IEspPassFailResponse &resp)
{
    std::string sessionId = req.getSessionId();
    const ConfigMgrSession *pSession = getConfigSession(sessionId);
    Status status;

    bool rc = true;

    if (pSession)
    {
        std::string nodeId = req.getNodeId();
        rc = pSession->m_pEnvMgr->removeEnvironmentNode(nodeId, status);
    }
    else
    {
        status.addStatusMsg(statusMsg::error, sessionId.c_str(), "", "", "The session ID is not valid");
    }
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

bool Cws_config2Ex::ongetParents(IEspContext &context, IEspNodeRequest &req, IEspGetParentsResponse &resp)
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


const ConfigMgrSession *Cws_config2Ex::getConfigSession(const std::string &sessionId) const
{
    const ConfigMgrSession *pSession = nullptr;
    auto it = m_sessions.find(sessionId);
    if (it != m_sessions.end())
        pSession = &(it->second);
    return pSession;
}


bool Cws_config2Ex::getNodelInfo(const std::shared_ptr<EnvironmentNode> &pNode, IEspGetNodeResponse &resp) const
{
    Status status;
    bool rc;
        
    const std::shared_ptr<ConfigItem> &pNodeConfigItem = pNode->getConfigItem();
    std::string nodeDisplayName = pNodeConfigItem->getDisplayName();
    IArrayOf<IEspnodeType> elements;

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
            pAttribute->setName(attributeName.c_str());
            pAttribute->setDisplayName(pCfgValue->getDisplayName().c_str());
            if (attributeName == "name" && pAttr->isValuePresent())
            {
                nodeDisplayName = pAttr->getValue();  // better usability value
            }

            pAttribute->updateDoc().setTooltip(pCfgValue->getTooltip().c_str());

            const std::shared_ptr<ConfigValueType> &pType = pCfgValue->getType();
            std::shared_ptr<ConfigTypeLimits> &pLimits = pType->getLimits();
            pAttribute->updateType().setName(pType->getName().c_str());
            pAttribute->updateType().updateLimits().setMin(pType->getLimits()->getMin());
            pAttribute->updateType().updateLimits().setMax(pType->getLimits()->getMax());
            pAttribute->setRequired(pCfgValue->isRequired());
            pAttribute->setReadOnly(pCfgValue->isReadOnly());
            pAttribute->setHidden(pCfgValue->isHidden());


            //StringArray excludeList;
            //excludeList.append("username");
            //pAttribute->updateType().updateLimits().setDisallowList(excludeList);

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

            nodeAttributes.append(*pAttribute.getLink());
        }
    }
    resp.setAttributes(nodeAttributes); 
    resp.setNodeName(nodeDisplayName.c_str());

    //
    // Now the children
    if (pNode->hasChildren())
    {
        std::vector<std::shared_ptr<EnvironmentNode>> children = pNode->getChildren();
        for (auto it=children.begin(); it!=children.end(); ++it)
        {
            std::shared_ptr<EnvironmentNode> pNode = *it;
            const std::shared_ptr<ConfigItem> pConfigItem = pNode->getConfigItem();
            Owned<IEspnodeType> pElement = createnodeType();
            pElement->updateElementInfo().setName(pConfigItem->getDisplayName().c_str());
            pElement->updateElementInfo().setElementType(pConfigItem->getItemType().c_str());
            pElement->updateElementInfo().setClass(pConfigItem->getClassName().c_str());
            pElement->updateElementInfo().setCategory(pConfigItem->getCategory().c_str());
            pElement->updateElementInfo().updateDoc().setTooltip("");
            pElement->setNodeId(pNode->getId().c_str());
            pElement->setNumChildren(pNode->getNumChildren());
            elements.append(*pElement.getLink());
        }
    }
    resp.setChildren(elements); 

    //
    // Build a list of items that can be inserted under this node
    IArrayOf<IEspelementInfoType> newElements;
    std::vector<std::shared_ptr<ConfigItem>> insertableList = pNode->getInsertableItems();
    for (auto it=insertableList.begin(); it!=insertableList.end(); ++it)
    {
        std::shared_ptr<ConfigItem> pConfigItem = *it;
        Owned<IEspelementInfoType> pNewElement = createelementInfoType();
        pNewElement->setName(pConfigItem->getDisplayName().c_str());
        pNewElement->setElementType(pConfigItem->getItemType().c_str());
        pNewElement->setClass(pConfigItem->getClassName().c_str());
        pNewElement->setCategory(pConfigItem->getCategory().c_str());
        pNewElement->setIsRequired(pConfigItem->isRequired());
        newElements.append(*pNewElement.getLink());
    }
    resp.setInsertable(newElements);


    if (pNodeConfigItem->isItemValueDefined())
    {
        resp.setNodeValueDefined(true);  

        const std::shared_ptr<ConfigValue> &pNodeCfgValue = pNodeConfigItem->getItemCfgValue();
        const std::shared_ptr<ConfigValueType> &pType = pNodeCfgValue->getType();
        resp.updateValue().updateType().setName(pType->getName().c_str());
        resp.updateValue().updateType().updateLimits().setMin(pType->getLimits()->getMin());
        resp.updateValue().updateType().updateLimits().setMax(pType->getLimits()->getMax());

        if (pNode->isNodeValueSet())
        {
            const std::shared_ptr<EnvironmentValue> &pNodeValue = pNode->getNodeEnvValue();
            resp.setNodeValueSet(true);
            resp.updateValue().setCurrentValue(pNodeValue->getValue().c_str());
        }
    }

    rc = true;
    
    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status);
    resp.updateStatus().setStatus(msgs); 
    resp.updateStatus().setError(status.isError());

    return rc;  
}