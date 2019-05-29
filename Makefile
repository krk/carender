CC      := $(if $(CC),$(CC),gcc)
C_FLAGS := -std=c++14 -Wall -Wextra -O3 -g -MMD -MP ${SANITIZE}
C_FLAGS_COVER := -std=c++14 -Wall -Wextra -O3 -g ${SANITIZE} --coverage -fno-exceptions -fno-inline

OBJ		:= obj
BIN		:= bin
SRC		:= src
INCLUDE	:= include
LIB		:= lib
TEST    := test
CMD     := cmd

LIBRARIES	:=
LIBNAME     := carender

ifeq ($(OS),Windows_NT)
STATIC_LIB     := $(LIBNAME).lib
TEST_BIN       := test.exe
TEST_COVER_BIN := test-cover.exe
CMD_BIN        := carender.exe
else
STATIC_LIB	   := lib$(LIBNAME).a
TEST_BIN       := test
TEST_COVER_BIN := test-cover
CMD_BIN        := carender
endif

lib: dirmake $(LIB)/$(STATIC_LIB)

all: lib test cmd

test: dirmake $(BIN)/$(TEST_BIN)

cmd: dirmake $(BIN)/$(CMD_BIN)

clean: dirmake
	$(RM) -rf $(OBJ)
	$(RM) -rf $(BIN)
	$(RM) -rf $(LIB)

DEPENDS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$(wildcard $(SRC)/*.cpp))

SRCS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))
SRCS_TEST := $(patsubst $(TEST)/%.cpp,$(OBJ)/%.o,$(wildcard $(TEST)/*.cpp))
SRCS_CMD := $(patsubst $(CMD)/%.cpp,$(OBJ)/%.o,$(wildcard $(CMD)/*.cpp))

$(LIB)/$(STATIC_LIB): $(SRCS)
	ar -r -o $@ $^

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(C_FLAGS) -c -I$(INCLUDE) -L$(LIB) $< -o $@ $(LIBRARIES)

$(OBJ)/%.o: $(TEST)/%.cpp
	$(CC) $(C_FLAGS) -c -I$(INCLUDE) -L$(LIB) $< -o $@ $(LIBRARIES)

$(OBJ)/%.o: $(CMD)/%.cpp
	$(CC) $(C_FLAGS) -c -I$(INCLUDE) -L$(LIB) $< -o $@ $(LIBRARIES)

$(BIN)/$(TEST_BIN): $(LIB)/$(STATIC_LIB) $(SRCS_TEST)
	$(CC) $(C_FLAGS) -I$(INCLUDE) -L$(LIB) $(SRCS_TEST) -o $@ -lcarender $(LIBRARIES)

$(BIN)/$(CMD_BIN): $(LIB)/$(STATIC_LIB) $(SRCS_CMD)
	$(CC) $(C_FLAGS) -I$(INCLUDE) -L$(LIB) $(SRCS_CMD) -o $@ -lcarender $(LIBRARIES)

$(BIN)/$(TEST_COVER_BIN): $(SRCS_TEST) $(SRC)/*.cpp
	$(CC) $(C_FLAGS_COVER) -I$(INCLUDE) -L$(LIB) $(SRCS_TEST) $(SRC)/*.cpp -o $@ $(LIBRARIES)

dirmake:
	@mkdir -p $(OBJ)
	@mkdir -p $(LIB)
	@mkdir -p $(BIN)

cover: dirmake $(BIN)/$(TEST_COVER_BIN)
	mv *.gcno $(BIN) > /dev/null || true
	lcov --base-directory $(BIN) --directory $(BIN) --zerocounters -q
	GCOV_PREFIX_STRIP=4 GCOV_PREFIX=$(BIN) $(BIN)/$(TEST_COVER_BIN)
	rm $(BIN)/test_*.gc* > /dev/null || true
	lcov --base-directory $(BIN) --directory $(BIN) --capture --rc lcov_branch_coverage=1 --output-file $(BIN)/carender.covdata
	lcov --remove $(BIN)/carender.covdata "/usr*" --rc lcov_branch_coverage=1 -o $(BIN)/carender.covdata # remove output for external libraries
	rm -rf test_coverage
	genhtml -o test_coverage -t "carender test coverage" --branch-coverage --num-spaces 4 $(BIN)/carender.covdata

cover-show: cover
	firefox test_coverage/index.html

-include ${DEPENDS}