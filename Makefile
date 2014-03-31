TARGET = fft_an

CC = gcc
LDFLAGS = -lm

SRC = $(TARGET).c
SRC += fft.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)
	
$(TARGET): obj
	$(CC) -Wall -o $(TARGET) -g $(OBJ) $(LDFLAGS)
	strip $(TARGET) > /dev/null 2>&1
	#cp $(TARGET) ../bin

obj: $(SRC)
	$(CC) -Wall -c -g $(SRC) 
clean:
	rm -f *.o $(TARGET) 

