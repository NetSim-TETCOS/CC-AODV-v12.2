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
* Author:    Dilip Kumar Kundrapu													*
* Standard:  TR 38.901(Release 15)													*
* Updated:   27th June, 2019											            *
* ----------------------------------------------------------------------------------*/

#ifndef _NETSIM_LTENR_PROPAGATIONMODEL_H_
#define _NETSIM_LTENR_PROPAGATIONMODEL_H_
#ifdef  __cplusplus
extern "C" {
#endif


	typedef enum
	{
		LTENR_SCENARIO_RMA,
		LTENR_SCENARIO_UMA,
		LTENR_SCENARIO_UMI,
		LTENR_SCENARIO_INH,
	}LTENR_SCENARIO;
	static const char* strLTENR_SCENARIO[] =
	{ "RURAL_MACRO","URBAN_MACRO","URBAN_MICRO","INDOOR_OFFICE" };

	typedef enum
	{
		LTENR_POSITION_OUTDOOR,
		LTENR_POSITION_INDOOR,
	}LTENR_POSITION;

	typedef enum
	{
		LTENR_INH_MIXED_OFFICE,
		LTENR_INH_OPEN_OFFICE,
	}LTENR_INH_TYPE;
	static const char* strLTENR_INH_TYPE[] =
	{ "MIXED_OFFICE","OPEN_OFFICE" };

	typedef enum
	{
		LTENR_STATE_LOS,
		LTENR_STATE_NLOS,
	}LTENR_STATE;
	static const char* strLTENR_STATE[] =
	{ "LOS","NLOS" };

	typedef enum
	{
		LTENR_O2IMODEL_LOW_LOSS,
		LTENR_O2IMODEL_HIGH_LOSS,
	}LTENR_O2I_MODEL;
	static const char* strLTENR_O2IMODEL[] =
	{ "LOW_LOSS_MODEL","HIGH_LOSS_MODEL" };

	typedef enum
	{
		LTENR_CHANNELCHAR_NO_PATHLOSS,
		LTENR_CHANNELCHAR_PATHLOSS_ONLY,
		LTENR_CHANNELCHAR_PATHLOSS_SHADOWFADING,
		LTENR_CHANNELCHAR_PATHLOSS_SHADOWFADING_O2ILOSS,
	}LTENR_CHANNEL_CHARACTERISTICS;
	static const char* strLTENR_CHANNELCHAR[] =
	{ "NO_PATHLOSS","PATHLOSS_ONLY","PATHLOSS_WITH_SHADOWFADING","PATHLOSS_WITH_SHADOWFADING_O2ILOSS" };

	typedef enum
	{
		LTENR_LOS_MODE_TR38_901,
		LTENR_LOS_MODE_USER_DEFINED,
	}LTENR_LOS_MODE;
	static const char* strLTENR_LOS_MODE[] =
	{ "TR38_901_STANDARD", "USER_DEFINED" };

	typedef struct stru_LTENR_PropagationConfig
	{
		LTENR_SCENARIO outScenario;
		LTENR_SCENARIO inScenario;
		LTENR_INH_TYPE indoor_type;
		LTENR_O2I_MODEL O2I_model;
		LTENR_CHANNEL_CHARACTERISTICS channel_character;
		LTENR_LOS_MODE los_mode;
		LTENR_STATE state;

		double los_probability;
		double gNB_height;
		double UE_height;
		double buildings_height;
		double street_width;
		double avg_snr_window;

		double txPower_dbm;
		double txPower_mw;

		double beamFormingGain;
	}LTENR_PROPAGATIONCONFIG, * ptrLTENR_PROPAGATIONCONFIG;


#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_PROPAGATIONMODEL_H_ */
