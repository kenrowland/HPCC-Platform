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


#ifndef _CONFIG2SERVICE_SESSION_HPP_
#define _CONFIG2SERVICE_SESSION_HPP_


#include "EnvironmentMgr.hpp"
#include "build-config.h"


struct ConfigMgrSession {

    std::string username;
    std::string configPath;
    std::string sourcePath;
    std::string activePath;
    std::string configType;
    std::string masterConfigFile;
    bool writeEnabled;
    EnvironmentMgr *m_pEnvMgr;

    bool initializeSession(std::vector<std::string> &cfgParms)
    {
        m_pEnvMgr = getEnvironmentMgrInstance(configType);
        if (m_pEnvMgr)
            m_pEnvMgr->loadConfig(configPath, masterConfigFile, cfgParms);
        return m_pEnvMgr;
    }


    bool loadEnvironment(const std::string &envFile)
    {
        return m_pEnvMgr->loadEnvironment(envFile);
    }


    std::string getEnvironmentFileExtension() const 
    {
        std::string ext = ".unk";
        if (configType == "XML")
        {
            ext = ".xml";
        }
        return ext;
    }

};


#endif