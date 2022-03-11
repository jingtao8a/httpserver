CC = gcc
INCLUDE = -I ./
TARGET = server
OBJ = fileparse.o cmdparse.o shttpd.o shttp_request.o shttp_error.o shttp_uri.o
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

shttp_request.o : shttp_request.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttp_error.o : shttp_error.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttp_uri.o : shttp_uri.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

clean:
	rm *.o
realclean:
	rm $(TARGET)
