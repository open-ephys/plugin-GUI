# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(TARGET_ARCH),)
  TARGET_ARCH := -march=native
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O3
  CXXFLAGS += $(CFLAGS) -export-dynamic -g -pg
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L/usr/X11R6/lib/ -lGL -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt -lftdi -pg -ldl -lXext
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  TARGET := open-ephys
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -O3
  CXXFLAGS += $(CFLAGS) -export-dynamic -g -pg
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L/usr/X11R6/lib/ -lGL -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt -lftdi -pg -ldl -lXext
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  TARGET := open-ephys-release
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/AccessClass_de9602d5.o \
  $(OBJDIR)/PracticalSocket_2574ecc8.o \
  $(OBJDIR)/Bessel_63b88b81.o \
  $(OBJDIR)/Biquad_479045c5.o \
  $(OBJDIR)/Butterworth_a752fb81.o \
  $(OBJDIR)/Cascade_7ab417a7.o \
  $(OBJDIR)/ChebyshevI_f2af4063.o \
  $(OBJDIR)/ChebyshevII_649cfe90.o \
  $(OBJDIR)/Custom_e24db5a8.o \
  $(OBJDIR)/Design_3ed2fd75.o \
  $(OBJDIR)/Documentation_c5f0da2d.o \
  $(OBJDIR)/Elliptic_8ad9a5b9.o \
  $(OBJDIR)/Filter_e4029a2f.o \
  $(OBJDIR)/Legendre_89451537.o \
  $(OBJDIR)/Param_13628c80.o \
  $(OBJDIR)/PoleFilter_e04f707.o \
  $(OBJDIR)/RBJ_418b612d.o \
  $(OBJDIR)/RootFinder_239a995f.o \
  $(OBJDIR)/State_22979684.o \
  $(OBJDIR)/AudioComponent_521bd9c9.o \
  $(OBJDIR)/LfpTriggeredAverageNode_ff52d7b9.o \
  $(OBJDIR)/FileReader_18023b0e.o \
  $(OBJDIR)/ChannelMappingNode_d9219b9c.o \
  $(OBJDIR)/PulsePalOutput_9f4ef492.o \
  $(OBJDIR)/ReferenceNode_519d3b68.o \
  $(OBJDIR)/PhaseDetector_7193f7dc.o \
  $(OBJDIR)/AudioResamplingNode_3dbffa2d.o \
  $(OBJDIR)/Channel_33a3e5fe.o \
  $(OBJDIR)/PulsePal_14932a18.o \
  $(OBJDIR)/ofArduino_12f202a5.o \
  $(OBJDIR)/ofSerial_c3b0a9e1.o \
  $(OBJDIR)/EventDetector_4807aa7b.o \
  $(OBJDIR)/FPGAOutput_73b5ffd4.o \
  $(OBJDIR)/ArduinoOutput_391e90c4.o \
  $(OBJDIR)/Parameter_ae008024.o \
  $(OBJDIR)/SpikeDisplayNode_9c52e4ad.o \
  $(OBJDIR)/WiFiOutput_fa464ec5.o \
  $(OBJDIR)/LfpDisplayNode_30ef22fd.o \
  $(OBJDIR)/RecordControl_3e1c6062.o \
  $(OBJDIR)/Merger_2f90542e.o \
  $(OBJDIR)/Splitter_6e27a57b.o \
  $(OBJDIR)/LfpTriggeredAverageCanvas_50148944.o \
  $(OBJDIR)/SpikeObject_24e8c655.o \
  $(OBJDIR)/SpikeDisplayCanvas_b208ff6e.o \
  $(OBJDIR)/DataWindow_83ce6754.o \
  $(OBJDIR)/LfpDisplayCanvas_4a58e87e.o \
  $(OBJDIR)/OpenGLCanvas_3c775a41.o \
  $(OBJDIR)/SpikeDetector_300d85e7.o \
  $(OBJDIR)/AudioNode_94606ff3.o \
  $(OBJDIR)/EventNode_95c842b7.o \
  $(OBJDIR)/LfpTriggeredAverageEditor_101d75b.o \
  $(OBJDIR)/ChannelMappingEditor_dd803b0.o \
  $(OBJDIR)/FileReaderEditor_16fd7744.o \
  $(OBJDIR)/PhaseDetectorEditor_f53aa520.o \
  $(OBJDIR)/PulsePalOutputEditor_ab5be148.o \
  $(OBJDIR)/RHD2000Editor_d7ddd6b.o \
  $(OBJDIR)/RecordControlEditor_244084cb.o \
  $(OBJDIR)/ReferenceNodeEditor_19ee0fac.o \
  $(OBJDIR)/ResamplingNodeEditor_f2d6f061.o \
  $(OBJDIR)/FPGAOutputEditor_cfe00f8a.o \
  $(OBJDIR)/ArduinoOutputEditor_1118f008.o \
  $(OBJDIR)/ChannelSelector_c1430874.o \
  $(OBJDIR)/ParameterEditor_91f62ae8.o \
  $(OBJDIR)/SpikeDisplayEditor_e1812c1.o \
  $(OBJDIR)/VisualizerEditor_3672b003.o \
  $(OBJDIR)/MergerEditor_d1fcc0df.o \
  $(OBJDIR)/ImageIcon_c89b23a6.o \
  $(OBJDIR)/WiFiOutputEditor_1d3ba5bb.o \
  $(OBJDIR)/EventNodeEditor_2198733b.o \
  $(OBJDIR)/SignalGeneratorEditor_3223ac0a.o \
  $(OBJDIR)/LfpDisplayEditor_99d75351.o \
  $(OBJDIR)/SourceNodeEditor_8d813122.o \
  $(OBJDIR)/SplitterEditor_c2199b6c.o \
  $(OBJDIR)/SpikeDetectorEditor_2fb63eeb.o \
  $(OBJDIR)/AudioEditor_fb2c6555.o \
  $(OBJDIR)/FilterEditor_dfe1f39d.o \
  $(OBJDIR)/GenericEditor_becb2ad6.o \
  $(OBJDIR)/okFrontPanelDLL_87687880.o \
  $(OBJDIR)/rhd2000datablock_722d8dae.o \
  $(OBJDIR)/rhd2000evalboard_e0b412d5.o \
  $(OBJDIR)/rhd2000registers_cf6cd63b.o \
  $(OBJDIR)/RHD2000Thread_23e0b041.o \
  $(OBJDIR)/FileReaderThread_933ea08.o \
  $(OBJDIR)/FPGAThread_a8dc34ed.o \
  $(OBJDIR)/DataBuffer_6ae4f549.o \
  $(OBJDIR)/IntanThread_32126b71.o \
  $(OBJDIR)/DataThread_b2a47a13.o \
  $(OBJDIR)/RecordNode_2b7a1a2.o \
  $(OBJDIR)/SignalGenerator_a9cf4806.o \
  $(OBJDIR)/ResamplingNode_27a58a6b.o \
  $(OBJDIR)/FilterNode_817e9c9.o \
  $(OBJDIR)/SourceNode_c2d6336c.o \
  $(OBJDIR)/GenericProcessor_733760aa.o \
  $(OBJDIR)/ProcessorGraph_68b34a0b.o \
  $(OBJDIR)/EditorViewportButtons_29af2a5c.o \
  $(OBJDIR)/SignalChainManager_d2b643f0.o \
  $(OBJDIR)/EditorViewport_1d991caf.o \
  $(OBJDIR)/ProcessorList_1ad3f3de.o \
  $(OBJDIR)/CustomLookAndFeel_53a8fcdb.o \
  $(OBJDIR)/InfoLabel_a2051bf4.o \
  $(OBJDIR)/DataViewport_2cf95d2c.o \
  $(OBJDIR)/MessageCenter_748a1cca.o \
  $(OBJDIR)/ControlPanel_a895ede3.o \
  $(OBJDIR)/UIComponent_d667ba37.o \
  $(OBJDIR)/MainWindow_499ac812.o \
  $(OBJDIR)/Main_90ebc5c2.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/juce_audio_basics_2442e4ea.o \
  $(OBJDIR)/juce_audio_devices_a4c8a728.o \
  $(OBJDIR)/juce_audio_formats_d349f0c8.o \
  $(OBJDIR)/juce_audio_processors_44a134a2.o \
  $(OBJDIR)/juce_audio_utils_f63b12e8.o \
  $(OBJDIR)/juce_core_aff681cc.o \
  $(OBJDIR)/juce_cryptography_25c7e826.o \
  $(OBJDIR)/juce_data_structures_bdd6d488.o \
  $(OBJDIR)/juce_events_79b2840.o \
  $(OBJDIR)/juce_graphics_c8f1e7a4.o \
  $(OBJDIR)/juce_gui_basics_a630dd20.o \
  $(OBJDIR)/juce_gui_extra_7767d6a8.o \
  $(OBJDIR)/juce_opengl_c7e3506c.o \
  $(OBJDIR)/juce_video_184321c8.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking open-ephys
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning open-ephys
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

strip:
	@echo Stripping open-ephys
	-@strip --strip-unneeded $(OUTDIR)/$(TARGET)

$(OBJDIR)/AccessClass_de9602d5.o: ../../Source/AccessClass.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AccessClass.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PracticalSocket_2574ecc8.o: ../../Source/Network/PracticalSocket.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PracticalSocket.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Bessel_63b88b81.o: ../../Source/Dsp/Bessel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Bessel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Biquad_479045c5.o: ../../Source/Dsp/Biquad.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Biquad.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Butterworth_a752fb81.o: ../../Source/Dsp/Butterworth.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Butterworth.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Cascade_7ab417a7.o: ../../Source/Dsp/Cascade.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Cascade.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChebyshevI_f2af4063.o: ../../Source/Dsp/ChebyshevI.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChebyshevI.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChebyshevII_649cfe90.o: ../../Source/Dsp/ChebyshevII.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChebyshevII.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Custom_e24db5a8.o: ../../Source/Dsp/Custom.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Custom.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Design_3ed2fd75.o: ../../Source/Dsp/Design.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Design.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Documentation_c5f0da2d.o: ../../Source/Dsp/Documentation.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Documentation.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Elliptic_8ad9a5b9.o: ../../Source/Dsp/Elliptic.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Elliptic.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Filter_e4029a2f.o: ../../Source/Dsp/Filter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Filter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Legendre_89451537.o: ../../Source/Dsp/Legendre.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Legendre.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Param_13628c80.o: ../../Source/Dsp/Param.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Param.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PoleFilter_e04f707.o: ../../Source/Dsp/PoleFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PoleFilter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RBJ_418b612d.o: ../../Source/Dsp/RBJ.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RBJ.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RootFinder_239a995f.o: ../../Source/Dsp/RootFinder.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RootFinder.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/State_22979684.o: ../../Source/Dsp/State.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling State.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioComponent_521bd9c9.o: ../../Source/Audio/AudioComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpTriggeredAverageNode_ff52d7b9.o: ../../Source/Processors/LfpTriggeredAverageNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpTriggeredAverageNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FileReader_18023b0e.o: ../../Source/Processors/FileReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FileReader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelMappingNode_d9219b9c.o: ../../Source/Processors/ChannelMappingNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChannelMappingNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PulsePalOutput_9f4ef492.o: ../../Source/Processors/PulsePalOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PulsePalOutput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ReferenceNode_519d3b68.o: ../../Source/Processors/ReferenceNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ReferenceNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PhaseDetector_7193f7dc.o: ../../Source/Processors/PhaseDetector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PhaseDetector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioResamplingNode_3dbffa2d.o: ../../Source/Processors/AudioResamplingNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioResamplingNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Channel_33a3e5fe.o: ../../Source/Processors/Channel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Channel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PulsePal_14932a18.o: ../../Source/Processors/Serial/PulsePal.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PulsePal.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ofArduino_12f202a5.o: ../../Source/Processors/Serial/ofArduino.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ofArduino.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ofSerial_c3b0a9e1.o: ../../Source/Processors/Serial/ofSerial.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ofSerial.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EventDetector_4807aa7b.o: ../../Source/Processors/EventDetector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EventDetector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FPGAOutput_73b5ffd4.o: ../../Source/Processors/FPGAOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FPGAOutput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ArduinoOutput_391e90c4.o: ../../Source/Processors/ArduinoOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ArduinoOutput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Parameter_ae008024.o: ../../Source/Processors/Parameter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Parameter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDisplayNode_9c52e4ad.o: ../../Source/Processors/SpikeDisplayNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDisplayNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/WiFiOutput_fa464ec5.o: ../../Source/Processors/WiFiOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling WiFiOutput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpDisplayNode_30ef22fd.o: ../../Source/Processors/LfpDisplayNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpDisplayNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RecordControl_3e1c6062.o: ../../Source/Processors/Utilities/RecordControl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RecordControl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Merger_2f90542e.o: ../../Source/Processors/Utilities/Merger.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Merger.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Splitter_6e27a57b.o: ../../Source/Processors/Utilities/Splitter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Splitter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpTriggeredAverageCanvas_50148944.o: ../../Source/Processors/Visualization/LfpTriggeredAverageCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpTriggeredAverageCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeObject_24e8c655.o: ../../Source/Processors/Visualization/SpikeObject.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeObject.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDisplayCanvas_b208ff6e.o: ../../Source/Processors/Visualization/SpikeDisplayCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDisplayCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DataWindow_83ce6754.o: ../../Source/Processors/Visualization/DataWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DataWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpDisplayCanvas_4a58e87e.o: ../../Source/Processors/Visualization/LfpDisplayCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpDisplayCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OpenGLCanvas_3c775a41.o: ../../Source/Processors/Visualization/OpenGLCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OpenGLCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDetector_300d85e7.o: ../../Source/Processors/SpikeDetector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDetector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioNode_94606ff3.o: ../../Source/Processors/AudioNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EventNode_95c842b7.o: ../../Source/Processors/EventNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EventNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpTriggeredAverageEditor_101d75b.o: ../../Source/Processors/Editors/LfpTriggeredAverageEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpTriggeredAverageEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelMappingEditor_dd803b0.o: ../../Source/Processors/Editors/ChannelMappingEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChannelMappingEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FileReaderEditor_16fd7744.o: ../../Source/Processors/Editors/FileReaderEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FileReaderEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PhaseDetectorEditor_f53aa520.o: ../../Source/Processors/Editors/PhaseDetectorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PhaseDetectorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PulsePalOutputEditor_ab5be148.o: ../../Source/Processors/Editors/PulsePalOutputEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PulsePalOutputEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RHD2000Editor_d7ddd6b.o: ../../Source/Processors/Editors/RHD2000Editor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RHD2000Editor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RecordControlEditor_244084cb.o: ../../Source/Processors/Editors/RecordControlEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RecordControlEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ReferenceNodeEditor_19ee0fac.o: ../../Source/Processors/Editors/ReferenceNodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ReferenceNodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ResamplingNodeEditor_f2d6f061.o: ../../Source/Processors/Editors/ResamplingNodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ResamplingNodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FPGAOutputEditor_cfe00f8a.o: ../../Source/Processors/Editors/FPGAOutputEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FPGAOutputEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ArduinoOutputEditor_1118f008.o: ../../Source/Processors/Editors/ArduinoOutputEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ArduinoOutputEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelSelector_c1430874.o: ../../Source/Processors/Editors/ChannelSelector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChannelSelector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ParameterEditor_91f62ae8.o: ../../Source/Processors/Editors/ParameterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ParameterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDisplayEditor_e1812c1.o: ../../Source/Processors/Editors/SpikeDisplayEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDisplayEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VisualizerEditor_3672b003.o: ../../Source/Processors/Editors/VisualizerEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling VisualizerEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MergerEditor_d1fcc0df.o: ../../Source/Processors/Editors/MergerEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MergerEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ImageIcon_c89b23a6.o: ../../Source/Processors/Editors/ImageIcon.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ImageIcon.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/WiFiOutputEditor_1d3ba5bb.o: ../../Source/Processors/Editors/WiFiOutputEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling WiFiOutputEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EventNodeEditor_2198733b.o: ../../Source/Processors/Editors/EventNodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EventNodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SignalGeneratorEditor_3223ac0a.o: ../../Source/Processors/Editors/SignalGeneratorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SignalGeneratorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpDisplayEditor_99d75351.o: ../../Source/Processors/Editors/LfpDisplayEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpDisplayEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SourceNodeEditor_8d813122.o: ../../Source/Processors/Editors/SourceNodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SourceNodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SplitterEditor_c2199b6c.o: ../../Source/Processors/Editors/SplitterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SplitterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDetectorEditor_2fb63eeb.o: ../../Source/Processors/Editors/SpikeDetectorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDetectorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioEditor_fb2c6555.o: ../../Source/Processors/Editors/AudioEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FilterEditor_dfe1f39d.o: ../../Source/Processors/Editors/FilterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FilterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GenericEditor_becb2ad6.o: ../../Source/Processors/Editors/GenericEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling GenericEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/okFrontPanelDLL_87687880.o: ../../Source/Processors/DataThreads/rhythm-api/okFrontPanelDLL.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling okFrontPanelDLL.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/rhd2000datablock_722d8dae.o: ../../Source/Processors/DataThreads/rhythm-api/rhd2000datablock.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling rhd2000datablock.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/rhd2000evalboard_e0b412d5.o: ../../Source/Processors/DataThreads/rhythm-api/rhd2000evalboard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling rhd2000evalboard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/rhd2000registers_cf6cd63b.o: ../../Source/Processors/DataThreads/rhythm-api/rhd2000registers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling rhd2000registers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RHD2000Thread_23e0b041.o: ../../Source/Processors/DataThreads/RHD2000Thread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RHD2000Thread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FileReaderThread_933ea08.o: ../../Source/Processors/DataThreads/FileReaderThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FileReaderThread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FPGAThread_a8dc34ed.o: ../../Source/Processors/DataThreads/FPGAThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FPGAThread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DataBuffer_6ae4f549.o: ../../Source/Processors/DataThreads/DataBuffer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DataBuffer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/IntanThread_32126b71.o: ../../Source/Processors/DataThreads/IntanThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling IntanThread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DataThread_b2a47a13.o: ../../Source/Processors/DataThreads/DataThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DataThread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RecordNode_2b7a1a2.o: ../../Source/Processors/RecordNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RecordNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SignalGenerator_a9cf4806.o: ../../Source/Processors/SignalGenerator.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SignalGenerator.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ResamplingNode_27a58a6b.o: ../../Source/Processors/ResamplingNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ResamplingNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FilterNode_817e9c9.o: ../../Source/Processors/FilterNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FilterNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SourceNode_c2d6336c.o: ../../Source/Processors/SourceNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SourceNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GenericProcessor_733760aa.o: ../../Source/Processors/GenericProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling GenericProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ProcessorGraph_68b34a0b.o: ../../Source/Processors/ProcessorGraph.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ProcessorGraph.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EditorViewportButtons_29af2a5c.o: ../../Source/UI/EditorViewportButtons.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EditorViewportButtons.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SignalChainManager_d2b643f0.o: ../../Source/UI/SignalChainManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SignalChainManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EditorViewport_1d991caf.o: ../../Source/UI/EditorViewport.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EditorViewport.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ProcessorList_1ad3f3de.o: ../../Source/UI/ProcessorList.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ProcessorList.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CustomLookAndFeel_53a8fcdb.o: ../../Source/UI/CustomLookAndFeel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CustomLookAndFeel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/InfoLabel_a2051bf4.o: ../../Source/UI/InfoLabel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling InfoLabel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DataViewport_2cf95d2c.o: ../../Source/UI/DataViewport.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DataViewport.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MessageCenter_748a1cca.o: ../../Source/UI/MessageCenter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MessageCenter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ControlPanel_a895ede3.o: ../../Source/UI/ControlPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ControlPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/UIComponent_d667ba37.o: ../../Source/UI/UIComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling UIComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MainWindow_499ac812.o: ../../Source/MainWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MainWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Main_90ebc5c2.o: ../../Source/Main.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Main.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_basics_2442e4ea.o: ../../JuceLibraryCode/modules/juce_audio_basics/juce_audio_basics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_basics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_devices_a4c8a728.o: ../../JuceLibraryCode/modules/juce_audio_devices/juce_audio_devices.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_devices.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_formats_d349f0c8.o: ../../JuceLibraryCode/modules/juce_audio_formats/juce_audio_formats.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_formats.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_processors_44a134a2.o: ../../JuceLibraryCode/modules/juce_audio_processors/juce_audio_processors.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_processors.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_utils_f63b12e8.o: ../../JuceLibraryCode/modules/juce_audio_utils/juce_audio_utils.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_utils.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_core_aff681cc.o: ../../JuceLibraryCode/modules/juce_core/juce_core.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_core.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_cryptography_25c7e826.o: ../../JuceLibraryCode/modules/juce_cryptography/juce_cryptography.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_cryptography.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_data_structures_bdd6d488.o: ../../JuceLibraryCode/modules/juce_data_structures/juce_data_structures.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_data_structures.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_events_79b2840.o: ../../JuceLibraryCode/modules/juce_events/juce_events.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_events.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_graphics_c8f1e7a4.o: ../../JuceLibraryCode/modules/juce_graphics/juce_graphics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_graphics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_basics_a630dd20.o: ../../JuceLibraryCode/modules/juce_gui_basics/juce_gui_basics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_basics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_extra_7767d6a8.o: ../../JuceLibraryCode/modules/juce_gui_extra/juce_gui_extra.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_extra.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_opengl_c7e3506c.o: ../../JuceLibraryCode/modules/juce_opengl/juce_opengl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_opengl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_video_184321c8.o: ../../JuceLibraryCode/modules/juce_video/juce_video.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_video.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
