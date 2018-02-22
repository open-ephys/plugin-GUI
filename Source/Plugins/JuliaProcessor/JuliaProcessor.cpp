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

#include "JuliaProcessor.h"
#include "JuliaEditor.h"
#include <stdio.h>
#include <julia.h>

JuliaProcessor::JuliaProcessor()
    : GenericProcessor("Julia Processor")
{
	hasJuliaInstance = false;
    dataHistoryBufferNumChannels = 256;
    dataHistoryBuffer = new AudioSampleBuffer(dataHistoryBufferNumChannels, 60000);
    dataHistoryBuffer->clear();
}

JuliaProcessor::~JuliaProcessor()
{
	jl_atexit_hook(0);
	deleteAndZero(dataHistoryBuffer);
}

AudioProcessorEditor* JuliaProcessor::createEditor()
{
    editor = new JuliaEditor(this, true);
    std::cout << "Creating Julia editor." << std::endl;
    return editor;
}

void JuliaProcessor::setFile(String fullpath)
{
	hasJuliaInstance = true;
	filePath = fullpath;

	FILE* fp = popen("echo $JULIA", "r");
	char input[255];
	fgets(input, sizeof(input), fp);
	
	String julia_bin_dir = input;
	julia_bin_dir = julia_bin_dir.trimEnd();
	julia_bin_dir += "/home/jvoigts/Documents/Github/julia/usr/bin";
	String julia_sys_dir = input;
	julia_sys_dir = julia_sys_dir.trimEnd();
	julia_sys_dir += "/home/jvoigts/Documents/Github/julia/usr/lib/julia/sys.so";
	
	const char* jbin = julia_bin_dir.toRawUTF8();
	const char* jsys = julia_sys_dir.toRawUTF8();
	
	jl_init_with_image(jbin, jsys);

	String juliaString = "include(\"" + filePath + "\")";
	run_julia_string(juliaString);
}

void JuliaProcessor::reloadFile()
{
    if (hasJuliaInstance) 
    {
        String juliaString = "reload(\"" + filePath + "\")";
        run_julia_string(juliaString);
    }
    else
    {
        std::cout << "No julia instance running - cant refresh" << std::endl;
    }
}


void JuliaProcessor::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
}

void JuliaProcessor::setBuffersize(int bufferSize)
{
    if (bufferSize > 1)
    {
        dataHistoryBufferSize=bufferSize;
        printf("Setting history buffer size to %d samples \n", dataHistoryBufferSize);
        dataHistoryBuffer->setSize(dataHistoryBufferNumChannels, dataHistoryBufferSize, false, true, false);
    }
    else 
    {
        printf("History buffer size has to be at least 1");
    }
}

void JuliaProcessor::run_julia_string(String juliaString)
{
    // need to convert from juce String to char array
    const char* jstr = juliaString.toRawUTF8();

    printf("executing julia cmd: %s\n", jstr);

    jl_eval_string(jstr);

    if (jl_exception_occurred())
    	printf("%s \n", jl_typeof_str(jl_exception_occurred()));
}



String JuliaProcessor::getFile()
{
    return filePath;
}

void JuliaProcessor::process(AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
	if (hasJuliaInstance)
	{
		jl_function_t *func = jl_get_function(jl_main_module, "oe_process!");    
	
		// create 2D array of float64 type
		jl_value_t *array_type = jl_apply_array_type(jl_float32_type, 1); // last arg is nDims

		for (int n = 0; n < getNumOutputs(); n++)
		{
			float* ptr = buffer.getWritePointer(n); // to perform in-place edits to the buffer
			jl_array_t *x = jl_ptr_to_array_1d(array_type, ptr , buffer.getNumSamples(), 0);
			JL_GC_PUSH1(&x);
			jl_call1(func, (jl_value_t*)x);
			JL_GC_POP();
		}
	}
}

void JuliaProcessor::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* childNode = parentElement->createNewChildElement("FILENAME");
    childNode->setAttribute("path", getFile());
}

void JuliaProcessor::loadCustomParametersFromXml()
{
    if (parametersAsXml != nullptr)
    {
        // use parametersAsXml to restore state
        forEachXmlChildElement(*parametersAsXml, xmlNode)
        {
            if (xmlNode->hasTagName("FILENAME"))
            {
                String filepath = xmlNode->getStringAttribute("path");
                JuliaEditor* fre = (JuliaEditor*) getEditor();
                fre->setFile(filepath);
            }
        }
    }
}