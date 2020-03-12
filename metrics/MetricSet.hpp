//
// Created by rowlke01 on 2/27/20.
//

#pragma once


#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <thread>
#include "Metric.hpp"

#include "stdio.h"


namespace hpcc_metrics {

    class MetricSet {

        public:

            explicit MetricSet(std::string name) : m_name{std::move(name)}, m_periodSeconds{10},
                                                   m_stopCollection(false) {}

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
                for (const auto &pMetricIt : m_metrics)
                {
                    auto values = pMetricIt.second->collect();
                    for (auto const &val : values)
                    {
                        printf("Collection %s -> %s\n", val->getName().c_str(), val->toString().c_str());
                    }
                    //printf("Collection: val = %d\n", val);
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
