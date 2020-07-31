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

#include "FileSink.hpp"
#include "Measurement.hpp"
#include <cstdio>
#include <map>

using namespace hpccMetrics;

extern "C" MetricSink* getMetricSinkInstance(const std::map<std::string, std::string> &parms)
{
    MetricSink *pSink = new FileMetricSink(parms);
    return pSink;
}


FileMetricSink::FileMetricSink(const std::map<std::string, std::string> &parms) :
    MetricSink{parms}
{
    auto it = parms.find("filename");
    if (it != parms.end())
    {
        m_filename = it->second;
    }
}


void FileMetricSink::send(const std::vector<std::shared_ptr<IMeasurement>> &values, const std::string &setName)
{
    auto handle = fopen(m_filename.c_str(), "a");
    for (const auto& pValue : values)
    {
        fprintf(handle, "%s -> %s\n", pValue->getName().c_str(), pValue->toString().c_str());
    }
    fprintf(handle, "\n");
    fclose(handle);
}


void FileMetricSink::init()
{
    //
    // Clear the file
    auto handle = fopen(m_filename.c_str(), "w");
    fclose(handle);
}
