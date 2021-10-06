#pragma once

#include <set>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <exception>

namespace model
{

    namespace graph
    {

        /**
         * A directed graph structure with vertices and edges.
         */
        class Graph
        {
        public:

            /* Types */

            /**
             * Vertices are defined by their ids. It is assumed that ids are
             * successive integers.
             */
            using vertex_type = unsigned long;
            
            /**
             * Edges are defined as pairs of vertices (without a cost metric).
             */
            using edge_type = std::pair<vertex_type, vertex_type>;            

            /**
             * The edge container, which is an ordered set of vertex pairs.
             * The ordering is first by source, then by target.
             */
            using edge_container        = std::set<edge_type>;
            using edge_iterator         = typename edge_container::iterator;
            using edge_const_iterator   = typename edge_container::const_iterator;

        protected:
            
            /* Members */

            size_t m_vertices = 0;

            edge_container m_edges = {};

            /* Constructors */

            Graph() {}; /* Cannot be instantiated */

        public:

            /* Virtual methods */

            /**
             * Returns the size of the graph, which is a pair of the number
             * of vertices and number of edges. 
             */
            virtual std::pair<size_t, size_t> size() const
            {
                return std::make_pair(m_vertices, m_edges.size());
            }

            /**
             * Checks if the graph is empty, meaning that it contains
             * no vertices or edges.
             */
            virtual bool empty() const
            {
                return m_vertices == 0 && m_edges.empty();
            }

            /* Vertex methods */

            /**
             * Retrieves the vertex count in the graph.
             */
            virtual size_t vertex_count() const
            {
                return m_vertices;
            }

            /**
             * Checks if a vertex exists in the graph.
             * 
             * @param edge The vertex.
             * @returns true If the vertex exists
             * 
             * Time complexity: Constant
             */
            virtual bool contains_vertex(const vertex_type& vertex) const
            {
                return vertex < m_vertices;
            }

            /* Edge methods */

            /**
             * Retrieves the edge count in the graph.
             */
            virtual size_t edge_count() const
            {
                return m_edges.size();
            }

            /**
             * Inserts an edge into the graph. 
             * 
             * @param edge The edge as <vertex, vertex> pair.
             */
            virtual void insert_edge(edge_type edge) = 0;

            /**
             * Checks if an edge exists in the graph.
             * 
             * @param edge The edge as <vertex, vertex> pair.
             * @returns true If the edge exists
             * 
             * Time complexity: Logarithmic
             */
            virtual bool contains_edge(const edge_type& edge) const = 0;

            /* Other methods */

            /**
             * Returns the degree (number of outgoing edges) for a specified
             * vertex in the graph.
             * 
             * @param Vertex The vertex.
             * @throws std::out_of_range If the vertex does not exist.
             */
            virtual size_t degree(const vertex_type& vertex) const = 0;
        	
            /**
             * Retrieves the adjacending vertices for a specified vertex
             * in the graph.
             * 
             * @param Vertex The vertex.
             * @throws std::out_of_range If the vertex does not exist.
             */
            virtual const std::vector<vertex_type> adjacents(const vertex_type& vertex) const = 0;

        };

    }

}