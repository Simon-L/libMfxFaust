cmake_minimum_required(VERSION 3.25.0)

project(
  libMfxFaust
  VERSION 1.0
  LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 17)

if( NOT DEFINED FaustPrefix)
  message( FATAL_ERROR "Please provide FaustPrefix eg. -DFaustPrefix=/usr/local" )
endif()

if( NOT DEFINED LuaPrefix)
  message( WARNING "LuaPrefix not provided, testing will fail" )
endif()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS on)
  cmake_policy(SET CMP0141 NEW)
ENDIF()

add_library(MfxFaust SHARED DspFaust.cpp)
  
IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")  
  set(LibFaust libfaustwithllvm.a)
  set(OSCFaust lOSCFaustvm.a)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  find_package(PkgConfig REQUIRED)
  # pkg_check_modules(jack REQUIRED jack)
  set(LibFaust libfaustwithllvm.a)
  set(OSCFaust lOSCFaustvm.a)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_compile_options(MfxFaust PUBLIC "/Zc:externC")
  set(LibFaust libfaustwithllvm.lib)
  set(OSCFaust libOSCFaust.lib)
  target_link_libraries(MfxFaust PRIVATE ws2_32)
ENDIF()

target_compile_definitions(MfxFaust PUBLIC DUMMY_DRIVER=1)
target_compile_definitions(MfxFaust PUBLIC DYNAMIC_DSP=1)
# target_compile_definitions(MfxFaust PUBLIC SOUNDFILE=1)
# target_compile_definitions(MfxFaust PUBLIC DYNAMIC_DSP=1)
# target_compile_definitions(MfxFaust PUBLIC MIDICTRL=1)
# target_compile_definitions(MfxFaust PUBLIC OSCCTRL=1)

IF(${MSVC_WINE})
  set(LUAJIT_DIR ${LuaPrefix}/bin)
  set(LUAJIT_EXE wine ./lua.exe)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LUAJIT_DIR ${LuaPrefix}/bin)
  set(LUAJIT_EXE pw-jack ./lua)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(LUAJIT_DIR ${LuaPrefix}/bin)
  set(LUAJIT_EXE ./lua)
ENDIF()

# Drivers:
# COREAUDIO_DRIVER 1
# ALSA_DRIVER 1
# JACK_DRIVER 1
# PORTAUDIO_DRIVER 1
# RTAUDIO_DRIVER 1
# DUMMY_DRIVER 1
# MIDICTRL 1
# OSCCTRL 1
# SOUNDFILE 1

target_include_directories(MfxFaust BEFORE
  PUBLIC "${FaustPrefix}/include"
)
target_link_directories(MfxFaust BEFORE
  PRIVATE ${FaustPrefix}/lib
)

target_link_libraries(MfxFaust PRIVATE
  ${LibFaust}
  # ${FaustPrefix}/lib/${OSCFaust}
  # OSCFaust
  # tinfo
  # z
  # jack
  # zstd
  # sndfile
)

add_custom_target(LuajitTest
  DEPENDS MfxFaust
  COMMENT "
  _______                      __      __                   
 |       |  .-----.  .-----.  |  |_   |__|  .-----.  .-----.
 |.|   | |  |  -__|  |__ --|  |   _|  |  |  |     |  |  _  |
 `-|.  |-'  |_____|  |_____|  |____|  |__|  |__|__|  |___  |
   |:  |                                             |_____|
   |::.|                                                    
   `---'                                                    "
  COMMAND ${LUAJIT_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ui.lua $<TARGET_FILE:MfxFaust> ${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ui_nostdfaust.dsp
  WORKING_DIRECTORY ${LUAJIT_DIR}
  COMMENT ---- Testing by running ${LUAJIT_EXE}, using $<TARGET_FILE:MfxFaust> in ${LUAJIT_DIR} ----
  VERBATIM
)
