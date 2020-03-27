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
#include <chrono>
#include <thread>
#include "Metric.hpp"
#include "MetricSink.hpp"

#include <cstdio>
#include <utility>


namespace hpcc_metrics {

    class MetricSet {

        public:

            explicit MetricSet(std::string name, std::shared_ptr<MetricSink> pSink) :
                m_name{std::move(name)},
                m_periodSeconds{10},
                m_stopCollection(false),
                m_pSink(std::move(pSink))
                {}

            bool addMetric(const std::shared_ptr<Metric> &pMetric) {
                auto rc = m_metrics.insert({pMetric->getName(), pMetric});
                return rc.second;
            }


            void setCollectionPeriod(unsigned periodSeconds) {
                m_periodSeconds = std::chrono::seconds(periodSeconds);
            }


            void startCollection() {
                m_collectThread = std::thread(collectionThread, this);
                m_collectThread.detach();
            }


            void stopCollection() {
                m_stopCollection = true;
            }


            void collect() {
                std::vector<std::shared_ptr<MetricValueBase>> values;
                for (const auto &pMetricIt : m_metrics)
                {
                    m_pSink->send(values);
                }
            }


            static void collectionThread(MetricSet *pMetricSet);

            std::shared_ptr<Metric> getMetric(const std::string &name) const {
                std::shared_ptr<Metric> pMetric;
                auto it = m_metrics.find(name);
                if (it != m_metrics.end())
                {
                    pMetric = it->second;
                }
                return pMetric;
            }


        protected:

            std::string m_name;
            std::map<std::string, std::shared_ptr<Metric>> m_metrics;
            std::chrono::seconds m_periodSeconds;
            std::thread m_collectThread;
            std::shared_ptr<MetricSink> m_pSink;
            bool m_stopCollection;
    };


    void MetricSet::collectionThread(MetricSet *pMetricSet) {
        while (!pMetricSet->m_stopCollection)
        {
            std::this_thread::sleep_for(pMetricSet->m_periodSeconds);
            pMetricSet->collect();
        }
        printf("Collected\n");
    }

}
