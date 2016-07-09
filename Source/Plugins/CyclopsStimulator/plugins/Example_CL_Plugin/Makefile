
CL_PLUGIN_NAME := $(notdir $(CURDIR))
OBJDIR := $(OBJDIR)/$(CL_PLUGIN_NAME)
TARGET := $(CL_PLUGIN_NAME).so
OS := $(shell uname)

SRC_DIR := ${shell find ./ -type d -print}
# SOURCE_DIRS is not "empty"
VPATH := $(SOURCE_DIRS)

SRC := $(foreach sdir,$(SRC_DIR), $(wildcard $(sdir)/*.cpp))
# Don't enter plugins directory!
OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.cpp=.o)))

#Extra macros and libraries needed by the plugin
CXXFLAGS := $(CXXFLAGS) -Wall
#LDFLAGS := $(LDFLAGS) -lExampleLib



BLDCMD := $(CXX) -shared -o $(OUTDIR)/$(TARGET) $(OBJ) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)

VPATH = $(SRC_DIR)

.PHONY: objdir

$(OUTDIR)/$(TARGET): objdir $(OBJ)
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@echo "Building $(TARGET)"
	@$(BLDCMD)

$(OBJDIR)/%.o : %.cpp
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"
	
objdir:
	-@mkdir -p $(OBJDIR)

clean:
	@echo "Cleaning $(CL_PLUGIN_NAME)"
	-@rm -rf $(OBJDIR)
	-@rm -f $(OUTDIR)/$(TARGET)

-include $(OBJ:%.o=%.d)
