#pragma once

#include <string>
#include <vector>

namespace util
{

    /**
     * Joins a list of values into a string while separating
     * the values with a specified delimiter.
     * 
     * @param list      The list collection
     * @param delimiter The delimiter string
     * @returns         The list as string representation
     */
    std::string join(const std::vector<std::string>& list, std::string delimiter = ",")
    {
        std::string result = "";
        for (auto it = list.begin(); it != list.end(); it++)
        {
            if (it != list.begin())
            {
                result += ", ";
            }
            result += *it;
        }
        return result;
    }

}