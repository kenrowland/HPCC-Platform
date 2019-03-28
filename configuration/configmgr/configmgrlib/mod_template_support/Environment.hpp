/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2019 HPCC Systems®.

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

#ifndef HPCCSYSTEMS_PLATFORM_ENVIRONMENT_HPP
#define HPCCSYSTEMS_PLATFORM_ENVIRONMENT_HPP

#include <memory>
#include <vector>
#include "EnvironmentMgr.hpp"


class Environment {

    public:

        Environment(const std::string &masterCfgFile, std::vector<std::string> configPaths);
        Environment(std::shared_ptr<EnvironmentMgr> pEnvMgr);
        ~Environment() = default;
        void initialize();
        bool isSave() const { return !m_outputEnvironment.empty(); }
        void saveEnvironment();
        void setOutputEnvironment(const std::string &outputName) { m_outputEnvironment = outputName; }
        void setLoadEnvironment(const std::string &inputName) { m_inputEnvironment = inputName; }


    public:


        std::shared_ptr<EnvironmentMgr> m_pEnvMgr;
        std::string m_masterCfgSchemaFile;
        std::vector<std::string> m_configPaths;
        std::string m_inputEnvironment;
        std::string m_outputEnvironment;
};


#endif //HPCCSYSTEMS_PLATFORM_ENVIRONMENT_HPP
