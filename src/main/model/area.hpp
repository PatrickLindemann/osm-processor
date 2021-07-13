#ifndef MAPMAKER_AREA_HPP
#define MAPMAKER_AREA_HPP

#include <cstdint>

#include "model/box.hpp"
#include "model/point.hpp"
#include "model/multipolygon.hpp"

namespace mapmaker
{
    
    class Area
    {
        int64_t m_id;
        std::string m_name;
        std::string m_boundary;
        int32_t m_level;
        std::string m_source;
        std::string m_wikidata;
        MultiPolygon m_geometry;

    public:

        Area(
            int64_t id,
            std::string name,
            std::string boundary,
            MultiPolygon& geometry,
            std::string source = "",
            std::string wikidata = ""
        )
        : Area(id, name, boundary, -1, geometry, source, wikidata) {};

        Area(
            int64_t id,
            std::string name,
            std::string boundary,
            int32_t level,
            MultiPolygon& geometry,
            std::string source = "",
            std::string wikidata = ""
        ) : m_id(id), m_name(name), m_boundary(boundary), m_level(level), m_geometry(geometry), m_source(source), m_wikidata(wikidata) {};

        long id()
        {
            return m_id;
        }

        const std::string name()
        {
            return m_name;
        }

        const std::string boundary()
        {
            return m_boundary;
        }

        const int32_t level()
        {
            return m_level;
        }

        const std::string source()
        {
            return m_source;
        }

        const std::string wikidata()
        {
            return m_wikidata;
        }
        
        const MultiPolygon geometry() const
        {
            return m_geometry;
        }

        const Point center() const
        {
            return m_geometry.center();
        }

    };

}

#endif