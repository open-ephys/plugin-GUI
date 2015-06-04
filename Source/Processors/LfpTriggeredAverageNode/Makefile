TYPE = sinks

ifeq ($(TARGET_ARCH),)
  TARGET_ARCH := -march=native
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

PLUGINDIR := /usr/local/lib/GUI
BINDIR := build
LIBDIR := build
OBJDIR := build/intermediate/Debug
OUTDIR := build
CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../../JuceLibraryCode
CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -fPIC -O0
CXXFLAGS += $(CFLAGS) 
LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -shared -L/usr/X11R6/lib/ -lGL -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt -lephys
LDDEPS :=
RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../../JuceLibraryCode
TARGET := LfpTriggeredAverageNode.so
BLDCMD = $(CXX) -shared -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)

OBJECTS := \
  $(OBJDIR)/LfpTriggeredAverageEditor.o \
  $(OBJDIR)/LfpTriggeredAverageNode.o \
  $(OBJDIR)/LfpTriggeredAverageCanvas.o \
  $(OBJDIR)/VisualizerEditor.o \

.PHONY: clean install

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

install:
	@echo ${TARGET} installed.
	-@sudo mv $(BINDIR)/$(TARGET) $(PLUGINDIR)/$(TYPE)/$(TARGET)

clean:
	@echo Plugin cleaned.
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)
	-@sudo rm -rf $(PLUGINDIR)/$(TYPE)/$(TARGET)

strip:
	@echo Plugin stripped.
	-@strip --strip-unneeded $(OUTDIR)/$(TARGET)

$(OBJDIR)/LfpTriggeredAverageEditor.o: LfpTriggeredAverageEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpTriggeredAverageEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpTriggeredAverageCanvas.o: LfpTriggeredAverageCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpTriggeredAverageCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpTriggeredAverageNode.o: LfpTriggeredAverageNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpTriggeredAverageNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VisualizerEditor.o: ../Editors/VisualizerEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling VisualizerEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
