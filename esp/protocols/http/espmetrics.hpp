//
// Created by ken on 7/22/20.
//

#ifndef HPCCSYSTEMS_PLATFORM_ESPMETRICS_HPP
#define HPCCSYSTEMS_PLATFORM_ESPMETRICS_HPP

#include <memory>
#include "../../metrics/Metrics.hpp"
#include "../../metrics/PeriodicMetricsReporter.hpp"

using namespace hpccMetrics;


class EspMetrics {

    public:
        EspMetrics() { };
        ~EspMetrics() { };
        void init(int config);


    public:

        std::shared_ptr<CountMetric> pCountRequests;
        PeriodicMetricsReporter collector;

};


#endif //HPCCSYSTEMS_PLATFORM_ESPMETRICS_HPP
