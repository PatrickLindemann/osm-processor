#ifndef GEOMETRY_ALGORITHM_HPP
#define GEOMETRY_ALGORITHM_HPP

#include <cfloat>
#include <initializer_list>
#include <mutex>
#include <vector>

#include "geometry/model.hpp"
#include "mapmaker/model.hpp"

namespace geometry
{

    namespace algorithm
    {

        using namespace model;

        /**
         * Calculae the dot product of two points
         * 
         * @param p The first point
         * @param q The second point
         * @return The dot product of p and q
         */
        template <typename T>
        inline T dot(
            const Point<T>& p,
            const Point<T>& q
        ) {
            return p.x * q.x + p.y * q.y;
        }

        /**
         * Calculate the distance between two points
         * 
         * @param p The first point
         * @param q The second point
         * @return The absolute distance between p and q
         */
        template <typename T>
        inline double_t dist(
            const Point<T>& p,
            const Point<T>& q
        ) {
            return std::hypot(p.x - q.x, p.y - q.y);
        }

        /**
         * Calculate the perpendicular distance of a point to a segment
         * 
         * @param point The point as location
         * @param line_start The line starting point as location
         * @param line_end The line ending point as location
         * @return The perpendicular distance of the point to the segment
         */
        template <typename T>
        inline double_t perpendicular(
            const Point<T>& point,
            const Point<T>& line_start,
            const Point<T>& line_end
        ) {
            // Calculate the direction vector of the line
            Point dir = line_end - line_start;

            // Normalize the direction vector
            double length = std::hypot(dir.x, dir.y);
            if (length > 0.0)
                dir /= length;

            // Calculate the point-to-line vector
            Point ps = point - line_start;

            // Calculate the dot product between the two points to retrieve
            // the length between line_start and the plumb point L relative to p
            double dt = dot(dir, ps);

            // Calculate the point of plumb on the line relative to p
            // by scaling the line direction vector
            Point line_plumb = dir * dt;

            // Calculate the vector from the plumb point and p and return its distance
            return dist(ps, line_plumb);
        }

        /**
         * Calculate the perpendicular distance of a point to a segment
         * 
         * @param point The point as location
         * @param segment The line segment
         * @return The perpendicular distance of the point to the segment
         */
        template <typename T>
        inline double_t perpendicular(
            const Point<T>& point,
            const Segment<T>& segment
        )
        {
            return perpendicular(point, segment[0], segment[1]);
        }

        /**
         * Calculate the centroid of a point list by calculating the
         * weigted sum of all nodes.
         * 
         * FIXME: This algorithm does not return "ideal" centerpoints
         * and should be refined
         * 
         * @param points The list of points
         * @return The centroid as Point
         */
        template <typename T>
        inline Point<T> center(const std::vector<Point<T>>& points)
        {
            Point center;
            for (const Point<T>& p : points)
                center += p;
            center /= points.size();
            return center;
        }

        /**
         * Calculates the minimal bounding box of a list of points.
         * 
         * @param points The list of points
         * @return The minimal bounding box
         */
        template <typename T>
        inline Rectangle<T> bounds(const std::vector<Point<T>>& points)
        {   
            std::numeric_limits<T> limits;
            T min_x = limits.max(), min_y = limits.max() ;
            T max_x = limits.lowest(), max_y = limits.lowest();
            for (const Point<T>& p : points)
            {
                min_x = std::min(min_x, p.x);
                min_y = std::min(min_y, p.y);
                max_x = std::max(max_x, p.x);
                max_y = std::max(max_y, p.y);
            }
            return Rectangle<T>{ min_x, min_y, max_x, max_y };
        }

        /**
         * Calculates the minimal bounding box of a Polygon.
         * 
         * @param polygon The Polygon
         * @return The minimal bounding box
         */
        template <typename T>
        inline Rectangle<T> bounds(const Polygon<T>& polygon)
        {   
            return bounds(polygon.outer);
        }

        /**
         * Calculates the minimal bounding box of a MultiPolygon.
         * 
         * @param multipolygon The MultiPolygon
         * @return The minimal bounding box
         */
        template <typename T>
        inline Rectangle<T> bounds(const MultiPolygon<T>& multipolygon)
        {   
            std::numeric_limits<T> limits;
            Rectangle<T> result = { limits.max(), limits.max(), limits.lowest(), limits.lowest() };
            for (const Polygon<T>& polygon : multipolygon.polygons)
            {
                result.extend(bounds(polygon.outer));
            }
            return result;
        }

        /**
         * Compresses a list of points with the Douglas-Peucker-Algorithm.
         * https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
         *
         * @param nodes The list of node references
         */
        template <typename T>
        inline void compress(
            std::vector<Point<T>>& out,
            const std::vector<Point<T>>& points,
            const double_t epsilon
        ) {
            using point_list = std::vector<Point<T>>;

            if (points.size() < 2)
                return; // TODO: Throw error
            
            //  Start with the first node in the list
            double_t d_max = 0;
            size_t index = 0;
            size_t end = points.size() - 1; 

            // Find the node with the greatest distance
            for (size_t i = 1; i < end; i++)
            {
                double d = perpendicular(points[i], points[0], points[end]);
                if (d > d_max)
                {
                    index = i;
                    d_max = d;
                }
            }

            // Check if the maximum distance is greater than the upper threshold
            if (d_max > epsilon)
            {   
                // Create the result lists
                point_list left_results, right_results;
                // Create the new lines
                point_list left_line{ points.begin(), points.begin() + index + 1 };
                point_list right_line{ points.begin() + index, points.end() };
                // Compress the lists recursively
                compress(left_results, left_line, epsilon);
                compress(right_results, right_line, epsilon);
                // Build the result list
                out.assign(left_results.begin(), left_results.end() - 1);
                out.insert(out.end(), right_results.begin(), right_results.end());
                if (out.size() < 2)
                    return; // TODO: throw error
            }
            else
            {
                // Distance less than upper threshold, return the first and last point
                out = { points[0], points[end] };
            }
        }

    }

}

#endif