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
	int dataPacketSize;

	////////////////////////////////////////////////

	UDG graph;
    int N;
    int k0;
	double neighborRange;

    int nloop;
	int countSuccess;
	int maxCHperUAV;
	
    vector<double> distanceToCH;
    map<int, double> ballWeight;
    double maxThres;
    double epsilon;
    double Eavg = 0;
	vector<list<int>> clusterMembers;
    double maxBallWeight;
    double minBallWeight;
    unordered_set<int> rmSet;
	vector<list<int>> centList;
    vector<bool> isCH;
	vector<int> outerSet;
	vector<int> innerSet;
	vector<int> A2;
	vector<int> fringeSet_0;
	vector<int> innerSet_0;
	vector<int> A2_0;
	double epsilon0;
	double epsilon_saved;
	int countAdjusment;
	bool debugRecruitProcess;

	double W;
	double W_opt;
	double W_start;
	double W_end;
	vector<double> w_max;
	double E_opt;
	double E_max;
	double E_min;
	double E0_min;
	double w_min;
	double W_total;

	double gamma;
	vector<double> E_tmp;

//////////////////////////////////////////////////


protected:

	void startup();
	void fromApplicationLayer(cPacket *, const char *);
	void fromMacLayer(cPacket *, int, double, double);
	void timerFiredCallback(int);
	void processBufferedPacket();

	void GAMBACinit();
	void reset();
	void mainAlg();
	vector<int> randomFromSet(vector<int> Candidates, double s);
	void growBalls(vector<int> lanmarkSet);
	void growBalls2(vector<int> lanmarkSet);
	double computeClusterWeight(int uNode);
	vector<int> getOuterOversizePart();
	vector<int> samplingCH(vector<int> Candidates, double b);
	void computeBallWeight();
	void recruitNewCHs();
	int buildDFT(double W);
	void updateNodeWeight();
	void updateCentList();
	void buildTrajectories();
	void buildTrajectories(bool isBreak);
	vector<vector<int>> buildInitialTrajectories();
	vector<int> TSP(vector<int>);
	vector<int> sortedCHVector();
	double computeWeight(list<int>);
};

#endif			
