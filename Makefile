#vesion 1
#objects = main.o colors.o shapes.o 
#brick:$(objects) 
#	cc -o brick main.o colors.o shapes.o -lcurses 
#
#main.o:main.c colors.h shapes.h tetris.h
#	cc -c main.c
#
#colors.o:colors.c colors.h
#	cc -c colors.c
#
#shapes.o: shapes.c shapes.h tetris.h
#	cc -c shapes.c
#
#clean:
#	rm $(objects) 

#vesion 2	
#CC:=gcc
objects = main.o colors.o shapes.o 

all: dirs brick $(objects)

dirs:
	mkdir -p obj
	mkdir -p bin

brick:$(objects) 
	cc -o ./bin/brick main.o colors.o shapes.o -lcurses -g

main.o:colors.h shapes.h tetris.h

colors.o:colors.h

shapes.o: shapes.h tetris.h

#main.o colors.o: colors.h
#main.o shapes.o: shapes.h tetris.h 

.PHONY:clean
clean:
	-rm $(objects) 

