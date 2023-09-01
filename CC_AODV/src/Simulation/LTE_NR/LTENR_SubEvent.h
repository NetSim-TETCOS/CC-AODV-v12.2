#pragma once
/************************************************************************************
 * Copyright (C) 2018
 * TETCOS, Bangalore. India															*

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 * Author:	Shashi Kant Suman														*
 * ---------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_SUBEVENT_H_
#define _NETSIM_LTENR_SUBEVENT_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define LTENR_MAX_SUBEVENT	100 // Read-only.

	typedef enum enum_LTENR_Subevnet_type
	{

		//RRC
		LTENR_SUBEVNET_GENERATE_MIB = MAC_PROTOCOL_LTE_NR * 100,
		LTENR_SUBEVENT_GENERATE_SIB1,
		LTENR_SUBEVENT_GENERATE_SI,
		LTENR_SUBEVENT_GENERATE_RRC_SETUP_REQUEST,
		LTENR_SUBEVENT_GENERATE_RRC_SETUP,
		LTENR_SUBEVENT_GENERATE_RRC_SETUP_COMPLETE,
		LTENR_SUBEVENT_GENERATE_RRC_SETUP_REJECT,
		LTENR_SUBEVENT_GENERATE_RRC_UE_MEASUREMENT_REPORT_REQUEST,
		LTENR_SUBEVENT_GENERATE_RRC_UE_MEASUREMENT_REPORT,
		LTENR_SUBEVENT_GENERATE_RRC_REESTABLISHMENT_REQUEST,
		LTENR_SUBEVENT_GENERATE_RRC_REESTABLISHMENT,
		LTENR_SUBEVENT_GENERATE_RRC_RECONFIGURATION,

		//NAS
		LTENR_SUBEVENT_GENERATE_HANDOVER_REQUEST,
		LTENR_SUBEVENT_GENERATE_HANDOVER_REQUEST_ACK,
		LTENR_SUBEVENT_GENERATE_HANDOVER_COMMAND,
		LTENR_SUBEVENT_GENERATE_HANDOVER_COMMAND_HANDLE,
		LTENR_SUBEVENT_GENERATE_PATH_SWITCH,
		LTENR_SUBEVENT_GENERATE_PATH_SWITCH_ACK,
		LTENR_SUBEVENT_GENERATE_UECONTEXTRELEASE,
		LTENR_SUBEVENT_GENERATE_UECONTEXTRELEASE_ACK,
		LTENR_SUBEVENT_GENERATE_HANDOVER_COMPLETE,

		//RRC Timer Event
		LTENR_SUBEVNT_RRC_T300,
		LTENR_SUBEVNT_RRC_T311,

		//PDCP
		LTENR_SUBEVENT_PDCP_DISCARDTIMER,
		LTENR_SUBEVENT_PDCP_TREORDERINGTIMER,

		//RLC
		LTENR_SUBEVNT_RLC_TREASSEMBLYTIMER,
		LTENR_SUBEVNT_RLC_TPOLLRETRANSMITTIMER,
		LTENR_SUBEVNT_RLC_TSTATUSPROHIBITTIMER,

		//MAC

		//PHY
		LTENR_SUBEVENT_PHY_STARTFRAME,
		LTENR_SUBEVENT_PHY_STARTSUBFRAME,
		LTENR_SUBEVENT_PHY_STARTSLOT,
		LTENR_SUBEVENT_PHY_DELETECHANNEL,

		//CORE
		LTENR_SUBEVNET_INTITAL_UE_MSG,

		LTENR_SUBEVENT_LAST, // Keep me at last
	}LTENR_SUBEVENTTYPE;
	
	typedef struct stru_LTENR_SubeventDataBase
	{
		LTENR_SUBEVENTTYPE subEvent;
		char* subeventName;
		char* ltesubeventName;
		void(*fnSubEventfunction)();
	}LTENR_SUBEVENT, *ptrLTENR_SUBEVENT;
	
	void LTENR_SUBEVENT_REGISTER(LTENR_SUBEVENTTYPE type,
								 char* name,
								 void(*fun)());
	char* LTENR_SUBEVNET_NAME(NETSIM_ID id);
	void LTENR_SUBEVENT_CALL();
	void LTENR_SET_SUBEVENT(ptrLTENR_PROTODATA pd, NetSim_EVENTDETAILS* pevent, LTENR_SUBEVENTTYPE type);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_SUBEVENT_H_
