#include <parquet/arrow/reader.h>
#include <parquet/arrow/writer.h>
#include <parquet/exception.h>

#include "serializer.hpp"

namespace Serializer
{
    
    void BoundaryArrowBuilder::BoundaryArrowBuilder::append(Model::Boundary &b)
    {
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

    void BoundaryArrowBuilder::BoundaryArrowBuilder::finish()
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
        m_table = arrow::Table::Make(m_table_schema, {
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

    void BoundaryArrowBuilder::BoundaryArrowBuilder::write(std::string file_path, int64_t chunk_size)
    {
        std::shared_ptr<arrow::io::FileOutputStream> file;
        PARQUET_ASSIGN_OR_THROW(
            file,
            arrow::io::FileOutputStream::Open(file_path)
        );

        // The last argument to the function call is the size of the RowGroup in
        // the parquet file. Normally you would choose this to be rather large but
        // for the example, we use a small value to have multiple RowGroups.
        PARQUET_THROW_NOT_OK(
            parquet::arrow::WriteTable(*m_table, pool, file, chunk_size)
        );
    }

}