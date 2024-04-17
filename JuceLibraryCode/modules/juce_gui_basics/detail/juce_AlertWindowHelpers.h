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

namespace juce::detail
{

struct AlertWindowHelpers
{
    AlertWindowHelpers() = delete;

    static std::unique_ptr<ScopedMessageBoxInterface> create (const MessageBoxOptions& opts)
    {
        class AlertWindowImpl : public detail::ScopedMessageBoxInterface
        {
        public:
            explicit AlertWindowImpl (const MessageBoxOptions& opts) : options (opts) {}

            void runAsync (std::function<void (int)> recipient) override
            {
                if (auto* comp = setUpAlert())
                    comp->enterModalState (true, ModalCallbackFunction::create (std::move (recipient)), true);
                else
                    NullCheckedInvocation::invoke (recipient, 0);
            }

            int runSync() override
            {
               #if JUCE_MODAL_LOOPS_PERMITTED
                if (auto comp = rawToUniquePtr (setUpAlert()))
                    return comp->runModalLoop();
               #endif

                jassertfalse;
                return 0;
            }

            void close() override
            {
                if (alert != nullptr)
                    if (alert->isCurrentlyModal())
                        alert->exitModalState();

                alert = nullptr;
            }

        private:
            Component* setUpAlert()
            {
                auto* component = options.getAssociatedComponent();

                auto& lf = component != nullptr ? component->getLookAndFeel()
                                                : LookAndFeel::getDefaultLookAndFeel();

                alert = lf.createAlertWindow (options.getTitle(),
                                              options.getMessage(),
                                              options.getButtonText (0),
                                              options.getButtonText (1),
                                              options.getButtonText (2),
                                              options.getIconType(),
                                              options.getNumButtons(),
                                              component);

                if (alert == nullptr)
                {
                    // You have to return an alert box!
                    jassertfalse;
                    return nullptr;
                }

                if (auto* parent = options.getParentComponent())
                {
                    parent->addAndMakeVisible (alert);

                    if (options.getAssociatedComponent() == nullptr)
                        alert->setCentrePosition (parent->getLocalBounds().getCentre());
                }

                alert->setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());

                return alert;
            }

            const MessageBoxOptions options;
            Component::SafePointer<AlertWindow> alert;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlertWindowImpl)
        };

        return std::make_unique<AlertWindowImpl> (opts);
    }
};

} // namespace juce::detail
