CC		= gcc 
CFLAGS	= -g -Wall
HEADERS = datatypes.h pingpong.h queue.h
OBJECTS = pingpong-barrier.o pingpong.o queue.o
PROJECTNAME = projeto11

debug: DEBUG = -DDEBUG

default: $(PROJECTNAME)

debug: default

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< $(DEBUG) -o $@

$(PROJECTNAME): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(CFLAGS)

# limpar os executáveis
clean:
	rm -f $(OBJECTS)
	rm -f $(PROJECTNAME)

