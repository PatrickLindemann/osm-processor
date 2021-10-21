# Set installation paths
set( ZLIB_ROOT_DIR     ${VENDOR_BUILD_DIR}/zlib )
set( ZLIB_DOWNLOAD_DIR ${ZLIB_ROOT_DIR}/src )
set( ZLIB_SOURCE_DIR   ${ZLIB_DOWNLOAD_DIR}/zlib )
set( ZLIB_BINARIES_DIR ${ZLIB_DOWNLOAD_DIR}/zlib-build )
set( ZLIB_INSTALL_DIR  ${ZLIB_ROOT_DIR}/target )

# Download, build and install the project
ExternalProject_Add(
    zlib
    PREFIX ${ZLIB_ROOT_DIR}
    INSTALL_DIR ${ZLIB_INSTALL_DIR}
    GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/madler/zlib"
    GIT_TAG master
    GIT_PROGRESS ON
    TIMEOUT 10
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${ZLIB_INSTALL_DIR}
)

# Set include and library paths
set( ZLIB_INCLUDE_DIR ${ZLIB_INSTALL_DIR}/include )
if ( WIN32 )
    set( ZLIB_LIBRARIES ${ZLIB_INSTALL_DIR}/lib/libzlib.dll.a )
else()
    set( ZLIB_LIBRARIES ${ZLIB_INSTALL_DIR}/lib/libz.a )
endif()