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
#ifndef _NETSIM_LTENR_UERRC_H_
#define _NETSIM_LTENR_UERRC_H_

#ifdef  __cplusplus
extern "C" {
#endif

	typedef enum enum_LTENR_UE_RRCSTATE
	{
		RRCSTATE_IDLE,
		RRCSTATE_CELLSEARCH,
		RRCSTATE_WAITMIB,
		RRCSTATE_WAITSIB1,
		RRCSTATE_WAITSIB2,
		RRCSTATE_RANDOMACCESS,
		RRCSTATE_CONNECTING,
		RRCSTATE_CONNECTED,
	}LTENR_UE_RRCSTATE;

	typedef struct stru_LTENR_UE_RRC
	{
		NETSIM_ID gnbId;
		NETSIM_ID gnbInterface;
		UINT cellId;
		LTENR_UE_RRCSTATE rrcState;
		double T300;
	}LTENR_UE_RRC,*ptrLTENR_UE_RRC;

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_UERRC_H_
