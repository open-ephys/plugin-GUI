/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2016 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef TIMESTAMPSOURCESELECTION_H_INCLUDED
#define TIMESTAMPSOURCESELECTION_H_INCLUDED

#include <JuceHeader.h>

class TimestampSourceSelectionComponent : 
	public Component,
	public ComboBox::Listener,
	public AsyncUpdater
{
public:
	TimestampSourceSelectionComponent();
	~TimestampSourceSelectionComponent();
	void paint(Graphics& g) override;
	void comboBoxChanged(ComboBox*) override;
	void handleAsyncUpdate() override;
	void setAcquisitionState(bool);

private:
	void updateProcessorList();

	struct SourceInfo
	{
		int processorIndex;
		int subProcessorIndex;
	};
	ScopedPointer<ComboBox> m_selector;
	Array<SourceInfo> m_sourcesArray;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimestampSourceSelectionComponent);
};

class TimestampSourceSelectionWindow :
	public DocumentWindow
{
public:
	TimestampSourceSelectionWindow();
	~TimestampSourceSelectionWindow();
	void updateProcessorList();
	void setAcquisitionState(bool);
	void closeButtonPressed() override;

private:
	ScopedPointer<TimestampSourceSelectionComponent> m_selectorComponent;

	WeakReference<TimestampSourceSelectionWindow>::Master masterReference;
	friend class WeakReference<TimestampSourceSelectionWindow>;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimestampSourceSelectionWindow);
};

#endif