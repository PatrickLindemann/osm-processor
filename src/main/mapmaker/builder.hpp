#pragma once

#include <cfloat>
#include <vector>

#include "model/geometry/multipolygon.hpp"
#include "model/geometry/polygon.hpp"
#include "model/map/boundary.hpp"
#include "model/map/map.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/memory/buffer.hpp"
#include "model/graph/undirected_graph.hpp"
#include "mapmaker/projector.hpp"
#include "mapmaker/algorithm.hpp"
#include "model/memory/ring.hpp"
#include "model/memory/node.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"

using namespace model;

namespace mapmaker
{

    namespace builder
    {

        /* Definitions */

        using NodeBuffer  = memory::Buffer<memory::Node>;
        using AreaBuffer  = memory::Buffer<memory::Area>;
        using Graph       = graph::UndirectedGraph;

        /**
         * 
         */
        struct Config
        {
            int width;
            int height;
            int territory_level;
            std::vector<int> bonus_levels;
        };

        /**
         * 
         */
        class Builder
        {
        public:

            /* Types */

            using point_type        = geometry::Point<double>;
            using rectangle_type    = geometry::Rectangle<double>;
            using polygon_type      = geometry::Polygon<double>;
            using multipolygon_type = geometry::MultiPolygon<double>;

        protected:
            
            /* Members */

            NodeBuffer m_node_buffer;
            AreaBuffer m_area_buffer;
            Graph m_graph;
            Config m_config;

            /* Methods */

            /**
             * 
             */
            void apply(const projector::Projector<double>& projector)
            {
                for (auto& node : m_node_buffer)
                {
                    node.point() = projector.project(node.lon(), node.lat());
                }
            }

            const geometry::Ring<double> build_ring(const memory::Ring& ring) const
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

            /* Constructor */

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

            /**
             * 
             */
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

            /**
             * 
             */
            map::Map build()
            {
                // Convert the map coordinates to radians
                apply(mapmaker::projector::RadianProjector<double>{});

                // Apply the MercatorProjection
                apply(mapmaker::projector::MercatorProjector<double>{});

                // Calculate the map bounds
                rectangle_type b = bounds();

                // Check if a dimension is set to auto and calculate its value
                // depending on the map bounds
                int width = m_config.width;
                int height = m_config.height;
                if (width == 0 || height == 0)
                {
                    if (width == 0)
                    {
                        width = b.width() / b.height() * height;
                    }
                    else
                    {
                        height = b.height() / b.width() * width;
                    }
                }

                // Scale the map according to the dimensions
                apply(mapmaker::projector::UnitProjector<double>{
                    { b.min.x, b.max.x }, {b.min.y, b.max.y }
                });
                apply(mapmaker::projector::IntervalProjector<double>{
                    { 0.0, 1.0 }, { 0.0, 1.0 }, { 0.0, width }, { 0.0, height }
                });

                // 1. Calculate neighbors
                // map<node_id, vector<territory_id>> connected_nodes;
                // For each node in node_buffer:
                // if (degree(node) >= 3)
                // connected_nodes[node.id()] = {}
                // Alternativ über alle areas? Muss man ja später sowieso

                // 2. Assemble boundaries

                // 1. Assemble Boundaries (territories & bonus levels)
                // 2. Create relationships from graph -> Two 
                // 3. 
                std::vector<map::Boundary> boundaries;

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
                    point_type center = algorithm::center(geometry.polygons.at(0));
                    boundaries.push_back(map::Boundary{
                        area.id(),
                        type,
                        area.name(),
                        area.level(),
                        geometry,
                        center
                    });

                }

                return map::Map{ width, height, boundaries };
            }

            /* Misc */

            
        };

    }

}