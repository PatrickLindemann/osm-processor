#include <cfloat>
#include <unordered_set>

#include <osmium/osm/area.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/relation.hpp>
#include <osmium/osm/location.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/tags/tags_filter.hpp>
#include <osmium/handler.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>
#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/index/index.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/io/file.hpp>

#include <boost/algorithm/string.hpp>

#include "extractor.hpp"
#include "serializer.hpp"

/**
 * 
 */
class RelationHandler : public osmium::handler::Handler
{
private:
    /**
     * 
     */
    static double perpendicular_distance(
    const osmium::Location& point,
    const osmium::Location& line_start,
    const osmium::Location& line_end
    ) {
        // Calculate the direction vector of the line
        double dir_x = line_end.lon() - line_start.lon();
        double dir_y = line_end.lat() - line_start.lat();

        // Normalize the direction vector
        double length = std::hypot(dir_x, dir_y);
        if (length > 0.0)
        {
            dir_x /= length;
            dir_y /= length;
        }

        // Calculate the point-to-line
        double ps_x = point.lon() - line_start.lon();
        double ps_y = point.lat() - line_start.lat();

        // Calculate the dot product between the two points to retrieve
        // the length between line_start and the plumb point L relative to p
        double dot = dir_x * ps_x + dir_y * ps_y;

        // Calculate the point of plumb on the line relative to p
        // by scaling the line direction vector
        double line_plumb_x = dot * dir_x;
        double line_plumb_y = dot * dir_y;

        // Calculate the vector from the plub point and p and return its distance
        double d_x = ps_x - line_plumb_x;
        double d_y = ps_y - line_plumb_y;

        return std::hypot(d_x, d_y);
    }

    // In-place compression of polygons with the Douglas-Peucker-Algorithm
    /**
     * Compresses a list of nodes with the Douglas-Peucker-Algorithm
     * https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
     * 
     * @param node_refs The list of node references.
     * @param epsilon The distance threshold as double
     */
    static const void compress(const osmium::NodeRefList& node_refs, double epsilon)
    {
        if (node_refs.size() < 2)
            // TODO: Throw error
            return;
        
        //  Start with the first node in the list
        double d_max = 0;
        size_t index = 0;
        size_t end = node_refs.size() - 1; 

        // Find the node with the greatest distance
        for (size_t i = 1; i < end; i++)
        {
            double d = perpendicular_distance(node_refs[i].location(), node_refs[0].location(), node_refs[end].location());
            if (d > d_max)
            {
                index = i;
                d_max = d;
            }
        }

        // Check if the maximum distance is greater than epsilon
        if (d_max > epsilon)
        {
            // TODO:
        }
        else
        {
            // TODO:
        }

    }

    /**
     * TODO:
     */
    osmium::memory::Buffer& m_buffer;

    /**
     * The boundary type that will be filtered for
     */ 
    std::string m_boundary_;

    /**
     * The epsilon used in the Douglas-Peucker compression
     */
    double m_epsilon_;

    /**
     * Storage for way indices that were processed already
     * This is needed to prevent multiple calculations for the
     * same ways
     */
    std::unordered_set<osmium::object_id_type> processed_ways{};

public:

    explicit RelationHandler(osmium::memory::Buffer& buffer, std::string boundary, double epsilon)
        : m_buffer(buffer), m_boundary_(boundary), m_epsilon_(epsilon) {}

    void relation(const osmium::Relation& relation)
    {   
        // Filter relations that have the specified boundary type
        // This has to be done again because the tag_filter only
        // at the area collection stage
        const char * boundary = relation.tags()["boundary"];
        if (boundary && boundary == m_boundary_)
        {
            // Retrieve way members
            for (const osmium::RelationMember& m : relation.members())
            {
                // Filter ways
                if (m.type() != osmium::item_type::way)
                    continue;
                // Check if way was already processed
                if (processed_ways.count(m.ref()))
                    continue;
                
                // Retrieve and compress way
                const osmium::Way& way = static_cast<const osmium::Way&>(m.get_object()); // TODO: muss den way im buffer verändern bevor der multipolygon collector die areas zusammenbaut
                // osmium::Way c_way = compress(way.nodes(), m_epsilon_);

                // Remember the way id to prevent multiple compressions
                processed_ways.insert(m.ref());
            };
        }

    }

};

/**
 * 
 */
class AreaHandler : public osmium::handler::Handler
{
private:
    /**
     * Calculate the centroid of a NodeRefList by calculating the
     * weigted sum of all nodes.
     * 
     * FIXME: This algorithm does not return "ideal" centerpoints
     * and should be reworked
     * 
     * @param node_refs The list of node references
     * @return The centroid as osmium::Location
     */
    static osmium::Location calc_center(const osmium::NodeRefList& node_refs) {
        osmium::geom::Coordinates center{ 0.0, 0.0 };
        for (const osmium::NodeRef& n : node_refs)
        {
            center.x += n.lon();
            center.y += n.lat();
        }
        center.x /= node_refs.size();
        center.y /= node_refs.size();
        return osmium::Location{ center.x, center.y };
    }

    // Calculate the bounding box of a NodeRefList.
    static osmium::Box calc_bounds(const osmium::NodeRefList& node_refs)
    {   
        double min_lon = DBL_MAX, min_lat = DBL_MAX ;
        double max_lon = -DBL_MAX, max_lat = -DBL_MAX;
        for (const osmium::NodeRef& n : node_refs)
        {
            min_lon = std::min(min_lon, n.lon());
            min_lat = std::min(min_lat, n.lat());
            max_lon = std::max(max_lon, n.lon());
            max_lat = std::max(max_lat, n.lat());
        }
        return osmium::Box{ min_lon, min_lat, max_lon, max_lat };
    }

    /**
     * The arrow table builder
     */ 
    Serializer::BoundaryArrowBuilder& m_builder_;

public:

    AreaHandler(Serializer::BoundaryArrowBuilder& builder) : m_builder_(builder) {};

    void area(const osmium::Area& area)
    {    
        // Retrieve tag values
        osmium::object_id_type id = area.id();                
        const char * name = area.tags()["name"];
        const char * type = area.tags()["boundary"];
        const char * admin_level = area.tags()["admin_level"];
        const char * source = area.tags()["source"];
        const char * wikidata = area.tags()["wikidata"];

        // Calculate the bounding box and center point of the first outer ring.
        // Because we set create_empty_areas = false in the assembler config,
        // we can be sure there will always be at least one outer ring.
        // Areas are unique, therefore no calculations will be performed
        // multiple times. There is no need to remember area indices.
        osmium::Box bounds = calc_bounds(*area.cbegin<osmium::OuterRing>());
        osmium::Location center = calc_center(*area.cbegin<osmium::OuterRing>());
        
        // Create convertable boundary
        Model::Boundary boundary = {
            id,
            name,
            type,
            admin_level ? std::stoi(admin_level) : 0,
            source,
            wikidata,
            center,
            bounds,
            area
        };

        // Add boundary to output
        m_builder_.append(boundary);

        // Print area
        // TODO: Only if verbose
        // Model::print_boundary(boundary);
    }

};

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

namespace Extractor 
{
    void run(Serializer::BoundaryArrowBuilder& builder, std::string input_path, std::string boundary_type, double epsilon)
    {   
        // The input file
        const osmium::io::File input_file{ input_path };

        // Configuration for the multipolygon assembler. We disable the option to
        // create empty areas when invalid multipolygons are encountered. This
        // means areas created have a valid geometry and invalid multipolygons
        // are simply ignored.
        osmium::area::Assembler::config_type assembler_config;
        assembler_config.create_empty_areas = false;

        // Set up a filter matching only boundaries with the specified boundary type
        osmium::TagsFilter filter{ false };
        filter.add_rule(true, "boundary", boundary_type);

        // Create the manager
        osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{ assembler_config, filter };

        // Pass through file for the first time and feed relations to the relation manager
        std::cout << "Starting first pass (reading relations)..." << std::endl;
        osmium::relations::read_relations(input_file, mp_manager);
        std::cout << "First pass done." << std::endl;

        std::cout << "Used Memory:" << std::endl;
        osmium::relations::print_used_memory(std::cout, mp_manager.used_memory());

        // The index storing all node locations
        index_type index;

        // The handler that stores all node locations in the index and adds them
        // to the ways
        location_handler_type location_handler{ index };
        location_handler.ignore_errors();

        // Pass through file for the second time and process the objects
        std::cout << "Starting second pass (reading nodes and ways and assembling areas)..." << std::endl;
        osmium::io::Reader reader{ input_file, osmium::io::read_meta::no };
        while (osmium::memory::Buffer input_buffer = reader.read()) {
            // Create an empty buffer with the same size as the input buffer.
            osmium::memory::Buffer output_buffer{ input_buffer.committed() };

            // Create the handler that processes boundary relations and handles way compression
            RelationHandler relation_handler { output_buffer, boundary_type, epsilon };

            // Apply the RelationHandler on the input buffer
            // The results are stored in the output buffer
            osmium::apply(input_buffer, location_handler, relation_handler);

            // Create the handler that processes assembled areas, calculates area
            // centers & bounds and serializes the results to an Arrow table
            AreaHandler area_handler { builder };

            // Apply the MultipolygonManager and AreaHandler to the output buffer
            osmium::apply(output_buffer, location_handler, mp_manager.handler([&area_handler](const osmium::memory::Buffer& buffer) {
                osmium::apply(buffer, area_handler);
            }));
        }
        std::cout << "Second pass done." << std::endl;
        reader.close();

        std::cout << "Used Memory:" << std::endl;
        osmium::relations::print_used_memory(std::cout, mp_manager.used_memory());
    }

}