CC		= gcc 
CFLAGS	= -g -Wall
HEADERS = datatypes.h pingpong.h queue.h
OBJECTS = pingpong-scheduler.o pingpong.o queue.o
PROJECTNAME = projeto04

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

