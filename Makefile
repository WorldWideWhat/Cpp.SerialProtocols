#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the Cpp compiler to use
#CXX = arm-linux-gnueabihf-g++
CXX = g++

# define any compile-time flags
CXXFLAGS	:= -std=c++17 -Wall -Wextra -g -Wno-unused-parameter

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = 

# define output directory
OUTPUT	:= output
BUILD_ROOT_PATH := build
OUTPUT_OBJECT_PATH = $(BUILD_ROOT_PATH)/obj
OUTPUT_BINARY_PATH = $(BUILD_ROOT_PATH)/bin
SOURCEDIRS := src/lib/uart src/lib/cctalk src/lib/stm src/lib/host

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= include

# define lib directory
LIB		:= lib

ifeq ($(OS),Windows_NT)
MAIN	:= SerialInterface.exe
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= rm -f
MD	:= mkdir
else
MAIN	:= SerialInterface
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -f
MD	:= mkdir -p
endif

define find
  $(foreach dir,$(1),$(foreach d,$(wildcard $(dir)/*),\
                        $(call find,$(d),$(2))) $(wildcard $(dir)/$(strip $(2))))
endef
unexport find

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))


SOURCES := $(call find, $(SOURCEDIRS),*.cpp) src/main.cpp

# define the C source files
#SOURCES		:= $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))

# define the C object files 
OBJECTS := $(SOURCES:%.cpp=$(OUTPUT_OBJECT_PATH)/%.o)


#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT_BINARY_PATH)/$(MAIN))

all: $(OUTPUT_BINARY_PATH) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT_BINARY_PATH):
	$(MD) $(OUTPUT_BINARY_PATH)

$(MAIN): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
$(OUTPUT_OBJECT_PATH)/%.o: %.cpp
	@echo C+ $<
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN)
	@echo Executing 'run: all' complete!