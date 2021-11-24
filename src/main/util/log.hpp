#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

namespace util
{

    using std::chrono::steady_clock;
    using std::chrono::duration_cast;

    template <typename StreamType = std::ostream>
    class Logger
    {
    protected:

        /* Members */

        /**
         *
         */
        StreamType& m_stream;

        /**
         *
         */
        std::size_t m_steps;

        /**
         *
         */
        std::size_t m_step;

       /**
        *
        */
        std::vector<steady_clock::time_point> m_times = {};

    public:

        /* Constructors */

        Logger(StreamType& stream) : m_stream(stream) {}
        Logger(StreamType& stream, std::size_t steps) : m_stream(stream), m_steps(steps), m_times(m_steps) {}

        /* Accessors */

        const std::size_t& steps() const
        {
            return m_steps;
        }

        const std::vector<steady_clock::time_point>& times() const
        {
            return m_times;
        }

        void reset()
        {
            m_step = 0;
            m_times = {};
        }

        void set_steps(std::size_t steps)
        {
            m_steps = steps;
        }

    protected:

        /* Helper Methods */

        std::string step_header(std::size_t step, std::size_t total)
        {
            return "[Step " + std::to_string(step) + "/" + std::to_string(total) + "] ";
        }

    public:

        /* Methods */

        StreamType& log()
        {
            return m_stream;
        }

        StreamType& debug()
        {
            return m_stream << "[Debug] ";
        }

        StreamType& info()
        {
            return m_stream << "[Info] ";
        }

        StreamType& warn()
        {
            return m_stream << "[Warning] ";
        }

        StreamType& error()
        {
            return m_stream << "[Error] ";
        }

        StreamType& start()
        {
            m_times.push_back(std::chrono::steady_clock::now());
            return m_stream << step_header(++m_step, m_steps);
        }

        StreamType& step()
        {
            return m_stream << step_header(m_step, m_steps);
        }

        void finish()
        {
            m_times.push_back(std::chrono::steady_clock::now());
            m_stream << step_header(m_step, m_steps) << "Finished after " << duration(m_step - 1, m_step) << " ms." << std::endl;
        }

        void end()
        {
            m_stream << "[End] Finished. Total execution time was " << total_duration() << " ms." << std::endl;
        }

        /* Misc */

        long duration(std::size_t start, std::size_t end)
        {
            return duration_cast<std::chrono::milliseconds>(m_times.at(end) - m_times.at(start)).count();
        }

        long total_duration()
        {
            steady_clock::time_point last = *std::max_element(m_times.begin(), m_times.end());
            return duration_cast<std::chrono::milliseconds>(last - m_times.front()).count();
        }

    };

}