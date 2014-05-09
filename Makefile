CFLAGS=-Wall -g -DNDEBUG

clean:
	find . \( ! -name "*.c" -a ! -name "*.h" -a ! -name "*Make*" -a ! -name "." \) -exec rm -f {} \;
