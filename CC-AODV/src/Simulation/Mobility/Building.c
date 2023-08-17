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
#define _NETSIM_MOBILITY_CODE_
#include "main.h"
#include "Mobility.h"
#include "MobilityInterface.h"

typedef struct stru_netsim_office
{
	UINT id;
	char* name;
	double X1;
	double X2;
	double Y1;
	double Y2;
}NETSIM_OFFICE, * ptrNETSIM_OFFICE;
static UINT officeCount = 0;
static ptrNETSIM_OFFICE* officeList;

#define MOBILITY_OFFICE_COUNT_DEFAULT			0
#define MOBILITY_OFFICE_ID_DEFAULT				0
#define MOBILITY_OFFICE_NAME_DEFAULT			_strdup("Office1")
#define MOBILITY_OFFICE_X1_DEFAULT				0
#define MOBILITY_OFFICE_Y1_DEFAULT				0
#define MOBILITY_OFFICE_X2_DEFAULT				0
#define MOBILITY_OFFICE_Y2_DEFAULT				0

static void configure_office(void* xmlNode, ptrNETSIM_OFFICE office)
{
	getXmlVar(&office->id, ID, xmlNode, 1, _UINT, MOBILITY_OFFICE);
	getXmlVar(&office->name, NAME, xmlNode, 1, _STRING, MOBILITY_OFFICE);
	getXmlVar(&office->X1, X1, xmlNode, 1, _DOUBLE, MOBILITY_OFFICE);
	getXmlVar(&office->X2, X2, xmlNode, 1, _DOUBLE, MOBILITY_OFFICE);
	getXmlVar(&office->Y1, Y1, xmlNode, 1, _DOUBLE, MOBILITY_OFFICE);
	getXmlVar(&office->Y2, Y2, xmlNode, 1, _DOUBLE, MOBILITY_OFFICE);
}

void fn_NetSim_MObility_configureOffice(void* xmlNetSimNode)
{
	getXmlVar(&officeCount, OFFICE_COUNT, xmlNetSimNode, 1, _UINT, MOBILITY);
	if (officeCount)
		officeList = calloc(officeCount, sizeof * officeList);
	else
		return;

	UINT i;
	for (i = 0; i < officeCount; i++)
	{
		void* xmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "OFFICE", i);
		if (!xmlChild)
			break;

		officeList[i] = calloc(1, sizeof * officeList[i]);
		configure_office(xmlChild, officeList[i]);
	}

}

_declspec(dllexport) bool fnMobility_isPosInsideBuilding(NetSim_COORDINATES* pos, NETSIM_ID id)
{
	if ((pos->X >= officeList[id - 1]->X1 && pos->X <= officeList[id - 1]->X2) &&
		(pos->Y >= officeList[id - 1]->Y1 && pos->Y <= officeList[id - 1]->Y2))
		return true;
	return false;
}

_declspec(dllexport) UINT fnMobility_isPosInsideAnyBuilding(NetSim_COORDINATES* pos)
{
	UINT i;
	for (i = 0; i < officeCount; i++)
	{
		if (fnMobility_isPosInsideBuilding(pos, i + 1))
			return i + 1;
	}
	return 0;
}

_declspec(dllexport) bool fnMobility_findIntersectionPointbyBuilding(NetSim_COORDINATES* p1, NetSim_COORDINATES* p2,
														  UINT id, NetSim_COORDINATES* ret)
{
	NetSim_COORDINATES of[4];

	int i;
	for (i = 0; i < 4; i++)
		memset(&of[i], 0, sizeof of[i]);

	of[0].X = officeList[id - 1]->X1;
	of[0].Y = officeList[id - 1]->Y1;

	of[1].X = officeList[id - 1]->X1;
	of[1].Y = officeList[id - 1]->Y2;

	of[2].X = officeList[id - 1]->X2;
	of[2].Y = officeList[id - 1]->Y2;

	of[3].X = officeList[id - 1]->X2;
	of[3].Y = officeList[id - 1]->Y1;

	if (fnMobility_findIntersect(p1, p2, &of[0], &of[1], ret))
		return true;

	if (fnMobility_findIntersect(p1, p2, &of[1], &of[2], ret))
		return true;

	if (fnMobility_findIntersect(p1, p2, &of[2], &of[3], ret))
		return true;

	if (fnMobility_findIntersect(p1, p2, &of[3], &of[0], ret))
		return true;

	return false;
}

_declspec(dllexport) double fnMobility_findIndoorDistance(NetSim_COORDINATES* p1, NetSim_COORDINATES* p2)
{
	UINT id1, id2, id;
	NetSim_COORDINATES* indoor;
	NetSim_COORDINATES* outdoor;

	id1 = fnMobility_isPosInsideAnyBuilding(p1);
	id2 = fnMobility_isPosInsideAnyBuilding(p2);

	if (id1 == 0 && id2 == 0)
		return 0.0; // Both pos are outdoor

	if (id1 > 0 && id2 > 0)
		return fn_NetSim_Utilities_CalculateDistance(p1, p2); //Both are indoor

	if (id1 > 0)
	{
		indoor = p1;
		outdoor = p2;
		id = id1;
	}
	else
	{
		indoor = p2;
		outdoor = p1;
		id = id2;
	}

	NetSim_COORDINATES ret;
	memset(&ret, 0, sizeof ret);
	fnMobility_findIntersectionPointbyBuilding(p1, p2, id, &ret);

	if (id1 > 0)
		return fn_NetSim_Utilities_CalculateDistance(p1, &ret);
	else
		return fn_NetSim_Utilities_CalculateDistance(p2, &ret);
}
