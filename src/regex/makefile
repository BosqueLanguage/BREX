MAKE_PATH=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR=$(MAKE_PATH)/../../build/
BIN_DIR=$(MAKE_PATH)/../../bin/
RE_DIR=$(MAKE_PATH)/
SRC_DIR=$(MAKE_PATH)/../

REGEX_TEST_SRC_DIR=$(MAKE_PATH)/../../test/regex/

OUT_EXE=$(BUILD_DIR)output/
OUT_OBJ=$(BUILD_DIR)output/obj/

JSON_INCLUDES=-I $(BUILD_DIR)include/headers/json/
LIB_PATH=$(OUT_EXE)

#dev is default, for another flavor : make BUILD=release or debug
BUILD := dev

CPP=g++
CPP_STDFLAGS=-Wall -Wextra -Wno-unused-parameter -Wuninitialized -Werror -std=gnu++20

CPPFLAGS_OPT.debug=-O0 -g -ggdb -fno-omit-frame-pointer -DBREX_DEBUG -fsanitize=address
CPPFLAGS_OPT.dev=-O0 -g -ggdb -fno-omit-frame-pointer -DBREX_DEBUG
CPPFLAGS_OPT.release=-O3 -march=x86-64-v3
CPPFLAGS=${CPPFLAGS_OPT.${BUILD}} ${CPP_STDFLAGS}
CPPFLAGS_TEST=${CPPFLAGS_OPT.dev} ${CPP_STDFLAGS}

AR=ar
ARFLAGS=rs

COMMON_HEADERS=$(SRC_DIR)common.h
COMMON_SOURCES=$(SRC_DIR)common.cpp
COMMON_OBJS=$(OUT_OBJ)common.o

REGEX_HEADERS=$(RE_DIR)brex.h $(RE_DIR)brex_parser.h $(RE_DIR)brex_compiler.h $(RE_DIR)nfa_machine.h $(RE_DIR)nfa_executor.h
REGEX_SOURCES=$(RE_DIR)brex.cpp $(RE_DIR)brex_compiler.cpp $(RE_DIR)nfa_machine.cpp
REGEX_OBJS=$(OUT_OBJ)brex.o $(OUT_OBJ)brex_compiler.o $(OUT_OBJ)nfa_machine.o

REGEX_TEST_SOURCES=$(REGEX_TEST_SRC_DIR)main.cpp $(REGEX_TEST_SRC_DIR)parsing_ok.cpp $(REGEX_TEST_SRC_DIR)parsing_err.cpp $(REGEX_TEST_SRC_DIR)test.cpp

MAKEFLAGS += -j4

all: $(OUT_EXE)brexlib.a $(BIN_DIR)brex

$(BIN_DIR)brex: $(COMMON_HEADERS) $(REGEX_HEADERS) $(OUT_EXE)brexlib.a $(RE_DIR)brex_cmd.cpp
	@mkdir -p $(OUT_EXE)
	$(CPP) $(CPPFLAGS) -L$(LIB_PATH) $(JSON_INCLUDES) -o $(BIN_DIR)brex $(RE_DIR)brex_cmd.cpp $(OUT_EXE)brexlib.a

$(OUT_EXE)brexlib.a: $(REGEX_OBJS) $(COMMON_OBJS)
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

testfiles: $(COMMON_HEADERS) $(REGEX_HEADERS) $(OUT_EXE)brexlib.a $(REGEX_TEST_SOURCES)
	@mkdir -p $(OUT_EXE)
	$(CPP) $(CPPFLAGS_TEST) -L$(LIB_PATH) $(JSON_INCLUDES) -o $(BIN_DIR)regex_test $(REGEX_TEST_SOURCES) $(OUT_EXE)brexlib.a -lboost_unit_test_framework

test: testfiles
	$(BIN_DIR)regex_test 

clean:
	rm -rf $(OUT_EXE)* $(OUT_OBJ)*.o $(BIN_DIR)*