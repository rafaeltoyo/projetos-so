CC		= gcc 
CFLAGS	= -g -Wall
HEADERS = datatypes.h pingpong.h
OBJECTS = pingpong.o
TASK01  = pingpong-tasks1.o
TASK02  = pingpong-tasks2.o
TASK03  = pingpong-tasks3.o
PROJECTNAME1 = projeto02-task01
PROJECTNAME2 = projeto02-task02
PROJECTNAME3 = projeto02-task03

debug: DEBUG = -DDEBUG

default: $(PROJECTNAME1)

debug: default

all: $(TASK01) $(TASK02) $(TASK03) $(OBJECTS)
	$(CC) $(TASK01)  $(OBJECTS) -o $(PROJECTNAME1) $(CFLAGS)
	$(CC) $(TASK02)  $(OBJECTS) -o $(PROJECTNAME2) $(CFLAGS)
	$(CC) $(TASK03)  $(OBJECTS) -o $(PROJECTNAME3) $(CFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< $(DEBUG) -o $@

$(PROJECTNAME1): $(TASK01) $(OBJECTS)
	$(CC) $(TASK01) $(OBJECTS) -o $@ $(CFLAGS)

$(PROJECTNAME2): $(TASK02) $(OBJECTS)
	$(CC) $(TASK02) $(OBJECTS) -o $@ $(CFLAGS)

$(PROJECTNAME3): $(TASK03) $(OBJECTS)
	$(CC) $(TASK03) $(OBJECTS) -o $@ $(CFLAGS)

# limpar os executáveis
clean:
	rm -f $(TASK1) $(TASK2) $(TASK3)  $(OBJECTS)
	rm -f $(PROJECTNAME1) $(PROJECTNAME2) $(PROJECTNAME3)

