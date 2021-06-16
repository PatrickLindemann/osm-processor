#include <memory>

#include <arrow/table.h>

#include "model.hpp"

namespace Serializer
{
    class ArrowTableBuilder
    {

        public:

            void add(Model::Boundary& b);

            std::shared_ptr<arrow::Table> build();

    };
}