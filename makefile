CC = gcc
INCLUDE = -I ./
TARGET = server
OBJ = fileparse.o cmdparse.o shttpd.o
GDB = -g
.PHONY : all clean realclean

all : $(TARGET) clean

$(TARGET) : $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(GDB)

shttpd.o : shttpd.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB) 

fileparse.o : fileparse.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

cmdparse.o : cmdparse.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

clean:
	rm *.o
realclean:
	rm $(TARGET)
