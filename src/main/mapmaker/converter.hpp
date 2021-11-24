#pragma once

#include <map>
#include <vector>

#include <osmium/memory/buffer.hpp>

#include "model/boundary.hpp"
#include "handler/convert_handler.hpp"

using namespace model;

namespace mapmaker
{

	template <typename T>
	class BoundaryConverter
	{
    protected:

        /* Types */

       /**
        * The abstract transformation type.
        */
        using Transformation = std::shared_ptr<functions::Transformation<T>>;

        /* Members */

        std::vector<Transformation> m_transformations;

	public:

        /* Constructors */

		BoundaryConverter() {}
        BoundaryConverter(Transformation transformation) : m_transformations({ transformation }) {}
        BoundaryConverter(std::initializer_list<Transformation> transformations) : m_transformations(transformations) {}

        /* Methods */

		std::map<model::object_id_type, model::Boundary<T>> run(const osmium::memory::Buffer& buffer)
		{
            handler::BoundaryConvertHandler<T> convert_handler{ m_transformations };
            osmium::apply(buffer, convert_handler);
            return convert_handler.boundaries();
		}

	};

}