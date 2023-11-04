GCC=g++
GCC_FLAGS=-Wall -lpthread -I include
INCLUDE_DIR=include
BUILD_DIR=build

.PHONY: all server client clean always

all: always server client

server: always
	@echo "Building server..."
	$(GCC) $(GCC_FLAGS) server/server.cpp $(INCLUDE_DIR)/newclasses.cpp -o $(BUILD_DIR)/server

client: always
	@echo "Building client..."
	$(GCC) $(GCC_FLAGS) client/client.cpp $(INCLUDE_DIR)/newclasses.cpp -o $(BUILD_DIR)/client

always:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)/*