cmake_minimum_required(VERSION 3.21)

project(eversion
    VERSION 0.1.0
    DESCRIPTION "Easy macro-free version metadata helpers for C++20"
    LANGUAGES CXX
)

option(EVERSION_BUILD_TESTS "Build EVersion tests" ${PROJECT_IS_TOP_LEVEL})
option(EVERSION_INSTALL "Install EVersion package files" ${PROJECT_IS_TOP_LEVEL})

add_library(eversion INTERFACE)
add_library(eversion::eversion ALIAS eversion)

target_compile_features(eversion INTERFACE cxx_std_20)
target_include_directories(eversion INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set(EVERSION_TEMPLATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates"
    CACHE INTERNAL "Path to EVersion generated-header templates")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/EVersionDeclare.cmake")

if(EVERSION_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(EVERSION_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS eversion
        EXPORT EVersionTargets
    )
    install(DIRECTORY include/
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    install(FILES cmake/EVersionDeclare.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EVersion"
    )
    install(DIRECTORY templates/
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EVersion/templates"
    )

    configure_package_config_file(
        cmake/EVersionConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/EVersionConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EVersion"
    )
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/EVersionConfigVersion.cmake"
        VERSION "${PROJECT_VERSION}"
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/EVersionConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/EVersionConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EVersion"
    )
    install(EXPORT EVersionTargets
        NAMESPACE eversion::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/EVersion"
    )
endif()

message(STATUS "eversion ${PROJECT_VERSION} configured")
message(STATUS "  tests  : ${EVERSION_BUILD_TESTS}")
message(STATUS "  install: ${EVERSION_INSTALL}")
