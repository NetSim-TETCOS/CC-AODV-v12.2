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
#include "IEEE802_11.h"
#include "IEEE802_11_Phy.h"

static char virtualInterfaceName[MAX_AC_CATEGORY][50] = { "IEEE802_11E_AC_BK","IEEE802_11E_AC_BE" ,"IEEE802_11E_AC_VI" ,"IEEE802_11E_AC_VO" };

static UINT get_ac_from_qos(QUALITY_OF_SERVICE qos)
{
	switch (qos)
	{
		case QOS_UGS:
		case QOS_rtPS:
			return 3;
		case QOS_nrtPS:
		case QOS_ertPS:
			return 2;
		case QOS_BE:
			return 1;
		default:
			return 1;
	}
}

static NETSIM_ID find_virtual_mac_interface(NETSIM_ID d, NETSIM_ID in, UINT ac)
{
	PIEEE802_11_MAC_VAR pstruMacVar = IEEE802_11_MAC(d, in);
	if (pstruMacVar->mediumAccessProtocol == DCF)	return in;

	return fn_NetSim_Stack_FindVirtualInterface(d, virtualInterfaceName[ac]);
}

int fn_NetSim_IEEE802_EDCA_Init(NETSIM_ID nDeviceId, NETSIM_ID nInterfaceId)
{
	if (isVirtualInterface(nDeviceId, nInterfaceId)) return -1;
	
	PIEEE802_11_MAC_VAR pstruMacVar = IEEE802_11_MAC(nDeviceId, nInterfaceId);
	PIEEE802_11_PHY_VAR phy = IEEE802_11_PHY(nDeviceId, nInterfaceId);

	pstruMacVar->parentInterfaceId = nInterfaceId;

	if (!pstruMacVar->nNumberOfAggregatedPackets)
		pstruMacVar->nNumberOfAggregatedPackets = 1;

	UINT i;
	for(i=0;i<MAX_AC_CATEGORY;i++)
		pstruMacVar->dot11EdcaTable[i].qosList.dMaximumSize = DEVICE_ACCESSBUFFER(nDeviceId, nInterfaceId)->dMaxBufferSize * 1024 * 1024;

	if (pstruMacVar->mediumAccessProtocol == DCF)
	{
		pstruMacVar->currEdcaTable = &pstruMacVar->dot11EdcaTable[0];
		pstruMacVar->currEdcaTable->AIFSN = 0;
		pstruMacVar->currEdcaTable->CWmax = phy->plmeCharacteristics.aCWmax;
		pstruMacVar->currEdcaTable->CWmin = phy->plmeCharacteristics.aCWmin;
		pstruMacVar->currEdcaTable->MSDULifeTime = 0;
		pstruMacVar->currEdcaTable->TXOPLimit = 0;
		return 1;
	}

	print_ieee802_11_log("EDCA is enable for device %d:%d\n", nDeviceId, nInterfaceId);

	for (i = 1; i < MAX_AC_CATEGORY; i++)
	{
		NETSIM_ID vin = fn_NetSim_Stack_CreateVirtualInterface(nDeviceId, nInterfaceId,
															   virtualInterfaceName[i],
															   false, true, false);
		PIEEE802_11_MAC_VAR pstruVMacVar = (PIEEE802_11_MAC_VAR)calloc(1, sizeof * pstruMacVar);
		memcpy(pstruVMacVar, pstruMacVar, sizeof * pstruVMacVar);
		SET_IEEE802_11_MAC(nDeviceId, vin, pstruVMacVar);
		pstruVMacVar->aceessCategory = i;
		pstruVMacVar->parentInterfaceId = nInterfaceId;
		pstruVMacVar->interfaceId = vin;

		print_ieee802_11_log("Creating Virtual Mac for access category %s. Virtual interface id is %d\n",
							 virtualInterfaceName[i], vin);

		pstruVMacVar->currEdcaTable = &pstruVMacVar->dot11EdcaTable[i];
	}
	return 0;
}

NETSIM_ID fn_NetSim_IEEE802_11e_AddtoQueue(NETSIM_ID d, NETSIM_ID in, NetSim_PACKET* packet)
{
	UINT ac = get_ac_from_qos(packet->nQOS);
	NETSIM_ID vin = find_virtual_mac_interface(d, in, ac);

	double siz = fnGetPacketSize(packet) * 8.0;
	packet->dEventTime = ldEventTime;

	PIEEE802_11_MAC_VAR mac = IEEE802_11_MAC(d, vin);
	if (mac->currEdcaTable->qosList.dMaximumSize != 0.0 &&
		mac->currEdcaTable->qosList.dCurrentSize + siz > mac->currEdcaTable->qosList.dMaximumSize)
	{
		fn_NetSim_Packet_FreePacket(packet);
		return 0;
	}

	if (mac->currEdcaTable->qosList.head)
	{
		mac->currEdcaTable->qosList.tail->pstruNextPacket = packet;
		mac->currEdcaTable->qosList.tail = packet;
	}
	else
	{
		mac->currEdcaTable->qosList.head = packet;
		mac->currEdcaTable->qosList.tail = packet;
	}

	mac->currEdcaTable->qosList.dCurrentSize += siz;

	return vin;
}

static NetSim_PACKET* get_packet_from_qoslist(QOS_LIST* list)
{
	NetSim_PACKET* p = NULL;
	if (list->head == NULL) return NULL;

	p = list->head;
	list->head = list->head->pstruNextPacket;
	p->pstruNextPacket = NULL;
	if (list->head == NULL) list->tail = NULL;

	double siz = fnGetPacketSize(p) * 8.0;
	list->dCurrentSize -= siz;
	return p;
}

NetSim_PACKET* fn_NetSim_IEEE802_11e_GetPacketFromQueue(NETSIM_ID d, NETSIM_ID in,
														UINT nPacketRequire, UINT* nPacketCount)
{
	PIEEE802_11_MAC_VAR mac = IEEE802_11_MAC(d, in);
	if (mac->currEdcaTable->qosList.head == NULL)
	{
		*nPacketCount = 0;
		return NULL;
	}

	NetSim_PACKET* p = NULL;
	NetSim_PACKET* pl = NULL;
	UINT i;

	for (i = 0; i < nPacketRequire; i++)
	{
		NetSim_PACKET* t = get_packet_from_qoslist(&mac->currEdcaTable->qosList);
		if (t == NULL) break;

		double lifeTime = t->dEventTime + mac->currEdcaTable->MSDULifeTime * TU;
		if (mac->mediumAccessProtocol == EDCAF && lifeTime < ldEventTime)
		{
			fn_NetSim_Packet_FreePacket(t);
			i--;
			continue;
		}

		if (p)
		{
			pl->pstruNextPacket = t;
			pl = t;
		}
		else
		{
			p = t;
			pl = t;
		}
		*nPacketCount += 1;
	}

	return p;
}

bool fn_NetSim_IEEE802_11e_IsPacketInQueue(NETSIM_ID d, NETSIM_ID in)
{
	PIEEE802_11_MAC_VAR mac = IEEE802_11_MAC(d, in);
	if (!mac->currEdcaTable) return false;
	return mac->currEdcaTable->qosList.head ? true : false;
}

void ieee802_11_edcaf_set_txop_time(PIEEE802_11_MAC_VAR mac, double currTime)
{
	if (!isEDCAF(mac)) return;

	if (mac->txopTime > 0) return;
	mac->txopTime = mac->currEdcaTable->TXOPLimit + currTime;
}

void ieee802_11_edcaf_unset_txop_time(PIEEE802_11_MAC_VAR mac)
{
	if (!isEDCAF(mac)) return;

	mac->txopTime = 0;
}

bool ieee802_11_edcaf_is_txop_timer_set(PIEEE802_11_MAC_VAR mac, double currTime)
{
	if (!isEDCAF(mac)) return false;

	PIEEE802_11_PHY_VAR phy = IEEE802_11_PHY(mac->deviceId, mac->parentInterfaceId);
	return mac->txopTime - currTime > getSIFSTime(phy);
}
