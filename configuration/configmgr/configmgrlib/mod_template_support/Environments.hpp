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

#ifndef HPCCSYSTEMS_PLATFORM_ENVIRONMENTS_HPP
#define HPCCSYSTEMS_PLATFORM_ENVIRONMENTS_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "Environment.hpp"
#include "Variables.hpp"
#include "Status.hpp"

class Environments {

    public:

        Environments() = default;
        ~Environments();
        void add(std::shared_ptr<Environment> &pEnv, const std::string &name);
        std::shared_ptr<Environment> get(const std::string &name) const;
        void save(const std::shared_ptr<Variables> &pVariables) const;
        void validate(std::map<std::string, Status> &envStatus) const;
        void release(const std::string &name);


    private:

        std::map<std::string, std::shared_ptr<Environment>> m_environments;
};


#endif //HPCCSYSTEMS_PLATFORM_ENVIRONMENTS_HPP
