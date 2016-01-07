
CFLAGS := -Wall -Werror

all: relayctl

relayctl: relayctl.c
	${CC} $< -o $@ ${CFLAGS} ${LDFLAGS}

