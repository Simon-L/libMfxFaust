cmake_minimum_required(VERSION 3.25.0)

project(
  libMfxFaust
  VERSION 1.0
  LANGUAGES CXX C)

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

add_library(MfxFaust SHARED DspFaust.cpp Extra.c)

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

IF(DUMMY)
  target_compile_definitions(MfxFaust PUBLIC DUMMY_DRIVER=1)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_compile_definitions(MfxFaust PUBLIC JACK_DRIVER=1)
  target_compile_definitions(MfxFaust PUBLIC MIDICTRL=1)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_compile_definitions(MfxFaust PUBLIC RTAUDIO_DRIVER=1)
  # target_compile_definitions(MfxFaust PUBLIC MIDICTRL=1)
ELSE()
  target_compile_definitions(MfxFaust PUBLIC DUMMY_DRIVER=1)
ENDIF()

target_compile_definitions(MfxFaust PUBLIC DYNAMIC_DSP=1)
# target_compile_definitions(MfxFaust PUBLIC SOUNDFILE=1)
# target_compile_definitions(MfxFaust PUBLIC OSCCTRL=1)

IF(${MSVC_WINE})
  set(LUAJIT_DIR ${LuaPrefix}/bin)
  set(LUAJIT_EXE wine ./lua.exe)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    cmake_path(SET LUAJIT_DIR "${LuaPrefix}/bin")
    set(LUAJIT_EXE lua.exe)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(LUAJIT_DIR ${LuaPrefix}/bin)
  set(LUAJIT_EXE ./lua)
  target_link_options(MfxFaust PRIVATE "LINKER:--version-script,${CMAKE_SOURCE_DIR}/linker_script.txt")
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

include(cmake/CPM.cmake)

CPMAddPackage(
  NAME sndfile
  GIT_TAG "1.2.2"
  GITHUB_REPOSITORY libsndfile/libsndfile
  OPTIONS "ENABLE_EXTERNAL_LIBS OFF" "BUILD_TESTING OFF" "ENABLE_MPEG OFF" "BUILD_PROGRAMS OFF" "BUILD_EXAMPLES OFF"
)

CPMAddPackage(
  NAME nfd
  GIT_TAG a1a4010
  GITHUB_REPOSITORY btzy/nativefiledialog-extended
  OPTIONS "NFD_PORTAL ON"
)

CPMAddPackage(
  NAME rtaudio
  GIT_TAG "6.0.1"
  GITHUB_REPOSITORY thestk/rtaudio
  OPTIONS "RTAUDIO_API_OSS OFF" "RTAUDIO_API_ALSA OFF" "RTAUDIO_API_PULSE OFF" "RTAUDIO_API_JACK OFF" "RTAUDIO_API_ASIO OFF" "RTAUDIO_API_DS ON" "RTAUDIO_API_WASAPI OFF" "RTAUDIO_BUILD_STATIC_LIBS ON"
)

target_include_directories(MfxFaust BEFORE
  PUBLIC "${FaustPrefix}/include"
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  add_custom_target(
    SetupIncludeDirectory
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/rtaudio
    COMMAND ${CMAKE_COMMAND} -E copy
            ${rtaudio_SOURCE_DIR}/RtAudio.h
            ${CMAKE_CURRENT_BINARY_DIR}/include/rtaudio/RtAudio.h
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/include/rtaudio/RtAudio.h
  )
  target_include_directories(MfxFaust BEFORE
    PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include"
  )
  add_dependencies(MfxFaust SetupIncludeDirectory)

ENDIF()

set_property(TARGET sndfile PROPERTY POSITION_INDEPENDENT_CODE ON)

target_link_libraries(MfxFaust PRIVATE
${FaustPrefix}/lib/${LibFaust}
nfd
# ${FaustPrefix}/lib/${OSCFaust}
# OSCFaust
# tinfo
# z
# rtaudio
# zstd
sndfile
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  target_link_libraries(MfxFaust PRIVATE rtaudio)
ELSEIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  target_link_libraries(MfxFaust PRIVATE jack)
ENDIF()

# add_executable(Test_1 test/test_1.c)
# target_link_libraries(Test_1 PUBLIC
  # MfxFaust
  # z
  # zstd
  # tinfo
# )

add_custom_target(LuajitTest
  DEPENDS MfxFaust
  COMMENT "
  _______                      __      __                   
 |       |  .-----.  .-----.  |  |_   |__|  .-----.  .-----.
 |.|   | |  |  -__|  |__ --|  |   _|  |  |  |     |  |  _  |
 `-|.  |-'  |_____|  |_____|  |____|  |__|  |__|__|  |___  |
   |:  |                                             |_____|
   |::.|                                                    
   `---'                                                    
---- Testing by running ${LUAJIT_EXE}, using $<TARGET_FILE:MfxFaust> in ${LUAJIT_DIR} ----"
  COMMAND ${LUAJIT_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ui.lua --lib=$<TARGET_FILE:MfxFaust> --dsp=${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ui_nostdfaust.dsp
  COMMAND ${LUAJIT_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ui.lua --lib=$<TARGET_FILE:MfxFaust> --dsp=${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ui.dsp --I=${FaustPrefix}/share/faust
  COMMAND ${LUAJIT_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ringbuffer.lua --lib=$<TARGET_FILE:MfxFaust> --dsp=${CMAKE_CURRENT_SOURCE_DIR}/test/test_create_dsp_ui.dsp --I=${FaustPrefix}/share/faust
  WORKING_DIRECTORY ${LUAJIT_DIR}
  VERBATIM
)
