# Developer tools

This folder contains software tools that are useful for plugin developers.

## Binary Builder
A tool to compile binary resources like images or fonts into source code files (.h and .cpp) so their contents can be easily accessed by C++ code.

### Compilation instructions

To generate build files for your platform, run the following in a command terminal:

```
cd Build
cmake -G <GENERATOR> ..
```

Valid generators are:

**Windows:**
`"Visual Studio 12 2013 Win64"`
`"Visual Studio 14 2015 Win64"`
`"Visual Studio 15 2017 Win64"`
`"Visual Studio 16 2019 Win64" -A x64`

**Mac:**
`"Xcode"`

**Linux:**
`"Unix Makefiles"`

Example: `cmake -G "Xcode" ..`

### Usage

Once you've compiled the `BinaryBuilder` executable, run it from the command line as follows (on Windows):

`BinaryBuilder.exe ./input ./output BinaryData`

The first argument is the name of the directory containing all of the files (e.g. images and typefaces) that will be packaged.

The second argument is the name of the directory where the source file will be saved.

The third argument is the name of the source file that will be generated (should always be `BinaryData`).

When the program finishes running, copy `BinaryData.h` and `BinaryData.cpp` to the GUI's `JuceLibraryCode` directory.

Images can now be loaded using, e.g.:

`bw_logo = ImageCache::getFromMemory(BinaryData::bw_logo72_png, BinaryData::bw_logo72_pngSize);`

Typefaces can be loaded using, e.g.:

`Typeface::Ptr firasansExtraLight = Typeface::createSystemTypefaceFor(BinaryData::FiraSansExtraLight_ttf, BinaryData::FiraSansExtraLight_ttfSize)`

Currently, `BinaryData.h` contains all of the typefaces from the `Resources/Fonts` directory and all of the images from the `Resources/Images` directory.





