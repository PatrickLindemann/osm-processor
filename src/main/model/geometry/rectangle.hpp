#pragma once

#include "model/geometry/point.hpp"

namespace model
{    

    namespace geometry
    {
    
        /**
         * 
         */
        template <typename T>
        class Rectangle
        {
        public:

            /* Types */

            using point_type = Point<T>;

            /* Members */
            
            point_type min, max;

            /* Constructors */
        
            Rectangle(point_type min, point_type max) : min(min), max(max) {};

            /* Accessors */

            const point_type bottom_left() const
            {
                return point_type{ this->min };
            }

            const point_type top_left() const
            {
                return point_type{ this->min.x, this->max.y };
            }

            const point_type top_right() const
            {
                return point_type{ this->max };
            }

            const point_type bottom_right() const
            {
                return point_type{ this->max.x, this->min.y };
            }

            /* Misc */

            const bool valid() const
            {
                return this->min.x <= this->max.x && this->min.y <= this->max.y; 
            }
    
            const double width() const
            {
                return this->max.x -  this->min.x;
            }

            const double height() const
            {
                return this->max.y - this->min.y;
            }

            Rectangle<T>& extend(const point_type& p) {
                if (p.x < this->min.x) this->min.x = p.x;
                if (p.y < this->min.y) this->min.y = p.y;
                if (p.x > this->max.x) this->max.x = p.x;
                if (p.y > this->max.y) this->max.y = p.y;
                return *this;
            }

            Rectangle<T>& extend(const Rectangle<T>& other) {
                if (other.min.x < this->min.x) this->min.x = other.min.x;
                if (other.min.y < this->min.y) this->min.y = other.min.y;
                if (other.max.x > this->max.x) this->max.x = other.max.x;
                if (other.max.y > this->max.y) this->max.y = other.max.y;
                return *this;
            }

        };

    }

}