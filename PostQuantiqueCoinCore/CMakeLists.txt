cmake_minimum_required(VERSION 3.20)
project(PostQuantiqueCoinCore VERSION 0.2.0 LANGUAGES CXX)

option(USE_LIBOQS "Enable Open Quantum Safe liboqs provider" ON)
option(DEV_ONLY_ALLOW_INSECURE_CRYPTO "Allow insecure development fallback crypto" OFF)
option(PQC_BUILD_TESTS "Build PostQuantiqueCoin tests" ON)
option(PQC_BUILD_TOOLS "Build PostQuantiqueCoin CLI tools" ON)
option(PQC_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(PQC_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(PQC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(PQC_ENABLE_COVERAGE "Enable coverage flags" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /permissive- /EHsc)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()
if(PQC_WARNINGS_AS_ERRORS)
  if(MSVC)
    add_compile_options(/WX)
  else()
    add_compile_options(-Werror)
  endif()
endif()
if(PQC_ENABLE_ASAN AND NOT MSVC)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()
if(PQC_ENABLE_UBSAN AND NOT MSVC)
  add_compile_options(-fsanitize=undefined)
  add_link_options(-fsanitize=undefined)
endif()
if(PQC_ENABLE_COVERAGE AND NOT MSVC)
  add_compile_options(--coverage)
  add_link_options(--coverage)
endif()

if(USE_LIBOQS)
  find_path(LIBOQS_INCLUDE_DIR oqs/oqs.h)
  find_library(LIBOQS_LIBRARY oqs)
  if(NOT LIBOQS_INCLUDE_DIR OR NOT LIBOQS_LIBRARY)
    message(FATAL_ERROR "liboqs not found. Install liboqs or configure with -DUSE_LIBOQS=OFF -DDEV_ONLY_ALLOW_INSECURE_CRYPTO=ON only for local development builds.")
  endif()
  add_compile_definitions(PQC_USE_LIBOQS=1)
else()
  if(NOT DEV_ONLY_ALLOW_INSECURE_CRYPTO)
    message(FATAL_ERROR "Production builds require liboqs. For local tests only, use -DUSE_LIBOQS=OFF -DDEV_ONLY_ALLOW_INSECURE_CRYPTO=ON.")
  endif()
  add_compile_definitions(PQC_DEV_ONLY_ALLOW_INSECURE_CRYPTO=1)
endif()

file(GLOB_RECURSE PQC_LIBRARY_SOURCES CONFIGURE_DEPENDS
  src/core/*.cpp
  src/crypto/*.cpp
  src/blockchain/*.cpp
  src/storage/*.cpp
  src/wallet/*.cpp
  src/mining/*.cpp
  src/cli/*.cpp
  src/p2p/*.cpp
)
add_library(pqc_core STATIC ${PQC_LIBRARY_SOURCES})
target_include_directories(pqc_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(USE_LIBOQS)
  target_include_directories(pqc_core PRIVATE ${LIBOQS_INCLUDE_DIR})
  target_link_libraries(pqc_core PRIVATE ${LIBOQS_LIBRARY})
endif()
if(WIN32)
  target_link_libraries(pqc_core PUBLIC ws2_32 bcrypt)
endif()

if(PQC_BUILD_TESTS)
  add_executable(pqc_tests src/main_tests.cpp)
  target_link_libraries(pqc_tests PRIVATE pqc_core)
endif()
if(PQC_BUILD_TOOLS)
  add_executable(pqc_node src/main_node.cpp)
  target_link_libraries(pqc_node PRIVATE pqc_core)
  add_executable(pqc_wallet src/main_wallet.cpp)
  target_link_libraries(pqc_wallet PRIVATE pqc_core)
  add_executable(pqc_miner src/main_miner.cpp)
  target_link_libraries(pqc_miner PRIVATE pqc_core)
endif()

include(CTest)
if(PQC_BUILD_TESTS)
  add_test(NAME pqc_tests COMMAND pqc_tests)
endif()
install(TARGETS pqc_core
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
if(PQC_BUILD_TOOLS)
  install(TARGETS pqc_node pqc_wallet pqc_miner RUNTIME DESTINATION bin)
endif()
install(DIRECTORY include/ DESTINATION include)
