#cmake_minimum_required(VERSION 3.22...3.30)
cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

function(remove_in_target_property target property value)
    if(NOT TARGET "${target}")
        message(WARNING "remove_in_target_property: Target '${target}' does not exist")
        return()
    endif()

    get_target_property(current "${target}" "${property}")

    if(NOT current)
        # Property not set or empty → nothing to do
        return()
    endif()

    list(FIND current "${old_value}" idx)
    if(idx EQUAL -1)
        # Not found → no change
        return()
    endif()

    list(REMOVE_AT current ${idx})

    set_property(TARGET "${target}" PROPERTY "${property}" ${current})
endfunction()

function(replace_in_target_property target property old_value new_value)
    if(NOT TARGET "${target}")
        message(WARNING "replace_in_target_property: Target '${target}' does not exist")
        return()
    endif()

    get_target_property(current "${target}" "${property}")

    if(NOT current)
        # Property not set or empty → nothing to do
        return()
    endif()

    list(FIND current "${old_value}" idx)
    if(idx EQUAL -1)
        # Not found → no change
        return()
    endif()

    list(REMOVE_AT current ${idx})

    if(NOT "${new_value}" STREQUAL "")
        list(INSERT current ${idx} "${new_value}")
        # or: list(APPEND current "${new_value}")  # ← if order doesn't matter
    endif()

    set_property(TARGET "${target}" PROPERTY "${property}" ${current})

    # Optional: show what happened (useful during debugging)
    # message(STATUS "Replaced '${old_value}' → '${new_value}' in ${property} of ${target}")
endfunction()

function(print_target_link_libraries tgt)
    if(NOT TARGET ${tgt})
        message(WARNING "print_target_link_libraries: ${tgt} is not a target")
        return()
    endif()

    include(CMakePrintHelpers)

    message(STATUS "")
    message(STATUS "Link information for target: ${tgt}")
    message(STATUS "----------------------------------------")

    cmake_print_properties(
        TARGETS ${tgt}
        PROPERTIES
            LINK_LIBRARIES
            INTERFACE_LINK_LIBRARIES
            LINK_OPTIONS
            INTERFACE_LINK_OPTIONS
    )

    get_target_property(libs ${tgt} LINK_LIBRARIES)
    if(libs)
        message(STATUS "Direct libraries:")
        foreach(lib IN LISTS libs)
            message(STATUS "  ${lib}")
        endforeach()
    endif()

    # Optional: try to resolve to final linker-style names (approximate)
    message(STATUS "Final link line approximation (not exact):")
    foreach(lib IN LISTS libs)
        if(TARGET ${lib})
            # It's a CMake target → would be resolved transitively
            message(STATUS "  <target ${lib}>")
        elseif(IS_ABSOLUTE "${lib}")
            # Full path
            message(STATUS "  ${lib}")
        elseif(lib MATCHES "^-.+")
            # Flag
            message(STATUS "  ${lib}")
        else()
            # -lxxx style
            message(STATUS "  -l${lib}")
        endif()
    endforeach()
endfunction()

project(libbitcoin-system
  VERSION 4.0.0
  DESCRIPTION "Bitcoin Cross-Platform C++ Development Toolkit - base library"
  LANGUAGES C CXX
)

set(Boost_VERBOSE true)

list( APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules" )

set( CANONICAL_LIBRARY system )

if (MSVC)
  set( CANONICAL_NAMESPACE libbitcoin )
else ()
  set( CANONICAL_NAMESPACE bitcoin )
endif ()

set( CANONICAL_SIMPLE_NAME ${CANONICAL_NAMESPACE}-${CANONICAL_LIBRARY} )
set( CANONICAL_PREFIX ${CANONICAL_NAMESPACE}_${CANONICAL_LIBRARY} )
set( CANONICAL_NAMESPACED_NAME ${CANONICAL_NAMESPACE}::${CANONICAL_LIBRARY} )

add_library( ${CANONICAL_SIMPLE_NAME} )
add_library( ${CANONICAL_NAMESPACED_NAME} ALIAS ${CANONICAL_SIMPLE_NAME} )

# ──────────────────────────────────────────────────────────────
#   Options
# ─────────────────────────────────────────────────────────────

#option(BUILD_SHARED_LIBS "Build shared library instead of static" OFF)
option(with-tests "Build unit tests" ON)
option(with-examples "Build examples" ON)
option(with-icu "Compile with International Components for Unicode." OFF)

set(LIBBITCOIN_CXX_STANDARD 20 CACHE STRING "Require C++ 20 standard.")

# ──────────────────────────────────────────────────────────────
#   Dependencies
# ──────────────────────────────────────────────────────────────

# Optional – comment out if you don't need mnemonic / BIP-39 support
if (with-icu)
  #  find_package( Icu-I18N 55.2 REQUIRED )
  set(ICU_USE_STATIC_LIBS ON)
  find_package( ICU 55.2 REQUIRED
    COMPONENTS uc i18n data
  )
endif()

# secp256k1 – in practice often built from source or via vcpkg/conan
# Here we assume it's already available as find_package target
# (adjust according to your environment)
find_package(libsecp256k1 0.7.0 REQUIRED)   # ← most common today

find_package(Boost 1.86.0 REQUIRED
  COMPONENTS
    iostreams
    locale
    program_options
    thread
    url
    unit_test_framework
)

print_target_link_libraries(Boost::locale)

replace_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icudata ICU::data )
replace_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icui18n ICU::i18n )
replace_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icuuc ICU::uc )
#remove_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icudata )
#remove_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icui18n )
#remove_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icuuc )

# ──────────────────────────────────────────────────────────────
#   Library definition
# ──────────────────────────────────────────────────────────────

file(GLOB_RECURSE ${CANONICAL_PREFIX}_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/bitcoin/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/bitcoin/*.ipp"
)

file( GLOB_RECURSE ${CANONICAL_PREFIX}_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.c"
)

target_sources( ${CANONICAL_SIMPLE_NAME} PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
  FILES ${${CANONICAL_PREFIX}_HEADERS}
)

target_sources( ${CANONICAL_SIMPLE_NAME} PRIVATE
  ${${CANONICAL_PREFIX}_SOURCES}
)

target_include_directories( ${CANONICAL_NAMESPACE}-${CANONICAL_LIBRARY}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_include_directories( ${CANONICAL_NAMESPACE}-${CANONICAL_LIBRARY}
  PRIVATE
    ${icu_i18n_INCLUDE_DIRS}
)

target_compile_features(${CANONICAL_SIMPLE_NAME} PUBLIC cxx_std_${LIBBITCOIN_CXX_STANDARD})

target_compile_definitions(${CANONICAL_SIMPLE_NAME}
  PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:BC_DLL>
    # Add other internal defines if needed, e.g. BC_USE_ICU
)

print_target_link_libraries(libsecp256k1::secp256k1)
print_target_link_libraries(Boost::iostreams)
print_target_link_libraries(Boost::locale)
print_target_link_libraries(Boost::program_options)
print_target_link_libraries(Boost::thread)
print_target_link_libraries(Boost::url)

target_link_libraries(${CANONICAL_SIMPLE_NAME}
  PUBLIC
    Boost::iostreams
    Boost::locale
    Boost::program_options
    Boost::thread
    Boost::url
    ICU::i18n
    ICU::uc
    ICU::data
    libsecp256k1::secp256k1
)

print_target_link_libraries(${CANONICAL_NAMESPACED_NAME})

# Very common flags for libbitcoin projects
target_compile_options(${CANONICAL_SIMPLE_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive- /W3>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

set_target_properties(${CANONICAL_SIMPLE_NAME} PROPERTIES
  VERSION   ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR}
  EXPORT_NAME system
)

# ──────────────────────────────────────────────────────────────
#   Tests
# ──────────────────────────────────────────────────────────────

if(with-tests)
  enable_testing()

  file(GLOB_RECURSE ${CANONICAL_PREFIX}_test_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../test/*.cpp"
  )

  add_executable( ${CANONICAL_SIMPLE_NAME}-test )

  target_sources( ${CANONICAL_SIMPLE_NAME}-test PRIVATE
    ${${CANONICAL_PREFIX}_test_SOURCES} )

  target_link_libraries( ${CANONICAL_SIMPLE_NAME}-test PRIVATE
    ${CANONICAL_NAMESPACED_NAME}
    Boost::unit_test_framework
  )

  target_include_directories( ${CANONICAL_SIMPLE_NAME}-test PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
  )

  add_test( NAME ${CANONICAL_SIMPLE_NAME}-test COMMAND ${CANONICAL_SIMPLE_NAME}-test
    --run_test=*
    --log_level=warning
    --show_progress=no
    --detect_memory_leak=0
    --report_level=no
    --build_info=yes
  )

endif()

# ──────────────────────────────────────────────────────────────
#   Examples (optional)
# ──────────────────────────────────────────────────────────────

if(LIBBITCOIN_WITH_EXAMPLES)
  file(GLOB example_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp")
  foreach(ex_file IN LISTS example_SOURCES)
    get_filename_component(ex_name "${ex_file}" NAME_WE)
    add_executable(${ex_name} ${ex_file})
    target_link_libraries(${ex_name} PRIVATE ${CANONICAL_SIMPLE_NAME})
  endforeach()
endif()

# ──────────────────────────────────────────────────────────────
#   Installation & CMake package config
# ──────────────────────────────────────────────────────────────

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ${CANONICAL_SIMPLE_NAME}
  EXPORT ${CANONICAL_SIMPLE_NAME}Targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILE_SET HEADERS DESTINATION include
)

#install(DIRECTORY include
#  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#  FILES_MATCHING
#    PATTERN "*.hpp"
#    PATTERN "*.ipp"
#)

# Generate and install version file
#write_basic_package_version_file(
#  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig-version.cmake"
#  VERSION       ${PROJECT_VERSION}
#  COMPATIBILITY SameMajorVersion
#)

# Generate config file from template (see below)
#configure_package_config_file(
#  "${CMAKE_CURRENT_SOURCE_DIR}/Bitcoin-SystemConfig.cmake.in"
#  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig.cmake"
#  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CANONICAL_SIMPLE_NAME}
#)

#install(EXPORT ${CANONICAL_SIMPLE_NAME}Targets
#  FILE ${CANONICAL_SIMPLE_NAME}-targets.cmake
#  NAMESPACE libbitcoin::
#  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CANONICAL_SIMPLE_NAME}
#)

#install(FILES
#  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig.cmake"
#  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig-version.cmake"
#  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CANONICAL_SIMPLE_NAME}
#)
