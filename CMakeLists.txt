cmake_minimum_required(VERSION 3.16)
project(
  nomadbsd-dmconfig-gui
  HOMEPAGE_URL https://github.com/nomadbsd/nomadbsd-dmconfig-gui
  LANGUAGES CXX)
set(CMAKE_INSTALL_PREFIX "/usr")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROGRAM ${CMAKE_PROJECT_NAME})
set(BACKEND "nomadbsd-dmconfig")

set(BACKEND_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/libexec)
set(PATH_BACKEND ${BACKEND_INSTALL_DIR}/${BACKEND})
set(PATH_SLIM_THEME_DIR "/usr/local/share/slim/themes")
set(PATH_SDDM_THEME_DIR "/usr/local/share/sddm/themes")

set(SOURCE_FILES src/backend.h src/main.cpp src/mainwin.cpp src/mainwin.h
                 lib/qt-helper/qt-helper.cpp)

find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
qt_standard_project_setup()

set(TS_FILES
    locale/${PROGRAM}_cs.ts locale/${PROGRAM}_de.ts locale/${PROGRAM}_fr.ts
    locale/${PROGRAM}_id.ts locale/${PROGRAM}_ru.ts)

add_executable(nomadbsd-dmconfig-gui ${SOURCE_FILES})

qt_add_translations(nomadbsd-dmconfig-gui TS_FILES ${TS_FILES})

target_include_directories(
  nomadbsd-dmconfig-gui
  PUBLIC "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}"
         "${CMAKE_CURRENT_SOURCE_DIR}/src" "${CMAKE_CURRENT_SOURCE_DIR}/lib")

configure_file(defs.h.in defs.h @ONLY)

set(LIBS Qt6::Widgets)
target_link_libraries(nomadbsd-dmconfig-gui PRIVATE ${LIBS})

install(TARGETS nomadbsd-dmconfig-gui DESTINATION bin)
install(PROGRAMS ${BACKEND} DESTINATION libexec)
install(FILES "${CMAKE_PROJECT_NAME}.desktop" DESTINATION share/applications)
