# List your *.h files (if you do not have them in your project then leave the variable "headers" empty):
headers =

# List your *.c files:
sources = banking_system.c

# Specify name of your program:
executable = banking_system

$(executable): $(headers) $(sources)
	gcc -g -Wall -pedantic $(sources) -o $(executable)

.PHONY: clean
clean:
	rm $(executable)

.PHONY: check
check: $(executable)
	valgrind --leak-check=full --track-origins=yes ./$(executable)