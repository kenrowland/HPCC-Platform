#include "ws_config2Service.hpp"
#include "jfile.hpp"
#include "SchemaItem.hpp"

static const std::string CFG2_MASTER_CONFIG_FILE = "environment.xsd";
static const std::string CFG2_CONFIG_DIR = COMPONENTFILES_DIR  "/config2xml/";
static const std::string CFG2_SOURCE_DIR = CONFIG_SOURCE_DIR;
static const std::string ACTIVE_ENVIRONMENT_FILE = CONFIG_DIR "/" ENV_XML_FILE;

Cws_config2Ex::Cws_config2Ex()
{
    m_sessionKey = 0;  // for now and default to 0 in esdl defs 74523;  // some number
}


Cws_config2Ex::~Cws_config2Ex()
{
}


bool Cws_config2Ex::onopenSession(IEspContext &context, IEspOpenSessionRequest &req, IEspOpenSessionResponse &resp)
{
    bool loaded = false;
    ConfigMgrSession *pNewSession = new ConfigMgrSession();

    std::string inputMasterFile = req.getMasterSchemaFile();
    std::string inputSchemaPath = req.getSchemaPath();
    std::string inputSourcePath = req.getSourcePath();
    std::string inputActivePath = req.getSourcePath();
    pNewSession->masterConfigFile = (inputMasterFile != "") ? inputMasterFile : CFG2_MASTER_CONFIG_FILE;
    pNewSession->username = req.getUsername();
    pNewSession->schemaPath = !inputSchemaPath.empty() ? inputSchemaPath : CFG2_CONFIG_DIR;
    pNewSession->sourcePath = !inputSourcePath.empty() ? inputSourcePath : CFG2_SOURCE_DIR;
    pNewSession->activePath = !inputActivePath.empty() ? inputActivePath : ACTIVE_ENVIRONMENT_FILE;

    std::string cfgType = req.getType();
    if (cfgType == "XML")
    {
        pNewSession->configType = XML;
    }

    //
    // Open the session by loading the schema, which is done during session init
    std::vector<std::string> cfgParms;
    cfgParms.push_back("buildset.xml");  // Note that this is hardcoded for now, when other types suppored, must be passed in
    if (pNewSession->initializeSession(cfgParms))
    {
        std::string sessionId = std::to_string(m_sessionKey);
        resp.setSessionId(sessionId.c_str());
        resp.setError(false);
        m_sessions[sessionId] = pNewSession;
        m_sessionKey++;
    }
    else
    {
        delete pNewSession;
        resp.setError(true);
        resp.setMsg(pNewSession->getLastMsg().c_str());
    }
    return true;
}


bool Cws_config2Ex::oncloseSession(IEspContext &context, IEspCloseSessionRequest &req, IEspPassFailResponse &resp)
{
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSession(sessionId);

    bool rc = true;

    if (pSession)
    {
        bool doClose = true;
        //
        // See if modified
        if (pSession->modified)
        {
            if (!req.getForceClose())
            {
                resp.setError(true);
                resp.setMsg("Current file has been modified, close refused (set forceClose to force close)");
                doClose = false;
            }
        }

        if (doClose)
        {
            deleteConfigSession(sessionId);
        }
    }
    else
    {
        resp.setError(true);
        resp.setMsg("The session ID is not valid");
    }
    return rc;
}


bool Cws_config2Ex::ongetEnvironmentFileList(IEspContext &context, IEspCommonSessionRequest &req, IEspGetEnvironmentListResponse &resp)
{
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSession(sessionId);

    if (pSession)
    {
        //
        // Calculate md5 checksum of the current active config
        StringBuffer activeConfig_md5sum;
        md5_filesum(pSession->activePath.c_str(), activeConfig_md5sum);

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

                    //
                    // See if active
                    StringBuffer curEnvFile_md5sum;
                    std::string fullPath;
                    std::string fname = filename.str();
                    pSession->getEnvironmentFullyQualifiedPath(fname, fullPath);
                    md5_filesum(fullPath.c_str(), curEnvFile_md5sum);
                    if (strcmp(curEnvFile_md5sum.str(),activeConfig_md5sum.str()) == 0)
                    {
                        pEnvFile->setIsActive(true);
                    }
                    environmentFiles.append(*pEnvFile.getLink());
                }
            }
            resp.setEnvironmentFiles(environmentFiles);
        }
    }
    else
    {
        resp.setError(true);
        resp.setMsg("The session ID is not valid");
    }
    return true;
}


bool Cws_config2Ex::onopenEnvironmentFile(IEspContext &context, IEspOpenEnvironmentFileRequest &req, IEspPassFailResponse &resp)
{
    bool doOpen = false;
    ConfigMgrSession *pSession = getConfigSession(req.getSessionId());
    if (pSession)
    {
        doOpen = true;

        //
        // See if modified (which can only be true if an environment is currently loaded)
        if (pSession->modified)
        {
            resp.setError(true);
            resp.setMsg("Current environment has been modified, eaither save or close it first.");
            doOpen = false;
        }

        if (doOpen)
        {
            std::string newEnvFile = req.getFilename();
            if (!pSession->loadEnvironment(newEnvFile))
            {
                resp.setError(true);
                resp.setMsg(pSession->getLastMsg().c_str());
            }
        }
    }
    else
    {
        resp.setError(true);
        resp.setMsg("The session ID is not valid");
    }
    return true;
}


bool Cws_config2Ex::oncloseEnvironmentFile(IEspContext &context, IEspCloseEnvironmentFileRequest &req, IEspPassFailResponse &resp)
{
    bool doClose = false;
    ConfigMgrSession *pSession = getConfigSession(req.getSessionId());
    if (pSession)
    {
        doClose = true;   // assume we are moving ahead with the open

        //
        // See if modified
        if (pSession->modified)
        {
            // todo, only allow discard if session lock key matches

            //
            // Code here to make sure a modifed file is only in this session (not someone else)
            if (!req.getDiscardChanges())
            {
                resp.setError(true);
                resp.setMsg("Current file has been modified, close refused (set discardChanges to force close)");
                doClose = false;
            }
        }

        if (doClose)
        {
            pSession->closeEnvironment();
        }
    }
    else
    {
        resp.setError(true);
        resp.setMsg("The session ID is not valid");
    }
    return true;
}


bool Cws_config2Ex::onsaveEnvironmentFile(IEspContext &context, IEspSaveEnvironmentFileRequest &req, IEspPassFailResponse &resp)
{
    // todo: If this envronment file is loaded by any other session, go mark that session that the environment has changed
    // and don't allow a save w/o reloading first. maybe add a reload request. Add relevant errors.
    // also make sure if filename is specified that it
    //  1. does not match the current loaded environment if current environment is modified and lock key is missing or does not match
    //  2. does not match the name of any other open session's loaded and locked environment.
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSession(sessionId);
    if (pSession)
    {
        std::string saveFilename = req.getFilename();

        //
        // Conditions where a save is done:
        //   - A file name is provided (save as). The current environment, even if changed, is saved to the
        //     new file. The session converts to the newly saved environment and is marked unlocked and un modified
        //   - No filename is provided
        //     - If modified, then the lockKey must be provided
        //     - If not modified, this is a noop
        if (saveFilename != "" || (pSession->modified && pSession->doesKeyFit(req.getSessionLockKey())))
        {
            if (!pSession->saveEnvironment(saveFilename))
            {
                resp.setError(true);
                resp.setMsg("There was a problem saving the environment");
            }
        }
    }
    else
    {
        resp.setError(true);
        resp.setMsg("The session ID is not valid");
    }
    return true;
}


bool Cws_config2Ex::onlockSession(IEspContext &context, IEspCommonSessionRequest &req, IEspLockSessionResponse &resp)
{
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSession(sessionId);
    if (pSession && !pSession->locked)
    {
        if (pSession->curEnvironmentFile.empty())
        {
            resp.setError(true);
            resp.setMsg("You must load an environment before locking.");
        }
        else
        {
            bool okToLock = true;
            //
            // Search existing sessions to see if any currently have this environment file locked
            for (auto sessionIt = m_sessions.begin(); sessionIt != m_sessions.end(); ++sessionIt)
            {
                if (sessionIt->second != pSession && sessionIt->second->locked && pSession->curEnvironmentFile == sessionIt->second->curEnvironmentFile)
                {
                    okToLock = false;
                    resp.setError(true);
                    resp.setMsg("The environment is locked by another session");
                }
            }

            if (okToLock)
            {
                if (pSession->lock())
                {
                    resp.setSessionLockKey(pSession->lockKey.c_str());
                }
                else
                {
                    resp.setError(true);
                    resp.setMsg("There was a problem locking the session (already locked?)");
                }
            }
        }
    }
    else
    {
        resp.setError(true);
        resp.setMsg("The session ID is not valid or is alredy locked");
    }
    return true;
}


bool Cws_config2Ex::onunlockSession(IEspContext &context, IEspUnlockSessionRequest &req, IEspPassFailResponse &resp)
{
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSession(sessionId);
    if (pSession)
    {
        std::string unlockKey = req.getSessionLockKey();
        if (!pSession->unlock(unlockKey))
        {
            resp.setError(true);
            resp.setMsg("There was a problem unlocking the session (key mismatch?)");
        }
    }
    else
    {
        resp.setError(true);
        resp.setMsg("The session ID is not valid");
    }
    return true;
}


bool Cws_config2Ex::ongetNode(IEspContext &context, IEspNodeRequest &req, IEspGetNodeResponse &resp)
{
    std::string sessionId = req.getSessionId();
    std::string id = req.getNodeId();
    ConfigMgrSession *pSession = getConfigSession(sessionId);
    Status status;

    if (pSession)
    {
        EnvironmentMgr *pEnvMgr = pSession->m_pEnvMgr;
        std::shared_ptr<EnvironmentNode> pNode = pEnvMgr->getEnvironmentNode(id);

        if (pNode)
        {
            getNodelInfo(pNode, resp);
            pNode->validate(status, false);  // validate this node only
        }
        else
        {
            status.addMsg(statusMsg::error, id.c_str(), "", "The input node ID is not a valid not in the environment");
            resp.setError(true);
        }
    }
    else
    {
        status.addMsg(statusMsg::error, id.c_str(), "", "The session ID is not valid");
        resp.setError(true);
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status, pSession);
    resp.updateStatus().setStatus(msgs);
    resp.updateStatus().setError(status.isError());

    //
    // Finalize the response
    resp.setNodeId(id.c_str());

    return true;
}


bool Cws_config2Ex::oninsertNode(IEspContext &context, IEspInsertNodeRequest &req, IEspGetNodeResponse &resp)
{
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSessionForUpdate(sessionId, req.getSessionLockKey());
    Status status;

    if (pSession)
    {
        std::string parentNodeId = req.getParentNodeId();
        std::shared_ptr<EnvironmentNode> pNode = pSession->m_pEnvMgr->getEnvironmentNode(parentNodeId);

        if (pNode)
        {
            std::shared_ptr<EnvironmentNode> pNewNode = pSession->m_pEnvMgr->addNewEnvironmentNode(parentNodeId, req.getNodeType(), status);
            if (pNewNode)
            {
                getNodelInfo(pNewNode, resp);
                resp.setNodeId(pNewNode->getId().c_str());
                pSession->modified = true;
            }
        }
        else
        {
            status.addMsg(statusMsg::error, sessionId.c_str(), "", "The input parent node ID is not a valid not in the environment");
            resp.setError(true);
        }
    }
    else
    {
        status.addMsg(statusMsg::error, sessionId.c_str(), "", "The session ID is not valid, session is not locked, or the key is not correct");
        resp.setError(true);
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status, pSession);
    resp.updateStatus().setStatus(msgs);
    resp.updateStatus().setError(status.isError());
    return true;
}


bool Cws_config2Ex::onremoveNode(IEspContext &context, IEspRemoveNodeRequest &req, IEspPassFailResponse &resp)
{
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSessionForUpdate(sessionId, req.getSessionLockKey());
    Status status;

    if (pSession)
    {
        std::string nodeId = req.getNodeId();
        bool rc = pSession->m_pEnvMgr->removeEnvironmentNode(nodeId);
        resp.setError(!rc);
        pSession->modified = true;
        if (!rc)
        {
            resp.setMsg("There was a problem removing the node (invalid nodeID?)");
            resp.setError(true);
        }
    }
    else
    {
        resp.setMsg("The session ID is not valid, session is not locked, or the key is not correct");
        resp.setError(true);
    }

    return true;
}


bool Cws_config2Ex::onvalidateEnvironment(IEspContext &context, IEspCommonSessionRequest &req, IEspPassFailWithStatusResponse &resp)
{
    Status status;
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSession(sessionId);
    if (pSession)
    {
        pSession->m_pEnvMgr->validate(status);
    }
    else
    {
        resp.setMsg("The session ID is not valid, session is not locked, or the key is not correct");
        resp.setError(true);
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status, pSession);
    resp.updateStatus().setStatus(msgs);
    resp.updateStatus().setError(status.isError());

    return true;
}


bool Cws_config2Ex::onsetValues(IEspContext &context, IEspSetValuesRequest &req, IEspSetValuesResponse &resp)
{
    Status status;
    std::string sessionId = req.getSessionId();
    ConfigMgrSession *pSession = getConfigSessionForUpdate(sessionId, req.getSessionLockKey());
    if (pSession)
    {
        std::string id = req.getNodeId();
        std::shared_ptr<EnvironmentNode> pNode = pSession->m_pEnvMgr->getEnvironmentNode(id);
        if (pNode)
        {
            bool forceCreate = req.getForceCreate();
            bool allowInvalid = req.getAllowInvalid();
            IArrayOf<IConstattributeValueType> &attrbuteValues = req.getAttributeValues();
            std::vector<NameValue> values;
            ForEachItemIn(i, attrbuteValues)
            {
                IConstattributeValueType& attrVal = attrbuteValues.item(i);
                NameValue value;
                value.name = attrVal.getName();
                value.value = attrVal.getValue();
                values.push_back(value);
            }
            pNode->setAttributeValues(values, status, allowInvalid, forceCreate);
            pSession->modified = true;
        }
        else
        {
            status.addMsg(statusMsg::error, id.c_str(), "", "The input node ID is not a valid not in the environment");
            resp.setError(true);
        }
    }
    else
    {
        status.addMsg(statusMsg::error, sessionId.c_str(), "", "The session ID is not valid, session is not locked, or the key is not correct");
        resp.setError(true);
    }

    //
    // Add the messages in the status object to the response
    IArrayOf<IEspstatusMsgType> msgs;
    buildStatusMessageObject(msgs, status, pSession);
    resp.updateStatus().setStatus(msgs);
    resp.updateStatus().setError(status.isError());

    return true;
}


bool Cws_config2Ex::ongetParents(IEspContext &context, IEspNodeRequest &req, IEspGetParentsResponse &resp)
{
    std::string nodeId = req.getNodeId();
    std::string sessionId = req.getSessionId();

    ConfigMgrSession *pSession = getConfigSession(sessionId);
    if (pSession)
    {
        StringArray ids;
        std::shared_ptr<EnvironmentNode> pNode = pSession->m_pEnvMgr->getEnvironmentNode(nodeId);
        if (pNode)
        {
            while (pNode)
            {
                pNode = pNode->getParent();
                if (pNode)
                {
                    ids.append(pNode->getId().c_str());
                }
            }
        }
        else
        {
            resp.setMsg("The input node ID is not a valid not in the environment");
            resp.setError(true);
        }
        resp.setParentIdList(ids);   // even if empty we should set it
    }
    else
    {
        resp.setMsg("The session ID is not valid, session is not locked, or the key is not correct");
        resp.setError(true);
    }

    return true;
}


void Cws_config2Ex::buildStatusMessageObject(IArrayOf<IEspstatusMsgType> &msgs, const Status &status, ConfigMgrSession *pSession) const
{
    std::vector<statusMsg> statusMsgs = status.getMessages();
    for (auto msgIt=statusMsgs.begin(); msgIt!=statusMsgs.end(); ++msgIt)
    {
        Owned<IEspstatusMsgType> pStatusMsg = createstatusMsgType();
        pStatusMsg->setNodeId((*msgIt).nodeId.c_str());
        pStatusMsg->setMsg((*msgIt).msg.c_str());
        pStatusMsg->setMsgLevel(status.getStatusTypeString((*msgIt).msgLevel).c_str());
        pStatusMsg->setAttribute((*msgIt).attribute.c_str());

        if (!(*msgIt).nodeId.empty() && pSession != nullptr)
        {
            StringArray ids;
            getNodeParents((*msgIt).nodeId, pSession, ids);
            std::shared_ptr<EnvironmentNode> pNode = pSession->m_pEnvMgr->getEnvironmentNode((*msgIt).nodeId);
            pStatusMsg->setParentIdList(ids);
        }
        msgs.append(*pStatusMsg.getLink());
    }
}


ConfigMgrSession *Cws_config2Ex::getConfigSession(const std::string &sessionId)
{
    ConfigMgrSession *pSession = nullptr;
    auto it = m_sessions.find(sessionId);
    if (it != m_sessions.end())
    {
        pSession = (it->second);
    }
    return pSession;
}


ConfigMgrSession *Cws_config2Ex::getConfigSessionForUpdate(const std::string &sessionId, const std::string &lockKey)
{
    ConfigMgrSession *pSession = getConfigSession(sessionId);
    if (!pSession || pSession->doesKeyFit(lockKey))
    {
        pSession = nullptr;
    }
    return pSession;
}


bool Cws_config2Ex::deleteConfigSession(const std::string &sessionId)
{
    bool rc = false;
    ConfigMgrSession *pSession = getConfigSession(sessionId);
    if (pSession)
    {
        m_sessions.erase(sessionId);
        delete pSession;
        rc = true;
    }
    return rc;
}


void Cws_config2Ex::getNodelInfo(const std::shared_ptr<EnvironmentNode> &pNode, IEspGetNodeResponse &resp) const
{
    const std::shared_ptr<SchemaItem> &pNodeSchemaItem = pNode->getSchemaItem();
    std::string nodeDisplayName = pNodeSchemaItem->getProperty("displayName");

    //
    // Fill in base node info struct
    resp.updateNodeInfo().setName(nodeDisplayName.c_str());
    resp.updateNodeInfo().setNodeType(pNodeSchemaItem->getItemType().c_str());
    resp.updateNodeInfo().setClass(pNodeSchemaItem->getProperty("className").c_str());
    resp.updateNodeInfo().setCategory(pNodeSchemaItem->getProperty("category").c_str());
    resp.updateNodeInfo().setTooltip(pNodeSchemaItem->getProperty("tooltip").c_str());
    resp.updateNodeInfo().setIsHidden(pNodeSchemaItem->isHidden());
    resp.setNodeId(pNode->getId().c_str());

    //
    // Handle the attributes
    IArrayOf<IEspattributeType> nodeAttributes;
    if (pNode->hasAttributes())
    {
        std::vector<std::shared_ptr<EnvironmentValue>> attributes;
        pNode->getAttributes(attributes);
        for (auto it=attributes.begin(); it!=attributes.end(); ++it)
        {
            std::shared_ptr<EnvironmentValue> pAttr = *it;
            Owned<IEspattributeType> pAttribute = createattributeType();

            const std::shared_ptr<SchemaValue> &pSchemaValue = pAttr->getSchemaValue();
            std::string attributeName = pAttr->getName();
            pAttribute->setName(attributeName.c_str());
            pAttribute->setDisplayName(pSchemaValue->getDisplayName().c_str());
            if (attributeName == "name" && pAttr->isValueSet())
            {
                nodeDisplayName = pAttr->getValue();  // better usability value
            }

            pAttribute->setTooltip(pSchemaValue->getTooltip().c_str());

            const std::shared_ptr<SchemaType> &pType = pSchemaValue->getType();
            std::shared_ptr<SchemaTypeLimits> &pLimits = pType->getLimits();
            pAttribute->updateType().setName(pType->getName().c_str());
            if (pType->getLimits()->isMaxSet())
            {
                pAttribute->updateType().updateLimits().setMaxValid(true);
                pAttribute->updateType().updateLimits().setMax(pType->getLimits()->getMax());
            }
            if (pType->getLimits()->isMinSet())
            {
                pAttribute->updateType().updateLimits().setMinValid(true);
                pAttribute->updateType().updateLimits().setMin(pType->getLimits()->getMin());
            }
            pAttribute->setRequired(pSchemaValue->isRequired());
            pAttribute->setReadOnly(pSchemaValue->isReadOnly());
            pAttribute->setHidden(pSchemaValue->isHidden());


            //StringArray excludeList;
            //excludeList.append("username");
            //pAttribute->updateType().updateLimits().setDisallowList(excludeList);

            std::vector<AllowedValue> allowedValues;
            pSchemaValue->getAllowedValues(allowedValues, pAttr.get());
            if (!allowedValues.empty())
            {
                IArrayOf<IEspchoiceType> choices;
                for (auto valueIt=allowedValues.begin(); valueIt!=allowedValues.end(); ++valueIt)
                {
                    Owned<IEspchoiceType> pChoice = createchoiceType();
                    pChoice->setDisplayName((*valueIt).m_displayName.c_str());
                    pChoice->setValue((*valueIt).m_value.c_str());
                    pChoice->setDesc((*valueIt).m_description.c_str());
                    choices.append(*pChoice.getLink());
                }
                pAttribute->updateType().updateLimits().setChoiceList(choices);
            }

            pAttribute->setCurrentValue(pAttr->getValue().c_str());
            pAttribute->setDefaultValue(pSchemaValue->getDefaultValue().c_str());

            nodeAttributes.append(*pAttribute.getLink());
        }
    }
    resp.setAttributes(nodeAttributes);

    //
    // Now the children
    IArrayOf<IEspnodeType> childNodes;
    if (pNode->hasChildren())
    {
        std::vector<std::shared_ptr<EnvironmentNode>> children;
        pNode->getChildren(children);
        for (auto it=children.begin(); it!=children.end(); ++it)
        {
            std::shared_ptr<EnvironmentNode> pChildEnvNode = *it;
            const std::shared_ptr<SchemaItem> pSchemaItem = pChildEnvNode->getSchemaItem();
            Owned<IEspnodeType> pChildNode = createnodeType();
            pChildNode->updateNodeInfo().setName(pSchemaItem->getProperty("displayName").c_str());
            pChildNode->updateNodeInfo().setNodeType(pSchemaItem->getItemType().c_str());
            pChildNode->updateNodeInfo().setClass(pSchemaItem->getProperty("className").c_str());
            pChildNode->updateNodeInfo().setCategory(pSchemaItem->getProperty("category").c_str());
            pChildNode->updateNodeInfo().setTooltip(pSchemaItem->getProperty("tooltip").c_str());
            pChildNode->updateNodeInfo().setIsHidden(pSchemaItem->isHidden());
            pChildNode->setNodeId(pChildEnvNode->getId().c_str());
            pChildNode->setNumChildren(pChildEnvNode->getNumChildren());
            childNodes.append(*pChildNode.getLink());
        }
    }
    resp.setChildren(childNodes);

    //
    // Build a list of items that can be inserted under this node
    IArrayOf<IEspnodeInfoType> newNodes;
    std::vector<std::shared_ptr<SchemaItem>> insertableList;
    pNode->getInsertableItems(insertableList);
    for (auto it=insertableList.begin(); it!=insertableList.end(); ++it)
    {
        std::shared_ptr<SchemaItem> pSchemaItem = *it;
        Owned<IEspnodeInfoType> pNodeInfo = createnodeInfoType();
        pNodeInfo->setName(pSchemaItem->getProperty("displayName").c_str());
        pNodeInfo->setNodeType(pSchemaItem->getItemType().c_str());
        pNodeInfo->setClass(pSchemaItem->getProperty("className").c_str());
        pNodeInfo->setCategory(pSchemaItem->getProperty("category").c_str());
        pNodeInfo->setTooltip(pSchemaItem->getProperty("tooltip").c_str());
        pNodeInfo->setIsRequired(pSchemaItem->isRequired());
        pNodeInfo->setIsHidden(pSchemaItem->isHidden());  // not likely, but just to be complete
        newNodes.append(*pNodeInfo.getLink());
    }
    resp.setInsertable(newNodes);

    if (pNodeSchemaItem->isItemValueDefined())
    {
        resp.setLocalValueDefined(true);

        const std::shared_ptr<SchemaValue> &pNodeSchemaValue = pNodeSchemaItem->getItemSchemaValue();
        const std::shared_ptr<SchemaType> &pType = pNodeSchemaValue->getType();
        resp.updateValue().updateType().setName(pType->getName().c_str());

        if (pType->getLimits()->isMaxSet())
        {
            resp.updateValue().updateType().updateLimits().setMaxValid(true);
            resp.updateValue().updateType().updateLimits().setMax(pType->getLimits()->getMax());
        }
        if (pType->getLimits()->isMinSet())
        {
            resp.updateValue().updateType().updateLimits().setMinValid(true);
            resp.updateValue().updateType().updateLimits().setMin(pType->getLimits()->getMin());
        }

        if (pNode->isLocalValueSet())
        {
            const std::shared_ptr<EnvironmentValue> &pLocalValue = pNode->getLocalEnvValue();
            resp.updateValue().setCurrentValue(pLocalValue->getValue().c_str());

            //
            // Type information
            const std::shared_ptr<SchemaValue> pLocalSchemaValue = pLocalValue->getSchemaValue();
            const std::shared_ptr<SchemaType> &pLocalType = pLocalSchemaValue->getType();
            std::shared_ptr<SchemaTypeLimits> &pLimits = pLocalType->getLimits();
            resp.updateValue().updateType().setName(pLocalType->getName().c_str());
            if (pLocalType->getLimits()->isMaxSet())
            {
                resp.updateValue().updateType().updateLimits().setMaxValid(true);
                resp.updateValue().updateType().updateLimits().setMax(pLocalType->getLimits()->getMax());
            }
            if (pLocalType->getLimits()->isMinSet())
            {
                resp.updateValue().updateType().updateLimits().setMinValid(true);
                resp.updateValue().updateType().updateLimits().setMin(pLocalType->getLimits()->getMin());
            }

            resp.updateValue().setRequired(pLocalSchemaValue->isRequired());
            resp.updateValue().setReadOnly(pLocalSchemaValue->isReadOnly());
            resp.updateValue().setHidden(pLocalSchemaValue->isHidden());
        }
    }
}


void Cws_config2Ex::getNodeParents(const std::string &nodeId, ConfigMgrSession *pSession, StringArray &parentNodeIds) const
{
    std::shared_ptr<EnvironmentNode> pNode = pSession->m_pEnvMgr->getEnvironmentNode(nodeId);
    if (pNode)
    {
        while (pNode)
        {
            pNode = pNode->getParent();
            if (pNode)
            {
                parentNodeIds.append(pNode->getId().c_str());
            }
        }
    }
}