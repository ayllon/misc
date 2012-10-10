CC=gcc -g

%.o: %.c
	$(CC) -c $< -o $@

encode: encode.o

clean:
	$(RM) *.o encode

