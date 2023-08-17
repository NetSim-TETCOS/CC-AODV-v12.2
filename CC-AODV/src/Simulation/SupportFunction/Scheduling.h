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
* Author:    Ramsaran Giri		                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_SCHEDULING_H_
#define _NETSIM_SCHEDULING_H_
#ifdef  __cplusplus
extern "C" {
#endif

	typedef struct stru_queuing_red
	{
		double min_th;
		double max_th;
		double max_p;
		double avg_queue_size;
		int count;
		double random_value;
		double wq;            // Weight for avg queue size
		double q_time;
	}QUEUING_RED_VAR, * ptrQUEUING_RED_VAR;

	typedef struct stru_queuing_wred
	{
		double* min_th;
		double* max_th;
		double* max_p;
		double avg_queue_size;
		int* count;
		double* random_value;
		double wq;
		double q_time;
	}QUEUING_WRED_VAR, * ptrQUEUING_WRED_VAR;

	typedef struct stru_scheduling_edf
	{
		double* max_latency;
	}SCHEDULING_EDF_VAR, * ptrSCHEDULING_EDF_VAR;

#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_SCHEDULING_H_
