#pragma once
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
#ifndef _NETSIM_SATELLITE_BUFFER_H_
#define _NETSIM_SATELLITE_BUFFER_H_

#ifdef  __cplusplus
extern "C" {
#endif
typedef struct stru_satellite_buffer
{
	NETSIM_ID utId;
	NETSIM_ID utIf;

	NETSIM_ID gwId;
	NETSIM_ID gwIf;

	double maxSizeInBytes;
	double maxUnitSizeInBytes;

	double sizeInBytes;
	UINT count;

	NetSim_PACKET* head;
	NetSim_PACKET* tail;

	double rank;

	UINT slotReqd;

	UINT allocatedSlotCount;
}SATELLITE_BUFFER, * ptrSATELLITE_BUFFER;
ptrSATELLITE_BUFFER satellite_buffer_init(NETSIM_ID utId, NETSIM_ID utIf,
										  NETSIM_ID gwId, NETSIM_ID gwIf,
										  double sizeInBytes, double maxUnitSizeInBytes);
void satellite_buffer_setMaxUnitSizeInBytes(ptrSATELLITE_BUFFER buffer, double maxUnitSizeInBytes);
bool satellite_buffer_add_packet(ptrSATELLITE_BUFFER buffer, NetSim_PACKET* packet);
NetSim_PACKET* satellite_buffer_remove_packet(ptrSATELLITE_BUFFER buffer);
NetSim_PACKET* satellite_buffer_head_packet(ptrSATELLITE_BUFFER buffer);

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_SATELLITE_BUFFER_H_ */
