/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2020 HPCC Systems®.

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


/*

 Need to know mapping of a metric set name to an index name

 Elasticsearch
 - server
   hostname: <hostname>
   port: port number
   protocol: http | https

 - metricsets
   - metricset1_name
     index: index template




*/


#pragma once

#include "MetricSink.hpp"
#include <map>

using namespace hpcc_metrics;


struct MetricSetInfo  {
    MetricSetInfo() = default;
    std::string setName;                                 // name of the set
    std::string indexTemplate;                           // template for generating the index name
    std::string lastIndexName;                           // last index name. If !empty, it also exists
    std::string timestampType;                           // type of timestamp to add
    std::string timestampName;                           // if !empty, add a timestamp with this name
    std::map<std::string, std::string> metricValueTypes; // used to create index mapping on new index
};



class ElasticsearchSink : public MetricSink
{
    public:

        explicit ElasticsearchSink(const std::map<std::string, std::string> &parms);
        void send(const std::vector<std::shared_ptr<MeasurementBase>> &values, const std::string &setName) override;
        void init(const std::vector<std::shared_ptr<MetricSet>> &metricSets) override;


    protected:

        bool initializeIndex(MetricSetInfo *pSetInfo);
        std::string buildIndexName(const std::string &nameTemplate, const std::string &setName);
        bool createNewIndex(MetricSetInfo *pSetInfo, const std::string &indexName);


    protected:

        std::map<std::string, MetricSetInfo> setInfo;
        std::string protocol;
        std::string domain;
        std::string port;
};
