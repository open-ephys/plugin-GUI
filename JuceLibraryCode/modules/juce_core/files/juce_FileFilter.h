/*
  ==============================================================================

   This file is part of the JUCE 8 technical preview.
   Copyright (c) Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Interface for deciding which files are suitable for something.

    For example, this is used by DirectoryContentsList to select which files
    go into the list.

    @see WildcardFileFilter, DirectoryContentsList, FileListComponent, FileBrowserComponent

    @tags{Core}
*/
class JUCE_API  FileFilter
{
public:
    //==============================================================================
    /** Creates a filter with the given description.

        The description can be returned later with the getDescription() method.
    */
    FileFilter (const String& filterDescription);

    /** Destructor. */
    virtual ~FileFilter();

    //==============================================================================
    /** Returns the description that the filter was created with. */
    const String& getDescription() const noexcept;

    //==============================================================================
    /** Should return true if this file is suitable for inclusion in whatever context
        the object is being used.
    */
    virtual bool isFileSuitable (const File& file) const = 0;

    /** Should return true if this directory is suitable for inclusion in whatever context
        the object is being used.
    */
    virtual bool isDirectorySuitable (const File& file) const = 0;


protected:
    //==============================================================================
    String description;
};

} // namespace juce
