# Set installation paths
set( BZIP2_ROOT_DIR     ${VENDOR_BUILD_DIR}/bzip2 )
set( BZIP2_DOWNLOAD_DIR ${BZIP2_ROOT_DIR}/src )
set( BZIP2_SOURCE_DIR   ${BZIP2_DOWNLOAD_DIR}/bzip2 )
set( BZIP2_BINARIES_DIR ${BZIP2_DOWNLOAD_DIR}/bzip2-build )
set( BZIP2_INSTALL_DIR  ${BZIP2_ROOT_DIR}/target )

# Download, build and install the project
ExternalProject_Add(
    bzip2
    PREFIX ${BZIP2_ROOT_DIR}
    INSTALL_DIR ${BZIP2_INSTALL_DIR}
    GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/commontk/bzip2.git"
    GIT_TAG master
    GIT_PROGRESS true
    TIMEOUT 10
    UPDATE_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${BZIP2_INSTALL_DIR}
)

# Set include and library paths
set( BZIP2_INCLUDE_DIR ${BZIP2_INSTALL_DIR}/include )
set ( BZIP2_LIBRARIES ${BZIP2_INSTALL_DIR}/lib/libbz2.a )