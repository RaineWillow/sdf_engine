CXX:=g++
CFLAGS:= -march=native -Ofast
DEBUGFLAGS := -Og -ggdb
DEBUGGER := gdb

ifeq ($(OS),Windows_NT)
	OSNAME := windows
else
	OSNAME := $(shell uname -s)
endif

# Test if you are on windows or linux, which will change some values for compilation and running the program
ifeq ($(OSNAME), windows)
	LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -lopengl32
	TARGET_EXEC := final_program.exe
	RUNPRECONDITION := 
	RELEASEFLAGS := -Wl,-subsystem,windows
else
	LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -lGL
	TARGET_EXEC := final_program
	RUNPRECONDITION := nvidia-offload
	RELEASEFLAGS :=
endif


# Locations for all compiled CIMGUI files
CIMGUI := compiled/imgui/imgui.o compiled/imgui/imgui_widgets.o compiled/imgui/imgui_draw.o compiled/imgui/imgui_tables.o compiled/imgui/imgui_demo.o compiled/imgui/imgui-SFML.o


BUILD_DIR := ./build
SRC_DIRS := ./src

# Find all the C and C++ files we want to compile
SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')

# Prepends BUILD_DIR and appends .o to every src file
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find $(SRC_DIRS) -type d)
# Add a prefix to INC_DIRS
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
CPPFLAGS := $(INC_FLAGS) -MMD -MP



# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) $(CIMGUI) -o $@ $(LIBS)

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

.PHONY: run
run: $(BUILD_DIR)/$(TARGET_EXEC)
	$(RUNPRECONDITION) $<

.PHONY: clean
clean:
	rm -r $(BUILD_DIR)


debug: CFLAGS = $(DEBUGFLAGS)
debug: clean $(BUILD_DIR)/$(TARGET_EXEC)
	$(RUNPRECONDITION) $(DEBUGGER) $(BUILD_DIR)/$(TARGET_EXEC)

release: CFLAGS += $(RELEASEFLAGS)
release: clean $(BUILD_DIR)/$(TARGET_EXEC)

.PHONY: cimgui
cimgui:
	mkdir -p compiled/imgui
	$(CXX) $(CFLAGS) -o compiled/imgui/imgui.o         -c imgui/imgui.cpp
	$(CXX) $(CFLAGS) -o compiled/imgui/imgui_widgets.o -c imgui/imgui_widgets.cpp
	$(CXX) $(CFLAGS) -o compiled/imgui/imgui_draw.o    -c imgui/imgui_draw.cpp
	$(CXX) $(CFLAGS) -o compiled/imgui/imgui_tables.o  -c imgui/imgui_tables.cpp
	$(CXX) $(CFLAGS) -o compiled/imgui/imgui_demo.o    -c imgui/imgui_demo.cpp
	$(CXX) $(CFLAGS) -o compiled/imgui/imgui-SFML.o    -c imgui/imgui-SFML.cpp

# Include the .d makefiles
-include $(DEPS)