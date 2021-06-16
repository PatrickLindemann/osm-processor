#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/memory_pool.h>
#include <parquet/exception.h>

#include "serializer.hpp"

namespace Serializer
{
    class ArrowBuilder
    {
        
        arrow::MemoryPool* pool = arrow::default_memory_pool();

        arrow::Int64Builder             m_id_builder_{ pool };
        arrow::StringBuilder            m_name_builder_{ pool };
        arrow::StringBuilder            m_type_builder_{ pool };
        arrow::Int32Builder             m_admin_level_builder_{ pool };
        arrow::StringBuilder            m_source_builder_{ pool };
        arrow::StringBuilder            m_wikidata_builder_{ pool };

        arrow::FixedSizeListBuilder     m_center_builder_{ pool, std::make_shared<arrow::DoubleBuilder>(pool), 2 };
        arrow::DoubleBuilder&           m_center_double_builder_ = *(static_cast<arrow::DoubleBuilder*>(m_center_builder_.value_builder()));

        arrow::FixedSizeListBuilder     m_bounds_builder_{ // Bounds
            pool, std::make_shared<arrow::FixedSizeListBuilder>( // Points (top-right, bottom-left)
                pool, std::make_shared<arrow::DoubleBuilder>(pool), 2 // Point values (lon, lat)
            ), 2
        };
        arrow::FixedSizeListBuilder&    m_bounds_point_builder_ = *(static_cast<arrow::FixedSizeListBuilder*>(m_bounds_builder_.value_builder()));
        arrow::DoubleBuilder&           m_bounds_double_builder_ = *(static_cast<arrow::DoubleBuilder*>(m_bounds_point_builder_.value_builder()));

        arrow::ListBuilder              m_geometry_builder_{ // Multipolygon
            pool, std::make_shared<arrow::ListBuilder>(  // Polygons (1 outer ring + n inner rings)
                pool, std::make_shared<arrow::FixedSizeListBuilder>( // Points (n pairs)
                    pool, std::make_shared<arrow::DoubleBuilder>(pool), 2 // Point Values (lon, lat)
                )
            )
        };
        arrow::ListBuilder&             m_geometry_polygon_builder_ = *(static_cast<arrow::ListBuilder*>(m_geometry_builder_.value_builder()));
        arrow::FixedSizeListBuilder&    m_geometry_point_builder_ = *(static_cast<arrow::FixedSizeListBuilder*>(m_geometry_polygon_builder_.value_builder()));
        arrow::DoubleBuilder&           m_geometry_double_builder_ = *(static_cast<arrow::DoubleBuilder*>(m_geometry_point_builder_.value_builder()));

        const std::shared_ptr<arrow::DataType> point_type = arrow::fixed_size_list(arrow::float64(), 2);

        const std::shared_ptr<arrow::DataType> bounds_type = arrow::fixed_size_list(point_type, 2);

        const std::shared_ptr<arrow::DataType> polygon_type = arrow::list(point_type);

        const std::shared_ptr<arrow::DataType> geometry_type = arrow::list(polygon_type);

        const std::shared_ptr<arrow::Schema> table_schema = arrow::schema({
            arrow::field("id", arrow::int64(), false),
            arrow::field("name", arrow::utf8()),
            arrow::field("type", arrow::utf8(), false),
            arrow::field("admin_level", arrow::int32()),
            arrow::field("source", arrow::utf8()),
            arrow::field("wikidata", arrow::utf8()),
            arrow::field("center", point_type, false),
            arrow::field("bounds", bounds_type, false),
            arrow::field("geometry", geometry_type, false)
        });

    public:

        void add(Model::Boundary& b) {
            
            // Append primitive values
            PARQUET_THROW_NOT_OK(b.id ? m_id_builder_.Append(b.id) : m_id_builder_.AppendNull());
            PARQUET_THROW_NOT_OK(b.name ? m_name_builder_.Append(b.name) : m_name_builder_.AppendNull()); 
            PARQUET_THROW_NOT_OK(b.type ? m_type_builder_.Append(b.type) : m_type_builder_.AppendNull());
            PARQUET_THROW_NOT_OK(b.admin_level ? m_admin_level_builder_.Append(b.admin_level) : m_admin_level_builder_.AppendNull());             
            PARQUET_THROW_NOT_OK(b.source ? m_source_builder_.Append(b.source) : m_source_builder_.AppendNull());
            PARQUET_THROW_NOT_OK(b.wikidata ? m_wikidata_builder_.Append(b.wikidata) : m_wikidata_builder_.AppendNull());
            
            // Build the center point as [double, double] array
            PARQUET_THROW_NOT_OK(m_center_builder_.Append());
            PARQUET_THROW_NOT_OK(m_center_double_builder_.AppendValues({
                b.center.lon(),
                b.center.lat()
            }));
            
            // Build the bounding box as [[double, double], [double, double]] array
            PARQUET_THROW_NOT_OK(m_bounds_builder_.Append());
            PARQUET_THROW_NOT_OK(m_bounds_point_builder_.Append());
            PARQUET_THROW_NOT_OK(m_bounds_double_builder_.AppendValues({
                b.bounds.top_right().lon(),
                b.bounds.top_right().lat(),
            }));
            PARQUET_THROW_NOT_OK(m_bounds_point_builder_.Append());
            PARQUET_THROW_NOT_OK(m_bounds_double_builder_.AppendValues({
                b.bounds.bottom_left().lon(),
                b.bounds.bottom_left().lat()
            }));

            // Build the boundary geometry
            PARQUET_THROW_NOT_OK(m_geometry_builder_.Append());
            for (const osmium::OuterRing& outer : b.geometry.outer_rings())
            {
                // Add the polygon defined by the outer ring first
                PARQUET_THROW_NOT_OK(m_geometry_polygon_builder_.Append());
                for (const osmium::NodeRef& n : outer)
                {
                    PARQUET_THROW_NOT_OK(m_geometry_point_builder_.Append());
                    PARQUET_THROW_NOT_OK(m_geometry_double_builder_.AppendValues({
                        n.lon(),
                        n.lat()
                    }));
                }
                // Add all inner rings
                for (const osmium::InnerRing& inner : b.geometry.inner_rings(outer))
                {
                    PARQUET_THROW_NOT_OK(m_geometry_polygon_builder_.Append());
                    for (const osmium::NodeRef& n : inner)
                    {
                        PARQUET_THROW_NOT_OK(m_geometry_point_builder_.Append());
                        PARQUET_THROW_NOT_OK(m_geometry_double_builder_.AppendValues({
                            n.lon(),
                            n.lat()
                        }));
                    }
                }
            }
        }

        const std::shared_ptr<arrow::Table> build()
        {   
            // Prepare the column arrays that contain the boundary data
            // Each array entry represents cell data per row
            std::shared_ptr<arrow::Array> ids, names, types, admin_levels, sources,
                                        wikidata, centers, bounds, geometries;

            // Create the column arrays with the builders
            PARQUET_THROW_NOT_OK(m_id_builder_.Finish(&ids));
            PARQUET_THROW_NOT_OK(m_name_builder_.Finish(&names));
            PARQUET_THROW_NOT_OK(m_type_builder_.Finish(&types));
            PARQUET_THROW_NOT_OK(m_admin_level_builder_.Finish(&admin_levels));
            PARQUET_THROW_NOT_OK(m_source_builder_.Finish(&sources));
            PARQUET_THROW_NOT_OK(m_wikidata_builder_.Finish(&wikidata));
            PARQUET_THROW_NOT_OK(m_center_builder_.Finish(&centers));
            PARQUET_THROW_NOT_OK(m_bounds_builder_.Finish(&bounds));
            PARQUET_THROW_NOT_OK(m_geometry_builder_.Finish(&geometries));

            // Create the table with the column arrays
            return arrow::Table::Make(table_schema, {
                ids,
                names,
                types,
                admin_levels,
                sources,
                wikidata,
                centers,
                bounds,
                geometries
            });
        }

    };
}