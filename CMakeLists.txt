cmake_minimum_required(VERSION 3.5)

project(PIIScanner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SOURCE_DIR src)
set(INCLUDE_DIR include)


add_executable(PIIScanner
    ${SOURCE_DIR}/main.cpp
    ${SOURCE_DIR}/PIIScanner.cpp
    ${INCLUDE_DIR}/PIIScanner.h
    ${INCLUDE_DIR}/FileReader.h
    ${INCLUDE_DIR}/PIIStats.h
    src/PIIConfigger.cpp
    include/PIIConfigger.h
)

target_include_directories(PIIScanner PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${INCLUDE_DIR}
    ${LLAMA_DIR}/include
)

find_package(nlohmann_json CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER REQUIRED IMPORTED_TARGET poppler-cpp)
find_package(cxxopts REQUIRED)

target_link_libraries(PIIScanner PRIVATE
    nlohmann_json::nlohmann_json
    PkgConfig::POPPLER
    cxxopts::cxxopts
)

include(GNUInstallDirs)
install(TARGETS PIIScanner
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
