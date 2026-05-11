add_library(eversion_test_consumer INTERFACE)
eversion_declare(eversion_test_consumer
    NAMESPACE eversion_demo::version_info
    MAJOR 1
    MINOR 2
    PATCH 3
    PRERELEASE rc.1
    BUILD_METADATA git.deadbeef
    OUTPUT eversion_demo/version_info.h
)

add_executable(eversion_tests
    version_test.cc
    version_declare_test.cc
    runtime_test.cc
)
target_link_libraries(eversion_tests PRIVATE
    eversion::eversion
    eversion_test_consumer
)
target_compile_features(eversion_tests PRIVATE cxx_std_20)

add_test(NAME eversion_tests COMMAND eversion_tests)

add_library(eversion_test_plugin MODULE)
set_target_properties(eversion_test_plugin PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/plugins"
)
eversion_declare_plugin(eversion_test_plugin
    ID test.plugin
    NAME "EVersion Test Plugin"
    NAMESPACE eversion_test_plugin::version_info
    MAJOR 9
    MINOR 8
    PATCH 7
    PRERELEASE beta.2
    BUILD_METADATA build.42
    OUTPUT eversion_test_plugin/version_info.h
)

add_executable(eversion_plugin_info_test
    plugin_info_test.cc
)
target_link_libraries(eversion_plugin_info_test PRIVATE
    eversion::eversion
    ${CMAKE_DL_LIBS}
)
target_compile_features(eversion_plugin_info_test PRIVATE cxx_std_20)
add_dependencies(eversion_plugin_info_test eversion_test_plugin)

add_test(NAME eversion_plugin_info_test
    COMMAND eversion_plugin_info_test "$<TARGET_FILE:eversion_test_plugin>"
)
