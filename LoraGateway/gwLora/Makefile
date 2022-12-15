CC=g++
CFLAGS=-c -Wall -std=c++11 
#-DOPENSSL_API_COMPAT=0x10101000
INC=-I../inc
LIBS= -L../lib -lHFactor -lwiringPi -lbson-static-1.0 -lssl -lcrypto -lpthread -lrt -ldl -lz
OBJ_DIR=obj
TARGET=gwLora


SRC_FILES = main.cpp \
			MainThread.cpp  UDPClient.cpp \
			LoRa.cpp Print.cpp WString.cpp itoa.cpp

OBJ_FILES = $(SRC_FILES:%.cpp=$(OBJ_DIR)/%.o)

all: $(TARGET)	
	ls -l $(TARGET)
$(TARGET):$(OBJ_FILES)
	$(CC) -o $(TARGET) $(OBJ_DIR)/*.o $(LIBS)
$(OBJ_DIR)/%.o:%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)