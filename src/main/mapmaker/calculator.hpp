#pragma once

#include <cmath>
#include <vector>

#include "functions/area.hpp"
#include "functions/center.hpp"
#include "functions/envelope.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/map/bonus.hpp"
#include "model/map/territory.hpp"
#include "model/memory/area.hpp"
#include "model/memory/buffer.hpp"
#include "model/memory/node.hpp"
#include "model/memory/ring.hpp"

using namespace model;

namespace mapmaker
{

    namespace calculator
    {

        /**
         * A calculator for territory centerpoints.
         */
        class CenterCalculator
        {
        
            /* Members */

            std::vector<map::Territory>& m_territories;

        public:

            /* Constructors */

            CenterCalculator(
                std::vector<map::Territory>& territories
            ) : m_territories(territories) {}

        protected:

            /* Constants */

            const double PRECISION_MIN = std::pow(10, -4);
            const double PRECISION_MIN_REVERSE = 1 / PRECISION_MIN;

            /* Helper functions */

            /**
             * Calculate the precision for centerpoint calculations.
             * This is done by applying a logistic function to the
             * relative size of a polygon to the total bounds size.
             * 
             * For an interactive version of this function, refer to
             * https://www.desmos.com/calculator/nfef7j6n3y
             * 
             * @param area  The total area  
             * @param total The polygon area
             * @param k     The growth constant of the logistic function
             * @returns     The precision
             * 
             * Complexity: Constant
             */
            double get_precision(double area, double total, double k = 25)
            {
                return area / ( 1 + std::exp(-k * area / total) * (PRECISION_MIN_REVERSE - 1)) + 1;
            }

        public:

            /* Methods */

            /**
             * Calculate the centerpoints for each territory with a relative
             * precision to the map bounds.
             * 
             * @param map_bounds The bounding box of the map
             * 
             * Time complexity: TODO
             */
            void calculate_centerpoints(const geometry::Rectangle<double>& map_bounds)
            {
                // Calculate the area of the map bounding box
                double total_area = functions::area(map_bounds);

                // Calculate the center point for each territory
                for (map::Territory& territory : m_territories)
                {
                    // Calculate the surface area of the current territory
                    double area = functions::area(territory.geometry());

                    // Get the precision for the centerpoint calculation
                    // according to the relative size of the territory
                    // area to the total map area
                    double precision = get_precision(area, total_area);

                    // Calculate the centerpoint and save it directly in
                    // the territory
                    auto [center, distance] = functions::center(territory.geometry());
                    territory.center() = center;
                }

            }

        };

    }

}