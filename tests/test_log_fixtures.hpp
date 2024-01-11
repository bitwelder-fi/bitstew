/*
 * Copyright (C) 2023 bitWelder
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see
 * <http://www.gnu.org/licenses/>
 */

#include <gtest/gtest.h>

#include <meta/meta.hpp>
#include <meta/library_config.hpp>
#include <meta/log/trace.hpp>
#include <meta/log/trace.hpp>
#include <meta/log/trace_printer.hpp>
#include "utils/trace_printer_mock.hpp"

#include <filesystem>
#include <tuple>

namespace meta_test
{

class FakeTracer : public meta::Tracer
{
public:
    explicit FakeTracer() :
        meta::Tracer(nullptr)
    {
    }
};

class FakeTracerTest : public ::testing::Test
{
public:
    std::shared_ptr<FakeTracer> m_tracer;

    void SetUp() override
    {
        m_tracer = std::make_shared<FakeTracer>();
    }
    void TearDown() override
    {
        m_tracer.reset();
    }

    void setupLogLevelFileLineFunctionHeader()
    {
        const auto path = std::filesystem::current_path();
        meta::TracePrinterPtr printer = std::make_shared<MockPrinter>();
        printer = std::make_shared<meta::MessageSeparator>(printer);
        printer = std::make_unique<meta::FunctionDecorator>(printer);
        printer = std::make_unique<meta::FileLineDecorator>(printer, path.native() + std::filesystem::path::preferred_separator);
        printer = std::make_unique<meta::LogLevelDecorator>(printer);
        m_tracer->addTracePrinter(printer);
    }
};


using LogLevelTestParam = std::tuple<meta::LogLevel, int, int, int, int, int>;
class LogLineTestParam : public FakeTracerTest, public ::testing::WithParamInterface<LogLevelTestParam>
{
public:
    int fatalCount = 0;
    int errorCount = 0;
    int warningCount = 0;
    int infoCount = 0;
    int debugCount = 0;

    void SetUp() override
    {
        FakeTracerTest::SetUp();

        meta::TracePrinterPtr printer = std::make_shared<MockPrinter>();
        printer = std::make_shared<meta::MessageSeparator>(printer);
        printer = std::make_unique<meta::LogLevelDecorator>(printer);
        m_tracer->addTracePrinter(printer);

        auto params = GetParam();
        m_tracer->setLogLevel(std::get<meta::LogLevel>(params));
        fatalCount = std::get<1>(params);
        errorCount = std::get<2>(params);
        warningCount = std::get<3>(params);
        infoCount = std::get<4>(params);
        debugCount = std::get<5>(params);
    }
};

class MetaTraceTest : public ::testing::Test, public ::testing::WithParamInterface<LogLevelTestParam>
{
protected:
    meta::Tracer* m_tracer;
    int fatalCount = 0;
    int errorCount = 0;
    int warningCount = 0;
    int infoCount = 0;
    int debugCount = 0;

    void SetUp() override
    {
        meta::Library::instance().initialize(meta::LibraryArguments());
        m_tracer = meta::Library::instance().tracer();
        m_tracer->clearTracePrinters();

        meta::TracePrinterPtr printer = std::make_shared<MockPrinter>();
        printer = std::make_shared<meta::MessageSeparator>(printer);
        printer = std::make_shared<meta::LogLevelDecorator>(printer);
        m_tracer->addTracePrinter(printer);

        auto params = GetParam();
        m_tracer->setLogLevel(std::get<meta::LogLevel>(params));
        fatalCount = std::get<1>(params);
        errorCount = std::get<2>(params);
        warningCount = std::get<3>(params);
        infoCount = std::get<4>(params);
        debugCount = std::get<5>(params);
    }
    void TearDown() override
    {
        m_tracer->clearTracePrinters();
        meta::Library::instance().uninitialize();
    }
};

}
