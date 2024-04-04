CXX=gcc
RM=rm -f
CPPFLAGS=-g -O2 -I$(SRC) -std=c11 -pedantic -W -Wno-unused-variable -Wno-unused-parameter -Wall -Wextra
LDFLAGS=-g
SRC=src
BUILD=build
DIST=bin
P_PROG_NAME=parent
C_PROG_NAME=child
P_TARGET=$(DIST)/parent
C_TARGET=$(DIST)/child

SRCS=$(shell find $(SRC) -name '*.c')
OBJS=$(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SRCS))
P_OBJS=$(shell find $(BUILD)/parent -name '*.o')
C_OBJS=$(shell find $(BUILD)/child -name '*.o')

all: $(PROG_NAME)

$(OBJS): $(SRCS)
	@mkdir -p $(@D)
	@echo $@
	@$(CXX) $(CPPFLAGS) -o $@ -c $(patsubst $(BUILD)/%.o, $(SRC)/%.c, $@)

$(P_TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $(P_OBJS)

$(C_TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $(C_OBJS)

all: $(P_TARGET) $(C_TARGET)

export CHILD_PATH=$(C_TARGET)

run: all
	./$(DIST)/$(P_PROG_NAME) environment.txt

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(DIST)/$(P_PROG_NAME)
	$(RM) $(DIST)/$(C_PROG_NAME)