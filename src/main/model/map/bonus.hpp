#pragma once

#include <string>

#include "model/geometry/multipolygon.hpp"
#include "model/geometry/rectangle.hpp"
#include "model/map/entity.hpp"
#include "model/map/type.hpp"

namespace model
{

    namespace map
    {
       
        using namespace model::geometry;

        class BonusRef : public EntityRef
        {
        public:

            /* Constructors */

            BonusRef(id_type ref) : EntityRef(ref) {};

        };

        /**
         * 
         */
        class Bonus : public Entity
        {

            /* Members */
            
            /**
             * 
             */
            std::string m_name;
            
            /**
             * 
             */
            army_count_type m_armies;
            
            /**
             * 
             */
            std::string m_color;

            /**
             * 
             */
            std::vector<EntityRef> m_children;

            /**
             * 
             */
            MultiPolygon<double> m_geometry;

            /**
             * 
             */
            Rectangle<double> m_bonus_link;

        public:

            /* Constructors */

            Bonus(id_type id) : Entity(id) {};

            /* Accessors */

            std::string& name()
            {
                return m_name;
            }

            const std::string& name() const
            {
                return m_name;
            }

            army_count_type& armies()
            {
                return m_armies;
            }

            const army_count_type& armies() const
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

            std::vector<EntityRef>& children()
            {
                return m_children;
            };

            const std::vector<EntityRef>& children() const
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

            Rectangle<double>& bonus_link()
            {
                return m_bonus_link;
            }

            const Rectangle<double>& bonus_link() const
            {
                return m_bonus_link;
            }

            /* Virtual methods */

            virtual bool is_regular() const = 0;

            virtual bool is_super() const = 0;

        };

        class RegularBonus : public Bonus
        {
        public:

            /* Methods */

            bool is_super() const override
            {
                return false;
            }

            bool is_regular() const override
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

            /* Methods */

            bool is_super() const override
            {
                return false;
            }

            bool is_regular() const override
            {
                return false;
            }

        };

    }

}