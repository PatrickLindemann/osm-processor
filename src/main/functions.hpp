#ifndef MAPMAPKER_FUNCTIONS_HPP
#define MAPMAPKER_FUNCTIONS_HPP

#include <cfloat>
#include <vector>

#include "model/box.hpp"
#include "model/point.hpp"

namespace mapmaker
{

namespace functions
{

inline double distance(
    const Point& p,
    const Point& q
) {
    return std::hypot(p.x - q.x, p.y - q.y);
}

inline double dot(
    const Point& p,
    const Point& q
) {
    return p.x * q.x + p.y * q.y;
}

/**
 * Calculate the perpendicular distance of a point to a finite line
 * 
 * @param point The point as location
 * @param line_start The line starting point as location
 * @param line_end The line ending point as location
 */
inline double perpendicular_distance(
    const Point& point,
    const Point& line_start,
    const Point& line_end
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
    return distance(ps, line_plumb);
}

/**
 * Compresses a list of nodes with the Douglas-Peucker-Algorithm.
 * https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
 *
 * @param nodes The list of node references
 */
inline void compress_nodes(
    std::vector<Point>& out,
    const std::vector<Point>& points,
    double_t epsilon
){
    if (points.size() < 2)
        // TODO: Throw error
        return;
    
    //  Start with the first node in the list
    double_t d_max = 0;
    size_t index = 0;
    size_t end = points.size() - 1; 

    // Find the node with the greatest distance
    for (size_t i = 1; i < end; i++)
    {
        double d = perpendicular_distance(points[i], points[0], points[end]);
        if (d > d_max)
        {
            index = i;
            d_max = d;
        }
    }

    // Check if the maximum distance is greater than epsilon
    if (d_max > epsilon)
    {   
        // Create the result lists
        PointList left_results, right_results;
        // Create the new lines
        PointList first_line(points.begin(), points.begin() + index + 1);
        PointList last_line(points.begin() + index, points.end());
        // Compress the ways recursively
        compress_nodes(left_results, first_line, epsilon);
        compress_nodes(right_results, last_line, epsilon);
        // Build the result list
        out.assign(left_results.begin(), left_results.end() - 1);
        out.insert(out.end(), right_results.begin(), right_results.end());
        if (out.size() < 2)
            // TODO: throw error
            return;
    }
    else
    {
        // Distance is OK, return the first and last point of the line
        out = { points[0], points[end] };
    }
}

/**
 * Calculate the centroid of a NodeRefList by calculating the
 * weigted sum of all nodes.
 * 
 * FIXME: This algorithm does not return "ideal" centerpoints
 * and should be refined
 * 
 * @param node_refs The list of node references
 * @return The centroid as osmium::Location
 */
inline Point calc_center(const PointList& points) {
    Point center{ 0.0, 0.0 };
    for (const Point& p : points)
        center += p;
    center /= points.size();
    return center;
}

// Calculate the bounding box of a NodeRefList.
inline Box calc_bounds(const PointList& points)
{   
    double min_x = DBL_MAX, min_y = DBL_MAX ;
    double max_x = -DBL_MAX, max_y = -DBL_MAX;
    for (const Point& p : points)
    {
        min_x = std::min(min_x, p.x);
        min_y = std::min(min_y, p.y);
        max_x = std::max(max_x, p.x);
        max_y = std::max(max_y, p.y);
    }
    return Box{ min_x, min_y, max_x, max_y };
}

}

}

#endif