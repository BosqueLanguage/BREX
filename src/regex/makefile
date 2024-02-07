MAKE_PATH=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR=$(MAKE_PATH)/../../build/
BIN_DIR=$(MAKE_PATH)../../bin/
RE_DIR=$(MAKE_PATH)/
SRC_DIR=$(MAKE_PATH)/../

OUT_EXE=$(BUILD_DIR)output/
OUT_OBJ=$(BUILD_DIR)output/obj/

JSON_INCLUDES=-I $(BUILD_DIR)include/headers/json/

#dev is default, for another flavor : make BUILD=release
BUILD := dev

CPP=g++
CPP_STDFLAGS=-Wall -Wextra -Wno-unused-parameter -Wuninitialized -Werror -std=gnu++20

CPPFLAGS_OPT.dev=-Og -ggdb -fno-omit-frame-pointer -fsanitize=address
CPPFLAGS_OPT.release=-O3 -march=x86-64-v3
CPPFLAGS := ${CPPFLAGS_OPT.${BUILD}} ${CPP_STDFLAGS}

AR=ar
ARFLAGS=rs

COMMON_HEADERS=$(SRC_DIR)common.h
COMMON_SOURCES=$(SRC_DIR)common.cpp
COMMON_OBJS=$(OUT_OBJ)common.o

REGEX_HEADERS=$(RE_DIR)brex.h $(RE_DIR)brex_parser.h $(RE_DIR)brex_compiler.h $(RE_DIR)nfa_machine.h $(RE_DIR)nfa_executor.h
REGEX_SOURCES=$(RE_DIR)brex.cpp $(RE_DIR)brex_compiler.cpp $(RE_DIR)nfa_machine.cpp
REGEX_OBJS=$(OUT_OBJ)brex.o $(OUT_OBJ)brex_compiler.o $(OUT_OBJ)nfa_machine.o

MAKEFLAGS += -j4

all: brexlib

brexlib: $(REGEX_HEADERS) $(REGEX_OBJS) $(COMMON_HEADERS) $(COMMON_OBJS)
	@mkdir -p $(OUT_EXE)
	$(AR) $(ARFLAGS) $(OUT_EXE)brexlib.a $(REGEX_OBJS) $(COMMON_OBJS)

$(OUT_OBJ)brex.o: $(COMMON_HEADERS) $(REGEX_HEADERS) $(RE_DIR)brex.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)brex.o -c $(RE_DIR)brex.cpp

$(OUT_OBJ)brex_compiler.o: $(COMMON_HEADERS) $(REGEX_HEADERS) $(RE_DIR)brex_compiler.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)brex_compiler.o -c $(RE_DIR)brex_compiler.cpp

$(OUT_OBJ)nfa_machine.o: $(COMMON_HEADERS) $(REGEX_HEADERS) $(RE_DIR)nfa_machine.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)nfa_machine.o -c $(RE_DIR)nfa_machine.cpp

$(OUT_OBJ)common.o: $(COMMON_HEADERS) $(COMMON_SOURCES)
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)common.o -c $(COMMON_SOURCES)

clean:
	rm -rf $(OUT_EXE)* $(OUT_OBJ)*.o