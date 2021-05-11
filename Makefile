# Project Name
TARGET = tight_pedal

# Sources
CPP_SOURCES = $(wildcard src/*.c) $(wildcard src/*.cc) $(wildcard src/*.cpp)

# Library Locations
LIBDAISY_DIR = lib/libDaisy
DAISYSP_DIR = lib/DaisySP

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
