#pragma once

#include <chrono>
#include <string>

namespace util
{

    /**
     * Retrieves the current timestamp, formats it according to
     * the ISO-8601 representation and returns it as string.
     * 
     * @returns The ISO-8601 timestamp string
     */
    std::string get_current_iso_timestamp()
    {
        time_t now;
        time(&now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
        // this will work too, if your compiler doesn't support %F or %T:
        return std::string(buffer);
    }

}