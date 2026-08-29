cmake_minimum_required(VERSION 4.4.2)

project(
    chat-over-ssl-tui
    DESCRIPTION "CHAT over ssl tui"
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSION OFF)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/boost.Asio/include/)
include_directories(${CMAKE_SOURCE_DIR}/include/boost_1_92_0)
include_directories(${CMAKE_SOURCE_DIR}/src)


set(SERVER ${CMAKE_SOURCE_DIR}/src/server/main.cpp)
set(CLIENT ${CMAKE_SOURCE_DIR}/src/client/main.cpp)

add_executable(server ${SERVER})
add_executable(client ${CLIENT})

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/ftxui)

target_link_libraries(
  client
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

