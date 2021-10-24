#pragma once

#include "functions/envelope.hpp"
#include "functions/intersect.hpp"
#include "model/geometry/multipolygon.hpp"
#include "model/geometry/polygon.hpp"
#include "model/geometry/ring.hpp"
#include "model/geometry/segment.hpp"
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

        /**
         * 
         */
        class ConnectionCalculator
        {

            /* Members */

            std::vector<map::Territory>& m_territories;

        public:

            ConnectionCalculator(std::vector<map::Territory>& territories) : m_territories(territories) {}

        protected:

        public:

            std::vector<geometry::Segment<double>> calculate_connections()
            {
                std::vector<geometry::Segment<double>> segments;
                for (const map::Territory& territory : m_territories)
                {
                    
                }
                return segments;
            };

        };


    }

}