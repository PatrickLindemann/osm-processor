#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace util
{

    /**
     * Joins a list of values into a string while separating
     * the values with a specified delimiter.
     * 
     * @param list      The list
     * @param delimiter The delimiter string
     * @returns         The list as string representation
     */
    template <typename T>
    std::string join(const std::vector<T>& list, std::string delimiter = ",")
    {
        std::stringstream stream;
        for (auto it = list.begin(); it != list.end(); it++)
        {
            if (it != list.begin())
            {
                stream << ", ";
            }
            stream << *it;
        }
        return stream.str();
    }

}