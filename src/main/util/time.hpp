#pragma once

#include <chrono>
#include <string>

namespace util
{

    /**
     * Retrieves the current timestamp with second precision, formats
     * it according to the ISO-8601 representation and returns it as string.
     * 
     * @returns The timestamp string in format "YYYY-MM-DD'T'hh:mm:ss'Z'"
     */
    std::string get_current_iso_timestamp()
    {
        time_t now;
        time(&now);
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", gmtime(&now));
        return std::string(buffer) + "Z";
    }

}