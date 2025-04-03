CXX = g++
CXXFLAGS = -std=c++17 -Wall -I./src -I./include
LDFLAGS = 
SRC_DIR = src
BUILD_DIR = build
TARGET = toydb

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SRCS))

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$(TARGET) $(OBJS) $(LDFLAGS)

all: $(TARGET)

clean:
	rm -rf $(BUILD_DIR)

run: all
	./$(BUILD_DIR)/$(TARGET)