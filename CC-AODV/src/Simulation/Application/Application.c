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
#include "Application.h"
#include "CoAP.h"

void encryptBlock(uint8_t* data, uint32_t* len, uint32_t* key);

/** 
This function is used to initialize the parameter for all the application based on 
the traffic type 
*/
_declspec(dllexport) int fn_NetSim_Application_Init(struct stru_NetSim_Network *NETWORK_Formal,
													NetSim_EVENTDETAILS *pstruEventDetails_Formal,
													char *pszAppPath_Formal,
													char *pszWritePath_Formal,
													int nVersion_Type,
													void **fnPointer)
{
	return fn_NetSim_Application_Init_F(NETWORK_Formal,
		pstruEventDetails_Formal,
		pszAppPath_Formal,
		pszWritePath_Formal,
		nVersion_Type,
		fnPointer);
}

/** This function is used to configure the applications like application id, source count, source id, destination count etc. */
_declspec(dllexport) int fn_NetSim_Application_Configure(void** var)
{
	return fn_NetSim_Application_Configure_F(var);
}

/**	
This function is called by NetworkStack.dll, whenever the event gets triggered	
inside the NetworkStack.dll for applications. This is the main function for the application.
It processes APPLICATION_OUT, APPLICATION_IN and TIMER events.
 */
_declspec (dllexport) int fn_NetSim_Application_Run()
{
	switch(pstruEventDetails->nEventType)
	{
	case APPLICATION_OUT_EVENT:
		handle_app_out();
		break;
	case APPLICATION_IN_EVENT:
		{
			NetSim_PACKET* pstruPacket=pstruEventDetails->pPacket;
			if(pstruPacket->nPacketType != PacketType_Control && pstruPacket->pstruAppData->nApplicationId && 
				pstruPacket->nControlDataType/100 != PROTOCOL_APPLICATION)
			{
				ptrAPPLICATION_INFO pstruappinfo;
				fnValidatePacket(pstruPacket);
				pstruappinfo=applicationInfo[pstruPacket->pstruAppData->nApplicationId-1];
				pstruPacket->pstruAppData->dEndTime = pstruEventDetails->dEventTime;
				fn_NetSim_Application_Plot(pstruPacket);
				appmetrics_dest_add(pstruappinfo, pstruPacket, pstruEventDetails->nDeviceId);
				if(pstruappinfo->nAppType==TRAFFIC_PEER_TO_PEER && pstruPacket->pstruAppData->nAppEndFlag==1)
				{
					fn_NetSim_Application_P2P_MarkReceivedPacket(pstruappinfo,pstruPacket);
					fn_NetSim_Application_P2P_SendNextPiece(pstruappinfo,get_first_dest_from_packet(pstruPacket),pstruEventDetails->dEventTime);
				}
				if(pstruappinfo->nAppType == TRAFFIC_EMULATION && pstruPacket->szPayload)
				{
					fn_NetSim_Dispatch_to_emulator(pstruPacket);
				}
				if (pstruappinfo->nAppType == TRAFFIC_BSM_APP)
				{
					process_saej2735_packet(pstruPacket);
				}
				//Delete the packet
				fn_NetSim_Packet_FreePacket(pstruPacket);
			}
			// Here which type is placed is only getting processed next one is not getting processed
			else if (pstruPacket->nControlDataType == packet_COAP_REQUEST)
			{
				ptrAPPLICATION_INFO pstruappinfo;
				fnValidatePacket(pstruPacket);
				pstruappinfo = applicationInfo[pstruPacket->pstruAppData->nApplicationId - 1];
				pstruPacket->pstruAppData->dEndTime = pstruEventDetails->dEventTime;
				//On receiving COAP Packet 
				appmetrics_dest_add(pstruappinfo, pstruPacket, pstruEventDetails->nDeviceId);
				fn_NetSim_Application_Plot(pstruPacket);
				fn_NetSim_Application_COAP_AppIn(pstruappinfo, pstruPacket);
				//Delete the packet
				fn_NetSim_Packet_FreePacket(pstruPacket);

			}
			else if(pstruPacket->nControlDataType == packet_HTTP_REQUEST)
			{
				ptrAPPLICATION_INFO pstruappinfo;
				fnValidatePacket(pstruPacket);
				pstruappinfo=applicationInfo[pstruPacket->pstruAppData->nApplicationId-1];
				pstruPacket->pstruAppData->dEndTime = pstruEventDetails->dEventTime;
				appmetrics_dest_add(pstruappinfo, pstruPacket, pstruEventDetails->nDeviceId);
				fn_NetSim_Application_HTTP_ProcessRequest(pstruappinfo,pstruPacket);
			}
		}
		break;
	case TIMER_EVENT:
		{
			switch(pstruEventDetails->nSubEventType)
			{
			case event_APP_START:
				//fn_NetSim_Call_LowerLayer(); //Not implemented now
				switch(pstruEventDetails->pPacket->pstruAppData->nAppType)
				{
				case TRAFFIC_DATABASE:
				case TRAFFIC_FTP:
				case TRAFFIC_CUSTOM:
				case TRAFFIC_DATA:
				case TRAFFIC_VOICE:
				case TRAFFIC_VIDEO:
				case TRAFFIC_HTTP:
				case TRAFFIC_EMAIL:
				case TRAFFIC_CBR:
				case TRAFFIC_SENSING:
				case TRAFFIC_BSM_APP:
					pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
					pstruEventDetails->nSubEventType=0;
					fnpAddEvent(pstruEventDetails);
					break;
				case TRAFFIC_COAP:
				{

					ptrAPPLICATION_INFO appInfo = (ptrAPPLICATION_INFO)pstruEventDetails->szOtherDetails;
					APP_COAP_INFO* info = appInfo->appData;
					
					pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
					pstruEventDetails->nSubEventType = 0;
					/* modified code starts*/
					if (info->pCOAPData->_RETRANSMIT < info->maxRetransmit && info->pCOAPData->Response_Received == 0) {

						fnpAddEvent(pstruEventDetails);
						info->pCOAPData->Response_Received = 0;
						info->pCOAPData->RequestACK_Received = 0;
						double temp = pstruEventDetails->dEventTime - (int)info->pageIAT + info->ackTimeOut*MILLISECOND;
						fn_NetSim_Application_StartCOAPAPP(appInfo, temp);
						info->pCOAPData->_RETRANSMIT += 1;


					}
					else {
						info->pCOAPData->_RETRANSMIT = 1;
						info->pCOAPData->Response_Received = 0;


					}

					pstruEventDetails->szOtherDetails = appInfo;
				}
				break;
				case TRAFFIC_ERLANG_CALL:
					{
						ptrAPPLICATION_INFO appInfo=(ptrAPPLICATION_INFO)pstruEventDetails->szOtherDetails;
						pstruEventDetails->nEventType=APPLICATION_OUT_EVENT;
						pstruEventDetails->nSubEventType=0;
						fnpAddEvent(pstruEventDetails);
						pstruEventDetails->szOtherDetails=appInfo;
						fn_NetSim_Application_ErlangCall_StartCall(pstruEventDetails);
					}
					break;

				default:
					fnNetSimError("Unknown application type %d in application_run.\n",pstruEventDetails->pPacket->pstruAppData->nAppType);
					break;
				}
				break;
			case event_APP_RESTART:
				fn_NetSim_App_RestartApplication();
				break;
			case event_APP_END:
				switch(applicationInfo[pstruEventDetails->nApplicationId-1]->nAppType)
				{
				case TRAFFIC_ERLANG_CALL:
					fn_NetSim_Application_ErlangCall_EndCall(pstruEventDetails);
					break;
				}
				break;
			}
		}
		break;
	default:
		fnNetSimError("Unknown event type for Application");
		break;
	}
	return 1;
}
/**
This function is called by NetworkStack.dll, while writing the event trace 
to get the sub event as a string.
*/
_declspec (dllexport) char *fn_NetSim_Application_Trace(int nSubEvent)
{
	return "";
}
/**
This function is called by NetworkStack.dll, to free the application information from a packets.
*/
_declspec(dllexport) int fn_NetSim_Application_FreePacket(NetSim_PACKET* pstruPacket)
{
	return 1;
}
/**
This function is called by NetworkStack.dll, to copy the application
related information to a new packet 
*/
_declspec(dllexport) int fn_NetSim_Application_CopyPacket(NetSim_PACKET* pstruDestPacket,NetSim_PACKET* pstruSrcPacket)
{
	return 1;
}
/**
This function writes the Application metrics in Metrics.txt	
*/
_declspec(dllexport) int fn_NetSim_Application_Metrics(PMETRICSWRITER metricsWriter)
{
	return fn_NetSim_Application_Metrics_F(metricsWriter);
}

/**
This function is used to configure the packet trace
*/
_declspec(dllexport) char* fn_NetSim_Application_ConfigPacketTrace()
{
	return "";
}
/**
This function is used to write the packet trace																									
*/
_declspec(dllexport) int fn_NetSim_Application_WritePacketTrace(NetSim_PACKET* pstruPacket, char** ppszTrace)
{
	return 1;
}
/**
	This function is called by NetworkStack.dll, once simulation ends, to free the 
	allocated memory.	
*/
_declspec(dllexport) int fn_NetSim_Application_Finish()
{
	unsigned int loop;
	for(loop=0;loop<nApplicationCount;loop++)
	{
		free(applicationInfo[loop]->sourceList);
		free(applicationInfo[loop]->destList);
		free((char*)(applicationInfo[loop]->appData));

		free_app_metrics(applicationInfo[loop]);

		while(applicationInfo[loop]->socketInfo)
			LIST_FREE((void**)&applicationInfo[loop]->socketInfo, applicationInfo[loop]->socketInfo);
		free(applicationInfo[loop]);
	}
	
	free(applicationInfo);
	return 1;
}

static PACKET_PRIORITY get_priority_based_on_qos(QUALITY_OF_SERVICE qos)
{
	switch (qos)
	{
		case QOS_UGS:
		//case QOS_GBR:
			return Priority_High;
		case QOS_rtPS:
			return Priority_Medium;
		case QOS_ertPS:
			return Priority_Normal;
		case QOS_nrtPS:
			return Priority_Low;
		case QOS_BE:
		//case QOS_NONGBR:
			return Priority_Low;
		default:
			return Priority_Low;
	}
}
/**
This function is used to generate the packets for application
*/
NetSim_PACKET* fn_NetSim_Application_GeneratePacket(ptrAPPLICATION_INFO info,
													double ldArrivalTime,
													NETSIM_ID nSourceId,
													UINT destCount,
													NETSIM_ID* nDestination,
													unsigned long long int nPacketId,
													APPLICATION_TYPE nAppType,
													QUALITY_OF_SERVICE nQOS,
													unsigned int sourcePort,
													unsigned int destPort)
{
	NetSim_PACKET* pstruPacket;
	pstruPacket = fn_NetSim_Packet_CreatePacket(5);
	pstruPacket->dEventTime = ldArrivalTime;

	add_destlist_to_packet(pstruPacket, nDestination, destCount);

	pstruPacket->nPacketId = nPacketId;
	pstruPacket->nPacketStatus = PacketStatus_NoError;
	pstruPacket->nPacketType = fn_NetSim_Application_GetPacketTypeBasedOnApplicationType(nAppType);
	pstruPacket->pstruAppData->nAppType = nAppType;
	pstruPacket->nPacketPriority = get_priority_based_on_qos(nQOS);
	pstruPacket->nQOS = nQOS;
	pstruPacket->nSourceId = nSourceId;
	pstruPacket->pstruTransportData->nSourcePort = sourcePort;
	pstruPacket->pstruTransportData->nDestinationPort = destPort;
	pstruPacket->pstruAppData->nApplicationProtocol = PROTOCOL_APPLICATION;

	if (NETWORK->ppstruDeviceList[nSourceId - 1]->pstruNetworkLayer && NETWORK->ppstruDeviceList[nSourceId - 1]->pstruNetworkLayer->ipVar)
	{
		//Add the source ip address
		pstruPacket->pstruNetworkData->szSourceIP = IP_COPY(fn_NetSim_Stack_GetFirstIPAddressAsId(nSourceId, 0));
		//Add the destination IP address

		if (info->nTransmissionType == MULTICAST)
		{
			//Multicast
			pstruPacket->pstruNetworkData->szDestIP = IP_COPY(info->multicastDestIP);
		}
		else if (info->nTransmissionType == UNICAST)
		{
			//UNICAST
			pstruPacket->pstruNetworkData->szDestIP = IP_COPY(DNS_QUERY(nSourceId, nDestination[0]));
		}
		else if (info->nTransmissionType == BROADCAST)
		{
			//BROADCAST
			NETSIM_IPAddress ip = pstruPacket->pstruNetworkData->szSourceIP;
			if (ip->type == 4)
				pstruPacket->pstruNetworkData->szDestIP = STR_TO_IP("255.255.255.255", 4);
			else
				pstruPacket->pstruNetworkData->szDestIP = STR_TO_IP("FF00:0:0:0:0:0:0:0", 6);
		}
		else
		{
			fnNetSimError("Unknown transmission type %d\n", info->nTransmissionType);
		}
	}
	//Set TTL value
	pstruPacket->pstruNetworkData->nTTL = MAX_TTL;
	return pstruPacket;
}

/**
This function is used to generate the next packet
*/
int fn_NetSim_Application_GenerateNextPacket(ptrAPPLICATION_INFO appInfo,
											 NETSIM_ID nSource,
											 UINT destCount,
											 NETSIM_ID* nDestination,
											 double time)
{
	if (appInfo->dEndTime <= time)
		return 0;

	if (appInfo->nAppState != appState_Started)
		return 0;

	switch (appInfo->nAppType)
	{
	case TRAFFIC_FTP:
	case TRAFFIC_CUSTOM:
	case TRAFFIC_DATA:
	case TRAFFIC_CBR:
	case TRAFFIC_SENSING:
	{
		double arrivalTime = 0;
		double packetSize = 0;
		fn_NetSim_TrafficGenerator_Custom((APP_DATA_INFO*)appInfo->appData,
										  &packetSize,
										  &arrivalTime,
										  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]),
										  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]));
		
		if (appInfo->dEndTime <= time + arrivalTime)
			return 0;

		pstruEventDetails->dEventTime = time + arrivalTime;
		pstruEventDetails->dPacketSize = (double)((int)packetSize);
		pstruEventDetails->nApplicationId = appInfo->id;
		pstruEventDetails->nDeviceId = nSource;
		pstruEventDetails->nDeviceType = DEVICE_TYPE(nSource);
		pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
		pstruEventDetails->nInterfaceId = 0;
		pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
		pstruEventDetails->nSegmentId = 0;
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket =
			fn_NetSim_Application_GeneratePacket(appInfo,
												 pstruEventDetails->dEventTime,
												 nSource,
												 destCount,
												 nDestination,
												 ++appInfo->nPacketId,
												 appInfo->nAppType,
												 appInfo->qos,
												 appInfo->sourcePort,
												 appInfo->destPort);
		pstruEventDetails->nPacketId = appInfo->nPacketId;
		pstruEventDetails->szOtherDetails = appInfo;
		fnpAddEvent(pstruEventDetails);
	}
	break;
	case TRAFFIC_DATABASE:
		break;
	case TRAFFIC_HTTP:
		break;
	case TRAFFIC_VIDEO:
	{
		double arrivalTime = 0;
		double packetSize = 0;
		fn_NetSim_TrafficGenerator_Video((APP_VIDEO_INFO*)appInfo->appData,
										 &packetSize,
										 &arrivalTime,
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]));
		if (appInfo->dEndTime <= time + arrivalTime)
			return 0;

		pstruEventDetails->dEventTime = time + arrivalTime;
		pstruEventDetails->dPacketSize = (double)((int)packetSize);
		pstruEventDetails->nApplicationId = appInfo->id;
		pstruEventDetails->nDeviceId = nSource;
		pstruEventDetails->nDeviceType = DEVICE_TYPE(nSource);
		pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
		pstruEventDetails->nInterfaceId = 0;
		pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
		pstruEventDetails->nSegmentId = 0;
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket =
			fn_NetSim_Application_GeneratePacket(appInfo,
												 pstruEventDetails->dEventTime,
												 nSource,
												 destCount,
												 nDestination,
												 ++appInfo->nPacketId,
												 appInfo->nAppType,
												 appInfo->qos,
												 appInfo->sourcePort,
												 appInfo->destPort);
		pstruEventDetails->nPacketId = appInfo->nPacketId;
		pstruEventDetails->szOtherDetails = appInfo;
		fnpAddEvent(pstruEventDetails);
	}
	break;
	case TRAFFIC_VOICE:
	{
		double arrivalTime = 0;
		double packetSize = 0;
		fn_NetSim_TrafficGenerator_Voice((APP_VOICE_INFO*)appInfo->appData,
										 &packetSize,
										 &arrivalTime,
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]));

		if (appInfo->dEndTime <= time + arrivalTime)
			return 0;

		pstruEventDetails->dEventTime = time + arrivalTime;
		pstruEventDetails->dPacketSize = (double)((int)packetSize);
		pstruEventDetails->nApplicationId = appInfo->id;
		pstruEventDetails->nDeviceId = nSource;
		pstruEventDetails->nDeviceType = DEVICE_TYPE(nSource);
		pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
		pstruEventDetails->nInterfaceId = 0;
		pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
		pstruEventDetails->nSegmentId = 0;
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket =
			fn_NetSim_Application_GeneratePacket(appInfo,
												 pstruEventDetails->dEventTime,
												 nSource,
												 destCount,
												 nDestination,
												 ++appInfo->nPacketId,
												 appInfo->nAppType,
												 appInfo->qos,
												 appInfo->sourcePort,
												 appInfo->destPort);
		pstruEventDetails->nPacketId = appInfo->nPacketId;
		pstruEventDetails->szOtherDetails = appInfo;
		fnpAddEvent(pstruEventDetails);
	}
	break;
	case TRAFFIC_ERLANG_CALL:
	{
		APP_CALL_INFO* info = (APP_CALL_INFO*)appInfo->appData;
		NetSim_PACKET* packet = info->nextPacket[nSource - 1][nDestination[0] - 1];
		double arrivalTime = 0;
		double packetSize = 0;
		if (info->nCallStatus[nSource - 1][nDestination[0] - 1] == 0 || packet == NULL)
			return 0; //Call is ended
		pstruEventDetails->dEventTime = packet->dEventTime;
		pstruEventDetails->dPacketSize = fnGetPacketSize(packet);
		pstruEventDetails->nApplicationId = appInfo->id;
		pstruEventDetails->nDeviceId = nSource;
		pstruEventDetails->nDeviceType = DEVICE_TYPE(nSource);
		pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
		pstruEventDetails->nInterfaceId = 0;
		pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
		pstruEventDetails->nSegmentId = 0;
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket = packet;
		pstruEventDetails->nPacketId = packet->nPacketId;
		pstruEventDetails->szOtherDetails = appInfo;
		if (info->nAppoutevent == NULL)
		{
			NETSIM_ID i;
			info->nAppoutevent = (unsigned long long**)calloc(NETWORK->nDeviceCount, sizeof* info->nAppoutevent);
			for (i = 0; i < NETWORK->nDeviceCount; i++)
				info->nAppoutevent[i] = (unsigned long long*)calloc(NETWORK->nDeviceCount, sizeof* info->nAppoutevent[i]);
		}
		info->nAppoutevent[nSource - 1][nDestination[0] - 1] = fnpAddEvent(pstruEventDetails);
		//Generate next packet
		fn_NetSim_TrafficGenerator_Voice(&(info->VoiceInfo), &packetSize, &arrivalTime,
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
										 &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]));
		if (info->dCallEndTime[nSource - 1][nDestination[0] - 1] <= time + arrivalTime)
		{
			info->nextPacket[nSource - 1][nDestination[0] - 1] = NULL;
			packet->pstruAppData->nAppEndFlag = 1;//last packet
			return 0;
		}
		info->nextPacket[nSource - 1][nDestination[0] - 1] =
			fn_NetSim_Application_GeneratePacket(appInfo,
												 pstruEventDetails->dEventTime + arrivalTime,
												 nSource,
												 destCount,
												 nDestination,
												 ++appInfo->nPacketId,
												 appInfo->nAppType,
												 appInfo->qos,
												 appInfo->sourcePort,
												 appInfo->destPort);
		info->nextPacket[nSource - 1][nDestination[0] - 1]->pstruAppData->dPacketSize = packetSize;
		info->nextPacket[nSource - 1][nDestination[0] - 1]->pstruAppData->dPayload = packetSize;
	}
	break;
	case TRAFFIC_BSM_APP:
	{
		double arrivalTime = 0;
		double packetSize = 0;
		fn_NetSim_Application_BSM((PAPP_BSM_INFO)appInfo->appData,
								  &packetSize,
								  &arrivalTime,
								  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
								  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]),
								  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[0]),
								  &(NETWORK->ppstruDeviceList[nSource - 1]->ulSeed[1]));

		if (appInfo->dEndTime <= time + arrivalTime)
			return 0;

		pstruEventDetails->dEventTime = time + arrivalTime;
		pstruEventDetails->dPacketSize = (double)((int)packetSize);
		pstruEventDetails->nApplicationId = appInfo->id;
		pstruEventDetails->nDeviceId = nSource;
		pstruEventDetails->nDeviceType = DEVICE_TYPE(nSource);
		pstruEventDetails->nEventType = APPLICATION_OUT_EVENT;
		pstruEventDetails->nInterfaceId = 0;
		pstruEventDetails->nProtocolId = PROTOCOL_APPLICATION;
		pstruEventDetails->nSegmentId = 0;
		pstruEventDetails->nSubEventType = 0;
		pstruEventDetails->pPacket =
			fn_NetSim_Application_GeneratePacket(appInfo,
												 pstruEventDetails->dEventTime,
												 nSource,
												 destCount,
												 nDestination,
												 ++appInfo->nPacketId,
												 appInfo->nAppType,
												 appInfo->qos,
												 appInfo->sourcePort,
												 appInfo->destPort);
		pstruEventDetails->nPacketId = appInfo->nPacketId;
		pstruEventDetails->szOtherDetails = appInfo;
		fnpAddEvent(pstruEventDetails);
	}
	break;
	default:
		fnNetSimError("Unknown application type %d in Generate_next_packet.\n", appInfo->nAppType);
		break;
	}
	return 1;
}

void copy_payload(UINT8 real[],NetSim_PACKET* packet,unsigned int* payload, ptrAPPLICATION_INFO info)
{
	u_short i;
	uint32_t key = 16;
	if (payload)
	{
		for (i = 0; i < *payload; i++)
		{
			if (info->encryption == Encryption_XOR)
				real[i] = xor_encrypt('a' + i % 26, 16);
			else
				real[i] = 'a' + i % 26;
		}
		if (info->encryption == Encryption_TEA)
			encryptBlock(real, payload, &key);
		else if (info->encryption == Encryption_AES)
			aes256(real,payload);
		else if(info->encryption==Encryption_DES)
			des(real,payload);
	}
}

int fn_NetSim_Add_DummyPayload(NetSim_PACKET* packet, ptrAPPLICATION_INFO info)
{
	if (add_sae_j2735_payload(packet,info))
		return 0; // Payload added by SAE J2735 Module

	while(packet)
	{
		if(!packet->szPayload &&
		   packet->pstruAppData &&
		   packet->pstruAppData->dPacketSize &&
		   packet->pstruAppData->nAppType != TRAFFIC_EMULATION /* Don't set payload in emulation */ &&
		   wireshark_flag)
		{
			unsigned int size=(unsigned int)packet->pstruAppData->dPacketSize;
			packet->szPayload = (PPACKET_INFO)calloc(1,sizeof* packet->szPayload);
			copy_payload(packet->szPayload->packet,packet,&size,info);
			packet->szPayload->packet_len = size;
			packet->pstruAppData->dPacketSize = (double)size;
		}
		packet=packet->pstruNextPacket;
	}
	return 0;
}

double get_random_startupdelay()
{
	return RANDOM_STARTUP_DELAY*NETSIM_RAND_01();
}
