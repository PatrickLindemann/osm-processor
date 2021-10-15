#pragma once

#include <algorithm>
#include <stack>
#include <string>

#include <boost/lexical_cast.hpp>
#include <osmium/osm/types.hpp>


#include "model/memory/entity.hpp"
#include "model/memory/entity_ref_list.hpp"
#include "model/memory/node.hpp"
#include "model/memory/ring.hpp"
#include "model/memory/type.hpp"
#include "model/memory/way.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/type.hpp"

#include "model/geometry/point.hpp"
#include "model/geometry/segment.hpp"
#include "functions/intersect.hpp"

namespace mapmaker
{
    
    namespace assembler
    {

        using namespace model::memory;
        using namespace model::geometry;

        class Assembler
        {
            
            /* Members */

            /**
             * The result area buffer
             */
            Buffer<Area>& m_area_buffer;

            /**
             * 
             */
            Buffer<Node>&      m_node_buffer;
            Buffer<Way>&       m_way_buffer;
            Buffer<Relation>&  m_relation_buffer;

            /**
             * 
             */
            bool m_split_areas;  
            
            /**
             * 
             */
            double m_threshold;

        public:

            /* Constructors */

            Assembler(
                Buffer<Area>& areas,
                Buffer<Node>& nodes,
                Buffer<Way>& ways,
                Buffer<Relation>& relations,
                bool split_areas,
                double threshold
            ) : m_area_buffer(areas), m_node_buffer(nodes), m_way_buffer(ways), m_relation_buffer(relations),
                m_split_areas(split_areas), m_threshold(threshold) {};

        protected:

            /* Helper classes */

            /**
             *
             */
            struct Ring
            {
                /**
                 * 
                 */
                EntityRefList<NodeRef> nodes;

                /**
                 * 
                 */
                EntityRefList<WayRef> ways;
            
                /**
                 * 
                 */
                void add_way_nodes(const Way& way)
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
                 */
                void remove_way_nodes(const Way& way)
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

            };

            /* Helper methods */

            /**
             * 
             */
            std::vector<Segment<double>> get_segments(const Ring& ring)
            {
                std::vector<Segment<double>> segments;
                for (size_t i = 0, j = ring.nodes.size() - 1; i < ring.nodes.size(); i++, j = i - 1)
                {
                    NodeRef left = ring.nodes.at(j);
                    NodeRef right = ring.nodes.at(i);
                    segments.push_back({
                        m_node_buffer.at(left).point(),
                        m_node_buffer.at(right).point() 
                    });
                }
                return segments;
            }

            /**
             * 
             */
            bool outside_x_range(
                const Segment<double>& s1,
                const Segment<double>& s2
            ) noexcept {
                return s1.first().x() > s2.last().x();
            }

            /**
             * 
             */
            bool y_range_overlap(
                const Segment<double>& s1,
                const Segment<double>& s2
            ) noexcept {
                std::pair<double, double> m1 = std::minmax(s1.first().y(), s1.last().y());
                std::pair<double, double> m2 = std::minmax(s2.first().y(), s2.last().y());
                return !(m1.first > m2.second || m2.first > m1.second);
            }

            /**
             * 
             */
            bool is_closed(const Ring& ring)
            {
                return ring.nodes.front() == ring.nodes.back();
            }

            /**
             * 
             */
            bool is_valid(const Ring& ring)
            {
                // Get ring segments
                std::vector<Segment<double>> segments = get_segments(ring);
                if (segments.size() < 3)
                {
                    return false;
                }
                // Check if the ring intersects itself
                for (auto it1 = segments.cbegin(); it1 != segments.cend(); it1++)
                {
                    const Segment<double>& s1 = *it1;
                    for (auto it2 = it1 + 1; it2 != segments.cend(); it2++)
                    {
                        const Segment<double>& s2 = *it2;
                        if (outside_x_range(s1, s2))
                        {
                            break;
                        }
                        else if (y_range_overlap(s1, s2))
                        {
                            if (functions::intersect(s1, s2))
                            {
                                // return false; // TODO return false
                            }
                        }
                    }
                }
                return true;
            }
            
            /**
             * 
             */
            bool complete_ring(
                Ring& ring,
                EntityRefList<WayRef>& ways,
                std::unordered_map<WayRef, bool, EntityRefHasher>& processed
            ) {
                if (is_closed(ring))
                {
                    return is_valid(ring);
                }
                // Find candidates
                std::stack<WayRef> candidates{};
                for (const WayRef& ref : ways)
                {
                    if (!processed.at(ref))
                    {
                        const Way& next = m_way_buffer.at(ref);
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
                    WayRef& candidate = candidates.top();
                    candidates.pop();
                    Way& way = m_way_buffer.at(candidate);
                    // Add current candidate to the ring
                    ring.add_way_nodes(way);
                    processed.at(candidate) = true;
                    // Try to create the ring with the current candidate
                    bool finished = complete_ring(ring, ways, processed);
                    // Check if ring was finished
                    if (finished)
                    {
                        return true;
                    }
                    // Backtrack: Remove candidate from ring and try another
                    ring.remove_way_nodes(way);
                    processed.at(candidate) = false;
                }
                return false;
            }

            /**
             * https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
             */
            std::vector<Ring> create_rings(EntityRefList<WayRef>& ways)
            {
                // Prepare the result vector of rings
                std::vector<Ring> result;
                // Prepare the lookup table for processed way indices
                std::unordered_map<WayRef, bool, EntityRefHasher> processed = {};
                for (const WayRef& ref : ways)
                {
                    processed[ref] = false;
                }
                // Find the next non-processed way
                for (const WayRef& ref : ways)
                {
                    if (!processed.at(ref))
                    {
                        // Start a new ring and create it with backtracking
                        Ring ring;
                        Way& way = m_way_buffer.at(ref);
                        ring.add_way_nodes(way);
                        processed.at(ref) = true;
                        // Create the ring
                        complete_ring(ring, ways, processed);
                        // Push the created ring to the result collection if
                        // it is valid
                        if (ring.nodes.size() > 2)
                        {
                            result.push_back(ring);
                        }
                    }
                }
                return result;
            }

            struct Group
            {
                Ring outer;
                std::vector<Ring> inners;
            };

            /**
             * https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
             */
            std::vector<Group> group_rings(
                const std::vector<Ring>& outer_rings,
                const std::vector<Ring>& inner_rings
            ) {
                std::vector<Group> result{};
                // Check if more than one outer rings were assembled
                if (outer_rings.size() < 2)
                {
                    if (inner_rings.size() == 0)
                    {
                        // Simple case: 1 outer, 0 inners
                        // No grouping needs to be performed
                        result.push_back({ outer_rings.at(0) });
                        return result;
                    }
                    // Simple case: 1 outer, N inners
                    // All inner rings have to be part of the single outer ring
                    result.push_back({ outer_rings.at(0), inner_rings });
                    return result;
                }
                if (inner_rings.size() == 0)
                {
                    // Simple case: N outers, 0 inners
                    // No grouping needs to be performed
                    for (const Ring& outer : outer_rings)
                    {
                        result.push_back({ outer });
                    }
                    return result;
                }
                // Complex case: N outers, N inners
                // Prepare the lookup list for processed inner ring indices
                std::vector<bool> processed(inner_rings.size(), false);
                // Perform grouping algorithm
                // TODO
                return result;
            }

            Ring convert_ring(const id_type id, const Ring& ring)
            {
                Ring result{ id };
                for (const id_type node_ref : ring.nodes)
                {
                    result.push_back(node_ref);
                }
                return result;
            }

            void add_way_nodes_references(Area& area, const Ring& ring)
            {
                for (const id_type way_ref : ring.ways)
                {
                    area.add_way_nodes_reference(way_ref);
                }
            }

            /**
             * 
             */
            void create_area(
                std::string name,
                unsigned short level,
                const Group& group
            ) {
                // Prepare result area
                Area area = { m_area_buffer.size(), name, level };
                // Convert and add outer ring
                Ring outer = convert_ring(0, group.outer);
                area.add_outer_ring(outer);
                add_way_nodes_references(area, group.outer);
                // Convert and add inner rings
                for (size_t i = 0; i < group.inners.size(); i++)
                {
                    Ring inner = convert_ring(
                        i,
                        group.inners.at(i)
                    );
                    area.add_inner_ring(outer, inner);
                    add_way_nodes_references(area, group.inners.at(i));
                }
                m_area_buffer.append(area);
            }

            void create_multipolygon_area(
                std::string name,
                unsigned short level,
                const std::vector<Group>& groups
            ) {
                // Prepare result area
                Area area = { m_area_buffer.size(), name, level };
                for (size_t i = 0; i < groups.size(); i++)
                {
                    // Get current group
                    const Group& group = groups.at(i);
                    // Convert and add outer ring
                    Ring outer = convert_ring(i, group.outer);
                    area.add_outer_ring(outer);
                    add_way_nodes_references(area, group.outer);
                    // Convert and add inner rings
                    for (size_t j = 0; j < group.inners.size(); j++)
                    {
                        Ring inner = convert_ring(
                            i * group.inners.size() + j,
                            group.inners.at(j)
                        );
                        area.add_inner_ring(outer, inner);
                        add_way_nodes_references(area, group.inners.at(j));
                    }
                }
                m_area_buffer.append(area);
            }

        public:

            void run()
            {
                for (const Relation& relation : m_relation_buffer)
                {
                    // Retrieve relation tags
                    std::string name = relation.get_tag("name");
                    unsigned short level = boost::lexical_cast<unsigned short>(relation.get_tag("admin_level"));

                    // Retrieve inner and outer way members
                    std::vector<id_type> outer_ways = relation.get_members_by_role("outer");
                    std::vector<id_type> inner_ways = relation.get_members_by_role("inner");

                    // Assemble the the rings
                    std::vector<Ring> outer_rings = create_rings(outer_ways);
                    if (outer_rings.empty())
                    {
                        continue;
                    }
                    std::vector<Ring> inner_rings = create_rings(inner_ways);

                    // Group the rings
                    const std::vector<Group>& groups = group_rings(outer_rings, inner_rings);
        
                    if (groups.size() > 1)
                    {
                        if (m_strategy == Strategy::KEEP)
                        {
                            // Create single, multipolygon area
                            create_multipolygon_area(name, level, groups);
                        }
                        else
                        {
                            // Split area into multiple polygon areas
                            for (size_t i = 0; i < groups.size(); i++)
                            {
                                create_area(
                                    name + " " + std::to_string(i),
                                    level,
                                    groups.at(i)
                                );
                            }
                        }
                    }
                    else
                    {
                        create_area(name, level, groups.at(0));
                    }
                }
            }

        };

    }

}