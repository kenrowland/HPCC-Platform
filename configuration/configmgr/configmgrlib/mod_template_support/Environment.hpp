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
#include "Variables.hpp"


class Environment {

    public:

        Environment(const std::string &masterCfgFile, std::vector<std::string> configPaths);
        explicit Environment(std::shared_ptr<EnvironmentMgr> pEnvMgr);
        ~Environment() = default;
        void initialize();
        bool isSave() const { return !m_outputEnvironment.empty(); }
        void save(const std::shared_ptr<Variables> &pVariables);
        void setOutputName(const std::string &outputName) { m_outputEnvironment = outputName; }
        void setLoadName(const std::string &inputName) { m_inputEnvironment = inputName; }
        void setInitializeEmpty(bool init) { m_initializeEmpty = init; }


    public:


        std::shared_ptr<EnvironmentMgr> m_pEnvMgr;
        std::string m_masterCfgSchemaFile;
        std::vector<std::string> m_configPaths;
        bool m_initializeEmpty;
        std::string m_inputEnvironment;
        std::string m_outputEnvironment;
};


#endif //HPCCSYSTEMS_PLATFORM_ENVIRONMENT_HPP
