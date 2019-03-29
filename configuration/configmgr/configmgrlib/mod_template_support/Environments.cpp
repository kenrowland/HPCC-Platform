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

#include "Environments.hpp"
#include "TemplateExecutionException.hpp"


Environments::~Environments()
{
    m_environments.clear();
}


//void Environments::add(const std::string &masterFile, const std::vector<std::string> &paths, const std::string &envFile, const std::string &name)
//{
//    EnvironmentType envType = XML;
//    std::shared_ptr<EnvironmentMgr> pEnvMgr = getEnvironmentMgrInstance(envType);
//    pEnvMgr->loadSchema(masterFile, paths);
//    if (!envFile.empty())
//    {
//        pEnvMgr->loadEnvironment(envFile);
//    }
//    add(pEnvMgr, name);
//}


void Environments::add(std::shared_ptr<Environment> &pEnv, const std::string &name)
{
    auto rc = m_environments.insert({name, pEnv});
    if (!rc.second)
    {
        std::string msg = "Envronment " + name + "already defined";
        throw (TemplateExecutionException(msg));
    }
}


std::shared_ptr<Environment> Environments::get(const std::string &name) const
{
    auto it = m_environments.find(name);
    if (it == m_environments.end())
    {
        std::string msg = "Unable to find envronment " + name;
        throw(TemplateExecutionException(msg));
    }
    return it->second;
}


void Environments::release(const std::string &name)
{
    auto it = m_environments.find(name);
    if (it == m_environments.end())
    {
        std::string msg = "Unable to find envronment " + name;
        throw(TemplateExecutionException(msg));
    }
    m_environments.erase(it);
}


void Environments::save() const
{
    for (auto const &env: m_environments)
    {
        if (env.second->isSave())
        {
            env.second->save();
        }
    }
}
