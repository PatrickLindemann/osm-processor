#pragma once

#include <string>

namespace http
{

    /**
     * A simple response wrapper.
     */
    class Response
    {   
    protected:

        /* Members */

        /**
         * The response HTTP code.
         */
        unsigned int m_code;

        /**
         * The response HTTP reason phrase.
         * https://www.w3.org/Protocols/rfc2616/rfc2616-sec6.html
         */
        std::string m_reason;

        /**
         * The response body.
         */
        std::string m_body;

    public:
    
        /* Constructors */

        Response() {}
        Response(unsigned int code, std::string reason, std::string body)
        : m_code(code), m_reason(reason), m_body(body) {}

        /* Accessors */

        const unsigned int code() const
        {
            return m_code;
        }

        const std::string reason() const
        {
            return m_reason;
        }

        const std::string body() const
        {
            return m_body;
        }

    };

}