cmake_minimum_required(VERSION 3.22...3.30)

project(libbitcoin-system
  VERSION 4.0.0
  DESCRIPTION "Bitcoin Cross-Platform C++ Development Toolkit - base library"
  LANGUAGES CXX
)

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

find_package(Boost 1.86.0 REQUIRED
  COMPONENTS
    iostreams
    locale
    program_options
    thread
    url
    unit_test_framework
)

# Optional – comment out if you don't need mnemonic / BIP-39 support
if (with-icu)
    find_package( Icu-I18N 55.2 REQUIRED )
endif()

# secp256k1 – in practice often built from source or via vcpkg/conan
# Here we assume it's already available as find_package target
# (adjust according to your environment)
find_package(libsecp256k1 0.7.0 REQUIRED)   # ← most common today

# ──────────────────────────────────────────────────────────────
#   Library definition
# ──────────────────────────────────────────────────────────────

file(GLOB_RECURSE ${CANONICAL_PREFIX}_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/bitcoin/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/bitcoin/*.ipp"
)

file(GLOB_RECURSE ${CANONICAL_PREFIX}_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.cpp"
)

add_library( ${CANONICAL_SIMPLE_NAME}
  ${${CANONICAL_PREFIX}_SOURCES}
  ${${CANONICAL_PREFIX}_HEADERS}
)

add_library( ${CANONICAL_NAMESPACED_NAME} ALIAS ${CANONICAL_SIMPLE_NAME} )

target_include_directories( ${CANONICAL_NAMESPACE}-${CANONICAL_LIBRARY}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(${CANONICAL_SIMPLE_NAME} PUBLIC cxx_std_${LIBBITCOIN_CXX_STANDARD})

target_compile_definitions(${CANONICAL_SIMPLE_NAME}
  PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:BC_DLL>
    # Add other internal defines if needed, e.g. BC_USE_ICU
)

target_link_libraries(${CANONICAL_SIMPLE_NAME}
  PUBLIC
    Boost::iostreams
    Boost::locale
    Boost::program_options
    Boost::thread
    Boost url
  PRIVATE
    $<$<TARGET_EXISTS:libsecp256k1>:libsecp256k1::libsecp256k1>
    $<$<TARGET_EXISTS:PkgConfig::ICU>:PkgConfig::ICU>
)

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

if(LIBBITCOIN_WITH_TESTS)
  enable_testing()

  file(GLOB_RECURSE test_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")

  foreach(test_file IN LISTS test_SOURCES)
    get_filename_component(test_name "${test_file}" NAME_WE)
    add_executable(${test_name} ${test_file})
    target_link_libraries(${test_name} PRIVATE
      ${CANONICAL_SIMPLE_NAME}
      Boost::unit_test_framework
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
  endforeach()
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
)

install(DIRECTORY include
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.ipp"
)

# Generate and install version file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig-version.cmake"
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Generate config file from template (see below)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Bitcoin-SystemConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CANONICAL_SIMPLE_NAME}
)

#install(EXPORT ${CANONICAL_SIMPLE_NAME}Targets
#  FILE ${CANONICAL_SIMPLE_NAME}-targets.cmake
#  NAMESPACE libbitcoin::
#  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CANONICAL_SIMPLE_NAME}
#)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/Bitcoin-SystemConfig-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CANONICAL_SIMPLE_NAME}
)
