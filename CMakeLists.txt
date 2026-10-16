cmake_minimum_required(VERSION 3.10...4.4.2)

# >=====> Project Logo <=====<

include(${CMAKE_SOURCE_DIR}/cmake/logo.cmake)


# >=====> Project Info <=====<

set(VERSION 0.1.0) # => Default Version

project(
    cyber-chat
    DESCRIPTION "CHAT over ssl tui"
    VERSION ${VERSION}
    LANGUAGES CXX
)


# >=====> Compiler Configurations <=====<

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSION OFF)


# >=====> Linker Configurations <=====<

set(ENABLE_SHARED OFF)
set(ENABLE_STATIC ON)


# >=====> Static Libraries Configurations <=====<
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")

# => FTXUI 7.0.3
set(ftxui_component ${CMAKE_SOURCE_DIR}/lib/ftxui/lib/libftxui-component.a) # => ftxui component
set(ftxui_dom       ${CMAKE_SOURCE_DIR}/lib/ftxui/lib/libftxui-dom.a      ) # => ftxui dom
set(ftxui_screen    ${CMAKE_SOURCE_DIR}/lib/ftxui/lib/libftxui-screen.a   ) # => ftxui screen

# => GPGME++ 2.2.0
set(Gpgmepp   ${CMAKE_SOURCE_DIR}/lib/gpgmepp/lib/libgpgmepp.a    ) # => gpgmepp-2.2.0
set(Gpgme     ${CMAKE_SOURCE_DIR}/lib/gpgme/lib/libgpgme.a        ) # => gpgme-2.2.0
set(Gpg_error ${CMAKE_SOURCE_DIR}/lib/gpg-error/lib/libgpg-error.a) # => gpg-error-3.0.2
set(Assuan    ${CMAKE_SOURCE_DIR}/lib/assuan/lib/libassuan.a      ) # => assuan-1.61

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")

# => FTXUI 7.0.3
set(ftxui_component ${CMAKE_SOURCE_DIR}/aarch64/ftxui/lib/libftxui-component.a) # => ftxui component
set(ftxui_dom       ${CMAKE_SOURCE_DIR}/aarch64/ftxui/lib/libftxui-dom.a      ) # => ftxui dom
set(ftxui_screen    ${CMAKE_SOURCE_DIR}/aarch64/ftxui/lib/libftxui-screen.a   ) # => ftxui screen

# => GPGME++ 2.2.0
set(Gpgmepp   ${CMAKE_SOURCE_DIR}/aarch64/gpgmepp/lib/libgpgmepp.a    ) # => gpgmepp-2.2.0
set(Gpgme     ${CMAKE_SOURCE_DIR}/aarch64/gpgme/lib/libgpgme.a        ) # => gpgme-2.2.0
set(Gpg_error ${CMAKE_SOURCE_DIR}/aarch64/gpg-error/lib/libgpg-error.a) # => gpg-error-3.0.2
set(Assuan    ${CMAKE_SOURCE_DIR}/aarch64/assuan/lib/libassuan.a      ) # => assuan-1.61

endif()

# >=====> Header Files Configurations <=====<

include_directories(${CMAKE_SOURCE_DIR}/lib                  ) # => others
include_directories(${CMAKE_SOURCE_DIR}/lib/gpgmepp/include  ) # => gpgmepp
include_directories(${CMAKE_SOURCE_DIR}/lib/gpgme/include    ) # => gpgme
include_directories(${CMAKE_SOURCE_DIR}/lib/gpg-error/include) # => gpg-error
include_directories(${CMAKE_SOURCE_DIR}/lib/assuan/include   ) # => assuan
include_directories(${CMAKE_SOURCE_DIR}/lib/ftxui/include    ) # => ftxui
include_directories(${CMAKE_SOURCE_DIR}/lib/asio             ) # => asio-1.38.2
include_directories(${CMAKE_SOURCE_DIR}/src                  ) # => c++ source file dircetory


# >=====> Main Source Files Configurations <=====<

set(SERVER ${CMAKE_SOURCE_DIR}/src/server/main.cpp) # => server main file
set(CLIENT ${CMAKE_SOURCE_DIR}/src/client/main.cpp) # => client main file

add_executable(server ${SERVER}) # server binary
add_executable(client ${CLIENT}) # client binary


# >=====> Cmake Libraries Configurations <=====<

add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/fmt     ) # => fmt-12.2.0
add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/sys_info) # => sys_info


# >======> Linking Libraries Configurations <======<

# => client
target_link_libraries(
  client
  PRIVATE ${ftxui_component}
  PRIVATE ${ftxui_dom}
  PRIVATE ${ftxui_screen}
  PRIVATE fmt::fmt
  PRIVATE ${Gpgmepp}
  PRIVATE ${Gpgme}
  PRIVATE ${Gpg_error}
  PRIVATE ${Assuan}
)

# => server
target_link_libraries(
  server
  PRIVATE ${ftxui_component}
  PRIVATE ${ftxui_dom}
  PRIVATE ${ftxui_screen}
  PRIVATE fmt::fmt
  PRIVATE sysinfo::core
)

# >======> Installing <======<

include(GNUInstallDirs)

install(
  TARGETS server client
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# >======> Packaging <======<

include(CPack)
set(CPACK_PACKAGE_NAME "cyber-chat")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_PACKAGE_VENDOR "paradox")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY " || A TUI E2EE chat || ")
