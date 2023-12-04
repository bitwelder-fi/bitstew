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

#ifndef DOMAIN_TEST_ENVIRONMENT_HPP
#define DOMAIN_TEST_ENVIRONMENT_HPP

#include <gtest/gtest.h>
#include "trace_printer_mock.hpp"

#include <meta/library_config.hpp>
#include <meta/meta.hpp>

class DomainTestEnvironment : public ::testing::Test
{
protected:
    std::shared_ptr<MockPrinter> m_mockPrinter;
    void initializeDomain(bool multiThreaded, bool mockTracePrinter)
    {
        auto arguments = meta::LibraryArguments();
        arguments.taskScheduler.createThreadPool = multiThreaded;
        meta::Domain::instance().initialize(arguments);

        if (mockTracePrinter)
        {
            meta::Domain::instance().tracer()->clearTracePrinters();
            m_mockPrinter = std::make_shared<MockPrinter>();
            meta::Domain::instance().tracer()->addTracePrinter(m_mockPrinter);
        }
    }

    void TearDown() override
    {
        meta::Domain::instance().uninitialize();
    }
};

#endif // DOMAIN_TEST_ENVIRONMENT_HPP
