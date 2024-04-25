MAKE_PATH=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))
BUILD_DIR=$(MAKE_PATH)/
BIN_DIR=$(MAKE_PATH)/../bin/
SRC_DIR=$(MAKE_PATH)/../src/
RE_DIR=$(SRC_DIR)regex/
PTH_DIR=$(SRC_DIR)path/

REGEX_TEST_SRC_DIR=$(MAKE_PATH)/../test/regex/

OUT_EXE=$(BUILD_DIR)output/
OUT_OBJ=$(BUILD_DIR)output/obj/
PCKG_DIR=$(BUILD_DIR)brex/

JSON_INCLUDES=-I $(BUILD_DIR)include/headers/json/
LIB_PATH=$(OUT_EXE)

#dev is default, for another flavor : make BUILD=release or debug
BUILD := dev

CPP=g++
CPP_STDFLAGS=-Wall -Wextra -Wno-unused-parameter -Wuninitialized -Werror -std=gnu++20 -fPIC

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

REGEX_HEADERS=$(RE_DIR)brex.h $(RE_DIR)brex_parser.h $(RE_DIR)brex_compiler.h $(RE_DIR)brex_executor.h $(RE_DIR)nfa_machine.h $(RE_DIR)nfa_executor.h
REGEX_SOURCES=$(RE_DIR)brex.cpp $(RE_DIR)brex_compiler.cpp $(RE_DIR)nfa_machine.cpp
REGEX_OBJS=$(OUT_OBJ)brex.o $(OUT_OBJ)brex_compiler.o $(OUT_OBJ)nfa_machine.o

PATH_HEADERS=$(PTH_DIR)path.h $(PTH_DIR)path_fragment.h $(PTH_DIR)path_glob.h
PATH_SOURCES=$(PTH_DIR)path_glob.cpp
PATH_OBJS=$(OUT_OBJ)path_glob.o

REGEX_TEST_SOURCES=$(REGEX_TEST_SRC_DIR)main.cpp $(REGEX_TEST_SRC_DIR)parsing_ok.cpp $(REGEX_TEST_SRC_DIR)parsing_err.cpp $(REGEX_TEST_SRC_DIR)test.cpp $(REGEX_TEST_SRC_DIR)other_ops.cpp $(REGEX_TEST_SRC_DIR)docs.cpp

MAKEFLAGS += -j4

all: $(OUT_EXE)libbrex.a $(BIN_DIR)brex $(BIN_DIR)brex_dbg $(PCKG_DIR)libbrex.a

$(PCKG_DIR)libbrex.a: $(COMMON_HEADERS) $(REGEX_HEADERS) $(PATH_HEADERS) $(OUT_EXE)libbrex.a
	@mkdir -p $(PCKG_DIR)
	@mkdir -p $(PCKG_DIR)regex/
	@mkdir -p $(PCKG_DIR)path/
	cp $(OUT_EXE)libbrex.a $(PCKG_DIR)libbrex.a
	cp $(COMMON_HEADERS) $(PCKG_DIR)
	cp $(REGEX_HEADERS) $(PCKG_DIR)regex/
	cp $(PATH_HEADERS) $(PCKG_DIR)path/

$(BIN_DIR)brex: $(COMMON_HEADERS) $(REGEX_HEADERS) $(OUT_EXE)libbrex.a $(RE_DIR)brex_cmd.cpp
	@mkdir -p $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -L$(LIB_PATH) $(JSON_INCLUDES) -o $(BIN_DIR)brex $(RE_DIR)brex_cmd.cpp $(OUT_EXE)libbrex.a

$(BIN_DIR)brex_dbg: $(COMMON_HEADERS) $(REGEX_HEADERS) $(OUT_EXE)libbrex.a $(RE_DIR)brex_dbg_host.cpp
	@mkdir -p $(BIN_DIR)
	$(CPP) $(CPPFLAGS) -L$(LIB_PATH) $(JSON_INCLUDES) -o $(BIN_DIR)brex_dbg $(RE_DIR)brex_dbg_host.cpp $(OUT_EXE)libbrex.a

$(OUT_EXE)libbrex.a: $(REGEX_OBJS) $(PATH_OBJS) $(COMMON_OBJS)
	@mkdir -p $(OUT_EXE)
	$(AR) $(ARFLAGS) $(OUT_EXE)libbrex.a $(REGEX_OBJS) $(PATH_OBJS) $(COMMON_OBJS)

$(OUT_OBJ)path_glob.o: $(COMMON_HEADERS) $(REGEX_HEADERS) $(PATH_HEADERS) $(PTH_DIR)path_glob.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)path_glob.o -c $(PTH_DIR)path_glob.cpp

$(OUT_OBJ)brex.o: $(COMMON_HEADERS) $(REGEX_HEADERS) $(RE_DIR)brex.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)brex.o -c $(RE_DIR)brex.cpp

$(OUT_OBJ)brex_compiler.o: $(COMMON_HEADERS) $(REGEX_HEADERS) $(RE_DIR)brex_compiler.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)brex_compiler.o -c $(RE_DIR)brex_compiler.cpp

$(OUT_OBJ)nfa_machine.o: $(COMMON_HEADERS) $(REGEX_HEADERS) $(RE_DIR)nfa_machine.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)nfa_machine.o -c $(RE_DIR)nfa_machine.cpp

$(OUT_OBJ)common.o: $(COMMON_HEADERS) $(SRC_DIR)common.cpp
	@mkdir -p $(OUT_OBJ)
	$(CPP) $(CPPFLAGS) $(JSON_INCLUDES) -o $(OUT_OBJ)common.o -c $(SRC_DIR)common.cpp

testfiles: $(COMMON_HEADERS) $(REGEX_HEADERS) $(OUT_EXE)libbrex.a $(REGEX_TEST_SOURCES)
	@mkdir -p $(BIN_DIR)
	$(CPP) $(CPPFLAGS_TEST) -L$(LIB_PATH) $(JSON_INCLUDES) -o $(BIN_DIR)regex_test $(REGEX_TEST_SOURCES) $(OUT_EXE)libbrex.a -lboost_unit_test_framework

test: testfiles
	$(BIN_DIR)regex_test --report_level=short --color_output

clean:
	rm -rf $(OUT_EXE)* $(OUT_OBJ)*.o $(BIN_DIR)*