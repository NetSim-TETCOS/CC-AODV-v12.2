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
#include "LTENR_Spectrum.h"

void LTENR_PHY_GET_OH(ptrLTENR_SPECTRUMCONFIG sc,
					  double* dlOH,
					  double* ulOH)
{
	if (sc->Series3GPP == 38) {
#pragma warning (disable : 4047)
		int CA_ID = pstruEventDetails->szOtherDetails;
#pragma warning (default : 4047)
		ptrLTENR_CA ca = sc->CA[CA_ID];
		if (!_stricmp(ca->frequencyRange, "FR1"))
		{
			*dlOH = 0.14;
			*ulOH = 0.08;
		}
		else if (!_stricmp(ca->frequencyRange, "FR2"))
		{
			*dlOH = 0.18;
			*ulOH = 0.10;
		}
		else
		{
			fnNetSimError("Unknown frequecy range %s\n", ca->frequencyRange);
			*dlOH = 0;
			*ulOH = 0;
		}
	}
	else if (sc->Series3GPP == 36) {
		*dlOH = 0.25;
		*ulOH = 0.25;
	}
}
