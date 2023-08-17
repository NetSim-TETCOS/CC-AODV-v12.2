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
* Author:    Shashi Kant Suman	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_SATELLITE_FRAME_H_
#define _NETSIM_SATELLITE_FRAME_H_

#include "Satellite.h"
#include "Satellite_Buffer.h"
#include "ErrorModel.h"

#ifdef  __cplusplus
extern "C" {
#endif

	typedef struct stru_tdma_slot_conf
	{
		UINT symbolPerSlot;
		UINT modulationBits;
		UINT symbolRate;

		double slotLength_us;
		UINT bitsPerSlot;
	}SLOTCONF,*ptrSLOTCONF;

	typedef enum enum_frame_usage_mode
	{
		FRUSAGE_NORMAL,
		FRUSAGE_SHORT,
	}FRAMEUSAGEMODE;

	typedef struct stru_tdma_frame_conf
	{
		UINT slotCountInFrame;
		double frameBandwidth_Hz;

		UINT pilotBlockSize_inSymbols;
		UINT pilotBlockInterval_inSlots;
		UINT plHdrSize_inSlots;
		UINT frameHdrLength_inBytes;
		FRAMEUSAGEMODE usageMode;

		double frameLength_us;
		double pilotBlockLength_us;
		UINT bitsPerFrame;
		UINT framesPerSuperframe;
	}FRAMECONF, * ptrFRAMECONF;
	
	typedef struct stru_tdma_frame
	{
		UINT frameId;
		double bandwidth_Hz;
		bool isRandomAccess;

		UINT carrierId;

		double relativeFrameStartTime;
		double relativeFrameEndTime;

		double frameStartTime;
		double frameEndTime;

		NetSim_PACKET* head;
		NetSim_PACKET* tail;
	}FRAME, * ptrFRAME;

	typedef enum enum_ber_model
	{
		BERMODEL_Fixed,
		BERMODEL_File,
		BERMODEL_Model,
	}BERMODEL;

	typedef struct stru_tdma_carrier_conf
	{
		double allocatedBandwidth_Hz;
		double occupiedBandwidth_Hz;
		double effectiveBandwidth_Hz;
		double rollOffFactor;
		double spacingFactor;

		BERMODEL berModel;
		double berValue;
		size_t berTableLen;
		ptrBER berTable;
		char* berFile;

		PHY_MODULATION modulation;
		double codingRate;
	}CARRIERCONF, * ptrCARRIERCONF;

	typedef struct stru_tdma_carrier
	{
		UINT carrierId;
		double centerFrequency_Hz;
		double bandwidth_Hz;

		UINT frameCount;
		ptrFRAME* frames;
	}CARRIER, * ptrCARRIER;

	typedef enum
	{
		SUPER_FRAME_CONFIG_0,  //!< SUPER_FRAME_CONFIG_0
		SUPER_FRAME_CONFIG_1,  //!< SUPER_FRAME_CONFIG_1
		SUPER_FRAME_CONFIG_2,  //!< SUPER_FRAME_CONFIG_2
		SUPER_FRAME_CONFIG_3,  //!< SUPER_FRAME_CONFIG_3
	} SuperFrameConfiguration_t;

	typedef struct stru_tdma_superframe
	{
		SuperFrameConfiguration_t superFrameConfigType; //???
		UINT superFrameId;
		LINKTYPE linkType;

		double bandwidth_Hz;
		double baseFreqnecy_Hz;
		double centralFrequency_Hz;
		char* band;
		char* accessProtocol;
		double superFrameDuration_us;
		
		ptrSLOTCONF slotConfig;
		ptrCARRIERCONF carrierConfig;
		ptrFRAMECONF frameConfig;

		UINT carrierCount;
		ptrCARRIER* carriers;
		
		double startTime_us;
		double endTime_us;

		UINT bufferCount;
		ptrSATELLITE_BUFFER* buffers;

	}SUPERFRAME, * ptrSUPERFRAME;
	ptrSUPERFRAME superframe_alloc(LINKTYPE linkType,
								   NETSIM_ID d, NETSIM_ID in);
	void superframe_init(NETSIM_ID d, NETSIM_ID in);
	void satellite_superframe_start();
	void satellite_frame_start();
	void satellite_allocate_slot(NETSIM_ID d, NETSIM_ID in,
								 ptrSUPERFRAME sf, ptrFRAME fr);
	ptrSUPERFRAME satellite_get_return_superframe(NETSIM_ID d, NETSIM_ID in);
	ptrSUPERFRAME satellite_get_forward_superframe(NETSIM_ID d, NETSIM_ID in);

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_SATELLITE_FRAME_H_ */
