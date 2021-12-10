/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#include "SimplePairApplication.h"

Define_Module(SimplePairApplication);

double fRand(double fMin, double fMax)
{
    double f = (double)rand() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

void SimplePairApplication::startup()
{
	startupDelay = par("startupDelay");
  isSource = par("isSource");
  sinkAddress = par("sink").stringValue();
	dataSN = 0;
	packetRate = par("packetRate").doubleValue();
  numPacketToSend = par("numPacketToSend").longValue();
	startSendingTime = par("startSendingTime").doubleValue();
	stopSendingTime = par("stopSendingTime").doubleValue();

	if (isSource) {
		setTimer(SEND_PACKET, 20); // 10s of set up
	}
	else
		trace() << "Not sending packets";
}

void SimplePairApplication::fromNetworkLayer(ApplicationPacket * rcvPacket,
		const char *source, double rssi, double lqi) {
	int sequenceNumber = rcvPacket->getSequenceNumber();
	trace() << "Received packet #" << sequenceNumber << " from node " << source;
}

void SimplePairApplication::timerFiredCallback(int index) {
	switch (index) {
		case SEND_PACKET:{
      // just send once
			trace() << "Sending packet #" << dataSN;
			toNetworkLayer(createGenericDataPacket(100, dataSN, 256), par("sink"));
			dataSN++;

            double packetSpacing = 1 / (packetRate * 1024 / 256);
			if (dataSN < numPacketToSend || numPacketToSend == -1) {
//			    if (simTime().dbl() < stopSendingTime) {
                    setTimer(SEND_PACKET, packetSpacing);
//			    }
			}
			break;
		}
	}
}

