#include "../JuceLibraryCode/JuceHeader.h"
#include <iostream>

void shutdownSequence(OutputStream* destStream)
{
    delete destStream;
    shutdownJuce_GUI();
}

int main (int argc, char* argv[])
{
    // This object makes sure that Juce is initialised and shut down correctly
    // for the scope of this function call. Make sure this declaration is the
    // first statement of this function.
    const ScopedJuceInitialiser_NonGUI juceSystemInitialiser;
   
    printf ("\n\n--------------------------------\n Font Serialiser by Niall Moody\n--------------------------------\n\n");
   
    if (argc != 3)
    {
        printf (" Usage: FontSerialiser <filename> <fontname>\n\n");
        printf (" FontSerialiser will turn a font into a compressed binary file.\n\n\n");
       
        return 1;
    }
   
    // because we're not using the proper application startup procedure, we need to call
    // this explicitly here to initialise some of the time-related stuff..
    initialiseJuce_GUI();
   
    // get file and font name from command line arguments
    const File destFile (File::getCurrentWorkingDirectory().getChildFile (argv[1]));
    String fontName(argv[2]);
   
    // make sure the destination file can be written to
    OutputStream *destStream = destFile.createOutputStream();

    if (destStream == 0)
    {
        String error;
        error << "\nError : Couldn't open " << destFile.getFullPathName() << " for writing.\n\n";
        std::cout << error;
        destFile.deleteFile();
        shutdownSequence(destStream);
        return 2;
    }
   
    // make sure the font is installed on the current system
    StringArray fontNames = Font::findAllTypefaceNames();
    if(!fontNames.contains(fontName))
    {
        String error ("\nError: The font " + fontName + " does not exist in the system\n");
        std::cout << error;
         destFile.deleteFile();
        shutdownSequence(destStream);
        return 3;
    }
   
    // load the font as a system-Typeface
    Font font(fontName, 10, 0);
    if(!Typeface::createSystemTypefaceFor  (font))
    {
        String error ("\nError : Where's the font?\n\n");
        std::cout << error;
         destFile.deleteFile();
        shutdownSequence(destStream);
        return 4;
    }
   
   
    // copy the font-properties to a CustomTypeface
    CustomTypeface customTypeface;
    customTypeface.setCharacteristics(font.getTypefaceName(), font.getAscent(),
                                      font.isBold(), font.isItalic(), ' ');
    // Here's the important part: copy all glyphs to a new instance of CustomTypeface
    customTypeface.addGlyphsFromOtherTypeface( *font.getTypeface(), 0, 256);
   
   
    // finally write the typeface into the destination file
    customTypeface.writeToStream(*destStream);
   
    String op;
    op << "\nWrote font " << fontName << " to file " << destFile.getFullPathName() << " successfully.\n\n";
    std::cout << op;
   
    std::cout << "\n(You might want to use Binary Builder to turn this file into a c++ file now)\n\n ";
   
    shutdownSequence(destStream);

    return 0;
}
