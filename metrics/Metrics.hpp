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
#include <utility>
#include <vector>
#include <map>
#include <atomic>
#include <chrono>
#include <mutex>
#include <memory>
#include <unordered_set>
#include "jiface.hpp"
#include "jptree.hpp"

#ifdef METRICSDLL
#define METRICS_API DECL_EXPORT
#else
#define METRICS_API DECL_IMPORT
#endif


namespace hpccMetrics {

enum ValueType {
    NONE,
    STRING,
    LONG,
    INTEGER,
    DOUBLE,
    FLOAT,
    DATE,
    DATE_NANOS,
    BOOLEAN
} ;


enum MetricType {
    CUSTOM,
    COUNTER,
    GAUGE
};

class IMetric;
class MeasurementBase
{
    public:
        MeasurementBase(const IMetric *_pMetric, std::string _name, std::string _description) :
                name{std::move(_name)},
                description{std::move(_description)},
                pMetric{_pMetric} {}
        std::string getName() const { return name; }
        std::string getDescription() const { return description; }
        const IMetric *getMetric() const { return pMetric; }
        virtual std::string valueToString() const = 0;

    protected:
        const IMetric *pMetric;
        std::string name;
        std::string description;
};


template<typename T>
class Measurement : public MeasurementBase
{
    public:
        Measurement(const IMetric *pMetric, const std::string &name, const std::string &description, T val) :
                MeasurementBase(pMetric, name, description),
                value{val}  { }
        std::string valueToString() const override { return std::to_string(value); }
        T getValue() const;

    protected:
        T value;
};


template <typename T>
T Measurement<T>::getValue() const { return value; }

typedef std::vector<std::shared_ptr<MeasurementBase>> MeasurementVector;

interface IMetric
{
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual MetricType getMetricType() const = 0;
    virtual ValueType getValueType() const = 0;
    //virtual void init(const std::string &context) = 0;
    virtual bool prepareMeasurements(const std::string &context) = 0;
    virtual void getMeasurement(const std::string &type, const std::string &measurementName,
                                const std::string &context, MeasurementVector &measurements) = 0;
};


class METRICS_API Metric : public IMetric
{
    public:
        virtual ~Metric() = default;
        std::string getName() const override { return name; }
        std::string getDescription() const override { return description; }
        ValueType getValueType() const override { return valueType; }
        MetricType getMetricType() const override { return metricType; }

    protected:
        // No one should be able to create one of these
        Metric(const char *_name, const char *_desc, ValueType _type, MetricType _metricType) :
                name{_name},
                description{_desc},
                valueType{_type},
                metricType{_metricType} { }

    protected:
        std::string name;
        std::string description;
        ValueType valueType;
        MetricType metricType;
};


class CounterMetric : public Metric
{
    public:
        CounterMetric(const char *name, const char *description) :
                Metric{name, description, ValueType::INTEGER, MetricType::COUNTER}  { }
        void inc(uint32_t val) { count.fetch_add(val, std::memory_order_relaxed);  }
        bool prepareMeasurements(const std::string &context) override;
        void getMeasurement(const std::string &measType, const std::string &measName, const std::string &context, MeasurementVector &measurements) override;


    protected:

        struct collectionValues {
            uint32_t lastValue = 0;
            uint32_t curValue = 0;
            std::chrono::time_point<std::chrono::high_resolution_clock> lastCollectedAt;
            std::chrono::time_point<std::chrono::high_resolution_clock> curCollectedAt;
        };

        std::atomic<uint32_t> count{0};
        std::map<std::string, collectionValues> contextValues;
};


template<typename T>
class GaugeMetric : public Metric {
    public:
        GaugeMetric(const char *name, const char *description, ValueType valueType) :
                Metric{name, description, valueType, MetricType::GAUGE}  { }
        void inc(T inc) { value += inc; }
        void dec(T dec) { value -= dec; }
        bool prepareMeasurements(const std::string &context) override { return true; }
        void getMeasurement(const std::string &type, const std::string &measurementName, const std::string &context, MeasurementVector &measurements) override;

    protected:
        std::atomic<T> value{0};
};

template<typename T>
void GaugeMetric<T>::getMeasurement(const std::string &type, const std::string &measurementName, const std::string &context, MeasurementVector &measurements)
{
    std::shared_ptr<MeasurementBase> pMeasurement;
    std::string reportName(measurementName);
    reportName.append(".").append(type);
    if (type == "reading" || type == "default")
    {
        pMeasurement = std::make_shared<Measurement<T>>(this, reportName, description, value.load());
        measurements.emplace_back(pMeasurement);
    }
    // else throw ?
}

class MetricsReporter;
interface IMetricSink
{
    virtual void startCollection(MetricsReporter *pReporter) = 0;
    virtual void stopCollection() = 0;
    virtual std::string getName() const = 0;
    virtual std::string getType() const = 0;
    virtual void reportMeasurements(const MeasurementVector &measurements) = 0;
};

extern "C" { typedef hpccMetrics::IMetricSink* (*getSinkInstance)(const char *, const IPropertyTree *pSettingsTree); }

class METRICS_API MetricSink : public IMetricSink
{
    public:
        virtual ~MetricSink() = default;
        std::string getName() const override { return name; }
        std::string getType() const override { return type; }
        static IMetricSink *getSinkFromLib(const char *type, const char *sinkName, const IPropertyTree *pSettingsTree);
        static IMetricSink *getSinkFromLib(const char *type, const char *getInstanceProcName, const char *sinkName, const IPropertyTree *pSettingsTree);

    protected:
        MetricSink(std::string _name, std::string _type) :
                name{std::move(_name)},
                type{std::move(_type)} { }

    protected:
        std::string name;
        std::string type;
        MetricsReporter *pReporter = nullptr;
};


struct SinkInfo
{
    IMetricSink *pSink = nullptr;
    struct measurementInfo
    {
        std::string name;
        std::string description;
    };
    std::vector<measurementInfo> reportMeasurements;
};


class METRICS_API MetricsReporter
{
    public:

        MetricsReporter() = default;
        virtual ~MetricsReporter() = default;
        bool init(IPropertyTree *pGlobalMetricsTree, IPropertyTree *pComponentMetricsTree);
        void addMetric(const std::shared_ptr<IMetric> &pMetric);
        void removeMetric(const std::shared_ptr<IMetric> &pMetric);
        void startCollecting();
        void stopCollecting();
        void collectMeasurements(const std::string &sinkName);
        SinkInfo *addSink(IMetricSink *pSink);

    protected:
        bool processSinks(IPropertyTreeIterator *pSinkIt);
        void initContexts();

        struct MetricReportInfo
        {
            std::shared_ptr<IMetric> pMetric;
            std::vector<std::string> measurementTypes;
            bool stateSaved;
        };

        std::map<std::string, MetricReportInfo> getMetricReportInfo(const std::vector<SinkInfo::measurementInfo> &reportMeasurements) const;
        static void extractNameAndMeasurementType(const std::string &measurement, std::string &name, std::string &measType);

    protected:
        StringBuffer metricNamePrefix;
        std::map<std::string, SinkInfo> sinks;
        std::map<std::string, std::shared_ptr<IMetric>> metrics;
        std::mutex reportMutex;
};

}
