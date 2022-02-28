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

#include "konstant2019.h"
#include <chrono>
#include <stdio.h>

Define_Module(konstant2019);

void konstant2019::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	neighborRange = par("neighborRange");
	dataPacketSize = par("dataPacketSize");
	if (isSink) init();
		
	setTimer(START_ROUND, 50);
}

void konstant2019::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		konstant2019Packet *netPacket = new konstant2019Packet("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setKonstant2019PacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSourceAddress(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void konstant2019::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	konstant2019Packet *netPacket = dynamic_cast <konstant2019Packet*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getKonstant2019PacketKind()) {

		case GP_ROUTING_DATA_PACKET:{
			string dst(netPacket->getDestinationAddress());
			if (dst.compare(SELF_NETWORK_ADDRESS) == 0){
				int sourceId = netPacket->getSourceId();
				if (isSink) {
					toApplicationLayer(decapsulatePacket(netPacket));
				} else {
					bufferAggregate.push_back(*netPacket);	
				}
			}
			break;
		}
	}
}

void konstant2019::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			if (roundNumber == 0) {
				resMgrModule->resetBattery();
				if (isSink)
					trace1() << "Energy\tE_min\ti_min\tlevel\tconfig.clus_id[i_min]\tcalculateRxSize(i_min)\tweights[i_min]\tE_total/numNodes\ttotalConsumed\tmaxConsumed\tmaxTxSize\tconfig.A.size()\tdevE\tavgConsumed\tobjective value\tE0_min\ttime_elapse\ttotalCollected\tmaxLengthRatio";
			}
			
			if (isSink) {
				trace() << "START_ROUND";
				setTimer(START_MAINALG, 1);
				setTimer(END_ROUND, 3);

				E_min = DBL_MAX;
				for (int i=0; i<numNodes; i++) {
					if (i == self) continue;
					double E_i = getResMgrModule(i)->getRemainingEnergy();
					if (E_i < E_min) {
						E_min = E_i;
					}
				}
			} else {
				setTimer(SEND_DATA, 2);
			}
			roundNumber++;
			setTimer(START_ROUND, roundLength);
			
			// if( remove( "network16000_konstant_trace.txt" ) != 0 )
			// 	trace1() << ( "Error deleting file" );
			// else
			// 	trace1() << ( "File successfully deleted" );
			// for (int i=0; i<5; i++) trace() << "setTimer " << (roundLength + simTime());
			break;
		}
		case START_MAINALG:{	
			totalConsumed = 0;
			maxConsumed = 0;
			maxTxSize = 0;
			auto start = std::chrono::steady_clock::now();
			totalCollected = 0;
			mainAlg();
			logConfig();
			auto end = std::chrono::steady_clock::now();
			time_elapse = end - start;
			break;
		}
		
		case SEND_DATA:{
			trace() << "SEND_DATA";
			RoutingPacket *dataPacket = new RoutingPacket("Routing data packet", NETWORK_LAYER_PACKET);;
			dataPacket->setByteLength(dataPacketSize);
			dataPacket->setKind(NETWORK_LAYER_PACKET);
			dataPacket->setSource(self);
			dataPacket->setSourceAddress(SELF_NETWORK_ADDRESS);
			dataPacket->setDestinationAddress(to_string(config.clus_id[self]).c_str());
  			dataPacket->setTTL(1000);
			sendData(dataPacket);
			break;
		}
		case END_ROUND:{	
			int i_min;	
			double E0_min = E_min;
			E_min = DBL_MAX;
			double E_total = 0;
			stringstream ss;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				double E_i = getResMgrModule(i)->getRemainingEnergy();
				if (E_i < E_min) {
					E_min = E_i;
					i_min = i;
				}
				E_total += E_i;
				ss << E_i << " ";
			}
			trace1() << "E " << ss.str();

			int level = 0;
			int current = i_min;
			while (config.clus_id[current] != -1) {
				level++;
				current = config.clus_id[current];
			}
			double E_avg = E_total/(numNodes-1);
			double devE = 0;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				double E_i = getResMgrModule(i)->getRemainingEnergy();
				devE += (E_i - E_avg) * (E_i - E_avg);
			}
			devE = devE/(numNodes-1);
			devE = sqrt(devE);
			double avgConsumed = 0;
			for (int i=0; i<numNodes; i++) {
				if (i == self) continue;
				avgConsumed += energyConsumeds[i];
			}
			avgConsumed = avgConsumed/(N-1);
			double objVal = -E_min/E0_min/2 + 1000*avgConsumed/1.8937;
			// trace1() << "E_min " << E_min;
			trace1() << "Energy\t" << E_min << "\t" << i_min << "\t" << level << "\t" << config.clus_id[i_min] << "\t" << calculateRxSize(i_min) << "\t" << weights[i_min] << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE << "\t" << avgConsumed << "\t" << objVal  << "\t" << E0_min << "\t" << time_elapse.count() << "\t" << totalCollected << "\t" << maxLengthRatio;
			break;
		}
	}
}

void konstant2019::processBufferedPacket()
{
		
}



void konstant2019::mainAlg() {
	for (int i=0; i<1; i++) trace1() << "mainAlg";
	reset();
	for (int u=0; u<numNodes; u++) E_tmp[u] = getResMgrModule(u)->getRemainingEnergy();
	findEnergyEfficientSolution();

	for (int k=0; k<numUAVs; k++) {
		// trace1() << "trajectory " << k << " " << calculatePathLength(config.trajectories[k]);
		stringstream ss;
		for(int l : config.trajectories[k]) {
			ss << l << " ";
		}
		for (int ii=0; ii<1; ii++) trace1() << ss.str();
	}
	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;
	}
	for (int ii=0; ii<1; ii++) trace1() << "maxLength " << maxLength;
	maxLengthRatio = maxLength / L_max;
	// trace1() << "A size " << config.A.size();
}


void konstant2019::init() {
	N = numNodes;
	graph.init(numNodes, self, neighborRange);
	trajectories.resize(numUAVs);

	distanceToCH.resize(N, 0.);
	clus_id.resize(N, -1);
	nextHop.resize(N, -1);
	isCH.resize(N, false);
	clusterMembers.resize(N);
	E_tmp = vector<double>(numNodes, 0);
	E0.resize(N);
	for (int u=0; u<N; u++) {
		E0[u] = getResMgrModule(u)->getInitialEnergy() + 0.04;
	}
}

void konstant2019::reset() {
	trajectories = vector<vector<int>>(numUAVs);

	A.clear();
	distanceToCH = vector<double>(N, 0.);
	clus_id = vector<int>(N, -1);
	nextHop = vector<int>(N, -1);
	isCH = vector<bool>(N, false);
	clusterMembers = vector<list<int>>(N);
}

void konstant2019::clusterTreeBuilding(vector<int> CHSet){
	for (int u : graph.getNodesExceptSink()){
		if (isCH[u]){
			distanceToCH[u] = 0;
		}
		else {
			distanceToCH[u] = DBL_MAX;
		}
		clus_id[u] = -1;
		nextHop[u] = -1;
		clusterMembers[u].clear();
	}
	vector<double> vals(numNodes, 0);
	vector<double> d(numNodes, DBL_MAX);
	for (int u : CHSet) {
		d[u] = 0;
		vals[u] = -DBL_MAX;
		clusterMembers[u].clear();
		clus_id[u] = u;
		nextHop[u] = -1;
	}

	unordered_set<int> removedSet;

	dCompare =  &vals;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : CHSet) {
		queue.push(l);
	}
	
	removedSet.clear();
	int countLoop = 0;
	while (!queue.empty()) {
		int u = queue.top();
		if (countLoop++ > 1600000) trace() << u;
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		if (removedSet.size() == numNodes-1) break;
		if (nextHop[u] >= 0) d[u] = d[nextHop[u]] + graph.getLength(u, nextHop[u]);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = - E_tmp[u] / (d[u] + graph.getLength(u,v)) / (d[u] + graph.getLength(u,v));
			
			if (alt - vals[v] < -EPSILON){
				vals[v] = alt;
				clus_id[v] = clus_id[u];
				nextHop[v] = u;
				queue.push(v);
			}
		}
	}
}


vector<vector<int>> konstant2019::partitionIntoSectors() {
	trace() << "partitionIntoSectors";
	vector<vector<int>> Sectors;
	for (int k=0; k<numUAVs; k++) Sectors.push_back(vector<int>());
	int i0 = rand() % numNodes;
	if (i0 == sinkId) i0 = (sinkId+1) % numNodes;
	Point sinkLocation = location(sinkId);

	auto P = GlobalLocationService::getLocation(i0);

	vector<double> PE (numNodes);
	double P_total = 0;
	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		PE[i] = (E0[i] - getResMgrModule(i)->getRemainingEnergy()) / E0[i];
		P_total += PE[i];
	}
	vector<int> sortedList;
    for (int i = 0; i < numNodes; i++) {
		if (i == sinkId) continue;
        Angle angle_i = G::angle(sinkLocation, location(i0), sinkLocation, location(i));

		// trace() << "angle " << sinkId << " " << i0 << " " << i << " " << angle_i;

		unsigned int j = 0;
        for (j = 0; j < sortedList.size(); j++) {
			Angle angle_j1 = G::angle(sinkLocation, location(i0), sinkLocation, location(sortedList[j]));
            if (angle_j1 > angle_i) {
                sortedList.insert(sortedList.begin() + j, i);
                break;
            }
        }

        if (sortedList.empty() || (j == sortedList.size())) {
            sortedList.push_back(i);
        }
    }
	int sectorId = 0;
	double sectorPerclus_id = P_total/numUAVs;
	for (int u : sortedList) {
		Sectors[sectorId].push_back(u);
		sectorPerclus_id -= PE[u];
		if (sectorPerclus_id < 0) {
			sectorId++;
			sectorPerclus_id = P_total/numUAVs;
		}
	}

	vector<vector<int>> RaySectors (numUAVs);
	for (int k=0; k<numUAVs; k++) {
		vector<int> sector = Sectors[k];
		// trace() << "RaySectors " << k << " size " << sector.size();
		int u0 = sector[0];
		Angle maxAngle = G::angle(sinkLocation, location(u0), sinkLocation, location(sector[sector.size()-1]));
		Angle ray2Angle = maxAngle/4;
		Angle ray4Angle = maxAngle*3/4;
		// trace() << "maxAngle " << maxAngle << " ray2Angle " << ray2Angle << " ray4Angle " << ray4Angle;
		for (int u : sector) {
			Angle uAngle = G::angle(sinkLocation, location(u0), sinkLocation, location(u));
			// trace() << "u " << u << " uAngle " << uAngle;
			if ((uAngle >= ray2Angle) && (uAngle <= ray4Angle)) {
				unsigned int j = 0;
				for (j = 0; j < RaySectors[k].size(); j++) {
					if (G::distance(location(RaySectors[k][j]), sinkLocation) > G::distance(location(u), sinkLocation)) {
						RaySectors[k].insert(RaySectors[k].begin() + j, u);
						break;
					}
				}

				if (RaySectors[k].empty() || (j == RaySectors[k].size())) {
					RaySectors[k].push_back(u);
				}
			}
		}
	}
	return RaySectors;
}

vector<vector<int>> konstant2019::basicToursPlanning() {
	vector<vector<int>> Sectors = partitionIntoSectors();
	for (int ii=0; ii<1; ii++) trace1() << "basicToursPlanning";
	
	vector<vector<int>> basicTours;
	for (int k=0; k<numUAVs; k++) {
		vector<Point> newTourPoint;
		vector<Point> innerPoint;
		innerPoint.push_back(location(sinkId));
		trace() << "basicToursPlanning 1";
		if (Sectors[k].size() >= 1) {
			innerPoint.push_back(location(Sectors[k][0]));
			newTourPoint = innerPoint;
			double per;
			if (Sectors[k].size() >= 2) {
				int i = 1;
				trace() << "basicToursPlanning 2";
				int n_min = 0;
				int n_max = Sectors[k].size()-1;
				int n_mid;
				do {
					trace() << i << " " << Sectors[k][i];
					n_mid = (n_min+n_max)/2;
					innerPoint.clear();
					innerPoint.push_back(location(sinkId));
					for (int j=0; j<=n_mid; j++) {
						innerPoint.push_back(location(Sectors[k][j]));
					}
					vector<Point> convexHull = G::convexHull(innerPoint);
					per = G::polygonPerimeter(convexHull);
					if (per <= L_max) {
						newTourPoint = convexHull;
						n_min = n_mid+1;
					} else {
						n_max = n_mid-1;
					}
					i++;
				} while (n_max >= n_min);
			}
		}
		vector<int> newTour;
		for (Point p : newTourPoint) newTour.push_back(GlobalLocationService::getId(p));
		basicTours.push_back(newTour);
	}
	for (int k=0; k<numUAVs; k++) {
		trace() << "tour " << k << " size " << basicTours[k].size();
		trace() << " length " << GlobalLocationService::convexHullPerimeter(basicTours[k]);
		stringstream ss;
		if (!basicTours[k].empty()) for(int l : basicTours[k]) {
			ss << l << " ";
		}
		trace() << ss.str();
	}
	return basicTours;
}

void konstant2019::findEnergyEfficientSolution() {
	vector<vector<int>> basicTours = basicToursPlanning();
	for (int k=0; k<basicTours.size(); k++) {
		for (int i=0; i<basicTours[k].size()-1; i++){
			auto P = GlobalLocationService::getLocation(basicTours[k][i]);
			auto Q = GlobalLocationService::getLocation(basicTours[k][i+1]);
			// CastaliaModule::trace2() << roundNumber << "\tLINE\tred\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
		}
	}

	vector<vector<int>> finalTours = basicTours;
	for (int ii=0; ii<1; ii++) trace1() << "findEnergyEfficientSolution";

	string str0 = "";

	vector<int> pointsNotInTour;
	for (int i=0; i<numNodes; i++) pointsNotInTour.push_back(i);
	for (int k=0; k<numUAVs; k++) {
		for (int u : basicTours[k]) {
			if (find(pointsNotInTour.begin(), pointsNotInTour.end(), u) != pointsNotInTour.end())
				pointsNotInTour.erase(find(pointsNotInTour.begin(), pointsNotInTour.end(), u));
		}
	}

	trace() << "findEnergyEfficientSolution 0";
	int retries = 1;
	int maxIter = 2000;
	stringstream ss_cnsmptn;
	stringstream ss_issaved;
	double cur_cnsmptn = DBL_MAX;//calculateConsumption(finalTours);
	string ss_rxsize_str;
	string ss_next_str;
	for (int i=0; i<retries; i++) {
		for (int j=0; j<maxIter; j++) {
			trace() << "iter " << j << " retry " << i;
			// if (j%10 == 0) trace1() << "iter " << j;
			int rdm;
			trace() << "findEnergyEfficientSolution 1";
			auto start = std::chrono::high_resolution_clock::now();
			do {
				int k = rand() % basicTours.size();
				if (basicTours[k].size() > 2) {
					int h = rand() % basicTours[k].size();
					if (basicTours[k][h] != sinkId) {
						pointsNotInTour.push_back(basicTours[k][h]);
						basicTours[k].erase(basicTours[k].begin() + h);
					}
				}
				rdm = rand() % 100;
				trace() << "rdm " << rdm;
			} while (rdm < 75);
			auto finish = std::chrono::high_resolution_clock::now();
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
			trace() << "TIME STAGE_1 " << microseconds.count() << " us";

			trace() << "findEnergyEfficientSolution 2";
			bool feasible;
			
			start = std::chrono::high_resolution_clock::now();
			do {
				int index = rand() % pointsNotInTour.size();
				int u = pointsNotInTour[index];
				feasible = false;
				random_shuffle ( basicTours.begin(), basicTours.end() );
				for (int k=0; k<numUAVs; k++)  {
					double minPer = DBL_MAX;
					vector<int> minTour;
					if (basicTours[k].size() > 8) continue;

					for (int h=0; h<basicTours[k].size(); h++) {
						vector<int> newTour;
						vector<Point> newTourPoint;
						for (int o=0; o<=h; o++) {
							newTour.push_back(basicTours[k][o]);
							newTourPoint.push_back(location(basicTours[k][o]));
						}
						newTour.push_back(u);
						newTourPoint.push_back(location(u));
						for (int o=h+1; o<basicTours[k].size(); o++) {
							newTour.push_back(basicTours[k][o]);
							newTourPoint.push_back(location(basicTours[k][o]));
						}

						double per = G::polygonPerimeter(newTourPoint);
						if (per < minPer) {
							minPer = per;
							minTour = newTour;
						}
					}
					if (minPer <= L_max) {
						basicTours[k] = minTour;
						pointsNotInTour.erase(find(pointsNotInTour.begin(), pointsNotInTour.end(), u));
						feasible = true;
						break;
					}
					if (feasible) break;
				}
			} while (feasible);
			finish = std::chrono::high_resolution_clock::now();
			microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
			trace() << "TIME STAGE_2 " << microseconds.count() << " us";

			trace() << "findEnergyEfficientSolution 3";
			start = std::chrono::high_resolution_clock::now();
			auto p_result = calculateConsumption(basicTours);
			double new_cnsmptn = p_result.first;
			trace() << "consumption cur " << cur_cnsmptn << " new " << new_cnsmptn;
			ss_cnsmptn << new_cnsmptn << " ";
			double delta = new_cnsmptn - cur_cnsmptn;
			// if (delta < 0 || exp(delta*j/maxIter) > ((double) rand() / RAND_MAX)) {
			if (cur_cnsmptn/j > new_cnsmptn/maxIter) {
				ss_issaved << 1 << " ";
				ss_rxsize_str = p_result.second;
				stringstream ss_next;
				for (int i=0; i<numNodes; i++) ss_next << nextHop[i] << " ";
				ss_next_str = ss_next.str();
				str0 = ss0.str();
				finalTours = basicTours;
				cur_cnsmptn = new_cnsmptn;
				config.save(A, clus_id, nextHop, trajectories);
				trace() << "SAVED";
				stringstream ss;
				for (auto trajectory: trajectories) {
					for (int u: trajectory) {
						ss << u << " ";
					}
				}
			}
			else ss_issaved << 0 << " ";
			finish = std::chrono::high_resolution_clock::now();
			microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
			trace() << "TIME STAGE_3 " << microseconds.count() << " us";
		}
	}
	trace1() << "ss_cnsmptn " << ss_cnsmptn.str();
	trace1() << "ss_issaved " << ss_issaved.str();
	trace() << "findEnergyEfficientSolution done";
}

pair<double, string> konstant2019::calculateConsumption(vector<vector<int>> tours) {
	trace() << "calculateConsumption";
	trace() << "A.size " << A.size();
	A.clear();
	for (int i=0; i<numNodes; i++)  isCH[i] = false;
	vector<int> startIndex;
	for (int k=0; k<numUAVs; k++) {
		for (int i=0; i<tours[k].size(); i++) {
			int l = tours[k][i];
			if (l == sinkId) {
				startIndex.push_back(i);
			}
			else if (!isCH[l]) {
				isCH[l] = true;
				A.push_back(l);
			}
		}
	}
	
	
	trace() << "growBalls";
	auto start = std::chrono::high_resolution_clock::now();
	clusterTreeBuilding(A);
	// growBalls(A);
	auto finish = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
	trace() << "TIME GROW_BALL " << microseconds.count() << " us";
	trace() << "growBalls end";
	trace() << "A.size " << A.size();
	for (int l : A) clus_id[l] = -1;
	for (int i=0; i<numNodes; i++) {
		if (i == sinkId) continue;
	}
	trace() << "trajectories";
	for (int k=0; k<numUAVs; k++) trajectories[k].clear();
	for (int k=0; k<numUAVs; k++) {
		for (int i=0; i<tours[k].size(); i++) {
			int trueIndex = (i+startIndex[k])%tours[k].size();
			trajectories[k].push_back(tours[k][trueIndex]);
		}
		trajectories[k].push_back(sinkId);
	}
	NetworkConfig oldConfig;
	oldConfig.save(config);
	config.save(A, clus_id, nextHop, trajectories);

	ss0.str("");

	start = std::chrono::high_resolution_clock::now();
	double rxSizes[numNodes];
	for (int u=0; u<numNodes; u++) { rxSizes[u]=0; }
	stringstream ss21;
	for (int u=0; u<numNodes; u++) {
		ss21 << rxSizes[u]/1000 << " ";
	}
	trace() << "rxSizes0 " << ss21.str();
	for (int u=0; u<numNodes; u++) {
		int tmp = u;
		while (config.nextHop[tmp] != -1) {
			rxSizes[config.nextHop[tmp]] += weights[u];
			tmp = config.nextHop[tmp];
		}
	}
	stringstream ss2;
	for (int u=0; u<numNodes; u++) {
		ss2 << rxSizes[u]/1000 << " ";
	}
	trace() << "rxSizes " << ss2.str();

	stringstream ss3, ss4;
	trace() << "calEnergy";
	double Er_min = DBL_MAX;
	double Er_total = 0;
	double new_cnsmptn = 0;
	for (int i=0; i<numNodes; i++) {
		double Er_i = E_tmp[i];
		if (i == sinkId) continue;
		double rxSize = rxSizes[i];//calculateRxSize(i);
		new_cnsmptn += rxEnergy(rxSize);
		ss3 << rxEnergy(rxSize) << " ";
		Er_i -= rxEnergy(rxSize);
		ss0 << "\ne_rxEnergy " << i << " " << rxEnergy(rxSize);
		double txSize = rxSize + weights[i];
		if (config.clus_id[i] == -1) {
			new_cnsmptn += txEnergy(txSize, D2UAV);
			Er_i -= txEnergy(txSize, D2UAV);
			ss0 << "\ne_txEnergy " << i << " " << txEnergy(txSize, D2UAV);
			ss4 << txEnergy(txSize, D2UAV) << " ";
		} else {
			double d2nextHop = G::distance(GlobalLocationService::getLocation(i), GlobalLocationService::getLocation(config.nextHop[i]));
			new_cnsmptn += txEnergy(txSize, d2nextHop);
			Er_i -= txEnergy(txSize, d2nextHop);
			ss0 << "\ne_txEnergy " << i << " " << txEnergy(txSize, d2nextHop);
			ss4 << txEnergy(txSize, d2nextHop) << " ";
		}
		Er_total += Er_i;
		if (Er_i < Er_min) Er_min = Er_i;
	}
	trace() << "rxEnergy " << ss3.str();
	trace() << "txEnergy " << ss4.str();
	finish = std::chrono::high_resolution_clock::now();
	microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
	trace() << "TIME CAL_ENERGY " << microseconds.count() << " us";
	config.save(oldConfig);
	double alpha = 0.9;
	// new_cnsmptn = - (alpha * Er_min + (1-alpha) * Er_total / (numNodes-1));
	string r_str = "";//ss2.str();
	return pair<double,string>(new_cnsmptn, r_str);
}