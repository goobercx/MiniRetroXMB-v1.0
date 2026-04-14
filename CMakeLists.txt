cmake_minimum_required(VERSION 3.2)
project(MiniRetroXMB)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "MiniRetroXMB")
set(VITA_TITLEID  "MNRXMB001")
set(VITA_VERSION  "01.00")

add_executable(MiniRetroXMB
  src/main.c
  src/colorpicker.c
  src/bgpicker.c
  src/export.c
  src/preview.c
  src/themebrowser.c
)

target_link_libraries(MiniRetroXMB
  vita2d vitashark SceShaccCg_stub
  SceDisplay_stub SceGxm_stub
  SceCtrl_stub SceTouch_stub
  SceAppMgr_stub
  ScePgf_stub
  SceCommonDialog_stub
  SceKernelModulemgr_stub
  png jpeg z m c
)

vita_create_self(MiniRetroXMB.self MiniRetroXMB)
vita_create_vpk(MiniRetroXMB.vpk ${VITA_TITLEID} MiniRetroXMB.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE sce_sys/livearea/contents/bg0.png sce_sys/livearea/contents/bg0.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
)
