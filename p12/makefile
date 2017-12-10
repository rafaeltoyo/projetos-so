CC		= gcc 
CFLAGS	= -g -Wall -lm
HEADERS = datatypes.h pingpong.h queue.h
OBJECTS = pingpong-mqueue.o pingpong.o queue.o
PROJECTNAME = projeto12

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

