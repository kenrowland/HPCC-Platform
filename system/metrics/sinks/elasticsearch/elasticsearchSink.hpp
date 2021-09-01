/*##############################################################################
    HPCC SYSTEMS software Copyright (C) 2021 HPCC Systems®.
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


#pragma once

#include "jmetrics.hpp"
#include "jptree.hpp"
#include "jstring.hpp"

using namespace hpccMetrics;

#ifdef ELASTICSEARCHSINK_EXPORTS
#define ELASTICSEARCHSINK_API DECL_EXPORT
#else
#define ELASTICSEARCHSINK_API DECL_IMPORT
#endif

class ELASTICSEARCHSINK_API ElasticsearchSink : public PeriodicMetricSink
{
    public:
        explicit ElasticsearchSink(const char *name, const IPropertyTree *pSettingsTree);
        ~ElasticsearchSink() override = default;

        void prepareToStartCollecting();
        void collectingHasStopped();
        void doCollection();

    protected:
        std::string getIndexName(const std::string &suffix);
        bool initializeIndex();


    protected:
        StringBuffer indexNameTemplate;
        StringBuffer protocol;
        StringBuffer hostName;
        StringBuffer port;
        std::string curIndexName;
};
