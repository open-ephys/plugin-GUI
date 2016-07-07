#include "Example_CL_Plugin.h"

Example_CL_Plugin::Example_CL_Plugin()
{
    std::cout << "Yo" << std::endl;
}

void Example_CL_Plugin::handleEvent(int eventType, MidiMessage& event, int samplePosition/* = 0 */)
{
    ;
}
