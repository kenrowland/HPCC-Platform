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


namespace hpcc_metrics {
    template<typename T>
    class GaugeMetric : public Metric {
        public:

            explicit GaugeMetric(std::string metricName) : Metric{std::move(metricName)}, value(0) {}

            ~GaugeMetric() override = default;

            void inc(T inc);

            void dec(T dec);

            bool collect(std::vector<std::shared_ptr<MeasurementBase>> &values) override;


        protected:

            std::atomic<T> value;
    };


    template<typename T>
    void GaugeMetric<T>::inc(T inc) {
        value += inc;
    }


    template<typename T>
    void GaugeMetric<T>::dec(T dec) {
        value -= dec;
    }


    template<typename T>
    bool GaugeMetric<T>::collect(std::vector<std::shared_ptr<MeasurementBase>> &values) {
        values.emplace_back(std::make_shared<Measurement<T>>(metricName, value.load()));
        return true;
    }
}
