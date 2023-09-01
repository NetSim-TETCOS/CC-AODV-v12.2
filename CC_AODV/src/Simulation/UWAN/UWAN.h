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
#ifndef _NETSIM_UWAN_H_
#define _NETSIM_UWAN_H_

//For MSVC compiler. For GCC link via Linker command
#pragma comment(lib,"NetworkStack.lib")
#pragma comment(lib,"PropagationModel.lib")
#ifndef _NETSIM_UWAN_CODE_
#pragma comment(lib,"libUWAN.lib")
#endif

#ifdef  __cplusplus
extern "C" {
#endif

	_declspec(dllexport) double UWAN_calculate_propagation_delay(NETSIM_ID tx,
																 NETSIM_ID txi,
																 NETSIM_ID rx,
																 NETSIM_ID rxi,
																 void* propagationHandle);
	_declspec(dllexport) double UWAN_Calculate_ber(NETSIM_ID tx,
												   NETSIM_ID rx,
												   PROPAGATION_HANDLE handle,
												   double rxPower,
												   PHY_MODULATION modulation,
												   double dataRate /* In kbps */,
												   double bandwidth /* In kHz */);
#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_UWAN_H_ */