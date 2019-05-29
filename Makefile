CC		:= g++
C_FLAGS := -std=c++14 -Wall -Wextra -g -MMD -MP ${SANITIZE}
# TODO Use -fsanitize-address in C_FLAGS_COVER. -fsanitize=address changes behavior without reporting an compile-time error in test "Lexer::lex StartDirective StartBlock".
C_FLAGS_COVER := -std=c++14 -Wall -Wextra -g -O0 ${SANITIZE} --coverage -fno-exceptions -fno-inline

OBJ		:= obj
BIN		:= bin
SRC		:= src
INCLUDE	:= include
LIB		:= lib
TEST    := test

LIBRARIES	:=
LIBNAME     := carender

ifeq ($(OS),Windows_NT)
STATIC_LIB	:= $(LIBNAME).lib
TEST        := test.exe
else
STATIC_LIB	:= lib$(LIBNAME).a
TEST        := test
endif

all: dirmake $(LIB)/$(STATIC_LIB)

test: dirmake $(BIN)/$(TEST)

clean: dirmake
	$(RM) -rf $(OBJ)
	$(RM) -rf $(BIN)
	$(RM) -rf $(LIB)

DEPENDS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.d,$(wildcard $(SRC)/*.cpp))

SRCS := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(wildcard $(SRC)/*.cpp))
SRCS_TEST := $(patsubst $(TEST)/%.cpp,$(OBJ)/%.o,$(wildcard $(TEST)/*.cpp))

$(LIB)/$(STATIC_LIB): $(SRCS)
	ar -r -o $@ $^

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CC) $(C_FLAGS) -c -I$(INCLUDE) -L$(LIB) $< -o $@ $(LIBRARIES)

$(OBJ)/%.o: $(TEST)/%.cpp
	$(CC) $(C_FLAGS) -c -I$(INCLUDE) -L$(LIB) $< -o $@ $(LIBRARIES)

$(BIN)/$(TEST): $(LIB)/$(STATIC_LIB) $(SRCS_TEST)
	$(CC) $(C_FLAGS) -I$(INCLUDE) -L$(LIB) $(SRCS_TEST) -o $@ -lcarender $(LIBRARIES)

$(BIN)/$(TEST)-cover: $(SRCS_TEST) $(SRC)/*.cpp
	$(CC) $(C_FLAGS_COVER) -I$(INCLUDE) -L$(LIB) $(SRCS_TEST) $(SRC)/*.cpp -o $@ $(LIBRARIES)

dirmake:
	@mkdir -p $(OBJ)
	@mkdir -p $(LIB)
	@mkdir -p $(BIN)

cover: dirmake $(BIN)/$(TEST)-cover
	mv *.gcno $(BIN) > /dev/null || true
	lcov --base-directory $(BIN) --directory $(BIN) --zerocounters -q
	GCOV_PREFIX_STRIP=4 GCOV_PREFIX=$(BIN) $(BIN)/$(TEST)-cover
	rm $(BIN)/test_*.gc* > /dev/null || true
	lcov --base-directory $(BIN) --directory $(BIN) --capture --rc lcov_branch_coverage=1 --output-file $(BIN)/carender.covdata
	lcov --remove $(BIN)/carender.covdata "/usr*" --rc lcov_branch_coverage=1 -o $(BIN)/carender.covdata # remove output for external libraries
	rm -rf test_coverage
	genhtml -o test_coverage -t "carender test coverage" --branch-coverage --num-spaces 4 $(BIN)/carender.covdata

cover-show: cover
	firefox test_coverage/index.html

-include ${DEPENDS}