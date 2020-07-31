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

#pragma once


#include <map>
#include <string>
#include <memory>
#include "IMetricSet.hpp"
#include "Metrics.hpp"


namespace hpccMetrics
{

class MetricSet : public IMetricSet
{
    public:

        MetricSet(std::string _name, std::string _prefix) :
            name{std::move(_name)},
            metricNamePrefix{std::move(_prefix)} { }

        virtual ~MetricSet() = default;


        std::string getName() const override
        {
            return name;
        }


        void addMetric(const std::shared_ptr<IMetric> &pMetric) override
        {
            //
            // A metric may only be added to one metric set
            if (!pMetric->isInMetricSet())
            {
                auto rc = metrics.insert({pMetric->getName(), pMetric});
                if (!rc.second)
                {
                    throw std::exception();   // metric already added with the same name
                }
                pMetric->setInMetricSet(true);
                //pMetric->setPrefix(metricNamePrefix);

                if (pMetric->isTyped())
                {
                    typedMetrics[pMetric->getName()] = pMetric->getType();
                }
            }
            else
            {
                throw std::exception();  // not sure if this is the right thing to do yet
            }
        }


        void init() override
        {
            for (const auto& metricIt : metrics)
            {
                metricIt.second->init();
            }
        }


        void collect(std::vector<std::shared_ptr<IMeasurement>> &values) override
        {
            for (const auto &metricIt : metrics)
            {
                metricIt.second->collect(values, metricNamePrefix);
            }
        }


        const std::map<std::string, std::string> &getTypedMetrics() const override
        {
            return typedMetrics;
        }


    protected:

        std::map<std::string, std::shared_ptr<IMetric>> metrics;
        std::map<std::string, std::string> typedMetrics;
        std::string name;
        std::string metricNamePrefix;
};
}
