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

#ifndef _GAMBACFPercent_H_
#define _GAMBACFPercent_H_

#include "GAMBAC.h"

using namespace std;

class GAMBACFPercent : public GAMBAC {

protected:
	void timerFiredCallback(int);
	void mainAlg();
	int selectCHsAndBuildDFT(double W);
	void growBallsPercent(vector<int> CHSet, double percent);
	template <typename T>
	vector<size_t> sort_indexes(const vector<T> &v);
};

#endif			
