CC		= gcc 
CFLAGS	= -g -Wall -ANSII
HEADERS = queue.h
OBJECTS = testafila.o queue.o
PROJECTNAME = projeto00

default: $(PROJECTNAME)
all: default

# 
%.o: %.c $(HEADERS)
	$(CC) -c $< -o $@
$(PROJECTNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(CFLAGS) > debug-msg.txt
	rm -f $(OBJECTS)
# limpar o executável
clean:
	rm -f $(PROJECTNAME)
# rodar o executável
run: $(PROJECTNAME)
	./$(PROJECTNAME)
