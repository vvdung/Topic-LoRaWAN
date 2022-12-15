CC=g++
AR=ar
CFLAGS=-c -Wall -std=c++11 -DOPENSSL_API_COMPAT=0x10101000
INC=-I../inc 
#LIBS=-lbson-static-1.0 -lcrypto -lssl -lpthread -lrt -ldl -lz -lcurl
TARGET=libHFactor.a
OBJ_DIR=obj
OUT_DIR=../lib

SRC_FILES = HFactor.cpp EThread.cpp PoolThread.cpp ETimer.cpp \
			ESocket.cpp EConnect.cpp EQueueConnect.cpp EServer.cpp \
			EClient.cpp \
			ECrypto.cpp Ebson.cpp \
			MD5.cpp AES.cpp HS256.cpp JWT.cpp \
			MongoClient.cpp MongoDatabase.cpp MongoCollection.cpp \
			MongoClientPool.cpp
			
OBJ_FILES = $(SRC_FILES:%.cpp=$(OBJ_DIR)/%.o)
#OBJ_FILES = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

all: $(TARGET)	
	ls -l $(OUT_DIR)/$(TARGET)
	$(AR) -t $(OUT_DIR)/$(TARGET)
$(TARGET):$(OBJ_FILES)
	$(AR) -rcs $(OUT_DIR)/$(TARGET) $(OBJ_DIR)/*.o
$(OBJ_DIR)/%.o:%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ $<

clean:
	rm -f $(OBJ_DIR)/*.o $(OUT_DIR)/$(TARGET)
#g++ -I../inc -fPIC -O0 -g -Wall -c -std=c++11 -fpermissive -o obj/HFactor.o  HFactor.cpp
#ar -rcs ../lib/HFactor.a obj/*.o
#g++ test1.cpp -L../lib -lHFactor -o test1
#https://dev.to/iamkhalil11/all-you-need-to-know-about-c-static-libraries-1o0b