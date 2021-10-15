#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

/**
 * A collection of miscellaneous helper functions
 */
namespace util
{

    /**
     * Converts a number to a string of its representation
     * in the hexadecimal number system (base 16).
     * 
     * @param value The number
     * @returns     The hexadecimal number
     */
    template <typename T>
    std::string to_hex(T value)
    {
        std::stringstream stream;
        stream << std::hex << value;
        return stream.str();
    }

    /**
     * Retrieves the current timestamp, formats it according to
     * the ISO-8601 representation and returns it as string.
     * 
     * @returns The ISO-8601 timestamp string
     */
    std::string get_current_iso_timestamp()
    {
        using namespace boost::posix_time;
        ptime t = microsec_clock::universal_time();
        return to_iso_extended_string(t) + "Z";
    }

    /**
     * Joins a list of values into a string while separating
     * the values with a specified delimiter.
     * 
     * @param list      The list collection
     * @param delimiter The delimiter string
     * @returns         The list as string representation
     */
    template <typename ListType>
    std::string join(const ListType& list, std::string delimiter = ",")
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

    /**
     * Converts an HSL color value to RGB in HEX string representation.
     * Conversion formula adapted from http://en.wikipedia.org/wiki/HSL_color_space.
     * Assumes h is contained in the set [0, 360] and s, l in the set [0, 1]
     * and returns the RGB color representation as HEX string #RRGGBB.
     * @param h The hue
     * @param s The saturation
     * @param l The lightness
     * @returns The HEX color representation
     */
    std::string hsl_to_hex(int h, float s, float l)
    {
        float a = s * std::min(l, 1 - l);
        auto f = [&h, &l, &a](int n) -> float {
            auto k = (n + h / 30) % 12;
            return l - a * std::max(-1, std::min({ k - 3, 9 - k, 1 }));
        };
        short r = std::round(f(0) * 255);
        short g = std::round(f(8) * 255);
        short b = std::round(f(4) * 255);
        return "#" + to_hex(r) + to_hex(g) + to_hex(b);
    }

}