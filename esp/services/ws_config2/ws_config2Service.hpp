#ifndef _WSCONFIG2_HPP_
#define _WSCONFIG2_HPP_

#include "ws_config2.hpp"
#include "ws_config2_esp.ipp"
#include <string>
#include "XSDConfigParser.hpp"
#include "EnvironmentMgr.hpp"
#include "XMLEnvironmentMgr.hpp"

class Cws_config2Ex : public Cws_config2
{
public:
    IMPLEMENT_IINTERFACE

    Cws_config2Ex();
    virtual ~Cws_config2Ex();

    virtual bool ongetPath(IEspContext &context, IEspGetPathRequest &req, IEspGetPathResponse &resp);

    virtual bool mockInterface(const std::string &path, IEspGetPathResponse &resp);

private:
    EnvironmentMgr *m_pEnvMgr;   
};

#endif // _WSCONFIG2_HPP_

