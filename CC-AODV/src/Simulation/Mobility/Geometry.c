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
#include "main.h"
#include "Mobility.h"

_declspec(dllexport) bool fnMobility_findIntersect(NetSim_COORDINATES* p1, NetSim_COORDINATES* q1,
												   NetSim_COORDINATES* p2, NetSim_COORDINATES* q2,
												   NetSim_COORDINATES* intersect)
{
	double s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
	s10_x = q1->X - p1->X;
	s10_y = q1->Y - p1->Y;
	s32_x = q2->X - p2->X;
	s32_y = q2->Y - p2->Y;

	denom = s10_x * s32_y - s32_x * s10_y;
	if (denom == 0)
		return false; // Collinear
	bool denomPositive = denom > 0;

	s02_x = p1->X - p2->X;
	s02_y = p1->Y - p2->Y;
	s_numer = s10_x * s02_y - s10_y * s02_x;

	if ((s_numer < 0) == denomPositive)
		return false; // No collision

	t_numer = s32_x * s02_y - s32_y * s02_x;
	if ((t_numer < 0) == denomPositive)
		return false; // No collision

	if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
		return false; // No collision

	// Collision detected
	t = t_numer / denom;
	if (intersect)
		intersect->X = p1->X + (t * s10_x);
	if (intersect)
		intersect->Y = p1->Y + (t * s10_y);

	return true;
}
