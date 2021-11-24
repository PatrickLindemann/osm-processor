#pragma once

#include <algorithm>
#include <regex>
#include <stdexcept>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

#include "model/types.hpp"
#include "util/join.hpp"

namespace fs = boost::filesystem;

namespace util
{

    /* Constants */

    const std::vector<std::string> ALLOWED_OSM_FORMATS{ "osm", "pbf", "osm.pbf" };


    /* Simple Validation Functions */

    void validate_id(long& id, std::string name)
    {
        if (id < 0)
        {
            throw std::invalid_argument("Invalid id specified for parameter '" + name + "'. Ids must be positive integers");
        }
    }

    /**
     * 
     */
    void validate_file(fs::path& path, std::string name)
    {
        // Verify that file was provided
        if (path.string() == "")
        {
            throw std::invalid_argument("No file specified for parameter '" + name + "'");
        }
        // Verify that the file exists by canonicalizing it
        try
        {
            path = fs::canonical(path);
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Specified file '" + path.string() + "' for parameter '" + name + "' does not exist"); 
        }
    }

    /**
     * 
     */
    void validate_dir(fs::path& path, std::string name)
    {
        // Verify that file was provided
        if (path.string() == "")
        {
            throw std::invalid_argument("No directory specified for parameter '" + name + "'");
        }
        // Verify that the file exists by canonicalizing it
        try
        {
            path = fs::canonical(path);
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Specified directory '" + path.string() + "' for parameter '" + name + "' does not exist"); 
        }
        if (!fs::is_directory(path))
        {
            throw std::invalid_argument("The path specified for parameter '" + name + "' is not a directory");
        }
    }

    void validate_format(std::string& format, std::string name)
    {
        // Normalize format
        format = std::regex_replace(format, std::regex("^\\."), "");
        boost::to_lower(format);
        // Check if format is valid
        if (std::find(ALLOWED_OSM_FORMATS.begin(), ALLOWED_OSM_FORMATS.end(), format) == ALLOWED_OSM_FORMATS.end())
        {
            throw std::invalid_argument("The specified format " + format + "is not supported.\nSupported formats are " + util::join(ALLOWED_OSM_FORMATS));
        }
    }

    void validate_epsilon(double& epsilon, std::string name)
    {
        if (epsilon < 0)
        {
            throw std::invalid_argument(
                "Invalid epsilon " + std::to_string(epsilon) + " for parameter '" + name + "'."
                + "Epsilons have to be positive or equal to 0 (none)"
            );
        }
    }


    /* Dependent Validation Functions */

    void validate_levels(model::level_type& territory_level, const std::vector<model::level_type>& bonus_levels)
    {
        // Check if territory level was set to auto
        if (territory_level == 0)
        {
            if (!bonus_levels.empty())
            {
                throw std::invalid_argument(
                    "Territory level was set to 0 (auto), but bonus levels were specified."
                    " Bonus levels can only be specified if the territory level was set"
                );
            }
            return;
        }
        // Validate territory level
        if (territory_level < 1 || territory_level > 12)
        {
            throw std::invalid_argument(
                "Invalid territory level " + std::to_string(territory_level) + " specified."
                + " Territory levels must be integers between 1 and 12, or 0 (auto)"
            );
        }
        // Validate bonus levels
        if (bonus_levels.size() > 2)
        {
            throw std::invalid_argument(
                "Specified more than 2 bonus levels: " + util::join(bonus_levels)
                + ". Only one or two bonus levels are supported"
            );
        }
        if (bonus_levels.size() == 2 && bonus_levels.at(0) == bonus_levels.at(1))
        {
            throw std::invalid_argument(
                "Specified duplicate bonus levels: " + util::join(bonus_levels) + "."
            );
        }
        for (const model::level_type& bonus_level : bonus_levels)
        {
            if (bonus_level < 1 || bonus_level > 12)
            {
                throw std::invalid_argument(
                    "Invalid bonus level " + std::to_string(bonus_level) + " specified."
                    + " Bonus levels must be integers between 1 and 12"
                );
            }
            else if (bonus_level >= territory_level)
            {
                throw std::invalid_argument(
                    "Bonus level " + std::to_string(bonus_level) + " is greater or equal"
                    + " to the territory level " + std::to_string(territory_level)
                );
            }
        }
    }

    void validate_dimensions(int& width, int& height)
    {
        if (width < 0)
        {
            throw std::invalid_argument(
                "Invalid width " + std::to_string(width) + " specified." 
                + " Dimensions have to be greater or equal to 0 (auto)"
            );
        }
        else if (height < 0)
        {
            throw std::invalid_argument(
                "Invalid height " + std::to_string(height) + " specified." 
                + " Dimensions have to be greater or equal to 0 (auto)"
            );
        }
        else if (width == 0 && height == 0)
        {
            throw std::invalid_argument(
                "Width and height are both set to 0 (auto). At least one dimension must be set"
            );

        }
    }

}
