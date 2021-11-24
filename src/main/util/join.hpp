#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <set>

namespace util
{

    /**
     * Joins a list of values into a string while separating
     * the values with a specified delimiter.
     * 
     * @param list      The list
     * @param delimiter The delimiter string
     * @returns         The list represented as string
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

    /**
     * Joins a set of values into a string while separating
     * the values with a specified delimiter.
     *
     * @param set       The set
     * @param delimiter The delimiter string
     * @returns         The set represented as string
     */
    template <typename T>
    std::string join(const std::set<T>& set, std::string delimiter = ",")
    {   
        std::stringstream stream;
        for (auto it = set.begin(); it != set.end(); it++)
        {
            if (it != set.begin())
            {
                stream << ", ";
            }
            stream << *it;
        }
        return stream.str();
    }

}