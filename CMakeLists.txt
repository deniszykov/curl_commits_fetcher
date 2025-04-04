cmake_minimum_required(VERSION 3.10)
project(CurlCommits VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(CURL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Add executable
add_executable(curl_commits src/curl_commits.cpp)

# Link libraries
target_link_libraries(curl_commits PRIVATE 
    CURL::libcurl 
    nlohmann_json::nlohmann_json
)

# Install target
install(TARGETS curl_commits DESTINATION bin)