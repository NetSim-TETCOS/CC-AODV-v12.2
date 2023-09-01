/************************************************************************************
* Copyright (C) 2019																*
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

#include "stdafx.h"
#include "LTENR_RLC.h"

ptrLTENR_RLCBUFFER LTENR_RLCBuffer_AddPacket(ptrLTENR_RLCBUFFER* buffer, NetSim_PACKET* packet,
											 UINT sn, UINT16 so, LTENR_LOGICALCHANNEL channel)
{
	ptrLTENR_RLCBUFFER b = LTENR_RLCBUFFER_ALLOC();
	b->packet = packet;
	b->SN = sn;
	b->channel = channel;
	b->SO = so;
	LTENR_RLCBUFFER_ADD(buffer, b);
	return b;
}

void LTENR_RLCBuffer_RemovePacket(ptrLTENR_RLCBUFFER* buffer, ptrLTENR_RLCBUFFER mem)
{
	LIST_FREE(buffer, mem);
}

void LTENR_RLCBuffer_FreePacket(ptrLTENR_RLCBUFFER* buffer, ptrLTENR_RLCBUFFER mem)
{
	fn_NetSim_Packet_FreePacket(mem->packet);
	LIST_FREE(buffer, mem);
}

void LTENR_RLCBuffer_UpdateBuffer(ptrLTENR_RLCBUFFER* buffer)
{
	ptrLTENR_RLCBUFFER b = *buffer;
	while (b)
	{
		if (b->isMarkedForRemoval)
		{
			LIST_FREE(buffer, b);
			b = *buffer;
			continue;
		}
		LTENR_RLCBUFFER_NEXT(b);
	}
}

ptrLTENR_RLCBUFFER LTENR_RLCBUFFER_FindAndRemoveAllBySN(ptrLTENR_RLCBUFFER* buffer, UINT sn)
{
	ptrLTENR_RLCBUFFER buf = *buffer;
	ptrLTENR_RLCBUFFER ret = NULL;
	while (buf)
	{
		if (buf->SN == sn)
		{
			LTENR_RLCBUFFER_REMOVE(buffer, buf);
			LTENR_RLCBUFFER_ADD(&ret, buf);
			buf = *buffer;
			continue;
		}

		LTENR_RLCBUFFER_NEXT(buf);
	}
	return ret;
}

ptrLTENR_RLCBUFFER LTENR_RLCBUFFER_FindAndRemoveBySNSO(ptrLTENR_RLCBUFFER* buffer, UINT sn, UINT16 so)
{
	ptrLTENR_RLCBUFFER buf = *buffer;
	while (buf)
	{
		if (buf->SN == sn && buf->SO == so)
		{
			LTENR_RLCBUFFER_REMOVE(buffer, buf);
			return buf;
		}
		LTENR_RLCBUFFER_NEXT(buf);
	}
	return NULL;
}
