all:
	cc -std=c99 -Wall main.c mpc.c lval.c -ledit -lm -o main
clean:
	rm main