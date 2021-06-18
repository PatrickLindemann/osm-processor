#ifndef OSM_PROCESSOR_EXTRACTOR
#define OSM_PROCESSOR_EXTRACTOR

#include <string>
#include <arrow/api.h>

#include "model.hpp"
#include "serializer.hpp"

namespace Extractor
{
    void run(Serializer::BoundaryArrowBuilder& builder, std::string input_path, std::string boundary_type, double epsilon);
}

#endif