//
// Created by rowlke01 on 2/27/20.
//

#pragma once

#include <map>
#include <string>
#include <memory>

//#include "Metric.hpp"
#include "MetricSet.hpp"

namespace hpcc_metrics {

    class MetricsManager {

        public:

            MetricsManager() = default;

            std::shared_ptr<MetricSet> createMetricSet(const std::string &name) {
                auto it = m_metricSets.find(name);
                if (it != m_metricSets.end())
                {
                    return it->second;
                }

                return std::make_shared<MetricSet>(name);
            }

            template<typename T>
            std::shared_ptr<T> createMetric(const std::string &metricName);

            template<typename T>
            std::shared_ptr<T> getMetric(const std::string &metricName);

        private:

            std::map<std::string, std::shared_ptr<Metric>> m_metrics;
            std::map<std::string, std::shared_ptr<MetricSet>> m_metricSets;

    };

    template<typename T>
    std::shared_ptr<T> MetricsManager::createMetric(const std::string &metricName) {
        auto it = m_metrics.find(metricName);
        if (it != m_metrics.end())
        {
            throw (std::exception());  // just throw for now, maybe add our own
        }
        std::shared_ptr<T> pMetric = std::make_shared<T>(metricName);
        m_metrics[metricName] = pMetric;
        return pMetric;
    }

    template<typename T>
    std::shared_ptr<T> MetricsManager::getMetric(const std::string &metricName) {
        auto it = m_metrics.find(metricName);
        if (it == m_metrics.end())
        {
            throw (std::exception());  // just throw for now, maybe add our own
        }

        std::shared_ptr<T> pMetric = std::dynamic_pointer_cast<T>(it->second);
        if (!pMetric)
        {
            throw (std::exception());
        }
        return pMetric;
    }

}
