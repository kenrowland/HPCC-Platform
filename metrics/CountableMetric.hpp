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
#include <atomic>
#include <vector>
#include <mutex>
#include "Metric.hpp"
#include "MetricValue.hpp"

namespace hpcc_metrics {


    class CountableMetric : public Metric {
        public:

            explicit CountableMetric(std::string metricName) : Metric{metricName}, m_count{0} {}
            ~CountableMetric() override = default;
            void inc(unsigned incVal) { m_count.fetch_add(incVal); }

            bool collect(std::vector<std::shared_ptr<MetricValueBase>> &values) override
            {
                values.emplace_back(std::make_shared<MetricValue<unsigned>>(m_metricName, m_count.exchange(0)));
                return true;
            }


        protected:

            std::atomic<unsigned> m_count;
    };

}
