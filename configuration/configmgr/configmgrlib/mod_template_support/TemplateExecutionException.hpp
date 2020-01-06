/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2018 HPCC Systems®.

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

#ifndef HPCCSYSTEMS_PLATFORM_TEMPLATEEXECUTIONEXCEPTION_HPP
#define HPCCSYSTEMS_PLATFORM_TEMPLATEEXECUTIONEXCEPTION_HPP

#include <exception>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

class TemplateExecutionException : public std::exception
{
    public:

        explicit TemplateExecutionException(std::string reason) :
            m_reason(std::move(reason)) { }
        TemplateExecutionException(std::string reason, const std::string &step, const std::string &filename) :
            m_reason(std::move(reason)) { setContext(step, filename); }
        TemplateExecutionException() = default;

        void setContext(const std::string &step, const std::string &filename)
        {
            m_reason = "There was a problem in template '" + filename + "', executing step " + step + ", cause: " + m_reason;
        }

        const char *what() const throw() override
        {
            return m_reason.c_str();
        }


    private:

        std::string m_reason;
};

#endif //HPCCSYSTEMS_PLATFORM_TEMPLATEEXECUTIONEXCEPTION_HPP
