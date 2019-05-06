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

#include "Environment.hpp"
#include "Exceptions.hpp"
#include "TemplateExecutionException.hpp"


Environment::Environment(const std::string &masterCfgFile, const std::vector<std::string> &configPaths) :
    m_masterCfgSchemaFile(masterCfgFile),
    m_configPaths(configPaths),
    m_initializeEmpty(false)
{

}


Environment::Environment(std::shared_ptr<EnvironmentMgr> pEnvMgr) :
    m_pEnvMgr(pEnvMgr),
    m_initializeEmpty(false)
{
}


void Environment::initialize()
{
    if (!m_pEnvMgr)
    {
        try
        {
            EnvironmentType envType = XML;
            m_pEnvMgr = getEnvironmentMgrInstance(envType);
            if (m_pEnvMgr->loadSchema(m_masterCfgSchemaFile, m_configPaths))
            {

                //
                // Load an environment ?
                if (!m_inputEnvironment.empty())
                {
                    m_pEnvMgr->loadEnvironment(m_inputEnvironment);
                }

                //
                // Or, init an empty one ?
                else if (m_initializeEmpty)
                {
                    m_pEnvMgr->initializeEmptyEnvironment();
                }

                m_pEnvMgr->enableValidationOnChange(false);  // skip validation on each change, wait until the end
            }
            else
            {
                std::string reason = "There was a problem loading the schema:" + m_pEnvMgr->getLastSchemaMessage();
                throw TemplateExecutionException(reason);
            }
        }
        catch (const ParseException &pe)
        {
            std::string reason = "There was a problem creating the environment: " + std::string(pe.what());
            throw TemplateExecutionException(reason);
        }
    }
}


void Environment::save(const std::shared_ptr<Variables> &pVariables)
{
    if (!m_outputEnvironment.empty())
    {
        m_pEnvMgr->saveEnvironment(pVariables->doValueSubstitution(m_outputEnvironment));
    }
}
