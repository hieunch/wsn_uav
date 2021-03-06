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

#ifndef _GAMBAC_H_
#define _GAMBAC_H_

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
#include "VirtualRouting.h"
#include "VirtualApplication.h"	
#include "GAMBACPacket_m.h"
#include "ApplicationPacket_m.h"
#include "NoMobilityManager.h"
#include "GeoMathHelper.h"
#include "GlobalLocationService.h"
#include "UDG.h"
#include "Vrptw.h"
#include "mainTSP.h"
#include "mainTspga.h"

using namespace std;

enum GAMBACTimers {
	START_ROUND = 1,	
	START_MAINALG = 2,
	SEND_DATA = 3,
	END_ROUND = 4,
	JOIN_CH = 5,
	SEND_ADV = 6,
	SEND_CONTROL = 7,
};

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

class GAMBAC : public VirtualRouting {

private:
	
	string applicationID;	

	queue <cPacket *> tempTXBuffer;
	vector <GAMBACPacket> bufferAggregate;
	vector <int> clusterMembers;
	list <NeighborInfo> neighborTable;

	////////////////////////////////////////////////

	UDG graph;
    int N;
    int k0;
	double d0;

    int nloop;
	int countSuccess;
	int maxCHperUAV;
	
    vector<double> d2CH;
    map<int, double> ballWeight;
    double maxThres;
    double epsilon;
    double Eavg = 0;
	vector<list<int>> representSet;
    double maxBallWeight;
    double minBallWeight;
    unordered_set<int> rmSet;
	vector<list<int>> centList;
    vector<bool> isCH;
	vector<int> fringeSet;
	vector<int> innerSet;
	vector<int> A2;
	vector<int> fringeSet_0;
	vector<int> innerSet_0;
	vector<int> A2_0;
	double epsilon0;
	double epsilon_saved;
	int countAdjusment;
	bool debugRecruitProcess;

	double Wt;
	double Wt_opt;
	double Wt_max;
	double Wt_min;
	vector<double> w_max;
	double E_opt;
	double E_max;
	double E_min;
	double E0_min;
	double w_min;
	double w_total;

	double gamma;
	vector<double> E_tmp;

//////////////////////////////////////////////////


protected:

	void startup();
	void fromApplicationLayer(cPacket *, const char *);
	void fromMacLayer(cPacket *, int, double, double);
	void timerFiredCallback(int);
	void processBufferedPacket();

	void GPinit();
	void reset();
	void mainAlg();
	vector<int> TZ_sample(vector<int>W, double s);
	void growBalls(vector<int> lanmarkSet);
	void growBalls2(vector<int> lanmarkSet);
	double computeClusterWeight(int uNode);
	vector<int> verifyFringeSet();
	vector<int> TZ_sample2(vector<int> W, double b);
	void computeBallWeight();
	void recruitNewCHsAlpha();
	void clearData();
	int constructClusters(int k);
	void updateNodeWeight();
	void updateCentList();
	void buildTrajectories();
	vector<int> sortedCHVector();
	double computeWeight(list<int>);
};

#endif			
