


#include "espmetrics.hpp"
#include "MetricSet.hpp"

using namespace hpccMetrics;

void EspMetrics::init(int config) {

    //
    // This is where the config would be processed, just do it by force for now

    pCountRequests = std::make_shared<CountMetric>("requests");

    auto pRequestMetricSet = std::make_shared<MetricSet>("http_requests", "esp");

    pRequestMetricSet->addMetric(pCountRequests);

    collector.addMetricSet(pRequestMetricSet);

    //
    // Just some temp stuff
    std::map<std::string, std::string> parms = { {"filename", "metrics.txt"}};
    auto pSink = MetricSink::getMetricSinkFromLib("elasticsearchsink", parms);
    collector.addSink(pSink);

    collector.start(10);
}
