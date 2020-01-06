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

#ifndef HPCCSYSTEMS_PLATFORM_TARGET_HPP
#define HPCCSYSTEMS_PLATFORM_TARGET_HPP

#include <string>
#include <vector>
#include <memory>

class EnvironmentMgr;
class Variables;

class Target {

    public:

        Target() : m_isPath(false) { }
        void setTargetNodeId(const std::string &nodeId) { m_target = nodeId; m_isPath = false; }
        void setTargetPath(const std::string &path) { m_target = path; m_isPath = true; }
        void setEnvironmentName(const std::string &name) { m_environmentName = name; }
        const std::string &getEnvironmentName() const { return m_environmentName; }
        bool getTargetNodeIds(const std::shared_ptr<EnvironmentMgr> &pEnvMgr, const std::shared_ptr<Variables> &pVariables, std::vector<std::string> &nodeIds);
        std::string getTargetNodeId(const std::shared_ptr<EnvironmentMgr> &pEnvMgr, const std::shared_ptr<Variables> &pVariables);


    private:

        std::string m_target;
        std::string m_environmentName;
        bool m_isPath;

};


#endif //HPCCSYSTEMS_PLATFORM_TARGET_HPP
