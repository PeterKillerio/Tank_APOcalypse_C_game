CFLAGS+= -Wall -Werror -std=gnu99 -g
LDFLAGS= -lm -lSDL2

CFLAGS+=$(shell sdl2-config --cflags)

HW=mzapo
BINARIES=mzapo

LDFLAGS+=$(shell sdl2-config --libs)

all: ${BINARIES}

OBJS=${patsubst %.c,%.o,${wildcard *.c}}

mzapo: ${OBJS}
	${CC} ${OBJS} ${LDFLAGS} -o $@

${OBJS}: %.o: %.c
	${CC} -c ${CFLAGS} $< -o $@
clean:
	rm -f ${BINARIES} ${OBJS}
	rm -f ${HW}-brute.zip
