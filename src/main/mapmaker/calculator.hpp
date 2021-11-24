#pragma once

#include "model/geometry/point.hpp"
#include "model/geometry/rectangle.hpp"

#include "handler/bounds_handler.hpp"

#include "functions/center.hpp"

using namespace model;

namespace mapmaker
{

    template <typename T>
    class BoundsCalculator
    {
    public:

        /* Constructors */

        BoundsCalculator() {}

        /* Methods */

        geometry::Rectangle<T> run(const osmium::memory::Buffer& buffer) const
        {
            // Prepare the bounds handler that calculates the minimum bounding
            // box over all nodes in the buffer
            handler::BoundsHandler bounds_handler{};
            osmium::apply(buffer, bounds_handler);
            // Retrieve the calculated bounds and convert them to a rectangle
            // geometry
            osmium::Box bounds = bounds_handler.bounds();
            return geometry::Rectangle<T>{
                T(bounds.bottom_left().lon()),
                T(bounds.bottom_left().lat()),
                T(bounds.top_right().lon()),
                T(bounds.top_right().lat())
            };
        }

    };

    template <typename T>
    class CenterCalculator
    {
    public:

        /* Constructors */

        CenterCalculator() {}

        /* Methods */

        void run(std::map<object_id_type, Boundary<T>>& boundaries)
        {
            for (auto& [id, boundary] : boundaries)
            {
                boundary.center = functions::center(boundary.geometry);
            }
        }

    };

    //class ArmyCalculator
    //{
    //protected:

    //    /* Members */

    //    /**
    //     * The minimum number of armies for a bonus.
    //     */
    //    int m_min = 1;

    //    /**
    //     * The maximum number of armies for a bonus.
    //     */
    //    int m_max = 10;

    //    /**
    //     * The territory count weight.
    //     */
    //    double m_tw = 0.5;

    //    /**
    //     * The outer connection count weight.
    //     */
    //    double m_ow = 0.5;

    //public:

    //    /* Constructors */

    //    ArmyCalculator() {}
    //    ArmyCalculator(int min, int max) : m_min(min), m_max(max) {}
    //    ArmyCalculator(int min, int max, std::vector<double> weights)
    //    : m_min(min), m_max(max), m_tw(weights.at(0)), m_ow(weights.at(1)) {}

    //protected:

    //    /* Helper Methods */

    //    std::set<object_id_type> outer_connections(const warzone::Map& map, const warzone::Bonus& bonus)
    //    {
    //        std::set<object_id_type> connections = {};

    //        // Retrieve the neighbors of all territories contained in the bonus
    //        for (const object_id_type& child : bonus.children)
    //        {
    //            const warzone::Territory& territory = map.territories.at(child);
    //            for (const object_id_type& neighbor : territory.neighbors)
    //            {
    //                connections.insert(neighbor);
    //            }
    //        }

    //        // Filter the neighbors that are territories in the bonus
    //        for (const object_id_type& child : bonus.children)
    //        {
    //            connections.erase(child);
    //        }

    //        return connections;
    //    }

    //public:

    //    /* Methods */

    //    void run(warzone::Map& map)
    //    {
    //        // Retrieve the total number of territories
    //        std::size_t t_total = map.territories.size();

    //        // Calculate the number of each outer connections for each bonus
    //        std::size_t o_total = 0;
    //        std::vector<std::set<object_id_type>> bonus_connections;
    //        for (warzone::Bonus& bonus : map.bonuses)
    //        {
    //            bonus_connections.push_back(outer_connections(map, bonus));
    //        }



    //        m_tw* (t / t_total) + m_ow * (o / o_total);
    //    }

    //};

}