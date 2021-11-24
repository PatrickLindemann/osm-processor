#pragma once

#include <map>
#include <set>

#include "model/graph/undirected_graph.hpp"

#include "util/insert.hpp"

namespace mapmaker
{

    /**
     * A class for finding neighborship relations on a collection of areas.
     */
    class NeighborInspector
    {

        /* Members */

        model::level_type m_level;

    public:

        /* Constructors */

        NeighborInspector(model::level_type level) : m_level(level) {};

        /* Methods */

        /**
         * Create the neighbor graph for a osmium buffer of areas by checking
         * which areas contain the same node references. If they do so, they are
         * considered to be neighbors. As the neighbor relation is symmetric,
         * the graph is chosen as undirected.
         * 
         * @returns The neighbor graph, where vertices represent the areas and
         *          edges represent a neighborship between to areas
         * 
         * Time complexity: Linear
         */
        model::graph::UndirectedGraph run(const osmium::memory::Buffer& buffer)
        {
            graph::UndirectedGraph neighbors;
            
            std::map<osmium::object_id_type, std::set<osmium::object_id_type>> references{};
            for (const osmium::Area& area : buffer.select<osmium::Area>())
            {
                // Create a vertex for the area in the neighbor graph
                neighbors.insert_vertex(area.id());

                // Collect the node references for this area
                std::set<osmium::object_id_type> nodes = {};
                for (const osmium::OuterRing& outer : area.outer_rings())
                {
                    for (const osmium::NodeRef& nr : outer)
                    {
                        util::insert(references, nr.ref(), area.id());
                    }
                    for (const osmium::InnerRing& inner : area.inner_rings(outer))
                    {
                        for (const osmium::NodeRef& nr : inner)
                        {
                            util::insert(references, nr.ref(), area.id());
                        }
                    }
                }
            }

            // Create edges in the graph for each two areas that share a common
            // node.
            for (const auto& [node, areas] : references)
            {
                for (auto it1 = areas.begin(); it1 != areas.end(); it1++)
                {
                    for (auto it2 = std::next(it1, 1); it2 != areas.end(); it2++)
                    {
                        neighbors.insert_edge(std::make_pair(*it1, *it2));
                    }
                }
            }

            return neighbors;
        }

    };

    class ComponentInspector
    {
    public:

        /* Constructors */

        ComponentInspector() {}

        /* Methods */

        /**
         * Retrieve the connected components of a neighborsship graph, where 
         * each vertex has a path to any other vertex in the same component.
         * Components are calculated by performing a depth-first search on the
         * specified neighbor graph.
         *
         * @param neighbors The neighbor graph
         * @returns         The list of connected components, where the index
         *                  represents the component and the set the areas that
         *                  are contained in it
         *
         * Time complexity: Linear
         */
        std::vector<std::set<osmium::object_id_type>> run(const model::graph::UndirectedGraph& neighbors)
        {
            if (neighbors.vertex_count() == 0)
            {
                return {};
            }

            // Prepare the component map that stores the component for each
            // vertex in the neighbor graph.
            std::map<osmium::object_id_type, std::size_t> components;

            // Calculate the connected components.
            std::size_t c = 0;
            for (const graph::vertex_type& vertex : neighbors.vertices())
            {
                // Check if the current vertex was visited already
                if (!components.count(vertex))
                {
                    // Perform depth-first-search with the current vertex as
                    // starting point
                    std::stack<graph::vertex_type> stack;
                    stack.push(vertex);
                    while (!stack.empty())
                    {
                        graph::vertex_type v = stack.top();
                        stack.pop();
                        // Mark the current vertex as part of the current
                        // component and add its unvisited adjacents to
                        // the stack for the next iterations.
                        components[v] = c;
                        for (const auto& adjacent : neighbors.adjacents(v))
                        {
                            if (!components.count(adjacent))
                            {
                                stack.push(adjacent);
                            }
                        }
                    }
                    // All vertices that were reachable from the chosen start
                    // vertex were vistied, therefore the connected component
                    // is complete.
                    ++c;
                }
            }

            // Reverse the component map, such that the component becomes the
            // index and the areas become the values.
            std::vector<std::set<osmium::object_id_type>> result(c, std::set<osmium::object_id_type>{});
            for (const auto& [area, component] : components)
            {
                result.at(component).insert(area);
            }

            return result;
        }

    };

    template <typename T>
    class HierarchyInspector
    {
    public:

        /* Types */

        using hierarchy_t = std::map<object_id_type, std::set<object_id_type>>;

    protected:

        /* Members */

    public:

        /* Constructors */

        HierarchyInspector() {}

        /* Methods */

        hierarchy_t run(const std::map<object_id_type, Boundary<T>>& boundaries)
        {
            hierarchy_t hierarchy{};
            for (const auto& [id, boundary] : boundaries)
            {
                hierarchy[id] = {};
            }
            return hierarchy;
        }

    };

    //class HirarchyInspector
    //{

    //    /* Classes */

    //    struct BoundsContainer
    //    {
    //        geometry::Rectangle<double> outer;
    //        std::vector<geometry::Rectangle<double>> inners;
    //    };

    //    /* Types */

    //    using cache_type = std::unordered_map<object_id_type, std::vector<BoundsContainer>>;

    //    /* Members */

    //    std::vector<map::Territory>& m_territories;
    //    std::vector<map::Bonus>& m_bonuses;
    //    std::vector<map::SuperBonus>& m_super_bonuses;

    //    /**
    //     *
    //     */
    //    cache_type m_bonus_bounds;
    //    cache_type m_super_bounds;

    //public:

    //    /* Constructors */

    //    HirarchyCreator(
    //        std::vector<map::Territory>& territories,
    //        std::vector<map::Bonus>& bonuses,
    //        std::vector<map::SuperBonus>& super_bonuses
    //    ) : m_territories(territories), m_bonuses(bonuses), m_super_bonuses(super_bonuses) {}

    //protected:

    //    /**
    //     *
    //     */
    //    BoundsContainer get_bounds(const geometry::Polygon<double>& polygon)
    //    {
    //        BoundsContainer container{
    //            functions::envelope(polygon.outer())
    //        };
    //        for (const geometry::Ring<double>& inner : polygon.inners())
    //        {
    //            container.inners.push_back(functions::envelope(inner));
    //        }
    //        return container;
    //    }

    //    /**
    //      *
    //      */
    //    std::vector<BoundsContainer> get_bounds(const geometry::MultiPolygon<double>& multipolygon)
    //    {
    //        std::vector<BoundsContainer> containers;
    //        for (const geometry::Polygon<double>& polygon : multipolygon.polygons())
    //        {
    //            containers.push_back(get_bounds(polygon));
    //        }
    //        return containers;
    //    }

    //    /**
    //     *
    //     */
    //    bool polygon_in_multipolygon(
    //        const geometry::Polygon<double>& polygon,
    //        const geometry::Rectangle<double>& polygon_bounds,
    //        const geometry::MultiPolygon<double>& multipolygon,
    //        const std::vector<BoundsContainer>& multipolygon_bounds
    //    ) {
    //        // Try to find the first bonus that the territory is fully
    //        // contained in
    //        for (std::size_t i = 0; i < multipolygon.polygons().size(); i++)
    //        {
    //            // Retrieve the exclave polygon
    //            const geometry::Polygon<double>& exclave = multipolygon.polygons().at(i);
    //            // Check the outer bounding boxes first
    //            if (!functions::rectangle_in_rectangle(
    //                polygon_bounds,
    //                multipolygon_bounds.at(i).outer
    //            )) {
    //                continue;
    //            }
    //            // Check the actual geometries
    //            if (!functions::ring_in_ring(polygon.outer(), exclave.outer()))
    //            {
    //                continue;
    //            }
    //            // Assert that the polygon ring is not contained
    //            // within a hole of the current parent outer ring
    //            bool in_hole = false;
    //            for (std::size_t j = 0; j < exclave.inners().size(); j++)
    //            {
    //                // Retrieve the inner hole
    //                const geometry::Ring<double>& inner = exclave.inners().at(i);
    //                // Check the bounding boxes first
    //                if (!functions::rectangle_in_rectangle(
    //                    polygon_bounds,
    //                    multipolygon_bounds.at(i).inners.at(j)
    //                )) {
    //                    continue;
    //                }
    //                // Check the actual geometries
    //                if (functions::ring_in_ring(polygon.outer(), inner))
    //                {
    //                    // An exclave of the child area is contained in an inner hole
    //                    // of the parent exclave, which means that the child exclave is
    //                    // not contained in this parent exclave.
    //                    in_hole = true;
    //                    break;
    //                }
    //            }
    //            if (!in_hole)
    //            {
    //                return true;
    //            }
    //        }

    //        // No parent area was found
    //        return false;
    //    }

    //    /**
    //     *
    //     */
    //    object_id_type group_territory(
    //        const map::Territory& territory,
    //        const std::vector<map::Bonus>& bonuses
    //    ) {
    //        for (const map::Bonus& bonus : bonuses)
    //        {
    //            // Compare bounding boxes first
    //            if (!functions::rectangle_in_rectangle(territory.bounds(), bonus.bounds()))
    //            {
    //                continue;
    //            }
    //            // Compare geometries
    //            if (polygon_in_multipolygon(
    //                territory.geometry(),
    //                territory.bounds(),
    //                bonus.geometry(),
    //                m_bonus_bounds.at(bonus.id())
    //            )) {
    //                return bonus.id();
    //            }
    //        }
    //        return -1;
    //    }

    //    /**
    //     *
    //     */
    //    object_id_type group_bonus(
    //        const map::Bonus& bonus,
    //        const std::vector<map::SuperBonus>& super_bonuses
    //    ) {
    //        for (const map::Bonus& super_bonus : super_bonuses)
    //        {
    //            // Compare general bounding boxes first
    //            if (!functions::rectangle_in_rectangle(bonus.bounds(), super_bonus.bounds()))
    //            {
    //                continue;
    //            }
    //            for (std::size_t i = 0; i < bonus.geometry().polygons().size(); i++)
    //            {
    //                // Retrieve the polygon and its bounds from the cache
    //                const geometry::Polygon<double>& polygon = bonus.geometry().polygons().at(i);
    //                const geometry::Rectangle<double>& bounds = m_bonus_bounds.at(bonus.id()).at(i).outer;
    //                // Compare the geometries
    //                if (polygon_in_multipolygon(
    //                    polygon,
    //                    bounds,
    //                    super_bonus.geometry(),
    //                    m_super_bounds.at(super_bonus.id())
    //                )) {
    //                    return super_bonus.id();
    //                }
    //            }
    //        }
    //        return -1;
    //    }

    //public:

    //    /**
    //     *
    //     */
    //    void create_hirarchy()
    //    {
    //        // If no bonuses were specified, no hirarchy needs
    //        // to be created
    //        if (m_bonuses.size() == 1)
    //        {
    //            return;
    //        }

    //        // Pre-calculate the bounds of the bonus geometries
    //        for (const map::Bonus& bonus : m_bonuses)
    //        {
    //            m_bonus_bounds[bonus.id()] = get_bounds(bonus.geometry());
    //        }

    //        // Group territories to the bonuses
    //        for (const map::Territory& territory : m_territories)
    //        {
    //            // Find the bonus that the current territory is contained in
    //            object_id_type bonus_id = group_territory(territory, m_bonuses);
    //            // Check if a bonus was found
    //            if (bonus_id >= 0)
    //            {
    //                // Add territory to child list of the bonus
    //                m_bonuses.at(bonus_id).children().push_back({ territory.id() });
    //            }
    //        }

    //        // Pre-calculate the bounds of the super bonus geometries
    //        for (const map::SuperBonus& super_bonus : m_super_bonuses)
    //        {
    //            m_super_bounds[super_bonus.id()] = get_bounds(super_bonus.geometry());
    //        }

    //        // Group bonuses to the super bonuses
    //        for (const map::Bonus& bonus : m_bonuses)
    //        {
    //            // Find the super bonus that the current territory is contained in
    //            object_id_type super_id = group_bonus(bonus, m_super_bonuses);
    //            // Check if a super bonus was found
    //            if (super_id > 0)
    //            {
    //                // Add territory to child list of the bonus
    //                m_bonuses.at(super_id).children().push_back({ bonus.id() });
    //            }
    //        }

    //    }
    //};

}