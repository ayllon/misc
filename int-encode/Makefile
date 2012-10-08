LDFLAGS=-laprutil-1

%.o: %.c
	$(CC) -c $< -o $@

encode: encode.o

clean:
	$(RM) *.o encode

