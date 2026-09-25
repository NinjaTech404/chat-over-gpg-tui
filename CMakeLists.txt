cmake_minimum_required(VERSION 3.8...4.4.2)

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
include_directories(${CMAKE_SOURCE_DIR}/include/asio) # asio-1.38.2
include_directories(${CMAKE_SOURCE_DIR}/src)


set(SERVER ${CMAKE_SOURCE_DIR}/src/server/main.cpp)
set(CLIENT ${CMAKE_SOURCE_DIR}/src/client/main.cpp)

add_executable(server ${SERVER})
add_executable(client ${CLIENT})

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/ftxui)
add_subdirectory(${CMAKE_SOURCE_DIR}/lib/fmt) # fmt-12.2.0

find_package(Gpgmepp REQUIRED)

target_link_libraries(
  client
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE fmt::fmt
  Gpgmepp
)

target_link_libraries(
  server
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE fmt::fmt
  Gpgmepp
)


