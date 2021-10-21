#pragma once

#include <iostream>

namespace util
{

    /* Functions */

    /**
     * Print the program title to a specified stream.
     * 
     * @param stream the output stream
     */
    template <typename StreamType>
    void print_title(StreamType& stream)
    {
        stream << ""
            << " _       __                                     __  ___                            __            "        << '\n'
            << "| |     / /___ __________  ____  ____  ___     /  |/  /___ _____  ____ ___  ____ _/ /_____  _____"        << '\n'
            << "| | /| / / __ `/ ___/_  / / __ \\/ __ \\/ _ \\   / /|_/ / __ `/ __ \\/ __ `__ \\/ __ `/ //_/ _ \\/ ___/"  << '\n'
            << "| |/ |/ / /_/ / /    / /_/ /_/ / / / /  __/  / /  / / /_/ / /_/ / / / / / / /_/ / ,< /  __/ /    "        << '\n'
            << "|__/|__/\\__,_/_/    /___/\\____/_/ /_/\\___/  /_/  /_/\\__,_/ .___/_/ /_/ /_/\\__,_/_/|_|\\___/_/     "  << '\n'
            << "                                                        /_/                                      "        << std::endl;

    }

}