#pragma once


namespace mapmaker
{

        /*

        class MapBuilder
        {
        public:

            using point_type        = geometry::Point<double>;
            using rectangle_type    = geometry::Rectangle<double>;
            using polygon_type      = geometry::Polygon<double>;
            using multipolygon_type = geometry::MultiPolygon<double>;

        protected:

            NodeBuffer m_node_buffer;
            AreaBuffer m_area_buffer;
            Graph m_graph;
            Config m_config;


            const geometry::Ring<double> convert_ring(const memory::Ring& ring) const
            {
                geometry::Ring<double> result;
                for (const auto& node_ref : ring)
                {
                    memory::Node node = m_node_buffer.get(node_ref);
                    result.push_back(node.point());
                }
                return result;
            }
            
        public:

            Builder(
                AreaBuffer& area_buffer,
                NodeBuffer& node_buffer,
                graph::UndirectedGraph& graph,
                Config& config
            ) : m_area_buffer(area_buffer),
                m_node_buffer(node_buffer),
                m_graph(graph),
                m_config(config)
            {

            };

            rectangle_type bounds() const
            {
                point_type min{ -DBL_MAX, -DBL_MAX };
                point_type max{  DBL_MAX,  DBL_MAX };
                rectangle_type result{ max, min };
                for (const auto& node : m_node_buffer)
                {
                    result.extend(node.point());
                }
                return result;
            }

            map::Map build()
            {



                // 2. Assemble boundaries

                // 1. Assemble Boundaries (territories & bonus levels)
                // 2. Create relationships from graph -> Two 
                // 3. 
                std::vector<map::Boundary> boundaries;

                // TODO before assembly: Filter components

                size_t id = 1;
                for (const auto& area : m_area_buffer)
                {
                    // Assemble geometry
                    multipolygon_type geometry;
                    for (const auto& outer_ring : area.outer_rings())
                    {
                        polygon_type polygon;
                        polygon.outer = build_ring(outer_ring);
                        for (const auto& inner_ring : area.inner_rings(outer_ring))
                        {
                            polygon.inners.push_back(build_ring(inner_ring));
                        }
                        geometry.polygons.push_back(polygon);
                    }
                    // Determine the area type and create a new boundary
                    map::Boundary::Type type;
                    if (area.level() == m_config.territory_level)
                    {
                        type = model::map::Boundary::TERRITORY;
                    }
                    else
                    {
                        type = model::map::Boundary::BONUS;
                    }
                    point_type center = algorithm::center(geometry);
                    boundaries.push_back(map::Boundary{
                        id,
                        type,
                        area.name(),
                        area.level(),
                        geometry,
                        center
                    });
                    ++id;
                }

                return map::Map{ width, height, boundaries };
            }
            
        };

        */

}