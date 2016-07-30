
LIBNAME := $(notdir $(CURDIR))
OBJDIR := $(OBJDIR)/$(LIBNAME)
TARGET := $(LIBNAME).so


SRC_DIR := ${shell find ./ -type d -print}
VPATH := $(SOURCE_DIRS)

SRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
OBJ := $(addprefix $(OBJDIR)/,$(notdir $(SRC:.cpp=.o)))


JULIA =  /home/jvoigts/Documents/Github/julia/

# flags to compile "JuliaProcessor". $(JULIA): toplevel of julia package
CXXFLAGS += -I $(JULIA)/src -I $(JULIA)/src/support -I $(JULIA)/usr/include
LDFLAGS += -L $(JULIA)/usr/lib -Wl,-R $(JULIA)/usr/lib -ljulia

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
	@echo "Cleaning $(LIBNAME)"
	-@rm -rf $(OBJDIR)
	-@rm -f $(OUTDIR)/$(TARGET)

-include $(OBJ:%.o=%.d)
