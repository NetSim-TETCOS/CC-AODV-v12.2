/************************************************************************************
* Copyright (C) 2020																*
* TETCOS, Bangalore. India															*
*																					*
* Tetcos owns the intellectual property rights in the Product and its content.		*
* The copying, redistribution, reselling or publication of any or all of the		*
* Product or its content without express prior written consent of Tetcos is			*
* prohibited. Ownership and / or any other right relating to the software and all	*
* intellectual property rights therein shall remain at all times with Tetcos.		*
*																					*
* This source code is licensed per the NetSim license agreement.					*
*																					*
* No portion of this source code may be used as the basis for a derivative work,	*
* or used, for any purpose other than its intended use per the NetSim license		*
* agreement.																		*
*																					*
* This source code and the algorithms contained within it are confidential trade	*
* secrets of TETCOS and may not be used as the basis for any other software,		*
* hardware, product or service.														*
*																					*
* Author:    Shashi Kant Suman	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#include "main.h"
#include "FastEmulation.h"
#include "EmulationPacket.h"
#include "ApplicationMap.h"

static bool isError(double ber, UINT packetSize)
{
	PACKET_STATUS s = fn_NetSim_Packet_DecideError(ber, (long double)packetSize);
	if (s == PacketStatus_Error) return true;
	else return false;
}

void errormodel_check_packet_for_error(ptrREALPACKET p, ptrAPPMAP map)
{
	UINT i;
	for (i = 0; i < p->realPacketCount; i++)
		p->isPacketErroed[i] = isError(map->ber, p->eachRealPacketLen[i]);
}
