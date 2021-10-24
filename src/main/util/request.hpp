#pragma once

#include <string>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <nlohmann/json.hpp>

#include "model/container.hpp"
#include "model/map/map.hpp"

namespace beast = boost::beast;
namespace http =  beast::http;
namespace net =   boost::asio;

using tcp = net::ip::tcp;
using json = nlohmann::ordered_json;

using namespace model;

namespace util
{

    /**
     * 
     */
    const std::string WARZONE_HOST = "warzone.com";

    /**
     * 
     */
    const std::string MAP_DETAILS_ENDPOINT = "/api/SetMapDetails";

    /**
     * https://www.warzone.com/wiki/Set_map_details_API
     */
    std::string create_payload(long map_id, const ConfigContainer& config, const map::Map& map)
    {
        json data;

        // Add the authentication information and map id
        data["mapID"] = map_id;
        data["email"] = config.email;
        data["APIToken"] = config.api_token;
    
        // Prepare the commands
        auto commands = json::array();
        
        // Add the territory commands
        for (const map::Territory& territory : map.territories())
        {
            // Add the setTerritoryName command
            auto name_command = json::object();
            name_command["command"] = "setTerritoryName";
            name_command["id"] = territory.id();
            name_command["name"] = territory.name();
            commands.push_back(name_command);

            // Add the setTerritoryCenter command
            auto center_command = json::object();
            center_command["command"] = "setTerritoryCenterPoint";
            center_command["id"] = territory.id();
            center_command["x"] = territory.center().x();
            center_command["y"] = territory.center().y();
            commands.push_back(center_command);

            // Add the addTerritoryConnection commands
            for (const map::TerritoryRef& neighbor : territory.neighbors())
            {
                auto connection_command = json::object();
                connection_command["command"] = "addTerritoryConnection";
                connection_command["id1"] = territory.id();
                connection_command["id2"] = neighbor.ref();
                connection_command["wrap"] = "Normal";
                commands.push_back(connection_command);
            }
        }

        // Add the bonus commands
        for (const map::Bonus& bonus : map.bonuses())
        {
            // Add the addBonus command
            auto add_bonus_command = json::object();
            add_bonus_command["command"] = "addBonus";
            add_bonus_command["name"] = bonus.name();
            add_bonus_command["armies"] = bonus.armies();
            add_bonus_command["color"] = bonus.color();
            commands.push_back(add_bonus_command);
            
            // Add the addTerritoryToBonus commands
            for (const map::BoundaryRef& child : bonus.children())
            {
                auto add_territory_command = json::object();
                add_territory_command["command"] = "addTerritoryToBonus";
                add_territory_command["id"] = child.ref();
                add_territory_command["bonusName"] = bonus.name();
                commands.push_back(add_territory_command);
            }
        }
        
        // Ignore super bonuses for now
        for (const map::SuperBonus& super_bonus : map.super_bonuses())
        {
            // Skip
        }

        // Add the commands to the json object
        data["commands"] = commands;

        // Serialize the json as string and return it
        return data.dump();
    }

    struct ResponseContainer
    {
        unsigned int code;
        std::string reason;
        std::string body;
    };

    /**
     * TODO perform this with HTTPS
     * https://github.com/boostorg/beast/issues/82
     */
    ResponseContainer post_metadata(const std::string& payload)
    {

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        // Look up the domain name
        auto const results = resolver.resolve(WARZONE_HOST, "http");

        // Make the connection on the IP address we get from a lookup
        stream.connect(results);

        // Set up an HTTP POST request message
        http::request<http::string_body> request;
        request.method(http::verb::post);
        request.target(MAP_DETAILS_ENDPOINT);
        request.version(11);
        
        // Add the request headers
        request.set(http::field::host, WARZONE_HOST);
        request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        request.set(http::field::content_type, "application/json; utf-8");
        request.set(http::field::accept, "*/*");

        // Add the request body
        request.body() = payload;
        request.prepare_payload();

        // Send the HTTP request to the remote host
        http::write(stream, request);

        // This buffer is used for reading and must be persisted
        beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> response;

        // Receive the HTTP response
        http::read(stream, buffer, response);

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        // not_connected happens sometimes
        // so don't bother reporting it.
        if(ec && ec != beast::errc::not_connected)
        {
            throw beast::system_error{ec};
        }
        // If we get here then the connection is closed gracefully

        // Convert the beast response to a Response object
        // and return it
        return ResponseContainer{
            response.result_int(),
            response.reason().to_string(),
            beast::buffers_to_string(response.body().data())
        };
    }

}