cmake_minimum_required(VERSION 3.6)

project(libzplay
    DESCRIPTION "libZPlay audio library - Lite version"
    LANGUAGES CXX C
)

option(LIBZPLAY_STATIC_CRT "Use static CRT library"                 FALSE)
option(LIBZPLAY_PACK_WITH_UPX "Pack programs with UPX"              FALSE)
option(LIBZPLAY_BUILD_EXAMPLES "Build example programs"             TRUE)
option(LIBZPLAY_INSTALL "Install library"                           TRUE)

option(LIBZPLAY_DSP_SOUNDTOUCH "Soundtouch audio processor support" TRUE)

option(LIBZPLAY_INPUT_AAC "AAC input support (faad2)"               TRUE)
option(LIBZPLAY_INPUT_AC3 "AC3 input support (liba52)"              TRUE)
option(LIBZPLAY_INPUT_FLAC "FLAC input support (libflac)"           TRUE)
option(LIBZPLAY_INPUT_MP3 "MP3 input support (libmad)"              TRUE)
option(LIBZPLAY_INPUT_VORBIS "Vorbis input support (vorbis)"        TRUE)
option(LIBZPLAY_INPUT_WAV "PCM/WAV input support"                   TRUE)
option(LIBZPLAY_INPUT_WAVEIN "WAVEIN input support"                 TRUE)

set(LIBZPLAY_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(LIBZPLAY_EXAMPLES ${LIBZPLAY_ROOT}/example)
set(LIBZPLAY_INCLUDE ${LIBZPLAY_ROOT}/include)
set(LIBZPLAY_SRC ${LIBZPLAY_ROOT}/src)
set(DECODERS_DIR ${LIBZPLAY_ROOT}/decoders)

set(LIBZPLAY_SOURCE_FILES
    ${LIBZPLAY_SRC}/config.h
    ${LIBZPLAY_SRC}/debug.h
    ${LIBZPLAY_SRC}/libzplay.cpp
    ${LIBZPLAY_SRC}/waudioprocessor.cpp
    ${LIBZPLAY_SRC}/waudioprocessor.h
    ${LIBZPLAY_SRC}/wdecoder.cpp
    ${LIBZPLAY_SRC}/wdecoder.h
    ${LIBZPLAY_SRC}/winputprocessor.cpp
    ${LIBZPLAY_SRC}/winputprocessor.h
    ${LIBZPLAY_SRC}/wmp3x.cpp
    ${LIBZPLAY_SRC}/wmp3x.h
    ${LIBZPLAY_SRC}/woutputprocessor.cpp
    ${LIBZPLAY_SRC}/woutputprocessor.h
    ${LIBZPLAY_SRC}/wqueue.cpp
    ${LIBZPLAY_SRC}/wqueue.h
    ${LIBZPLAY_SRC}/wtags.cpp
    ${LIBZPLAY_SRC}/wtags.h
    ${LIBZPLAY_SRC}/wtools.cpp
    ${LIBZPLAY_SRC}/wtools.h
    ${LIBZPLAY_SRC}/wwavelet.cpp
    ${LIBZPLAY_SRC}/wwavelet.h
)

if (WIN32)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/resource.rc
    )
endif()

if(LIBZPLAY_DSP_SOUNDTOUCH)
    SET(SOUNDTOUCH_SRC ${DECODERS_DIR}/soundtouch/source/SoundTouch)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/wstprocessor.cpp
        ${LIBZPLAY_SRC}/wstprocessor.h
        ${SOUNDTOUCH_SRC}/AAFilter.cpp
        ${SOUNDTOUCH_SRC}/FIFOSampleBuffer.cpp
        ${SOUNDTOUCH_SRC}/FIRFilter.cpp
        ${SOUNDTOUCH_SRC}/PeakFinder.cpp
        ${SOUNDTOUCH_SRC}/RateTransposer.cpp
        ${SOUNDTOUCH_SRC}/SoundTouch.cpp
        ${SOUNDTOUCH_SRC}/TDStretch.cpp
        ${SOUNDTOUCH_SRC}/cpu_detect_x86_win.cpp
        ${SOUNDTOUCH_SRC}/mmx_optimized.cpp
        ${SOUNDTOUCH_SRC}/sse_optimized.cpp
    )
endif()

if(LIBZPLAY_INPUT_AAC)
    SET(FAAD_SRC ${DECODERS_DIR}/faad/libfaad)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/waacdecoder.cpp
        ${LIBZPLAY_SRC}/waacdecoder.h
        ${FAAD_SRC}/bits.c
        ${FAAD_SRC}/cfft.c
        ${FAAD_SRC}/common.c
        ${FAAD_SRC}/decoder.c
        ${FAAD_SRC}/drc.c
        ${FAAD_SRC}/drm_dec.c
        ${FAAD_SRC}/error.c
        ${FAAD_SRC}/filtbank.c
        ${FAAD_SRC}/hcr.c
        ${FAAD_SRC}/huffman1.c
        ${FAAD_SRC}/ic_predict.c
        ${FAAD_SRC}/is.c
        ${FAAD_SRC}/lt_predict.c
        ${FAAD_SRC}/mdct.c
        ${FAAD_SRC}/mp4.c
        ${FAAD_SRC}/ms.c
        ${FAAD_SRC}/output.c
        ${FAAD_SRC}/pns.c
        ${FAAD_SRC}/ps_dec.c
        ${FAAD_SRC}/ps_syntax.c
        ${FAAD_SRC}/pulse.c
        ${FAAD_SRC}/rvlc.c
        ${FAAD_SRC}/sbr_dct.c
        ${FAAD_SRC}/sbr_dec.c
        ${FAAD_SRC}/sbr_e_nf.c
        ${FAAD_SRC}/sbr_fbt.c
        ${FAAD_SRC}/sbr_hfadj.c
        ${FAAD_SRC}/sbr_huff.c
        ${FAAD_SRC}/sbr_qmf.c
        ${FAAD_SRC}/sbr_syntax.c
        ${FAAD_SRC}/sbr_tf_grid.c
        ${FAAD_SRC}/specrec.c
        ${FAAD_SRC}/ssr.c
        ${FAAD_SRC}/ssr_fb.c
        ${FAAD_SRC}/ssr_ipqf.c
        ${FAAD_SRC}/syntax.c
        ${FAAD_SRC}/tns.c
    )
endif()

if(LIBZPLAY_INPUT_AC3)
    SET(A52_SRC ${DECODERS_DIR}/a52dec/liba52)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/wac3decoder.cpp
        ${LIBZPLAY_SRC}/wac3decoder.h
        ${A52_SRC}/bit_allocate.c
        ${A52_SRC}/bitstream.c
        ${A52_SRC}/downmix.c
        ${A52_SRC}/imdct.c
        ${A52_SRC}/parse.c
    )
endif()

if(LIBZPLAY_INPUT_FLAC)
    SET(FLAC_SRC ${DECODERS_DIR}/libFLAC)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/wflacdecoder.cpp
        ${LIBZPLAY_SRC}/wflacdecoder.h
        ${FLAC_SRC}/bitmath.c
        ${FLAC_SRC}/bitreader.c
        ${FLAC_SRC}/bitwriter.c
        ${FLAC_SRC}/cpu.c
        ${FLAC_SRC}/crc.c
        ${FLAC_SRC}/fixed.c
        ${FLAC_SRC}/float.c
        ${FLAC_SRC}/format.c
        ${FLAC_SRC}/lpc.c
        ${FLAC_SRC}/md5.c
        ${FLAC_SRC}/memory.c
        ${FLAC_SRC}/metadata_iterators.c
        ${FLAC_SRC}/metadata_object.c
        ${FLAC_SRC}/ogg_decoder_aspect.c
        ${FLAC_SRC}/ogg_encoder_aspect.c
        ${FLAC_SRC}/ogg_helper.c
        ${FLAC_SRC}/ogg_mapping.c
        ${FLAC_SRC}/stream_decoder.c
        ${FLAC_SRC}/stream_encoder.c
        ${FLAC_SRC}/stream_encoder_framing.c
        ${FLAC_SRC}/window.c
    )
endif()

if(LIBZPLAY_INPUT_MP3)
    SET(MAD_SRC ${DECODERS_DIR}/libmad)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/wmp3decoder.cpp
        ${LIBZPLAY_SRC}/wmp3decoder.h
        ${MAD_SRC}/bit.c
        ${MAD_SRC}/decoder.c
        ${MAD_SRC}/fixed.c
        ${MAD_SRC}/frame.c
        ${MAD_SRC}/huffman.c
        ${MAD_SRC}/layer12.c
        ${MAD_SRC}/layer3.c
        ${MAD_SRC}/stream.c
        ${MAD_SRC}/synth.c
        ${MAD_SRC}/timer.c
        ${MAD_SRC}/version.c
    )
endif()

if(LIBZPLAY_INPUT_VORBIS)
    SET(OGG_SRC ${DECODERS_DIR}/libogg/src)
    SET(VORBIS_SRC ${DECODERS_DIR}/libvorbis/lib)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/woggdecoder.cpp
        ${LIBZPLAY_SRC}/woggdecoder.h
        ${OGG_SRC}/bitwise.c
        ${OGG_SRC}/framing.c
        ${VORBIS_SRC}/analysis.c
        ${VORBIS_SRC}/bitrate.c
        ${VORBIS_SRC}/block.c
        ${VORBIS_SRC}/codebook.c
        ${VORBIS_SRC}/envelope.c
        ${VORBIS_SRC}/floor0.c
        ${VORBIS_SRC}/floor1.c
        ${VORBIS_SRC}/info.c
        ${VORBIS_SRC}/lookup.c
        ${VORBIS_SRC}/lpc.c
        ${VORBIS_SRC}/lsp.c
        ${VORBIS_SRC}/mapping0.c
        ${VORBIS_SRC}/mdct.c
        ${VORBIS_SRC}/psy.c
        ${VORBIS_SRC}/registry.c
        ${VORBIS_SRC}/res0.c
        ${VORBIS_SRC}/sharedbook.c
        ${VORBIS_SRC}/smallft.c
        ${VORBIS_SRC}/synthesis.c
        ${VORBIS_SRC}/window.c
        ${VORBIS_SRC}/vorbisfile.c
    )
endif()

if(LIBZPLAY_INPUT_WAV)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/wpcmdecoder.cpp
        ${LIBZPLAY_SRC}/wpcmdecoder.h
        ${LIBZPLAY_SRC}/wwavedecoder.cpp
        ${LIBZPLAY_SRC}/wwavedecoder.h
    )
endif()

if(LIBZPLAY_INPUT_WAVEIN)
    SET(LIBZPLAY_SOURCE_FILES ${LIBZPLAY_SOURCE_FILES}
        ${LIBZPLAY_SRC}/wwavein.cpp
        ${LIBZPLAY_SRC}/wwavein.h
    )
endif()

add_library(${PROJECT_NAME} ${LIBZPLAY_SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${LIBZPLAY_INCLUDE}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${LIBZPLAY_MINIAUDIO_PATH} ${LIBZPLAY_SRC}
        ${DECODERS_DIR}/soundtouch/include
        ${DECODERS_DIR}/faad/include
        ${DECODERS_DIR}/a52dec/include
        ${DECODERS_DIR}/libFLAC/include
        ${DECODERS_DIR}/libmad
        ${DECODERS_DIR}/libogg/include
        ${DECODERS_DIR}/libvorbis/include
        ${DECODERS_DIR}/libvorbis/lib
)

if(WIN32 AND MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -O3 -w)
    if(WIN32)
        target_compile_options(${PROJECT_NAME} PRIVATE -ffunction-sections -fdata-sections)
        target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_MEMCPY)
    endif()
endif()

if(LIBZPLAY_STATIC_CRT AND WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_Z_PLAY_DYNAMIC)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(${PROJECT_NAME} PRIVATE -s)
        if(WIN32)
            target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
        endif()
    endif()

    if(LIBZPLAY_PACK_WITH_UPX)
        include(FindSelfPackers)
        if(SELF_PACKER_FOR_EXECUTABLE)
            ADD_CUSTOM_COMMAND(
                COMMAND ${SELF_PACKER_FOR_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
                ARGS ${SELF_PACKER_FOR_EXECUTABLE_FLAGS} -9q
                TARGET ${PROJECT_NAME}
            )
        endif()
    endif()
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE LIB_Z_PLAY_STATIC)
endif()

if(LIBZPLAY_BUILD_EXAMPLES)
    set(examples)
    file(GLOB sources ${LIBZPLAY_EXAMPLES}/*.cpp)
    list(APPEND examples ${sources})

    foreach(example_source ${examples})
        get_filename_component(example_name ${example_source} NAME)
        string(REPLACE ".cpp" "${OUTPUT_EXT}" example_name ${example_name})

        add_executable(${example_name} ${example_source})
        target_compile_features(${example_name} PRIVATE cxx_std_11)
        target_link_libraries(${example_name} PRIVATE ${PROJECT_NAME})

        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${example_name} PRIVATE -Wall -Wpedantic -O3 -w)
            if(WIN32)
                target_compile_options(${example_name} PRIVATE -ffunction-sections -fdata-sections)
            endif()
        endif()
    endforeach()
endif()

macro(message_bool_option _NAME _VALUE)
    if(${_VALUE})
        message(STATUS "  ${_NAME}: enabled")
    else()
        message(STATUS "  ${_NAME}: disabled")
    endif()
endmacro()

message(STATUS "libZPlay Lite will be built with the following options:")
message_bool_option("Build shared library" BUILD_SHARED_LIBS)
message_bool_option("Use static CRT library" LIBZPLAY_STATIC_CRT)
message_bool_option("Pack programs with UPX" LIBZPLAY_PACK_WITH_UPX)
message_bool_option("Build example programs" LIBZPLAY_BUILD_EXAMPLES)

message(STATUS "libZPlay Lite will be built with the following DSP audio processors:")
message_bool_option("Soundtouch audio processor support" LIBZPLAY_DSP_SOUNDTOUCH)

message(STATUS "libZPlay Lite will be built with the following input formats:")
message_bool_option("AAC input support (faad2)" LIBZPLAY_INPUT_AAC)
message_bool_option("AC3 input support (liba52)" LIBZPLAY_INPUT_AC3)
message_bool_option("FLAC input support (libflac)" LIBZPLAY_INPUT_FLAC)
message_bool_option("MP3 input support (libmad)" LIBZPLAY_INPUT_MP3)
message_bool_option("Vorbis input support (vorbis)" LIBZPLAY_INPUT_VORBIS)
message_bool_option("PCM/WAV input support" LIBZPLAY_INPUT_WAV)
message_bool_option("WAVEIN input support" LIBZPLAY_INPUT_WAVEIN)
