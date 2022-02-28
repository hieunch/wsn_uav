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

cplusplus {{
#include "RoutingPacket_m.h"
#include "GeoMathHelper.h"
typedef std::vector<int> IntVector;
}}

class noncobject Point;
class noncobject IntVector;

class RoutingPacket;

enum GAMBACPacket_Type
{
	GAMBAC_ROUTING_CONTROL_PACKET = 1;			
	GAMBAC_ROUTING_DATA_PACKET = 2;
	GAMBAC_ROUTING_JOIN_PACKET = 3;
	GAMBAC_ROUTING_ADV_PACKET = 4;
};

packet GAMBACPacket extends RoutingPacket
{	
	int GAMBACPacketKind enum (GAMBACPacket_Type); 
	int schedule[];	
	double remainingEnergy;
	IntVector isCH;
	IntVector nextCH;
	IntVector trace;
};
