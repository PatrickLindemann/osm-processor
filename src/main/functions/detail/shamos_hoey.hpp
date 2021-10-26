#pragma once

#include <functional>
#include <iterator>
#include <queue>
#include <set>

#include "model/geometry/point.hpp"
#include "model/geometry/segment.hpp"
#include "model/type.hpp"

using namespace model;

namespace functions
{

    namespace detail
    {

        /**
         * 
         */
        template <typename T>
        bool compare_lt(const geometry::Point<T>& p1, const geometry::Point<T>& p2)
        {
            // Test the x coordinate first
            if (p1.x() < p2.x()) return true;
            if (p1.x() > p2.x()) return false;
            // Test the y coordinate
            if (p1.y() < p2.y()) return true;
            if (p1.y() > p2.y()) return false;
            // Points are the same
            return false;
        }

        /**
         * 
         */
        template <typename T>
        bool compare_gt(const geometry::Point<T>& p1, const geometry::Point<T>& p2)
        {
            // Test the x coordinate first
            if (p1.x() > p2.x()) return true;
            if (p1.x() < p2.x()) return false;
            // Test the y coordinate
            if (p1.y() > p2.y()) return true;
            // if (p1.y() < p2.y()) return false;
            // Points are the same
            return false;
        }

        /**
         * 
         */
        template <typename T>
        float is_left(
            const geometry::Point<T>& p1,
            const geometry::Point<T>& p2,
            const geometry::Point<T>& p3
        ) {
            return (p2.x() - p1.x()) * (p3.y() - p1.y()) - (p3.x() - p1.x()) * (p2.x() - p1.y());
        }

        /* Events */

        /**
         * 
         */
        enum Type
        {
            LEFT,
            RIGHT
        };

        /**
         * 
         */
        template <typename T>
        struct Event
        {
            size_t edge;
            geometry::Point<T> point;
            Type type;
        };

        template <typename T>
        class EventGreaterComparator
        {
        public:

            bool operator() (const Event<T>& e1, const Event<T>& e2) const
            {
                return compare_gt(e1.point, e2.point);
            }

        };

        /**
         * 
         */
        template <typename T>
        class EventQueue : public std::priority_queue<Event<T>, std::vector<Event<T>>, EventGreaterComparator<T>>
        {  
        public:

            /* Constructors */
            
            EventQueue(std::vector<geometry::Segment<T>>& segments)
            {
                // Convert segments to events and add them to the internal queue                
                for (size_t i = 0; i < segments.size(); i++)
                {
                    // Retrieve the current segment and its points
                    geometry::Segment<T>& segment = segments.at(i);
                    geometry::Point<T>& p1 = segment.first();
                    geometry::Point<T>& p2 = segment.last();
                    // Create the events for each point
                    Event<T> e1{ i, p1 };
                    Event<T> e2{ i, p2 };
                    // Determine the event types
                    if (compare_lt(p1, p2))
                    {
                        e1.type = LEFT;
                        e2.type = RIGHT;
                    }
                    else
                    {
                        e1.type = RIGHT;
                        e2.type = LEFT;
                    }
                    // Add the events to the queue
                    this->push(e1);
                    this->push(e2);
                }
            }

        };

        /* Sweepline */

        /**
         * 
         */
        template <typename T>
        struct SLSegment
        {
            size_t edge;
            geometry::Point<T> left;
            geometry::Point<T> right;
            SLSegment<T>* above = nullptr;
            SLSegment<T>* below = nullptr;
        };

        /**
         * 
         */
        template <typename T>
        class SLSegmentLessComparator
        {
        public:

            bool operator() (const SLSegment<T>& s1, const SLSegment<T>& s2) const
            {
                return s1.edge < s2.edge;
            }
        };

        /**
         * 
         */
        template <typename T>
        class SweepLine
        {

            /* Types */

            /**
             * 
             */
            using tree_type = std::set<SLSegment<T>, SLSegmentLessComparator<T>>;

            /**
             * 
             */
            using iterator_type       = typename tree_type::iterator;
            using const_iterator_type = typename tree_type::const_iterator;

            /* Members */

            /**
             * 
             */
            const std::vector<geometry::Segment<T>>& m_segments;
            
            /**
             * 
             */
            tree_type m_tree;

        public:

            /* Constructors */

            SweepLine<T>(std::vector<geometry::Segment<T>>& segments)
            : m_segments(segments) {};

        protected:


            /**
             * Update the above pointer for an SLSegment in the tree at a
             * specified iterator position with a new pointer.
             * This is needed because set iterators are const, which means that
             * referenced values can not be manipulated directly
             * 
             * @param it    The iterator of the SLSegment
             * @param above The new above pointer
             *
             * Time complexity: Constant (Amortized), Logarithmic (Worst-Case) 
             */
            void update_above(iterator_type it, SLSegment<T>* above)
            {
                if (it != m_tree.end())
                {
                    // Update the above value
                    SLSegment<T> s = *it;
                    s.above = above;
                    // Erase and re-insert the segment
                    it = m_tree.erase(it);
                    m_tree.insert(it, s);
                }
            }

            /**
             * Update the below pointer for an SLSegment in the tree at a
             * specified iterator position with a new pointer.
             * This is needed because set iterators are const, which means that
             * referenced values can not be manipulated directly
             * 
             * @param it    The iterator of the SLSegment
             * @param below The new below pointer
             *
             * Time complexity: Constant (Amortized), Logarithmic (Worst-Case) 
             */
            void update_below(iterator_type it, SLSegment<T>* below)
            {
                if (it != m_tree.end())
                {
                    // Update the below value
                    SLSegment<T> s = *it;
                    s.below = below;
                    // Erase and re-insert the segment
                    it = m_tree.erase(it);
                    m_tree.insert(it, s);
                }
            }

        public:

            /* Methods */

            /**
             * 
             */
            inline iterator_type add(Event<T>& e)
            {
                // Fill the SLSegment data
                SLSegment<T> s{ e.edge };

                // If it is being added, then it must be a LEFT edge event
                // Determine which of the points is the left one
                const geometry::Segment<T>& segment = m_segments.at(e.edge); 
                const geometry::Point<T>& p1 = segment.first();
                const geometry::Point<T>& p2 = segment.last();
                if (compare_lt(p1, p2))
                {
                    // p1 is left of p2
                    s.left = p1;
                    s.right = p2;
                }
                else
                {
                    // p2 is left of p1
                    s.left = p2;
                    s.right = p1;
                }

                // Add the node to the tree
                auto [it, inserted] = m_tree.insert(s);

                // Determine the above and below segments
                if (std::next(it) != m_tree.end())
                {
                    iterator_type nx = std::next(it);
                    update_below(nx, &s);
                    SLSegment<T> above = *nx;
                    update_above(it, &above);
                }
                if (it != m_tree.begin())
                {
                    iterator_type pv = std::prev(it);
                    update_above(pv, &s);
                    SLSegment<T> below = *pv;
                    update_below(it, &below);
                }

                // Return the inserted segment
                return it;
            }

            /**
             * 
             */
            inline iterator_type find(Event<T>& e)
            {
                // Search the segment and return the result iterator
                return m_tree.find({ e.edge });
            }

            /**
             * 
             */
            inline void remove(SLSegment<T>& s)
            {
                // Find the segment in the tree
                iterator_type it = m_tree.find(s);
                if (it == m_tree.end())
                {
                    // Segment doesn't exist, no removal can be performed
                    return;
                }

                // Update the above and below segments pointing to each other
                if (std::next(it) != m_tree.end())
                {
                    iterator_type nx = std::next(it);
                    update_below(nx, s.below);
                }
                if (it != m_tree.begin())
                {
                    iterator_type pv = std::prev(it);
                    update_above(pv, s.above);
                }

                // Now, the segment can be safely removed from the tree
                m_tree.erase(it);
            }

            /**
             * 
             */
            inline bool intersect(SLSegment<T>& s1, SLSegment<T>& s2)
            {
                // Check if the edges are consecutive in the line
                const int e1 = s1.edge;
                const int e2 = s2.edge;
                if ((e1 + 1) % m_segments.size() == e2 
                 || (e2 + 1) % m_segments.size() == e1
                ) {
                    return false;
                }

                // Test for existence of an intersection point
                float l_sign = is_left(s1.left, s1.right, s2.left);
                float r_sign = is_left(s1.left, s1.right, s2.right);
                if (l_sign * r_sign > 0)
                {
                    // Endpoints of s1 have the same sign relative
                    // to s2
                    return false;
                }
                l_sign = is_left(s2.left, s2.right, s1.left);
                r_sign = is_left(s2.left, s2.right, s1.right);
                if (l_sign * r_sign > 0)
                {
                    // Endpoints of s2 have the same sign relative
                    // to s1
                    return false;
                }

                // Found intersection
                return true;
            }
        };

        /**
         * 
         */
        template <typename T>
        bool intersects_shamos_hoey(std::vector<geometry::Segment<T>>& segments)
        {
            SweepLine<T> sl{ segments };
            EventQueue<T> queue{ segments };
            
            // Process all events in the sorted event queue
            // Events are only left or right vertices since
            // no new events will be added
            while(!queue.empty())
            {
                // Retrieve the next event
                Event<T> e = queue.top();
                queue.pop();
                // Process the event
                if (e.type == LEFT)
                {
                    // Process a left vertex
                    SLSegment<T> s = *sl.add(e);
                    if (s.above != nullptr && sl.intersect(s, *s.above))
                    {
                        return true;
                    }
                    else if (s.below != nullptr && sl.intersect(s, *s.below))
                    {
                        return true;
                    }
                }
                else if (e.type == RIGHT)
                {
                    // Process a right vertex
                    SLSegment<T> s = *sl.find(e);
                    if (s.above != nullptr && s.below != nullptr)
                    {
                        if (sl.intersect(*s.above, *s.below))
                        {
                            return true;
                        }
                    }
                    sl.remove(s);
                }
            }

            // No intersection was found
            return false;
        }

    }

}