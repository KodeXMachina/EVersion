# Defines:
#
#   eversion_declare(<target>
#       NAMESPACE      mylib::version_info
#       MAJOR          1
#       MINOR          2
#       PATCH          3
#       PRERELEASE     rc.1            # optional
#       BUILD_METADATA git.deadbeef    # optional
#       OUTPUT         mylib/version_info.h)
#
#   eversion_declare_plugin(<target>
#       ID             my.plugin
#       NAME           "My Plugin"
#       NAMESPACE      my_plugin::version_info
#       MAJOR          1
#       MINOR          2
#       PATCH          3
#       PRERELEASE     rc.1            # optional
#       BUILD_METADATA git.deadbeef    # optional
#       OUTPUT         my_plugin/version_info.h
#       SYMBOL         eversion_plugin_info)
#
# The generated header contains only inline constexpr C++ identifiers. It does
# not expose VERSION_* preprocessor macros to consumers.

if(COMMAND eversion_declare AND COMMAND eversion_declare_plugin)
    return()
endif()

if(NOT DEFINED EVERSION_TEMPLATE_DIR)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../templates/version_info.h.in")
        set(EVERSION_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}/../templates"
            CACHE INTERNAL "Path to EVersion generated-header templates")
    elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/templates/version_info.h.in")
        set(EVERSION_TEMPLATE_DIR "${CMAKE_CURRENT_LIST_DIR}/templates"
            CACHE INTERNAL "Path to EVersion generated-header templates")
    endif()
endif()

set(_EVERSION_DECLARE_TEMPLATE
    "${EVERSION_TEMPLATE_DIR}/version_info.h.in"
    CACHE INTERNAL "Default EVersion generated-header template")

set(_EVERSION_PLUGIN_INFO_TEMPLATE
    "${EVERSION_TEMPLATE_DIR}/plugin_info.cc.in"
    CACHE INTERNAL "Default EVersion generated plugin info template")

function(eversion_declare target_name)
    set(options "")
    set(one_value_args
        NAMESPACE
        MAJOR
        MINOR
        PATCH
        PRERELEASE
        BUILD_METADATA
        OUTPUT
        TEMPLATE
    )
    set(multi_value_args "")
    cmake_parse_arguments(EV "${options}" "${one_value_args}"
        "${multi_value_args}" ${ARGN})

    if(EV_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "eversion_declare(${target_name}): unknown arguments: "
            "${EV_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR
            "eversion_declare: target '${target_name}' does not exist")
    endif()

    get_target_property(aliased_target "${target_name}" ALIASED_TARGET)
    if(aliased_target)
        message(FATAL_ERROR
            "eversion_declare(${target_name}): pass the real target "
            "'${aliased_target}', not an ALIAS target")
    endif()

    foreach(required_arg IN ITEMS NAMESPACE MAJOR MINOR PATCH)
        if(NOT DEFINED EV_${required_arg})
            message(FATAL_ERROR
                "eversion_declare(${target_name}): missing required argument "
                "${required_arg}")
        endif()
    endforeach()

    if(NOT EV_NAMESPACE MATCHES
        "^[A-Za-z_][A-Za-z0-9_]*(::[A-Za-z_][A-Za-z0-9_]*)*$")
        message(FATAL_ERROR
            "eversion_declare(${target_name}): NAMESPACE='${EV_NAMESPACE}' "
            "is not a valid C++ nested namespace")
    endif()

    foreach(number_arg IN ITEMS MAJOR MINOR PATCH)
        set(number_value "${EV_${number_arg}}")
        if(NOT number_value MATCHES "^[0-9]+$")
            message(FATAL_ERROR
                "eversion_declare(${target_name}): ${number_arg}="
                "'${number_value}' must be a non-negative integer")
        endif()
    endforeach()

    if(NOT DEFINED EV_PRERELEASE)
        set(EV_PRERELEASE "")
    endif()
    if(NOT DEFINED EV_BUILD_METADATA)
        set(EV_BUILD_METADATA "")
    endif()

    set(identifier_pattern "^[0-9A-Za-z-]+(\\.[0-9A-Za-z-]+)*$")
    if(NOT EV_PRERELEASE STREQUAL ""
        AND NOT EV_PRERELEASE MATCHES "${identifier_pattern}")
        message(FATAL_ERROR
            "eversion_declare(${target_name}): PRERELEASE='${EV_PRERELEASE}' "
            "is not a valid dot-separated version identifier list")
    endif()
    if(NOT EV_BUILD_METADATA STREQUAL ""
        AND NOT EV_BUILD_METADATA MATCHES "${identifier_pattern}")
        message(FATAL_ERROR
            "eversion_declare(${target_name}): BUILD_METADATA="
            "'${EV_BUILD_METADATA}' is not a valid dot-separated version "
            "identifier list")
    endif()

    if(NOT DEFINED EV_OUTPUT)
        string(REPLACE "::" "/" namespace_path "${EV_NAMESPACE}")
        set(EV_OUTPUT "${namespace_path}/version_info.h")
    endif()
    if(NOT DEFINED EV_TEMPLATE)
        set(EV_TEMPLATE "${_EVERSION_DECLARE_TEMPLATE}")
    endif()
    if(NOT EXISTS "${EV_TEMPLATE}")
        message(FATAL_ERROR
            "eversion_declare(${target_name}): template '${EV_TEMPLATE}' "
            "does not exist")
    endif()

    set(version_string "${EV_MAJOR}.${EV_MINOR}.${EV_PATCH}")
    if(NOT EV_PRERELEASE STREQUAL "")
        set(version_string "${version_string}-${EV_PRERELEASE}")
    endif()
    if(NOT EV_BUILD_METADATA STREQUAL "")
        set(version_string "${version_string}+${EV_BUILD_METADATA}")
    endif()

    set(output_dir
        "${CMAKE_CURRENT_BINARY_DIR}/eversion_generated/${target_name}/include")
    set(output_file "${output_dir}/${EV_OUTPUT}")

    set(EVERSION_NAMESPACE "${EV_NAMESPACE}")
    set(EVERSION_NAMESPACE_OPEN "namespace ${EV_NAMESPACE} {")
    set(EVERSION_NAMESPACE_CLOSE "}  // namespace ${EV_NAMESPACE}")
    set(EVERSION_MAJOR_LITERAL "${EV_MAJOR}U")
    set(EVERSION_MINOR_LITERAL "${EV_MINOR}U")
    set(EVERSION_PATCH_LITERAL "${EV_PATCH}U")
    set(EVERSION_PRERELEASE "${EV_PRERELEASE}")
    set(EVERSION_BUILD_METADATA "${EV_BUILD_METADATA}")
    if(EV_PRERELEASE STREQUAL "")
        set(EVERSION_PRERELEASE_VIEW_LITERAL "std::string_view{}")
    else()
        set(EVERSION_PRERELEASE_VIEW_LITERAL
            "std::string_view{\"${EV_PRERELEASE}\"}")
    endif()
    if(EV_BUILD_METADATA STREQUAL "")
        set(EVERSION_BUILD_METADATA_VIEW_LITERAL "std::string_view{}")
    else()
        set(EVERSION_BUILD_METADATA_VIEW_LITERAL
            "std::string_view{\"${EV_BUILD_METADATA}\"}")
    endif()
    set(EVERSION_VERSION_STRING "${version_string}")
    set(EVERSION_DECLARE_CALLER "${CMAKE_CURRENT_LIST_FILE}")

    configure_file("${EV_TEMPLATE}" "${output_file}" @ONLY)

    get_target_property(target_type "${target_name}" TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY")
        set(scope INTERFACE)
    else()
        set(scope PUBLIC)
    endif()

    target_include_directories("${target_name}" ${scope}
        $<BUILD_INTERFACE:${output_dir}>
        $<INSTALL_INTERFACE:include>
    )
    if(NOT "${target_name}" STREQUAL "eversion")
        target_link_libraries("${target_name}" ${scope} eversion::eversion)
    endif()

    set_target_properties("${target_name}" PROPERTIES
        EVERSION_GENERATED_HEADER "${output_file}"
        EVERSION_VERSION_STRING "${version_string}"
    )
endfunction()

function(_eversion_reject_unsafe_cxx_string argument_name argument_value)
    string(FIND "${argument_value}" "\"" quote_position)
    string(FIND "${argument_value}" "\\" slash_position)
    string(FIND "${argument_value}" "\n" newline_position)
    string(FIND "${argument_value}" "\r" carriage_return_position)

    if(quote_position GREATER_EQUAL 0
        OR slash_position GREATER_EQUAL 0
        OR newline_position GREATER_EQUAL 0
        OR carriage_return_position GREATER_EQUAL 0)
        message(FATAL_ERROR
            "${argument_name} must not contain quotes, backslashes, "
            "or newlines")
    endif()
endfunction()

function(eversion_declare_plugin target_name)
    set(options "")
    set(one_value_args
        ID
        NAME
        NAMESPACE
        MAJOR
        MINOR
        PATCH
        PRERELEASE
        BUILD_METADATA
        OUTPUT
        SYMBOL
    )
    set(multi_value_args "")
    cmake_parse_arguments(EVP "${options}" "${one_value_args}"
        "${multi_value_args}" ${ARGN})

    if(EVP_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "eversion_declare_plugin(${target_name}): unknown arguments: "
            "${EVP_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR
            "eversion_declare_plugin: target '${target_name}' does not exist")
    endif()

    get_target_property(aliased_target "${target_name}" ALIASED_TARGET)
    if(aliased_target)
        message(FATAL_ERROR
            "eversion_declare_plugin(${target_name}): pass the real target "
            "'${aliased_target}', not an ALIAS target")
    endif()

    get_target_property(target_type "${target_name}" TYPE)
    if(NOT target_type STREQUAL "SHARED_LIBRARY"
        AND NOT target_type STREQUAL "MODULE_LIBRARY")
        message(FATAL_ERROR
            "eversion_declare_plugin(${target_name}): target must be SHARED "
            "or MODULE, got ${target_type}")
    endif()

    foreach(required_arg IN ITEMS ID NAME NAMESPACE MAJOR MINOR PATCH)
        if(NOT DEFINED EVP_${required_arg})
            message(FATAL_ERROR
                "eversion_declare_plugin(${target_name}): missing required "
                "argument ${required_arg}")
        endif()
    endforeach()

    if(NOT EVP_ID MATCHES "^[0-9A-Za-z_.-]+$")
        message(FATAL_ERROR
            "eversion_declare_plugin(${target_name}): ID='${EVP_ID}' must "
            "contain only letters, digits, dots, underscores, and hyphens")
    endif()
    if(EVP_NAME STREQUAL "")
        message(FATAL_ERROR
            "eversion_declare_plugin(${target_name}): NAME must not be empty")
    endif()
    _eversion_reject_unsafe_cxx_string("NAME" "${EVP_NAME}")

    if(NOT DEFINED EVP_SYMBOL)
        set(EVP_SYMBOL "eversion_plugin_info")
    endif()
    if(NOT EVP_SYMBOL MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "eversion_declare_plugin(${target_name}): SYMBOL='${EVP_SYMBOL}' "
            "is not a valid C function identifier")
    endif()

    set(declare_args
        NAMESPACE "${EVP_NAMESPACE}"
        MAJOR "${EVP_MAJOR}"
        MINOR "${EVP_MINOR}"
        PATCH "${EVP_PATCH}"
    )
    if(DEFINED EVP_PRERELEASE)
        list(APPEND declare_args PRERELEASE "${EVP_PRERELEASE}")
    endif()
    if(DEFINED EVP_BUILD_METADATA)
        list(APPEND declare_args BUILD_METADATA "${EVP_BUILD_METADATA}")
    endif()
    if(DEFINED EVP_OUTPUT)
        list(APPEND declare_args OUTPUT "${EVP_OUTPUT}")
    endif()
    eversion_declare("${target_name}" ${declare_args})

    if(NOT EXISTS "${_EVERSION_PLUGIN_INFO_TEMPLATE}")
        message(FATAL_ERROR
            "eversion_declare_plugin(${target_name}): template "
            "'${_EVERSION_PLUGIN_INFO_TEMPLATE}' does not exist")
    endif()

    set(plugin_output_dir
        "${CMAKE_CURRENT_BINARY_DIR}/eversion_generated/${target_name}/src")
    set(plugin_output_file
        "${plugin_output_dir}/${target_name}_plugin_info.cpp")

    set(EVERSION_PLUGIN_ID "${EVP_ID}")
    set(EVERSION_PLUGIN_NAME "${EVP_NAME}")
    set(EVERSION_PLUGIN_SYMBOL "${EVP_SYMBOL}")
    set(EVERSION_MAJOR_LITERAL "${EVP_MAJOR}U")
    set(EVERSION_MINOR_LITERAL "${EVP_MINOR}U")
    set(EVERSION_PATCH_LITERAL "${EVP_PATCH}U")
    if(DEFINED EVP_PRERELEASE)
        set(EVERSION_PRERELEASE "${EVP_PRERELEASE}")
    else()
        set(EVERSION_PRERELEASE "")
    endif()
    if(DEFINED EVP_BUILD_METADATA)
        set(EVERSION_BUILD_METADATA "${EVP_BUILD_METADATA}")
    else()
        set(EVERSION_BUILD_METADATA "")
    endif()
    set(EVERSION_DECLARE_CALLER "${CMAKE_CURRENT_LIST_FILE}")

    if(WIN32)
        set(EVERSION_PLUGIN_EXPORT "__declspec(dllexport)")
    else()
        set(EVERSION_PLUGIN_EXPORT
            "__attribute__((visibility(\"default\")))")
    endif()

    configure_file("${_EVERSION_PLUGIN_INFO_TEMPLATE}"
        "${plugin_output_file}" @ONLY)
    target_sources("${target_name}" PRIVATE "${plugin_output_file}")

    set_target_properties("${target_name}" PROPERTIES
        EVERSION_PLUGIN_INFO_SOURCE "${plugin_output_file}"
        EVERSION_PLUGIN_INFO_SYMBOL "${EVP_SYMBOL}"
    )
endfunction()
