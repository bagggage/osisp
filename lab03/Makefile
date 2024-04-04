CXX=gcc
RM=rm -f
CPPFLAGS=-g -O2 -I$(SRC) -std=c11 -pedantic -W -Wno-unused-variable -Wno-unused-parameter -Wall -Wextra
LDFLAGS=-g
SRC=src
BUILD=build
DIST=bin
PROG_NAME=lab
TARGET=$(DIST)/$(PROG_NAME)

SRCS=$(shell find $(SRC) -name '*.c')
OBJS=$(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SRCS))

$(OBJS): $(SRCS)
	@mkdir -p $(@D)
	@echo $@
	@$(CXX) $(CPPFLAGS) -o $@ -c $(patsubst $(BUILD)/%.o, $(SRC)/%.c, $@)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS)

all: $(TARGET)

run: all
	./$(TARGET)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) $(TARGET)