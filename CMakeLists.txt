cmake_minimum_required(VERSION 3.15)
project(PIIScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SOURCE_DIR src)
set(INCLUDE_DIR include)

add_executable(PIIScanner
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/PIIScanner.cpp
    ${SOURCE_DIR}/PIIConfigger.cpp
    ${SOURCE_DIR}/PatternRegistry.cpp
    ${SOURCE_DIR}/PIIResultHandler.cpp
    ${SOURCE_DIR}/clp.cpp

    ${INCLUDE_DIR}/clp.h
    ${INCLUDE_DIR}/PIIScanner.h
    ${INCLUDE_DIR}/FileReader.h
    ${INCLUDE_DIR}/PatternRegistry.h
    ${INCLUDE_DIR}/PIIConfigger.h
    ${INCLUDE_DIR}/ScanConfig.h
    ${INCLUDE_DIR}/PIIResultHandler.h
    ${INCLUDE_DIR}/PIIGeneralStats.h
)

target_include_directories(PIIScanner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_DIR}
)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(RE2 CONFIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)

target_link_libraries(PIIScanner PRIVATE
    nlohmann_json::nlohmann_json
    cxxopts::cxxopts
    PkgConfig::POPPLER_CPP
    re2::re2
)

include(GNUInstallDirs)
install(TARGETS PIIScanner
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
