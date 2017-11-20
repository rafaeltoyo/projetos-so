CC		= gcc 
CFLAGS	= -g -Wall
HEADERS = datatypes.h pingpong.h queue.h
MAINFILE1 = pingpong-semaphore.o
MAINFILE2 = pingpong-racecond.o
OBJECTS = pingpong.o queue.o
PROJECTNAME1 = projeto10-1
PROJECTNAME2 = projeto10-2

debug: DEBUG = -DDEBUG

default: $(PROJECTNAME1)

all: $(MAINFILE1) $(MAINFILE2) $(OBJECTS)
	$(CC) $(MAINFILE1) $(OBJECTS) -o $(PROJECTNAME1) $(CFLAGS)
	$(CC) $(MAINFILE2) $(OBJECTS) -o $(PROJECTNAME2) $(CFLAGS)

debug: all

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< $(DEBUG) -o $@

$(PROJECTNAME1): $(MAINFILE1) $(OBJECTS)
	$(CC) $(MAINFILE1) $(OBJECTS) -o $@ $(CFLAGS)

$(PROJECTNAME2): $(MAINFILE2) $(OBJECTS)
	$(CC) $(MAINFILE2) $(OBJECTS) -o $@ $(CFLAGS)

# limpar os executáveis
clean:
	rm -f $(MAINFILE1) $(MAINFILE2) $(OBJECTS)
	rm -f $(PROJECTNAME1) $(PROJECTNAME2)

