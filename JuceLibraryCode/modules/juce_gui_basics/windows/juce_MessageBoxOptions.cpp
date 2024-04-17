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

MessageBoxOptions MessageBoxOptions::makeOptionsOk (MessageBoxIconType iconType,
                                                    const String& title,
                                                    const String& message,
                                                    const String& buttonText,
                                                    Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (buttonText.isEmpty() ? TRANS ("OK") : buttonText)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType iconType,
                                                          const String& title,
                                                          const String& message,
                                                          const String& button1Text,
                                                          const String& button2Text,
                                                          Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("OK") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("Cancel") : button2Text)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsYesNo (MessageBoxIconType iconType,
                                                       const String& title,
                                                       const String& message,
                                                       const String& button1Text,
                                                       const String& button2Text,
                                                       Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("Yes") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("No") : button2Text)
        .withAssociatedComponent (associatedComponent);
}

MessageBoxOptions MessageBoxOptions::makeOptionsYesNoCancel (MessageBoxIconType iconType,
                                                             const String& title,
                                                             const String& message,
                                                             const String& button1Text,
                                                             const String& button2Text,
                                                             const String& button3Text,
                                                             Component* associatedComponent)
{
    return MessageBoxOptions()
        .withIconType (iconType)
        .withTitle (title)
        .withMessage (message)
        .withButton (button1Text.isEmpty() ? TRANS ("Yes") : button1Text)
        .withButton (button2Text.isEmpty() ? TRANS ("No") : button2Text)
        .withButton (button3Text.isEmpty() ? TRANS ("Cancel") : button3Text)
        .withAssociatedComponent (associatedComponent);
}

} // namespace juce
