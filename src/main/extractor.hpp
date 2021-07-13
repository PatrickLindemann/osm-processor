#ifndef MAPMAKER_EXTRACTOR_HPP
#define MAPMAKER_EXTRACTOR_HPP

#include <set>
#include <string>
#include <algorithm>

#include <osmium/visitor.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/index/map/flex_mem.hpp>
#include <osmium/area/assembler.hpp>
#include <osmium/area/multipolygon_manager.hpp>
#include <osmium/geom/factory.hpp>
#include <osmium/geom/coordinates.hpp>
#include <osmium/handler/node_locations_for_ways.hpp>

#include "model/area.hpp"
#include "model/line.hpp"
#include "model/polygon.hpp"
#include "model/multipolygon.hpp"

#include "functions.hpp"

namespace mapmaker
{

// The type of index used. This must match the include file above
using index_type = osmium::index::map::FlexMem<osmium::unsigned_object_id_type, osmium::Location>;

// The location handler always depends on the index type
using location_handler_type = osmium::handler::NodeLocationsForWays<index_type>;

class AreaFactoryImpl {

public:

    using point_type = Point;
    using linestring_type = Line;
    using polygon_type = Polygon;
    using multipolygon_type = MultiPolygon;
    using ring_type = Ring;

private:

    std::vector<point_type> m_point_sequence;

    ring_type m_outer_ring;

    std::vector<ring_type> m_inner_rings;

    std::vector<polygon_type> m_polygons;

public:

    AreaFactoryImpl(int srid) {};

    /* Point */

    point_type make_point(const osmium::geom::Coordinates& coords) const
    {
        return point_type{ coords.x, coords.y };
    }

    /* Line */

    void linestring_start()
    {
        m_point_sequence.clear();
    }

    void linestring_add_location(const osmium::geom::Coordinates& coords)
    {
       m_point_sequence.push_back(point_type{ coords.x, coords.y });
    }

    linestring_type linestring_finish(size_t /* num_points */)
    {
        assert(m_point_sequence.size() >= 2);
        return linestring_type{ m_point_sequence };
    }

    /* Polygon */

    void polygon_start() {
        m_point_sequence.clear();
    }

    void polygon_add_location(const osmium::geom::Coordinates& coords) {
        m_point_sequence.push_back(point_type{ coords.x, coords.y });
    }

    polygon_type polygon_finish(size_t /* num_points */) {
        return polygon_type{ ring_type{ m_point_sequence } };
    }

    /* MultiPolygon */

    void multipolygon_start() {
        m_polygons.clear();
    }

    void multipolygon_polygon_start() {
        m_inner_rings.clear();
    }

    void multipolygon_polygon_finish() {
        m_polygons.push_back(Polygon{
            m_outer_ring,
            m_inner_rings
        });
    }

    void multipolygon_outer_ring_start() {
        m_point_sequence.clear();
    }

    void multipolygon_outer_ring_finish() {
        m_outer_ring = m_point_sequence;
    }

    void multipolygon_inner_ring_start() {
        m_point_sequence.clear();
    }

    void multipolygon_inner_ring_finish() {
        m_inner_rings.push_back(m_point_sequence);
    }

    void multipolygon_add_location(const osmium::geom::Coordinates& coords) {
        m_point_sequence.push_back(point_type{ coords.x, coords.y });
    }

    multipolygon_type multipolygon_finish() {
        return multipolygon_type{ m_polygons };
    }

};

template <typename TProjection = osmium::geom::IdentityProjection>
using AreaFactory = osmium::geom::GeometryFactory<AreaFactoryImpl, TProjection>;

/**
 * 
 */
class AreaHandler : public osmium::handler::Handler
{

    AreaFactory<> m_factory;

    std::vector<Area> m_container;

public:

    void area(const osmium::Area& area)
    {    
        // Retrieve tag values
        osmium::object_id_type id = area.id();                
        const char * name = area.tags()["name"];
        const char * type = area.tags()["boundary"];
        const char * admin_level = area.tags()["admin_level"];
        const char * source = area.tags()["source"];
        const char * wikidata = area.tags()["wikidata"];

        // Create MultiPolygon
        MultiPolygon multi_polygon = m_factory.create_multipolygon(area);

        // Create new area and add it to the container
        m_container.push_back({
            id,
            name,
            type,
            admin_level ? std::stoi(admin_level) : 0,
            multi_polygon,
            source,
            wikidata
        });
    }

    const std::vector<Area>& container() const
    {
        return m_container;
    }

};

namespace extractor 
{
    
inline std::vector<Area> run(std::string input_path, bool cache)
{   
    // todo check if cache and if file was already cached. If yes, parse it and return results

    const osmium::io::File input_file{ input_path };

    // Configuration for the multipolygon assembler. Here the default settings
    // are used, but you could change multiple settings.
    osmium::area::Assembler::config_type assembler_config;
    
    // Set up a filter matching only forests. This will be used to only build
    // areas with matching tags.    
    osmium::TagsFilter filter{ false };
    filter.add_rule(true, "boundary", "administrative");

    // Initialize the MultipolygonManager. Its job is to collect all
    // relations and member ways needed for each area. It then calls an
    // instance of the osmium::area::Assembler class (with the given config)
    // to actually assemble one area. The filter parameter is optional, if
    // it is not set, all areas will be built.
    osmium::area::MultipolygonManager<osmium::area::Assembler> mp_manager{ assembler_config, filter };

    // Pass through file for the first time and feed relations to the
    // multipolygon manager
    std::cout << "Starting first pass (reading relations)..." << std::endl;
    osmium::relations::read_relations(input_file, mp_manager);
    std::cout << "First pass done." << std::endl;

    std::cout << "Used Memory:" << std::endl;
    osmium::relations::print_used_memory(std::cout, mp_manager.used_memory());

    // The index storing all node locations
    index_type index;
    
    // The handler that stores all node locations in the index and adds
    // them to the ways
    location_handler_type location_handler{ index };
    location_handler.ignore_errors();

    // The area handler
    AreaHandler handler{};

    // Pass through file for the second time and process the objects
    std::cout << "Starting second pass (reading and compressing nodes and ways)..." << std::endl;
    osmium::io::Reader reader{ input_file, osmium::io::read_meta::no };
    osmium::apply(reader, location_handler, mp_manager.handler([&handler, &cache](osmium::memory::Buffer&& buffer) {
        osmium::apply(buffer, handler);
        if (cache)
        {
            osmium::io::Writer writer{ "../out/write.osm", osmium::io::overwrite::allow };
            writer(std::move(buffer));
            writer.close();
        }
    }));
    reader.close();
    std::cout << "Second pass done." << std::endl;

    std::cout << "Used Memory:" << std::endl;
    osmium::relations::print_used_memory(std::cout, mp_manager.used_memory());

    // If there were multipolgyon relations in the input, but some of their
    // members are not in the input file (which often happens for extracts)
    // this will write the IDs of the incomplete relations to stderr.
    std::vector<osmium::object_id_type> incomplete_relations_ids;
    mp_manager.for_each_incomplete_relation([&](const osmium::relations::RelationHandle& handle){
        incomplete_relations_ids.push_back(handle->id());
    });
    if (!incomplete_relations_ids.empty()) {
        std::cerr << "Warning! Some member ways missing for these multipolygon relations:";
        for (const auto id : incomplete_relations_ids) {
            std::cerr << " " << id;
        }
        std::cerr << "\n";
    }

    return handler.container();
}

}

}

#endif