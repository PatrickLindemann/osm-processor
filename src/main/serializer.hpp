#ifndef OSM_PROCESSOR_SERIALIZER
#define OSM_PROCESSOR_SERIALIZER

#include <memory>

#include <arrow/api.h>
#include <arrow/io/api.h>
#include <arrow/memory_pool.h>

#include "model.hpp"

namespace Serializer
{   

    class BoundaryArrowBuilder
    {   
    private:

        /**
         * The arrow table
         */
        std::shared_ptr<arrow::Table> m_table;

        /* Schema */

        const std::shared_ptr<arrow::DataType> point_type = arrow::fixed_size_list(arrow::float64(), 2);

        const std::shared_ptr<arrow::DataType> bounds_type = arrow::fixed_size_list(point_type, 2);

        const std::shared_ptr<arrow::DataType> polygon_type = arrow::list(point_type);

        const std::shared_ptr<arrow::DataType> geometry_type = arrow::list(polygon_type);

        const std::shared_ptr<arrow::Schema> m_table_schema = arrow::schema({
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

        /* Builders */

        /**
         * The memory pool for this table builder
         */
        arrow::MemoryPool* pool = arrow::default_memory_pool();

        /**
         * The primitive column builders
         */
        arrow::Int64Builder             m_id_builder_{ pool };
        arrow::StringBuilder            m_name_builder_{ pool };
        arrow::StringBuilder            m_type_builder_{ pool };
        arrow::Int32Builder             m_admin_level_builder_{ pool };
        arrow::StringBuilder            m_source_builder_{ pool };
        arrow::StringBuilder            m_wikidata_builder_{ pool };

        /**
         * The center point builders
         */
        arrow::FixedSizeListBuilder     m_center_builder_{ pool, std::make_shared<arrow::DoubleBuilder>(pool), 2 };
        arrow::DoubleBuilder&           m_center_double_builder_ = *(static_cast<arrow::DoubleBuilder*>(m_center_builder_.value_builder()));

        /**
         * The bounding box builders
         */
        arrow::FixedSizeListBuilder     m_bounds_builder_{ // Bounds
            pool, std::make_shared<arrow::FixedSizeListBuilder>( // Points (top-right, bottom-left)
                pool, std::make_shared<arrow::DoubleBuilder>(pool), 2 // Point values (lon, lat)
            ), 2
        };
        arrow::FixedSizeListBuilder&    m_bounds_point_builder_ = *(static_cast<arrow::FixedSizeListBuilder*>(m_bounds_builder_.value_builder()));
        arrow::DoubleBuilder&           m_bounds_double_builder_ = *(static_cast<arrow::DoubleBuilder*>(m_bounds_point_builder_.value_builder()));

        // The geometry builders
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

    public:

        const std::shared_ptr<arrow::Table> table()
        {
            return m_table;
        }

        const std::shared_ptr<arrow::Schema> schema()
        {
            return m_table_schema;
        }

        void append(Model::Boundary &b);

        void finish();

        void write(std::string file_path, int64_t chunk_size = 3);

    };

}

#endif