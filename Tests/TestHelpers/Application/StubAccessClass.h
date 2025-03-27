#ifndef STUBACCESSCLASS_H
#define STUBACCESSCLASS_H

#include <NonAPIHeaders.h>

class TESTABLE StubAccessClass {
public:
    void addMessageCenter();
private:
    std::unique_ptr<MessageCenter> stubMC;
};


#endif
