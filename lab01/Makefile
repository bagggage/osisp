CXX=g++
RM=rm -f
CPPFLAGS=-g -O2 -I$(SRC) -std=c++20
LDFLAGS=-g
SRC=src
BUILD=build
DIST=bin

SRCS=$(shell find $(SRC) -name '*.cpp')
OBJS=$(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SRCS))

all: dirwalk

$(OBJS): $(SRCS)
	mkdir -p $(@D)
	@echo > $@
	$(CXX) $(CPPFLAGS) -o $@ -c $(patsubst $(BUILD)/%.o, $(SRC)/%.cpp, $@)

dirwalk: $(OBJS)
	mkdir -p $(DIST)
	$(CXX) $(LDFLAGS) -o $(DIST)/dirwalk $(OBJS)

run: dirwalk
	./$(DIST)/dirwalk

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(DIST)/dirwalk