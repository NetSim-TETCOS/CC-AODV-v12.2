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
#include "SATELLITE.h"
#include "Satellite_MAC.h"
#include "Satellite_PHY.h"
#include "Satellite_Frame.h"

#pragma region SATELLITE_DEFAULT_CONFIG
//Default config parameter
#define SATELLITE_DEVICE_TYPE_DEFAULT	_strdup("USER_TERMINAL")
#define SATELLITE_GATEWAY_DEFAULT		NULL
#define SATELLITE_BUFFER_SIZE_DEFAULT	(1024*1024*8)	//1 MB

//Carrier config
#define SATELLITE_ROLLOFF_FACTOR_DEFAULT		1
#define SATELLITE_SPACING_FACTOR_DEFAULT		1
#define SATELLITE_CARRIER_BANDWIDTH_HZ_DEFAULT	1000000
#define SATELLITE_BER_MODEL_DEFAULT				_strdup("FIXED")
#define SATELLITE_BER_DEFAULT					1e-7
#define SATELLITE_BER_FILE_DEFAULT				_strdup("")

//Slot config
#define SATELLITE_CODING_RATE_DEFAULT					_strdup("1/2")
#define SATELLITE_MODULATION_DEFAULT					_strdup("QPSK")
#define SATELLITE_SYMBOL_PER_SLOT_DEFAULT				90

//Frame config
#define SATELLITE_SLOT_COUNT_IN_FRAME_DEFAULT			10
#define SATELLITE_FRAME_BANDWIDTH_HZ_DEFAULT			1000000
#define SATELLITE_PILOT_BLOCK_SIZE_symbols_DEFAULT		36
#define SATELLITE_PILOT_BLOCK_INTERVAL_slots_DEFAULT	16
#define SATELLITE_PLHEARDER_slots_DEFAULT				1
#define SATELLITE_BB_FRAME_HEADER_LEN_bytes_DEFAULT		1
#define SATELLITE_BB_FRAME_USAGE_MODE_DEFAULT			_strdup("Normal")
#define SATELLITE_FRAME_COUNT_IN_SUPERFRAME_DEFAULT		10

//Superframe config
#define SATELLITE_FRAME_COUNT_IN_SUPERFRAME_DEFAULT		10
#define SATELLITE_BAND_DEFAULT							_strdup("KA")
#define SATELLITE_ACCESS_PROTOCOL_DEFAULT				_strdup("TDMA")
#define SATELLITE_BASE_FREQUENCY_HZ_DEFAULT				12000000000

//PHY config
#define SATELLITE_TX_ANTENNA_GAIN_DB_DEFAULT			48
#define SATELLITE_RX_ANTENNA_GAIN_DB_DEFAULT			48
#define SATELLITE_TX_POWER_DBM_DEFAULT					-23
#define SATELLITE_ANTENNA_GAIN_TO_NOISE_TEMPERATURE_DBK_DEFAULT		28.4
#pragma endregion

#pragma region SATELLITE_MAC_CONFIG

static void SATELLITE_ConfigureSatelliteMacLayer(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	ptrSATELLITE_MAC mac = satellite_mac_alloc(pd);
}

static void SATELLITE_ConfigureGatewayMacLayer(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	ptrSATELLITE_GW_MAC mac = satellite_gw_mac_alloc(pd);
	mac->bufferSize_bits = (UINT)fn_NetSim_Config_read_dataLen(xmlNetSimNode, "Buffer_Size",
															   SATELLITE_BUFFER_SIZE_DEFAULT, "bits");
}

static void SATELLITE_ConfigureUTMacLayer(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	ptrSATELLITE_UT_MAC mac = satellite_ut_mac_alloc(pd);
	getXmlVar(&mac->szGatewayName, GATEWAY, xmlNetSimNode, 1, _STRING, SATELLITE);
	mac->bufferSize_bits = (UINT)fn_NetSim_Config_read_dataLen(xmlNetSimNode, "Buffer_Size",
															   SATELLITE_BUFFER_SIZE_DEFAULT, "bits");
}

static void SATELLITE_ConfigureMacLayer(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	if (pd->deviceType == SATELLITE_DEVICETYPE_SATELLITE)
		SATELLITE_ConfigureSatelliteMacLayer(pd, xmlNetSimNode);
	else if(pd->deviceType == SATELLITE_DEVICETYPE_SATELLITE_GATEWAY)
		SATELLITE_ConfigureGatewayMacLayer(pd, xmlNetSimNode);
	else if(pd->deviceType == SATELLITE_DEVICETYPE_USER_TERMINAL)
		SATELLITE_ConfigureUTMacLayer(pd, xmlNetSimNode);
	else
	{
		fnNetSimError("Unknown satellite device type %d in function %s\n",
					  pd->deviceType, __FUNCTION__);
	}
}

#pragma endregion

#pragma region SATELLITE_PHY_CONFIG
static BERMODEL get_ber_model_from_str(char* s)
{
	if (!_stricmp(s, "Fixed")) return BERMODEL_Fixed;
	else if (!_stricmp(s, "FILE_BASED")) return BERMODEL_File;
	else if (!_stricmp(s, "Model_based")) return BERMODEL_Model;
	else
	{
		fnNetSimError("Unknown BER model %s.\n", s);
		return BERMODEL_Fixed;
	}
}

static double getCodingRate(char* s)
{
	if (!_stricmp(s, "1/3")) return 1.0 / 3.0;
	else if (!_stricmp(s, "1/2")) return 1.0 / 2.0;
	else if (!_stricmp(s, "3/5")) return 3.0 / 5.0;
	else if (!_stricmp(s, "2/3")) return 2.0 / 3.0;
	else if (!_stricmp(s, "3/4")) return 3.0 / 4.0;
	else if (!_stricmp(s, "4/5")) return 4.0 / 5.0;
	else if (!_stricmp(s, "5/6")) return 5.0 / 6.0;
	else if (!_stricmp(s, "8/9")) return 8.0 / 9.0;
	else if (!_stricmp(s, "9/10")) return 9.0 / 10.0;
	else return 0.0;
}

static void satellite_configure_carrier(ptrCARRIERCONF cf, void* xmlNetSimNode)
{
	char* szVal;
	getXmlVar(&cf->rollOffFactor, ROLLOFF_FACTOR, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	if (cf->rollOffFactor > 1.0 || cf->rollOffFactor < 0.0)
	{
		fnNetSimError("Roll factor must be [0.0,1.0]\n");
		cf->rollOffFactor = 1.0;
	}

	getXmlVar(&cf->spacingFactor, SPACING_FACTOR, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	if (cf->spacingFactor > 1.0 || cf->spacingFactor < 0.0)
	{
		fnNetSimError("Spacing factor must be [0.0,1.0]\n");
		cf->spacingFactor = 1.0;
	}

	cf->allocatedBandwidth_Hz = fn_NetSim_Config_read_Frequency(xmlNetSimNode, "CARRIER_BANDWIDTH",
																SATELLITE_CARRIER_BANDWIDTH_HZ_DEFAULT, "Hz");
	cf->occupiedBandwidth_Hz = cf->allocatedBandwidth_Hz / (cf->rollOffFactor + 1.0);
	cf->effectiveBandwidth_Hz = cf->allocatedBandwidth_Hz / ((cf->rollOffFactor + 1.0) * (cf->spacingFactor + 1.0));

	getXmlVar(&szVal, BER_MODEL, xmlNetSimNode, 1, _STRING, SATELLITE);
	cf->berModel = get_ber_model_from_str(szVal);
	free(szVal);

	if (cf->berModel == BERMODEL_Fixed)
	{
		getXmlVar(&cf->berValue, BER, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	}
	else if (cf->berModel == BERMODEL_File)
	{
		getXmlVar(&cf->berFile, BER_FILE, xmlNetSimNode, 1, _STRING, SATELLITE);
		cf->berTable = read_ber_file(cf->berFile, &cf->berTableLen);
	}

	getXmlVar(&szVal, CODING_RATE, xmlNetSimNode, 1, _STRING, SATELLITE);
	cf->codingRate = getCodingRate(szVal);
	free(szVal);

	getXmlVar(&szVal, MODULATION, xmlNetSimNode, 1, _STRING, SATELLITE);
	cf->modulation = getModulationFromStr(szVal);
	free(szVal);
}

static void satellite_configure_frame(ptrFRAMECONF frConf, void* xmlNetSimNode)
{
	char* szVal;
	getXmlVar(&frConf->slotCountInFrame, SLOT_COUNT_IN_FRAME, xmlNetSimNode, 1, _UINT, SATELLITE);
	frConf->frameBandwidth_Hz = fn_NetSim_Config_read_Frequency(xmlNetSimNode, "FRAME_BANDWIDTH",
																SATELLITE_FRAME_BANDWIDTH_HZ_DEFAULT, "Hz");
	getXmlVar(&frConf->pilotBlockSize_inSymbols, PILOT_BLOCK_SIZE_symbols, xmlNetSimNode, 1, _UINT, SATELLITE);
	getXmlVar(&frConf->pilotBlockInterval_inSlots, PILOT_BLOCK_INTERVAL_slots, xmlNetSimNode, 1, _UINT, SATELLITE);
	getXmlVar(&frConf->plHdrSize_inSlots, PLHEARDER_slots, xmlNetSimNode, 1, _UINT, SATELLITE);
	getXmlVar(&frConf->frameHdrLength_inBytes, BB_FRAME_HEADER_LEN_bytes, xmlNetSimNode, 1, _UINT, SATELLITE);
	getXmlVar(&szVal, BB_FRAME_USAGE_MODE, xmlNetSimNode, 1, _STRING, SATELLITE);
	if (!_stricmp(szVal, "Normal")) frConf->usageMode = FRUSAGE_NORMAL;
	else frConf->usageMode = FRUSAGE_SHORT;
	free(szVal);

	getXmlVar(&frConf->framesPerSuperframe, FRAME_COUNT_IN_SUPERFRAME, xmlNetSimNode, 1, _UINT, SATELLITE);
}

static void satellite_configure_slot(ptrSLOTCONF sc, void* xmlNetSimNode)
{
	getXmlVar(&sc->symbolPerSlot, SYMBOL_PER_SLOT, xmlNetSimNode, 1, _UINT, SATELLITE);
}

static void satellite_configure_forward_link(ptrSATELLITE_PHY phy, void* xmlNetSimNode)
{
	ptrSUPERFRAME sf = superframe_alloc(LINKTYPE_FORWARD, phy->d, phy->in);
	
	satellite_configure_carrier(sf->carrierConfig, xmlNetSimNode);
	
	satellite_configure_frame(sf->frameConfig, xmlNetSimNode);

	satellite_configure_slot(sf->slotConfig, xmlNetSimNode);

	sf->baseFreqnecy_Hz = fn_NetSim_Config_read_Frequency(xmlNetSimNode, "BASE_FREQUENCY", SATELLITE_BASE_FREQUENCY_HZ_DEFAULT, "HZ");
	getXmlVar(&sf->accessProtocol, ACCESS_PROTOCOL, xmlNetSimNode, 1, _STRING, SATELLITE);
	getXmlVar(&sf->band, BAND, xmlNetSimNode, 1, _STRING, SATELLITE);
}

static void satellite_configure_return_link(ptrSATELLITE_PHY phy, void* xmlNetSimNode)
{
	ptrSUPERFRAME sf = superframe_alloc(LINKTYPE_RETURN, phy->d, phy->in);

	satellite_configure_carrier(sf->carrierConfig, xmlNetSimNode);

	satellite_configure_frame(sf->frameConfig, xmlNetSimNode);

	satellite_configure_slot(sf->slotConfig, xmlNetSimNode);

	sf->baseFreqnecy_Hz = fn_NetSim_Config_read_Frequency(xmlNetSimNode, "BASE_FREQUENCY", SATELLITE_BASE_FREQUENCY_HZ_DEFAULT, "HZ");
	getXmlVar(&sf->accessProtocol, ACCESS_PROTOCOL, xmlNetSimNode, 1, _STRING, SATELLITE);
	getXmlVar(&sf->band, BAND, xmlNetSimNode, 1, _STRING, SATELLITE);
}

static void satellite_configure_phy(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	ptrSATELLITE_PHY phy = satellite_phy_alloc(pd->deviceId, pd->interfaceId);

	void* xmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "FORWARD", 0);
	satellite_configure_forward_link(phy, xmlChild);

	xmlChild = fn_NetSim_xmlGetChildElement(xmlNetSimNode, "RETURN", 0);
	satellite_configure_return_link(phy, xmlChild);

	phy->txPower_dbm = fn_NetSim_Config_read_txPower(xmlNetSimNode, "TX_POWER", SATELLITE_TX_POWER_DBM_DEFAULT, "dBm");
	phy->txPower_mw = DBM_TO_MW(phy->txPower_dbm);

	getXmlVar(&phy->txAntennaGain_db, TX_ANTENNA_GAIN_DB, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	getXmlVar(&phy->rxAntennaGain_db, RX_ANTENNA_GAIN_DB, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	getXmlVar(&phy->antennaGainToNoiseTemp_dBk, ANTENNA_GAIN_TO_NOISE_TEMPERATURE_DBK, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	phy->tempeature_k = DBM_TO_MW(phy->antennaGainToNoiseTemp_dBk);
}

static void satellite_configure_gw_phy(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	ptrSATELLITE_GW_PHY phy = satellite_gw_phy_alloc(pd->deviceId, pd->interfaceId);

	getXmlVar(&phy->txAntennaGain_db, TX_ANTENNA_GAIN_DB, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	getXmlVar(&phy->rxAntennaGain_db, RX_ANTENNA_GAIN_DB, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	getXmlVar(&phy->antennaGainToNoiseTemp_dBk, ANTENNA_GAIN_TO_NOISE_TEMPERATURE_DBK, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	phy->tempeature_k = DBM_TO_MW(phy->antennaGainToNoiseTemp_dBk);

	phy->txPower_dbm = fn_NetSim_Config_read_txPower(xmlNetSimNode, "TX_POWER", SATELLITE_TX_POWER_DBM_DEFAULT, "dBm");
	phy->txPower_mw = DBM_TO_MW(phy->txPower_dbm);
}

static void satellite_configure_ut_phy(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	ptrSATELLITE_UT_PHY phy = satellite_ut_phy_alloc(pd->deviceId, pd->interfaceId);

	getXmlVar(&phy->txAntennaGain_db, TX_ANTENNA_GAIN_DB, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	getXmlVar(&phy->rxAntennaGain_db, RX_ANTENNA_GAIN_DB, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	getXmlVar(&phy->antennaGainToNoiseTemp_dBk, ANTENNA_GAIN_TO_NOISE_TEMPERATURE_DBK, xmlNetSimNode, 1, _DOUBLE, SATELLITE);
	phy->tempeature_k = DBM_TO_MW(phy->antennaGainToNoiseTemp_dBk);

	phy->txPower_dbm = fn_NetSim_Config_read_txPower(xmlNetSimNode, "TX_POWER", SATELLITE_TX_POWER_DBM_DEFAULT, "dBm");
	phy->txPower_mw = DBM_TO_MW(phy->txPower_dbm);
}

static void SATELLITE_ConfigurePhyLayer(ptrSATELLITE_PROTODATA pd, void* xmlNetSimNode)
{
	switch (pd->deviceType)
	{
		case SATELLITE_DEVICETYPE_SATELLITE:
			satellite_configure_phy(pd, xmlNetSimNode);
			break;
		case SATELLITE_DEVICETYPE_SATELLITE_GATEWAY:
			satellite_configure_gw_phy(pd, xmlNetSimNode);
			break;
		case SATELLITE_DEVICETYPE_USER_TERMINAL:
			satellite_configure_ut_phy(pd, xmlNetSimNode);
			break;
		default:
			fnNetSimError("Unknown device type %d in function %s\n",
						  pd->deviceType, __FUNCTION__);
			break;
	}
}
#pragma endregion


void fn_NetSim_SATELLITE_Configure_F(void** var)
{
	char* szVal;
	char* tag;
	void* xmlNetSimNode;
	NETSIM_ID nDeviceId;
	NETSIM_ID nInterfaceId;
	LAYER_TYPE nLayerType;

	tag = (char*)var[0];
	xmlNetSimNode = var[2];
	if (!strcmp(tag, "PROTOCOL_PROPERTY"))
	{
		NETWORK = (struct stru_NetSim_Network*)var[1];
		nDeviceId = *((NETSIM_ID*)var[3]);
		nInterfaceId = *((NETSIM_ID*)var[4]);
		nLayerType = *((LAYER_TYPE*)var[5]);

		ptrSATELLITE_PROTODATA pd = SATELLITE_PROTOCOLDATA_ALLOC(nDeviceId, nInterfaceId);

		getXmlVar(&szVal, DEVICE_TYPE, xmlNetSimNode, 1, _STRING, SATELLITE);
		pd->deviceType = SATELLITE_DEVICETYPE_FROM_STR(szVal);
		free(szVal);

		if (nLayerType == MAC_LAYER)
			SATELLITE_ConfigureMacLayer(pd, xmlNetSimNode);
		else if (nLayerType == PHYSICAL_LAYER)
			SATELLITE_ConfigurePhyLayer(pd, xmlNetSimNode);
	}
}
