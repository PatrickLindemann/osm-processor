#pragma once

#include <cfloat>
#include <system_error>
#include <unordered_set>

#include "functions/distance.hpp"
#include "functions/envelope.hpp"
#include "functions/intersect.hpp"
#include "functions/convex_hull.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/geometry/point.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/segment.hpp"
#include "model/graph/undirected_graph.hpp"
#include "model/map/territory.hpp"
#include "model/map/bonus.hpp"
#include "model/type.hpp"


using namespace model;

namespace mapmaker
{

    namespace connector
    {

       /**
         * 
         */
        class HirarchyCreator
        {

            /* Classes */

            struct BoundsContainer
            {
                geometry::Rectangle<double> outer;
                std::vector<geometry::Rectangle<double>> inners;
            };

            /* Types */

            using cache_type = std::unordered_map<object_id_type, std::vector<BoundsContainer>>;

            /* Members */

            std::vector<map::Territory>& m_territories;
            std::vector<map::Bonus>& m_bonuses;
            std::vector<map::SuperBonus>& m_super_bonuses;

            /**
             * 
             */
            cache_type m_bonus_bounds;
            cache_type m_super_bounds;

        public:

            /* Constructors */

            HirarchyCreator(
                std::vector<map::Territory>& territories,
                std::vector<map::Bonus>& bonuses,
                std::vector<map::SuperBonus>& super_bonuses
            ) : m_territories(territories), m_bonuses(bonuses), m_super_bonuses(super_bonuses) {}

        protected:

            /**
             * 
             */
            BoundsContainer get_bounds(const geometry::Polygon<double>& polygon)
            {
                BoundsContainer container {
                    functions::envelope(polygon.outer())
                };
                for (const geometry::Ring<double>& inner : polygon.inners())
                {
                    container.inners.push_back(functions::envelope(inner));
                }
                return container;
            }

           /**
             * 
             */
            std::vector<BoundsContainer> get_bounds(const geometry::MultiPolygon<double>& multipolygon)
            {
                std::vector<BoundsContainer> containers;
                for (const geometry::Polygon<double>& polygon : multipolygon.polygons())
                {
                    containers.push_back(get_bounds(polygon));
                }
                return containers;
            }

            /**
             * 
             */
            bool polygon_in_multipolygon(
                const geometry::Polygon<double>& polygon,
                const geometry::Rectangle<double>& polygon_bounds,
                const geometry::MultiPolygon<double>& multipolygon,
                const std::vector<BoundsContainer>& multipolygon_bounds
            ) {
                // Try to find the first bonus that the territory is fully
                // contained in
                for (size_t i = 0; i < multipolygon.polygons().size(); i++)
                {
                    // Retrieve the exclave polygon
                    const geometry::Polygon<double>& exclave = multipolygon.polygons().at(i);
                    // Check the outer bounding boxes first
                    if (!functions::rectangle_in_rectangle(
                        polygon_bounds,
                        multipolygon_bounds.at(i).outer
                    )) {
                        continue;
                    }
                    // Check the actual geometries
                    if (!functions::ring_in_ring(polygon.outer(), exclave.outer()))
                    {
                        continue;
                    }
                    // Assert that the polygon ring is not contained
                    // within a hole of the current parent outer ring
                    bool in_hole = false;
                    for (size_t j = 0; j < exclave.inners().size(); j++)
                    {
                        // Retrieve the inner hole
                        const geometry::Ring<double>& inner = exclave.inners().at(i);
                        // Check the bounding boxes first
                        if (!functions::rectangle_in_rectangle(
                            polygon_bounds,
                            multipolygon_bounds.at(i).inners.at(j)
                        )) {
                            continue;
                        }
                        // Check the actual geometries
                        if (functions::ring_in_ring(polygon.outer(), inner))
                        {
                            // An exclave of the child area is contained in an inner hole
                            // of the parent exclave, which means that the child exclave is
                            // not contained in this parent exclave.
                            in_hole = true;
                            break;
                        }
                    }
                    if (!in_hole)
                    {
                        return true;
                    }
                }

                // No parent area was found
                return false;
            }

            /**
             * 
             */
            object_id_type group_territory(
                const map::Territory& territory,
                const std::vector<map::Bonus>& bonuses
            ) {
                for (const map::Bonus& bonus : bonuses)
                {
                    // Compare bounding boxes first
                    if (!functions::rectangle_in_rectangle(territory.bounds(), bonus.bounds()))
                    {
                        continue;
                    }
                    // Compare geometries
                    if (polygon_in_multipolygon(
                        territory.geometry(),
                        territory.bounds(),
                        bonus.geometry(),
                        m_bonus_bounds.at(bonus.id())
                    )) {
                        return bonus.id();
                    }
                }
                return -1;
            }

            /**
             * 
             */
            object_id_type group_bonus(
                const map::Bonus& bonus,
                const std::vector<map::SuperBonus>& super_bonuses
            ) {
                for (const map::Bonus& super_bonus : super_bonuses)
                {
                    // Compare general bounding boxes first
                    if (!functions::rectangle_in_rectangle(bonus.bounds(), super_bonus.bounds()))
                    {
                        continue;
                    }
                    for (size_t i = 0; i < bonus.geometry().polygons().size(); i++)
                    {
                        // Retrieve the polygon and its bounds from the cache
                        const geometry::Polygon<double>& polygon = bonus.geometry().polygons().at(i);
                        const geometry::Rectangle<double>& bounds = m_bonus_bounds.at(bonus.id()).at(i).outer;                    
                        // Compare the geometries
                        if (polygon_in_multipolygon(
                            polygon,
                            bounds,
                            super_bonus.geometry(),
                            m_super_bounds.at(super_bonus.id())
                        )) {
                            return super_bonus.id();
                        }
                    }
                }
                return -1;
            }

        public:

            /**
             * 
             */
            void create_hirarchy()
            {
                // If no bonuses were specified, no hirarchy needs
                // to be created
                if (m_bonuses.size() == 1)
                {
                    return;
                }

                // Pre-calculate the bounds of the bonus geometries
                for (const map::Bonus& bonus : m_bonuses)
                {
                    m_bonus_bounds[bonus.id()] = get_bounds(bonus.geometry());
                }
                
                // Group territories to the bonuses
                for (const map::Territory& territory : m_territories)
                {
                    // Find the bonus that the current territory is contained in
                    object_id_type bonus_id = group_territory(territory, m_bonuses);
                    // Check if a bonus was found
                    if (bonus_id >= 0)
                    {
                        // Add territory to child list of the bonus
                        m_bonuses.at(bonus_id).children().push_back({ territory.id() });
                    }
                }

                // Pre-calculate the bounds of the super bonus geometries
                for (const map::SuperBonus& super_bonus : m_super_bonuses)
                {
                    m_super_bounds[super_bonus.id()] = get_bounds(super_bonus.geometry());
                }

                // Group bonuses to the super bonuses
                for (const map::Bonus& bonus : m_bonuses)
                {
                    // Find the super bonus that the current territory is contained in
                    object_id_type super_id = group_bonus(bonus, m_super_bonuses);
                    // Check if a super bonus was found
                    if (super_id > 0)
                    {
                        // Add territory to child list of the bonus
                        m_bonuses.at(super_id).children().push_back({ bonus.id() });
                    }
                }

            }
        };

        // /**
        //  * 
        //  */
        // class ConnectionCalculator
        // {

        //     /* Types */

        //     using component_map_type = std::unordered_map<component_type, std::vector<object_id_type>>;

        //     /* Members */

        //     std::vector<map::Territory>& m_territories;
        //     component_map_type& m_components;

        //     // Caches
        //     std::unordered_map<component_type, geometry::Rectangle<double>> m_bounds_cache;
        //     std::unordered_map<component_type, geometry::Ring<double>> m_hull_cache;

        // public:

        //     ConnectionCalculator(
        //         std::vector<map::Territory>& territories,
        //         component_map_type& components
        //     ) : m_territories(territories), m_components(components) {}

        // protected:

        //     struct PointHasher
        //     {
        //         size_t operator()(const geometry::Point<double>& point) const
        //         {
        //             size_t xHash = std::hash<int>()(point.x());
        //             size_t yHash = std::hash<int>()(point.y()) << 1;
        //             return xHash ^ yHash;
        //         }
        //     };

        //     void calculate_bounds()
        //     {
        //         for (const auto[component, territories] : m_components)
        //         {
        //             double min_x = DBL_MAX;
        //             double min_y = DBL_MAX;
        //             double max_x = -DBL_MAX;
        //             double max_y = -DBL_MAX;
        //             for (const object_id_type& t : territories)
        //             {
        //                 const map::Territory& territory = m_territories.at(t);
        //                 geometry::Rectangle<double> envelope = functions::envelope(territory.geometry());
        //                 min_x = std::min(min_x, envelope.min().x());
        //                 min_y = std::min(min_y, envelope.min().y());
        //                 max_x = std::min(max_x, envelope.max().x());
        //                 max_y = std::min(max_y, envelope.max().y());
        //             }
        //             m_bounds_cache[component] = geometry::Rectangle<double>{ min_x, min_y, max_x, max_y };
        //         }
        //     }

        //     void calculate_hulls()
        //     {
        //         for (const auto[component, territories] : m_components)
        //         {
        //             // Collect the outer points of all territories
        //             std::unordered_set<geometry::Point<double>, PointHasher> unique_points{};
        //             for (const object_id_type& t : territories)
        //             {
        //                 const map::Territory& territory = m_territories.at(t);
        //                 for (const geometry::Point<double>& point : territory.geometry().outer())
        //                 {
        //                     unique_points.insert(point);
        //                 }
        //             }
        //             // Calculate the convex hull and save it
        //             std::vector<geometry::Point<double>> points;
        //             points.insert(points.end(), unique_points.begin(), unique_points.end());
        //             m_hull_cache[component] = functions::convex_hull(points);
        //         }
        //     }

        //     /**
        //      * 
        //      */
        //     geometry::Segment<double> find_nearest_connection(
        //         const geometry::Ring<double> hull1,
        //         const geometry::Ring<double> hull2
        //     ) {
        //         std::pair<geometry::Point<double>,geometry::Point<double>> pair;
        //         double d_min = DBL_MAX;
        //         for (auto it1 = hull1.begin(); it1 != hull1.end(); it1++)
        //         {
        //             for (auto it2 = hull2.begin(); it2 != hull2.end(); it2++)
        //             {
        //                 double d = functions::distance(*it1, *it2);
        //                 if (d < d_min)
        //                 {
        //                     d_min = d;
        //                     pair.first = *it1;
        //                     pair.second = *it2; 
        //                 }
        //             }
        //         }
        //         return geometry::Segment<double> { pair.first, pair.second };
        //     }

        //     map::Territory& get_territory_for_point(
        //         const component_type& component1,
        //         const geometry::Point<double>& point
        //     ) {
        //         std::vector<object_id_type> territories = m_components.at(component1);
        //         for (const object_id_type t : territories)
        //         {
        //             map::Territory& territory = m_territories.at(t);
        //             for (const geometry::Point<double>& p : territory.geometry().outer())
        //             {
        //                 if (point == p)
        //                 {
        //                     return territory;
        //                 }
        //             }
        //         }
        //         // Should not happen
        //         throw std::invalid_argument("No territory found");
        //     }

        //     void merge(const component_type& c1, const component_type& c2)
        //     {
        //         if (m_components.size() == 2)
        //         {
        //             // Only one component left after merge, no need to perform other calculations
        //             m_components.erase(c2);
        //             return;
        //         }

        //         // Calculate new bounding box


        //     }

        // public:

        //     std::vector<geometry::Segment<double>> calculate_connections()
        //     {
        //         if (m_components.size() < 2)
        //         {
        //             // No connecting needs to be performed
        //             return {};
        //         }

        //         // Prepare the resulting connections vector
        //         std::vector<geometry::Segment<double>> connections;

        //         // Pre-Calculate the bounding boxes and convex hulls of each component
        //         calculate_bounds();
        //         calculate_hulls();

        //         // Merge the components until only one is left
        //         while (m_components.size() > 1)
        //         {
        //             // Find the nearest component for each component using their bounding
        //             // boxes
        //             for (auto it1 = m_components.begin(); it1 != m_components.end(); it1++)
        //             {
        //                 component_type component1 = it1->first;
        //                 const geometry::Rectangle<double>& bounds1 = m_bounds_cache.at(it1->first);
        //                 // Find nearest other component by using the bounding boxes
        //                 double d_min = DBL_MAX;
        //                 component_type nearest_component = -1;
        //                 for (auto it2 = it1; it2 != m_components.end(); it2++)
        //                 {
        //                     component_type component2 = it2->first;
        //                     const geometry::Rectangle<double>& bounds2 = m_bounds_cache.at(it2->first);
        //                     double d = functions::distance(bounds1, bounds2);
        //                     if (d < d_min)
        //                     {
        //                         d_min = d;
        //                         nearest_component = component2;
        //                         if (d_min == 0)
        //                         {
        //                             // Component bounds intersect, skip the rest
        //                             break;
        //                         }
        //                     }
        //                 }
        //                 // Find nearest points of the convex hulls
        //                 const geometry::Ring<double>& hull1 = m_hull_cache.at(component1);
        //                 const geometry::Ring<double>& hull2 = m_hull_cache.at(nearest_component);
        //                 geometry::Segment<double> connection = find_nearest_connection(hull1, hull2);
        //                 // Add the connection to the result list
        //                 connections.push_back(connection);
        //                 // Find the territories the points belong to and add a neighbor relation
        //                 map::Territory& t1 = get_territory_for_point(component1, connection.first());
        //                 map::Territory& t2 = get_territory_for_point(nearest_component, connection.last());
        //                 t1.neighbors().push_back({ t2.id() });
        //                 t2.neighbors().push_back({ t1.id() });
        //                 // Merge the current two components, their bounding boxes and convex hulls
        //                 merge(component1, nearest_component);
        //             }
        //         }
        //         return connections;
        //     };

        // };

    }

}