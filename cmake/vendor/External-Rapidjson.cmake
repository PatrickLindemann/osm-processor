# Set installation paths
set( RAPIDJSON_ROOT_DIR     ${VENDOR_BUILD_DIR}/rapidjson )
set( RAPIDJSON_DOWNLOAD_DIR ${RAPIDJSON_ROOT_DIR}/src )
set( RAPIDJSON_SOURCE_DIR   ${RAPIDJSON_DOWNLOAD_DIR}/rapidjson )

# Download the project. As it is a header-only library, there is no need
# to build and install it.
ExternalProject_Add(
    rapidjson
    PREFIX ${RAPIDJSON_ROOT_DIR}
    GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/Tencent/rapidjson.git"
    GIT_TAG master
    GIT_PROGRESS ON
    TIMEOUT 10
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

# Set include path
set( RAPIDJSON_INCLUDE_DIR ${RAPIDJSON_SOURCE_DIR}/include )