cmake_minimum_required(VERSION 3.21)

project(eversion_install_consumer
    VERSION 4.5.6
    LANGUAGES CXX
)

find_package(EVersion CONFIG REQUIRED)

add_library(install_consumer INTERFACE)
eversion_declare(install_consumer
    NAMESPACE install_consumer::version_info
    MAJOR ${PROJECT_VERSION_MAJOR}
    MINOR ${PROJECT_VERSION_MINOR}
    PATCH ${PROJECT_VERSION_PATCH}
    OUTPUT install_consumer/version_info.h
)

add_library(install_consumer_plugin MODULE)
eversion_declare_plugin(install_consumer_plugin
    ID install.consumer.plugin
    NAME "Install Consumer Plugin"
    NAMESPACE install_consumer_plugin::version_info
    MAJOR 7
    MINOR 8
    PATCH 9
    PRERELEASE beta.1
    OUTPUT install_consumer_plugin/version_info.h
)

add_executable(install_consumer_app
    main.cpp
)
target_link_libraries(install_consumer_app PRIVATE
    eversion::eversion
    install_consumer
)
target_compile_features(install_consumer_app PRIVATE cxx_std_20)
