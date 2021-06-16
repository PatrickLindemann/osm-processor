#include <osmium/osm/box.hpp>
#include <osmium/osm/area.hpp>
#include <osmium/osm/location.hpp>

namespace Model
{
    struct Boundary
    {
        public:

            const long id;
            const char * name;
            const char * type;
            const int admin_level;
            const char * source;
            const char * wikidata;
            const osmium::Location center;
            const osmium::Box bounds;
            const osmium::Area& geometry;
    };

    /**
     * Prints a boundary to the console
     * 
     * @param b The boundary
     */
    static void print_boundary(Boundary& b) {
        std::printf(
            "Boundary("
                "id=%ld,"
                "name=\"%s\","
                "type=\"%s\","
                "admin_level=\"%i\","
                "source=\"%s\","
                "wikidata=\"%s\","
                "center=[%6.4f,%6.4f],"
                "bounds=[[%6.4f,%6.4f],[%6.4f,%6.4f]]"
            ")\n",
            b.id,
            b.name ? b.name : "",
            b.type ? b.type : "",
            b.admin_level,
            b.source ? b.source : "",
            b.wikidata ? b.wikidata : "",
            b.center.lon(),
            b.center.lat(),
            b.bounds.top_right().lon(),
            b.bounds.top_right().lat(),
            b.bounds.bottom_left().lon(),
            b.bounds.bottom_left().lat()
        );
    }
}
