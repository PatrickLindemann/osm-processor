#pragma once

#include <string>

namespace http
{

    /**
     * A simple request wrapper.
     */
    class Request
    {
    protected:

        /**
         * The request payload string.
         */
        std::string m_payload;

    public:

        /* Constructors */

        Request() {}
        Request(std::string payload) : m_payload(payload) {}

        /* Accessors */

        const std::string payload() const
        {
            return m_payload;
        }

    };

}