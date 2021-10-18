#pragma once

#include <algorithm>
#include <stack>
#include <string>

#include <boost/lexical_cast.hpp>

#include <osmium/osm/types.hpp>

#include "functions/intersect.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/entity.hpp"
#include "model/memory/entity_ref_list.hpp"
#include "model/memory/node.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/ring.hpp"
#include "model/memory/way.hpp"

namespace mapmaker
{
    
    namespace assembler
    {

        using namespace model;

        namespace detail
        {
            
            /**
             *
             */
            struct ProtoRing
            {
                /**
                 * 
                 */
                memory::EntityRefList<memory::NodeRef> nodes;

                /**
                 * 
                 */
                memory::EntityRefList<memory::WayRef> ways;

                /**
                 * 
                 */
                geometry::Ring<double> geometry;

                /**
                 * 
                 */
                geometry::Rectangle<double> envelope;

                /**
                 * 
                 * @param way
                 */
                void add_way_nodes(const memory::Way& way)
                {
                    // Add way reference to ring
                    ways.push_back(way.id());
                    // Check if this is the first way that gets inserted
                    if (nodes.empty())
                    {
                        // Insert all node references
                        nodes.insert(nodes.end(), way.cbegin(), way.cend());
                    }
                    // Check if the way needs to be reversed
                    else if (nodes.back() == way.front())
                    {
                        // Insert all node references except the first
                        nodes.insert(nodes.end(), way.cbegin() + 1, way.cend());
                    }
                    else
                    {
                        // Insert all node references except the first in reverse order
                        nodes.insert(nodes.end(), way.crbegin() + 1, way.crend());
                    }
                }

                /**
                 * 
                 * @param way
                 */
                void remove_way_nodes(const memory::Way& way)
                {
                    assert(ways.back() == way.id());
                    // Remove node references of the way
                    for (size_t i = 0; i < way.size(); i++)
                    {
                        nodes.pop_back();
                    }
                    // Remove way reference
                    ways.pop_back();
                }

                /**
                 * 
                 */
                void calculate_geometry(const memory::Buffer<memory::Node>& node_buffer)
                {
                    double min_x = DBL_MAX;
                    double min_y = DBL_MAX;
                    double max_x = -DBL_MAX;
                    double max_y = -DBL_MAX;
                    geometry.clear();
                    for (const memory::NodeRef& nr : nodes)
                    {
                        // Retrieve node from buffer
                        const memory::Node& node = node_buffer.at(nr);
                        const geometry::Point<double>& point = node.point();
                        // Add the node point to the buffer
                        geometry.push_back(point);
                        // Extend the bounding box if neccessary
                        min_x = std::min(min_x, point.x());
                        min_y = std::min(min_y, point.y());
                        max_x = std::max(max_x, point.x());
                        max_y = std::max(max_y, point.y());
                    }
                    envelope = { min_x, min_y, max_x, max_y };
                }

            };

            /**
             * 
             */
            struct ProtoGroup
            {
                ProtoRing outer;
                std::vector<ProtoRing> inners;
            };

        }

        using namespace detail;

        /**
         * 
         */
        class AreaAssembler
        {
        protected:

            /* Members */

            const memory::Buffer<memory::Node>&      m_node_buffer;
            const memory::Buffer<memory::Way>&       m_way_buffer;
            const memory::Buffer<memory::Relation>&  m_relation_buffer;

        public:

            /* Constructors */

            AreaAssembler(
                const memory::Buffer<memory::Node>& nodes,
                const memory::Buffer<memory::Way>& ways,
                const memory::Buffer<memory::Relation>& relations
            ) : m_node_buffer(nodes), m_way_buffer(ways), m_relation_buffer(relations) {}

        protected:
           
            /* Helper methods */

            /**
             * 
             */
            bool is_closed(const ProtoRing& ring) const
            {
                return ring.nodes.front() == ring.nodes.back();
            }

            /**
             * 
             */
            bool is_valid(const ProtoRing& ring) const
            {
                // Check if ring has at least three nodes
                if (ring.nodes.size() < 3)
                {
                    return false;
                }
                // Convert ring to geometry and check if it
                // self-intersects. If not, the ring is valid.
                return !functions::ring_self_intersects(ring.geometry);
            }
            
            /**
             * 
             */
            bool complete_ring(
                ProtoRing& ring,
                memory::EntityRefList<memory::WayRef>& ways,
                std::unordered_map<object_id_type, bool>& processed
            ) {
                if (is_closed(ring))
                {
                    // Pre-calculate the geometry for further checks
                    ring.calculate_geometry(m_node_buffer);
                    return is_valid(ring);
                }
                // Find candidates
                std::stack<memory::WayRef> candidates{};
                for (const memory::WayRef& way : ways)
                {
                    if (!processed.at(way.ref()))
                    {
                        const memory::Way& next = m_way_buffer.at(way.ref());
                        if (ring.nodes.back() == next.front()
                         || ring.nodes.back() == next.back())
                        {
                            candidates.push({ next.id() });
                        }
                    }
                }
                // Try to continue ring with the identified candidates
                while (!candidates.empty())
                {
                    // Get next candidate
                    memory::WayRef& candidate = candidates.top();
                    candidates.pop();
                    const memory::Way& candidate_way = m_way_buffer.at(candidate);
                    // Add current candidate to the ring
                    ring.add_way_nodes(candidate_way);
                    processed.at(candidate.ref()) = true;
                    // Try to create the ring with the current candidate
                    bool finished = complete_ring(ring, ways, processed);
                    // Check if ring was finished
                    if (finished)
                    {
                        return true;
                    }
                    // Backtrack: Remove candidate from ring and try another
                    ring.remove_way_nodes(candidate_way);
                    processed.at(candidate.ref()) = false;
                }
                return false;
            }

            /**
             * 
             * https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
             */
            std::vector<ProtoRing> create_rings(
                memory::EntityRefList<memory::WayRef>& ways
            ) {
                // Prepare the result vector of rings
                std::vector<ProtoRing> rings;
                // Prepare the lookup table for processed way indices
                std::unordered_map<object_id_type, bool> processed = {};
                for (const memory::WayRef& way : ways)
                {
                    processed[way.ref()] = false;
                }
                // Find the next non-processed way
                for (const memory::WayRef& way : ways)
                {
                    if (!processed.at(way.ref()))
                    {
                        // Start a new ring and create it with backtracking
                        ProtoRing ring;
                        ring.add_way_nodes(m_way_buffer.at(way));
                        processed.at(way.ref()) = true;
                        // Create the ring
                        complete_ring(ring, ways, processed);
                        // Push the created ring to the result collection if
                        // it is valid
                        if (ring.nodes.size() > 2)
                        {
                            rings.push_back(ring);
                        }
                    }
                }
                return std::move(rings);
            }

            /**
             * 
             * https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
             */
            std::vector<ProtoGroup> group_rings(
                const std::vector<ProtoRing>& outer_rings,
                const std::vector<ProtoRing>& inner_rings
            ) {
                std::vector<ProtoGroup> groups;

                if (inner_rings.size() == 0)
                {
                    // Simple case: N outers, 0 inners
                    // No grouping needs to be performed
                    for (const ProtoRing& outer : outer_rings)
                    {
                        groups.push_back({ outer });
                    }
                }
                else
                {
                    // Complex case: N outers, N inners
                    // Perform ring grouping algorithm

                    // ProtoGroup the inner rings to the outer ring by ring-in-ring
                    // checks
                    for (const ProtoRing& inner : inner_rings)
                    {
                        // Try to find the outer ring that the inner ring is
                        // fully contained in
                        for (size_t i = 0; i < outer_rings.size(); i++)
                        {
                            // Retrieve the outer ring
                            const ProtoRing& outer = outer_rings.at(i);
                            // Compare the bounding boxes first
                            if (functions::rectangle_in_rectangle(
                                inner.envelope,
                                outer.envelope
                            )) {
                                // Check if the inner ring is actually contained
                                // in the outer ring
                                if (functions::ring_in_ring(inner.geometry, outer.geometry))
                                {
                                    groups.at(i).inners.push_back(inner);
                                    continue;
                                }
                            }
                            // Inner ring was not assigned to an outer ring.
                            // This happens if
                            //  1) the inner ring is invalid (intersects with any outer ring) or
                            //  2) the inner ring is actually an outer ring.
                            // For either case, we ignore this ring.
                        }
                    }
                }

                return groups;
            }

            /* Virtual methods */

            virtual void create_areas(
                memory::Buffer<memory::Area>& buffer,
                std::string name,
                level_type level,
                const std::vector<ProtoGroup>& groups
            ) const = 0;

        public:

            /**
             * 
             */
            void assemble_areas(memory::Buffer<memory::Area>& buffer, const std::vector<level_type>& levels)
            {
                // Initialize the level filter
                std::array<bool, 13> filter;
                for (const level_type& level : levels)
                {
                    filter.at(level) = true;
                }

                // Convert areas from the relations
                for (const memory::Relation& relation : m_relation_buffer)
                {
                    // Filter relations by their level
                    level_type level = boost::lexical_cast<level_type>(
                        relation.get_tag("admin_level", "0")
                    );
                    if (!filter.at(level))
                    {
                        continue;
                    }

                    // Retrieve other relevant relation tags
                    std::string name = relation.get_tag("name");

                    // Retrieve inner and outer ways
                    memory::EntityRefList<memory::WayRef> outer_ways;
                    memory::EntityRefList<memory::WayRef> inner_ways;
                    for (const memory::Member& member : relation.members())
                    {
                        if (member.role() == "outer")
                        {
                            outer_ways.push_back(member.ref());
                        }
                        else if (member.role() == "inner")
                        {
                            inner_ways.push_back(member.ref());
                        }
                    }

                    // Assemble the outer rings first
                    const auto outer_rings = create_rings(outer_ways);
                    if (outer_rings.empty())
                    {
                        continue;
                    }
                    const auto inner_rings = create_rings(inner_ways);

                    // ProtoGroup the rings
                    const auto groups = group_rings(outer_rings, inner_rings);

                    // Create the area
                    this->create_areas(buffer, name, level, groups);
                }

            }

            /**
             * 
             */
            memory::Buffer<memory::Area> assemble_areas(const std::vector<level_type>& levels)
            {
                memory::Buffer<memory::Area> areas;
                assemble_areas(areas, levels);
                return areas;
            }

        };

        /**
         * 
         */
        class SimpleAreaAssembler : public AreaAssembler
        {
        protected:

            /**
             * 
             */
            void create_areas(
                memory::Buffer<memory::Area>& areas,
                std::string name,
                level_type level,
                const std::vector<ProtoGroup>& groups
            ) const override {
                // Retrieve the group
                assert(groups.size() == 1);
                ProtoGroup group = groups.at(0);

                // Prepare the result area
                memory::Area area( areas.size(), name, level );

                // Convert and add outer ring
                memory::Ring outer( 0, group.outer.nodes );
                area.add_outer(outer);
                area.add_ways(group.outer.ways);

                // Convert and add inner rings
                for (size_t i = 0; i < group.inners.size(); i++)
                {
                    memory::Ring inner( i, group.inners.at(i).nodes) ;
                    area.add_inner(outer, inner);
                    area.add_ways(group.inners.at(i).ways);
                }

                // Add the created area to the buffer
                areas.push_back(area);
            }

        public:

            using AreaAssembler::AreaAssembler;

        };

        /**
         * 
         */
        class ComplexAreaAssembler : public AreaAssembler
        {
        protected:

            /**
             * 
             */
            void create_areas(
                memory::Buffer<memory::Area>& areas,
                std::string name,
                level_type level,
                const std::vector<ProtoGroup>& groups
            ) const override {
                // Prepare the result area
                memory::Area area( areas.size(), name, level );

                object_id_type inner_id = 0;
                for (size_t i = 0; i < groups.size(); i++)
                {
                    // Retrieve the group
                    ProtoGroup group = groups.at(i);

                    // Convert and add outer ring
                    memory::Ring outer( i, group.outer.nodes );
                    area.add_outer(outer);
                    area.add_ways(group.outer.ways);

                    // Convert and add inner rings
                    for (const ProtoRing& i_con : group.inners)
                    {
                        memory::Ring inner(inner_id, i_con.nodes) ;
                        area.add_inner(outer, inner);
                        area.add_ways(i_con.ways);
                        ++inner_id;
                    }

                }

                // Add the created area to the buffer
                areas.push_back(area);
            }

        public:

            using AreaAssembler::AreaAssembler;

        };

    }

}