#pragma once

#include <string>
#include <chrono>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace util
{

    std::string get_current_iso_timestamp()
    {
        using namespace boost::posix_time;
        ptime t = microsec_clock::universal_time();
        return to_iso_extended_string(t) + "Z";
    }

    template <typename Collection>
    std::string join(const Collection& list, std::string delimiter = ",")
    {
        std::string result = "";
        for (auto it = list.begin(); it != list.end(); it++)
        {
            if (it != list.begin())
            {
                result += ", ";
            }
            result += std::to_string(*it);
        }
        return result;
    }

}