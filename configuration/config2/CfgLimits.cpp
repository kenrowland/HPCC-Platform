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

#include "CfgLimits.hpp"
#include "EnvValue.hpp"

std::vector<AllowedValue> CfgLimits::getAllowedValues(const std::shared_ptr<EnvValue> &pEnvValue) const 
{ 
    return m_allowedValues.empty() ? getEnumeratedKeyValues(pEnvValue) : m_allowedValues; 
}


std::vector<AllowedValue> CfgLimits::getEnumeratedKeyValues(const std::shared_ptr<EnvValue> &pEnvValue) const 
{ 
    return std::vector<AllowedValue>(); 
}