/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2011                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Yuriy Tselishchev, Athanassios Boulis                        *
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *
 ****************************************************************************/

#include "VirtualRouting.h"

bool VirtualRouting::initialized = false;
int VirtualRouting::sinkId;
NetworkConfig VirtualRouting::config;
vector<double> VirtualRouting::weights;
vector<double>* VirtualRouting::dCompare;
double VirtualRouting::totalConsumed;
double VirtualRouting::maxConsumed;
int VirtualRouting::maxTxSize;
vector<double> VirtualRouting::energyConsumeds;
vector<double> VirtualRouting::rxSizes;
int VirtualRouting::totalCollected;
void VirtualRouting::initialize()
{
  GlobalLocationService::initialize(getParentModule()->getParentModule()->getParentModule());
  if (!initialized) {
    initialized = true;
    srand(time(NULL));
  }
	maxNetFrameSize = par("maxNetFrameSize");
	netDataFrameOverhead = par("netDataFrameOverhead");
	netBufferSize = par("netBufferSize");
	roundLength = par("roundLength");
	L_max = par("maxUAVLength");
	numPacketReceived = 0;

	/* Get a valid references to the Resources Manager module and the
	 * Radio module, so that we can make direct calls to their public methods
	 */
	radioModule = check_and_cast <Radio*>(getParentModule()->getSubmodule("Radio"));
	resMgrModule = check_and_cast <ResourceManager*>(getParentModule()->getParentModule()->getSubmodule("ResourceManager"));


	if (!resMgrModule || !radioModule)
		opp_error("\n Virtual Routing init: Error in geting a valid reference module(s).");

	self = getParentModule()->getParentModule()->getIndex();
    neighborTable = GlobalLocationService::getNeighborTable(self);
    selfLocation = GlobalLocationService::getLocation(self);
	// create the routing level address using self
	stringstream out; out << self; 	selfAddress = out.str();

	isSink = par("isSink"); 
	numNodes = getParentModule()->getParentModule()->getParentModule()->par("numNodes");
	numUAVs = getParentModule()->getParentModule()->getParentModule()->par("numUAVs");
	roundNumber=0;

	cpuClockDrift = resMgrModule->getCPUClockDrift();
	setTimerDrift(cpuClockDrift);
	pktHistory.clear();

	disabled = true;
	currentSequenceNumber = 0;

	if (weights.empty()) weights.resize(numNodes);
	weights[self] = par("dataPacketSize");
	int dataScaleFactor = par("dataScaleFactor");
	weights[self] = weights[self]*4*dataScaleFactor;

	if (isSink) {
		sinkId = self;
		// weights.resize(numNodes, 250);
		// for (int i=0; i<numNodes; i++) {
		// 	int p = rand()%3;
		// 	if (p == 0) weights[i] = 500;
		// 	else if (p == 1) weights[i] = 1000;
		// }
		energyConsumeds.resize(numNodes);
		rxSizes.resize(numNodes);
	}

	declareOutput("Buffer overflow");
	declareOutput("Setup enery");
	declareOutput("Setup time");
	declareOutput("Collect energy");
	declareOutput("Collect time");
	declareOutput("End energy");
	declareOutput("End time");
	declareOutput("Round energy");
	declareOutput("preList size");
}

// A function to send control messages to lower layers
void VirtualRouting::toMacLayer(cMessage * msg)
{
	if (msg->getKind() == NETWORK_LAYER_PACKET)
		opp_error("toMacLayer() function used incorrectly to send NETWORK_LAYER_PACKET without destination MAC address");
	send(msg, "toMacModule");
}

// A function to send packets to MAC, requires a destination address
void VirtualRouting::toMacLayer(cPacket * pkt, int destination)
{
	RoutingPacket *netPacket = check_and_cast <RoutingPacket*>(pkt);

  // energy ---------
	double byteLength = netPacket->getByteLength();
	// for (int i=0; i<10; i++) trace() << "OK " << destination;
	Point nextHopLocation = GlobalLocationService::getLocation(destination);
	// for (int i=0; i<10; i++) trace() << "OK2";
	double distance = G::distance(selfLocation, nextHopLocation);
	// resMgrModule->consumeEnergy(txEnergy(byteLength, distance));
	// --------------

	netPacket->getNetMacInfoExchange().nextHop = destination;
	if (destination == -1) {
		// for (int i=0; i<10; i++) trace() << "toMacModule";
	    send(netPacket, "toMacModule");
		// for (int i=0; i<10; i++) trace() << "finished";
	} else {
//        debugLine(selfLocation, GlobalLocationService::getLocation(destination), "black");
        cModule* desModule = getParentModule()->getParentModule()->getParentModule()->getSubmodule("node", destination)
            ->getSubmodule("Communication")->getSubmodule("Routing");
			// for (int i=0; i<10; i++) trace() << "toMacModule";
        sendDirect(netPacket, desModule, "fromDirect");
		// for (int i=0; i<10; i++) trace() << "finished";
	}

//
}

void VirtualRouting::toApplicationLayer(cMessage * msg)
{
	send(msg, "toCommunicationModule");
}

void VirtualRouting::encapsulatePacket(cPacket * pkt, cPacket * appPkt)
{
	RoutingPacket *netPkt = check_and_cast <RoutingPacket*>(pkt);
	// set the size to just the overhead. encapsulate(appPkt)
	// will add the size of the app packet automatically
	netPkt->setByteLength(netDataFrameOverhead);
	netPkt->setKind(NETWORK_LAYER_PACKET);
	netPkt->setSource(SELF_NETWORK_ADDRESS);
	netPkt->setSequenceNumber(currentSequenceNumber++);
	netPkt->encapsulate(appPkt);
}

cPacket* VirtualRouting::decapsulatePacket(cPacket * pkt)
{
	RoutingPacket *netPkt = check_and_cast <RoutingPacket*>(pkt);
	ApplicationPacket *appPkt = check_and_cast <ApplicationPacket*>(netPkt->decapsulate());

	appPkt->getAppNetInfoExchange().RSSI = netPkt->getNetMacInfoExchange().RSSI;
	appPkt->getAppNetInfoExchange().LQI = netPkt->getNetMacInfoExchange().LQI;
	appPkt->getAppNetInfoExchange().source = netPkt->getSource();
	return appPkt;
}

void VirtualRouting::handleMessage(cMessage * msg)
{
	int msgKind = msg->getKind();
	if (disabled && msgKind != NODE_STARTUP)
	{
		delete msg;
		return;
	}

	switch (msgKind) {

		case NODE_STARTUP:
		{
			disabled = false;
			send(new cMessage("Network --> Mac startup message", NODE_STARTUP), "toMacModule");
			startup();
			break;
		}

		case APPLICATION_PACKET:
		{
			ApplicationPacket *appPacket = check_and_cast <ApplicationPacket*>(msg);
			if (maxNetFrameSize > 0 && maxNetFrameSize < appPacket->getByteLength() + netDataFrameOverhead)
			{
				trace() << "Oversized packet dropped. Size:" << appPacket->getByteLength() <<
				    ", Network layer overhead:" << netDataFrameOverhead <<
				    ", max Network packet size:" << maxNetFrameSize;
				break;
			}
			// trace1() << "Received [" << appPacket->getName() << "] from application layer";

			/* Control is now passed to a specific routing protocol by calling fromApplicationLayer()
			 * Notice that after the call we RETURN (not BREAK) so that the packet is not deleted.
			 * This is done since the packet will most likely be encapsulated and forwarded to the
			 * MAC layer. If the protocol specific function wants to discard the packet is has
			 * to delete it.
			 */
			numPacketReceived++;
			fromApplicationLayer(appPacket, appPacket->getAppNetInfoExchange().destination.c_str());
			return;
		}

		case NETWORK_LAYER_PACKET:
		{
			RoutingPacket *netPacket = check_and_cast <RoutingPacket*>(msg);
			/* trace() << "Received [" << netPacket->getName() << "] from MAC layer"; */
			NetMacInfoExchange_type info = netPacket->getNetMacInfoExchange();

			/* Control is now passed to a specific routing protocol by calling fromMacLayer()
			 * Notice that after the call we BREAK so that the NET packet gets deleted.
			 * This will not delete the encapsulated APP packet if it gets decapsulated
			 * by fromMacLayer(), i.e., the normal/expected action.
			 */
			numPacketReceived++;
      // energy ---------
      double byteLength = netPacket->getByteLength();
      Point nextHopLocation = GlobalLocationService::getLocation(info.lastHop);
      double distance = G::distance(selfLocation, nextHopLocation);
    //   resMgrModule->consumeEnergy(rxEnergy(byteLength));
      // --------------




			if (netPacket->getTTL() > 0) {
			  string dst(netPacket->getDestination());


			  netPacket->setTTL(netPacket->getTTL() - 1);
			  netPacket->setHopCount(netPacket->getHopCount() + 1);
        if (dst.compare(SELF_NETWORK_ADDRESS) == 0) {
          double hopCount = (double) netPacket->getHopCount();
          int s = atoi(netPacket->getSource());
          int d = atoi(netPacket->getDestination());
          endCount++;
        }
			  fromMacLayer(netPacket, info.lastHop, info.RSSI, info.LQI);
			}

			break;
		}

		case TIMER_SERVICE:{
			handleTimerMessage(msg);
			break;
		}

		case MAC_CONTROL_MESSAGE:{
			handleMacControlMessage(msg);
			return; // msg not deleted
		}

		case RADIO_CONTROL_MESSAGE:{
			handleRadioControlMessage(msg);
			return; // msg not deleted
		}

		case MAC_CONTROL_COMMAND:{
			toMacLayer(msg);
			return; // msg not deleted
		}

		case RADIO_CONTROL_COMMAND:{
			toMacLayer(msg);
			return; // msg not deleted
		}

		case NETWORK_CONTROL_COMMAND:{
			break;
		}

		case OUT_OF_ENERGY:{
			disabled = true;
			break;
		}

		case DESTROY_NODE:{
			disabled = true;
			break;
		}

		default:{
			opp_error("Network module recieved unexpected message: [%s]", msg->getName());
		}
	}

	delete msg;
}

// handleMacControlMessage needs to either process and DELETE the message OR forward it
void VirtualRouting::handleMacControlMessage(cMessage * msg)
{
	toApplicationLayer(msg);
}

// handleRadioControlMessage needs to either process and DELETE the message OR forward it
void VirtualRouting::handleRadioControlMessage(cMessage * msg)
{
	toApplicationLayer(msg);
}

void VirtualRouting::finish()
{
//  if (self == 801 || self == 0)
//    log() << resMgrModule->getSpentEnergy() << " days " << self;

  // trace1() << "WSN_EVENT STATISTICS " << "id:" << self << " totalPacketReceived:" << numPacketReceived << " "
    // << "estimateLifetime:" << resMgrModule->estimateLifetime() << " x:" << selfLocation.x() << " y:" << selfLocation.y()
    // << " endPointCount:" << endCount
    // << " energyConsumed:" << resMgrModule->getSpentEnergy();

	CastaliaModule::finish();
	cPacket *pkt;
	// clear the buffer from all remaining packets
	while (!TXBuffer.empty()) {
		pkt = TXBuffer.front();
		TXBuffer.pop();
		cancelAndDelete(pkt);
	}
}

int VirtualRouting::bufferPacket(cPacket * rcvFrame)
{
	if ((int)TXBuffer.size() >= netBufferSize) {
		collectOutput("Buffer overflow");
		cancelAndDelete(rcvFrame);
		return 0;
	} else {
		TXBuffer.push(rcvFrame);
		trace() << "Packet buffered from application layer, buffer state: " <<
				TXBuffer.size() << "/" << netBufferSize;
		return 1;
	}
}

int VirtualRouting::resolveNetworkAddress(const char *netAddr)
{
	if (!netAddr[0] || netAddr[0] < '0' || netAddr[0] > '9')
		return BROADCAST_MAC_ADDRESS;
	return atoi(netAddr);
}

bool VirtualRouting::isNotDuplicatePacket(cPacket * pkt)
{
	//extract source address and sequence number from the packet
	RoutingPacket *netPkt = check_and_cast <RoutingPacket*>(pkt);
	int src = resolveNetworkAddress(netPkt->getSource());
	unsigned int sn = netPkt->getSequenceNumber();

	//resize packet history vector if necessary
	if (src >= (int)pktHistory.size())
		pktHistory.resize(src+1);

	//search for this sequence number in the list, corresponding to address 'src'
	list<unsigned int>::iterator it1;
	for (it1 = pktHistory[src].begin(); it1 != pktHistory[src].end(); it1++) {
		//if such sequence number is found, packet is duplicate
		if (*it1 == sn) return false;
	}

	//no such sequence number found, this is new packet
	pktHistory[src].push_front(sn);
	if (pktHistory[src].size() > PACKET_HISTORY_SIZE)
		pktHistory[src].pop_back();
	return true;
}

void VirtualRouting::debugLine(double x1, double y1, double x2, double y2, string color) {
  trace() << "WSN_EVENT DRAW LINE 0 " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << color;
}
void VirtualRouting::debugLine(Point p1, Point p2, string color) {
  debugLine(p1.x(), p1.y(), p2.x(), p2.y(), color);
}
void VirtualRouting::debugCircle(double centerX, double centerY, double radius, string color) {
  trace() << "WSN_EVENT DRAW CIRCLE 0 " << centerX << " " << centerY << " " << radius << " " << color;
}
void VirtualRouting::debugCircle(Point center, double radius, string color) {
  debugCircle(center.x(), center.y(), radius, color);
}
void VirtualRouting::debugPoint(double x1, double x2, string color) {
  trace() << "WSN_EVENT DRAW POINT 0 " << x1 << " " << x2 << " " << color;
}
void VirtualRouting::debugPoint(Point p, string color) {
  debugPoint(p.x(), p.y(), color);
}

void VirtualRouting::debugArc(Point from, Point to, double radius, string color) {
  trace() << "WSN_EVENT DRAW ARC 0 " << from.x() << " " << from.y() << " " << to.x() << " "
    << to.y() << " " << radius << " " << color;
}

void VirtualRouting::debugPolygon(vector<Point> ps, string color) {
  for (int i = 0; i < ps.size(); i++) {
    Point from = ps[i];
    Point to = ps[(i + 1) % ps.size()];
    debugLine(from, to, color);
  }
}

void VirtualRouting::debugPath(vector<Point> ps, string color) {
  for (int i = 0; i < ps.size() - 1; i++) {
    Point from = ps[i];
    Point to = ps[(i + 1) % ps.size()];
    debugLine(from, to, color);
  }
}

////////////////////////////////////////
void VirtualRouting::debugLine(double x1, double y1, double x2, double y2, string color, int k) {
  trace() << "WSN_EVENT DRAW LINE " << k << " " << x1 << " " << y1 << " " << x2 << " " << y2 << " " << color;
}
void VirtualRouting::debugLine(Point p1, Point p2, string color, int k) {
  debugLine(p1.x(), p1.y(), p2.x(), p2.y(), color, k);
}
void VirtualRouting::debugCircle(double centerX, double centerY, double radius, string color, int k) {
  trace() << "WSN_EVENT DRAW CIRCLE " << k << " " << centerX << " " << centerY << " " << radius << " " << color;
}
void VirtualRouting::debugCircle(Point center, double radius, string color, int k) {
  debugCircle(center.x(), center.y(), radius, color);
}
void VirtualRouting::debugPoint(double x1, double x2, string color, int k) {
  trace() << "WSN_EVENT DRAW POINT " << k << " " << x1 << " " << x2 << " " << color;
}
void VirtualRouting::debugPoint(Point p, string color, int k) {
  debugPoint(p.x(), p.y(), color, k);
}

void VirtualRouting::debugArc(Point from, Point to, double radius, string color, int k) {
  trace() << "WSN_EVENT DRAW ARC " << k << " " << from.x() << " " << from.y() << " " << to.x() << " "
    << to.y() << " " << radius << " " << color;
}

void VirtualRouting::debugPolygon(vector<Point> ps, string color, int k) {
  for (int i = 0; i < ps.size(); i++) {
    Point from = ps[i];
    Point to = ps[(i + 1) % ps.size()];
    debugLine(from, to, color, k);
  }
}

void VirtualRouting::debugPath(vector<Point> ps, string color, int k) {
  for (int i = 0; i < ps.size() - 1; i++) {
    Point from = ps[i];
    Point to = ps[(i + 1) % ps.size()];
    debugLine(from, to, color, k);
  }
}

bool VirtualRouting::reached(Point location) {
  for (auto n: neighborTable) {
    if (G::distance(n.location, location) < G::distance(selfLocation, location)) {
      return false;
    }
  }

  return G::distance(selfLocation, location) < RADIO_RANGE;
}

int VirtualRouting::getRandomNumber(int from, int to) {
//    srand(time(0));
//    int result = from + rand() % (to - from + 1);
//    return result;
    int result = intrand(to - from + 1) + from;
    return result;
}

double VirtualRouting::txEnergy(double byteLength, double distance) {
	return 1 * (50e-9 * byteLength * 8 + 10e-12 * byteLength * 8 * distance * distance);
	// return 1 * (50e-7 * byteLength * 8 + 10e-10 * byteLength * 8 * distance * distance);
}

double VirtualRouting::rxEnergy(double byteLength) {
	return 1 * (50e-9 * byteLength * 8);
}

double VirtualRouting::calculateCHEnergy(int u, double byteLength) {
	return getResMgrModule(u)->getRemainingEnergy() - rxEnergy(byteLength) - txEnergy(byteLength + weights[u], D2UAV);
}

ResourceManager* VirtualRouting::getResMgrModule(int id) {
	if (isSink) return check_and_cast <ResourceManager*>(getParentModule()->getParentModule()
		->getParentModule()->getSubmodule("node", id)->getSubmodule("ResourceManager"));
	return nullptr;
}

double VirtualRouting::estimateMaxWeight(double Ec) {
	double w_max = 0;
	while (true) {
		w_max += 1;
		if (Ec < txEnergy(w_max, D2UAV)+rxEnergy(w_max)) break;
	}
	return w_max-1;
}

void VirtualRouting::sendAggregate() {
	for (int jj=0; jj<1; jj++) trace() << "sendAggregate " << self;
	energyConsumeds[self] = 0;
	rxSizes[self] = calculateRxSize0(self);
	//resMgrModule->resetBattery();
	// trace1() << "rxEnergy " << self << " " << rxSizes[self]/1000 << " " << rxEnergy(rxSizes[self]) << " " << resMgrModule->getRemainingEnergy();
	if (rxSizes[self] > 0) resMgrModule->consumeEnergy(rxEnergy(rxSizes[self]));
	totalConsumed += rxEnergy(rxSizes[self]);
	energyConsumeds[self] += rxEnergy(rxSizes[self]);
	double txSize = rxSizes[self] + weights[self];
	if (txSize > maxTxSize) maxTxSize = txSize;
	if (config.cent[self] == -1) {
		resMgrModule->consumeEnergy(txEnergy(txSize, D2UAV));
		// trace1() << "txEnergy " << self << " " << txSize/1000 << " " << txEnergy(txSize, D2UAV);
		// resMgrModule->consumeEnergy(rxEnergy(txSize));
		totalConsumed += txEnergy(txSize, D2UAV);
		energyConsumeds[self] += txEnergy(txSize, D2UAV);
		totalCollected += txSize;
		trace1() << self << " calculateRxSize " << rxSizes[self] << " calculateRxSize0 " << calculateRxSize(self) << " txSize " << txSize;
		if (energyConsumeds[self] > maxConsumed) maxConsumed = energyConsumeds[self];
		debugPoint(selfLocation, "red");
		debugCircle(selfLocation, resMgrModule->getRemainingEnergy()/500, "blue");
		// for (int jj=0; jj<10; jj++) trace() << "CH " << self << " " << config.next[self];
		for (auto trajectory : config.trajectories) {
			if (trajectory.empty()) continue;
			for (int i=0; i<trajectory.size()-1; i++) {
				// for (int jj=0; jj<10; jj++) trace() << i;
				if (trajectory[i] == self) {
					// for (int jj=0; jj<10; jj++) trace() << "trajectory " << trajectory[i+1];
					debugLine(selfLocation, location(trajectory[i+1]), "green");
					// for (int jj=0; jj<10; jj++) trace() << "energyConsumed " << totalConsumed;
					if (i == 1) {
						debugLine(selfLocation, location(trajectory[0]), "green");
						trace() << "i1 " << trajectory[0] << " " << self;
					}
					//trace1() << "energyConsumed " << totalConsumed << " " << energyConsumeds[self] << " " << config.next[self];
				}
			}
		}
	} else {
		double d2next = G::distance(selfLocation, GlobalLocationService::getLocation(config.next[self]));
		resMgrModule->consumeEnergy(txEnergy(txSize, d2next));
		totalConsumed += txEnergy(txSize, d2next);
		energyConsumeds[self] += txEnergy(txSize, d2next);
		// for (int jj=0; jj<10; jj++) trace() << "GN " << self << " " << config.next[self];
		debugLine(selfLocation, location(config.next[self]), "black");
		// trace1() << "txEnergy " << self << " " << txSize/1000 << " " << txEnergy(txSize, d2next);
	}

	// double Er = resMgrModule->getRemainingEnergy();
	// if (Er < E_min) {
	// 	E_min = Er;
	// 	debugString.clear();
	// }
	// for (int jj=0; jj<10; jj++) 
	// trace1() << "energyConsumed " << totalConsumed << " " << energyConsumeds[self] << " " << config.next[self];
}

double VirtualRouting::calculateRxSize(int curnode) {
	double rxSize = 0;
	queue<int> q;
	q.push(curnode);
	while (!q.empty()) {
		int u = q.front();
		q.pop();

		for (int i=0; i<numNodes; i++) {
			if (i == sinkId) continue;
			if (config.next[i] == u) {
				rxSize += weights[i];
				q.push(i);
			}
		}
	}
	return rxSize;
}

double VirtualRouting::calculateRxSize0(int curnode) {
	double rxSize = 0;
	queue<int> q;
	q.push(curnode);
	while (!q.empty()) {
		int u = q.front();
		q.pop();

		for (int i=0; i<numNodes; i++) {
			if (config.next[i] == u) {
				rxSize += weights[i];
				q.push(i);
			}
		}
	}
	return rxSize;
}

Point VirtualRouting::location(int id) {
	return GlobalLocationService::getLocation(id);
}

double VirtualRouting::calculatePathLength(vector<int> path) {
	if (path.empty()) return 0;
	double pathLength = 0;
	for (int i=0; i<path.size()-1; i++){
		pathLength += G::distance(GlobalLocationService::getLocation(path[i]), 
			GlobalLocationService::getLocation(path[i+1]));
	}
	return pathLength;
}

map<int, vector<int>> VirtualRouting::kMeansClustering(vector<int> nodes, int k) {
	// for (int jj=0; jj<10; jj++) trace() << "kMeansClustering";
	map<int, vector<int>> Clusters;
	vector<int> centers;
	vector<bool> isCenter (nodes.size(), false);
	for (int i=0; i<k; i++) {
		int ch = rand() % nodes.size();
		if (isCenter[ch]) i--;
		else {
			isCenter[ch] = true;
			Clusters.insert(pair<int, vector<int>>(ch, vector<int>()));
			centers.push_back(ch);
		}
	}

	double T_c = 0;
	for (int i : nodes) {
		double d_i = DBL_MAX;
		int center;
		for (int ch : centers) {
			double dist = G::distance(location(i), location(ch));
			if (dist < d_i) {
				d_i = dist;
				center = ch;
			}
		}
		T_c += d_i;
		Clusters[center].push_back(i);
	}
	
	double T_p;
	int n_iter = 0;
	do {
		// for (int jj=0; jj<10; jj++) trace() << "n_iter " << n_iter;
		T_p = T_c;
		T_c = 0;
		map<int, vector<int>> NewClusters;
		vector<int> newCenters;
		for (auto p : Clusters) {
			int ch = p.first;
			vector<int> Cluster = p.second;
			int i_min;
			double d_min = DBL_MAX;
			for (int i : Cluster) {
				double d_i = 0;
				for (int j : Cluster) {
					d_i += G::distance(location(i), location(j));
				}
				if (d_i < d_min) {
					d_min = d_i;
					i_min = i;
				}
			}
			newCenters.push_back(i_min);
			NewClusters.insert(pair<int, vector<int>>(i_min, vector<int>()));
		}

		for (int i : nodes) {
			double d_i = DBL_MAX;
			int center;
			for (int ch : newCenters) {
				double dist = G::distance(location(i), location(ch));
				if (dist < d_i) {
					d_i = dist;
					center = ch;
				}
			}
			T_c += d_i;
			NewClusters[center].push_back(i);
		}
		Clusters = NewClusters;
		n_iter++;
	} while ((T_c != T_p) && (n_iter < 50));
	return Clusters;
}

bool Comparebydistance(int lhs, int rhs) {
	return (VirtualRouting::dCompare->at(lhs) > VirtualRouting::dCompare->at(rhs));
}

void VirtualRouting::logConfig() {
	auto next = config.next;
	auto cent = config.cent;
	auto trajectories = config.trajectories;

	int N = numNodes;
	for (int i=0; i<N; i++) {
		auto P = GlobalLocationService::getLocation(i);
		CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << P.x() << "\t" << P.y() << "\t" << weights[i] << "\t" << cent[i] << "\t" << i;
	}
	for (int i=0; i<N; i++) {
		if (cent[i] != -1) {
			auto P = GlobalLocationService::getLocation(i);
			auto Q = GlobalLocationService::getLocation(next[i]);
			CastaliaModule::trace2() << roundNumber << "\tLINE\tgreen\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
		}
	}
	for (int k=0; k<trajectories.size(); k++) {
		for (int i=0; i<trajectories[k].size()-1; i++){
			auto P = GlobalLocationService::getLocation(trajectories[k][i]);
			auto Q = GlobalLocationService::getLocation(trajectories[k][i+1]);
			CastaliaModule::trace2() << roundNumber << "\tLINE\tblack\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
		}
	}
}