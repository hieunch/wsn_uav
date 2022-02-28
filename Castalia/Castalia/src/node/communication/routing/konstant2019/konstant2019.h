//****************************************************************************
//*  Copyright (c) Federal University of Para, brazil - 2011                 *
//*  Developed at the Research Group on Computer Network and Multimedia      *
//*  Communication (GERCOM)     				             *
//*  All rights reserved    				                     *
//*                                                                          *
//*  Permission to use, copy, modify, and distribute this protocol and its   *
//*  documentation for any purpose, without fee, and without written         *
//*  agreement is hereby granted, provided that the above copyright notice,  *
//*  and the author appear in all copies of this protocol.                   *
//*                                                                          *
//*  Module:   LEACH Clustering Protocol for Castalia Simulator              *
//*  Version:  0.2                                                           *
//*  Author(s): Adonias Pires <adonias@ufpa.br>                              *
//*             Claudio Silva <claudio.silva@itec.ufpa.br>                   *
//****************************************************************************/

#ifndef _konstant2019_H_
#define _konstant2019_H_

#include <queue>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <omnetpp.h>
#include <algorithm>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "VirtualRouting.h"
#include "VirtualApplication.h"	
#include "konstant2019Packet_m.h"
#include "ApplicationPacket_m.h"
#include "NoMobilityManager.h"
#include "GeoMathHelper.h"
#include "GlobalLocationService.h"
#include "UDG.h"
// #include "mainVrptw.h"
#include "mainTSP.h"
#include "mainTspga.h"

using namespace std;

struct CHInfo
{
	int src;
	double rssi;
};

struct NeighborInfo
{
	int id;
	double energy;
};

class konstant2019 : public VirtualRouting {

private:
	
	string applicationID;	

	queue <cPacket *> tempTXBuffer;
	vector <konstant2019Packet> bufferAggregate;
	int dataPacketSize;

	////////////////////////////////////////////////

	UDG graph;
    int N;
	double neighborRange;

	stringstream ss0;
	
    vector<double> distanceToCH;
	vector<list<int>> clusterMembers;
    double maxBallWeight;
    double minBallWeight;
    unordered_set<int> rmSet;
    vector<bool> isCH;

	double E_min;

	vector<double> E0;
	vector<double> E_tmp;

//////////////////////////////////////////////////


protected:

	void startup();
	void fromApplicationLayer(cPacket *, const char *);
	void fromMacLayer(cPacket *, int, double, double);
	void timerFiredCallback(int);
	void processBufferedPacket();

	void init();
	void reset();
	void mainAlg();
	vector<vector<int>> partitionIntoSectors();
	vector<vector<int>> basicToursPlanning();
	void findEnergyEfficientSolution();
	pair<double,string> calculateConsumption(vector<vector<int>> tours);
	void clusterTreeBuilding(vector<int> A);
};

#endif			
