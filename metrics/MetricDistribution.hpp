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

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include "Metric.hpp"

namespace hpcc_metrics {

    template <typename T>
    class MetricDistribution : public Metric {

        public:

            MetricDistribution(std::string &name, const std::vector<T> &dist);
            void increment(const T &level);

            bool collect(std::vector<std::shared_ptr<MetricValueBase>> &values) override;


        protected:

            std::map<T, std::atomic<unsigned> *> m_distribution;
    };



    template <typename T>
    MetricDistribution<T>::MetricDistribution(std::string &name, const std::vector<T> &dist) :
        Metric(name)
    {
        if (dist[0] != std::numeric_limits<T>::min())
            m_distribution[0] = new std::atomic<unsigned>();

        for (auto const &distValue : dist)
        {
            m_distribution.insert(std::pair<T, std::atomic<unsigned> *>(distValue, new std::atomic<unsigned>()));
        }

        if (dist[dist.size()-1] != std::numeric_limits<T>::max())
            m_distribution.insert(std::pair<T, std::atomic<unsigned> *>(std::numeric_limits<T>::max(), new std::atomic<unsigned>()));
    }


    template <typename T>
    void MetricDistribution<T>::increment(const T &level)
    {
        auto it = m_distribution.lower_bound(level);
        if (it != m_distribution.end())
        {
            ++(it.second);
        }
    }

    template<typename T>
    bool MetricDistribution<T>::collect(std::vector<std::shared_ptr<MetricValueBase>> &values) {
        return false;
    }


}
