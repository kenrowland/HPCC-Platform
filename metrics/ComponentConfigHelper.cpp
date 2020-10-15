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

/*

 Some notes on configuration

Global level:

 from queryGlobalConfig

 Defines the storage sinks for the platform and how reporting is done
 This information should not be repeated or changed at the component level.
 It can, however, be expanded by adding an addition sink

  metrics:
    name: config_name
    sinks:                  # array of sink definitions (usually one)
      - type: sinktype        # required - indicates which backend system it supports
        name: sinkname        # optional unique name for the sink. If no name is given, use the sink type as the name
        libname: libname      # optional name of lib to load (overrides the type when finding the lib to load)
        instance_proc: name   # optional way to specify the get instance proc name
        settings:             # sink specific configuration data based on sink type
          key1: data1         # key/value pairs that are passed to the sink during config
          key2: data2
          Key3:
            key3.1: data3.1
            key3.2: data3.2
          ipaddr: {{sys.io.helm/kube.xxx}}
      - type: sink2type

    report_trigger:
      type: trigger_type      # type of trigger (may be an external lib)
      libname: lib name       # Optional library name to override the trigger type
      instance_proc: name     # optional name of get instance proc to look up
      settings:               # configuration data for the trigger passed in during config
        key1: data1
        key2: data2

    report_config:          # this report config is applied to all components, done as a "best fit"
      - sink: sinkname      # name of the sink defined in the sinks section above
        metric_sets:        # defines the metric sets that are part of the report
          - set1
          - set2

Component level
 component may define any of the same items allowed at the global level, but should not override any, only add
 (not sure about that statement yet)

 from queryComponentConfig

  metrics:                            # may define a sink if necessary, cannot define a trigger
    prefix: {{name}}                  # define domain prefix for all sets (probably a substituted variable reference, could include cluster and component information
    set_prefixes:
      - setname1: prefix
      - setname2: prefix

    sinks:   optionally define more sinks for the component

    # By default, all metric sets are reported to all sinks. Any sink listed below is a modification
    report_config:         # same as global level, but optional not supported
      - sink: sinkname     # we are overriding the report to this sink
        metric_sets:       # array of metric sets to assign to the sink referenced by name
          - set1
          - set2
          - setn
      - sink: sinkname
        metric_sets
          - set1
          - set2

*/


#include "ComponentConfigHelper.hpp"

using namespace hpccMetrics;

bool ComponentConfigHelper::init(IPropertyTree *pGlobalMetricsTree, IPropertyTree *pComponentMetricsTree)
{
//    pGlobalConfig = _pGlobalConfig;
//    pComponentConfig = _pComponentConfig;

    //
    // Process global
    processSinks(pGlobalMetricsTree->getElements("sinks"));

    //
    // Now the trigger
    IPropertyTree *pTriggerTree = pGlobalMetricsTree->getPropTree("report_trigger");
    createTrigger(pTriggerTree);

    processReportConfig(pGlobalMetricsTree->getElements("report_config"));

    //
    // Component config
    if (pComponentMetricsTree != nullptr)
    {
        pComponentMetricsTree->getProp("@prefix", componentReportingPrefix);

        //
        // Any additional sinks defined for the component?
        processSinks(pComponentMetricsTree->getElements("sinks"));

    }
    return true;
}


bool ComponentConfigHelper::processSinks(IPropertyTreeIterator *pSinkIt)
{
    if (pSinkIt->isValid())
    {
        for (pSinkIt->first(); pSinkIt->isValid(); pSinkIt->next())
        {
            IPropertyTree &sinkTree = pSinkIt->query();

            StringBuffer cfgSinkType, cfgLibName, cfgSinkName, cfgProcName;
            sinkTree.getProp("@type", cfgSinkType);  // this one is required
            sinkTree.getProp("@libname", cfgLibName);
            sinkTree.getProp("@name", cfgSinkName);
            sinkTree.getProp("@instance_proc", cfgProcName);

            const char *type = cfgLibName.isEmpty() ? cfgSinkType.str() : cfgLibName.str();
            IPropertyTree *pSinkSettings = sinkTree.getPropTree("setting");
            IMetricSink *pSink = MetricSink::getSinkFromLib(type, cfgProcName.str(),
                                                            cfgSinkName.isEmpty() ? "default" : cfgSinkName.str(),
                                                            pSinkSettings);

            //
            // Add the sink, but make sure it's unique
            auto insertIt = sinks.insert({pSink->getName(), pSink});
            if (!insertIt.second)
            {
                // set an error message for a getLastError call?
                return false;
            }
        }
    }
    return true;
}


void ComponentConfigHelper::processReportConfig(IPropertyTreeIterator *pReportSinksIt)
{
    if (pReportSinksIt->isValid())
    {
        for (pReportSinksIt->first(); pReportSinksIt->isValid(); pReportSinksIt->next())
        {
            IPropertyTree &reportSinkTree = pReportSinksIt->query();
            StringBuffer _sinkName;
            reportSinkTree.getProp("@sink", _sinkName);

            std::string sinkName(_sinkName.str());
            auto sinkNameIt = metricSetsBySinkName.find(sinkName);
            if (sinkNameIt == metricSetsBySinkName.end())
            {
                metricSetsBySinkName.insert({sinkName, std::unordered_set<std::string>()});
            }

            IPropertyTreeIterator *pMetricSetNamesIt = reportSinkTree.getElements("metric_sets");
            if (pMetricSetNamesIt->isValid())
            {
                StringBuffer _setName;
                for (pMetricSetNamesIt->first(); pMetricSetNamesIt->isValid(); pMetricSetNamesIt->next())
                {
                    pMetricSetNamesIt->query().getProp("", _setName);
                    metricSetsBySinkName[sinkName].emplace(std::string(_setName.str()));
                }
            }
        }
    }
}


bool ComponentConfigHelper::createTrigger(IPropertyTree *pTriggerTree)
{
    StringBuffer cfgTriggerType, cfgLibName, cfgProcName;
    pTriggerTree->getProp("@type", cfgTriggerType);  // this one is required
    pTriggerTree->getProp("@libname", cfgLibName);
    pTriggerTree->getProp("@instance_proc", cfgProcName);
    const char *type = cfgLibName.isEmpty() ? cfgTriggerType.str() : cfgLibName.str();
    IPropertyTree *pTriggerSettings = pTriggerTree->getPropTree("settings");
    pTrigger = MetricsReportTrigger::getTriggerFromLib(type, cfgProcName.str(), pTriggerSettings);
    return (pTrigger != nullptr);
}



bool ComponentConfigHelper::addMetricToSet(const std::shared_ptr<IMetric> &pMetric, const char *metricReportingPrefix, const char *_setName)
{
    std::string setName(_setName);

    auto it = metricsBySetName.find(setName);
    if (it == metricsBySetName.end())
    {
        metricsBySetName[setName] = std::vector<std::shared_ptr<IMetric>>();
    }
    metricsBySetName[setName].emplace_back(pMetric);

    //
    // Set the reporting name for the metric
    std::string reportingName(componentReportingPrefix);
    reportingName.append(metricReportingPrefix);
    reportingName.append(pMetric->getName());
    pMetric->setReportingName(reportingName);
    return true;
}


bool ComponentConfigHelper::start()
{
    buildReportConfig();
    pReporter = new MetricsReporter(reportConfig, pTrigger);
    pReporter->start();
    return true;
}


void ComponentConfigHelper::stop()
{
    pTrigger->stop();
}


bool ComponentConfigHelper::buildReportConfig()
{
    //
    // Create the metric sets
    std::map<std::string, std::shared_ptr<IMetricSet>> metricSets;
    for (auto const &it : metricsBySetName)
    {
        std::shared_ptr<IMetricSet> pMetricSet = std::make_shared<MetricSet>(it.first.c_str(), "", it.second);
        metricSets.insert({it.first, pMetricSet});
    }

    //
    // For each sink assign it its configured metric sets. If no metricsets have been
    // actively assigned to a sink, then by default assign all metricsets to it
    for (auto const &sinkIt : sinks)
    {
        auto metricSetToSinkIt = metricSetsBySinkName.find(sinkIt.first);
        if (metricSetToSinkIt == metricSetsBySinkName.end())
        {
            for (auto const &metricSetIt : metricSets)
            {
                reportConfig.addReportConfig(sinkIt.second, metricSetIt.second);
            }
        }
        else
        {
            for (auto const &metricSetName : metricSetToSinkIt->second)
            {
                reportConfig.addReportConfig(sinkIt.second, metricSets[metricSetName]);
            }
        }
    }
    return true;
}
