#include <string>
#include <arrow/api.h>

namespace AreaExtractor
{
    std::shared_ptr<arrow::Table> run(std::string input_path, std::string boundary_type, double epsilon);
}