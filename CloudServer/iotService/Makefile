CC=g++
CFLAGS=-c -Wall -std=c++11 
#-DOPENSSL_API_COMPAT=0x10101000
INC=-I../inc
LIBS= -L../lib -lHFactor -lmongoc-static-1.0 -lbson-static-1.0 -lcurl -lssl -lcrypto -lpthread -lrt -ldl -lz -lresolv -lsasl2 -lsnappy
OBJ_DIR=obj
#TARGET=HFRandNode
TARGET=HFServer

#

SRC_FILES = main.cpp \
			MainThread.cpp ApiHFactor.cpp HttpClient.cpp \
			HFactorClientPool.cpp HFactorClient.cpp \
			HFactorDatabase.cpp \
			UDPConnect.cpp UDPServer.cpp

OBJ_FILES = $(SRC_FILES:%.cpp=$(OBJ_DIR)/%.o)

all: $(TARGET)	
	ls -l $(TARGET)
$(TARGET):$(OBJ_FILES)
	$(CC) -o $(TARGET) $(OBJ_DIR)/*.o $(LIBS)
$(OBJ_DIR)/%.o:%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)