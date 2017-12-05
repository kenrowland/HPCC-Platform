#ifndef _WSCONFIG2_HPP_
#define _WSCONFIG2_HPP_

#include "ws_config2.hpp"
#include "ws_config2_esp.ipp"
#include <string>
#include "XSDConfigParser.hpp"
#include "EnvironmentMgr.hpp"
#include "XMLEnvironmentMgr.hpp"

class Status;

class Cws_config2Ex : public Cws_config2
{
public:
    IMPLEMENT_IINTERFACE

    Cws_config2Ex();
    virtual ~Cws_config2Ex();

    virtual bool ongetNode(IEspContext &context, IEspGetNodeRequest &req, IEspGetNodeResponse &resp);
    virtual bool onsetValues(IEspContext &context, IEspSetValuesRequest &req, IEspSetValuesResponse &resp);
    virtual bool ongetParents(IEspContext &context, IEspGetParentsRequest &req, IEspGetParentsResponse &resp);

    virtual bool onopenSession(IEspContext &context, IEspOpenSessionRequest &req, IEspOpenSessionResponse &resp);
    virtual bool oncloseSession(IEspContext &context, IEspCloseSessionRequest &req, IEspPassFailResponse &resp);
    virtual bool onsetEnvironmentConfig(IEspContext &context, IEspSetEnvironmentConfigRequest &req, IEspPassFailResponse &resp);
    virtual bool ongetEnvironmentFileList(IEspContext &context, IEspGetEnvironmentListRequest &req, IEspGetEnvironmentListResponse &resp);


private:

    void buildStatusMessageObject(IArrayOf<IEspstatusMsgType> &msgs, const Status &status) const;
    

private:
    EnvironmentMgr *m_pEnvMgr;   
};

#endif // _WSCONFIG2_HPP_

