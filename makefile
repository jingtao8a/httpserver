CC = gcc
INCLUDE = -I ./
TARGET = server
OBJ = fileparse.o cmdparse.o shttpd.o \
	  shttpd_request.o shttpd_error.o shttpd_uri.o \
	  shttpd_mine.o  shttpd_method.o \
	  shttpd_worker.o
GDB = -g
.PHONY : all clean realclean

all : $(TARGET) clean

$(TARGET) : $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(GDB) -lpthread

shttpd.o : shttpd.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB) 

fileparse.o : fileparse.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

cmdparse.o : cmdparse.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttpd_request.o : shttpd_request.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttpd_error.o : shttpd_error.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttpd_uri.o : shttpd_uri.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttpd_mine.o : shttpd_mine.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttpd_cgi.o : shttpd_cgi.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttpd_method.o : shttpd_method.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

shttpd_worker.o : shttpd_worker.c
	$(CC) -o $@ -c $< $(INCLUDE) $(GDB)

clean:
	rm *.o
realclean:
	rm $(TARGET)
