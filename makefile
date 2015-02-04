# Hugh McDonald
# 12/5/13

PROG = compute

CC = icc
CFLAGS = -std=c99 -Wall -pthread

${PROG}: ${PROG}.c
	${CC} ${CFLAGS} ${@}.c -o ${@}
