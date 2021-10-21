#pragma once

#include <string>

#include "model/geometry/multipolygon.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/map/boundary.hpp"
#include "model/type.hpp"

namespace model
{

    namespace map
    {
       
        using namespace model::geometry;

        class BonusRef : public BoundaryRef {};

        /**
         * 
         */
        class Bonus : public Boundary
        {

            /* Members */
            
            /**
             * 
             */
            std::string m_name;
            
            /**
             * 
             */
            army_type m_armies;
            
            /**
             * 
             */
            std::string m_color;

            /**
             * 
             */
            std::vector<BoundaryRef> m_children;

            /**
             * 
             */
            MultiPolygon<double> m_geometry;

            /**
             * 
             */
            Rectangle<double> m_bounds;

            /**
             * 
             */
            Rectangle<double> m_bonus_link;

        public:

            /* Constructors */

            Bonus(object_id_type id) : Boundary(id) {};

            /* Accessors */

            std::string& name()
            {
                return m_name;
            }

            const std::string& name() const
            {
                return m_name;
            }

            army_type& armies()
            {
                return m_armies;
            }

            const army_type& armies() const
            {
                return m_armies;
            }

            std::string& color()
            {
                return m_color;
            }

            const std::string& color() const
            {
                return m_color;
            }

            std::vector<BoundaryRef>& children()
            {
                return m_children;
            };

            const std::vector<BoundaryRef>& children() const
            {
                return m_children;
            };

            MultiPolygon<double>& geometry()
            {
                return m_geometry;
            }

            const MultiPolygon<double>& geometry() const
            {
                return m_geometry;
            }

            Rectangle<double>& bounds()
            {
                return m_bounds;
            }

            const Rectangle<double>& bounds() const
            {
                return m_bounds;
            }
            
            Rectangle<double>& bonus_link()
            {
                return m_bonus_link;
            }

            const Rectangle<double>& bonus_link() const
            {
                return m_bonus_link;
            }

            /* Virtual methods */

            virtual bool is_super() const 
            {
                return false;
            }

        };

        /**
         * 
         */
        class SuperBonus : public Bonus
        {
        public:

            using Bonus::Bonus;

            bool is_super() const override
            {
                return true;
            }

        };

    }

}