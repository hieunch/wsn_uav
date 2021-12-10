/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Yuriy Tselishchev                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#ifndef _SIMPLEPAIRAPPLICATION_H_
#define _SIMPLEPAIRAPPLICATION_H_

#include "VirtualApplication.h"

using namespace std;

enum SimplePairApplicationTimers {
	SEND_PACKET = 1
};

class SimplePairApplication: public VirtualApplication {
 private:
	double packetRate;
	double startupDelay;
  bool isSource;
  string sinkAddress;
  int numPacketToSend;

	int dataSN;
	double startSendingTime;
	double stopSendingTime;


 protected:
	void startup();
	void fromNetworkLayer(ApplicationPacket *, const char *, double, double);
	void timerFiredCallback(int);
};

#endif				
