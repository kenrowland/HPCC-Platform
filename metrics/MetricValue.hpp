//
// Created by rowlke01 on 3/6/20.
//
#pragma once

#include <string>

namespace hpcc_metrics {

    class MetricValueBase {
        public:

            explicit MetricValueBase(std::string name) : m_name{std::move(name)} {}

            virtual ~MetricValueBase() = default;

            virtual std::string toString() const = 0;

            virtual const std::string &getName() const { return m_name; }

        protected:

            std::string m_name;
    };


    template<typename T>
    class MetricValue : public MetricValueBase {
        public:
            MetricValue(const std::string &name, T value) :
                    m_value{value},
                    MetricValueBase(name) {}

            std::string toString() const override {
                return std::to_string(m_value);
            }

        protected:

            T m_value;
    };

}
