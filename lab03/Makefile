CXX=gcc
RM=rm -f
CPPFLAGS=-g -O2 -I$(SRC) -std=c11 -pedantic -W -Wno-unused-variable -Wno-unused-parameter -Wall -Wextra
LDFLAGS=-g
SRC=src
BUILD=build
DIST=bin
PROG_NAME=parent
TARGET=$(DIST)/$(PROG_NAME)

SRCS=$(shell find $(SRC) -name '*.c')
OBJS=$(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SRCS))

all: $(PROG_NAME)

$(OBJS): $(SRCS)
	@mkdir -p $(@D)
	@echo $@
	@$(CXX) $(CPPFLAGS) -o $@ -c $(patsubst $(BUILD)/%.o, $(SRC)/%.c, $@)

$(TARGET):
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $(C_OBJS)

all: $(TARGET)

run: all
	./$(TARGET) environment.txt

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGET)