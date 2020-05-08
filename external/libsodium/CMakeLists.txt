cmake_minimum_required(VERSION 3.12)

project(LIBSODIUM_DOWNLOAD VERSION 1.0.18)

include(ExternalProject)
ExternalProject_Add(EP_LIBSODIUM
    URL                 https://download.libsodium.org/libsodium/releases/libsodium-${PROJECT_VERSION}-stable.tar.gz
    TMP_DIR             ${CMAKE_CURRENT_LIST_DIR}/tmp
    STAMP_DIR           ${CMAKE_CURRENT_LIST_DIR}/stamp
    DOWNLOAD_DIR        ""
    SOURCE_DIR          ${CMAKE_CURRENT_LIST_DIR}/src
    BINARY_DIR          ${CMAKE_CURRENT_LIST_DIR}/src
    CONFIGURE_COMMAND   "./configure"
    BUILD_COMMAND       make -j8
    INSTALL_COMMAND     make install
    TEST_COMMAND        make check)
