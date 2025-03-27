#include "StubAccessClass.h"

void StubAccessClass::addMessageCenter(){
    stubMC = std::make_unique<MessageCenter>();
    stubMC->addSpecialProcessorChannels();
    AccessClass::setMessageCenter(stubMC.get());
}
