cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_FLAGS_INIT "")
set(CMAKE_CXX_FLAGS_INIT "")
set(CMAKE_C_FLAGS_DEBUG_INIT "")
set(CMAKE_C_FLAGS_RELEASE_INIT "")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "")

project(libzplay_build LANGUAGES C CXX)

if(MSVC)
    set(CMAKE_C_FLAGS "" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "" FORCE)
endif()

if(MSVC AND BUILD_ARCH STREQUAL "arm64")
    set(CMAKE_SYSTEM_PROCESSOR ARM64)
    set(CMAKE_VS_PLATFORM_NAME "ARM64")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_ARCH "Architecture: x64, x86, arm64" "x64")
option(BUILD_TYPE_OPT "Build type: release, debug" "release")
option(BUILD_OPTIMIZATION "Optimization: O0, O3, Os, OF" "O3")
option(BUILD_CRT "C runtime: MSVCRT, UCRT" "MSVCRT")

if(NOT DEFINED ENV{LIBZPLAY_COMPILER})
    set(ENV{LIBZPLAY_COMPILER} "mingw")
endif()
if(NOT DEFINED ENV{LIBZPLAY_ARCH})
    set(ENV{LIBZPLAY_ARCH} "${BUILD_ARCH}")
endif()
if(NOT DEFINED ENV{LIBZPLAY_CRT})
    set(ENV{LIBZPLAY_CRT} "${BUILD_CRT}")
endif()

set(COMPILER "$ENV{LIBZPLAY_COMPILER}")
set(ARCH "$ENV{LIBZPLAY_ARCH}")
set(CRT "$ENV{LIBZPLAY_CRT}")

message(STATUS "Compiler: ${COMPILER}, Arch: ${ARCH}, CRT: ${CRT}")

if(BUILD_OPTIMIZATION STREQUAL "OF")
    set(OPT_FLAG "-O3")
    set(FAST_MATH ON)
elseif(BUILD_OPTIMIZATION STREQUAL "O0")
    set(OPT_FLAG "-O0")
    set(IS_DEBUG ON)
elseif(BUILD_OPTIMIZATION STREQUAL "O3")
    set(OPT_FLAG "-O3")
elseif(BUILD_OPTIMIZATION STREQUAL "Os")
    set(OPT_FLAG "-Os")
endif()

if(NOT DEFINED IS_DEBUG AND (BUILD_TYPE_OPT STREQUAL "debug" OR IS_DEBUG))
    set(IS_DEBUG ON)
endif()

set(IS_MINGW OFF)
set(IS_LLVM OFF)
set(IS_VSCLANG OFF)
set(IS_MSVC OFF)

if(COMPILER STREQUAL "mingw")
    set(IS_MINGW ON)
elseif(COMPILER MATCHES "^llvm")
    set(IS_LLVM ON)
elseif(COMPILER MATCHES "vs-clang")
    set(IS_VSCLANG ON)
elseif(COMPILER STREQUAL "msvc")
    set(IS_MSVC ON)
endif()

set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/out/${COMPILER}/${CRT}/${ARCH}/${BUILD_TYPE_OPT}/-${BUILD_OPTIMIZATION}")
set(LIB_DIR "${OUTPUT_DIR}/lib")
set(BIN_DIR "${OUTPUT_DIR}/bin")
set(INCLUDE_DIR "${OUTPUT_DIR}/include")
set(EXAMPLE_DIR "${OUTPUT_DIR}/example")
set(OBJ_DIR "${OUTPUT_DIR}/obj")
set(DBG_DIR "${OUTPUT_DIR}/dbg")

file(MAKE_DIRECTORY ${LIB_DIR} ${BIN_DIR} ${INCLUDE_DIR} ${EXAMPLE_DIR} ${OBJ_DIR} ${DBG_DIR})

if(ARCH STREQUAL "x64" OR ARCH STREQUAL "arm64")
    set(SRC_PREFIX "src_x64")
else()
    set(SRC_PREFIX "src_x86")
endif()

set(SRC_ROOT "${CMAKE_SOURCE_DIR}/${SRC_PREFIX}")

if(IS_MSVC)
    if(BUILD_OPTIMIZATION STREQUAL "OF")
        set(MSVC_OPT "/O2 /GL /fp:fast")
    elseif(BUILD_OPTIMIZATION STREQUAL "O0")
        set(MSVC_OPT "/Od")
    elseif(BUILD_OPTIMIZATION STREQUAL "O3")
        set(MSVC_OPT "/O2")
    elseif(BUILD_OPTIMIZATION STREQUAL "Os")
        set(MSVC_OPT "/Os")
    endif()

    if(CRT STREQUAL "UCRT")
        if(IS_DEBUG)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
    else()
        if(IS_DEBUG)
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
        else()
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
        endif()
    endif()

    if(IS_DEBUG)
        if(BUILD_OPTIMIZATION STREQUAL "O0")
            set(MSVC_DEBUG_FLAGS "/Zi /Od /D_DEBUG")
        else()
            set(MSVC_DEBUG_FLAGS "/Zi /D_DEBUG")
        endif()
    else()
        set(MSVC_DEBUG_FLAGS "/Zi /DNDEBUG")
    endif()

    set(COMMON_FLAGS
        /W0
        /DWIN32
        /D_WINDOWS
        /DFLAC__NO_DLL
        /DFLAC__STATIC_BUILD
        /DLIB_ZPLAY_DYNAMIC
        /DLIB_ZPLAY_EXPORT_WITH_DEF_FILE
        /D_USE_MATH_DEFINES
        /D_CRT_SECURE_NO_WARNINGS
        /D_CRT_NONSTDC_NO_WARNINGS
        /D_CRT_NON_CONFORMING_SWPRINTFS
        /EHsc
        /Fd${DBG_DIR}/
        ${MSVC_DEBUG_FLAGS}
        ${MSVC_OPT}
    )

    set(CMAKE_STATIC_LIBRARY_SUFFIX ".lib")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Z7" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Z7" CACHE STRING "" FORCE)
else()
    set(COMMON_FLAGS
        -w
        -DWIN32
        -D_WINDOWS
        -DFLAC__NO_DLL
        -DFLAC__STATIC_BUILD
        -DLIB_ZPLAY_DYNAMIC
        -DLIB_ZPLAY_EXPORT_WITH_DEF_FILE
    )

    if(IS_LLVM OR IS_VSCLANG)
        list(APPEND COMMON_FLAGS
            /EHsc
        )
    endif()

    if(IS_DEBUG)
        list(APPEND COMMON_FLAGS -g3 -gdwarf-4 -D_DEBUG)
    else()
        list(APPEND COMMON_FLAGS -g -DNDEBUG)
    endif()

    list(APPEND COMMON_FLAGS ${OPT_FLAG})

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g" CACHE STRING "" FORCE)

    if(FAST_MATH)
        list(APPEND COMMON_FLAGS
            -ffast-math
            -fno-math-errno
            -ffinite-math-only
            -fno-rounding-math
            -fno-signaling-nans
            -freciprocal-math
            -fassociative-math
        )
    endif()

    if(CRT STREQUAL "UCRT" AND IS_MINGW)
        list(APPEND COMMON_FLAGS "-mcrtdll=ucrt")
        set(CRT_LINK_FLAG "-mcrtdll=ucrt")
    endif()

    if(IS_VSCLANG)
        if(CRT STREQUAL "UCRT")
            list(APPEND COMMON_FLAGS "-D_UCRT")
        else()
            list(APPEND COMMON_FLAGS "-D_MSVCPRT")
        endif()
        if(ARCH STREQUAL "arm64")
            list(APPEND COMMON_FLAGS "--target=arm64-pc-windows-msvc")
        endif()
    endif()
endif()

if(MSVC)
    string(REPLACE ";" " " CMAKE_C_FLAGS_STR "${COMMON_FLAGS} /wd4244 /wd4305 /wd4146 /wd4018 /wd4267 /wd4101 /wd5286 /wd4819")
    string(REPLACE ";" " " CMAKE_CXX_FLAGS_STR "${COMMON_FLAGS} /permissive- /wd4244 /wd4305 /wd4146 /wd4018 /wd4267 /wd4101 /wd5286 /wd4819")
else()
    string(REPLACE ";" " " CMAKE_C_FLAGS_STR "${COMMON_FLAGS}")
    string(REPLACE ";" " " CMAKE_CXX_FLAGS_STR "${COMMON_FLAGS} -fpermissive")
endif()

if(IS_LLVM OR IS_VSCLANG)
    set(CLANG_WARNINGS "-Wno-deprecated-declarations -Wno-unused-variable -Wno-unused-function -Wno-missing-braces -Wno-deprecated-non-prototype -Wno-nonportable-include-path -Wno-unused-but-set-variable -Wno-unused-private-field -Wno-misleading-indentation -Wno-shift-overflow -Wno-switch -Wno-enum-conversion -Wno-unused-value -Wno-gnu-folding-constant -Wno-array-parameter -Wno-absolute-value -Wno-macro-redefined")
    set(CMAKE_C_FLAGS_STR "${CMAKE_C_FLAGS_STR} ${CLANG_WARNINGS}")
    set(CMAKE_CXX_FLAGS_STR "${CMAKE_CXX_FLAGS_STR} ${CLANG_WARNINGS}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_STR}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_STR}" CACHE STRING "" FORCE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

if(ARCH STREQUAL "x86" AND IS_MINGW)
    set(STD_CALL_FIXUP ON)
endif()

if(ARCH STREQUAL "x86" AND NOT IS_MSVC)
    if(IS_LLVM OR IS_VSCLANG)
        set(MMX_SSE_FLAGS -msse -mmmx)
    elseif(IS_MINGW)
        set(MMX_SSE_FLAGS -mmmx -msse)
    endif()
endif()

function(add_static_lib LIB_NAME SRC_DIR INCLUDES PREFIX EXTRA_DEFS)
    file(GLOB _src_files ${ARGN})
    if(NOT _src_files)
        message(WARNING "No sources found for ${LIB_NAME} with pattern: ${ARGN}")
        return()
    endif()
    add_library(${LIB_NAME} STATIC ${_src_files})
    target_include_directories(${LIB_NAME} PRIVATE ${INCLUDES} ${SRC_DIR})
    if(EXTRA_DEFS)
        target_compile_definitions(${LIB_NAME} PRIVATE ${EXTRA_DEFS})
    endif()
    set_target_properties(${LIB_NAME} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}
        OUTPUT_NAME ${PREFIX}${LIB_NAME}
        POSITION_INDEPENDENT_CODE ON
    )
endfunction()

add_static_lib(zlib
    "${SRC_ROOT}/decoders/zlib"
    "${SRC_ROOT}/decoders/zlib"
    "z_"
    ""
    ${SRC_ROOT}/decoders/zlib/adler32.c
    ${SRC_ROOT}/decoders/zlib/compress.c
    ${SRC_ROOT}/decoders/zlib/crc32.c
    ${SRC_ROOT}/decoders/zlib/deflate.c
    ${SRC_ROOT}/decoders/zlib/gzclose.c
    ${SRC_ROOT}/decoders/zlib/gzlib.c
    ${SRC_ROOT}/decoders/zlib/gzread.c
    ${SRC_ROOT}/decoders/zlib/gzwrite.c
    ${SRC_ROOT}/decoders/zlib/infback.c
    ${SRC_ROOT}/decoders/zlib/inffast.c
    ${SRC_ROOT}/decoders/zlib/inflate.c
    ${SRC_ROOT}/decoders/zlib/inftrees.c
    ${SRC_ROOT}/decoders/zlib/trees.c
    ${SRC_ROOT}/decoders/zlib/uncompr.c
    ${SRC_ROOT}/decoders/zlib/zutil.c
)

add_static_lib(ogg
    "${SRC_ROOT}/decoders/libogg/src"
    "${SRC_ROOT}/decoders/libogg/include"
    "ogg_"
    ""
    ${SRC_ROOT}/decoders/libogg/src/*.c
)

add_static_lib(vorbis
    "${SRC_ROOT}/decoders/libvorbis/lib"
    "${SRC_ROOT}/decoders/libvorbis/include;${SRC_ROOT}/decoders/libogg/include"
    "vorbis_"
    ""
    ${SRC_ROOT}/decoders/libvorbis/lib/vorbisfile.c
    ${SRC_ROOT}/decoders/libvorbis/lib/floor0.c
    ${SRC_ROOT}/decoders/libvorbis/lib/res0.c
    ${SRC_ROOT}/decoders/libvorbis/lib/mapping0.c
    ${SRC_ROOT}/decoders/libvorbis/lib/info.c
    ${SRC_ROOT}/decoders/libvorbis/lib/floor1.c
    ${SRC_ROOT}/decoders/libvorbis/lib/envelope.c
    ${SRC_ROOT}/decoders/libvorbis/lib/codebook.c
    ${SRC_ROOT}/decoders/libvorbis/lib/block.c
    ${SRC_ROOT}/decoders/libvorbis/lib/synthesis.c
    ${SRC_ROOT}/decoders/libvorbis/lib/sharedbook.c
    ${SRC_ROOT}/decoders/libvorbis/lib/bitrate.c
    ${SRC_ROOT}/decoders/libvorbis/lib/analysis.c
    ${SRC_ROOT}/decoders/libvorbis/lib/window.c
    ${SRC_ROOT}/decoders/libvorbis/lib/vorbisenc.c
    ${SRC_ROOT}/decoders/libvorbis/lib/smallft.c
    ${SRC_ROOT}/decoders/libvorbis/lib/registry.c
    ${SRC_ROOT}/decoders/libvorbis/lib/psy.c
    ${SRC_ROOT}/decoders/libvorbis/lib/mdct.c
    ${SRC_ROOT}/decoders/libvorbis/lib/lsp.c
    ${SRC_ROOT}/decoders/libvorbis/lib/lpc.c
    ${SRC_ROOT}/decoders/libvorbis/lib/lookup.c
)

add_static_lib(mad
    "${SRC_ROOT}/decoders/libmad"
    "${SRC_ROOT}/decoders/libmad"
    "mad_"
    "FPM_DEFAULT"
    ${SRC_ROOT}/decoders/libmad/timer.c
    ${SRC_ROOT}/decoders/libmad/layer3.c
    ${SRC_ROOT}/decoders/libmad/stream.c
    ${SRC_ROOT}/decoders/libmad/frame.c
    ${SRC_ROOT}/decoders/libmad/layer12.c
    ${SRC_ROOT}/decoders/libmad/version.c
    ${SRC_ROOT}/decoders/libmad/synth.c
    ${SRC_ROOT}/decoders/libmad/huffman.c
    ${SRC_ROOT}/decoders/libmad/fixed.c
    ${SRC_ROOT}/decoders/libmad/decoder.c
    ${SRC_ROOT}/decoders/libmad/bit.c
)

add_static_lib(mp3lame
    "${SRC_ROOT}/decoders/lame/libmp3lame"
    "${SRC_ROOT}/decoders/lame/include;${SRC_ROOT}/decoders/lame/libmp3lame"
    "lame_"
    "STDC_HEADERS;HAVE_STRCHR;HAVE_MEMCPY"
    ${SRC_ROOT}/decoders/lame/libmp3lame/*.c
)

add_static_lib(FLAC
    "${SRC_ROOT}/decoders/libFLAC"
    "${SRC_ROOT}/decoders/libFLAC/include;${SRC_ROOT}/decoders/libFLAC;${SRC_ROOT}/decoders/libogg/include"
    "flac_"
    "FLAC__NO_DLL;FLAC__STATIC_BUILD"
    ${SRC_ROOT}/decoders/libFLAC/*.c
)

add_static_lib(faad
    "${SRC_ROOT}/decoders/faad/libfaad"
    "${SRC_ROOT}/decoders/faad/include;${SRC_ROOT}/decoders/faad/libfaad"
    "faad_"
    "STDC_HEADERS;HAVE_MEMCPY;HAVE_STRCHR"
    ${SRC_ROOT}/decoders/faad/libfaad/*.c
)

add_static_lib(faac
    "${SRC_ROOT}/decoders/faac/libfaac"
    "${SRC_ROOT}/decoders/faac/include"
    "faac_"
    ""
    ${SRC_ROOT}/decoders/faac/libfaac/*.c
)

add_static_lib(a52
    "${SRC_ROOT}/decoders/a52dec/liba52"
    "${SRC_ROOT}/decoders/a52dec/include"
    "a52_"
    ""
    ${SRC_ROOT}/decoders/a52dec/liba52/*.c
)

add_static_lib(jpeg
    "${SRC_ROOT}/decoders/jpeglib"
    "${SRC_ROOT}/decoders/jpeglib"
    "jpeg_"
    ""
    ${SRC_ROOT}/decoders/jpeglib/jcapimin.c
    ${SRC_ROOT}/decoders/jpeglib/jcapistd.c
    ${SRC_ROOT}/decoders/jpeglib/jcarith.c
    ${SRC_ROOT}/decoders/jpeglib/jchuff.c
    ${SRC_ROOT}/decoders/jpeglib/jcmainct.c
    ${SRC_ROOT}/decoders/jpeglib/jcprepct.c
    ${SRC_ROOT}/decoders/jpeglib/jccoefct.c
    ${SRC_ROOT}/decoders/jpeglib/jcsample.c
    ${SRC_ROOT}/decoders/jpeglib/jccolor.c
    ${SRC_ROOT}/decoders/jpeglib/jcparam.c
    ${SRC_ROOT}/decoders/jpeglib/jcinit.c
    ${SRC_ROOT}/decoders/jpeglib/jcmarker.c
    ${SRC_ROOT}/decoders/jpeglib/jcmaster.c
    ${SRC_ROOT}/decoders/jpeglib/jctrans.c
    ${SRC_ROOT}/decoders/jpeglib/jdapimin.c
    ${SRC_ROOT}/decoders/jpeglib/jdapistd.c
    ${SRC_ROOT}/decoders/jpeglib/jdarith.c
    ${SRC_ROOT}/decoders/jpeglib/jdhuff.c
    ${SRC_ROOT}/decoders/jpeglib/jdmainct.c
    ${SRC_ROOT}/decoders/jpeglib/jdcoefct.c
    ${SRC_ROOT}/decoders/jpeglib/jdsample.c
    ${SRC_ROOT}/decoders/jpeglib/jdcolor.c
    ${SRC_ROOT}/decoders/jpeglib/jdpostct.c
    ${SRC_ROOT}/decoders/jpeglib/jdmerge.c
    ${SRC_ROOT}/decoders/jpeglib/jdinput.c
    ${SRC_ROOT}/decoders/jpeglib/jdmarker.c
    ${SRC_ROOT}/decoders/jpeglib/jdmaster.c
    ${SRC_ROOT}/decoders/jpeglib/jdtrans.c
    ${SRC_ROOT}/decoders/jpeglib/jddctmgr.c
    ${SRC_ROOT}/decoders/jpeglib/jcdctmgr.c
    ${SRC_ROOT}/decoders/jpeglib/jfdctfst.c
    ${SRC_ROOT}/decoders/jpeglib/jfdctflt.c
    ${SRC_ROOT}/decoders/jpeglib/jfdctint.c
    ${SRC_ROOT}/decoders/jpeglib/jidctflt.c
    ${SRC_ROOT}/decoders/jpeglib/jidctfst.c
    ${SRC_ROOT}/decoders/jpeglib/jidctint.c
    ${SRC_ROOT}/decoders/jpeglib/jutils.c
    ${SRC_ROOT}/decoders/jpeglib/jerror.c
    ${SRC_ROOT}/decoders/jpeglib/jcomapi.c
    ${SRC_ROOT}/decoders/jpeglib/jmemmgr.c
    ${SRC_ROOT}/decoders/jpeglib/jmemnobs.c
    ${SRC_ROOT}/decoders/jpeglib/jaricom.c
    ${SRC_ROOT}/decoders/jpeglib/jquant1.c
    ${SRC_ROOT}/decoders/jpeglib/jquant2.c
    ${SRC_ROOT}/decoders/jpeglib/transupp.c
    ${SRC_ROOT}/decoders/jpeglib/rdbmp.c
    ${SRC_ROOT}/decoders/jpeglib/wrbmp.c
    ${SRC_ROOT}/decoders/jpeglib/rdgif.c
    ${SRC_ROOT}/decoders/jpeglib/wrgif.c
    ${SRC_ROOT}/decoders/jpeglib/rdppm.c
    ${SRC_ROOT}/decoders/jpeglib/wrppm.c
    ${SRC_ROOT}/decoders/jpeglib/rdrle.c
    ${SRC_ROOT}/decoders/jpeglib/wrrle.c
    ${SRC_ROOT}/decoders/jpeglib/rdcolmap.c
    ${SRC_ROOT}/decoders/jpeglib/rdtarga.c
    ${SRC_ROOT}/decoders/jpeglib/wrtarga.c
    ${SRC_ROOT}/decoders/jpeglib/rdswitch.c
    ${SRC_ROOT}/decoders/jpeglib/jdatasrc.c
    ${SRC_ROOT}/decoders/jpeglib/jdatadst.c
)

add_static_lib(png
    "${SRC_ROOT}/decoders/lpng"
    "${SRC_ROOT}/decoders/lpng;${SRC_ROOT}/decoders/zlib"
    "png_"
    ""
    ${SRC_ROOT}/decoders/lpng/*.c
)

set(SOUNDTOUCH_SOURCES
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/AAFilter.cpp
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/BPMDetect.cpp
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/FIFOSampleBuffer.cpp
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/FIRFilter.cpp
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/PeakFinder.cpp
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/RateTransposer.cpp
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/SoundTouch.cpp
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/TDStretch.cpp
)

if(ARCH STREQUAL "x86")
    list(APPEND SOUNDTOUCH_SOURCES
        ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/mmx_optimized.cpp
        ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/sse_optimized.cpp
    )
    if(IS_MSVC)
        list(APPEND SOUNDTOUCH_SOURCES
            ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/cpu_detect_x86_win.cpp
            ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/3dnow_win.cpp
        )
    else()
        list(APPEND SOUNDTOUCH_SOURCES
            ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/cpu_detect_x86_gcc.cpp
        )
    endif()
elseif(ARCH STREQUAL "x64")
    list(APPEND SOUNDTOUCH_SOURCES
        ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/mmx_optimized.cpp
        ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/sse_optimized.cpp
        ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/cpu_detect_x64.cpp
    )
elseif(ARCH STREQUAL "arm64")
    list(APPEND SOUNDTOUCH_SOURCES
        ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch/cpu_detect_x64.cpp
    )
endif()

add_library(SoundTouch STATIC ${SOUNDTOUCH_SOURCES})
target_include_directories(SoundTouch PUBLIC
    ${SRC_ROOT}/decoders/soundtouch/include
    ${SRC_ROOT}/decoders/soundtouch/source/SoundTouch
)
if(ARCH STREQUAL "x86")
    target_compile_options(SoundTouch PRIVATE ${MMX_SSE_FLAGS})
endif()
set_target_properties(SoundTouch PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}
    OUTPUT_NAME "st"
)

set(ZPLAY_INCLUDES
    ${SRC_ROOT}/include
    ${SRC_ROOT}/src
    ${SRC_ROOT}/decoders/lame/include
    ${SRC_ROOT}/decoders/libogg/include
    ${SRC_ROOT}/decoders/libvorbis/include
    ${SRC_ROOT}/decoders/libFLAC/include
    ${SRC_ROOT}/decoders/libFLAC
    ${SRC_ROOT}/decoders/faad/include
    ${SRC_ROOT}/decoders/faac/include
    ${SRC_ROOT}/decoders/a52dec/include
    ${SRC_ROOT}/decoders/soundtouch/include
    ${SRC_ROOT}/decoders/lpng
    ${SRC_ROOT}/decoders/zlib
)

file(GLOB ZPLAY_SOURCES "${SRC_ROOT}/src/*.cpp")

if(BUILD_TYPE_OPT STREQUAL "release")
    set(DLL_NAME "libzplay.dll")
else()
    set(DLL_NAME "libzplay_d.dll")
endif()

add_library(zplay_shared SHARED ${ZPLAY_SOURCES})
target_include_directories(zplay_shared PRIVATE ${ZPLAY_INCLUDES})
target_link_libraries(zplay_shared PRIVATE
    SoundTouch png jpeg a52 faac faad FLAC mp3lame mad vorbis ogg zlib
    winmm user32 gdi32 kernel32 ws2_32
)
if(STD_CALL_FIXUP AND NOT IS_MSVC)
    target_link_options(zplay_shared PRIVATE "-Wl,--enable-stdcall-fixup")
endif()
if(DEFINED CRT_LINK_FLAG)
    target_link_options(zplay_shared PRIVATE ${CRT_LINK_FLAG})
endif()
if(IS_MSVC)
    target_sources(zplay_shared PRIVATE ${SRC_ROOT}/src/libzplay.def)
    set_target_properties(zplay_shared PROPERTIES LINK_FLAGS "/DEF:${SRC_ROOT}/src/libzplay.def")
endif()
set_target_properties(zplay_shared PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${LIB_DIR}
    OUTPUT_NAME ${DLL_NAME}
    PREFIX ""
    SUFFIX ""
    PDB_OUTPUT_DIRECTORY ${DBG_DIR}
    COMPILE_PDB_OUTPUT_DIRECTORY ${DBG_DIR}
)
if(IS_MINGW)
    if(BUILD_TYPE_OPT STREQUAL "release")
        target_link_options(zplay_shared PRIVATE -s -Wl,--gc-sections)
    endif()
elseif(IS_MSVC OR IS_LLVM OR IS_VSCLANG)
    target_link_options(zplay_shared PRIVATE /DEBUG)
endif()
target_sources(zplay_shared PRIVATE ${SRC_ROOT}/src/libzplay.def)

add_library(zplay_static STATIC ${ZPLAY_SOURCES})
target_include_directories(zplay_static PRIVATE ${ZPLAY_INCLUDES})
target_link_libraries(zplay_static PRIVATE
    SoundTouch png jpeg a52 faac faad FLAC mp3lame mad vorbis ogg zlib
    winmm user32 gdi32 kernel32 ws2_32
)
if(DEFINED CRT_LINK_FLAG)
    target_link_options(zplay_static PRIVATE ${CRT_LINK_FLAG})
endif()
set_target_properties(zplay_static PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${LIB_DIR}
    OUTPUT_NAME "libzplay_static"
    PREFIX ""
    SUFFIX ".lib"
)
target_compile_definitions(zplay_static PRIVATE LIB_ZPLAY_STATIC)

configure_file(
    ${SRC_ROOT}/include/libzplay.h
    ${INCLUDE_DIR}/libzplay.h
    COPYONLY
)

file(READ ${SRC_ROOT}/include/libzplay.h HEADER_CONTENT)
string(REPLACE "#ifdef LIB_ZPLAY_STATIC"
    "/* DLL version */\n#if 0 && defined(LIB_ZPLAY_STATIC)"
    DLL_HEADER "${HEADER_CONTENT}")
file(WRITE ${INCLUDE_DIR}/libzplay_dll.h "${DLL_HEADER}")
file(WRITE ${INCLUDE_DIR}/libzplay_static.h "#define LIB_ZPLAY_STATIC\n\n${HEADER_CONTENT}")

file(WRITE ${EXAMPLE_DIR}/test_dll.cpp [=[
#include <windows.h>
#include <stdio.h>
#include "libzplay_dll.h"

int main(int argc, char* argv[])
{
    printf("libzplay DLL Test\n");
    printf("==================\n\n");
    
    libZPlay::ZPlay *player = libZPlay::CreateZPlay();
    if (!player) {
        printf("ERROR: Failed to create ZPlay instance\n");
        return 1;
    }
    
    printf("ZPlay instance created successfully!\n");
    printf("Version: %u\n", player->GetVersion());
    
    const char* testFile = (argc > 1) ? argv[1] : "test.mp3";
    
    int result = player->OpenFile(testFile, libZPlay::sfAutodetect);
    if (result == 0) {
        printf("Note: Could not open test file: %s\n", testFile);
    } else {
        printf("File opened: %s\n", testFile);
        printf("Press Enter to play...\n");
        getchar();
        player->Play();
        printf("Playing... Press Enter to stop.\n");
        getchar();
        player->Stop();
    }
    
    player->Release();
    printf("\nDLL test completed successfully!\n");
    return 0;
}
]=])

file(WRITE ${EXAMPLE_DIR}/test_static.cpp [=[
#include <windows.h>
#include <stdio.h>
#include "libzplay_static.h"

int main(int argc, char* argv[])
{
    printf("libzplay Static Library Test\n");
    printf("============================\n\n");
    
    libZPlay::ZPlay *player = libZPlay::CreateZPlay();
    if (!player) {
        printf("ERROR: Failed to create ZPlay instance\n");
        return 1;
    }
    
    printf("ZPlay instance created successfully!\n");
    printf("Version: %u\n", player->GetVersion());
    
    const char* testFile = (argc > 1) ? argv[1] : "test.mp3";
    
    int result = player->OpenFile(testFile, libZPlay::sfAutodetect);
    if (result == 0) {
        printf("Note: Could not open test file: %s\n", testFile);
    } else {
        printf("File opened: %s\n", testFile);
        printf("Press Enter to play...\n");
        getchar();
        player->Play();
        printf("Playing... Press Enter to stop.\n");
        getchar();
        player->Stop();
    }
    
    player->Release();
    printf("\nStatic library test completed successfully!\n");
    return 0;
}
]=])

add_executable(test_dll ${EXAMPLE_DIR}/test_dll.cpp)
target_include_directories(test_dll PRIVATE ${INCLUDE_DIR})
target_link_directories(test_dll PRIVATE ${LIB_DIR})
target_link_libraries(test_dll PRIVATE zplay_shared winmm user32 gdi32 kernel32 ws2_32)
if(DEFINED CRT_LINK_FLAG)
    target_link_options(test_dll PRIVATE ${CRT_LINK_FLAG})
endif()
set_target_properties(test_dll PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_DIR})

add_executable(test_static ${EXAMPLE_DIR}/test_static.cpp)
target_include_directories(test_static PRIVATE ${INCLUDE_DIR})
target_link_directories(test_static PRIVATE ${LIB_DIR})
target_link_libraries(test_static PRIVATE
    zplay_static
    SoundTouch png jpeg a52 faac faad FLAC mp3lame mad vorbis ogg zlib
    winmm user32 gdi32 kernel32 ws2_32
)
if(DEFINED CRT_LINK_FLAG)
    target_link_options(test_static PRIVATE ${CRT_LINK_FLAG})
endif()
set_target_properties(test_static PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXAMPLE_DIR})

add_custom_command(TARGET zplay_shared POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OBJ_DIR}/zplay_shared"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OBJ_DIR}/zplay_static"
    COMMENT "Creating obj directories"
)

if(IS_MSVC OR IS_LLVM OR IS_VSCLANG OR IS_MINGW)
    add_custom_command(TARGET zplay_shared POST_BUILD
        COMMAND cmd /c "robocopy \"${CMAKE_BINARY_DIR}\" \"${OBJ_DIR}/zplay_shared\" *.obj /s /njh /njs /nc /ns /np & if %ERRORLEVEL% LEQ 7 exit 0"
        COMMENT "Copying .obj files to ${OBJ_DIR}/zplay_shared"
    )
endif()

add_custom_command(TARGET zplay_shared POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DBG_DIR}"
    COMMENT "Ensuring debug directory exists"
)

if(IS_MINGW)
    find_program(OBJCOPY objcopy)
    if(OBJCOPY)
        add_custom_command(TARGET zplay_shared POST_BUILD
            COMMAND ${OBJCOPY} --only-keep-debug "${BIN_DIR}/${DLL_NAME}" "${DBG_DIR}/${DLL_NAME}.debug"
            COMMENT "Extracting debug symbols to ${DBG_DIR}/${DLL_NAME}.debug"
        )
    endif()
endif()

message(STATUS "")
message(STATUS "=========================================")
message(STATUS " libzplay Build Configuration")
message(STATUS "=========================================")
message(STATUS " Compiler:   ${COMPILER}")
message(STATUS " CRT:       ${CRT}")
message(STATUS " Arch:      ${ARCH}")
message(STATUS " Type:      ${BUILD_TYPE_OPT}")
message(STATUS " Optimize:  -${BUILD_OPTIMIZATION}")
message(STATUS " Output:    ${OUTPUT_DIR}")
message(STATUS " Obj:       ${OBJ_DIR}")
message(STATUS " Debug:     ${DBG_DIR}")
message(STATUS "=========================================")
