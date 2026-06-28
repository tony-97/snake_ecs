#==============================================================================
# libs/raylib/Makefile — Raylib configuration & build
#
# This file serves TWO roles:
#   1. CONSUMER CONFIG — when included by the root Makefile, it provides
#      include paths, link libraries, defines, and system dependencies.
#   2. BUILD CONFIG — when invoked directly ($(MAKE) -f libs/raylib/Makefile),
#      it additionally sets build-only flags and delegates to Makefile.rules.
#
# The MAKEFILE_LIST trick auto-detects which role we're in:
#   - Direct invocation: we're the first file in MAKEFILE_LIST
#   - Included by root: root's Makefile is the first file
#==============================================================================

# Capture our filename before any includes modify MAKEFILE_LIST
_LAST_MK := $(lastword $(MAKEFILE_LIST))
_INCLUDED_AS_CONFIG := $(filter $(_LAST_MK),$(firstword $(MAKEFILE_LIST)))

# When invoked directly, load the generic build infrastructure
ifneq ($(_INCLUDED_AS_CONFIG),)
include Makefile.vars
endif

#==============================================================================
# Consumer configuration (always active)
# Root Makefile and this build both need these.
#==============================================================================

# Include path so #include <raylib.h> works
INCLUDE_DIRS += ./external/raylib/src/

# Link against the static library
ifndef MSVC
    LDLIBS  += -lraylib
    CFLAGS  += -flto
    LDFLAGS += -flto
endif

RAYLIB_VERSION     := 6.0.0
RAYLIB_API_VERSION := 600

#==============================================================================
# Backend detection
#==============================================================================
ifeq ($(TARGET),WEB)
    BACKEND = PLATFORM_WEB
endif

ifneq ($(filter LINUX WINDOWS OSX,$(TARGET)),)
    GRAPHICS := GRAPHICS_API_OPENGL_33
    BACKEND ?= PLATFORM_DESKTOP_GLFW
endif

ifneq ($(filter ANDROID WEB DRM,$(TARGET)),)
    GRAPHICS := GRAPHICS_API_OPENGL_ES2
endif

DEFINES += $(BACKEND) $(GRAPHICS)

X11 = 1
WAYLAND = 0

#==============================================================================
# System library dependencies
#==============================================================================
ifndef MSVC
    LDLIBS += -lc -lm -latomic
endif

# Desktop backends — common system libs
ifneq ($(filter PLATFORM_DESKTOP_GLFW PLATFORM_DESKTOP_SDL PLATFORM_DESKTOP_RGFW,$(BACKEND)),)
    ifeq ($(TARGET),WINDOWS)
    ifndef MSVC
        LDLIBS += -static-libgcc -lopengl32 -lgdi32 -lwinmm
    endif
    endif
    ifeq ($(TARGET),LINUX)
        # NOTE: Required packages: libegl1-mesa-dev
        LDLIBS += -lGL -lpthread -ldl -lrt
        ifeq ($(X11),1)
            DEFINES += _GLFW_X11
            LDLIBS  += -lX11
        endif
    endif
    ifeq ($(TARGET),OSX)
        LDLIBS += -framework Cocoa -framework OpenGL -framework CoreVideo -framework IOKit -framework CoreAudio
    endif
endif

# GLFW backend — Wayland support
ifeq ($(BACKEND),PLATFORM_DESKTOP_GLFW)
    ifneq ($(filter LINUX WINDOWS OSX,$(TARGET)),)
        ifeq ($(TARGET),LINUX)
            ifdef ($(WAYLAND),1)
                LDFLAGS += $(shell pkg-config wayland-client wayland-cursor wayland-egl xkbcommon --libs)

                WL_PROTOCOLS_DIR := external/glfw/deps/wayland

                wl_generate = \
                    $(eval protocol=$(1)) \
                    $(eval basename=$(2)) \
                    $(shell wayland-scanner client-header $(protocol) $(RAYLIB_SRC_PATH)/$(basename).h) \
                    $(shell wayland-scanner private-code $(protocol) $(RAYLIB_SRC_PATH)/$(basename)-code.h)

                $(call wl_generate, $(WL_PROTOCOLS_DIR)/wayland.xml, wayland-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/xdg-shell.xml, xdg-shell-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/xdg-decoration-unstable-v1.xml, xdg-decoration-unstable-v1-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/viewporter.xml, viewporter-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/relative-pointer-unstable-v1.xml, relative-pointer-unstable-v1-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/pointer-constraints-unstable-v1.xml, pointer-constraints-unstable-v1-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/fractional-scale-v1.xml, fractional-scale-v1-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/xdg-activation-v1.xml, xdg-activation-v1-client-protocol)
                $(call wl_generate, $(WL_PROTOCOLS_DIR)/idle-inhibit-unstable-v1.xml, idle-inhibit-unstable-v1-client-protocol)
            endif
        endif
    endif
endif

# SDL backend
ifeq ($(BACKEND),PLATFORM_DESKTOP_SDL)
    LDLIBS += $(SDL_LIBRARIES)
endif

# RGFW backend
ifeq ($(BACKEND),PLATFORM_DESKTOP_RGFW)
    ifeq ($(TARGET),LINUX)
        LDLIBS += -lX11 -lXrandr -lXinerama -lXi -lXcursor
    endif
endif

#==============================================================================
# Per-target link flags
#==============================================================================
ifeq ($(TARGET),WEB)
    LDFLAGS += -sUSE_GLFW=3
endif
ifeq ($(TARGET),ANDROID)
    ANDROID_ARCH ?= arm64
    ANDROID_API_VERSION ?= 35
    BACKEND  := PLATFORM_ANDROID
    LDLIBS   += -llog -landroid -lEGL -lGLESv2 -lOpenSLES
    LDFLAGS  += -Wl,--exclude-libs,libatomic.a
endif
ifeq ($(TARGET),WINDOWS)
ifdef MSVC
    LDLIBS += /L:libraylib.a libraylib.a Shell32.lib user32.lib opengl32.lib gdi32.lib winmm.lib
endif
endif

#==============================================================================
# BUILD-ONLY section — everything below only applies when building raylib,
# not when included by the root Makefile for consumer config.
#==============================================================================
ifneq ($(_INCLUDED_AS_CONFIG),)

LIB_NAME     := raylib

SRC_DIR      := ./external/raylib/src
EXCLUDE_SRCS := $(wildcard $(SRC_DIR)/external/glfw/src/*.c) \
                $(wildcard $(SRC_DIR)/external/*.c) \
                $(wildcard $(SRC_DIR)/platforms/*.c)

ifndef MSVC
    DEBUG_FLAGS   += -g -ggdb -O0 -D_DEBUG
    RELEASE_FLAGS += -Ofast -DNDEBUG
    DEFINES       += _GNU_SOURCE
    CFLAGS        += -std=c99 -fno-strict-aliasing $(CUSTOM_CFLAGS) -fPIC -flto
    WFLAGS        += -Wall -Wno-missing-braces -Wno-unused-parameter \
                     -Wno-c11-extensions -Wno-shadow \
                     -Werror=pointer-arith -Werror=implicit-function-declaration
endif

# GLFW include path (build-only — consumers use ./external/raylib/src/)
ifneq ($(filter PLATFORM_DESKTOP_GLFW PLATFORM_WEB,$(BACKEND)),)
    INCLUDE_DIRS += $(SRC_DIR)/external/glfw/include
endif

# Exclude rglfw.c if not using GLFW backend
ifneq ($(BACKEND),PLATFORM_DESKTOP_GLFW)
    EXCLUDE_SRCS += $(SRC_DIR)/rglfw.c
endif

ifeq ($(TARGET),WEB)
    WFLAGS        += -Wno-unused-command-line-argument
    CFLAGS         = -std=gnu99 -fno-strict-aliasing
    DEBUG_FLAGS    = -O0 -sASSERTIONS=1 --profiling
    RELEASE_FLAGS  = -Oz --closure 2 -sMODULARIZE -flto
endif
ifeq ($(TARGET),ANDROID)
    LDFLAGS  += -Wl,-soname,libraylib.$(RAYLIB_API_VERSION).so -L$(SRC_DIR)
endif
ifeq ($(TARGET),WINDOWS)
ifdef MSVC
    DEBUG_FLAGS   += /Od
    RELEASE_FLAGS += /O2yit
    WFLAGS        += /wd4668 /wd4820 /wd4061 /wd5045 /wd4711 /wd4710 /wd4191 /wd4100 /wd4244 /wd4996
    CFLAGS        += /std:c11 /nologo
else
    LDLIBS        += -lshell32
    INCLUDE_DIRS  += $(SRC_DIR)/external/glfw/deps/mingw
endif
endif

export

#==============================================================================
# Targets — delegate to the generic build system
#==============================================================================
.PHONY: all lib info clean cleanall

all: lib

lib:
	@$(MAKE) -f Makefile.rules lib

info:
	@$(MAKE) -f Makefile.rules info

clean:
	@$(MAKE) -f Makefile.rules clean

cleanall:
	@$(MAKE) -f Makefile.rules cleanall

endif # _INCLUDED_AS_CONFIG
