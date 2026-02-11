cmake_minimum_required(VERSION 3.22...3.30)

project(libbitcoin-system
  VERSION 3.8.0
  DESCRIPTION "Bitcoin Cross-Platform C++ Development Toolkit - base library"
  LANGUAGES CXX
)

# ──────────────────────────────────────────────────────────────
#   Options
# ──────────────────────────────────────────────────────────────

option(BUILD_SHARED_LIBS "Build shared library instead of static" OFF)
option(LIBBITCOIN_WITH_TESTS "Build unit tests" ON)
option(LIBBITCOIN_WITH_EXAMPLES "Build examples" ON)

set(LIBBITCOIN_CXX_STANDARD 17 CACHE STRING "C++ standard to use (17, 20, 23)")
set_property(CACHE LIBBITCOIN_CXX_STANDARD PROPERTY STRINGS 17 20 23)

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
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(ICU ucs4 IMPORTED_TARGET GLOBAL)
endif()

# secp256k1 – in practice often built from source or via vcpkg/conan
# Here we assume it's already available as find_package target
# (adjust according to your environment)
find_package(secp256k1 QUIET)   # ← most common today

# ──────────────────────────────────────────────────────────────
#   Library definition
# ──────────────────────────────────────────────────────────────

file(GLOB_RECURSE libbitcoin_system_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/bitcoin/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/bitcoin/*.ipp"
)

file(GLOB_RECURSE libbitcoin_system_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(libbitcoin-system ${libbitcoin_system_SOURCES} ${libbitcoin_system_HEADERS})

add_library(libbitcoin::system ALIAS libbitcoin-system)

target_include_directories(libbitcoin-system
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(libbitcoin-system PUBLIC cxx_std_${LIBBITCOIN_CXX_STANDARD})

target_compile_definitions(libbitcoin-system
  PRIVATE
    $<$<BOOL:${BUILD_SHARED_LIBS}>:BC_DLL>
    # Add other internal defines if needed, e.g. BC_USE_ICU
)

target_link_libraries(libbitcoin-system
  PUBLIC
    Boost::atomic Boost::chrono Boost::date_time
    Boost::filesystem Boost::iostreams Boost::locale
    Boost::log Boost::program_options Boost::regex
    Boost::system Boost::thread
  PRIVATE
    $<$<TARGET_EXISTS:secp256k1>:secp256k1::secp256k1>
    $<$<TARGET_EXISTS:PkgConfig::ICU>:PkgConfig::ICU>
)

# Very common flags for libbitcoin projects
target_compile_options(libbitcoin-system PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/permissive- /W3>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

set_target_properties(libbitcoin-system PROPERTIES
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
      libbitcoin-system
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
    target_link_libraries(${ex_name} PRIVATE libbitcoin-system)
  endforeach()
endif()

# ──────────────────────────────────────────────────────────────
#   Installation & CMake package config
# ──────────────────────────────────────────────────────────────

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS libbitcoin-system
  EXPORT libbitcoin-systemTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/bitcoin
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.ipp"
)

# Generate and install version file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/libbitcoin-system-config-version.cmake"
  VERSION       ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

# Generate config file from template (see below)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/libbitcoin-system-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/libbitcoin-system-config.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libbitcoin-system
)

install(EXPORT libbitcoin-systemTargets
  FILE libbitcoin-system-targets.cmake
  NAMESPACE libbitcoin::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libbitcoin-system
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/libbitcoin-system-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/libbitcoin-system-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libbitcoin-system
)

# Optional: pkg-config file (many users still expect it)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/libbitcoin-system.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/libbitcoin-system.pc"
  @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libbitcoin-system.pc"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
