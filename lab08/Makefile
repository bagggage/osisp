CC=gcc
RM=rm -f
CC_FLAGS=-g -O2 -std=c11 -pedantic -W -Wno-unused-variable -Wno-unused-parameter -Wall -Wextra
LDFLAGS=-g -lpthread
BUILD=build
DIST=bin
SRC=src

SRC_APP=${SRC}/${PROG_NAME}
BUILD_APP=${BUILD}/${PROG_NAME}
OUT=${DIST}/${PROG_NAME}

CC_INCLUDE:=-I${SRC_APP}

SRCS:=$(shell find ${SRC_APP} -name '*.c')
OBJS:=$(patsubst ${SRC_APP}/%.c, ${BUILD_APP}/%.o, ${SRCS})

${OBJS}: ${SRCS}
	@mkdir -p ${@D}
	@${CC} ${CC_FLAGS} ${CC_INCLUDE} -c $(patsubst ${BUILD_APP}/%.o, ${SRC_APP}/%.c, $@) -o $@

bin: ${OBJS}
	@echo Linking: ${PROG_NAME}
	@mkdir -p @{DIST}
	@${CC} -o ${OUT} ${OBJS}

PROGS_DIR:=${shell find -maxdepth 1 -type d -not -path . -printf '%f\n'}

all:
	@for dir in ${PROGS_DIR}; do make -s bin PROG_NAME=$${dir}; done
