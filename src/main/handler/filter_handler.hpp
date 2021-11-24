#pragma once

#include <map>
#include <set>

#include <osmium/handler.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/types.hpp>

namespace handler
{

    class FilterReferenceHandler : public osmium::handler::Handler
    {
    protected:

        /* Members */

        std::set<osmium::object_id_type> m_ids;

        std::set<osmium::object_id_type> m_references;

    public:

        /* Constructors */

        FilterReferenceHandler(const std::set<osmium::object_id_type> ids) : m_ids(ids) {}

        /* Accessors */

        const std::set<osmium::object_id_type>& references() const
        {
            return m_references;
        };

    };

    class AreaNodeFilterHandler : public FilterReferenceHandler
    {
    public:

        /* Constructors */

        using FilterReferenceHandler::FilterReferenceHandler;

        /* Osmium Methods */

        void area(const osmium::Area& area) noexcept
        {
            if (m_ids.count(area.id()))
            {
                for (const osmium::OuterRing& outer : area.outer_rings())
                {
                    for (const osmium::NodeRef& nr : outer)
                    {
                        m_references.insert(nr.ref());
                    }
                    for (const osmium::InnerRing& inner : area.inner_rings(outer))
                    {
                        for (const osmium::NodeRef& nr : outer)
                        {
                            m_references.insert(nr.ref());
                        }
                    }
                }
            }
        }

    };

    class NodeWayFilterHandler : public FilterReferenceHandler
    {
    public:

        /* Constructors */

        using FilterReferenceHandler::FilterReferenceHandler;

        /* Osmium Methods */

        void way(const osmium::Way& way) noexcept
        {
            for (const osmium::NodeRef& nr : way.nodes())
            {
                if (m_ids.count(nr.ref()))
                {
                    m_references.insert(way.id());
                }
            }
        }

    };

}