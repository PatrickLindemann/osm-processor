# Set installation paths
set( NLOHMANN_JSON_ROOT_DIR     ${VENDOR_BUILD_DIR}/nlohmann-json )
set( NLOHMANN_JSON_DOWNLOAD_DIR ${NLOHMANN_JSON_ROOT_DIR}/src )
set( NLOHMANN_JSON_SOURCE_DIR   ${NLOHMANN_JSON_DOWNLOAD_DIR}/nlohmann-json )

# Download the project. As it is a header-only library, there is no need
# to build and install it.
ExternalProject_Add(
    nlohmann-json
    PREFIX ${NLOHMANN_JSON_ROOT_DIR}
    GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/nlohmann/json.git"
    GIT_TAG master
    GIT_PROGRESS ON
    TIMEOUT 10
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)

# Set include path
set( NLOHMANN_JSON_INCLUDE_DIR ${NLOHMANN_JSON_SOURCE_DIR}/include )