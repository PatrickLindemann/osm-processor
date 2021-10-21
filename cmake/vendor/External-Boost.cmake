# Set installation paths
set( BOOST_ROOT_DIR     ${VENDOR_BUILD_DIR}/boost )
set( BOOST_DOWNLOAD_DIR ${BOOST_ROOT_DIR}/src )
set( BOOST_SOURCE_DIR   ${BOOST_DOWNLOAD_DIR}/boost )
set( BOOST_BINARIES_DIR ${BOOST_DOWNLOAD_DIR}/boost-build )
set( BOOST_INSTALL_DIR  ${BOOST_ROOT_DIR}/target )

# Set Variables
set( BOOST_BOOTSTRAP_COMMAND )
set( BOOST_B2_COMMAND )
if( UNIX )
    set( BOOST_BOOTSTRAP_COMMAND ./bootstrap.sh )
    set( BOOST_B2_COMMAND ./b2 )
elseif( WIN32 )
    set( BOOST_BOOTSTRAP_COMMAND bootstrap.bat )
    set( BOOST_B2_COMMAND b2.exe )
endif()

if( BOOST_SUBMODULES )
    list( TRANSFORM BOOST_SUBMODULES PREPEND "libs/" )
endif()

# Download, build and install the project
ExternalProject_Add(
    boost
    PREFIX ${BOOST_ROOT_DIR}
    INSTALL_DIR ${BOOST_INSTALL_DIR}
    GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/boostorg/boost.git"
    GIT_TAG master
    GIT_SUBMODULES  ${BOOST_SUBMODULES}
    GIT_PROGRESS true
    TIMEOUT 10
    BUILD_IN_SOURCE 1
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND ${BOOST_BOOTSTRAP_COMMAND}
    BUILD_COMMAND  ${BOOST_B2_COMMAND} install
        --without-python
        --without-mpi
        --disable-icu
        --prefix=${BOOST_INSTALL_DIR}
        --threading=single,multi
        --link=shared
        --variant=release
        -j8
    INSTALL_COMMAND ""
)

# Set include and library paths
if( NOT WIN32 )
    set( BOOST_INCLUDE_DIR ${BOOST_INSTALL_DIR}/include )
    set( BOOST_LIBRARIES   ${BOOST_INSTALL_DIR}/lib/boost )
else()
    set( BOOST_INCLUDE_DIR ${BOOST_INSTALL_DIR}/include/boost )
    set( BOOST_LIBRARIES   ${BOOST_INSTALL_DIR}/lib )
endif()