# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "ZEROMQ" -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "JUCE_APP_VERSION=0.3.5" -D "JUCE_APP_VERSION_HEX=0x305" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode -I ../../JuceLibraryCode/modules
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O3 -export-dynamic -g -pg -std=c++0x
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -L/usr/X11R6/lib/ -L/usr/local/include -lGL -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt -pg -ldl -lXext -lGLU -lhdf5 -lhdf5_cpp -lzmq
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "ZEROMQ" -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "JUCE_APP_VERSION=0.3.5" -D "JUCE_APP_VERSION_HEX=0x305" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode -I ../../JuceLibraryCode/modules
  TARGET := open-ephys
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "ZEROMQ" -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "JUCE_APP_VERSION=0.3.5" -D "JUCE_APP_VERSION_HEX=0x305" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode -I ../../JuceLibraryCode/modules
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -O3 -export-dynamic -g -pg -std=c++0x
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -fvisibility=hidden -L/usr/X11R6/lib/ -lGL -lX11 -lXext -lXinerama -lasound -ldl -lfreetype -lpthread -lrt -pg -ldl -lXext -lGLU -lhdf5 -lhdf5_cpp -lzmq
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "ZEROMQ" -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "JUCE_APP_VERSION=0.3.5" -D "JUCE_APP_VERSION_HEX=0x305" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode -I ../../JuceLibraryCode/modules
  TARGET := open-ephys-release
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)
endif

OBJECTS := \
  $(OBJDIR)/AccessClass_de9602d5.o \
  $(OBJDIR)/PracticalSocket_2574ecc8.o \
  $(OBJDIR)/AudioComponent_521bd9c9.o \
  $(OBJDIR)/Rectifier_21cc94b6.o \
  $(OBJDIR)/ArduinoOutput_d5a968de.o \
  $(OBJDIR)/ArduinoOutputEditor_e1b7e52b.o \
  $(OBJDIR)/AudioEditor_3931be27.o \
  $(OBJDIR)/AudioNode_3db3557c.o \
  $(OBJDIR)/CAR_9a7e50f4.o \
  $(OBJDIR)/Channel_5cb2d4d2.o \
  $(OBJDIR)/ChannelMappingEditor_9b145f15.o \
  $(OBJDIR)/ChannelMappingNode_ec0559ea.o \
  $(OBJDIR)/EcubeEditor_ba242592.o \
  $(OBJDIR)/RHD2000Editor_dbd5a24.o \
  $(OBJDIR)/EcubeThread_d0477baf.o \
  $(OBJDIR)/okFrontPanelDLL_87687880.o \
  $(OBJDIR)/rhd2000datablock_722d8dae.o \
  $(OBJDIR)/rhd2000evalboard_e0b412d5.o \
  $(OBJDIR)/rhd2000registers_cf6cd63b.o \
  $(OBJDIR)/RHD2000Thread_23e0b041.o \
  $(OBJDIR)/DataBuffer_6ae4f549.o \
  $(OBJDIR)/DataThread_b2a47a13.o \
  $(OBJDIR)/Bessel_7e54cb27.o \
  $(OBJDIR)/Biquad_622c856b.o \
  $(OBJDIR)/Butterworth_6aca939b.o \
  $(OBJDIR)/Cascade_b39fccc1.o \
  $(OBJDIR)/ChebyshevI_e0373d09.o \
  $(OBJDIR)/ChebyshevII_281496aa.o \
  $(OBJDIR)/Custom_fce9f54e.o \
  $(OBJDIR)/Design_596f3d1b.o \
  $(OBJDIR)/Documentation_89e2d3c7.o \
  $(OBJDIR)/Elliptic_6f6493df.o \
  $(OBJDIR)/Filter_fe9ed9d5.o \
  $(OBJDIR)/Legendre_6dd0035d.o \
  $(OBJDIR)/Param_4e0cc01a.o \
  $(OBJDIR)/PoleFilter_fb8cf3ad.o \
  $(OBJDIR)/RBJ_6081b347.o \
  $(OBJDIR)/RootFinder_11229605.o \
  $(OBJDIR)/State_5d41ca1e.o \
  $(OBJDIR)/ChannelSelector_c1430874.o \
  $(OBJDIR)/ElectrodeButtons_a6064cc.o \
  $(OBJDIR)/GenericEditor_becb2ad6.o \
  $(OBJDIR)/ImageIcon_c89b23a6.o \
  $(OBJDIR)/VisualizerEditor_3672b003.o \
  $(OBJDIR)/NetworkEventsEditor_fd22826.o \
  $(OBJDIR)/PeriStimulusTimeHistogramEditor_f7323eb0.o \
  $(OBJDIR)/EventDetector_d0f4d00c.o \
  $(OBJDIR)/EventNode_857d5604.o \
  $(OBJDIR)/EventNodeEditor_2652ddd1.o \
  $(OBJDIR)/KwikFileSource_58456030.o \
  $(OBJDIR)/FileSource_a1ad7002.o \
  $(OBJDIR)/FileReader_e4a9ccaa.o \
  $(OBJDIR)/FileReaderEditor_e1193ff7.o \
  $(OBJDIR)/FilterEditor_93e366f5.o \
  $(OBJDIR)/FilterNode_d2b4d9ca.o \
  $(OBJDIR)/GenericProcessor_3e79932a.o \
  $(OBJDIR)/LfpDisplayCanvas_9bbf9660.o \
  $(OBJDIR)/LfpDisplayEditor_e7c32ff5.o \
  $(OBJDIR)/LfpDisplayNode_fdf2e2ca.o \
  $(OBJDIR)/Merger_53fb4e4a.o \
  $(OBJDIR)/MergerEditor_e36b0997.o \
  $(OBJDIR)/MessageCenter_bd1ba084.o \
  $(OBJDIR)/MessageCenterEditor_afaf4851.o \
  $(OBJDIR)/ParameterEditor_112258eb.o \
  $(OBJDIR)/Parameter_b3e5ac9e.o \
  $(OBJDIR)/PhaseDetector_8a25ed0e.o \
  $(OBJDIR)/PhaseDetectorEditor_eaec855b.o \
  $(OBJDIR)/ProcessorGraph_8c3a250a.o \
  $(OBJDIR)/PulsePalOutput_f41ce62a.o \
  $(OBJDIR)/PulsePalOutputEditor_3d333977.o \
  $(OBJDIR)/RecordControl_ecb8ada4.o \
  $(OBJDIR)/RecordControlEditor_4355fd71.o \
  $(OBJDIR)/EngineConfigWindow_4fd44ceb.o \
  $(OBJDIR)/HDF5FileFormat_be712135.o \
  $(OBJDIR)/HDF5Recording_d14f7b19.o \
  $(OBJDIR)/OriginalRecording_d6dc3293.o \
  $(OBJDIR)/RecordEngine_97ef83aa.o \
  $(OBJDIR)/RecordNode_cc21a82a.o \
  $(OBJDIR)/NetworkEvents_5344c99a.o \
  $(OBJDIR)/PeriStimulusTimeHistogramNode_9631ca2a.o \
  $(OBJDIR)/tictoc_cdca1ed.o \
  $(OBJDIR)/TrialCircularBuffer_4a4cef0c.o \
  $(OBJDIR)/ResamplingNode_9825590a.o \
  $(OBJDIR)/ResamplingNodeEditor_8b120457.o \
  $(OBJDIR)/PulsePal_14932a18.o \
  $(OBJDIR)/ofArduino_12f202a5.o \
  $(OBJDIR)/ofSerial_c3b0a9e1.o \
  $(OBJDIR)/SerialInput_f5b197b8.o \
  $(OBJDIR)/SerialInputEditor_46740885.o \
  $(OBJDIR)/SignalGenerator_993274e2.o \
  $(OBJDIR)/SignalGeneratorEditor_b8e0de2f.o \
  $(OBJDIR)/SourceNode_de3985ea.o \
  $(OBJDIR)/SourceNodeEditor_cdc90937.o \
  $(OBJDIR)/SpikeDetector_50b619e4.o \
  $(OBJDIR)/SpikeDetectorEditor_502139b1.o \
  $(OBJDIR)/SpikeDisplayCanvas_2219bd20.o \
  $(OBJDIR)/SpikeDisplayEditor_6e1d56b5.o \
  $(OBJDIR)/SpikeDisplayNode_be27b98a.o \
  $(OBJDIR)/SpikeSortBoxes_17f37282.o \
  $(OBJDIR)/SpikeSorter_5f160cae.o \
  $(OBJDIR)/SpikeSorterCanvas_f0ead366.o \
  $(OBJDIR)/SpikeSorterEditor_3cee6cfb.o \
  $(OBJDIR)/Splitter_889c68aa.o \
  $(OBJDIR)/SplitterEditor_93a6dbf7.o \
  $(OBJDIR)/DataWindow_83ce6754.o \
  $(OBJDIR)/SpikeObject_24e8c655.o \
  $(OBJDIR)/MatlabLikePlot_fb09c37f.o \
  $(OBJDIR)/EcubeDialogComponent_2ec3bd57.o \
  $(OBJDIR)/CustomArrowButton_206e4278.o \
  $(OBJDIR)/GraphViewer_e43fd2ce.o \
  $(OBJDIR)/EditorViewportButtons_29af2a5c.o \
  $(OBJDIR)/SignalChainManager_d2b643f0.o \
  $(OBJDIR)/EditorViewport_1d991caf.o \
  $(OBJDIR)/ProcessorList_1ad3f3de.o \
  $(OBJDIR)/CustomLookAndFeel_53a8fcdb.o \
  $(OBJDIR)/InfoLabel_a2051bf4.o \
  $(OBJDIR)/DataViewport_2cf95d2c.o \
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
	@$(CLEANCMD)

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

$(OBJDIR)/AudioComponent_521bd9c9.o: ../../Source/Audio/AudioComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Rectifier_21cc94b6.o: ../../Source/Processors/Rectifier/Rectifier.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Rectifier.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ArduinoOutput_d5a968de.o: ../../Source/Processors/ArduinoOutput/ArduinoOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ArduinoOutput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ArduinoOutputEditor_e1b7e52b.o: ../../Source/Processors/ArduinoOutput/ArduinoOutputEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ArduinoOutputEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioEditor_3931be27.o: ../../Source/Processors/AudioNode/AudioEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioNode_3db3557c.o: ../../Source/Processors/AudioNode/AudioNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CAR_9a7e50f4.o: ../../Source/Processors/CAR/CAR.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CAR.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Channel_5cb2d4d2.o: ../../Source/Processors/Channel/Channel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Channel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelMappingEditor_9b145f15.o: ../../Source/Processors/ChannelMappingNode/ChannelMappingEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChannelMappingEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelMappingNode_ec0559ea.o: ../../Source/Processors/ChannelMappingNode/ChannelMappingNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChannelMappingNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EcubeEditor_ba242592.o: ../../Source/Processors/DataThreads/EcubeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EcubeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RHD2000Editor_dbd5a24.o: ../../Source/Processors/DataThreads/RHD2000Editor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RHD2000Editor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EcubeThread_d0477baf.o: ../../Source/Processors/DataThreads/EcubeThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EcubeThread.cpp"
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

$(OBJDIR)/DataBuffer_6ae4f549.o: ../../Source/Processors/DataThreads/DataBuffer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DataBuffer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DataThread_b2a47a13.o: ../../Source/Processors/DataThreads/DataThread.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DataThread.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Bessel_7e54cb27.o: ../../Source/Processors/Dsp/Bessel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Bessel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Biquad_622c856b.o: ../../Source/Processors/Dsp/Biquad.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Biquad.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Butterworth_6aca939b.o: ../../Source/Processors/Dsp/Butterworth.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Butterworth.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Cascade_b39fccc1.o: ../../Source/Processors/Dsp/Cascade.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Cascade.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChebyshevI_e0373d09.o: ../../Source/Processors/Dsp/ChebyshevI.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChebyshevI.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChebyshevII_281496aa.o: ../../Source/Processors/Dsp/ChebyshevII.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChebyshevII.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Custom_fce9f54e.o: ../../Source/Processors/Dsp/Custom.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Custom.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Design_596f3d1b.o: ../../Source/Processors/Dsp/Design.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Design.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Documentation_89e2d3c7.o: ../../Source/Processors/Dsp/Documentation.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Documentation.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Elliptic_6f6493df.o: ../../Source/Processors/Dsp/Elliptic.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Elliptic.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Filter_fe9ed9d5.o: ../../Source/Processors/Dsp/Filter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Filter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Legendre_6dd0035d.o: ../../Source/Processors/Dsp/Legendre.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Legendre.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Param_4e0cc01a.o: ../../Source/Processors/Dsp/Param.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Param.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PoleFilter_fb8cf3ad.o: ../../Source/Processors/Dsp/PoleFilter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PoleFilter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RBJ_6081b347.o: ../../Source/Processors/Dsp/RBJ.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RBJ.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RootFinder_11229605.o: ../../Source/Processors/Dsp/RootFinder.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RootFinder.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/State_5d41ca1e.o: ../../Source/Processors/Dsp/State.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling State.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ChannelSelector_c1430874.o: ../../Source/Processors/Editors/ChannelSelector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ChannelSelector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ElectrodeButtons_a6064cc.o: ../../Source/Processors/Editors/ElectrodeButtons.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ElectrodeButtons.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GenericEditor_becb2ad6.o: ../../Source/Processors/Editors/GenericEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling GenericEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ImageIcon_c89b23a6.o: ../../Source/Processors/Editors/ImageIcon.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ImageIcon.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/VisualizerEditor_3672b003.o: ../../Source/Processors/Editors/VisualizerEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling VisualizerEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/NetworkEventsEditor_fd22826.o: ../../Source/Processors/Editors/NetworkEventsEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling NetworkEventsEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PeriStimulusTimeHistogramEditor_f7323eb0.o: ../../Source/Processors/Editors/PeriStimulusTimeHistogramEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PeriStimulusTimeHistogramEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EventDetector_d0f4d00c.o: ../../Source/Processors/EventDetector/EventDetector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EventDetector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EventNode_857d5604.o: ../../Source/Processors/EventNode/EventNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EventNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EventNodeEditor_2652ddd1.o: ../../Source/Processors/EventNode/EventNodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EventNodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/KwikFileSource_58456030.o: ../../Source/Processors/FileReader/KwikFileSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling KwikFileSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FileSource_a1ad7002.o: ../../Source/Processors/FileReader/FileSource.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FileSource.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FileReader_e4a9ccaa.o: ../../Source/Processors/FileReader/FileReader.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FileReader.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FileReaderEditor_e1193ff7.o: ../../Source/Processors/FileReader/FileReaderEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FileReaderEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FilterEditor_93e366f5.o: ../../Source/Processors/FilterNode/FilterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FilterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FilterNode_d2b4d9ca.o: ../../Source/Processors/FilterNode/FilterNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FilterNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GenericProcessor_3e79932a.o: ../../Source/Processors/GenericProcessor/GenericProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling GenericProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpDisplayCanvas_9bbf9660.o: ../../Source/Processors/LfpDisplayNode/LfpDisplayCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpDisplayCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpDisplayEditor_e7c32ff5.o: ../../Source/Processors/LfpDisplayNode/LfpDisplayEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpDisplayEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/LfpDisplayNode_fdf2e2ca.o: ../../Source/Processors/LfpDisplayNode/LfpDisplayNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling LfpDisplayNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Merger_53fb4e4a.o: ../../Source/Processors/Merger/Merger.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Merger.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MergerEditor_e36b0997.o: ../../Source/Processors/Merger/MergerEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MergerEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MessageCenter_bd1ba084.o: ../../Source/Processors/MessageCenter/MessageCenter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MessageCenter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MessageCenterEditor_afaf4851.o: ../../Source/Processors/MessageCenter/MessageCenterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MessageCenterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ParameterEditor_112258eb.o: ../../Source/Processors/Parameter/ParameterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ParameterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Parameter_b3e5ac9e.o: ../../Source/Processors/Parameter/Parameter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Parameter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PhaseDetector_8a25ed0e.o: ../../Source/Processors/PhaseDetector/PhaseDetector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PhaseDetector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PhaseDetectorEditor_eaec855b.o: ../../Source/Processors/PhaseDetector/PhaseDetectorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PhaseDetectorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ProcessorGraph_8c3a250a.o: ../../Source/Processors/ProcessorGraph/ProcessorGraph.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ProcessorGraph.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PulsePalOutput_f41ce62a.o: ../../Source/Processors/PulsePalOutput/PulsePalOutput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PulsePalOutput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PulsePalOutputEditor_3d333977.o: ../../Source/Processors/PulsePalOutput/PulsePalOutputEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PulsePalOutputEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RecordControl_ecb8ada4.o: ../../Source/Processors/RecordControl/RecordControl.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RecordControl.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RecordControlEditor_4355fd71.o: ../../Source/Processors/RecordControl/RecordControlEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RecordControlEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EngineConfigWindow_4fd44ceb.o: ../../Source/Processors/RecordNode/EngineConfigWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EngineConfigWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/HDF5FileFormat_be712135.o: ../../Source/Processors/RecordNode/HDF5FileFormat.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling HDF5FileFormat.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/HDF5Recording_d14f7b19.o: ../../Source/Processors/RecordNode/HDF5Recording.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling HDF5Recording.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OriginalRecording_d6dc3293.o: ../../Source/Processors/RecordNode/OriginalRecording.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OriginalRecording.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RecordEngine_97ef83aa.o: ../../Source/Processors/RecordNode/RecordEngine.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RecordEngine.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RecordNode_cc21a82a.o: ../../Source/Processors/RecordNode/RecordNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RecordNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/NetworkEvents_5344c99a.o: ../../Source/Processors/NetworkEvents/NetworkEvents.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling NetworkEvents.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PeriStimulusTimeHistogramNode_9631ca2a.o: ../../Source/Processors/PSTH/PeriStimulusTimeHistogramNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PeriStimulusTimeHistogramNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/tictoc_cdca1ed.o: ../../Source/Processors/PSTH/tictoc.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling tictoc.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TrialCircularBuffer_4a4cef0c.o: ../../Source/Processors/PSTH/TrialCircularBuffer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling TrialCircularBuffer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ResamplingNode_9825590a.o: ../../Source/Processors/ResamplingNode/ResamplingNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ResamplingNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ResamplingNodeEditor_8b120457.o: ../../Source/Processors/ResamplingNode/ResamplingNodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ResamplingNodeEditor.cpp"
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

$(OBJDIR)/SerialInput_f5b197b8.o: ../../Source/Processors/SerialInput/SerialInput.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SerialInput.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SerialInputEditor_46740885.o: ../../Source/Processors/SerialInput/SerialInputEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SerialInputEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SignalGenerator_993274e2.o: ../../Source/Processors/SignalGenerator/SignalGenerator.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SignalGenerator.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SignalGeneratorEditor_b8e0de2f.o: ../../Source/Processors/SignalGenerator/SignalGeneratorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SignalGeneratorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SourceNode_de3985ea.o: ../../Source/Processors/SourceNode/SourceNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SourceNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SourceNodeEditor_cdc90937.o: ../../Source/Processors/SourceNode/SourceNodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SourceNodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDetector_50b619e4.o: ../../Source/Processors/SpikeDetector/SpikeDetector.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDetector.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDetectorEditor_502139b1.o: ../../Source/Processors/SpikeDetector/SpikeDetectorEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDetectorEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDisplayCanvas_2219bd20.o: ../../Source/Processors/SpikeDisplayNode/SpikeDisplayCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDisplayCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDisplayEditor_6e1d56b5.o: ../../Source/Processors/SpikeDisplayNode/SpikeDisplayEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDisplayEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeDisplayNode_be27b98a.o: ../../Source/Processors/SpikeDisplayNode/SpikeDisplayNode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeDisplayNode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeSortBoxes_17f37282.o: ../../Source/Processors/SpikeSorter/SpikeSortBoxes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeSortBoxes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeSorter_5f160cae.o: ../../Source/Processors/SpikeSorter/SpikeSorter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeSorter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeSorterCanvas_f0ead366.o: ../../Source/Processors/SpikeSorter/SpikeSorterCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeSorterCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeSorterEditor_3cee6cfb.o: ../../Source/Processors/SpikeSorter/SpikeSorterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeSorterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/Splitter_889c68aa.o: ../../Source/Processors/Splitter/Splitter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling Splitter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SplitterEditor_93a6dbf7.o: ../../Source/Processors/Splitter/SplitterEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SplitterEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DataWindow_83ce6754.o: ../../Source/Processors/Visualization/DataWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DataWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/SpikeObject_24e8c655.o: ../../Source/Processors/Visualization/SpikeObject.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling SpikeObject.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MatlabLikePlot_fb09c37f.o: ../../Source/Processors/Visualization/MatlabLikePlot.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MatlabLikePlot.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/EcubeDialogComponent_2ec3bd57.o: ../../Source/UI/EcubeDialogComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling EcubeDialogComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CustomArrowButton_206e4278.o: ../../Source/UI/CustomArrowButton.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CustomArrowButton.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GraphViewer_e43fd2ce.o: ../../Source/UI/GraphViewer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling GraphViewer.cpp"
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
