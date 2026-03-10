###############################################################################
#  Copyright (c) 2014-2026 libbitcoin-system developers (see COPYING).
#
#         GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
#
###############################################################################
cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project( libbitcoin-system
  VERSION 4.0.0
  DESCRIPTION "Bitcoin Cross-Platform C++ Development Toolkit"
  LANGUAGES C CXX )

list( APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules" )

if ( MSVC )
  set( CMAKE_STATIC_LIBRARY_PREFIX "lib" )
  set( CMAKE_SHARED_LIBRARY_PREFIX "lib" )
endif()

#------------------------------------------------------------------------------
# Project options.
#------------------------------------------------------------------------------
option( with-icu "Build using ICU libraries." OFF )
option( enable-avx2 "Use Intel AVX2 intrinsics." OFF )
option( enable-avx512 "Use Intel AVX512 intrinsics." OFF )
option( enable-sse41 "Use SSE4.1 hardware instructions." OFF )
option( enable-shani "Use Intel SHA Extensions." OFF )
option( with-test "Build test." ON )
option( with-examples "Build examples." ON )

#------------------------------------------------------------------------------
# Dependencies.
#------------------------------------------------------------------------------

if ( BUILD_SHARED_LIBS )
  set( Boost_USE_STATIC_LIBS "OFF" )
  set( ICU_USE_STATIC_LIBS "OFF" )
  set( libsecp256k1_USE_STATIC_LIBS "OFF" )
else()
  set( Boost_USE_STATIC_LIBS "ON" )
  set( ICU_USE_STATIC_LIBS "ON" )
  set( libsecp256k1_USE_STATIC_LIBS "ON" )
endif()

find_package( Boost 1.86.0 REQUIRED
  COMPONENTS
    iostreams
    locale
    program_options
    thread
    url
    unit_test_framework
)

if ( with-icu )
  find_package( ICU 55.2 REQUIRED
    COMPONENTS
      i18n
      uc
      data
  )
endif()

find_package( libsecp256k1 0.7.0 REQUIRED )

if ( with-icu )
  include( InTargetProperty )
  remove_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icudata )
  remove_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icui18n )
  remove_in_target_property( Boost::locale INTERFACE_LINK_LIBRARIES icuuc )
endif()
#------------------------------------------------------------------------------
# Compiler options.
#------------------------------------------------------------------------------

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)

if ( enable-avx2 )
  set( CMAKE_REQUIRED_FLAGS_PREV "${CMAKE_REQUIRED_FLAGS}" )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -mavx -mavx2" )
  check_cxx_source_compiles( "
    #include <stdint.h>
    #include <immintrin.h>
      int main() {
      __m256i a = _mm256_set_epi32(8, 7, 6, 5, 4, 3, 2, 1);
      return _mm256_extract_epi32(_mm256_slli_epi64(a, 2), 5);
      }" SUPPORTS_FLAG_AVX2 )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_PREV}" )
  if ( !SUPPORTS_FLAG_AVX2 )
    message( FATAL_ERROR "Compiler does not support '-mavx -mavx2'." )
  endif()
endif()

if ( enable-avx512 )
  set( CMAKE_REQUIRED_FLAGS_PREV "${CMAKE_REQUIRED_FLAGS}" )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -mavx512f -mavx512bw" )
  check_cxx_source_compiles( "
    #include <stdint.h>
    #include <immintrin.h>
      int main() {
      __m512i a = _mm512_set_epi32(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
      return _mm_extract_epi32(_mm512_extracti32x4_epi32(_mm512_slli_epi64(a, 2), 2), 1);
      }" SUPPORTS_FLAG_AVX512 )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_PREV}" )
  if ( !SUPPORTS_FLAG_AVX512 )
    message( FATAL_ERROR "Compiler does not support '-mavx512f -mavx512bw'." )
  endif()
endif()

if ( enable-shani )
  set( CMAKE_REQUIRED_FLAGS_PREV "${CMAKE_REQUIRED_FLAGS}" )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -msse4 -msha" )
  check_cxx_source_compiles( "
    #include <stdint.h>
    #include <immintrin.h>
      int main() {
      __m128i a = _mm_set1_epi32(0);
      __m128i b = _mm_set1_epi32(15);
      __m128i k = _mm_set1_epi32(31);
      return _mm_extract_epi32(_mm_sha256msg2_epu32(_mm_sha256msg1_epu32(_mm_sha256rnds2_epu32(a, b, k), b), a), 2);
      }" SUPPORTS_FLAG_SHANI )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_PREV}" )
  if ( !SUPPORTS_FLAG_SHANI )
    message( FATAL_ERROR "Compiler does not support '-msse4 -msha'." )
  endif()
endif()

if ( enable-sse41 )
  set( CMAKE_REQUIRED_FLAGS_PREV "${CMAKE_REQUIRED_FLAGS}" )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -msse4.1" )
  check_cxx_source_compiles( "
    #include <stdint.h>
    #include <immintrin.h>
      int main() {
      __m128i a = _mm_set1_epi32(0);
      __m128i b = _mm_set1_epi32(15);
      return _mm_extract_epi32(_mm_add_epi64(a, b), 2);
      }" SUPPORTS_FLAG_SSE41 )
  set( CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS_PREV}" )
  if ( !SUPPORTS_FLAG_SSE41 )
    message( FATAL_ERROR "Compiler does not support '-msse4.1'." )
  endif()
endif()

check_cxx_compiler_flag( "-Wall" HAS_FLAG_WALL )
if ( !HAS_FLAG_WALL )
    message( FATAL_ERROR "Compiler does not support '-Wall'." )
endif()

check_cxx_compiler_flag( "-Wextra" HAS_FLAG_WEXTRA )
if ( !HAS_FLAG_WEXTRA )
    message( FATAL_ERROR "Compiler does not support '-Wextra'." )
endif()

check_cxx_compiler_flag( "-Wno-reorder" HAS_FLAG_WNO-REORDER )
if ( !HAS_FLAG_WNO-REORDER )
    message( FATAL_ERROR "Compiler does not support '-Wno-reorder'." )
endif()

check_cxx_compiler_flag( "-Wno-missing-field-initializers" HAS_FLAG_WNO-MISSING-FIELD-INITIALIZERS )
if ( !HAS_FLAG_WNO-MISSING-FIELD-INITIALIZERS )
    message( FATAL_ERROR "Compiler does not support '-Wno-missing-field-initializers'." )
endif()

check_cxx_compiler_flag( "-Wno-missing-braces" HAS_FLAG_WNO-MISSING-BRACES )
if ( !HAS_FLAG_WNO-MISSING-BRACES )
    message( FATAL_ERROR "Compiler does not support '-Wno-missing-braces'." )
endif()

check_cxx_compiler_flag( "-Wno-comment" HAS_FLAG_WNO-COMMENT )
if ( !HAS_FLAG_WNO-COMMENT )
    message( FATAL_ERROR "Compiler does not support '-Wno-comment'." )
endif()

check_cxx_compiler_flag( "-Wno-deprecated-copy" HAS_FLAG_WNO-DEPRECATED-COPY )
if ( !HAS_FLAG_WNO-DEPRECATED-COPY )
    message( FATAL_ERROR "Compiler does not support '-Wno-deprecated-copy'." )
endif()

check_cxx_compiler_flag( "-Wno-ignored-attributes" HAS_FLAG_WNO-IGNORED-ATTRIBUTES )
if ( !HAS_FLAG_WNO-IGNORED-ATTRIBUTES )
    message( FATAL_ERROR "Compiler does not support '-Wno-ignored-attributes'." )
endif()

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" )
  check_cxx_compiler_flag( "-Wno-mismatched-tags" HAS_FLAG_WNO-MISMATCHED-TAGS )
  if ( !HAS_FLAG_WNO-MISMATCHED-TAGS )
      message( FATAL_ERROR "Compiler does not support '-Wno-mismatched-tags'." )
  endif()
endif()

check_cxx_compiler_flag( "-Wno-long-long" HAS_FLAG_WNO-LONG-LONG )
if ( !HAS_FLAG_WNO-LONG-LONG )
    message( FATAL_ERROR "Compiler does not support '-Wno-long-long'." )
endif()

if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  check_cxx_compiler_flag( "-fno-var-tracking-assignments" HAS_FLAG_FNO-VAR-TRACKING-ASSIGNMENTS )
  if ( !HAS_FLAG_FNO-VAR-TRACKING-ASSIGNMENTS )
      message( FATAL_ERROR "Compiler does not support '-fno-var-tracking-assignments'." )
  endif()
endif()

#------------------------------------------------------------------------------
# bitcoin-system library
#------------------------------------------------------------------------------
add_library( bitcoin-system )
add_library( bitcoin::system ALIAS bitcoin-system )

target_compile_features( bitcoin-system
  PUBLIC
    cxx_std_20
)

target_compile_options( bitcoin-system
  PUBLIC
    $<$<BOOL:${enable-avx2}>:-mavx -mavx2>
    $<$<BOOL:${enable-avx512}>:-mavx512f -mavx512bw>
    $<$<BOOL:${enable-shani}>:-msse4 -msha>
    $<$<BOOL:${enable-sse41}>:-msse4.1>
  PRIVATE
    -Wall
    -Wextra
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-reorder>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-missing-field-initializers>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-missing-braces>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-comment>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-copy>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-ignored-attributes>
    $<$<CXX_COMPILER_ID:Clang>:-Wno-mismatched-tags>
    $<$<COMPILE_LANGUAGE:CXX>:-Wno-long-long>
    $<$<CXX_COMPILER_ID:GNU>:-fno-var-tracking-assignments>
)

target_compile_definitions( bitcoin-system
  PUBLIC
    $<$<BOOL:${with-icu}>:WITH_ICU>
)

file( GLOB_RECURSE bitcoin_system_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../include/*.ipp"
)

file( GLOB_RECURSE bitcoin_system_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/../../src/*.cpp"
)

target_sources( bitcoin-system
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
    FILES
      ${bitcoin_system_HEADERS}
  PRIVATE
    ${bitcoin_system_SOURCES}
)

target_include_directories( bitcoin-system
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries( bitcoin-system
  PUBLIC
    Boost::iostreams
    Boost::locale
    Boost::program_options
    Boost::thread
    Boost::url
    $<$<BOOL:${with-icu}>:ICU::i18n>
    $<$<BOOL:${with-icu}>:ICU::uc>
    $<$<BOOL:${with-icu}>:ICU::data>
    libsecp256k1::secp256k1
)

set_target_properties(bitcoin-system
  PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME system
)

#------------------------------------------------------------------------------
# bitcoin-system-test tests
#------------------------------------------------------------------------------
if ( with-test )
  add_executable( bitcoin-system-test )

  enable_testing()

  add_test( NAME bitcoin-system-test
    COMMAND bitcoin-system-test
    --run_test=*
    --log_level=warning
    --show_progress=no
    --detect_memory_leak=0
    --report_level=no
    --build_info=yes
  )

  file( GLOB_RECURSE bitcoin_system_test_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../test/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../test/*.cpp"
  )

  target_sources( bitcoin-system-test
    PRIVATE
      ${bitcoin_system_test_SOURCES}
  )

  target_link_libraries( bitcoin-system-test
    PRIVATE
      Boost::unit_test_framework
      bitcoin::system
  )
endif()

#------------------------------------------------------------------------------
# examples executable
#------------------------------------------------------------------------------
if ( with-examples )
  add_executable( examples )

  file( GLOB_RECURSE examples_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../examples/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../examples/*.cpp"
  )

  target_sources( examples
    PRIVATE
      ${examples_SOURCES}
  )

  target_link_libraries( examples
    PRIVATE
      bitcoin::system
  )
endif()
#------------------------------------------------------------------------------
# Installation routine.
#------------------------------------------------------------------------------
include( GNUInstallDirs )
include( CMakePackageConfigHelpers )

install( TARGETS bitcoin-system
  EXPORT bitcoin-system-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILE_SET HEADERS
)

install( EXPORT bitcoin-system-targets
  NAMESPACE bitcoin::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bitcoin-system
  FILE bitcoin-system-targets.cmake
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/bitcoin-system-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/bitcoin-system-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bitcoin-system
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/bitcoin-system-config-version.cmake"
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/bitcoin-system-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/bitcoin-system-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bitcoin-system
)
