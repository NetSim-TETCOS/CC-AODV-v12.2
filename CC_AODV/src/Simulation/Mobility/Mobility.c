/************************************************************************************
* Copyright (C) 2013                                                               *
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
#include "main.h"
#include "Mobility.h"
#include "Animation.h"

static unsigned int nCallBackCount=0;

int fn_NetSim_Mobility_configure_NodeJoinProfile(void* xmlNode,NETSIM_ID nDeviceId);
int fn_NetSim_Mobility_NodeJoinInit(NETSIM_ID nDevIndex);
int fn_NetSim_Mobility_NodeJoined();
int fn_NetSim_Mobility_NodeLeaved();

ANIM_HANDLE animHandle;
static void init_mobility_animation()
{
	animHandle = anim_add_new_menu(NULL,
								   "Node Movement",
								   false,
								   false,
								   true,
								   256,
								   ANIMFILETYPE_GENERIC);
}

void add_mobility_animation(NETSIM_ID d,
							double t,
							double x,
							double y,
							double z)
{
	MOVENODE mn;
	mn.d = DEVICE_CONFIGID(d);
	mn.time = t;
	mn.x = x;
	mn.y = y;
	mn.z = z;
	animation_add_new_entry(animHandle, ANIM_MOVENODE, &mn);
}

/** Function to configure the mobility model for all the devices */
_declspec(dllexport) int fn_NetSim_Mobility_Configure(void** var)
{
	char* tagname = (char*)var[0];
	void* xmlNetSimNode=var[2];
	NETSIM_ID nDeviceId=var[3]?*((NETSIM_ID*)var[3]):0;
	NETSIM_ID nInterfaceId = var[4]?*((NETSIM_ID*)var[4]):0;
	LAYER_TYPE nLayerType = var[5]?*((LAYER_TYPE*)var[5]):LAYER_NULL;
	char* szVal;
	if(!_stricmp(tagname,"MOBILITY"))
	{
		MOBILITY_VAR* pstruMobilityVar=(MOBILITY_VAR*)calloc(1,sizeof* pstruMobilityVar);
		NETWORK=(struct stru_NetSim_Network*)var[1];
		NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->pstruMobVar = pstruMobilityVar;
		//Configure the Model
		getXmlVar(&szVal,MODEL,xmlNetSimNode,1,_STRING,Mobility);

		if(!strcmp(szVal,"NO_MOBILITY"))
		{
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_NOMOBILITY;
			return 0;
		}
		else if(!strcmp(szVal,"RANDOM_WALK"))
		{
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_RANDOMWALK;
		}
		else if(!strcmp(szVal,"RANDOM_WAY_POINT"))
		{
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_RANDOMWAYPOINT;
		}
		else if(!strcmp(szVal,"FILE_BASED_MOBILITY"))
		{
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_FILEBASEDMOBILITY;
		}
		else if(!strcmp(szVal,"GROUP_MOBILITY"))
		{
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_GROUP;
		}
		else if (!_stricmp(szVal, "SUMO")) {
			NETWORK->ppstruDeviceList[nDeviceId - 1]->pstruDeviceMobility->nMobilityType = MobilityModel_SUMO;
		}
		else if (!_stricmp(szVal, "PEDESTRIAN_MOBILITY")) {
			NETWORK->ppstruDeviceList[nDeviceId - 1]->pstruDeviceMobility->nMobilityType = MobilityModel_PEDESTRAIN;
		}
		else
		{
			NetSimxmlError("Unknown mobility model",szVal,xmlNetSimNode);
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType = MobilityModel_NOMOBILITY;
			return -1;
		}
		free(szVal);

		if(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType == MobilityModel_FILEBASEDMOBILITY)
		{
			//IF node has FileBasedMobility
			pstruMobilityVar->dLastTime = 0;
			return 1;
		}
		//Get the velocity
		szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"VELOCITY",
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType!=MobilityModel_SUMO&&
			NETWORK->ppstruDeviceList[nDeviceId - 1]->pstruDeviceMobility->nMobilityType != MobilityModel_PEDESTRAIN);
		if(szVal)
		{
			pstruMobilityVar->dVelocity = atof(szVal);
			NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->dAvgSpeed = atof(szVal);
			free(szVal);
		}

		getXmlVar(&pstruMobilityVar->dCalculationInterval, CALCULATION_INTERVAL, xmlNetSimNode, 0, _DOUBLE, Mobility);
		pstruMobilityVar->dCalculationInterval *= SECOND;
		if (pstruMobilityVar->dCalculationInterval == 0.0)
		{
			fnNetSimError("Calculation_interval can't be 0. Leads infinite loop. Assigning 1 us");
			pstruMobilityVar->dCalculationInterval = 1;
		}

		switch(NETWORK->ppstruDeviceList[nDeviceId-1]->pstruDeviceMobility->nMobilityType)
		{
		case MobilityModel_RANDOMWAYPOINT:
			{
				//Get the pause time
				szVal = fn_NetSim_xmlConfig_GetVal(xmlNetSimNode,"PAUSE_TIME",1);
				if(szVal)
				{
					pstruMobilityVar->dPauseTime = atof(szVal);
					pstruMobilityVar->dLastTime = -1000000*atof(szVal);
					free(szVal);
				}
			}
			break;
		case MobilityModel_GROUP:
			{
				getXmlVar(&pstruMobilityVar->nGroupId,GROUP_ID,xmlNetSimNode,1,_INT,Mobility);
				add_to_group(pstruMobilityVar->nGroupId,nDeviceId);
			}
			break;
		case MobilityModel_SUMO:
			{
				extern char* sumoname;
				extern double step_size;
				getXmlVar(&pstruMobilityVar->sumoFileName,FILE_NAME,xmlNetSimNode,1,_STRING,Mobility);
				sumoname = pstruMobilityVar->sumoFileName;
				getXmlVar(&pstruMobilityVar->step_size,STEP_SIZE,xmlNetSimNode,1,_DOUBLE,Mobility);
				step_size = pstruMobilityVar->step_size;
			}
			break;
		case MobilityModel_PEDESTRAIN:
			{
				getXmlVar(&pstruMobilityVar->Max_Speed, MAX_SPEED_M_S, xmlNetSimNode, 1, _DOUBLE, Mobility);
				getXmlVar(&pstruMobilityVar->Min_Speed, MIN_SPEED_M_S, xmlNetSimNode, 1, _DOUBLE, Mobility);
				getXmlVar(&pstruMobilityVar->Stop_Probability, STOP_PROBABILITY, xmlNetSimNode, 1, _DOUBLE, Mobility);
				getXmlVar(&pstruMobilityVar->Stop_Duration, STOP_DURATION_S, xmlNetSimNode, 1, _DOUBLE, Mobility);
				pstruMobilityVar->Stop_Duration *= SECOND;
				pstruMobilityVar->Pedestrain_Speed = NETSIM_RAND_RN(pstruMobilityVar->Max_Speed, pstruMobilityVar->Min_Speed);
				pstruMobilityVar->angel = 0;
			}
			break;
		}

		//Save the seed value
		pstruMobilityVar->ulSeed1 = DEVICE(nDeviceId)->ulSeed[0];
		pstruMobilityVar->ulSeed2 = DEVICE(nDeviceId)->ulSeed[1];
	}
	else if(!_stricmp(tagname,"NODE_JOIN_PROFILE"))
	{
		fn_NetSim_Mobility_configure_NodeJoinProfile(xmlNetSimNode, nDeviceId);
	}
	else if (!_stricmp(tagname, "PROTOCOL"))
	{
		fn_NetSim_MObility_configureOffice(xmlNetSimNode);
	}
	return 1;
}
/** Function to initialize the parameters of positions for all nodes*/
_declspec(dllexport) int fn_NetSim_Mobility_Init(struct stru_NetSim_Network *NETWORK_Formal,
	NetSim_EVENTDETAILS *pstruEventDetails_Formal,
	char *pszAppPath_Formal,
	char *pszWritePath_Formal,
	int nVersion_Type,
	void **fnPointer)
{
	NETSIM_ID nLoop;
	NETWORK = NETWORK_Formal;		//Get the Network structure from NetworkStack	
	pstruEventDetails=pstruEventDetails_Formal;	//Get the Eventdetails from NetworkStack	
	pszAppPath=pszAppPath_Formal;	//Get the application path from NetworkStack	
	pszIOPath=pszWritePath_Formal;	//Get the write path from NetworkStack	
	nVersion_Type = nVersion_Type; //Get the version type from NetworkStack

	srand(1); //Initialize seed as 1 for same result for every run

	init_mobility_animation();

	if(nVersion_Type/10 != VERSION)
	{
		printf("Mobility---Version number mismatch\nDll Version=%d\nNetSim Version=%d\nFileName=%s\nLine=%d\n",VERSION,nVersion_Type/10,__FILE__,__LINE__);
		exit(0);
	}
	
	{
		char* temp=NULL;
		temp = getenv("NETSIM_SIM_AREA_X");
		if(temp)
		{
			if(atoi(temp))
				dSimulationArea_X = atoi(temp);
			else
				dSimulationArea_X = 0xFFFFFFFF;
		}
		else
			dSimulationArea_X = 0xFFFFFFFF;
		temp=NULL;
		temp = getenv("NETSIM_SIM_AREA_Y");
		if(temp)
		{
			if(atoi(temp))
				dSimulationArea_Y = atoi(temp);
			else
				dSimulationArea_Y = 0xFFFFFFFF;
		}
		else
			dSimulationArea_Y = 0xFFFFFFFF;
	}
	for(nLoop=0;nLoop<NETWORK->nDeviceCount;nLoop++)
	{
		fn_NetSim_Mobility_NodeJoinInit(nLoop);
		if(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility)
		{
			MOBILITY_VAR* var = (MOBILITY_VAR*)NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruMobVar;
			var->ulSeed1 = NETWORK->ppstruDeviceList[nLoop]->ulSeed[0];
			var->ulSeed2 = NETWORK->ppstruDeviceList[nLoop]->ulSeed[1];
			switch(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->nMobilityType)
			{
			case MobilityModel_NOMOBILITY:
				//No action
				break;
			case MobilityModel_FILEBASEDMOBILITY:
				{
					//IF node has FileBasedMobility
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					FileBasedMobilityReadingFile();
				}
				break;
			case MobilityModel_RANDOMWALK:
			case MobilityModel_RANDOMWAYPOINT:
			case MobilityModel_PEDESTRAIN:
				{
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					pstruEventDetails->dEventTime = 0;
					pstruEventDetails->dPacketSize = 0;
					pstruEventDetails->nApplicationId = 0;
					pstruEventDetails->nDeviceId = nLoop+1;
					pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[nLoop]->nDeviceType;
					pstruEventDetails->nEventType = TIMER_EVENT;
					pstruEventDetails->nInterfaceId = 0;
					pstruEventDetails->nPacketId = 0;
					pstruEventDetails->nProtocolId = PROTOCOL_MOBILITY;
					pstruEventDetails->nSubEventType = 0;
					pstruEventDetails->pPacket = NULL;
					fnpAddEvent(pstruEventDetails);
				}
				break;
			case MobilityModel_GROUP:
				{
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);	
				}
				break;
			case MobilityModel_SUMO:
				{
					init_sumo();
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition = calloc(1,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruCurrentPosition);
					memcpy(NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition,NETWORK->ppstruDeviceList[nLoop]->pstruDevicePosition,sizeof* NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->pstruNextPosition);
					
					pstruEventDetails->dEventTime = 0;
					pstruEventDetails->dPacketSize = 0;
					pstruEventDetails->nApplicationId = 0;
					pstruEventDetails->nDeviceId = nLoop+1;
					pstruEventDetails->nDeviceType = NETWORK->ppstruDeviceList[nLoop]->nDeviceType;
					pstruEventDetails->nEventType = TIMER_EVENT;
					pstruEventDetails->nInterfaceId = 0;
					pstruEventDetails->nPacketId = 0;
					pstruEventDetails->nProtocolId = PROTOCOL_MOBILITY;
					pstruEventDetails->nSubEventType = 0;
					pstruEventDetails->pPacket = NULL;
					fnpAddEvent(pstruEventDetails);
				}
				break;
			default:
				{
					fnNetSimError("Unknown mobility model %d in fn_NetSim_Mobility_Init\n",NETWORK->ppstruDeviceList[nLoop]->pstruDeviceMobility->nMobilityType);
				}
				break;
			} //End switch(mobility_type)
		}// End if(device_mobility)
	}// End for(loop_all_device)

	fn_NetSim_Mobility_Group_init();
	return 1;
}
/** This function is to free the memory space allocated for the variables that are used in mobily */
_declspec(dllexport) int fn_NetSim_Mobility_Finish()
{

	NETSIM_ID loop;

	for(loop=0;loop<NETWORK->nDeviceCount;loop++)
	{
		if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility)
		{
			if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruMobVar)
			{
				free(((MOBILITY_VAR*)NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruMobVar)->sumoFileName);
				free((MOBILITY_VAR*)NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruMobVar);
			}
			if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruCurrentPosition)
				free(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruCurrentPosition);
			if(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruNextPosition)
				free(NETWORK->ppstruDeviceList[loop]->pstruDeviceMobility->pstruNextPosition);
		}
	}
	FileBasedMobilityPointersFree();
	free(fnMobilityCallBack);
	return 1;
}

void mobility_pass_position_to_animation(NETSIM_ID devId,double time, NetSim_COORDINATES* coor)
{
	if (coor->corrType == CORRTYPE_MAP)
	{
		fn_NetSim_Coordinate_ConvertCartesianToMap(coor);
		add_mobility_animation(devId, time,
							   coor->lat, coor->lon, 0);
	}
	else if (coor->corrType == CORRTYPE_GEO)
	{
		fnNetSimError("change to appropriate function..\n");
		assert(false);
		fn_NetSim_Coordinate_ConvertCartesianToMap(coor);
		add_mobility_animation(devId, time,
							   coor->lat, coor->lon, coor->altitude);
	}
	else
	{
		add_mobility_animation(devId, time,
							   coor->X, coor->Y, coor->Z);
	}
}

/** This function is used to change the positions of the devices over simulation. At the end of this function cummulativereceivedpower[][] will be updated. */
_declspec(dllexport) int fn_NetSim_Mobility_Run()
{
	switch(pstruEventDetails->nSubEventType)
	{
	case MOVE_GROUP:
		fn_NetSim_MoveGroup();
		break;
	case NODE_JOIN:
		fn_NetSim_Mobility_NodeJoined();
		break;
	case NODE_LEAVE:
		fn_NetSim_Mobility_NodeLeaved();
		break;
	default:
		{
			unsigned int nLoop;
			double X = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition->X;
			double Y = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->pstruCurrentPosition->Y;
			double vel = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->dAvgSpeed;
			MOBILITY_VAR* pstruMobilityVar = (MOBILITY_VAR*)NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->pstruDeviceMobility->pstruMobVar;
			double dPresentTime = pstruMobilityVar->dLastTime;
			if (NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->pstruDeviceMobility
				&& NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->pstruDeviceMobility->nMobilityType == MobilityModel_FILEBASEDMOBILITY)
			{
				//IF node has FileBasedMobility
				process_filebased_mobility_event();
				
			}
			else if(NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility
				&& NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId-1]->pstruDeviceMobility->nMobilityType == MobilityModel_SUMO)
			{
				sumo_run();
			}
			else if (NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->pstruDeviceMobility
				&& NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->pstruDeviceMobility->nMobilityType == MobilityModel_PEDESTRAIN) {
				NetSim_MOBILITY* mob = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->pstruDeviceMobility;
				NetSim_COORDINATES* pos = DEVICE_POSITION(pstruEventDetails->nDeviceId);
				NetSim_COORDINATES* cor = mob->pstruCurrentPosition;
				NetSim_COORDINATES* ncor = mob->pstruNextPosition;
				bool isStop = false;
				memcpy(cor, ncor, sizeof * cor);

				double probability = NETSIM_RAND_01();
				if (probability < pstruMobilityVar->Stop_Probability) {
					pstruMobilityVar->Pedestrain_Speed = NETSIM_RAND_RN(pstruMobilityVar->Max_Speed, pstruMobilityVar->Min_Speed);
					pstruMobilityVar->angel = NETSIM_RAND_RN(360, 0);
					isStop = true;
				}
				X = pstruMobilityVar->Pedestrain_Speed * cos(pstruMobilityVar->angel) * (pstruMobilityVar->dCalculationInterval / SECOND) + cor->X;
				Y = pstruMobilityVar->Pedestrain_Speed * sin(pstruMobilityVar->angel) * (pstruMobilityVar->dCalculationInterval / SECOND) + cor->Y;
				while (cor->corrType == CORRTYPE_CARTESIAN &&
					(X > dSimulationArea_X || X < 0 || Y < 0 || Y > dSimulationArea_Y)) {
					pstruMobilityVar->Pedestrain_Speed = NETSIM_RAND_RN(pstruMobilityVar->Max_Speed, pstruMobilityVar->Min_Speed);
					pstruMobilityVar->angel = NETSIM_RAND_RN(360, 0);
					X = pstruMobilityVar->Pedestrain_Speed * cos(pstruMobilityVar->angel) * (pstruMobilityVar->dCalculationInterval / SECOND) + cor->X;
					Y = pstruMobilityVar->Pedestrain_Speed * sin(pstruMobilityVar->angel) * (pstruMobilityVar->dCalculationInterval / SECOND) + cor->Y;
				}
				if (!isStop) {
					ncor->X = X;
					ncor->Y = Y;
				}
				else {
					ncor->X = cor->X;
					ncor->Y = cor->Y;
				}
				//store the last time
				if (isStop)
					pstruMobilityVar->dLastTime = pstruEventDetails->dEventTime + pstruMobilityVar->dCalculationInterval + pstruMobilityVar->Stop_Duration;
				else
					pstruMobilityVar->dLastTime = pstruEventDetails->dEventTime + pstruMobilityVar->dCalculationInterval;

				//update the device position
				memcpy(pos, cor, sizeof * pos);

				mobility_pass_position_to_animation(pstruEventDetails->nDeviceId,
					pstruEventDetails->dEventTime,
					pos);

				//Add event for next point 
				if (isStop) {
					pstruEventDetails->dEventTime = pstruEventDetails->dEventTime + pstruMobilityVar->Stop_Duration + pstruMobilityVar->dCalculationInterval;
					fnpAddEvent(pstruEventDetails);
					pstruEventDetails->dEventTime = pstruEventDetails->dEventTime - pstruMobilityVar->Stop_Duration - pstruMobilityVar->dCalculationInterval;
				}
				else {
					pstruEventDetails->dEventTime += pstruMobilityVar->dCalculationInterval;
					fnpAddEvent(pstruEventDetails);
					pstruEventDetails->dEventTime -= pstruMobilityVar->dCalculationInterval;
				}
			}
			else
			{
				NetSim_MOBILITY* mob = NETWORK->ppstruDeviceList[pstruEventDetails->nDeviceId - 1]->pstruDeviceMobility;
				NetSim_COORDINATES* pos = DEVICE_POSITION(pstruEventDetails->nDeviceId);
				NetSim_COORDINATES* cor = mob->pstruCurrentPosition;
				NetSim_COORDINATES* ncor = mob->pstruNextPosition;

				memcpy(cor, ncor, sizeof* cor);
				if(pstruMobilityVar->dLastTime+pstruMobilityVar->dPauseTime*SECOND<pstruEventDetails->dEventTime+pstruMobilityVar->dCalculationInterval)
				{
					fn_NMo_RandomPoint(&X, &Y, vel, pstruMobilityVar->dCalculationInterval, &pstruMobilityVar->ulSeed1, &pstruMobilityVar->ulSeed2);
					while (cor->corrType == CORRTYPE_CARTESIAN &&
						(X > dSimulationArea_X || X < 0 || Y < 0 || Y > dSimulationArea_Y))
					{
						X = cor->X;
						Y = cor->Y;
						fn_NMo_RandomPoint(&X, &Y, vel, pstruMobilityVar->dCalculationInterval, &pstruMobilityVar->ulSeed1, &pstruMobilityVar->ulSeed2);
					}
					ncor->X = X;
					ncor->Y = Y;
					//store the last time
					pstruMobilityVar->dLastTime = pstruEventDetails->dEventTime+pstruMobilityVar->dCalculationInterval;
				}
				//update the device position
				memcpy(pos,cor,sizeof* pos);
				
				mobility_pass_position_to_animation(pstruEventDetails->nDeviceId,
													pstruEventDetails->dEventTime,
													pos);

				//Add event for next point 
				pstruEventDetails->dEventTime+=pstruMobilityVar->dCalculationInterval;
				fnpAddEvent(pstruEventDetails);
				pstruEventDetails->dEventTime-=pstruMobilityVar->dCalculationInterval;
			}

			//call all the callback function
			for(nLoop=0;nLoop<nCallBackCount;nLoop++)
			{
				fnMobilityCallBack[nLoop](pstruEventDetails->nDeviceId);
			}
		}
		break;
	}
	return 1;
};

_declspec(dllexport) char* fn_NetSim_Mobility_Trace(NETSIM_ID id)
{
	switch(id)
	{
	case MOVE_GROUP:
		return "MOVE_GROUP";
		break;
	case NODE_JOIN:
		return "NODE_JOIN";
	case NODE_LEAVE:
		return "NODE_LEAVE";
	}
	return "";
};
_declspec(dllexport) int fn_NetSim_Mobility_FreePacket()
{
	return 1;
};
_declspec(dllexport) int fn_NetSim_Mobility_CopyPacket()
{
	return 1;
};
_declspec(dllexport) int fn_NetSim_Mobility_Metrics()
{
	return 1;
};
_declspec(dllexport) int fn_NetSim_Mobility_ConfigurePrimitives()
{
	return 1;
};
_declspec(dllexport) char* fn_NetSim_Mobility_ConfigPacketTrace()
{
	return "";
};
_declspec(dllexport) char* fn_NetSim_Mobility_WritePacketTrace()
{
	return "";
};
/** This function is used to generate the random point */
int fn_NMo_RandomPoint(double* X, double* Y,double velocity,double interval,unsigned long *pulSeed1, unsigned long *pulSeed2)
{
	int min;
	int max;
	int ldRandNo;

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1, pulSeed2);

	min = (int)(*X - ldRandNo % ((int)(velocity * interval / SECOND) + 1));

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1, pulSeed2);

	max = (int)(*X + ldRandNo % ((int)(velocity * interval / SECOND) + 1));

	if (min > max)
	{
		*X = max + (int)((min - max + 1) * NETSIM_RAND_01());
	}
	else
	{
		*X = min + (int)((min - max + 1) * NETSIM_RAND_01());
	}

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1, pulSeed2);

	min = (int)(*Y - ldRandNo % ((int)(velocity * interval / SECOND) + 1));

	ldRandNo = (unsigned int)fn_NetSim_Utilities_GenerateRandomNo(pulSeed1, pulSeed2);

	max = (int)(*Y + ldRandNo % ((int)(velocity * interval / SECOND) + 1));

	if (min > max)
	{
		*Y = max + (int)((min - max + 1) * NETSIM_RAND_01());

	}
	else
	{
		*Y = min + (int)((min - max + 1) * NETSIM_RAND_01());
	}
	return 1;
}

_declspec(dllexport) int fnMobilityRegisterCallBackFunction(_fnMobilityCallBack fnCallBack)
{
	if(!nCallBackCount)
	{
		fnMobilityCallBack = (_fnMobilityCallBack*)calloc(1,sizeof* fnMobilityCallBack);
	}
	else
	{
		fnMobilityCallBack = (_fnMobilityCallBack*)realloc(fnMobilityCallBack,(nCallBackCount+1)*sizeof* fnMobilityCallBack);
	}
	fnMobilityCallBack[nCallBackCount] = fnCallBack;
	nCallBackCount++;
	return 0;
}
