#pragma once

#include <string>

namespace model
{

    /**
     * A container for the program configuration
     */
    struct Config
    {
        std::string email;
        std::string api_token;
    };

}