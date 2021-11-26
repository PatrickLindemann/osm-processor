#pragma once

#include "routine.hpp"

#include "model/graph/undirected_graph.hpp"
#include "model/boundary.hpp"
#include "model/types.hpp"

#include "io/reader/header_reader.hpp"
#include "io/reader/osm_reader.hpp"
#include "io/writer/map_writer.hpp"
#include "io/writer/mapdata_writer.hpp"

#include "mapmaker/assembler.hpp"
#include "mapmaker/builder.hpp"
#include "mapmaker/calculator.hpp"
#include "mapmaker/compressor.hpp"
#include "mapmaker/converter.hpp"
#include "mapmaker/counter.hpp"
#include "mapmaker/filter.hpp"
#include "mapmaker/inspector.hpp"

#include "functions/transform.hpp"

#include "util/log.hpp"
#include "util/title.hpp"
#include "util/validate.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

using namespace model;

/**
 * The upload routine uploads map metadata contained in a JSON file to Warzone
 * through the Warzone API.
 *
 * More informtion on the API can be found under the following link:
 * https://www.warzone.com/wiki/Set_map_details_API
 */
class Create : public Routine
{

    /* Types */

    using T = double;

    using buffer_t = osmium::memory::Buffer;

    using graph_t = graph::UndirectedGraph;

    using component_t = std::vector<std::set<osmium::object_id_type>>;

    using container_t = std::map<object_id_type, Boundary<T>>;

    using hierarchy_t = std::map<object_id_type, std::set<object_id_type>>;

    /* Members */

    /**
     * The path to the input OSM file.
     */
    fs::path m_input;

    /**
     * The output directory for the warzone map geometry and mapdata.
     */
    fs::path m_outdir;

   /**
    * The admin_level for territories.
    */
    level_type m_territory_level;

    /**
    * The admin_levels for bonuses.
    */
    std::vector<level_type> m_bonus_levels;

   /**
    * The width of the generated map in pixels.
    */
    int m_width;

    /**
     * The height of the generated map in pixels.
     */
    int m_height;

    /**
     * The compression distance tolerance for the Douglas-Peucker algorithm.
     */
    double m_compression_tolerance;

    /**
     * The surface area tolerance for the filter algorithm.
     */
    double m_filter_tolerance;

   /**
    * The verbose logging flag.
    */
    bool m_verbose;

    /**
     * The logger.
     */
    util::Logger<std::ostream> m_log{ std::cout };

public:

    /* Constructors */

    Create() : Routine()
    {
        m_options.add_options()
            ("input", po::value<fs::path>()->required(), "Sets the input file path.\nAllowed file formats: .osm, .pbf")
            ("outdir,o", po::value<fs::path>()->default_value(""), "Sets the output folder for the generated map files.")
            ("territory-level,t", po::value<level_type>()->default_value(0), "Sets the admin_level of boundaries that will be be used as territories.\nInteger between 1 and 12.")
            ("bonus-levels,b", po::value<std::vector<level_type>>()->multitoken(), "Sets the admin_level of boundaries that will be be used as bonus links.\nInteger between 1 and 12. If none are specified, no bonus links will be generated.")
            ("width", po::value<int>()->default_value(1000), "Sets the generated map width in pixels.\nIf set to 0, the width will be determined automatically with the height.")
            ("height", po::value<int>()->default_value(0), "Sets the generated map height in pixels.\nIf set to 0, the height will be determined automatically with the width.")
            ("compression-tolerance,c", po::value<double>()->default_value(0.0), "Sets the minimum distance tolerance for the compression algorithm.\nIf set to 0, no compression will be applied.")
            ("filter-tolerance,f", po::value<double>()->default_value(0.0), "Sets the surface area ratio tolerance for filtering boundaries.\nIf set to 0, no filter will be applied.")
            ("verbose", po::bool_switch()->default_value(false), "Enables verbose logging.")
            ("help,h", "Shows this help message.");
        m_positional.add("input", 1);
    }

    /* Override Methods */

    const std::string name() const noexcept override
    {
        return "upload";
    }

    void setup() override
    {
        Routine::setup();
        this->set<fs::path>(&m_input, "input", util::validate_file);
        this->set<fs::path>(&m_outdir, "outdir", m_dir, util::validate_dir);
        this->set<level_type>(&m_territory_level, "territory-level");
        this->set<std::vector<level_type>>(&m_bonus_levels, "bonus-levels", std::vector<level_type>{});
        std::sort(m_bonus_levels.begin(), m_bonus_levels.end());
        util::validate_levels(m_territory_level, m_bonus_levels);
        this->set<int>(&m_width, "width");
        this->set<int>(&m_height, "height");
        util::validate_dimensions(m_width, m_height);
        this->set<double>(&m_compression_tolerance, "compression-tolerance", util::validate_epsilon);
        this->set<double>(&m_filter_tolerance, "filter-tolerance", util::validate_epsilon);
        this->set<bool>(&m_verbose, "verbose");
        // fs::create_directory(m_dir / "out");#
        // Calculate the total number of steps for the routine
        std::size_t steps = 10 + (m_compression_tolerance > 0.0)
                    + (m_filter_tolerance > 0.0)
                    + (!m_bonus_levels.empty());
        m_log.set_steps(steps);
    }

private:

    /* Helper methods */

    Header read_header(const fs::path& file_path)
    {
        // Prepare the header reader for the input file and retrieve the header
        io::HeaderReader reader{ file_path.string() };
        return reader.read();
    }

    osmium::memory::Buffer read_data(const fs::path& file_path, std::set<level_type> levels)
    {
        // Retrieve the administrative boundaries with and admin_level that
        // matches the prepared level filter from the input file
        io::BoundaryReader reader{ m_input, levels };
        return reader.read();
    }

    void compress(buffer_t& buffer)
    {
        // Count the nodes before the compression
        mapmaker::NodeCounter counter;
        std::size_t before = counter.run(buffer);

        // Compress the extracted ways using the specified compression
        // tolerance
        mapmaker::Compressor compressor{ m_compression_tolerance };
        compressor.run(buffer);

        // Count the nodes after the compression
        std::size_t after = counter.run(buffer);

        m_log.step() << "Compressed " << before << " nodes to " << after << " nodes.\n";
    }

    void assemble(buffer_t& buffer, std::set<level_type> levels, bool split)
    {
        // Create the assembler depending on the split strategy.
        mapmaker::Assembler assembler{ levels, split };
        assembler.run(buffer);
    }

    graph_t get_neighbors(const buffer_t& buffer, level_type level)
    {
        mapmaker::NeighborInspector inspector{ level };
        return inspector.run(buffer);
    }

    component_t get_components(const graph_t& neighbors)
    {
        mapmaker::ComponentInspector inspector;
        return inspector.run(neighbors);
    }
    
    void filter(buffer_t& buffer, graph_t& neighbors, component_t& components){
        // Count the areas before the filter process
        mapmaker::AreaCounter counter;
        std::size_t before = counter.run(buffer);

        // Apply the area filter on the area buffer using the specified tolerance
        mapmaker::AreaFilter filter{ m_filter_tolerance };
        filter.run(buffer, neighbors, components);

        // Count the nodes after the filter process
        std::size_t after = counter.run(buffer);

        m_log.step() << "Compressed " << before << " nodes to " << after << " nodes.\n";
    }
    
    template <typename T>
    void transform(functions::Transformation<T>& transformation, geometry::Rectangle<T>& bounds)
    {
        transformation.transform(bounds.min().x(), bounds.min().y());
        transformation.transform(bounds.max().x(), bounds.max().y());
    }

    container_t convert(buffer_t& buffer)
    {     
        // Prepare the transformations that will be applied on the buffer before
        // the geometry conversion. At first, calculate the bounding box of the
        // nodes in the buffer.
        mapmaker::BoundsCalculator<T> bounds_calculator{};
        geometry::Rectangle<T> bounds = bounds_calculator.run(buffer);

        // The radian transformation converts the nodes, for which the locations
        // are specified in degrees, to radians, for futher usage in the Mercator
        // projection.
        functions::RadianTransformation<T> radian_transformation{};

        // The Mercator projection maps the spherical earth coordinates to two-
        // dimensional planar coordinates.
        functions::MercatorProjection<T> mercator_transformation{};

        // The normalization transformations normalizes and fits the locations within
        // the unit interval.
        transform(radian_transformation, bounds);
        transform(mercator_transformation, bounds);
        functions::UnitTransformation<T> normalize_transformation{
            { bounds.min().x(), bounds.max().x() },
            { bounds.min().y(), bounds.max().y() }
        };

        // The mirror transformation mirrors the map coordinates on the horizontal
        // axis, so that they are displayed correctly in the svg coordinate system.
        functions::MirrorTransformation<T> mirror_transformation{ false, true };

        // Check if a dimension is set to auto and calculate its value
        // depending on the transformed map bounds
        if (m_width == 0 || m_height == 0)
        {
            if (m_width == 0)
            {
                m_width = bounds.width() / bounds.height() * m_height;
            }
            else
            {
                m_height = bounds.height() / bounds.width() * m_width;
            }
        }

        // The scaling transformation maps the normalized 
        functions::ScaleTransformation<T> scale_transformation{ (double) m_width, (double) m_height };

        // Create the converter, which will apply the specified transformations
        // and convert the areas to multipolygon geometries afterwards.
        mapmaker::BoundaryConverter<T> converter{
            std::make_shared<functions::RadianTransformation<T>>(radian_transformation),
            std::make_shared<functions::MercatorProjection<T>>(mercator_transformation),
            std::make_shared<functions::UnitTransformation<T>>(normalize_transformation),
            // std::make_shared<functions::MirrorTransformation<T>>(mirror_transformation),
            std::make_shared<functions::ScaleTransformation<T>>(scale_transformation)
        };
        return converter.run(buffer);
    }

    void calculate_centers(container_t& boundaries)
    {
        mapmaker::CenterCalculator<T> calculator;
        calculator.run(boundaries);
    }

    hierarchy_t calculate_hierarchy(const container_t& boundaries)
    {
        mapmaker::HierarchyInspector<T> inspector;
        return inspector.run(boundaries);
    }
    
    warzone::Map<T> build_map(std::string name, container_t& boundaries, const graph_t& neighbors, const hierarchy_t& hierarchy)
    {
        mapmaker::MapBuilder<T> builder{};
        builder.name(name);
        builder.width(m_width);
        builder.height(m_height);
        builder.territory_level(m_territory_level);
        if (!m_bonus_levels.empty())
        {
            builder.bonus_level(m_bonus_levels.at(0));
            if (m_bonus_levels.size() > 1)
            {
                builder.super_bonus_level(m_bonus_levels.at(1));
            }
        }
        builder.neighbors(neighbors);
        builder.hierarchy(hierarchy);
        return builder.run(boundaries);
    }

    void export_map(warzone::Map<T>&& map)
    {
        fs::path file_path = m_outdir / fs::path(map.name).replace_extension(".svg");
        io::MapWriter<T> writer{ file_path };
        m_log.step() << "Exporting map to " << file_path << ".\n";
        writer.write(std::move(map));
        m_log.step() << "Map export finished.\n";
    }

    void export_mapdata(warzone::Map<T>&& map)
    {
        fs::path file_path = m_outdir / fs::path(map.name).replace_extension(".json");
        io::MapdataWriter<T> writer{ file_path };
        m_log.step() << "Exporting map data to " << file_path << ".\n";
        writer.write(std::move(map));
        m_log.step() << "Map data export finished\n.";
    }

public:

    void run() override
    {        
        // Print the title
        std::cout << util::title() << std::endl;

        // Step 1: Read the file header and determine the territory level
        // automatically if it was not set.
        m_log.start() << "Retrieving headers from file " << m_input << ".\n";
        Header header = read_header(m_input);
        if (m_territory_level == 0)
        {
            auto [l, c] = *std::max_element(header.levels.cbegin(), header.levels.cend(),
                [](const std::pair<short, std::size_t>& e1, const std::pair<short, std::size_t>& e2)
                {
                    return e1.second < e2.second;
                }
            );
            m_territory_level = l;
        } 
        // Prepare the level filter with the specified territory and bonus
        // levels
        std::set<level_type> levels{ m_bonus_levels.begin(), m_bonus_levels.end() };
        levels.insert(m_territory_level);
        m_log.finish();

        // Step 2: Prepare the level filter and read the boundaries from
        // the specified input file.
        m_log.start() << "Reading boundaries from file " << m_input << ".\n";
        buffer_t buffer = read_data(m_input, levels);
        m_log.finish();

        // Step 3: Compress the extracted ways using the Douglas-Peucker
        // algorithm if a compression threshold was specified.
        if (m_compression_tolerance > 0)
        {
            m_log.start() << "Compressing ways with tolerance " << m_compression_tolerance << ".\n";
            compress(buffer);
            m_log.finish();
        }

        // Step 4: Assemble the territory boundaries using the built-in
        // multipolygon assembler.
        m_log.start() << "Assembling territories with level " << m_territory_level << ".\n";
        assemble(buffer, { m_territory_level }, true);
        m_log.finish();
        
        // Step 5: Create the neighbor graph for the assembled territories.
        m_log.start() << "Calculating neighborships for territories.\n";
        graph::UndirectedGraph neighbors = get_neighbors(buffer, m_territory_level);
        m_log.finish();

        // Step 6: Calculate the connected components for the neighbor graph.
        // This yields the islands of the map.
        m_log.start() << "Finding territory islands.\n";
        component_t components = get_components(neighbors);
        m_log.finish();

        // Step 7: Filter connected components by their surface area if a filter
        // threshold was specified.
        if (m_filter_tolerance > 0)
        {
            m_log.start() << "Compressing ways with tolerance " << m_filter_tolerance << ".\n";
            filter(buffer, neighbors, components);
            m_log.finish();
        }

        // Step 8: Assemble the bonus boundarties using the built-in multipolygon
        // assembler if any bonus levels were specified.
        if (!m_bonus_levels.empty())
        {
            m_log.start() << "Assembling bonuses with the levels " << util::join(m_bonus_levels) << ".\n";
            assemble(buffer, std::set<level_type>(m_bonus_levels.begin(), m_bonus_levels.end()), false);
            m_log.finish();
        }
        
        // Step 9: Create the boundary geometries from the assembled boundaries by
        // applying the map projections and transformations first and converting
        // the osmium objects to geometry objects afterwards.
        m_log.start() << "Building the boundary geometries from the OpenStreetMap objects.\n";
        std::map<object_id_type, Boundary<T>> boundaries = convert(buffer);
        m_log.finish();
        
        // Step 10: Calculate the center points for each boundary
        m_log.start() << "Calculating the center points for " << boundaries.size() << " boundaries.\n";
        calculate_centers(boundaries);
        m_log.finish();

        // Step 11: Calculate the hirarchy of territories, bonuses and super bonuses
        // if any bonus levels were specified
        hierarchy_t hierarchy = {};
        if (!m_bonus_levels.empty())
        {
            m_log.start() << "Calculating the hierarchy for " << boundaries.size() << " boundaries.\n";
            hierarchy = calculate_hierarchy(boundaries);
            m_log.finish();
        }

        // Step 12: Build the map with the generated data
        m_log.start() << "Building the Warzone map.\n";
        // Create the map name from the input file name
        std::string name = std::regex_replace(
            m_input.filename().string(),
            std::regex("(\\.osm|\\.pbf)"),
            ""
        );
        // Build the map
        warzone::Map map = build_map(name, boundaries, neighbors, hierarchy);
        m_log.finish();

        // Step 13: Export the generated Warzone map and the calculated mapdata
        // to the specified output directory
        m_log.start() << "Exporting the generated map files.\n";
        export_map(std::move(map));
        export_mapdata(std::move(map));
        m_log.finish();

        // Routine finished, print the total duration.
        m_log.end();
    }

};