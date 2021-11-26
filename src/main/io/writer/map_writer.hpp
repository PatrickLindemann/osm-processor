#pragma once

#include <nlohmann/json.hpp>

#include "io/writer/writer.hpp"
#include "model/warzone/map.hpp"

namespace io
{

    using namespace model;

    template <typename T>
    class MapWriter : public Writer<warzone::Map<T>>
    {
    protected:

        /* Constants */

        const double BONUS_LINK_SIZE = 20.0;
        const double BONUS_LINK_ROUNDING = 3.0;

        //  const double SUPER_BONUS_LINK_SIZE = 30.0;
        // const double SUPER_BONUS_LINK_SIDE_LENGTH = 40.0;

    public:

        /* Constructors */

        MapWriter(fs::path file_path) : Writer<warzone::Map<T>>(file_path) {}
        
    protected:

        /* Helper Methods */

        template <typename StreamType>
        void write_geometry(StreamType& stream, const geometry::Polygon<T>& geometry)
        {
            // Add outer points (counter-clockwise)
            stream << "M ";
            for (auto it = geometry.outer().begin(); it != geometry.outer().end(); ++it)
            {
                if (it == geometry.outer().begin() + 1)
                {
                    stream << "L ";
                }
                stream << it->x() << " " << it->y() << " ";
            }
            stream << "Z";
            if (geometry.inners().size() > 0)
            {
                // Add inner points (clockwise)
                for (const geometry::Ring<T>& inner : geometry.inners())
                {
                    stream << " M ";
                    for (auto it = inner.rbegin(); it != inner.rend(); ++it)
                    {
                        if (it == inner.rbegin() + 1)
                        {
                            stream << "L ";
                        }
                        stream << it->x() << " " << it->y() << " ";
                    }
                    stream << "Z";
                }
            }
        }

        template <typename StreamType>
        void write_geometry(StreamType& stream, const geometry::MultiPolygon<T>& geometry)
        {
            for (auto it = geometry.polygons().begin(); it != geometry.polygons().end(); it++)
            {
                if (it != geometry.polygons().begin())
                {
                    stream << " ";
                }
                write_geometry(stream, *it);
            }
        }

    public:

        /* Override Methods */

        void write(warzone::Map<T>&& map) override
        {
            std::ofstream ofs{ this->m_path, std::ios::trunc };
            ofs.precision(4);

            // Write headers
            ofs << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            << "id=\"my-svg\" "
            << "width=\"" << map.width << "px\" "
            << "height=\"" << map.height << "px\""
            << ">";

            // Write the super bonuses
            for (const warzone::SuperBonus<T>& super_bonus : map.super_bonuses)
            {
                ofs << "<path "
                    << "name=\"" << super_bonus.name << "\" "
                    << "style=\"fill:none; stroke:black; stroke-width: 3px;\" "
                    << "d=\"";
                write_geometry(ofs, super_bonus.geometry);
                ofs << "\"/>"; // End path
            }

            // Write the bonuses
            for (const warzone::Bonus<T>& bonus : map.bonuses)
            {
                ofs << "<path "
                    << "name=\"" << bonus.name << "\" "
                    << "style=\"fill:none; stroke:black; stroke-width: 2px;\" "
                    << "d=\"";
                write_geometry(ofs, bonus.geometry);
                ofs << "\"/>"; // End path
            }

            // Write territories
            for (const warzone::Territory<T>& territory : map.territories)
            {
                ofs << "<path "
                    << "id=\"Territory_" << territory.id << "\" "
                    << "name=\"" << territory.name << "\" "
                    << "style=\"fill:none; stroke:black; stroke-width: 1px;\" "
                    << "d=\"";
                write_geometry(ofs, territory.geometry);
                ofs << "\"/>"; // End path
            }

            // Write centers
            for (const warzone::Territory<T>& territory : map.territories)
            {
                ofs << "<circle "
                    << "id=\"Center_" << territory.id << "\" "
                    << "cx=\"" << territory.center.x() << "\" "
                    << "cy=\"" << territory.center.y() << "\" "
                    << "r=\"2\" "
                    << "fill=\"black\""
                    << "/>";
            }

            // Write the bonus links
            for (const warzone::Bonus<T>& bonus : map.bonuses)
            {
                ofs << "<rect "
                    << "id=\"BonusLink_" << bonus.name << "\" "
                    << "x=\"" << bonus.center.x() - (BONUS_LINK_SIZE / 2) << "\" "
                    << "y=\"" << bonus.center.y() - (BONUS_LINK_SIZE / 2) << "\" "
                    << "width=\"" << BONUS_LINK_SIZE << "\" "
                    << "height=\"" << BONUS_LINK_SIZE << "\" "
                    << "rx=\"" << BONUS_LINK_ROUNDING << "\" "
                    << "ry=\"" << BONUS_LINK_ROUNDING << "\" "
                    << "style=\"fill: " << bonus.color << "; stroke: black;\" "
                    << "/>";
            }

            // Write the super bonus links
            //for (const warzone::SuperBonus<t>& super_bonus : map.super_bonuses)
            //{
            //    ofs << "<rect "
            //        << "id=\"bonuslink_" << super_bonuses.name << "\" "
            //        << "name=\"" << bonus.name << "\" "
            //        << "x=\"" << bonus.center.x() - (bonus_link_size / 2) << "\" "
            //        << "y=\"" << bonus.center.y() - (bonus_link_size / 2) << "\" "
            //        << "width=\"" << bonus_link_size << "\" "
            //        << "height=\"" << bonus_link_size << "\" "
            //        << "rx=\"" << bonus_link_rounding << "\" "
            //        << "ry=\"" << bonus_link_rounding << "\" "
            //        << "style=\"fill: " << bonus.color << "; stroke: black;\" "
            //        << "/>";
            //}

            ofs << "</svg>" << std::endl;
        }

    };

}