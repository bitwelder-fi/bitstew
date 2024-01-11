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

#include "test_log_fixtures.hpp"
#include <meta/tasks/task_scheduler.hpp>

using namespace meta_test;

#define CORE_LOG(tracer, logLevel) \
meta::LogLine(tracer, logLevel, __FUNCTION__, __FILE__, __LINE__)()

#define TEST_LOG_FATAL(text) CORE_LOG(m_tracer.get(), meta::LogLevel::Fatal) << text
#define TEST_LOG_ERROR(text) CORE_LOG(m_tracer.get(), meta::LogLevel::Error) << text
#define TEST_LOG_WARNING(text) CORE_LOG(m_tracer.get(), meta::LogLevel::Warning) << text
#define TEST_LOG_INFO(text) CORE_LOG(m_tracer.get(), meta::LogLevel::Info) << text
#define TEST_LOG_DEBUG(text) CORE_LOG(m_tracer.get(), meta::LogLevel::Debug) << text

TEST_F(FakeTracerTest, testMocPrinter)
{
    MockPrinter printer;
    EXPECT_CALL(printer, log("A"));

    printer.write("A");
}

TEST_F(FakeTracerTest, testMultipleLogLine)
{
    m_tracer->addTracePrinter(std::make_shared<MockPrinter>());
    // ::testing::InSequence sequence;
    auto mock = m_tracer->getPrinterAt<MockPrinter>(0u);
    EXPECT_CALL(*mock, log("second line"));
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("debug line"));
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("first line"));

    meta::LogLine log(m_tracer.get(), meta::LogLevel::Debug, __PRETTY_FUNCTION__, __FILE__, __LINE__);
    log() << "first line";
    TEST_LOG_DEBUG("second line");
    TEST_LOG_DEBUG("debug line");
}

TEST_F(FakeTracerTest, testFileLineDecorator)
{
    const auto path = std::filesystem::current_path();
    meta::TracePrinterPtr printer = std::make_shared<MockPrinter>();
    printer = std::make_shared<meta::MessageSeparator>(printer);
    printer = std::make_unique<meta::FileLineDecorator>(printer, path.native() + std::filesystem::path::preferred_separator);
    m_tracer->addTracePrinter(printer);

    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("tests/test_log.cpp:65 - decorated with file and line"));
    TEST_LOG_DEBUG("decorated with file and line");
}

TEST_F(FakeTracerTest, testFunctionDecorator)
{
    meta::TracePrinterPtr printer = std::make_shared<MockPrinter>();
    printer = std::make_shared<meta::MessageSeparator>(printer);
    printer = std::make_unique<meta::FunctionDecorator>(printer);
    m_tracer->addTracePrinter(printer);

    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("TestBody - decorated with function name"));
    TEST_LOG_DEBUG("decorated with function name");
}

TEST_F(FakeTracerTest, testLogLevelDecorator)
{
    meta::TracePrinterPtr printer = std::make_shared<MockPrinter>();
    printer = std::make_shared<meta::MessageSeparator>(printer);
    printer = std::make_unique<meta::LogLevelDecorator>(printer);
    m_tracer->addTracePrinter(printer);

    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[DEBUG] - decorated with log level"));
    TEST_LOG_DEBUG("decorated with log level");
}

TEST_F(FakeTracerTest, testComplexDecorator)
{
    setupLogLevelFileLineFunctionHeader();
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[DEBUG] tests/test_log.cpp:94 TestBody - complex header"));
    TEST_LOG_DEBUG("complex header");
}

TEST_F(FakeTracerTest, testRestrictLogLevel)
{
    setupLogLevelFileLineFunctionHeader();
    m_tracer->setLogLevel(meta::LogLevel::Info);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[DEBUG] tests/test_log.cpp:102 TestBody - testRestrictLogLevelToInfo")).Times(0);
    TEST_LOG_DEBUG("testRestrictLogLevelToInfo");
}

INSTANTIATE_TEST_SUITE_P(LogLevelTests, LogLineTestParam,
                            ::testing::Values(
                                LogLevelTestParam(meta::LogLevel::Debug, 1, 1, 1, 1, 1),
                                LogLevelTestParam(meta::LogLevel::Info, 1, 1, 1, 1, 0),
                                LogLevelTestParam(meta::LogLevel::Warning, 1, 1, 1, 0, 0),
                                LogLevelTestParam(meta::LogLevel::Error, 1, 1, 0, 0, 0),
                                LogLevelTestParam(meta::LogLevel::Fatal, 1, 0, 0, 0, 0),
                                LogLevelTestParam(meta::LogLevel::Suppressed, 0, 0, 0, 0, 0)));

TEST_P(LogLineTestParam, testRestrictLogLevelToInfo)
{
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[FATAL] - testRestrictLogLevelToInfo")).Times(fatalCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[ERROR] - testRestrictLogLevelToInfo")).Times(errorCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[WARNING] - testRestrictLogLevelToInfo")).Times(warningCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[INFO] - testRestrictLogLevelToInfo")).Times(infoCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[DEBUG] - testRestrictLogLevelToInfo")).Times(debugCount);
    TEST_LOG_FATAL("testRestrictLogLevelToInfo");
    TEST_LOG_ERROR("testRestrictLogLevelToInfo");
    TEST_LOG_WARNING("testRestrictLogLevelToInfo");
    TEST_LOG_INFO("testRestrictLogLevelToInfo");
    TEST_LOG_DEBUG("testRestrictLogLevelToInfo");
}


INSTANTIATE_TEST_SUITE_P(LogLevelTests, MetaTraceTest,
                         ::testing::Values(
                             LogLevelTestParam(meta::LogLevel::Debug, 1, 1, 1, 1, 1),
                             LogLevelTestParam(meta::LogLevel::Info, 1, 1, 1, 1, 0),
                             LogLevelTestParam(meta::LogLevel::Warning, 1, 1, 1, 0, 0),
                             LogLevelTestParam(meta::LogLevel::Error, 1, 1, 0, 0, 0),
                             LogLevelTestParam(meta::LogLevel::Fatal, 1, 0, 0, 0, 0),
                             LogLevelTestParam(meta::LogLevel::Suppressed, 0, 0, 0, 0, 0)));
TEST_P(MetaTraceTest, testMetaTracer)
{
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[FATAL] - testMetaTracer")).Times(fatalCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[ERROR] - testMetaTracer")).Times(errorCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[WARNING] - testMetaTracer")).Times(warningCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[INFO] - testMetaTracer")).Times(infoCount);
    EXPECT_CALL(*m_tracer->getPrinterAt<MockPrinter>(0u), log("[DEBUG] - testMetaTracer")).Times(debugCount);
    META_LOG_FATAL("testMetaTracer");
    META_LOG_ERROR("testMetaTracer");
    META_LOG_WARNING("testMetaTracer");
    META_LOG_INFO("testMetaTracer");
    META_LOG_DEBUG("testMetaTracer");
    // Ensure the log happens.
    meta::Library::instance().taskScheduler()->schedule(std::chrono::milliseconds(10));
}
