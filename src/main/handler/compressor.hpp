#pragma once

#include <osmium/builder/osm_object_builder.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>

namespace handler
{

    class Comressor : public osmium::handler::Handler
    {

        class Compressor
        {

            /* Types */

            using id_map_type = std::unordered_map<object_id_type, object_id_type>;

            /* Members */

            Buffer<Node>& m_node_buffer;
            Buffer<Way>& m_way_buffer;
            
            /**
             *  The compression result vector of node indices that indicates
             *  which nodes should be kept or removed.
             *  If kept_indices[i] == true, the node with index i will be kept,
             *  else it will be removed.
             */
            std::vector<bool> m_removed_nodes;

            /**
             * The lookup vector for node degrees.
             * These nodes will be ignored by the compressor in order
             * to prevent different compressions of node segments
             */
            std::vector<size_t> m_degrees;

        public:

            /* Constructor */

            /**
             * Create a new compressor for a set of buffers and a specified
             * tolerance greater than zero.
             *
             * @param nodes     The node buffer
             * @param ways      The way buffer
             */
            Compressor(Buffer<Node>& nodes, Buffer<Way>& ways)
            : m_node_buffer(nodes), m_way_buffer(ways)
            {
                // Prepare node degree lookup list
                m_degrees = std::vector<size_t>(nodes.size(), 0);
                for (const Way& way : m_way_buffer)
                {
                    for (const NodeRef& node : way)
                    {
                        ++m_degrees.at(node.ref());
                    }
                }
            };

        protected:
            
            /**
             * Compresses a list of points with the Douglas-Peucker-Algorithm.
             * This method implements the iterative version of the algorithm,
             * as the recursive method initializes multiple new collections
             * that will be destroyed by the garbage collector anyway.
             * 
             * For more information on the original algorithm, refer to
             * https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
             *
             * @param nodes     The node list
             * @param tolerance The compression distance tolerance (epsilon)
             * 
             * Time complexity: Log-Linear (Average-case), Quadratic (Worst-case)
             */
            inline void douglas_peucker(
                const std::vector<Node>& nodes,
                double tolerance
            ) {
                // Create the index stack for the iterative version
                // of the algorithm
                std::stack<std::pair<int, int>> stack;
                stack.push(std::make_pair(0, nodes.size() - 1));

                while (!stack.empty())
                {
                    // Get the current start and end index
                    auto [start, end] = stack.top();
                    stack.pop();

                    // Find the node with the greatest perpendicular distance to
                    // the line between the current start and end node
                    int index = start;
                    double d_max = 0.0;
                    for (int i = start + 1; i < end; i++)
                    {
                        // Check if node was removed already in another
                        // iteration
                        if (!m_removed_nodes.at(nodes.at(i).id()))
                        {
                            double d = functions::perpendicular_distance(
                                nodes.at(i).point(),
                                nodes.at(start).point(), 
                                nodes.at(end).point()
                            );
                            if (d > d_max)
                            {
                                index = i;
                                d_max = d;
                            }
                        }
                    }

                    // Check if the maximum distance is greater than the upper tolerance
                    if (d_max > tolerance)
                    {
                        // Compress the left and right part of the polyline
                        stack.push(std::make_pair(start, index));
                        stack.push(std::make_pair(index, end));
                    }
                    else
                    {
                        // Remove all nodes from the current polyline that are between the
                        // start and end node, except nodes with degree > 2
                        for (int i = start + 1; i < end; i++) {
                            if (m_degrees.at(nodes.at(i).id()) < 3)
                            {
                                m_removed_nodes.at(nodes.at(i).id()) = true;
                            }
                        }
                    }
                }
            }

        public:

            /**
             * Run the compressor on the way and node buffers.
             * Nodes and ways that were removed by the compression will be
             * removed in the respective buffers.
             * 
             * For more information on finding a good tolerance value, refer
             * to https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm
             * 
             * @param tolerance The distance epsilon for the Douglas-Peucker-Algorithm.
             * 
             * Time complexity: 
             */
            void compress_ways(double tolerance)
            {
                // If the tolerance is less or equal to zero,
                // no compression will be applied.
                if (tolerance <= 0)
                {
                    return;
                }

                // Compress the ways according to the Douglas-Peucker-Algorithm
                m_removed_nodes = std::vector<bool>(m_node_buffer.size());
                for (const Way& way : m_way_buffer)
                {
                    // Retrieve the referenced way nodes from the buffer
                    std::vector<Node> nodes;
                    for (const NodeRef& node : way)
                    {
                        // Filter nodes that were removed already to avoid
                        // multiple compression iterations for the same line
                        // segements 
                        if (!m_removed_nodes.at(node.ref()))
                        {
                            nodes.push_back(m_node_buffer.at(node.ref()));
                        }
                    }
                    douglas_peucker(nodes, tolerance);
                }
                
                // Prepare the result buffers that contain the compressed,
                // reindexed nodes
                Buffer<Node> compressed_nodes;
                Buffer<Way> compressed_ways;

                // Prepare the index map that maps the current node indices to
                // compressed node indices
                id_map_type n_ids;

                // Create the new node buffer by adding all nodes from the old
                // buffer that have not been marked as removed
                for (const Node& node : m_node_buffer)
                {
                    if (!m_removed_nodes.at(node.id()))
                    {
                        object_id_type mapped_id = n_ids.size();
                        n_ids[node.id()] = mapped_id;
                        compressed_nodes.push_back(Node{ mapped_id, node.point() });
                    }
                }

                // Create the new way buffer by re-creating the ways with the nodes
                // that have not been marked as removed
                for (const Way& way : m_way_buffer)
                {
                    Way compressed_way{ way.id() };
                    for (const NodeRef& node : way)
                    {
                        if (!m_removed_nodes.at(node.ref()))
                        {
    
                            compressed_way.push_back(n_ids.at(node.ref()));
                        }
                    }
                    compressed_ways.push_back(compressed_way);
                }
                
                // Swap the old buffer and graph references with the result
                // buffers and graph
                std::swap(m_node_buffer, compressed_nodes);
                std::swap(m_way_buffer, compressed_ways);
            }
        
        osmium::memory::Buffer& m_buffer;


        template <typename T>
        void copy_attributes(T& builder, const osmium::OSMObject& object)
        {
            // The setter functions on the builder object all return the same
            // builder object so they can be chained.
            builder.set_id(object.id())
                .set_version(object.version())
                .set_changeset(object.changeset())
                .set_timestamp(object.timestamp())
                .set_uid(object.uid())
                .set_user(object.user());
        }

        static void copy_tags(osmium::builder::Builder& parent, const osmium::TagList& tags)
        {

            // The TagListBuilder is used to create a list of tags. The parameter
            // to create it is a reference to the builder of the object that
            // should have those tags.
            osmium::builder::TagListBuilder builder{parent};

            // Iterate over all tags and build new tags using the new builder
            // based on the old ones.
            for (const auto& tag : tags) {
                if (!std::strcmp(tag.key(), "created_by")) {
                    // ignore
                } else if (!std::strcmp(tag.key(), "landuse") && !std::strcmp(tag.value(), "forest")) {
                    // add_tag() can be called with key and value C strings
                    builder.add_tag("natural", "wood");
                } else {
                    // add_tag() can also be called with an osmium::Tag
                    builder.add_tag(tag);
                }
            }
        }

    public:

        // Constructor. New data will be added to the given buffer.
        explicit RewriteHandler(osmium::memory::Buffer& buffer) :
            m_buffer(buffer) {
        }

        // The node handler is called for each node in the input data.
        void node(const osmium::Node& node) {
            // Open a new scope, because the NodeBuilder we are creating has to
            // be destructed, before we can call commit() below.
            {
                // To create a node, we need a NodeBuilder object. It will create
                // the node in the given buffer.
                osmium::builder::NodeBuilder builder{m_buffer};

                // Copy common object attributes over to the new node.
                copy_attributes(builder, node);

                // Copy the location over to the new node.
                builder.set_location(node.location());

                // Copy (changed) tags.
                copy_tags(builder, node.tags());
            }

            // Once the object is written to the buffer completely, we have to call
            // commit().
            m_buffer.commit();
        }

        // The way handler is called for each way in the input data.
        void way(const osmium::Way& way) {
            {
                osmium::builder::WayBuilder builder{m_buffer};
                copy_attributes(builder, way);
                copy_tags(builder, way.tags());

                // Copy the node list over to the new way.
                builder.add_item(way.nodes());
            }
            m_buffer.commit();
        }

        // The relation handler is called for each relation in the input data.
        void relation(const osmium::Relation& relation) {
            {
                osmium::builder::RelationBuilder builder{m_buffer};
                copy_attributes(builder, relation);
                copy_tags(builder, relation.tags());

                // Copy the relation member list over to the new way.
                builder.add_item(relation.members());
            }
            m_buffer.commit();
        }

    };

}

