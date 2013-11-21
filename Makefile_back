cc = gcc
TARGET = Tetris
OBJS = main.o shapes.o colors.o
SRCS = main.c
CFLAGS = -Wall -O -g -c
LIB = -lcdk -lpanel -lncurses

$(TARGET):$(OBJS)
$(cc) -o $(TARGET) $(OBJS) $(LIB)
main.o:main.c tetris.h colors.h shapes.h
$(cc) $(CFLAGS) main.c
shapes.o:shapes.c shapes.h tetris.h 
$(cc) $(CFLAGS) shapes.c
colors.o:colors.c tetris.h
$(cc) $(CFLAGS) colors.c
.PHONY:clean
clean:
rm -rf *.o $(TARGET)
