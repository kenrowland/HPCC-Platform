#ifndef _WSCONFIG2_HPP_
#define _WSCONFIG2_HPP_

#include "ws_config2.hpp"
#include "ws_config2_esp.ipp"
#include <string>
#include "XSDSchemaParser.hpp"
#include "EnvironmentMgr.hpp"
#include "XMLEnvironmentMgr.hpp"
#include "ws_config2Session.hpp"


class Status;

class Cws_config2Ex : public Cws_config2
{
public:
    IMPLEMENT_IINTERFACE

    Cws_config2Ex();
    virtual ~Cws_config2Ex();

    virtual bool ongetNode(IEspContext &context, IEspNodeRequest &req, IEspGetNodeResponse &resp);
    virtual bool onsetValues(IEspContext &context, IEspSetValuesRequest &req, IEspSetValuesResponse &resp);
    virtual bool ongetParents(IEspContext &context, IEspNodeRequest &req, IEspGetParentsResponse &resp);
    virtual bool oninsertNode(IEspContext &context, IEspInsertNodeRequest &req, IEspGetNodeResponse &resp);
    virtual bool onremoveNode(IEspContext &context, IEspNodeRequest &req, IEspCommonStatusResponse &resp);

    virtual bool onopenSession(IEspContext &context, IEspOpenSessionRequest &req, IEspOpenSessionResponse &resp);
    virtual bool oncloseSession(IEspContext &context, IEspCloseSessionRequest &req, IEspPassFailResponse &resp);
    virtual bool ongetEnvironmentFileList(IEspContext &context, IEspCommonSessionRequest &req, IEspGetEnvironmentListResponse &resp);
    virtual bool onopenEnvironmentFile(IEspContext &context, IEspOpenEnvironmentFileRequest &req, IEspPassFailResponse &resp);
    virtual bool onsaveEnvironmentFile(IEspContext &context, IEspSaveEnvironmentFileRequest &req, IEspPassFailResponse &resp);
    virtual bool onlockSession(IEspContext &context, IEspCommonSessionRequest &req, IEspLockSessionResponse &resp);
    virtual bool onunlockSession(IEspContext &context, IEspUnlockSessionRequest &req, IEspPassFailResponse &resp);


private:

    void buildStatusMessageObject(IArrayOf<IEspstatusMsgType> &msgs, const Status &status) const;
    ConfigMgrSession *getConfigSession(const std::string &sessionId);
    ConfigMgrSession *getConfigSessionForUpdate(const std::string &sessionId, const std::string &lockKey);
    void getNodelInfo(const std::shared_ptr<EnvironmentNode> &pNode, IEspGetNodeResponse &resp) const;


private:

    std::map<std::string, ConfigMgrSession *> m_sessions;
    unsigned m_sessionKey;
};

#endif // _WSCONFIG2_HPP_