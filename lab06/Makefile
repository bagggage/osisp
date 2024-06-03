CXX=gcc
RM=rm -r
CPPFLAGS=-g -O2 -I$(SRC) -std=c11 -pedantic -W -Wno-unused-variable -Wno-unused-parameter -Wall -Wextra -Wno-unused-result
LDFLAGS=-g
SRC=src
BUILD=build
DIST=bin

reader:
	@mkdir -p ${BUILD}
	@mkdir -p ${DIST}
	$(CXX) $(CPPFLAGS) -o ${DIST}/reader ${SRC}/reader.c

generator:
	@mkdir -p ${BUILD}
	@mkdir -p ${DIST}
	$(CXX) $(CPPFLAGS) -o ${DIST}/generator ${SRC}/generator.c

main:
	@mkdir -p ${BUILD}
	@mkdir -p ${DIST}
	$(CXX) $(CPPFLAGS) -o ${DIST}/sort_index ${SRC}/main.c -lpthread

all: reader generator main

clean:
	$(RM) $(BUILD)
	$(RM) $(DIST)