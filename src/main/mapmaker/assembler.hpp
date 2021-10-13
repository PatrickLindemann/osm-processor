#pragma once

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <stack>

#include <osmium/osm/types.hpp>
#include <string>

#include "model/memory/node.hpp"
#include "model/memory/ring.hpp"
#include "model/memory/way.hpp"
#include "model/memory/relation.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/types.hpp"

#include "model/geometry/point.hpp"
#include "model/geometry/segment.hpp"
#include "functions/intersect.hpp"

namespace mapmaker
{
    
    namespace assembler
    {

        using namespace model;

        template <typename T>
        class Assembler
        {
        public:

            using id_type = memory::object_id_type;

            enum Strategy
            {
                KEEP,
                SPLIT
            };

        protected:

            const memory::Buffer<memory::Node<T>> m_nodes;
            const memory::Buffer<memory::Way<T>> m_ways;
            const memory::Buffer<memory::Relation<T>> m_relations;             
            Strategy m_strategy;  

            /**
             * The output area buffer.
             */
            memory::Buffer<memory::Area<T>> m_areas;

        public:

            Assembler(
                const memory::Buffer<memory::Node<T>>& nodes,
                const memory::Buffer<memory::Way<T>>& ways,
                const memory::Buffer<memory::Relation<T>> relations,
                Strategy strategy = Strategy::SPLIT
            ) : m_nodes(nodes), m_ways(ways), m_relations(relations), m_strategy(strategy) {}

        protected:

            struct Ring
            {
                std::vector<id_type> ways;
                std::vector<id_type> nodes;
            };

            void add_way(Ring& ring, id_type way_ref)
            {
                // Add way reference to ring
                const memory::Way<T>& way = m_ways.get(way_ref);
                ring.ways.push_back(way_ref);
                // Check if this is the first way that gets inserted
                // into this ring
                if (ring.nodes.empty())
                {
                    // Add all node references of the way in normal order
                    for (auto it = way.cbegin(); it < way.cend(); it++)
                    {
                        ring.nodes.push_back(*it);
                    }
                    return;
                }
                // Check if the way needs to be reversed
                if (ring.nodes.back() == way.back())
                {
                    // Add node references except the first to ring in reverse order
                    for (auto it = way.crbegin() + 1; it < way.crend(); it++)
                    {
                        ring.nodes.push_back(*it);
                    }
                    return;
                }
                // Add node references except the first to ring in normal order
                for (auto it = way.cbegin() + 1; it < way.cend(); it++)
                {
                    ring.nodes.push_back(*it);
                }
            }

            void remove_last_way(Ring& ring)
            {
                // Remove last node references
                const memory::Way<T>& way = m_ways.get(ring.ways.back());
                for (size_t i = 0; i < way.size(); i++)
                {
                    ring.nodes.pop_back();
                }
                // Remove last way reference
                ring.ways.pop_back();
            }

            std::vector<geometry::Segment<T>> get_segments(const Ring& ring)
            {
                std::vector<geometry::Segment<T>> result;
                // Add the way segments
                for (size_t i = 0, j = 1; j < ring.nodes.size(); i++, j++)
                {
                    result.push_back({
                        m_nodes.get(i).point(),
                        m_nodes.get(j).point() 
                    });
                }
                return result;
            }

            bool outside_x_range(
                const geometry::Segment<T>& s1,
                const geometry::Segment<T>& s2
            ) noexcept {
                return s1.first().x > s2.last().x;
            }

            bool y_range_overlap(
                const geometry::Segment<T>& s1,
                const geometry::Segment<T>& s2
            ) noexcept {
                const std::pair<T, T> m1 = std::minmax(s1.first().y, s1.last().y);
                const std::pair<T, T> m2 = std::minmax(s2.first().y, s2.last().y);
                return !(m1.first > m2.second || m2.first > m1.second);
            }

            bool is_closed(const Ring& ring)
            {
                return ring.nodes.front() == ring.nodes.back();
            }

            bool is_valid(const Ring& ring)
            {
                // Get ring segments
                std::vector<geometry::Segment<T>> segments = get_segments(ring);
                if (segments.size() < 2)
                {
                    return true;
                }
                // Check if the ring intersects itself
                for (auto it1 = segments.cbegin(); it1 != segments.cend(); it1++)
                {
                    const geometry::Segment<T>& s1 = *it1;
                    for (auto it2 = it1 + 1; it2 != segments.cend(); it2++)
                    {
                        const geometry::Segment<T>& s2 = *it2;
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

            bool create_ring(
                Ring& ring,
                std::vector<id_type>& way_refs,
                std::unordered_map<id_type, bool>& processed
            ) {
                if (is_closed(ring))
                {
                    return is_valid(ring);
                }
                // Find candidates
                std::stack<id_type> candidates{};
                for (const auto& way_ref : way_refs)
                {
                    if (!processed.at(way_ref))
                    {
                        const memory::Way<T>& next = m_ways.get(way_ref);
                        if (ring.nodes.back() == next.front()
                         || ring.nodes.back() == next.back())
                        {
                            candidates.push(next.id());
                        }
                    }
                }
                // Try to continue ring with the identified candidates
                while (!candidates.empty())
                {
                    // Get next candidate
                    id_type& candidate = candidates.top();
                    candidates.pop();
                    // Add current candidate to the ring
                    add_way(ring, candidate);
                    processed.at(candidate) = true;
                    // Try to create the ring with the current candidate
                    bool finished = create_ring(ring, way_refs, processed);
                    // Check if ring was finished
                    if (finished)
                    {
                        return true;
                    }
                    // Backtrack: Remove candidate from ring and try another
                    remove_last_way(ring);
                    processed.at(candidate) = false;
                }
                return false;
            }

            /**
             * https://wiki.openstreetmap.org/wiki/Relation:multipolygon/Algorithm
             */
            std::vector<Ring> create_rings(std::vector<id_type>& way_refs)
            {
                // Prepare the result vector of rings
                std::vector<Ring> result;
                // Prepare the lookup table for processed way indices
                std::unordered_map<id_type, bool> processed = {};
                for (const auto& way_ref : way_refs)
                {
                    processed[way_ref] = false;
                }
                // Find the next non-processed way
                for (const auto& way_ref : way_refs)
                {
                    if (!processed.at(way_ref))
                    {
                        // Start a new ring and create it with backtracking
                        Ring ring;
                        add_way(ring, way_ref);
                        processed.at(way_ref) = true;
                        // Create the ring
                        create_ring(ring, way_refs, processed);
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

            memory::Ring<T> convert_ring(const id_type id, const Ring& ring)
            {
                memory::Ring<T> result{ id };
                for (const id_type node_ref : ring.nodes)
                {
                    result.push_back(node_ref);
                }
                return result;
            }

            void add_way_references(memory::Area<T>& area, const Ring& ring)
            {
                for (const id_type way_ref : ring.ways)
                {
                    area.add_way_reference(way_ref);
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
                memory::Area<T> area = { m_areas.size(), name, level };
                // Convert and add outer ring
                memory::Ring<T> outer = convert_ring(0, group.outer);
                area.add_outer_ring(outer);
                add_way_references(area, group.outer);
                // Convert and add inner rings
                for (size_t i = 0; i < group.inners.size(); i++)
                {
                    memory::Ring<T> inner = convert_ring(
                        i,
                        group.inners.at(i)
                    );
                    area.add_inner_ring(outer, inner);
                    add_way_references(area, group.inners.at(i));
                }
                m_areas.append(area);
            }

            void create_multipolygon_area(
                std::string name,
                unsigned short level,
                const std::vector<Group>& groups
            ) {
                // Prepare result area
                memory::Area<T> area = { m_areas.size(), name, level };
                for (size_t i = 0; i < groups.size(); i++)
                {
                    // Get current group
                    const Group& group = groups.at(i);
                    // Convert and add outer ring
                    memory::Ring<T> outer = convert_ring(i, group.outer);
                    area.add_outer_ring(outer);
                    add_way_references(area, group.outer);
                    // Convert and add inner rings
                    for (size_t j = 0; j < group.inners.size(); j++)
                    {
                        memory::Ring<T> inner = convert_ring(
                            i * group.inners.size() + j,
                            group.inners.at(j)
                        );
                        area.add_inner_ring(outer, inner);
                        add_way_references(area, group.inners.at(j));
                    }
                }
                m_areas.append(area);
            }

        public:

            memory::Buffer<memory::Area<T>>& build()
            {
                for (const memory::Relation<T>& relation : m_relations)
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
                return m_areas;
            }

        };

    }

}