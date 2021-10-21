# Set installation paths
set( PROTOZERO_ROOT_DIR     ${VENDOR_BUILD_DIR}/protozero )
set( PROTOZERO_DOWNLOAD_DIR ${PROTOZERO_ROOT_DIR}/src )
set( PROTOZERO_SOURCE_DIR   ${PROTOZERO_DOWNLOAD_DIR}/protozero )

# Download, build and install the project
ExternalProject_Add(
    protozero
    PREFIX ${PROTOZERO_ROOT_DIR}
    GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/mapbox/protozero.git"
    GIT_TAG master
    GIT_PROGRESS true
    TIMEOUT 10
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

# Set include and library paths
set( PROTOZERO_INCLUDE_DIR ${PROTOZERO_SOURCE_DIR}/include )

# Check version number
if( PROTOZERO_FIND_VERSION )
     file( STRINGS "${PROTOZERO_INCLUDE_DIR}/protozero/version.hpp" _version_define REGEX "#define PROTOZERO_VERSION_STRING" )
     if( "${_version_define}" MATCHES "#define PROTOZERO_VERSION_STRING \"([0-9.]+)\"" )
         set( PROTOZERO_VERSION "${CMAKE_MATCH_1}" )
     else()
         set( PROTOZERO_VERSION "unknown" )
     endif()
 endif()