#ifndef __EPHYSSOCKETEDITORH__
#define __EPHYSSOCKETEDITORH__

#ifdef _WIN32
#include <Windows.h>
#endif

#include <VisualizerEditorHeaders.h>
//#include <EditorHeaders.h>

namespace EphysSocketNode
{
    class EphysSocket;

    class EphysSocketEditor : public GenericEditor
    {

    public:

        EphysSocketEditor(GenericProcessor* parentNode, EphysSocket *node);

        /** Button listener callback, called by button when pressed. */
        void buttonEvent(Button* button);

        /** Called by processor graph in beginning of the acqusition, disables editor completly. */
        void startAcquisition();

        /** Called by processor graph at the end of the acqusition, reenables editor completly. */
        void stopAcquisition();

        /** Called when configuration is saved. Adds editors config to xml. */
        void saveEditorParameters(XmlElement* xml);

        /** Called when configuration is loaded. Reads editors config from xml. */
        void loadEditorParameters(XmlElement* xml);

    private:

        // Button that tried to connect to client
        ScopedPointer<UtilityButton> connectButton;

        // Port
        ScopedPointer<Label> portLabel;
        ScopedPointer<TextEditor> portText;

        // Chans
        ScopedPointer<Label> chanLabel;
        ScopedPointer<TextEditor> chanText;

        // Samples
        ScopedPointer<Label> sampLabel;
        ScopedPointer<TextEditor> sampText;

        // Fs
        ScopedPointer<Label> fsLabel;
        ScopedPointer<TextEditor> fsText;

        // Scale
        ScopedPointer<Label> scaleLabel;
        ScopedPointer<TextEditor> scaleText;

        // Offset
        ScopedPointer<Label> offsetLabel;
        ScopedPointer<TextEditor> offsetText;

        // Transpose
        TextButton transposeButton{ "Transpose" };

        // Parent node
        EphysSocket* node;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EphysSocketEditor);
    };
}

#endif