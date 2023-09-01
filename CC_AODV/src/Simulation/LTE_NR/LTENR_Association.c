/************************************************************************************
* Copyright (C) 2019                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/

#include "stdafx.h"

#pragma region ASSOCIATION_INIT
void fn_NetSim_LTENR_InitAssociation()
{
	for (NETSIM_ID d = 0; d < NETWORK->nDeviceCount; d++)
	{
		for (NETSIM_ID i = 0; i < DEVICE(d + 1)->nNumOfInterface; i++)
		{
			if (!isLTE_NRInterface(d + 1, i + 1))
				continue;

			ptrLTENR_PROTODATA data = LTENR_PROTODATA_GET(d + 1, i + 1);
			
			if (!data || data->deviceType != LTENR_DEVICETYPE_GNB)
				continue;

			NetSim_LINKS* link = DEVICE_PHYLAYER(d + 1, i + 1)->pstruNetSimLinks;
			
			for (NETSIM_ID c = 0; c < link->puniDevList.pstrup2MP.nConnectedDeviceCount-1; c++)
			{
				LTENR_ASSOCIATEINFO_ADD(d + 1,
										i + 1,
										link->puniDevList.pstrup2MP.anDevIds[c],
										link->puniDevList.pstrup2MP.anDevInterfaceIds[c]);
			}
		}
	}
}

#pragma endregion

#pragma region ASSOCIATION_CALLBACK
#define MAX_ASSOCIATION_CALLBACK 100
typedef void(*fnAssociationCallback)(NETSIM_ID, NETSIM_ID,
									 NETSIM_ID, NETSIM_ID,
									 bool);
static fnAssociationCallback fnpAssociationCallback[MAX_ASSOCIATION_CALLBACK];
static UINT associationCallbackCount = 0;

void fn_NetSim_LTENR_RegisterCallBackForAssociation(void(*fnCallBack)(NETSIM_ID gnbId,
																	  NETSIM_ID gnbIf,
																	  NETSIM_ID ueId,
																	  NETSIM_ID ueIf,
																	  bool isAssociated))
{
	fnpAssociationCallback[associationCallbackCount] = fnCallBack;
	associationCallbackCount++;
}

static void LTENR_ASSOCIATION_CALL_CALLBACK(NETSIM_ID gnbId,
											NETSIM_ID gnbIf,
											NETSIM_ID ueId,
											NETSIM_ID ueIf,
											bool isAssociated)
{
	UINT i;
	for (i = 0; i < associationCallbackCount; i++)
		fnpAssociationCallback[i](gnbId, gnbIf, ueId, ueIf, isAssociated);
}

#pragma endregion
