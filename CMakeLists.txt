cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(vita_rss)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "VitaRSS")
set(VITA_TITLEID  "CFHN00001")
set(VITA_VERSION  "01.00")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O2 -fno-inline -fno-builtin -fshort-wchar")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions -Wl,-q -Wall -O2 -fno-builtin -fshort-wchar -Wno-unused-function -Wno-sign-compare")

set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2F00000000000101")

enable_language(CXX ASM)

link_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../vitasdk
)

include_directories(
)

add_executable(${PROJECT_NAME}
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
  SceAppMgr_stub
  SceKernelThreadMgr_stub
  SceSysmodule_stub
  SceCtrl_stub
  SceLibKernel_stub
  SceIofilemgr_stub
  SceProcessmgr_stub
  SceNet_stub
  SceNetCtl_stub
  SceSsl_stub
  SceHttp_stub
  ScePaf_stub_weak
  ScePafWebMapView_stub_weak
  SceWebUIPlugin_stub_weak
)

set_target_properties(${PROJECT_NAME}
  PROPERTIES LINK_FLAGS "-nostartfiles -nostdlib"
  COMPILE_FLAGS "'-D__declspec(__v__)=/**/'"
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME}
  CONFIG exports.yml
  UNSAFE
  STRIPPED
  REL_OPTIMIZE
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cxml/main.rco
  COMMAND psp2cxml-tool ${CMAKE_CURRENT_SOURCE_DIR}/cxml/main.xml
  MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/cxml/main.xml
)

vita_create_vpk(${PROJECT_NAME}.vpk "${VITA_TITLEID}" ${PROJECT_NAME}.self
  VERSION "${VITA_VERSION}"
  NAME "${PROJECT_NAME}"
  FILE ${CMAKE_CURRENT_SOURCE_DIR}/cxml/main.rco main.rco
  FILE ${CMAKE_CURRENT_SOURCE_DIR}/sce_sys sce_sys
)

vita_create_stubs(stubs_${PROJECT_NAME} ${PROJECT_NAME} exports.yml)

add_custom_target(
  upload
  COMMAND curl --ftp-method nocwd -T ${PROJECT_NAME}.vpk ftp://192.168.1.122:1337/ux0:/downloads/
  DEPENDS ${PROJECT_NAME}.vpk
)
