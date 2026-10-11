cmake_minimum_required(VERSION 3.10)

# Load the PSP platform configuration BEFORE project()
set(CMAKE_SYSTEM_CMAKE_PLATFORM_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(${CMAKE_SYSTEM_CMAKE_PLATFORM_PATH}/PSP.cmake)

project(OSLib C CXX ASM)

# Set C and C++ standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Set the output library
set(TARGET_LIB osl)
add_library(${TARGET_LIB} STATIC)

find_package(libxmp-lite CONFIG REQUIRED)
target_link_libraries(${TARGET_LIB} PUBLIC libxmp-lite::xmp_lite_static)

# PSP-Math is provided by its own package, not compiled into libosl.a.
set(PSP_MATH_ARCHIVE "${PSPDIR}/lib/psp-math.a")
if(NOT EXISTS "${PSP_MATH_ARCHIVE}")
    message(FATAL_ERROR "OSLib requires the installed psp-math package: ${PSP_MATH_ARCHIVE}")
endif()
target_link_libraries(${TARGET_LIB} PUBLIC "${PSP_MATH_ARCHIVE}")

# Specify the output directory for the .a file (project root directory)
set_target_properties(${TARGET_LIB} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# Include directories
include_directories(
    INTERFACE PUBLIC_HEADER  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/libintraFont/include>
    INTERFACE PRIVATE_HEADER $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/giflib>
    ${PROJECT_BINARY_DIR}  # For generated osl_config.h
    src
    src/adhoc
)

# Treat third-party library directories as system includes to suppress warnings
include_directories(SYSTEM
    ${PROJECT_SOURCE_DIR}/lib/giflib
    ${PROJECT_SOURCE_DIR}/lib/libintraFont/include
)

# Define the source files
set(SOURCE_DIR src)
set(SFONT_SOURCES
    ${SOURCE_DIR}/sfont.c
)
set(GIFLIB_SOURCES
    lib/giflib/dgif_lib.c
    lib/giflib/egif_lib.c
    lib/giflib/gif_err.c
    lib/giflib/gifalloc.c
    lib/giflib/gif_hash.c
    lib/giflib/quantize.c
    lib/giflib/openbsd-reallocarray.c
)
set(INTRAFONT_SOURCES
    lib/libintraFont/src/intraFont.c
    lib/libintraFont/src/libccc.c
)
set(LIB_SOURCES
    ${SOURCE_DIR}/adhoc/pspadhoc.c
    ${SOURCE_DIR}/audio/audio.c
    ${SOURCE_DIR}/audio/bgm.c
    ${SOURCE_DIR}/audio/media.c
    ${SOURCE_DIR}/audio/mod.c
    ${SOURCE_DIR}/browser.c
    ${SOURCE_DIR}/dialog.c
    ${SOURCE_DIR}/drawing.c
    ${SOURCE_DIR}/image.c
    ${SOURCE_DIR}/image/oslConvertImageTo.c
    ${SOURCE_DIR}/image/oslDrawImage.c
    ${SOURCE_DIR}/image/oslDrawImageBig.c
    ${SOURCE_DIR}/image/oslDrawImageSimple.c
    ${SOURCE_DIR}/image/oslGetImagePixel.c
    ${SOURCE_DIR}/image/oslLockImage.c
    ${SOURCE_DIR}/image/oslMoveImageTo.c
    ${SOURCE_DIR}/image/oslResetImageProperties.c
    ${SOURCE_DIR}/image/oslScaleImage.c
    ${SOURCE_DIR}/image/oslSetDrawBuffer.c
    ${SOURCE_DIR}/image/oslSetImagePixel.c
    ${SOURCE_DIR}/image/oslSwizzleImage.c
    ${SOURCE_DIR}/image/oslUnswizzleImage.c
    ${SOURCE_DIR}/keys.c
    ${SOURCE_DIR}/map.c
    ${SOURCE_DIR}/mem/oslGetRamStatus.c
    ${SOURCE_DIR}/messagebox.c
    ${SOURCE_DIR}/net.c
    ${SOURCE_DIR}/osk.c
    ${SOURCE_DIR}/oslHandleLoadNoFailError.c
    ${SOURCE_DIR}/oslib.c
    ${SOURCE_DIR}/palette.c
    ${SOURCE_DIR}/saveload.c
    ${SOURCE_DIR}/shape.c
    ${SOURCE_DIR}/image/oslLoadImageFile.c
    ${SOURCE_DIR}/image/oslWriteImageFile.c
    ${SOURCE_DIR}/splash/oslShowSplashScreen1.c
    ${SOURCE_DIR}/splash/oslShowSplashScreen2.c
    ${SOURCE_DIR}/stub.S
    ${SOURCE_DIR}/text.c
    ${SOURCE_DIR}/usb.c
    ${SOURCE_DIR}/vfile/vfsFile.c
    ${SOURCE_DIR}/vfile/VirtualFile.c
    ${SOURCE_DIR}/vram_mgr.c
)

include(${CMAKE_SYSTEM_CMAKE_PLATFORM_PATH}/IMG.cmake)

# Specify that stub.S is an assembly file
set_source_files_properties(${SOURCE_DIR}/stub.S PROPERTIES LANGUAGE ASM)

# Suppress warnings for third-party library sources
set_source_files_properties(
    ${GIFLIB_SOURCES}
    ${INTRAFONT_SOURCES}
    PROPERTIES
    COMPILE_FLAGS "-w"
)

# Add sources to the target
target_sources(${TARGET_LIB} PRIVATE
    ${SFONT_SOURCES}
    ${GIFLIB_SOURCES}
    ${INTRAFONT_SOURCES}
    ${LIB_SOURCES}
)

# Installation instructions
install(TARGETS ${TARGET_LIB} DESTINATION ${PSPDIR}/lib)

# Include directories installation (headers)
install(FILES
    lib/libintraFont/include/intraFont.h
    lib/libintraFont/include/libccc.h
    DESTINATION ${PSPDIR}/include/oslib
)
install(FILES
    ${SOURCE_DIR}/adhoc/pspadhoc.h
    DESTINATION ${PSPDIR}/include/oslib/adhoc
)
install(FILES
    ${SOURCE_DIR}/oslmath.h
    ${SOURCE_DIR}/net.h
    ${SOURCE_DIR}/browser.h
    ${SOURCE_DIR}/audio.h
    ${SOURCE_DIR}/bgm.h
    ${SOURCE_DIR}/dialog.h
    ${SOURCE_DIR}/drawing.h
    ${SOURCE_DIR}/keys.h
    ${SOURCE_DIR}/map.h
    ${SOURCE_DIR}/messagebox.h
    ${SOURCE_DIR}/osk.h
    ${SOURCE_DIR}/saveload.h
    ${SOURCE_DIR}/oslib.h
    ${SOURCE_DIR}/text.h
    ${SOURCE_DIR}/usb.h
    ${SOURCE_DIR}/VirtualFile.h
    ${SOURCE_DIR}/vram_mgr.h
    ${SOURCE_DIR}/ccc.h
    ${SOURCE_DIR}/sfont.h
    ${PROJECT_BINARY_DIR}/osl_config.h
    DESTINATION ${PSPDIR}/include/oslib
)

# Add Cppcheck execution
find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    set(CPPCHECK_COMMAND
        ${CPPCHECK} --enable=all --inconclusive --quiet
        --suppressions-list=${CMAKE_SOURCE_DIR}/.cppcheck-suppress
        --checkers-report=${CMAKE_SOURCE_DIR}/cppcheck-report.txt
    )
    add_custom_target(
        cppcheck
        COMMAND ${CPPCHECK_COMMAND} ${CMAKE_SOURCE_DIR}/src || true
        COMMENT "Running Cppcheck and generating checkers report..."
    )
else()
    message(WARNING "Cppcheck not found, skipping static analysis.")
endif()

# Add Uncrustify execution
find_program(UNCRUSTIFY uncrustify)
if(UNCRUSTIFY)
    add_custom_target(
        uncrustify
        COMMAND ${CMAKE_SOURCE_DIR}/run-uncrustify.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running Uncrustify to format the code (respecting .uncrustifyignore)..."
    )
    add_custom_target(
        uncrustify-check
        COMMAND ${CMAKE_SOURCE_DIR}/run-uncrustify.sh --check
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking code formatting with Uncrustify (no files are modified)..."
    )
else()
    message(WARNING "Uncrustify not found, skipping code formatting.")
endif()
