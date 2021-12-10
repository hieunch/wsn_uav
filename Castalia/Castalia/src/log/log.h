#pragma once

#include <vector>
#include <BaseMessage_m.h>
#include <iostream>

//
void logBoundHoleDrop(int pid, const char*reason);
void logBoundHoleRecv(int pid, int myid);
void logSend(cMessage *msg, int ID);

