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

#ifndef _VIRTUALROUTING_H_
#define _VIRTUALROUTING_H_

#include <queue>
#include <vector>
#include <omnetpp.h>

#include "CastaliaModule.h"
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "TimerService.h"
#include "CastaliaMessages.h"
#include "Radio.h"
#include "Util.h"
#include <sstream>
#include "ResourceManager.h"
#include "RoutingPacket_m.h"
#include "ApplicationPacket_m.h"
#include "GlobalLocationService.h"
#include "GeoMathHelper.h"
#include <chrono> 
#include <ctime>
using namespace std::chrono; 

#define SELF_NETWORK_ADDRESS selfAddress.c_str()
#define ROUTE_DEST_DELIMITER "#"
#define PACKET_HISTORY_SIZE 5
#define RADIO_RANGE 100
#define D2UAV 100

using namespace std;

struct NetworkConfig
{
	NetworkConfig() {};
	NetworkConfig(int numNodes, int numUAVs) {
		cent.resize(numNodes, -1);
		trajectories.resize(numUAVs);
	};

	void save(vector<int> A_, vector<int> cent_, vector<int> next_, vector<vector<int>> trajectories_) {
		A = A_;
		cent = cent_;
		next = next_;
		trajectories = trajectories_;
	}

	void save(NetworkConfig config) {
		A = config.A;
		cent = config.cent;
		next = config.next;
		trajectories = config.trajectories;
	}

	void clear() {
		A.clear();
		int numNodes = cent.size();
		int numUAVs = trajectories.size();
		cent.clear();
		trajectories.clear();
		cent.resize(numNodes, -1);
		trajectories.resize(numUAVs);
	}

	vector<int> A;
	vector<int> cent;
	vector<int> next;
	vector<vector<int>> trajectories;
};

class VirtualRouting: public CastaliaModule, public TimerService {
 public:
	static vector<double> *dCompare;
	static vector<double> weights;

 protected:
  	static bool initialized;
	static NetworkConfig config;
	string debugString;

	chrono::duration<double> time_elapse;
	static int totalCollected;
	double maxLengthRatio;
	
	/*--- The .ned file's parameters ---*/
	int maxNetFrameSize;		//in bytes
	int netDataFrameOverhead;	//in bytes
	int netBufferSize;			//in # of messages
	unsigned int currentSequenceNumber;

	int numPacketReceived;
	int endCount;

  	vector<NeighborRecord> neighborTable;
	/*--- Custom class parameters ---*/
	ResourceManager *resMgrModule;
	Point selfLocation;

	queue<int> controlPacketBuffer;
	vector<int> powers;
	
	queue< cPacket* > TXBuffer;
	vector< list< unsigned int> > pktHistory;

	double cpuClockDrift;
	bool disabled;

	Radio *radioModule;
	string selfAddress;
	int self;
	static int sinkId;
	bool isSink;
	int roundLength;
	int roundNumber;
	int numNodes;
	int numUAVs;
	int numExt;

	vector<int> A;
    vector<int> cent;
	vector<int> next;
	vector<vector<int>> trajectories;
	double L_max;

	double sensibility;
	double aggrConsumption;
	static double totalConsumed;
	static double maxConsumed;
	static vector<double> energyConsumeds;
	static vector<double> rxSizes;
	static int maxTxSize;

	virtual void initialize();
	virtual void startup() { }
	virtual void handleMessage(cMessage * msg);
	virtual void finish();

	virtual void fromApplicationLayer(cPacket *, const char *) = 0;
	virtual void fromMacLayer(cPacket *, int, double, double) = 0;

	int bufferPacket(cPacket *);

	void toApplicationLayer(cMessage *);
	void toMacLayer(cMessage *);
	void toMacLayer(cPacket *, int);
	bool isNotDuplicatePacket(cPacket *);
    int getRandomNumber(int from, int to);

	bool reached(Point location);

	void debugLine(double, double, double, double, string color);
	void debugLine(Point, Point, string color);
	void debugCircle(double, double, double, string color);
	void debugCircle(Point, double, string color);
	void debugPoint(double, double, string color);
	void debugPoint(Point, string color);
	void debugPolygon(vector<Point>, string color);
	void debugPath(vector<Point>, string color);
	void debugArc(Point from, Point to, double radius, string color);

	void debugLine(double, double, double, double, string color, int k);
	void debugLine(Point, Point, string color, int k);
	void debugCircle(double, double, double, string color, int k);
	void debugCircle(Point, double, string color, int k);
	void debugPoint(double, double, string color, int k);
	void debugPoint(Point, string color, int k);
	void debugPolygon(vector<Point>, string color, int k);
	void debugPath(vector<Point>, string color, int k);
	void debugArc(Point from, Point to, double radius, string color, int k);
	
	void logConfig();

	void encapsulatePacket(cPacket *, cPacket *);
	cPacket *decapsulatePacket(cPacket *);
	int resolveNetworkAddress(const char *);

	virtual void handleMacControlMessage(cMessage *);
	virtual void handleRadioControlMessage(cMessage *);

	double txEnergy(double, double);
	double rxEnergy(double);
	double calculateCHEnergy(int, double);
	ResourceManager* getResMgrModule(int);
	double estimateMaxWeight(double);
	void sendAggregate();
	double calculateRxSize(int i);
	double calculateRxSize0(int i);
	double calculatePathLength(vector<int>);
	map<int, vector<int>> kMeansClustering(vector<int> nodes, int k);
	// map<int, vector<int>> kMeansClustering(int k);

	Point location(int id);
	double distance(int u, int v) {
		if (u == v) return 0;
		return G::distance(location(u), location(v));
	}
};

bool Comparebydistance(int a, int b);

#endif				//_VIRTUALROUTING_H_
