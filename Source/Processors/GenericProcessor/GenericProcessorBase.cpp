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
#include "GenericProcessorBase.h"

GenericProcessorBase::GenericProcessorBase(const String& name) : m_name(name) {}

GenericProcessorBase::~GenericProcessorBase() {}

const String GenericProcessorBase::getName() const { return m_name; }

bool GenericProcessorBase::hasEditor() const { return true; }

void GenericProcessorBase::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {}

void GenericProcessorBase::releaseResources() {}

void GenericProcessorBase::reset() {}

void GenericProcessorBase::setCurrentProgramStateInformation(const void* data, int sizeInBytes) {}

void GenericProcessorBase::setStateInformation(const void* data, int sizeInBytes) {}

void GenericProcessorBase::getCurrentProgramStateInformation(MemoryBlock& destData) {}

void GenericProcessorBase::getStateInformation(MemoryBlock& destData) {}

void GenericProcessorBase::changeProgramName(int index, const String& newName) {}

void GenericProcessorBase::setCurrentProgram(int index) {}

const String GenericProcessorBase::getProgramName(int index)   { return ""; }

int GenericProcessorBase::getCurrentChannel() const { return 0; }

const String GenericProcessorBase::getParameterName(int parameterIndex) { return ""; }

const String GenericProcessorBase::getParameterText(int parameterIndex) { return ""; }

float GenericProcessorBase::getParameter(int parameterIndex) { return 0.0f;}

bool GenericProcessorBase::isInputChannelStereoPair(int index) const { return true; }

bool GenericProcessorBase::isOutputChannelStereoPair(int index) const { return true; }

bool GenericProcessorBase::acceptsMidi() const  { return true; }

bool GenericProcessorBase::producesMidi() const { return true; }

bool GenericProcessorBase::silenceInProducesSilenceOut() const  { return false; }

bool GenericProcessorBase::isParameterAutomatable(int parameterIndex) const { return false; }

bool GenericProcessorBase::isMetaParameter(int parameterIndex) const { return false; }

int GenericProcessorBase::getNumParameters() { return 0;}

int GenericProcessorBase::getNumPrograms() {return 0;}

int GenericProcessorBase::getCurrentProgram() {	return 0;}

double GenericProcessorBase::getTailLengthSeconds() const {	return 0.0f;}