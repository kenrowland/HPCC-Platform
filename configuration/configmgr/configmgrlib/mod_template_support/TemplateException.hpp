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

#ifndef HPCCSYSTEMS_PLATFORM_TEMPLATEEXCEPTION_HPP
#define HPCCSYSTEMS_PLATFORM_TEMPLATEEXCEPTION_HPP

#include <exception>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

class TemplateException : public std::exception {
    public:

        explicit TemplateException(const rapidjson::Document *pDocument, const std::string &templateFilename);

        explicit TemplateException(std::string reason, bool badTemplate = false) :
            m_reason(std::move(reason)), m_invalidTemplate(badTemplate) {};

        TemplateException() = default;

        bool isTemplateInvalid() const { return m_invalidTemplate; }

        const char *what() const throw() override {
            return m_reason.c_str();
        }


    private:

        std::string m_reason;
        bool m_invalidTemplate = false;
};


class TemplateExceptionVarIndexOutOfBounds : public TemplateException {

    public:

        explicit TemplateExceptionVarIndexOutOfBounds(std::string reason, bool badTemplate = false) :
                TemplateException(reason, badTemplate) {}

        TemplateExceptionVarIndexOutOfBounds() = default;
};

#endif //HPCCSYSTEMS_PLATFORM_TEMPLATEEXCEPTION_HPP
