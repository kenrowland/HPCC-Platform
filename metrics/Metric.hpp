/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2012 HPCC Systems®.

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
#include "MetricValue.hpp"


namespace hpcc_metrics {

    class Metric {
        public:

            explicit Metric(std::string &metricName) : m_metricName(std::move(metricName)) {}
            virtual ~Metric() = default;
            const std::string &getName() const { return m_metricName; }
            virtual std::vector<std::shared_ptr<MetricValueBase>> collect() = 0;


        protected:

            std::string m_metricName;
    };


    class CountableMetric : public Metric {
        public:

            explicit CountableMetric(std::string metricName) : Metric{metricName}, m_count{0} {}
            ~CountableMetric() override = default;
            void inc(unsigned incVal) { m_count.fetch_add(incVal); }

            std::vector<std::shared_ptr<MetricValueBase>> collect() override
            {
                return std::vector<std::shared_ptr<MetricValueBase>>{std::make_shared<MetricValue<unsigned>>(m_metricName, m_count.exchange(0))};
            }


        protected:

            std::atomic<unsigned> m_count;
    };


//class QueueSizeMetric : public Metric
//{
//    public:
//
//        QueueSizeMetric(const std::string &metricName) : Metric(metricName), m_queueSize(0) { }
//        void inc(unsigned incVal = 1) { m_queueSize.fetch_add(incVal); }
//        void dec(unsigned decVal = 1) { m_queueSize.fetch_sub(decVal); }
//
//
//    protected:
//
//        std::atomic<unsigned> m_queueSize;
//};
//
//
//class QueueLatencyMetric : Metric
//{
//    public:
//
//        QueueLatencyMetric(const std::string &metricName) : Metric(metricName) {}
//
//
//    protected:
//
//        //std::queue<std::chrono::time_point();
//};
//
//
//template<typename T>
//void update_maximum(std::atomic<T>& maximum_value, T const& value) noexcept
//{
//    T prev_value = maximum_value;
//    while(prev_value < value &&
//          !maximum_value.compare_exchange_weak(prev_value, value))
//        ;
//}

}
