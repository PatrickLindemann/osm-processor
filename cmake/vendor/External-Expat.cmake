# Set installation paths
set( EXPAT_ROOT_DIR     ${VENDOR_BUILD_DIR}/expat )
set( EXPAT_DOWNLOAD_DIR ${EXPAT_ROOT_DIR}/src )
set( EXPAT_SOURCE_DIR   ${EXPAT_DOWNLOAD_DIR}/expat )
set( EXPAT_BINARIES_DIR ${EXPAT_DOWNLOAD_DIR}/expat-build )
set( EXPAT_INSTALL_DIR  ${EXPAT_ROOT_DIR}/target )

# Download, build and install the project
if ( WIN32 )
    ExternalProject_Add(
        expat
        PREFIX ${EXPAT_ROOT_DIR}
        INSTALL_DIR ${EXPAT_INSTALL_DIR}
        SOURCE_SUBDIR expat
        GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/libexpat/libexpat"
        GIT_TAG master
        GIT_PROGRESS ON
        TIMEOUT 10
        UPDATE_COMMAND ""
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${EXPAT_INSTALL_DIR}
            -DEXPAT_SHARED_LIBS=OFF 
    )
else()
    ExternalProject_Add(
        expat
        PREFIX ${EXPAT_ROOT_DIR}
        INSTALL_DIR ${EXPAT_INSTALL_DIR}
        GIT_REPOSITORY "${GIT_PROTOCOL}://github.com/libexpat/libexpat"
        GIT_TAG master
        GIT_PROGRESS true
        TIMEOUT 10
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND cd ${EXPAT_SOURCE_DIR}/expat && ./buildconf.sh  && ./configure
            --prefix=${EXPAT_INSTALL_DIR}
            --without-xmlwf
            CC=${CMAKE_C_COMPILER}
            CXX=${CMAKE_CXX_COMPILER}
            "CFLAGS=${EXPAT_CFLAGS} -w -DXML_POOR_ENTROPY"
            "CXXFLAGS=${EXPAT_CXXFLAGS} -w -DXML_POOR_ENTROPY"
        BUILD_COMMAND cd ${EXPAT_SOURCE_DIR}/expat &&  make -j ${CPU_COUNT}
        INSTALL_COMMAND cd ${EXPAT_SOURCE_DIR}/expat && make install
    )
endif()

# Set include and library paths
set( EXPAT_INCLUDE_DIR ${EXPAT_INSTALL_DIR}/include )
set( EXPAT_LIBRARIES ${EXPAT_INSTALL_DIR}/lib/libexpat.a )