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

#include "GPRouting.h"

Define_Module(GPRouting);

void GPRouting::startup()
{	
	/*--- The .ned file's parameters ---*/
	applicationID = par("applicationID").stringValue(); 	

	d0 = par("d0");
	gamma = par("gamma");
	maxCHperUAV = par("maxCHperUAV");
	Wt_opt = -1;
	Wt_max = 0;
	countSuccess = 0;
	countAdjusment = 0;
	debugRecruitProcess = par("debugRecruitProcess");
	epsilon = par("epsilon");
	if (isSink) GPinit();
		
	setTimer(START_ROUND, 50);
}

void GPRouting::fromApplicationLayer(cPacket *pkt, const char *destination)
{	
	if(!isSink)
	{
		// weights[self] = pkt->getByteLength();
		string dst(destination);
		GPRoutingPacket *netPacket = new GPRoutingPacket("GP routing data packet", NETWORK_LAYER_PACKET);
		netPacket->setGPRoutingPacketKind(GP_ROUTING_DATA_PACKET);
		netPacket->setSource(SELF_NETWORK_ADDRESS);
		encapsulatePacket(netPacket, pkt);
		bufferAggregate.push_back(*netPacket);
	}	
}

void GPRouting::fromMacLayer(cPacket *pkt, int macAddress, double rssi, double lqi){
	GPRoutingPacket *netPacket = dynamic_cast <GPRoutingPacket*>(pkt);

	if (!netPacket)
		return;

	switch (netPacket->getGPRoutingPacketKind()) {

		case GP_ROUTING_DATA_PACKET:{
			string dst(netPacket->getDestination());
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

void GPRouting::timerFiredCallback(int index)
{
	switch (index) {
		
		case START_ROUND:{
			// for (int i=0; i<5; i++) trace() << "START_ROUND";
			if (roundNumber == 0) {
				resMgrModule->resetBattery();
				if (isSink) {
					trace1() << "Energy\tE_min\ti_min\tlevel\tconfig.cent[i_min]\tcalculateRxSize(i_min)\tweights[i_min]\tE_total/numNodes\ttotalConsumed\tmaxConsumed\tmaxTxSize\tconfig.A.size()\tdevE\tavgConsumed\tobjective value\tE0_min\ttime_elapse\ttotalCollected\tmaxLengthRatio";
					for (int u=0; u<numNodes; u++) E_0[u] = getResMgrModule(u)->getRemainingEnergy();
				}
			}
			
			if (isSink) {
				trace() << "START_ROUND";
				setTimer(START_CLUSTERING, 1);
				setTimer(END_ROUND, 3);

				E_min = DBL_MAX;
				for (int i=0; i<numNodes; i++) {
					if (i == self) continue;
					double E_i = getResMgrModule(i)->getRemainingEnergy();
					if (E_i < E_min) {
						E_min = E_i;
					}
				}
				E0_min = E_min;
			} else {
				setTimer(START_SLOT, 2);
			}
			roundNumber++;
			setTimer(START_ROUND, roundLength);
			// for (int i=0; i<5; i++) trace() << "setTimer " << (roundLength + simTime());
			break;
		}
		case START_CLUSTERING:{	
			totalConsumed = 0;
			maxConsumed = 0;
			maxTxSize = 0;
			auto start = std::chrono::steady_clock::now();
			totalCollected = 0;
			mainAlg3();
			trace1() << "next[sinkId] = " << config.next[sinkId];
			logConfig();
			auto end = std::chrono::steady_clock::now();
			time_elapse = end - start;
			break;
		}
		
		case START_SLOT:{
			sendAggregate();
			// trace() << "Send aggregated packet to " << -1;
			// processBufferedPacket();
			break;
		}
		case END_ROUND:{
			int i_min;
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
				// trace1() << "E " << i << " " << E_i << " " << config.cent[i] << " " << config.next[i] << " " << rxSizes[i]/1000 << " " << energyConsumeds[i];
			}
			trace1() << ss.str();

			int level = 0;
			int current = i_min;
			while (config.cent[current] != -1) {
				level++;
				current = config.next[current];
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
			trace1() << "Energy\t" << E_min << "\t" << i_min << "\t" << level << "\t" << config.cent[i_min] << "\t" << calculateRxSize(i_min) << "\t" << weights[i_min] << "\t" << E_total/numNodes << "\t" << totalConsumed << "\t" << maxConsumed << "\t" << maxTxSize << "\t" << config.A.size() << "\t" << devE << "\t" << avgConsumed << "\t" << objVal  << "\t" << E0_min << "\t" << time_elapse.count() << "\t" << totalCollected << "\t" << maxLengthRatio;
			// for (int u : fringeSet_0) debugPoint(location(u), "blue");
			// for (int u : innerSet_0) debugPoint(location(u), "yellow");
			// for (int u : A2_0) debugPoint(location(u), "green");
			break;
		}
	}
}

void GPRouting::processBufferedPacket()
{
		
}



void GPRouting::mainAlg() {

	for (int u=0; u<numNodes; u++) E_tmp[u] = getResMgrModule(u)->getRemainingEnergy();
	config.A.clear();

	for (int i=0; i<10; i++) trace() << "mainAlg";
	E_min = DBL_MAX;
	E_max = 0;
	for (int i=0; i<numNodes; i++) {
		if (i == self) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (E_i > E_max) E_max = E_i;
		if (E_i < E_min) E_min = E_i;
	}
	w_total = 0;
	for (int i=0; i<numNodes; i++) w_total += weights[i];
	Wt_max = 4*w_total/numUAVs;
	Wt_min = 1000;

	
	// int count2 = 0;
	// for (int k_min=8; k_min<=100; k_min+=2) {
	// 	Wt = (1 + epsilon) * w_total / k_min;
	// 	for (int ii=0; ii<10; ii++) {
	// 		reset();
	// 		clearData();
	// 		int loop_count = constructClusters(k_min);
	// 		buildTrajectories();
	// 		double ratio = 1;
	// 		double maxLen = 0;
	// 		double minLen = DBL_MAX;
	// 		double avgLen = 0;
	// 		for (int t=0; t<numUAVs; t++) {
	// 			double len = calculatePathLength(trajectories[t]);
	// 			if (len > maxLen) maxLen = len;
	// 			if (len < minLen) minLen = len;
	// 			avgLen += len;
	// 		}
	// 		avgLen /= numUAVs;

	// 		if (loop_count <= 51) {
	// 			ratio = sqrt(maxLen / L_max);
	// 			CastaliaModule::trace2() << "RESULT\t" << ii << "\tKMIN\t" << k_min << "\tK\t" << A.size() << "\tLMAX\t" << maxLen << "\tEPS\t" << epsilon0-1;
	// 		}
	// 		else {
	// 			CastaliaModule::trace2() << "RESULT_FAILED\t" << ii << "\tKMIN\t" << k_min << "\tK\t" << A.size() << "\tLMAX\t" << maxLen << "\tEPS\t" << epsilon0-1;
	// 		}

	// 		int N = numNodes;
	// 		for (int i=0; i<N; i++) {
	// 			auto P = GlobalLocationService::getLocation(i);
	// 			CastaliaModule::trace2() << "RESULT" << count2 << "\tPOINT\tblue\t" << P.x() << "\t" << P.y() << "\t" << weights[i] << "\t" << cent[i] << "\t" << i;
	// 		}
	// 		for (int i=0; i<N; i++) {
	// 			if (cent[i] != -1) {
	// 				auto P = GlobalLocationService::getLocation(i);
	// 				auto Q = GlobalLocationService::getLocation(next[i]);
	// 				CastaliaModule::trace2() << "RESULT" << count2 << "\tLINE\tgreen\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
	// 			}
	// 		}
	// 		for (int k=0; k<trajectories.size(); k++) {
	// 			for (int i=0; i<trajectories[k].size()-1; i++){
	// 				auto P = GlobalLocationService::getLocation(trajectories[k][i]);
	// 				auto Q = GlobalLocationService::getLocation(trajectories[k][i+1]);
	// 				CastaliaModule::trace2() << "RESULT" << count2 << "\tLINE\tred\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
	// 			}
	// 		}
	// 		CastaliaModule::trace2() << "RESULT" << count2 << "\tKMIN\t" << k_min << "\tK\t" << A.size() << "\tLMAX\t" << maxLen << "\tLMIN\t" << minLen << "\tLAVG\t" << avgLen << "\tEPS\t" << epsilon0-1;

	// 		count2++;
	// 	}
	// }
	// return;
	

	// if (Wt_opt != -1) {
	// 	if (countSuccess == 10) {
	// 		Wt_opt = -1;
	// 		countSuccess = 0;
	// 	}
	// 	else {
	// 		Wt_max = Wt_opt*3/2;
	// 		Wt_min = Wt_opt*1/2;
	// 	}
	// }
	Wt = (Wt_max+Wt_min)/2;

	// Wt_max = 116344;
	// Wt_min = 58171.9;

	// if (Wt_max == 0) {
	// 	Wt_max = 4*w_total/numUAVs;
	// 	Wt_min = 0;
	// } else {
	// 	double dW = (Wt_max-Wt_min)/2;
	// 	Wt_max = 4*Wt;
	// 	Wt_min = 0;
	// }

	for (int i=0; i<1; i++) trace() << "loop";
	bool fl;
	bool isSaved = false;
	int count = 0;
	double ratio = 1;
	double Wt_saved;
	double maxLen = 0;
	int k_start = 0;
	int k_end = 50;
	int k = numUAVs;
	vector<int> A_old_saved;
	do {
		// if (count == 5) {
		// 	Wt_opt = -1;
		// 	Wt_max = 4*w_total/numUAVs;
		// 	Wt_min = 0;
		// 	count = 0;
		// }

		reset();
		clearData();
		// Wt = (Wt_max+Wt_min)/2;

		// k = (k_start+k_end)/2;
		Wt = (1 + epsilon) * w_total / k;
		
		// if (Wt_opt != -1) {
		// 	Wt = Wt_max = Wt_min = Wt_opt;
		// }
		// Wt = 50976;
		for (int i=0; i<1; i++) trace1() << "k " << k << " k_start " << k_start << " k_end " << k_end;
		for (int i=0; i<1; i++) trace1() << "constructClusters";
		int loop_count = constructClusters(k);
		for (int i=0; i<1; i++) trace1() << "constructClusters " << loop_count;
		for (int i=0; i<1; i++) trace1() << "buildTrajectories " << A.size();
		fl = false;
		maxLen = 0;
		// if (loop_count <= 50) {
			buildTrajectories();
			for (int i=0; i<0; i++) trace1() << "done " << A.size();
			// vector<int> RPfull;
			// RPfull.push_back(self);
			// RPfull.insert(RPfull.end(), A.begin(), A.end());
			// trajectories[0] = mainTSP(RPfull);
			
			for (int i=0; i<10; i++) trace() << "calculatePathLength";
			for (int t=0; t<numUAVs; t++) {
				// for (int i=0; i<10; i++) trace() << t;
				double len = calculatePathLength(trajectories[t]);
				if (len > L_max) {
					// trace() << "calculatePathLength " << calculatePathLength(trajectories[t]);
					fl = true;
					// break;
				};
				if (len > maxLen) maxLen = len;
			}

			// ratio = sqrt(maxLen / L_max);
			ratio = pow(maxLen / L_max, 2);
		// }
		// else {
		// 	fl = true;
		// 	// ratio = 1;
		// }


		if (fl) {
			// if (Wt_opt != -1) {
			// 	// count++;
			// 	Wt_max = 4*w_total/numUAVs;
			// 	Wt_min = 0;
			// 	for (int i=0; i<10; i++) trace() << "continue " << count;
			// 	continue;
			// }
			for (int i=0; i<10; i++) trace() << "increase Wmin";
			
			// if (ratio < 1.1) count++;
			// else if (ratio < 1.2) count += 2;
			// else 
			count += 1;

			for (int i=0; i<1; i++) trace1() << "reduce num. clusters, max length = " << maxLen << ", ratio = " << ratio;
			if (count >= 5) {
				k_end = k;
				count = 0;
			}
			else continue;
		} else {
			// trace1() << "save config";
			if (A.size() > config.A.size() && loop_count <= 50){
				for (int i=0; i<1; i++) trace1() << "save config";
				config.save(A, cent, next, trajectories);
				A_old_saved = A_old;
				epsilon_saved = epsilon0;
				isSaved = true;
				k_start = k;
				// if (ratio > 0.95) break;
			}
			else if (A.size() > config.A.size()) {
				count += 1;

				if (count >= 5) {
					if (!isSaved) {
						for (int i=0; i<1; i++) trace1() << "save config 2";
						config.save(A, cent, next, trajectories);
						A_old_saved = A_old;
						epsilon_saved = epsilon0;
						isSaved = true;
					}
					k_start = k;
					count = 0;
				}
				else continue;
			}
			fringeSet_0 = fringeSet;
			innerSet_0 = innerSet;
			A2_0 = A2;
			k_start = k;
			for (int i=0; i<1; i++) trace1() << "increase num. clusters, max length = " << maxLen << ", ratio = " << ratio;
			count = 0;
			// if (Wt == Wt_opt) {
			// 	break;
			// }
		}

		if (k == (int)ceil(k/ratio)) break;
		trace() << "Alter k old value " << k << " new value " << (int)ceil(k/ratio) << " ratio " << ratio;
		k = ceil(k / ratio);
		if (k <= k_start) break;
		// if (k_start >= k_end) break;

		// if ((Wt_max-Wt_min>1000) && (Wt_max+Wt_min>4000)){
			
		// } else if (fl) {
		// 	Wt_opt = -1;
		// 	Wt_max = 4*w_total/numUAVs;
		// 	Wt_min = 0;
		// }

		if ((Wt_max-Wt_min>1000) && (Wt_max+Wt_min>1000)){
			
		} 
		
		if ((Wt_max-Wt_min<=10000) && !isSaved) {
			// Wt_max = 4*w_total/numUAVs;
			// Wt_min = 0;
			Wt_max = 4*w_total/numUAVs;
			Wt_min = 1000;
			Wt_opt = -1;
			countSuccess = 0;
		}

	} while (k_end > k_start+1); //((Wt_max-Wt_min>10000) && (Wt_max+Wt_min>1000));
	if (Wt_opt == -1) Wt_opt = Wt;
	countSuccess++;

	
	A = config.A;
	stringstream ss1;
	for(int l : A) {
		ss1 << l << " ";
	}
	for (int i=0; i<10; i++) trace1() << Wt << " " << ss1.str();

	isLandmark = vector<bool>(N, false);
	for (int l : A) isLandmark[l] = true;
	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (distance(i, sinkId) <= L_max/2)
			w_max[i] = Wt_saved;//*(E_i-E_min)/(E_max-E_min);
		else
			w_max[i] = weights[i];
	}
	clearData();
	growBallsKonstant(A);
	for (auto pair : ballWeight) {
		trace1() << "cluster " << pair.first << " weight " << pair.second << " threshold " << w_max[pair.first];
	}

	CastaliaModule::trace2() << roundNumber << "\tWTHR\t" << Wt_saved;
	CastaliaModule::trace2() << roundNumber << "\tEPS\t" << epsilon_saved;

	/*
	for (int i=0; i<10; i++) trace1() << "insertion start";
	int u_min;
	int insTra;
	int insPos;
	bool feasible;
	cent = config.cent;
	A = config.A;
	trajectories = config.trajectories;
	for (int k=0; k<trajectories.size(); k++) {
		for (int i=0; i<trajectories[k].size()-1; i++){
			auto P = GlobalLocationService::getLocation(trajectories[k][i]);
			auto Q = GlobalLocationService::getLocation(trajectories[k][i+1]);
			CastaliaModule::trace2() << roundNumber << "\tLINE\tred\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
		}
	}

	vector<double> lengths;	
	vector<int> Anew;
	for (int k=0; k<trajectories.size(); k++) {
		if (trajectories[k].empty()) {
			trajectories[k].push_back(sinkId);
			trajectories[k].push_back(sinkId);
		}
		lengths.push_back(calculatePathLength(trajectories[k]));
	}
	do {
		double minDist = DBL_MAX;
		u_min = -1;
		feasible = false;
		// for (int jj=0; jj<10; jj++) trace1() << "for " << lengths.size() << " " << trajectories.size();
		for (int u=0; u<numNodes; u++) {
			if (cent[u] == -1 || u == self) continue;
			for (int k=0; k<trajectories.size(); k++) {
				if (trajectories[k].size() >= maxCHperUAV) continue;
				for (int j=0; j<trajectories[k].size()-1; j++) {
					double newLength = lengths[k] - distance(trajectories[k][j], trajectories[k][j+1]) + distance(trajectories[k][j], u) + distance(u, trajectories[k][j+1]);
					if (newLength > L_max) continue;
					// double dist = G::distanceToLineSegment(GlobalLocationService::getLocation(trajectories[k][j]), GlobalLocationService::getLocation(trajectories[k][j+1]), GlobalLocationService::getLocation(u));
					if (newLength < minDist) {
						minDist = newLength;
						u_min = u;
						insTra = k;
						insPos = j+1;
					}
				}
			}
		}

		if (u_min != -1) {
			auto Tnew = trajectories[insTra];
			Tnew.insert(Tnew.begin()+insPos, u_min);

			double length = calculatePathLength(Tnew);
			if (length <= L_max) {
				feasible = true;
				A.push_back(u_min);
				Anew.push_back(u_min);
				isLandmark[u_min] = true;
				cent[u_min] = -1;
				trajectories[insTra] = Tnew;
				lengths[insTra] = length;
			}
		}

	} while (feasible);
	// isLandmark = vector<bool>(N, false);
	// for (int l : A) isLandmark[l] = true;
	for (int i=0; i<10; i++) trace1() << "insertion done";

	// Insertion alg. 1
	/*
	// stringstream ss;
	// for(int l : A) {
	// 	ss << l << " ";
	// }
	// for (int i=0; i<10; i++) trace1() << ss.str();

	// clearData();
	// for (int i=0; i<10; i++) trace1() << "clearData done";
	// growBalls2(Anew);
	

	// Insertion alg. 2
	
	for (int l : A) {
		cent[l] = l;
		next[l] = -1;
	}
	for (int l : Anew) {
		dLandmark[l] = 0;
		dCompare =  &dLandmark;
		priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
		queue.push(l);

		while (!queue.empty()) {
			int u = queue.top();
			queue.pop();
			
			for (int v : graph.getAdjExceptSink(u)) {
				if (cent[v] == v) continue;
				double alt = dLandmark[u] + graph.getLength(u,v);
				// if ((removedSet.find(v) != removedSet.end())) continue;				
				// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "if";
				if (alt < 1.1*dLandmark[v] && calculateCHEnergy(l, computeWeight(representSet[l])) > calculateCHEnergy(cent[v], computeWeight(representSet[cent[v]]))) {
					dLandmark[v] = dLandmark[u] + graph.getLength(u,v);
					representSet[cent[v]].remove(v);
					cent[v] = l;
					next[v] = u;
					representSet[l].push_back(v);
					queue.push(v);
				}
			}
		}
	}
	

	for (int i=0; i<10; i++) trace1() << "growBall done";
	cent[self] = -1;
	for (int l : A) cent[l] = -1;
	config.save(A, cent, next, trajectories);
	*/

	///////////////////////////////////////////////////////////

	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;

		stringstream ss;
		for(int l : T) {
			ss << l << " ";
		}
		trace1() << ss.str();
	}
	trace1() << "maxLength " << maxLength;
	maxLengthRatio = maxLength / L_max;
	for (int i=0; i<0; i++) trace1() << "END mainAlg";

	for (int i : A_old_saved) {
		auto P = GlobalLocationService::getLocation(i);
		CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << P.x() << "\t" << P.y() << "\t" << weights[i] << "\t" << cent[i] << "\t" << i;
	}

	// for (int u=0; u<N; u++) {
	// 	if (cent[u] >= 0) debugLine(location(u), location(cent[u]), "black");
	// }
	// for (int i=0; i<numUAVs; i++) {
	// 	for (int j=0; j<trajectories[i].size()-1; j++) {
	// 		debugLine(location(trajectories[i][j]), location(trajectories[i][j+1]), "red");
	// 	}
	// }
	// trace1() << "A size " << config.A.size();
	// E_min = DBL_MAX;
}

void GPRouting::mainAlg2() {

	for (int u=0; u<numNodes; u++) E_tmp[u] = getResMgrModule(u)->getRemainingEnergy();
	config.A.clear();

	for (int i=0; i<10; i++) trace1() << "mainAlg";
	E_min = DBL_MAX;
	E_max = 0;
	for (int i=0; i<numNodes; i++) {
		if (i == self) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (E_i > E_max) E_max = E_i;
		if (E_i < E_min) E_min = E_i;
	}
	w_total = 0;
	for (int i=0; i<numNodes; i++) w_total += weights[i];
	double Wt_end = 4*w_total/numUAVs;
	double Wt_start = 1000;

	
	Wt = (Wt_end+Wt_start)/2;

	for (int i=0; i<1; i++) trace() << "loop";
	bool fl;
	bool isSaved = false;
	int count = 0;
	double ratio = 1;
	double Wt_saved;
	double maxLen = 0;
	vector<int> A_old_saved;
	do {
		reset();
		clearData();
		
		for (int i=0; i<1; i++) trace1() << "Wt " << Wt << " Wt_start " << Wt_start << " Wt_end " << Wt_end;
		for (int i=0; i<1; i++) trace1() << "constructClusters";
		int loop_count = constructClusters(0);
		for (int i=0; i<1; i++) trace1() << "constructClusters " << loop_count;
		if (loop_count == -1) {
			for (int i=0; i<1; i++) trace1() << "loop_count -1";
			Wt_start = Wt;
			Wt = (Wt_end+Wt_start)/2;
			count = 0;
			continue;
		}
		for (int i=0; i<1; i++) trace1() << "buildTrajectories " << A.size();
		fl = false;
		maxLen = 0;

		buildTrajectories();
		for (int i=0; i<0; i++) trace1() << "done " << A.size();
		
		for (int i=0; i<10; i++) trace() << "calculatePathLength";
		for (int t=0; t<numUAVs; t++) {
			// for (int i=0; i<10; i++) trace() << t;
			double len = calculatePathLength(trajectories[t]);
			if (len > L_max) {
				// trace() << "calculatePathLength " << calculatePathLength(trajectories[t]);
				fl = true;
				// break;
			};
			if (len > maxLen) maxLen = len;
		}

		ratio = pow(maxLen / L_max, 2);


		if (fl) {
			for (int i=0; i<10; i++) trace() << "increase Wmin";
			// break;
			
			count += 1;

			for (int i=0; i<1; i++) trace1() << "reduce num. clusters, max length = " << maxLen << ", ratio = " << ratio;
			if (count >= 5) {
				Wt_start = Wt;
				count = 0;
			}
			else continue;
		} else {
			// trace1() << "save config";
			if (A.size() > config.A.size() && loop_count <= 50){
				for (int i=0; i<1; i++) trace1() << "save config";
				config.save(A, cent, next, trajectories);
				A_old_saved = A_old;
				epsilon_saved = epsilon0;
				isSaved = true;
				Wt_end = Wt;
			}
			else {
				count += 1;

				if (count >= 5) {
					if (!isSaved) {
						for (int i=0; i<1; i++) trace1() << "save config";
						config.save(A, cent, next, trajectories);
						A_old_saved = A_old;
						epsilon_saved = epsilon0;
						isSaved = true;
					}
					Wt_end = Wt;
					count = 0;
				}
				else continue;
			}
			fringeSet_0 = fringeSet;
			innerSet_0 = innerSet;
			A2_0 = A2;
			Wt_end = Wt;
			for (int i=0; i<1; i++) trace1() << "increase num. clusters, max length = " << maxLen << ", ratio = " << ratio;
			count = 0;
			// if (Wt == Wt_opt) {
			// 	break;
			// }
		}

		trace() << "Alter k old value " << Wt << " new value " << (Wt*ratio) << " ratio " << ratio;
		Wt *= ratio;
		if (Wt >= Wt_end) break;

		if ((Wt_end-Wt_start>1000) && (Wt_end+Wt_start>1000)){
			
		} 
		
		if ((Wt_end-Wt_start<=10000) && !isSaved) {
			// Wt_max = 4*w_total/numUAVs;
			// Wt_min = 0;
			Wt_end = 4*w_total/numUAVs;
			Wt_start = 1000;
			Wt_opt = -1;
			countSuccess = 0;
		}

	} while ((Wt_end-Wt_start>10000) && (Wt_end+Wt_start>1000));
	if (Wt_opt == -1) Wt_opt = Wt;
	countSuccess++;

	
	A = config.A;
	stringstream ss1;
	for(int l : A) {
		ss1 << l << " ";
	}
	for (int i=0; i<10; i++) trace1() << Wt << " " << ss1.str();

	isLandmark = vector<bool>(N, false);
	for (int l : A) isLandmark[l] = true;
	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (distance(i, sinkId) <= L_max/2)
			w_max[i] = Wt_saved;//*(E_i-E_min)/(E_max-E_min);
		else
			w_max[i] = weights[i];
	}
	clearData();
	growBallsKonstant(A);
	for (auto pair : ballWeight) {
		trace1() << "cluster " << pair.first << " weight " << pair.second << " threshold " << w_max[pair.first];
	}

	CastaliaModule::trace2() << roundNumber << "\tWTHR\t" << Wt_saved;
	CastaliaModule::trace2() << roundNumber << "\tEPS\t" << epsilon_saved;


	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;

		stringstream ss;
		for(int l : T) {
			ss << l << " ";
		}
		trace1() << ss.str();
	}
	trace1() << "maxLength " << maxLength;
	CastaliaModule::trace2() << roundNumber << "\tMAXLENGTH\t" << maxLength;
	maxLengthRatio = maxLength / L_max;
	for (int i=0; i<0; i++) trace1() << "END mainAlg";

	for (int i : A_old_saved) {
		auto P = GlobalLocationService::getLocation(i);
		CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << P.x() << "\t" << P.y() << "\t" << weights[i] << "\t" << cent[i] << "\t" << i;
	}

}

void GPRouting::mainAlg3() {

	for (int u=0; u<numNodes; u++) E_tmp[u] = getResMgrModule(u)->getRemainingEnergy();
	config.A.clear();

	for (int i=0; i<10; i++) trace1() << "mainAlg";
	E_min = DBL_MAX;
	E_max = 0;
	for (int i=0; i<numNodes; i++) {
		if (i == self) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (E_i > E_max) E_max = E_i;
		if (E_i < E_min) E_min = E_i;
	}
	w_total = 0;
	for (int i=0; i<numNodes; i++) w_total += weights[i];
	double p_end = 100;
	double p_start = 0;

	
	double p = (p_end+p_start)/2;

	bool fl;
	bool isSaved = false;
	int count = 0;
	double ratio = 1;
	double maxLen = 0;
	vector<int> A_old_saved;
	do {
		reset();
		clearData();
		
		for (int i=0; i<1; i++) trace1() << "p " << p << " p_start " << p_start << " p_end " << p_end;
		for (int i=0; i<1; i++) trace1() << "constructClusters";
		int loop_count = constructClusters3SF(p);
		for (int i=0; i<1; i++) trace1() << "constructClusters " << loop_count;
		for (int i=0; i<1; i++) trace1() << "buildTrajectories " << A.size();
		fl = false;
		maxLen = 0;

		if (A == config.A) {
			trajectories = config.trajectories;
		}
		else buildTrajectories(true);
		for (int i=0; i<0; i++) trace1() << "done " << A.size();
		
		for (int i=0; i<10; i++) trace() << "calculatePathLength";
		for (int t=0; t<numUAVs; t++) {
			double len = calculatePathLength(trajectories[t]);
			if (len > L_max) {
				fl = true;
			};
			if (len > maxLen) maxLen = len;
		}

		ratio = pow(maxLen / L_max, 2);


		if (fl) {
			count += 1;
			for (int i=0; i<1; i++) trace1() << "reduce num. clusters, max length = " << maxLen << ", ratio = " << ratio;
			if (count >= 1) {
				p_start = p;
				count = 0;

				p *= ratio;
				if (p >= p_end) break;
				continue;
			}
			else continue;
		} else {
			if (A.size() > config.A.size() && loop_count <= 50){
				for (int i=0; i<1; i++) trace1() << "save config";
				config.save(A, cent, next, trajectories);
				TSP_tour_saved = TSP_tour;
				new_trajectories_saved = new_trajectories;
				A_old_saved = A_old;
				epsilon_saved = epsilon0;
				isSaved = true;
				p_end = p;
			}
			else {
				count += 1;
				if (count >= 1) {
					if (!isSaved) {
						for (int i=0; i<1; i++) trace1() << "save config";
						config.save(A, cent, next, trajectories);
						TSP_tour_saved = TSP_tour;
						new_trajectories_saved = new_trajectories;
						A_old_saved = A_old;
						epsilon_saved = epsilon0;
						isSaved = true;
					}
					p_end = p;
					count = 0;
				}
				else continue;
			}
			fringeSet_0 = fringeSet;
			innerSet_0 = innerSet;
			A2_0 = A2;
			p_end = p;
			for (int i=0; i<1; i++) trace1() << "increase num. clusters, max length = " << maxLen << ", ratio = " << ratio;
			count = 0;
		}

		// p *= ratio;
		p = (p_end + p_start) / 2;
		if (p >= p_end) break;

	} while (p_end-p_start>0.002);
	countSuccess++;

	double maxLength = 0;
	for (auto T : config.trajectories) {
		double length = calculatePathLength(T);
		if (length > maxLength) maxLength = length;

		stringstream ss;
		for(int l : T) {
			ss << l << " ";
		}
		trace1() << ss.str();
	}
	trace1() << "maxLength " << maxLength;
	CastaliaModule::trace2() << roundNumber << "\tMAXLENGTH\t" << maxLength;
	maxLengthRatio = maxLength / L_max;
	for (int i=0; i<0; i++) trace1() << "END mainAlg";

	for (int i : A_old_saved) {
		auto P = GlobalLocationService::getLocation(i);
		CastaliaModule::trace2() << roundNumber << "\tPOINT\tblue\t" << P.x() << "\t" << P.y() << "\t" << weights[i] << "\t" << cent[i] << "\t" << i;
	}

	// for (int i=0; i<TSP_tour_saved.size(); i++){
	// 	auto P = GlobalLocationService::getLocation(TSP_tour_saved[i]);
	// 	auto Q = GlobalLocationService::getLocation(TSP_tour_saved[(i+1)%TSP_tour_saved.size()]);
	// 	CastaliaModule::trace2() << roundNumber << "\tLINE\tred\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
	// }
	for (int k=0; k<new_trajectories_saved.size(); k++) {
		for (int i=0; i<new_trajectories_saved[k].size()-1; i++){
			auto P = GlobalLocationService::getLocation(new_trajectories_saved[k][i]);
			auto Q = GlobalLocationService::getLocation(new_trajectories_saved[k][i+1]);
			CastaliaModule::trace2() << roundNumber << "\tLINE\tred\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
		}
	}

}


void GPRouting::GPinit() {
	for (int i=0; i<10; i++) trace() << "GPinit";
	
	N = numNodes;
	graph.init(numNodes, self, d0);
	trajectories.resize(numUAVs);

	dLandmark.resize(N, 0.);
	cent.resize(N, -1);
	next.resize(N, -1);
	centList.resize(N);
	isLandmark.resize(N, false);
	representSet.resize(N);
	w_max.resize(N);
	E_0 = vector<double>(numNodes, 0);
	E_tmp = vector<double>(numNodes, 0);
}

void GPRouting::reset() {
	
	trajectories = vector<vector<int>>(numUAVs);

	A.clear();
	dLandmark = vector<double>(N, 0.);
	cent = vector<int>(N, -1);
	next = vector<int>(N, -1);
	centList = vector<list<int>>(N);
	isLandmark = vector<bool>(N, false);	
	representSet = vector<list<int>>(N);
	w_max = vector<double>(N, 0);
	branchId = vector<int>(N, -1);
	branchWeight = vector<double>(N, 0);
}

vector<int> GPRouting::TZ_sample(vector<int>W, double s) {
	// for (int i=0; i<5; i++) trace() << "TZ_sample";
	int nodeW = W.size();
	if (nodeW > 0) {
		if (nodeW < s) {
			for (int node : W) {
				isLandmark[node] = true;
			}
			return W;
		} else {
			double prob_net = s / nodeW;
			vector<int> new_W;
			for (int node : W) {
				if (isLandmark[node]) continue;
				double random_float = uniform(0,1);
				if (random_float < prob_net) {
					new_W.push_back(node);
					isLandmark[node] = true;
				}
			}
			if (new_W.empty()) return TZ_sample(W, s);
			else {
				return new_W;
			}
		}
	}
	return W;

}

void GPRouting::growBalls(vector<int> landmarkSet){
	// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "growBalls Asize " << A.size();
	for (int u : landmarkSet) {
		dLandmark[u] = 0;
		representSet[u].clear();
		cent[u] = u;
	}

	dCompare =  &dLandmark;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : landmarkSet) {
		queue.push(l);
	}
	unordered_set<int> removedSet;

	while (!queue.empty()) {
		// // for (int i=0; i<5; i++) trace() << "queue " << queue.size() << " removedSet " << removedSet.size();
		int u = queue.top();
		queue.pop();
		for (int i=0; i<0; i++) trace() << "u " << u << " d[u] " << dLandmark[u];
		// auto tmp_q = queue; //copy the original queue to the temporary queue
		// while (!tmp_q.empty()) {
		// 	int v = tmp_q.top();
		// 	tmp_q.pop();
		// 	trace() << "v " << v << " d[v] " << dLandmark[v];
		// }
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		// if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "for";
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = dLandmark[u] + graph.getLength(u,v);
			
			// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "if";
			if (alt - dLandmark[v] < -EPSILON){
				dLandmark[v] = alt;
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
			}
			// else if ((abs(alt - dLandmark[v]) < EPSILON) && E_tmp[next[v]] < E_tmp[u]){// && (calculateCHEnergy(cent[u], computeWeight(representSet[cent[u]])) > calculateCHEnergy(cent[v], computeWeight(representSet[cent[v]])))) {
			// 	// representSet[cent[v]].remove(v);
			// 	cent[v] = cent[u];
			// 	next[v] = u;
			// 	// representSet[cent[v]].push_back(v);
			// }
		}
	}
	int totalSize = 0;
	// vector<bool> check (N, false);
	// for (int l : landmarkSet) {
	// 	totalSize += representSet[l].size();
	// 	trace() << l << " " << representSet[l].size();
	// 	for (int i : representSet[l]) {
	// 		if (!check[i]) check[i] = true;
	// 		else trace() << "violate " << i;
	// 	}
	// }
	// for (int jj=0; jj<10; jj++) trace() << "total ball size " << totalSize;
	for (int u=0; u<numNodes; u++) {
		if (u ==sinkId) continue;
		if (cent[u] == -1) {
			stringstream ss;
			for(int l : A) {
				// ss << l << " ";
				trace() << l << " " << ballWeight[l];
			}
			trace1() << "growBall ERROR " << ss.str();
			for (int v=0; v<numNodes; v++) {
				trace1() << v << " " << cent[v];
			}
		}
	}
	computeBallWeight();
}


void GPRouting::growBallsPercent(vector<int> landmarkSet, double percent){
	// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "growBalls Asize " << A.size();
	for (int u : landmarkSet) {
		dLandmark[u] = 0;
		representSet[u].clear();
		cent[u] = u;
	}

	dCompare =  &dLandmark;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : landmarkSet) {
		queue.push(l);
	}
	unordered_set<int> removedSet;
	vector<double> T(N, 0);
	vector<int> level1_node(N, -1);

	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		for (int i=0; i<0; i++) trace() << "u " << u << " d[u] " << dLandmark[u];
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		// double T_new = T[cent[u]] + weights[u];
		// if (rxEnergy(T_new) + txEnergy(T_new,D2UAV) >= E_tmp[cent[u]] * percent) continue;
		// T[cent[u]] = T_new;
		
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = dLandmark[u] + graph.getLength(u,v);
			double T_new = T[cent[u]] + weights[v];
			double T_level1 = 0;
			if (level1_node[u] != -1) T_level1 = T[level1_node[u]] + weights[v];
			
			// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "if";
			if (alt - dLandmark[v] < -EPSILON && rxEnergy(T_new) + txEnergy(T_new,D2UAV) <= E_tmp[cent[u]] * percent){
				// if (level1_node[u] != -1 && rxEnergy(T_level1) + txEnergy(T_level1,distance(u, next[u])) > E_tmp[level1_node[u]] * percent) continue;
				T[cent[u]] = T_new;
				if (cent[v] != -1) T[cent[v]] -= weights[v];
				dLandmark[v] = alt;
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
				// if (level1_node[u] != -1) T[level1_node[u]] = T_level1;
				// if (level1_node[v] != -1) T[level1_node[v]] -= weights[v];
				// level1_node[v] = level1_node[u];
				// if (next[v] == cent[v]) {
				// 	level1_node[v] = v;
				// }
			}
			// else if ((abs(alt - dLandmark[v]) < EPSILON) && E_tmp[next[v]] < E_tmp[u]){// && (calculateCHEnergy(cent[u], computeWeight(representSet[cent[u]])) > calculateCHEnergy(cent[v], computeWeight(representSet[cent[v]])))) {
			// 	// representSet[cent[v]].remove(v);
			// 	cent[v] = cent[u];
			// 	next[v] = u;
			// 	// representSet[cent[v]].push_back(v);
			// }
		}
	}
	/* int totalSize = 0;
	
	for (int u=0; u<numNodes; u++) {
		if (u ==sinkId) continue;
		if (cent[u] == -1) {
			stringstream ss;
			for(int l : A) {
				// ss << l << " ";
				trace() << l << " " << ballWeight[l];
			}
			trace1() << "growBall ERROR " << ss.str();
			for (int v=0; v<numNodes; v++) {
				trace1() << v << " " << cent[v];
			}
			break;
		}
	}
	computeBallWeight(); */
}

/* void GPRouting::growBallsKonstant(vector<int> landmarkSet){
	// for (int i=0; i<20; i++) trace() << "growBalls Asize " << A.size();
	vector<double> W_thres(numNodes, 0);
	for (int i=0; i<numNodes; i++) {
		W_thres[i] = w_max[i];
	}

	vector<double> vals(numNodes, 0);
	vector<double> d(numNodes, DBL_MAX);
	for (int u : landmarkSet) {
		d[u] = 0;
		vals[u] = -DBL_MAX;
		representSet[u].clear();
		cent[u] = u;
		next[u] = -1;
	}

	unordered_set<int> removedSet;

	auto start = std::chrono::high_resolution_clock::now();
	auto finish = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
	for (int ii=0; ii<1; ii++) trace() << "TIME FOR_GROW_BALL " << microseconds.count() << " us";

	dCompare =  &dLandmark;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : landmarkSet) {
		queue.push(l);
	}
	
	for (int ii=0; ii<1; ii++) trace() << "growBalls.whileLoop";
	removedSet.clear();
	int countLoop = 0;
	start = std::chrono::high_resolution_clock::now();
	while (!queue.empty()) {
		int u = queue.top();
		if (countLoop++ > 1600000) trace() << u;
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		if (removedSet.size() == numNodes-1) break;
		if (next[u] >= 0) d[u] = d[next[u]] + graph.getLength(u, next[u]);
		int u_tmp = u;
		double min_thres = DBL_MAX;
		while (next[u_tmp] >= 0) {
			W_thres[next[u_tmp]] -= weights[u];
			if (W_thres[u_tmp] < min_thres) min_thres = W_thres[u_tmp];
			u_tmp = next[u_tmp];
		}
		if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			if (weights[v] > min_thres) continue;
			double alt = dLandmark[u] + graph.getLength(u,v);
			
			// trace() << "v" << v << " val " << vals[v] << " alt " << alt;
			if (alt - dLandmark[v] < -EPSILON){
				vals[v] = - E_tmp[u] / (d[u] + graph.getLength(u,v)) / (d[u] + graph.getLength(u,v));
				dLandmark[v] = alt;
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
			}
		}
	}

	dCompare =  &vals;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue2(Comparebydistance);
	for (int i=0; i<numNodes; i++) {
		if (i == sinkId) continue;
		if (cent[i] != -1) queue2.push(i);
	}
	
	for (int ii=0; ii<1; ii++) trace() << "growBalls.whileLoop";
	removedSet.clear();
	int countLoop = 0;
	start = std::chrono::high_resolution_clock::now();
	while (!queue2.empty()) {
		int u = queue.top();
		if (countLoop++ > 1600000) trace() << u;
		queue2.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		if (removedSet.size() == numNodes-1) break;
		// trace() << "u" << u << " next " << next[u];
		if (next[u] >= 0) d[u] = d[next[u]] + graph.getLength(u, next[u]);
		int u_tmp = u;
		
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			if (cent[v] != -1) continue;
			double alt = - E_tmp[u] / (d[u] + graph.getLength(u,v)) / (d[u] + graph.getLength(u,v));
			
			if (alt - vals[v] < -EPSILON){
				vals[v] = alt;
				dLandmark[v] = dLandmark[u] + graph.getLength(u,v);
				cent[v] = cent[u];
				next[v] = u;
				queue2.push(v);
			}
		}
	}
	finish = std::chrono::high_resolution_clock::now();
	microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
	for (int ii=0; ii<1; ii++) trace() << "TIME WHILE_GROW_BALL " << microseconds.count() << " us";

	for (int u=0; u<numNodes; u++) {
		if (u ==sinkId) continue;
		if (cent[u] == -1) {
			stringstream ss;
			for(int l : A) {
				// ss << l << " ";
				trace() << l << " " << ballWeight[l];
			}
			trace1() << "growBall ERROR " << ss.str();
			for (int v=0; v<numNodes; v++) {
				trace1() << v << " " << cent[v];
			}
		}
	}
	computeBallWeight();
} */


void GPRouting::growBallsKonstant(vector<int> landmarkSet){
	// for (int i=0; i<20; i++) trace() << "growBalls Asize " << A.size();
	// vector<double> E_thres(numNodes, 0);
	// for (int i=0; i<numNodes; i++) {
	// 	E_thres[i] = E_tmp[i] * percent / 100;
	// }

	vector<double> vals(numNodes, 0);
	vector<double> d(numNodes, DBL_MAX);
	for (int u : landmarkSet) {
		d[u] = 0;
		vals[u] = -DBL_MAX;
		representSet[u].clear();
		cent[u] = u;
		next[u] = -1;
	}

	unordered_set<int> removedSet;
	// removedSet.insert(landmarkSet.begin(), landmarkSet.end());

	auto start = std::chrono::high_resolution_clock::now();
	// for (int v=0; v<numNodes; v++){
	// 	if (removedSet.find(v) != removedSet.end()) continue;
	// 	for (int u : landmarkSet){
	// 		double val = - getResMgrModule(u)->getRemainingEnergy() / (d[u] + distance(u,v)) / (d[u] + distance(u,v));
	// 		if (val < vals[v]) {
	// 			vals[v] = val;
	// 			cent[v] = u;
	// 			next[v] = u;
	// 		}
	// 	}
	// }
	// for (int l : landmarkSet) {
	// 	for (int v : graph.getAdjExceptSink(l)) {
	// 		if (isLandmark[v]) continue;
	// 		double val = - getResMgrModule(l)->getRemainingEnergy() / (d[l] + distance(l,v)) / (d[l] + distance(l,v));
	// 		if (val < vals[v]) {
	// 			vals[v] = val;
	// 			cent[v] = l;
	// 			next[v] = l;
	// 		}
	// 	}
	// }
	auto finish = std::chrono::high_resolution_clock::now();
	auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
	for (int ii=0; ii<1; ii++) trace() << "TIME FOR_GROW_BALL " << microseconds.count() << " us";

	dCompare =  &vals;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : landmarkSet) {
		queue.push(l);
	}
	
	for (int ii=0; ii<1; ii++) trace() << "growBalls.whileLoop";
	removedSet.clear();
	int countLoop = 0;
	start = std::chrono::high_resolution_clock::now();
	while (!queue.empty()) {
		int u = queue.top();
		if (countLoop++ > 1600000) trace() << u;
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		if (removedSet.size() == numNodes-1) break;
		// trace() << "u" << u << " next " << next[u];
		if (next[u] >= 0) d[u] = d[next[u]] + graph.getLength(u, next[u]);
		int u_tmp = u;
		double min_thres = DBL_MAX;
		// while (next[u_tmp] >= 0) {
		// 	E_thres[next[u_tmp]] -= rxEnergy(weights[u]);
		// 	E_thres[u_tmp] -= txEnergy(weights[u], distance(u_tmp, next[u_tmp]));
		// 	if (E_thres[u_tmp] < min_thres) min_thres = E_thres[u_tmp];
		// 	u_tmp = next[u_tmp];
		// }
		// if (next[u_tmp] == -1) {
		// 	E_thres[u_tmp] -= txEnergy(weights[u], D2UAV);
		// 	if (E_thres[u_tmp] < min_thres) min_thres = E_thres[u_tmp];
		// }
		// if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			// if (rxEnergy(weights[v]) > min_thres) continue;
			double alt = - E_tmp[u] / (d[u] + graph.getLength(u,v)) / (d[u] + graph.getLength(u,v));
			
			// trace() << "v" << v << " val " << vals[v] << " alt " << alt;
			if (alt - vals[v] < -EPSILON/100){
				vals[v] = alt;
				dLandmark[v] = dLandmark[u] + graph.getLength(u,v);
				cent[v] = cent[u];
				next[v] = u;
				int b_id = next[v];
				while (b_id != -1) {
					branchWeight[b_id] += weights[v];
					b_id = next[b_id];
				}
				queue.push(v);
			}
		}
	}
	finish = std::chrono::high_resolution_clock::now();
	microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish-start);
	for (int ii=0; ii<1; ii++) trace() << "TIME WHILE_GROW_BALL " << microseconds.count() << " us";

	for (int u=0; u<numNodes; u++) {
		if (u ==sinkId) continue;
		if (cent[u] == -1) {
			stringstream ss;
			for(int l : A) {
				// ss << l << " ";
				trace() << l << " " << ballWeight[l];
			}
			trace1() << "growBall ERROR " << ss.str();
			for (int v=0; v<numNodes; v++) {
				trace1() << v << " " << cent[v] << " " << vals[v];
			}
		}
	}
	computeBallWeight();
}

void GPRouting::growBalls2(vector<int> landmarkSet){
	// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "growBalls Asize " << A.size();
	for (int u : landmarkSet) {
		dLandmark[u] = 0;
		representSet[u].clear();
		cent[u] = u;
	}

	dCompare =  &dLandmark;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	for (int l : landmarkSet) {
		queue.push(l);
	}
	unordered_set<int> removedSet;

	while (!queue.empty()) {
		// // for (int i=0; i<5; i++) trace() << "queue " << queue.size() << " removedSet " << removedSet.size();
		int u = queue.top();
		queue.pop();
		// // for (int i=0; i<5; i++) trace() << "u " << u << " d[u] " << dLandmark[u];
		// auto tmp_q = queue; //copy the original queue to the temporary queue
		// while (!tmp_q.empty()) {
		// 	int v = tmp_q.top();
		// 	tmp_q.pop();
		// 	trace() << "v " << v << " d[v] " << dLandmark[v];
		// }
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		// if (!isLandmark[u]) representSet[cent[u]].push_back(u);
		// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "for";
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = dLandmark[u] + graph.getLength(u,v);
			
			// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "if";
			if (alt - dLandmark[v] < -EPSILON && computeWeight(representSet[cent[u]]) + weights[v] > w_max[cent[u]]) {
				trace1() << "v " << v << " cent[u] " << cent[u] << " weight " << (computeWeight(representSet[cent[u]]) + weights[v]) << " threshold " << w_max[cent[u]];
				stringstream ss;
				for(int i : representSet[cent[u]]) {
					ss << i << " ";
				}
				trace1() << "representSet " << ss.str();
			}
			// if (alt - dLandmark[v] < -EPSILON && computeWeight(representSet[cent[u]]) + weights[v] <= w_max[cent[u]]){
			// 	dLandmark[v] = alt;
			// 	cent[v] = cent[u];
			// 	next[v] = u;
			// 	queue.push(v);
			// }
			// else if ((abs(alt - dLandmark[v]) < EPSILON) && (calculateCHEnergy(cent[u], computeWeight(representSet[cent[u]])) > calculateCHEnergy(cent[v], computeWeight(representSet[cent[v]]))) && computeWeight(representSet[cent[u]]) + weights[v] <= w_max[cent[u]]) {
			// 	// representSet[cent[v]].remove(v);
			// 	cent[v] = cent[u];
			// 	next[v] = u;
			// 	// representSet[cent[v]].push_back(v);
			// }
			if  (calculateCHEnergy(cent[u], computeWeight(representSet[cent[u]])) > calculateCHEnergy(cent[v], computeWeight(representSet[cent[v]]))) {
				cent[v] = cent[u];
				next[v] = u;
				queue.push(v);
			}
		}
	}
	int totalSize = 0;
	// vector<bool> check (N, false);
	// for (int l : landmarkSet) {
	// 	totalSize += representSet[l].size();
	// 	trace() << l << " " << representSet[l].size();
	// 	for (int i : representSet[l]) {
	// 		if (!check[i]) check[i] = true;
	// 		else trace() << "violate " << i;
	// 	}
	// }
	// for (int jj=0; jj<10; jj++) trace() << "total ball size " << totalSize;
	for (int u=0; u<numNodes; u++) {
		if (u ==sinkId) continue;
		if (cent[u] == -1) {
			stringstream ss;
			for(int l : A) {
				// ss << l << " ";
				trace() << l << " " << ballWeight[l];
			}
			trace1() << "growBall ERROR " << ss.str();
			for (int v=0; v<numNodes; v++) {
				trace1() << v << " " << cent[v];
			}
		}
	}
	computeBallWeight();
}

double GPRouting::computeWeight(list<int> S) {
	double weight = 0;
	for (int u : S) weight += weights[u];
	return weight;
}

void GPRouting::updateCentList() {
	for (int u=0; u<numNodes; u++) {
		if ((u == self)) continue;
		double minDist = DBL_MAX;
		for (int l : A) {
			if (!graph.hasEdge(l, u) || (l == cent[u])) continue;
			double dist = graph.getLength(l, u);
			if (dist < minDist) {
				minDist = dist;
				centList.clear();
				centList[u].push_back(l);
			}
		}
		centList[u].push_back(cent[u]);
	}
}

double GPRouting::computeClusterWeight(int uNode){
	// for (int i=0; i<5; i++) trace() << "computeClusterWeight";
	vector<double> d(N, DBL_MAX);
	d[uNode] = 0;
	dCompare =  &d;
	priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
	queue.push(uNode);
	unordered_set<int> removedSet;
	while (!queue.empty()) {
		int u = queue.top();
		queue.pop();
		if (removedSet.find(u) != removedSet.end()) continue;
		removedSet.insert(u);
		for (int v : graph.getAdjExceptSink(u)) {
			if ((removedSet.find(v) != removedSet.end())) continue;
			double alt = d[u] + graph.getLength(u,v);
			if (alt < d[v]){
				d[v] = alt;
				if (d[v]<dLandmark[v]) queue.push(v);
			}
		}
	}
	double totalWeight = 0;
	for (int u : removedSet){
		totalWeight += weights[u];
	}
	return totalWeight;
}

vector<int> GPRouting::verifyFringeSet(){
	for (int i=0; i<1; i++) trace() << "verifyFringeSet A.size " << A.size();
	int count = 1;
	fringeSet.clear();
	innerSet.clear();
	vector<int> returnlist;
	vector<double> maxBranchWeight(w_max);
	for (int landmark : A){
		for (int i=0; i<1; i++) trace() << "landmark " << landmark;
		double weight = ballWeight.find(landmark)->second;
		if (weight > w_max[landmark]) {
		// if (weight > Wt) {
			weight = 0;
			// for (int i=0; i<5; i++) trace() << "landmark " << landmark << " ballWeight " << weight << " w_max " << w_max[landmark];
			dCompare =  &dLandmark;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
			for (int u : representSet[landmark]) queue.push(u);
			while (weight < w_max[landmark]){
			// while (weight < Wt){
				int u = queue.top();
				queue.pop();
				if (debugRecruitProcess) for (int i=0; i<1; i++) trace1() << "landmark " << landmark << " inner " << u << " dLandmark " << dLandmark[u] << " weight " << weight;
				innerSet.push_back(u);
				weight += weights[u];
				// int b_id = next[u];
				// bool isPushed = false;
				// while (b_id != -1 && !isPushed) {
				// 	maxBranchWeight[b_id] -= weights[u];
				// 	if (maxBranchWeight[b_id] < 0) {
				// 		fringeSet.push_back(u);
				// 		isPushed = true;
				// 	}
				// }
				// for (int i=0; i<5; i++) trace() << "ballWeight " << weight;
			}
			while (!queue.empty()) {
				int u = queue.top();
				if (debugRecruitProcess) for (int i=0; i<1; i++) trace1() << "landmark " << landmark << " fringe " << u << " dLandmark " << dLandmark[u];
				queue.pop();
				fringeSet.push_back(u);
			}
		}
	}
	return returnlist.empty() ? fringeSet : returnlist;
}

vector<int> GPRouting::TZ_sample2(vector<int> W, double b) {
	for (int i=0; i<0; i++) trace1() << "TZ_sample2";
	int maxTrial = 3;
	vector<int> new_W;
	while (!W.empty()){
		double Etotal = 0;
		for (int u : W) {
			Etotal += getResMgrModule(u)->getRemainingEnergy();
		}
		// int i = rand() % W.size();
		// int nextLandmark = W[i];

		double f = (double)rand() / RAND_MAX * Etotal;
		int nextLandmark = 0;
		for (int u : W) {
			f -= getResMgrModule(u)->getRemainingEnergy();
			if (f <= 0) {
				nextLandmark = u;
				break;
			}
		}
		if (isLandmark[nextLandmark]) {
			W.erase(std::remove(W.begin(), W.end(), nextLandmark), W.end());
			continue;
		}

		vector<int> intersectBall;
		double newBallSize = 0;
		double intersectBallSize = 0;

		vector<double> d(N, DBL_MAX);
		d[nextLandmark] = 0;
		dCompare =  &d;
		priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
		queue.push(nextLandmark);
		unordered_set<int> removedSet;
		while (!queue.empty()){
			int v = queue.top();
			queue.pop();
			if (removedSet.find(v) != removedSet.end()) continue;
			removedSet.insert(v);
			newBallSize += weights[v];
			if (newBallSize > w_max[nextLandmark]) break;
			if (std::find(W.begin(), W.end(), v) != W.end()){
				W.erase(std::remove(W.begin(), W.end(), v), W.end());
				intersectBall.push_back(v);
				intersectBallSize += weights[v];
			}
			for (int w : graph.getAdjExceptSink(v)){
				if ((w < N) && (removedSet.find(w) == removedSet.end())){
					double alt = d[v] + graph.getLength(v,w);
					if (alt < d[w]){
						d[w] = alt;
						queue.push(w);
					}
				}
			}
		}
		if ((maxTrial == 0) || (intersectBallSize>w_min)){//new_W.empty() &&
			new_W.push_back(nextLandmark);
			isLandmark[nextLandmark] = true;
			maxTrial = 3;
		}
		else{
			for (int v : intersectBall) W.push_back(v);
			if (maxTrial>0){
				maxTrial--;
				continue;
			}
			else break;
		}
	}
	//StdOut.printf("Adding... %d\n", new_W.size());
	return new_W;
}

void GPRouting::computeBallWeight(){
	// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "computeBallWeight";
	maxBallWeight = 0;
	minBallWeight = INT64_MAX;
	// totalWeights = 0;
	for (int u : graph.getNodesExceptSink()){
		representSet[cent[u]].push_back(u);
	}

	double tempBallWeight[A.size()];
	for (int i=0; i<A.size(); i++) tempBallWeight[i] = 0;
	int size = 0;
	// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "for 1";
	for (int i=0; i<A.size(); i++){
		size += representSet[A[i]].size();
		for (int u : representSet[A[i]]) {
			tempBallWeight[i] += weights[u];
			// trace() << "node " << u << " weight " << graph.getWeight(u);
		}
		// totalWeights += tempBallWeight[i];
		// if (tempBallWeight[i] > maxBallWeight) maxBallWeight = tempBallWeight[i];
		// if (tempBallWeight[i] < minBallWeight) minBallWeight = tempBallWeight[i];
	}
	ballWeight.clear();

	// if (roundNumber == 169) for (int i=0; i<10; i++) trace1() << "for 2";
	for (int i=0; i<A.size(); i++){
		ballWeight.insert(pair<int,double>(A[i], tempBallWeight[i]));
		// trace() << "computeBallWeight " << A[i] << " " << tempBallWeight[i];
	}
}

void GPRouting::recruitNewCHsAlpha(){
	for (int i=0; i<0; i++) trace1() << "recruitNewCHsAlpha";;
	vector<int> W;//verifyFringeSet(representSet, graph.getTotalWeights()/A.size());
	if (W.empty()){
		vector<int> fringeSet = verifyFringeSet();
		// for (int jj=0; jj<10; jj++) trace() << "fringeSet " << fringeSet.size();
		
		for (int w : fringeSet){
			auto P = GlobalLocationService::getLocation(w);
			if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tPOINT\tgreen\t" << P.x() << "\t" << P.y() << "\t" << weights[w] << "\t" << cent[w] << "\t" << w; 
			if (w_max[w] >= w_min && distance(w, sinkId) <= L_max/2) 
				W.push_back(w);
		};
		// for (int jj=0; jj<10; jj++) trace() << "W " << W.size();
	}
	A2 = TZ_sample2(W, 0);
	for (int jj=0; jj<10; jj++) trace() << "A2 " << A2.size();
	if (!A2.empty()) A.insert(A.end(), A2.begin(), A2.end());
	for (int w : A2){
		auto P = GlobalLocationService::getLocation(w);
		if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tPOINT\tred\t" << P.x() << "\t" << P.y() << "\t" << weights[w] << "\t" << cent[w] << "\t" << w;
	};
	// growBalls(A2);
	clearData();
	growBallsKonstant(A);
	// for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha OK";
}

void GPRouting::clearData(){
	for (int u : graph.getNodesExceptSink()){
		//for (int i=0; i<10; i++) trace1() << u;
		if (isLandmark[u]){
			dLandmark[u] = 0;
		}
		else {
			dLandmark[u] = DBL_MAX;
		}
		// for (int i=0; i<10; i++) trace1() << "cent";
		cent[u] = -1;
		// for (int i=0; i<10; i++) trace1() << "next";
		next[u] = -1;
		branchId[u] = -1;
		branchWeight[u] = 0;
		// for (int i=0; i<10; i++) trace1() << "centList";
		// centList[u].clear();
		// for (int i=0; i<10; i++) trace1() << "representSet";
		representSet[u].clear();
	}
}

int GPRouting::constructClusters(int k){
	// for (int i=0; i<10; i++) trace() << "constructClusters";
	E_min = DBL_MAX;
	E_max = 0;
	for (int i=0; i<numNodes; i++) {
		if (i == self || distance(i, sinkId) > L_max/2) continue;
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		if (E_i > E_max) E_max = E_i;
		if (E_i < E_min) E_min = E_i;
	}

	double E_2 = E_max - txEnergy(Wt, D2UAV) - rxEnergy(Wt);
	// if (E_2 < E_min) E_min = E_2;
	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		if (E_min == E_max) {
			w_max[i] = Wt;
			continue;
		}
		double E_i = getResMgrModule(i)->getRemainingEnergy();
		// trace() << "E_i " << i << " " << E_i;
		if (distance(i, sinkId) <= L_max/2)
			w_max[i] = Wt*(E_i-E_min)/(E_max-E_min);
		else
			w_max[i] = weights[i];
		// trace() << "w_max " << i << " " << w_max[i];
	}

	// double percent = Wt / (1 + epsilon) / w_total;
	// double E1 = rxEnergy(1) + txEnergy(1,D2UAV);
	// vector<bool> isSelected(N, true);
	// for (int i=0; i<N; i++) {
	// 	if (i == sinkId) continue;
	// 	for (int j : graph.getAdjExceptSink(i)) {
	// 		if (E_tmp[j] > E_tmp[i]) {
	// 			w_max[i] = 0;
	// 			isSelected[i] = false;
	// 			break;
	// 		}
	// 	}
	// 	if (isSelected[i])
	// 		w_max[i] = E_tmp[i] * percent / E1;
	// }

	int k_max;
	vector<double> w_max_ordered (w_max);
	sort(w_max_ordered.begin(), w_max_ordered.end(), greater<double>());
	double w_total_tmp = w_total;
	trace() << "w_total " << w_total;
	for (int i=0; i<N; i++) {
		trace() << "w_max_ordered " << i << " " << w_max_ordered[i];
		w_total_tmp -= w_max_ordered[i];
		if (w_total_tmp < 0) {
			k_max = i+1;
			break;
		}
		// trace() << "w_total_tmp " << w_total_tmp;
	}
	
	// while (w_total_tmp > 0) {
	// 	double w_total_tmp0 = w_total_tmp;
	// 	for (int i=0; i<N; i++) {
	// 		if (i == sinkId || isSelected[i]) continue;
	// 		for (int j : graph.getAdjExceptSink(i)) {
	// 			if (E_tmp[j] > E_tmp[i]) {
	// 				w_max[i] = 0;
	// 				isSelected[i] = false;
	// 				break;
	// 			}
	// 		}
	// 		if (isSelected[i]) {
	// 			w_max[i] = E_tmp[i] * percent / E1;
	// 			w_total_tmp -= w_max[i];
	// 			k_max++;
	// 		}
	// 	}
	// 	if (w_total_tmp0 == w_total_tmp) return -1;
	// }

	// int k_min = (int)(w_total/Wt);
	int k_min = k_max;
	if (k_min <= 0) k_min = 1;
	k_max = k_min*2;
	// // trace1() << "k_max " << k_max;
	w_min = w_total/k_max;
	// for (int i=0; i<10; i++) trace() << "w_min " << w_min;
	trace1() << "Wt " << Wt;
	trace1() << "k_max " << k_max;

	// k_max = k;//(int) ((1 + epsilon) * w_total / Wt);
	// Wt = (1 + epsilon) * w_total / k;

	for (int l : A) {
		if (w_max[l] < w_min || distance(l, sinkId) > L_max/2) {
			A.erase(std::remove(A.begin(), A.end(), l), A.end());
			isLandmark[l] = false;
		}
	}
	
	int count = 0;
	clearData();
	vector<int> satisfiedNodes;
	for (int i=0; i<N; i++) {
		if ((w_max[i] >= w_min) && distance(i, sinkId) <= L_max/2 && !isLandmark[i]) 
			satisfiedNodes.push_back(i);
	}
	while (A.empty()) {
		A = TZ_sample(satisfiedNodes, 8);
	}
	growBallsKonstant(A);
	bool fl;
	// for (int i=0; i<10; i++) trace() << "subloop";
	do {
		

		int Asize = A.size();
		recruitNewCHsAlpha();
		// for (int i=0; i<10; i++) trace() << "recruitNewCHsAlpha " << count << " before " << Asize << " after " << A.size();
		
		/*
		// for (int i=0; i<10; i++) trace() << "Trim CH set";
		if (A.size() > k_max) {
			vector<double> tempBallWeight(N, 0);
			for (auto pair : ballWeight) {
				tempBallWeight[pair.first] = pair.second;
			}
			dCompare =  &tempBallWeight;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> Q(Comparebydistance);
			for (int l : A) Q.push(l);
			while (A.size() > k_max){
				int u = Q.top();
				Q.pop();
				A.erase(std::remove(A.begin(), A.end(), u), A.end());
				isLandmark[u] = false;
			}
			// vector<int> Anew;
			// for (int l : A) {
			// 	if (tempBallWeight[l] <= Wt && tempBallWeight[l] >= w_min) Anew.push_back(l);
			// }
			// A = Anew;
		}
		clearData();
		growBalls(A);
		*/

		fl = false;
		double Wt_new = min(Wt, (1 + epsilon) * w_total / A.size());
		double maxWeight = 0;
		double totalWeight = 0;
		for (auto pair : ballWeight) {
			// for (int i=0; i<5; i++) trace() << "check " << pair.first << " ballWeight " << pair.second << " w_max " << w_max[pair.first];
			if (maxWeight < pair.second) {
				maxWeight = pair.second;
				// fl = true;
				// for (int i=0; i<5; i++) trace() << "violate " << pair.first << " ballWeight " << pair.second << " w_max " << w_max[pair.first];
				// break;
			}
			totalWeight += pair.second;
		}
		trace1() << "w_total " << w_total << " totalWeight " << totalWeight;
		if (maxWeight > Wt_new) fl = true;
		double epsilon_tmp = maxWeight/w_total*A.size();
		if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tEPS\t" << epsilon_tmp;
		if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tK\t" << A.size();
		if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tWTHR\t" << totalWeight/w_total;

		if (A.size() > k_max) {
		// if (fl) {
			vector<double> tempBallWeight(N, 0);
			for (auto pair : ballWeight) {
				tempBallWeight[pair.first] = pair.second;
			}
			dCompare =  &tempBallWeight;
			priority_queue<int,vector<int>, decltype(&Comparebydistance)> Q(Comparebydistance);
			for (int l : A) Q.push(l);
			while (A.size() > k_max){
				// int u = Q.top();
				// Q.pop();
				// A.erase(std::remove(A.begin(), A.end(), u), A.end());
				// isLandmark[u] = false;

				double WeightTotal = 0;
				for (int l : A) {
					WeightTotal += 1./tempBallWeight[l];
				}
				// int i = rand() % W.size();
				// int nextLandmark = W[i];

				double f = (double)rand() / RAND_MAX * WeightTotal;
				int removedLandmark;
				for (int l : A) {
					f -= 1./tempBallWeight[l];
					if (f <= 0) {
						removedLandmark = l;
						break;
					}
				}
				A.erase(std::remove(A.begin(), A.end(), removedLandmark), A.end());
				isLandmark[removedLandmark] = false;
			}
		}
		clearData();
		growBallsKonstant(A);

		// fl = false;
		// Wt_new = min(Wt, (1 + epsilon) * w_total / A.size());
		// maxWeight = 0;
		// totalWeight = 0;
		// for (auto pair : ballWeight) {
		// 	// for (int i=0; i<5; i++) trace() << "check " << pair.first << " ballWeight " << pair.second << " w_max " << w_max[pair.first];
		// 	if (maxWeight < pair.second) {
		// 		maxWeight = pair.second;
		// 		// fl = true;
		// 		// for (int i=0; i<5; i++) trace() << "violate " << pair.first << " ballWeight " << pair.second << " w_max " << w_max[pair.first];
		// 		// break;
		// 	}
		// }
		// if (maxWeight > Wt_new) fl = true;

		fl = false;
		Wt_new = min(Wt, (1 + epsilon) * w_total / A.size());
		maxWeight = 0;
		totalWeight = 0;
		for (auto pair : ballWeight) {
			if (w_max[pair.first] < pair.second) {
				maxWeight = pair.second;
				fl = true;
				break;
			}
		}

		for (int u=0; u<N; u++) {
			if (u == sinkId) continue;
			if (branchWeight[branchId[u]] > w_max[branchId[u]]) {
				fl = true;
				break;
			}
		}


		if (count++ > 50) {
			// clearData();
			// growBalls(A);
			break;
		}

		countAdjusment++;
		// if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tWTHR\t" << Wt;
		if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tKMAX\t" << k_max;
		for (int i=0; i<N; i++) {
			auto P = GlobalLocationService::getLocation(i);
			if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment << "\tPOINT\tblue\t" << P.x() << "\t" << P.y() << "\t" << weights[i] << "\t" << cent[i] << "\t" << i;
		}
		for (int i=0; i<N; i++) {
			if (cent[i] != -1) {
				auto P = GlobalLocationService::getLocation(i);
				auto Q = GlobalLocationService::getLocation(next[i]);
				if (debugRecruitProcess) CastaliaModule::trace2() << "RECRUIT" << countAdjusment  << "\tLINE\tgreen\t" << P.x() << "\t" << P.y() << "\t" << Q.x() << "\t" << Q.y();
			}
		}


		// if (count++ > 50) break;
		trace() << "subloop " << count;

		nloop = count;
	} while (fl);

	trace1() << "E_min " << E_min << " E_max " << E_max;
	trace1() << "l E[l] w_max[l] ballweight[l]";
	for (auto pair : ballWeight) {
		trace1() << pair.first << " " << E_tmp[pair.first] << " " << w_max[pair.first] << " " << pair.second;
	}

	// vector<int> A_new;
	// A_old = A;
	// for (int l : A) {
	// 	Point center(0, 0);
	// 	double clusterWeight = 0;
	// 	for (int u : representSet[l]) {
	// 		Point p_u = location(u);
	// 		center.x_ += p_u.x_ * weights[u];
	// 		center.y_ += p_u.y_ * weights[u];
	// 		clusterWeight += weights[u];
	// 	}
	// 	center.x_ /= clusterWeight;
	// 	center.y_ /= clusterWeight;
	// 	double R = G::distance(center, location(l));
	// 	int newLandmark = l;
	// 	for (int u : representSet[l]) {
	// 		if (G::distance(center, location(u)) <= R) {
	// 			if (E_tmp[u] < E_tmp[newLandmark]) newLandmark = u;
	// 		}
	// 	}
	// 	isLandmark[l] = false;
	// 	A_new.push_back(newLandmark);
	// 	isLandmark[newLandmark] = true;
	// }
	// A = A_new;
	// clearData();
	// growBallsKonstant(A);

	// if (count > 50) trace1() << "violate";
	// else trace1() << "success";
	double maxWeight = 0;
	for (auto pair : ballWeight) {
		if (pair.second > maxWeight) maxWeight = pair.second;
	}
	trace() << "epsilon " << maxWeight/w_total*A.size();
	epsilon0 = maxWeight/w_total*A.size();
	// updateCentList();
	cent[self] = -1;
	for (int l : A) cent[l] = -1;
	for (int i=0; i<1; i++) trace() << "A size " << A.size();
	stringstream ss;
	for(int l : A) {
		// ss << l << " ";
		trace1() << l << " " << ballWeight[l];
	}
	// trace() << ss.str();
	return count;
}

template <typename T>
vector<size_t> GPRouting::sort_indexes(const vector<T> &v) {

  // initialize original index locations
  vector<size_t> idx(v.size());
  iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  // using std::stable_sort instead of std::sort
  // to avoid unnecessary index re-orderings
  // when v contains elements of equal values 
  stable_sort(idx.begin(), idx.end(),
       [&v](size_t i1, size_t i2) {return v[i1] > v[i2];});

  return idx;
}

int GPRouting::constructClusters3(double p){
	
	double percent = p/100;
	double E1 = rxEnergy(1) + txEnergy(1,D2UAV);
	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		if (distance(i, sinkId) > L_max/2) w_max[i] = 0;
		else w_max[i] = E_tmp[i];// * percent / E1;
	}
	
	
	clearData();
	auto sorted_idx = sort_indexes(w_max);
	vector<bool> isSelected(N, false);
	isSelected[sinkId] = true;
	double totalWeight = 0;

	for (int new_CH : sorted_idx) {
		if (isSelected[new_CH]) continue;
		trace1() << "newCH " << new_CH << " ,E = " << E_tmp[new_CH] << " ,max_weight = " << w_max[new_CH];

		// Grow Ball
		A.push_back(new_CH);
		/* vector<double> d(N, DBL_MAX);
		vector<int> next_tmp(N, -1);
		vector<int> level1(N, -1);
		vector<double> T_level1(N, 0);
		d[new_CH] = 0;
		isLandmark[new_CH] = true;
		representSet[new_CH].clear();
		cent[new_CH] = new_CH;

		dCompare =  &d;
		priority_queue<int,vector<int>, decltype(&Comparebydistance)> queue(Comparebydistance);
		queue.push(new_CH);
		double T = 0;

		while (!queue.empty() && rxEnergy(T) + txEnergy(T,D2UAV) < E_tmp[new_CH] * percent) {
			int u = queue.top();
			queue.pop();
			for (int i=0; i<0; i++) trace() << "u " << u << " d[u] " << d[u];
			
			if (isSelected[u]) continue;
			isSelected[u] = true;
			dLandmark[u] = d[u];
			cent[u] = new_CH;
			next[u] = next_tmp[u];
			T += weights[u];
			if (next[u] == new_CH) level1[u] = u;
			// if (level1[u] != -1) {
			// 	T_level1[level1[u]] += weights[u];
			// 	if (rxEnergy(T_level1[level1[u]]) + txEnergy(T_level1[level1[u]],D2UAV) >= E_tmp[level1[u]]  * percent) break;
			// }

			// bool isBreak = false;
			// int u_tmp = u;
			// while (next[u_tmp] != -1) {
			// 	T_level1[next[u_tmp]] += weights[u_tmp];
			// 	if (rxEnergy(T_level1[next[u_tmp]]) + txEnergy(T_level1[next[u_tmp]],D2UAV) >= E_tmp[next[u_tmp]]  * percent) {
			// 		isBreak = true;
			// 		break;
			// 	}
			// 	u_tmp = next[u_tmp];
			// }
			// if (isBreak) break;

			for (int v : graph.getAdjExceptSink(u)) {
				if (isSelected[v]) continue;
				double alt = d[u] + graph.getLength(u,v);
				
				if (alt - d[v] < -EPSILON){
					d[v] = alt;
					next_tmp[v] = u;
					level1[v] = level1[u];
					queue.push(v);
				}
			}
		}

		trace1() << "clusterWeight = " << T;

		////////////////////////
		cent[new_CH] = -1;
		totalWeight += T; */
		clearData();
		growBallsPercent(A, percent);
		for (int i=0; i<N; i++) {
			if (i == sinkId) continue;
			if (cent[i] != -1) isSelected[i] = true;
		}
	}
	// clearData();
	// growBallsPercent(A, percent);
	// growBallsKonstant(A);
	cent[self] = -1;
	for (int l : A) cent[l] = -1;
	trace1() << "w_total " << w_total << " totalWeight " << totalWeight;
}


int GPRouting::constructClusters3SF(double p){
	
	double percent = p/100;
	double E1 = rxEnergy(1) + txEnergy(1,D2UAV);
	for (int i=0; i<N; i++) {
		if (i == sinkId) continue;
		if (distance(i, sinkId) > L_max/2) w_max[i] = 0;
		else w_max[i] = E_tmp[i];// * percent / E1;
	}
	
	clearData();
	
	double totalWeight = 0;

	vector<int> A_min;
	double min_cnsmptn = DBL_MAX;
	stringstream ss_csmpt;

	for (int retries=0; retries<50; retries++) {
		A.clear();
		unordered_set<int> uncovered_nodes;
		for (int u=0; u<numNodes; u++) {
			if (u != sinkId) uncovered_nodes.insert(u);
		}
		while (!uncovered_nodes.empty()) {
			// trace1() << "num_uncovered = " << uncovered_nodes.size();
			// stringstream ss_uncover;
			// for (int l : uncovered_nodes) ss_uncover << l << " ";
			// trace1() << ss_uncover.str();

			// uniform selection
			auto it = std::begin(uncovered_nodes);
			auto r = rand() % uncovered_nodes.size();
			std::advance(it,r);
			int new_CH = *it;

			// Roulette wheel selection
			/* int new_CH = *uncovered_nodes.begin();
			double Etotal = 0;
			for (auto itr = uncovered_nodes.begin(); itr != uncovered_nodes.end(); ++itr) {
				Etotal += E_tmp[*itr];
			}
			double f = (double)rand() / RAND_MAX * Etotal;
			for (auto itr = uncovered_nodes.begin(); itr != uncovered_nodes.end(); ++itr) {
				f -= E_tmp[*itr];
				if (f <= 0) {
					new_CH = *itr;
					break;
				}
			} */

			// trace1() << "newCH " << new_CH << " ,E = " << E_tmp[new_CH] << " ,max_weight = " << w_max[new_CH];

			// Grow Ball
			A.push_back(new_CH);
			
			clearData();
			growBallsPercent(A, percent);
			for (int i=0; i<N; i++) {
				if (i == sinkId) continue;
				if (cent[i] != -1) uncovered_nodes.erase(i);
			}
		}

		double rxSizes[numNodes];
		for (int u=0; u<numNodes; u++) { rxSizes[u]=0; }
		for (int u=0; u<numNodes; u++) {
			if (u == sinkId) continue;
			int tmp = u;
			while (next[tmp] != -1) {
				rxSizes[next[tmp]] += weights[u];
				tmp = next[tmp];
			}
		}
		double new_cnsmptn = 0;
		for (int i=0; i<numNodes; i++) {
			if (i == sinkId) continue;
			double rxSize = rxSizes[i];//
			
			new_cnsmptn += rxEnergy(rxSize);
			double txSize = rxSize + weights[i];
			if (cent[i] == i) {
				new_cnsmptn += txEnergy(txSize, D2UAV);
			} else {
				double d2next = G::distance(GlobalLocationService::getLocation(i), GlobalLocationService::getLocation(next[i]));
				new_cnsmptn += txEnergy(txSize, d2next);
			}
		}
		if (new_cnsmptn < min_cnsmptn) {
			min_cnsmptn = new_cnsmptn;
			A_min = A;
			ss_csmpt << min_cnsmptn << " ";
		}
	}

	trace1() << "min_cnsmptn " << ss_csmpt.str();
	A = A_min;
	clearData();
	stringstream ss_A;
	for (int l : A) ss_A << l << " ";
	trace1() << "A " << ss_A.str();
	growBallsPercent(A, percent);;
	
	cent[self] = -1;
	for (int l : A) cent[l] = -1;
	trace1() << "w_total " << w_total << " totalWeight " << totalWeight;
}

void GPRouting::buildTrajectories(){
	buildTrajectories(false);
}

vector<int> GPRouting::TSP(vector<int> AA) {
	vector<int> TSP_tour;
	vector<Point> sites;
	for (int l : AA) {
		Point p = location(l);
		p.id_ = l;
		sites.push_back(p);
	}
	// Point p0 = location(sinkId);
	// p0.id_ = sinkId;
	// sites.push_back(p0);
	while (!sites.empty()) {
		vector<Point> convexHull = G::convexHull(sites);
		if (convexHull.empty()) {
			convexHull = sites;
		}
		stringstream ss;
		for (Point p : convexHull) {
			sites.erase(std::remove(sites.begin(), sites.end(), p), sites.end());
			ss << p.id_ << " ";
		}
		// trace1() << ss.str();
		if (TSP_tour.empty()) {
			for (Point p : convexHull) {
				TSP_tour.push_back(p.id_);
			}
		}
		else {
			while (!convexHull.empty()) {
				double minIncreased = DBL_MAX;
				int i_min = -1;
				int pos = -1;
				for (int i=0; i<convexHull.size(); i++) {
					Point p = convexHull[i];
					for (int j=0; j<TSP_tour.size(); j++) {
						Point A = location(TSP_tour[j]);
						Point B = location(TSP_tour[(j+1)%TSP_tour.size()]);
						double lengthIncreased = G::distance(A, p) + G::distance(p, B) - G::distance(A, B);
						if (lengthIncreased < minIncreased) {
							minIncreased = lengthIncreased;
							i_min = i;
							pos = (j+1)%TSP_tour.size();
						}
					}
				}

				Point insertedNode = convexHull[i_min];
				convexHull.erase(convexHull.begin() + i_min);
				TSP_tour.insert(TSP_tour.begin() + pos, insertedNode.id_);
			}
		}
	}
	return TSP_tour;
}

void GPRouting::buildTrajectories(bool isBreak){
	vector<int> A_tmp = A;
	// A_tmp.push_back(sinkId);
	TSP_tour = TSP(A_tmp);
	// TSP_tour.erase(std::remove(TSP_tour.begin(), TSP_tour.end(), sinkId), TSP_tour.end());

	trace1() << "TSPLength = " << calculatePathLength(TSP_tour);
	stringstream ss_tsp;
	for (int l : TSP_tour) {
		ss_tsp << l << " ";
	}
	trace1() << "TSP tour: " << ss_tsp.str();
	double minMaxLength = DBL_MAX;
	for (int offset=0; offset<TSP_tour.size(); offset++) {
		double L_max_last = L_max;
		double maxLengthLast;
		double maxLength = DBL_MAX;
		vector<vector<int>> tmp_trajectories;
		do {
			maxLengthLast = maxLength;
			maxLength = 0;
			tmp_trajectories = vector<vector<int>>(numUAVs);
			for (auto &trajectory : tmp_trajectories) trajectory.push_back(sinkId);
			int cur_trajectory_id = 0;
			double curLength = 0;
			double totalLength = 0;
			for (int i=0; i<TSP_tour.size(); i++) {
				int node = TSP_tour[(i+offset)%TSP_tour.size()];
				vector<int> &cur_trajectory = tmp_trajectories[cur_trajectory_id];
				int lastNode = cur_trajectory[cur_trajectory.size()-1];
				double curLengthLast = curLength;
				curLength += distance(lastNode, node) + distance(node, sinkId) - distance(lastNode, sinkId);
				if (curLength > L_max_last) {
					if (cur_trajectory_id < numUAVs-1) {
						cur_trajectory.push_back(sinkId);
						// stringstream ss_tra;
						// for (int u : cur_trajectory) ss_tra << u << " ";
						// trace1() << "Tour " << cur_trajectory_id+1 << ": " << ss_tra.str() << " - Length = " << curLengthLast;
						totalLength += curLengthLast;
						cur_trajectory_id++;
						vector<int> &next_trajectory = tmp_trajectories[cur_trajectory_id];
						next_trajectory.push_back(node);
						curLength = distance(sinkId, node)*2;
						if (curLength > maxLength) maxLength = curLength;
					}
					else {
						cur_trajectory.push_back(node);
						if (curLength > maxLength) maxLength = curLength;
					}
				}
				else {
					cur_trajectory.push_back(node);
					if (curLength > maxLength) maxLength = curLength;
				}
			}
			tmp_trajectories[cur_trajectory_id].push_back(sinkId);
			totalLength += curLength;
			// stringstream ss_tra;
			// for (int u : tmp_trajectories[cur_trajectory_id]) ss_tra << u << " ";
			// trace1() << "Tour " << cur_trajectory_id+1 << ": " << ss_tra.str() << " - Length = " << curLength;
			// trace1() << "L_max = " << L_max_last << " maxLength = " << maxLength;
			L_max_last = totalLength / numUAVs;
			if (maxLength < minMaxLength) {
				minMaxLength = maxLength;
				new_trajectories = tmp_trajectories;
			}
		} while (maxLength < maxLengthLast);
		// trace1() << "curLength = " << curLength;
		// int j = 1;
		// for (auto trajectory : tmp_trajectories) {
		// 	stringstream ss_tra;
		// 	for (int u : trajectory) ss_tra << u << " ";
		// 	trace1() << "Tour " << j << ": " << ss_tra.str();
		// }
		// if (maxLengthLast < minMaxLength) {
		// 	minMaxLength = maxLengthLast;
		// 	new_trajectories = tmp_trajectories;
		// }
	}
	// for (auto& trajectory : new_trajectories) {
	// 	vector<int> new_trajectory;
	// 	for (int i=0; i<trajectory.size()-1; i++) {
	// 		new_trajectory.push_back(trajectory[i]);
	// 	}
	// 	new_trajectory = TSP(new_trajectory);
	// 	stringstream ss_tsp;
	// 	for (int l : new_trajectory) {
	// 		ss_tsp << l << " ";
	// 	}
	// 	trace1() << ss_tsp.str();
	// 	int offset = 0;
	// 	for (int i=0; i<new_trajectory.size(); i++) {
	// 		if (new_trajectory[i] == sinkId) {
	// 			offset = i;
	// 			break;
	// 		}
	// 	}
	// 	trace1() << "offset = " << offset;
	// 	trajectory.clear();
	// 	for (int i=0; i<new_trajectory.size(); i++) {
	// 		trajectory.push_back(new_trajectory[(i+offset)%new_trajectory.size()]);
	// 	}
	// 	trajectory.push_back(sinkId);
	// }

	trace1() << "minMaxLength = " << minMaxLength;
	if (minMaxLength < L_max) {
		trajectories = new_trajectories;
		return;
	}
	int jj = 1;
	for (auto trajectory : new_trajectories) {
		stringstream ss_tra;
		for (int u : trajectory) ss_tra << u << " ";
		trace1() << "Tour " << jj << ": " << ss_tra.str() << " - Length: " << calculatePathLength(trajectory);
		jj++;
	}
	vector<vector<int>> tours(numUAVs);
	vector<int> A_new;
	int matchingId = 1;
	for (int i=0; i<numUAVs; i++) {
		for (int u : new_trajectories[i]) {
			if (u != sinkId) {
				A_new.push_back(u);
				tours[i].push_back(matchingId);
				matchingId++;
			}
		}
	}
	A = A_new;
	// growBallsKonstant(A);


	for (int i=0; i<numUAVs; i++) trajectories[i].clear();
	if (A.size() < numUAVs) {
		for (int i=0; i<A.size(); i++) {
			trajectories[i].push_back(self);
			trajectories[i].push_back(A[i]);
			trajectories[i].push_back(self);
		}
	}
	else {
		for (int i=0; i<10; i++) trace() << "buildTrajectories";
		// double maxLength = 0;
		// vector<int> CHvector = sortedCHVector();
		// for (int k=0; k<numUAVs; k++) {
		// 	vector<int> zone_k;
		// 	int start = CHvector.size()/numUAVs*k;
		// 	int end = CHvector.size()/numUAVs*(k+1);
		// 	for (int i=start; i<end; i++) {
		// 		zone_k.push_back(CHvector[i]);
		// 	}

		// 	// trajectories[k] = mainTSP(zone_k);
		// 	vector<int> trajectory_k;
		// 	trajectory_k.push_back(self);
		// 	for (int i=0; i<10; i++) trace() << "zone " << k;
		// 	stringstream ss;
		// 	for(int i : zone_k) {
		// 		ss << i << " ";
		// 	}
		// 	for (int i=0; i<10; i++) trace() << ss.str();
		// 	vector<int> CH_tr = mainTSP(zone_k);
		// 	for (int i=0; i<10; i++) trace() << "mainTSP";
		// 	trajectory_k.insert(trajectory_k.end(), CH_tr.begin(), CH_tr.end());
		// 	trajectory_k.push_back(self);
		// 	trajectories[k] = trajectory_k;
		// 	double length = calculatePathLength(trajectory_k);
		// 	for (int i=0; i<10; i++) trace() << "zone " << k << " size " << zone_k.size() << " length " << length;
		// 	if (length > maxLength) maxLength = length;
		// }

		auto start = high_resolution_clock::now(); 
		trajectories = Vrptw::call(*this, sinkId, A, numUAVs, L_max, isBreak, tours);
		auto stop = high_resolution_clock::now(); 
		auto duration = duration_cast<microseconds>(stop - start);
		
		trace() << "Check Vrptw " << A.size() << " " << duration.count();
		// trajectories = callTspga(*this, sinkId, A, numUAVs, L_max);
		while (trajectories.size() < numUAVs) {
			trajectories.push_back(vector<int>());
		}
	}

	double maxLength = 0;
	unordered_set<int> landmarks;
	landmarks.insert(A.begin(), A.end());
	for (int k=0; k<numUAVs; k++) {
		double length = calculatePathLength(trajectories[k]);
		if (length > maxLength) maxLength = length;
		trace() << "trajectory " << k << " length " << length;
		stringstream ss;
		for(int i : trajectories[k]) {
			ss << i << " ";
			landmarks.erase(i);
		}
		trace() << ss.str();
	}
	trace1() << "maxLength = " << maxLength;
	jj = 1;
	for (auto trajectory : trajectories) {
		stringstream ss_tra;
		for (int u : trajectory) ss_tra << u << " ";
		trace1() << "Tour " << jj << ": " << ss_tra.str() << " - Length: " << calculatePathLength(trajectory);;
		jj++;
	}
	for (int i : landmarks) trace() << i << " is unvisited!!!";

	for (int k=0; k<trajectories.size(); k++) {
		if (trajectories[k].empty()) {
			trajectories[k].push_back(sinkId);
			trajectories[k].push_back(sinkId);
		}
	}
};

vector<int> GPRouting::sortedCHVector() {
	vector<int> CHvector;
    if (A.empty()) {
        return CHvector;
    }
    CHvector.push_back(A[0]);

    unsigned int i = 1;
    for (i = 1; i < A.size(); i++) {
        Angle angle = G::angle(selfLocation, location(CHvector[0]), selfLocation, location(A[i]));

        unsigned int j = 0;
        for (j = 0; j + 1 < CHvector.size(); j++) {
            if (G::angle(selfLocation, location(CHvector[0]), selfLocation, location(CHvector[j + 1])) > angle) {
                CHvector.insert(CHvector.begin() + j + 1, A[i]);
                break;
            }
        }

        if (j == CHvector.size() - 1) {
            CHvector.push_back(A[i]);
        }
    }
	return CHvector;
}