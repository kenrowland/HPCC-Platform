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

#include "Metrics.hpp"

using namespace hpccMetrics;


bool CounterMetric::prepareMeasurements(const std::string &context)
{
    bool rc = false;

    //
    // Find the current context values object and update it. If not found, this is the first call
    // for this, so create context.
    auto it = contextValues.find(context);
    if (it != contextValues.end())
    {
        it->second.lastValue = it->second.curValue;
        it->second.curValue = count.load(std::memory_order_relaxed);
        it->second.lastCollectedAt = it->second.curCollectedAt;
        it->second.curCollectedAt = std::chrono::high_resolution_clock::now();
        rc = true;
    }
    else
    {
        CounterMetric::collectionValues initialContextValues;
        initialContextValues.curValue = initialContextValues.lastValue = 0;
        initialContextValues.curCollectedAt = std::chrono::high_resolution_clock::now();
        contextValues.insert({context, initialContextValues});
    }
    return rc;
}


void CounterMetric::getMeasurement(const std::string &measType, const std::string &measName, const std::string &context, MeasurementVector &measurements)
{
    std::shared_ptr<MeasurementBase> pMeasurement;
    auto it = contextValues.find(context);
    if (it != contextValues.end())
    {
        std::string measurementName(measName);
        measurementName.append(".").append(measType);
        if (measType == "count" || measType == "default")
        {
            pMeasurement = std::make_shared<Measurement<uint32_t>>(this, measurementName, description, it->second.curValue);
        }
        else if (measType == "resetting_count")
        {
            pMeasurement = std::make_shared<Measurement<uint32_t>>(this, measurementName, description, it->second.curValue - it->second.lastValue);
        }
        else if (measType == "rate")
        {
            unsigned numEvents = it->second.curValue - it->second.lastValue;
            auto seconds = (std::chrono::duration_cast<std::chrono::seconds>(it->second.curCollectedAt - it->second.lastCollectedAt)).count();
            float rate = (seconds != 0) ? (static_cast<float>(numEvents) / static_cast<float>(seconds)) : 0;
            pMeasurement = std::make_shared<Measurement<float>>(this, measurementName, description, rate);
        }
        // throw ?
    }
    // else throw ?

    if (pMeasurement)
    {
        measurements.emplace_back(pMeasurement);
    }
}


IMetricSink *MetricSink::getSinkFromLib(const char *type, const char *sinkName, const IPropertyTree *pSettingsTree)
{
    return MetricSink::getSinkFromLib(type, "", sinkName, pSettingsTree);
}

IMetricSink *MetricSink::getSinkFromLib(const char *type, const char *getInstanceProcName, const char *sinkName, const IPropertyTree *pSettingsTree)
{
    std::string libName;

    //
    // First, treat type as a full library name
    libName = type;
    if (libName.find(SharedObjectExtension) == std::string::npos)
    {
        libName.append(SharedObjectExtension);
    }
    HINSTANCE libHandle = LoadSharedObject(libName.c_str(), true, false);

    //
    // If type wasn't the lib name, treat it as a type
    if (libHandle == nullptr)
    {
        libName.clear();
        libName.append("libhpccmetrics_").append(type).append(SharedObjectExtension);
        libHandle = LoadSharedObject(libName.c_str(), true, false);
    }

    //
    // If able to load the lib, get the instance proc and create the sink instance
    IMetricSink *pSink = nullptr;
    if (libHandle != nullptr)
    {
        const char *epName = !isEmptyString(getInstanceProcName) ? getInstanceProcName : "getSinkInstance";
        auto getInstanceProc = (getSinkInstance) GetSharedProcedure(libHandle, epName);
        if (getInstanceProc != nullptr)
        {
            const char *name = isEmptyString(sinkName) ? type : sinkName;
            pSink = getInstanceProc(name, pSettingsTree);
        }
    }
    return pSink;
}



void MetricsReporter::addMetric(const std::shared_ptr<IMetric> &pMetric)
{
    std::unique_lock<std::mutex> lock(reportMutex);
    std::string metricReportName = metricNamePrefix.str();
    metricReportName.append(pMetric->getName());
    auto it = metrics.find(metricReportName);
    if (it == metrics.end())
    {
        metrics.insert({metricReportName, pMetric});
    }
}


void MetricsReporter::removeMetric(const std::shared_ptr<IMetric> &pMetric)
{
    std::unique_lock<std::mutex> lock(reportMutex);
    std::string metricName = metricNamePrefix.str();
    metricName.append(pMetric->getName());
    auto it = metrics.find(metricName);
    if (it != metrics.end())
    {
        metrics.erase(it);
    }
}


void MetricsReporter::collectMeasurements(const std::string &sinkName)
{
    auto sinkIt = sinks.find(sinkName);
    if (sinkIt != sinks.end())
    {
        std::unique_lock<std::mutex> lock(reportMutex);

        //
        // Get metric report info based on what the sink wants to report
        std::map<std::string, MetricReportInfo> reportMetrics = getMetricReportInfo(sinkIt->second.reportMeasurements);

        //
        // Save state for each metric in the report
        for (auto &reportMetric : reportMetrics)
        {
            reportMetric.second.stateSaved = reportMetric.second.pMetric->prepareMeasurements(sinkName);
        }

        //
        // Build vector of measurements to report
        MeasurementVector measurements;
        for (auto &reportMetric : reportMetrics)
        {
            if (reportMetric.second.stateSaved)
            {
                for (auto const &measType : reportMetric.second.measurementTypes)
                {
                    reportMetric.second.pMetric->getMeasurement(measType, reportMetric.first, sinkName, measurements);
                }
            }
        }

        //
        // Send the measurements to the sink for reporting
        sinkIt->second.pSink->reportMeasurements(measurements);
    }
}


std::map<std::string, MetricsReporter::MetricReportInfo> MetricsReporter::getMetricReportInfo(const std::vector<SinkInfo::measurementInfo> &reportMeasurements) const
{
    std::map<std::string, MetricsReporter::MetricReportInfo>  reportMetrics;

    //
    // If the sink has a specific list of measurements to report, build the list, otherwise report the default measurement for all metrics
    if (!reportMeasurements.empty())
    {
        for (auto const &measurementInfo : reportMeasurements)
        {
            std::string reportMetricName, measType;
            extractNameAndMeasurementType(measurementInfo.name, reportMetricName, measType);

            for (auto const &metricIt : metrics)
            {
                //
                // reportMetricName is to be a pattern to match against each metric name.
                // Not sure if we'll just user a '*' or regex. Will be added later. Just
                // do an exact match for now
                bool isMatch = metricIt.first == reportMetricName;

                //
                //
                // If the metric is a match, add it to the report info
                if (isMatch)
                {
                    auto it = reportMetrics.find(metricIt.first);
                    if (it == reportMetrics.end())
                    {
                        MetricReportInfo mri;
                        mri.pMetric = metricIt.second;
                        it = reportMetrics.insert({metricIt.first, mri}).first;
                    }
                    it->second.measurementTypes.emplace_back(measType);
                }
            }
        }
    }
    else
    {
        for (auto const &metricIt : metrics)
        {
            MetricReportInfo mri;
            mri.pMetric = metricIt.second;
            mri.measurementTypes.emplace_back("default");
            reportMetrics.insert({metricIt.first, mri});
        }
    }
    return reportMetrics;
}


void MetricsReporter::extractNameAndMeasurementType(const std::string &measurement, std::string &name, std::string &measType)
{
    name = measurement;
    measType = "default";

    //
    // Extract the type of measurement from the report namme
    auto parenPos = measurement.find_first_of('(');
    if (parenPos != std::string::npos)
    {
        auto endPos = measurement.find_first_of(')');
        if (endPos != std::string::npos)
        {
            measType = measurement.substr(parenPos + 1, endPos - parenPos - 1);
            name = measurement.substr(0, parenPos);
        }
    }
}


// true if new, false if already existed
SinkInfo *MetricsReporter::addSink(IMetricSink *pSink)
{
    SinkInfo sinkInfo;
    sinkInfo.pSink = pSink;
    auto rc = sinks.insert({pSink->getName(), sinkInfo});
    return &(rc.first->second);
}


bool MetricsReporter::init(IPropertyTree *pGlobalMetricsTree, IPropertyTree *pComponentMetricsTree)
{
    bool rc = false;

    //
    // Process the global config (which is really just sinks)
    processSinks(pGlobalMetricsTree->getElements("sinks"));

    //
    // Process component config
    if (pComponentMetricsTree != nullptr)
    {
        pComponentMetricsTree->getProp("@prefix", metricNamePrefix);

        //
        // Process sinks for the component, which should include metrics to report
        processSinks(pComponentMetricsTree->getElements("sinks"));
    }
    return rc;
}

void MetricsReporter::startCollecting()
{
    for (auto const &sinkIt : sinks)
    {
        sinkIt.second.pSink->startCollection(this);
    }
}


void MetricsReporter::stopCollecting()
{
    for (auto const &sinkIt : sinks)
    {
        sinkIt.second.pSink->stopCollection();
    }
}


bool MetricsReporter::processSinks(IPropertyTreeIterator *pSinkIt)
{
    bool rc = true;
    for (pSinkIt->first(); pSinkIt->isValid() && rc; pSinkIt->next())
    {
        SinkInfo *pSinkInfo;
        IPropertyTree &sinkTree = pSinkIt->query();

        StringBuffer cfgSinkType, cfgLibName, cfgSinkName, cfgProcName;
        sinkTree.getProp("@type", cfgSinkType);  // this one is required
        sinkTree.getProp("@libname", cfgLibName);
        sinkTree.getProp("@name", cfgSinkName);
        sinkTree.getProp("@instance_proc", cfgProcName);

        std::string sinkName = cfgSinkName.isEmpty() ? "default" : cfgSinkName.str();

        //
        // If sink already registered, use it, otherwise it's new
        auto sinkIt = sinks.find(sinkName);
        if (sinkIt != sinks.end())
        {
            pSinkInfo = &(sinkIt->second);
        }
        else
        {
            const char *type = cfgLibName.isEmpty() ? cfgSinkType.str() : cfgLibName.str();
            IPropertyTree *pSinkSettings = sinkTree.getPropTree("./settings");
            IMetricSink *pSink = MetricSink::getSinkFromLib(type, cfgProcName.str(), sinkName.c_str(), pSinkSettings);
            if (pSink != nullptr)
            {
                pSinkInfo = addSink(pSink);
            }
        }

        //
        // Retrieve metrics to be reported
        if (pSinkInfo != nullptr)
        {
            //
            // Now add defined metrics if present
            IPropertyTreeIterator *pSinkMetricsIt = sinkTree.getElements("metrics");
            for (pSinkMetricsIt->first(); pSinkMetricsIt->isValid(); pSinkMetricsIt->next())
            {
                StringBuffer metricName, description;
                pSinkMetricsIt->query().getProp("@name", metricName);
                pSinkMetricsIt->query().getProp("@description", description);
                SinkInfo::measurementInfo mi{std::string(metricName.str()), std::string(description.str())};
                pSinkInfo->reportMeasurements.emplace_back(mi);
            }
        }
        else
        {
            rc = false;
            // todo - need to decide how to handle bad metrics configurations. Hobble along with what is valid? Log the error?
        }
    }
    return rc;
}
