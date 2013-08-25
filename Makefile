PROJECT = dcpu_vm
# Compiler
CC = g++

# Run Options       
COMMANDLINE_OPTIONS =  tester.bin

# Compiler options during compilation
COMPILE_OPTIONS = -std=c++11 -Wall -Wno-packed-bitfield-compat
RELEASE_OPTIONS = -O3
DEBUG_OPTIONS = -g -O0

DEBUG ?= 1
ifeq ($(DEBUG), 1)
	COMPILE_OPTIONS := $(COMPILE_OPTIONS) $(DEBUG_OPTIONS)
else
	COMPILE_OPTIONS := $(COMPILE_OPTIONS) $(RELEASE_OPTIONS)
endif


#Header include directories
HEADERS =
#Libraries for linking
LIBS = -lpthread -lsfml-graphics -lsfml-window -lsfml-system

# Dependency options
DEPENDENCY_OPTIONS = -MM -std=c++11

#-- Do not edit below this line --

# Subdirs to search for additional source files
SUBDIRS := $(shell ls -F | grep "\/" )
DIRS := ./ $(SUBDIRS)
SOURCE_FILES := $(foreach d, $(DIRS), $(wildcard $(d)*.cpp) )

# Create an object file of every cpp file
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCE_FILES))

# Dependencies
DEPENDENCIES = $(patsubst %.cpp, %.d, $(SOURCE_FILES))

# Create .d files
%.d: %.cpp
	$(CC) $(DEPENDENCY_OPTIONS) $< -MT "$*.o $*.d" -MF $*.d

# Make $(PROJECT) the default target
all: $(DEPENDENCIES) $(PROJECT)

$(PROJECT): $(OBJECTS)
	@echo " "
	@echo "---------- Linking"
	$(CC) -o $(PROJECT) $(OBJECTS) $(LIBS)

# Include dependencies (if there are any)
ifneq ("$(strip $(DEPENDENCIES))", "")
 include $(DEPENDENCIES)
endif

# Compile every cpp file to an object
%.o: %.cpp
	$(CC) -c $(COMPILE_OPTIONS) -o $@ $< $(HEADERS)

# Do the release version
.PHONY: release
release:
	make DEBUG=0

# Build & Run Project
.PHONY: run
run: $(PROJECT)
	./$(PROJECT) $(COMMANDLINE_OPTIONS)

# Clean
.PHONY: clean
clean:
	rm -f $(PROJECT) $(OBJECTS)

.PHONY: depclean
depclean:
	rm -f $(DEPENDENCIES)

clean-all: clean depclean
