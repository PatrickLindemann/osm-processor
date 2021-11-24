#pragma once

#include <string>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "http/mapdata_request.hpp"
#include "http/request.hpp"
#include "http/response.hpp"

namespace http
{

    namespace beast = boost::beast;
    namespace http =  beast::http;
    namespace net =   boost::asio;

    using tcp = net::ip::tcp;

    /**
     * The request protocol. The default is HTTP.
     * FIXME: Perform requests with HTTPS https://github.com/boostorg/beast/issues/82
     */
    const std::string UPLOAD_PROTOCOL = "http";

    /**
     * The request host address. This defaults to the official Warzone domain.
     */
    const std::string UPLOAD_HOST = "warzone.com";

    /**
     * The request endpoint. This defaults to the SetMapDetails API, which is used
     * to add metadata to an existing map.
     */
    const std::string UPLOAD_ENDPOINT = "/api/SetMapDetails";

    template <typename T>
    class MapdataUploader
    {
    public:

        /* Constructors */

        MapdataUploader() {}

        /* Methods */

        /**
         * Sends an upload request with the specified map metadata to Warzone.
         * 
         * @param payload The upload request payload
         */
        Response send(const MapdataRequest<T>& request)
        {
            // The io_context is required for all I/O
            net::io_context ioc;

            // These objects perform our I/O
            tcp::resolver resolver(ioc);
            beast::tcp_stream stream(ioc);

            // Look up the domain name
            auto const lookup = resolver.resolve(UPLOAD_HOST, UPLOAD_PROTOCOL);

            // Make the connection on the IP address we get from a lookup
            stream.connect(lookup);

            // Prepare the HTTP POST request message
            http::request<http::string_body> http_request;
            http_request.method(http::verb::post);
            http_request.target(UPLOAD_ENDPOINT);
            http_request.version(11);
            
            // Add the request headers
            http_request.set(http::field::host, UPLOAD_HOST);
            http_request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            http_request.set(http::field::content_type, "application/json; utf-8");
            http_request.set(http::field::accept, "*/*");

            // Add the request body
            http_request.body() = request.payload();
            http_request.prepare_payload();

            // Send the HTTP request to the remote host
            http::write(stream, http_request);

            // This buffer is used for reading and must be persisted
            beast::flat_buffer buffer;

            // Declare a container to hold the response
            http::response<http::dynamic_body> http_response;

            // Receive the HTTP response
            http::read(stream, buffer, http_response);

            // Gracefully close the socket
            beast::error_code ec;
            stream.socket().shutdown(tcp::socket::shutdown_both, ec);

            // not_connected happens sometimes
            // so don't bother reporting it.
            if(ec && ec != beast::errc::not_connected)
            {
                throw beast::system_error{ec};
            }

            // If we get here, the connection is closed gracefully

            return Response {
                http_response.result_int(),
                http_response.reason().to_string(),
                beast::buffers_to_string(http_response.body().data())
            };
        }

    };

}