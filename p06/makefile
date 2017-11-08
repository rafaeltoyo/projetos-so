CC		= gcc 
CFLAGS	= -g -Wall
HEADERS = datatypes.h pingpong.h queue.h
OBJECTS = pingpong.o queue.o
PROJECT1 = pingpong-contab.o
PROJECT2 =pingpong-contab-prio.o
PROJECTNAME1 = projeto06
PROJECTNAME2 = projeto06-prio

debug: DEBUG = -DDEBUG

default: $(PROJECTNAME1)

debug: default

all: $(PROJECT1) $(PROJECT2) $(OBJECTS)
	$(CC) $(PROJECT1) $(OBJECTS) -o $(PROJECTNAME1) $(CFLAGS)
	$(CC) $(PROJECT2) $(OBJECTS) -o $(PROJECTNAME2) $(CFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< $(DEBUG) -o $@

$(PROJECTNAME1): $(PROJECT1) $(OBJECTS)
	$(CC) $(PROJECT1) $(OBJECTS) -o $@ $(CFLAGS)
$(PROJECTNAME2): $(PROJECT2) $(OBJECTS)
	$(CC) $(PROJECT2) $(OBJECTS) -o $@ $(CFLAGS)

# limpar os executáveis
clean:
	rm -f $(PROJECT1) $(PROJECT2) $(OBJECTS)
	rm -f $(PROJECTNAME1) $(PROJECTNAME2)

