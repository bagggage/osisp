CXX=g++
RM=rm -f
CPPFLAGS=-g -O2 -I$(SRC) -std=c++11 -pedantic -W -Wall -Wextra -Werror
LDFLAGS=-g
SRC=src
BUILD=build
DIST=bin
PROG_NAME=dirwalk

SRCS=$(shell find $(SRC) -name '*.cpp')
OBJS=$(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SRCS))

all: $(PROG_NAME)

$(OBJS): $(SRCS)
	@mkdir -p $(@D)
	@echo $@
	@$(CXX) $(CPPFLAGS) -o $@ -c $(patsubst $(BUILD)/%.o, $(SRC)/%.cpp, $@)

$(PROG_NAME): $(OBJS)
	mkdir -p $(DIST)
	$(CXX) $(LDFLAGS) -o $(DIST)/$(PROG_NAME) $(OBJS)

run: $(PROG_NAME)
	./$(DIST)/$(PROG_NAME)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(DIST)/$(PROG_NAME)