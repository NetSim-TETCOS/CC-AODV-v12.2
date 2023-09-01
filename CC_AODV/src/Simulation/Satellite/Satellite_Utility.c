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

#pragma region CONFIG_UTILITY
SATELLITE_DEVICETYPE SATELLITE_DEVICETYPE_FROM_STR(const char* type)
{
	for (int i = 0; i < SATELLITE_DEVICETYPE_UNKNOWN; i++)
		if (!_stricmp(type, strSATELLITE_DEVICETYPE[i])) return (SATELLITE_DEVICETYPE)i;
	fnNetSimError("Unknown SATELLITE device type %s in function %s\n",
				  type, __FUNCTION__);
	return SATELLITE_DEVICETYPE_UNKNOWN;
}
#pragma endregion

#pragma region PROTOCOLDATA
static ptrSATELLITE_PROTODATA check_for_data_alloc(NETSIM_ID d,
												   NETSIM_ID in)
{
	if (DEVICE_MACLAYER(d, in) &&
		DEVICE_MACLAYER(d,in)->nMacProtocolId == MAC_PROTOCOL_SATELLITE &&
		DEVICE_MACVAR(d, in))
		return DEVICE_MACVAR(d, in);

	if (DEVICE_PHYLAYER(d, in) &&
		DEVICE_MACLAYER(d,in)->nMacProtocolId == MAC_PROTOCOL_SATELLITE &&
		DEVICE_PHYVAR(d, in))
		return DEVICE_PHYVAR(d, in);

	return NULL;
}

static void set_protocol_data(NETSIM_ID d, NETSIM_ID in, void* data)
{
	if (DEVICE_MACLAYER(d, in))
	{
		DEVICE_MACLAYER(d, in)->nMacProtocolId = MAC_PROTOCOL_SATELLITE;
		DEVICE_MACVAR(d, in) = data;
	}

	if (DEVICE_PHYLAYER(d, in))
	{
		DEVICE_PHYVAR(d, in) = data;
	}
}

ptrSATELLITE_PROTODATA SATELLITE_PROTOCOLDATA_ALLOC(NETSIM_ID d,
													NETSIM_ID in)
{
	ptrSATELLITE_PROTODATA data = check_for_data_alloc(d, in);
	if (data)
	{
		set_protocol_data(d, in, data);
		return data;
	}

	data = calloc(1, sizeof * data);
	set_protocol_data(d, in, data);

	data->deviceId = d;
	data->interfaceId = in;

	data->SATELLITE_LAYER_DATA = calloc(SATELLITE_DEVICETYPE_UNKNOWN, sizeof * data->SATELLITE_LAYER_DATA);
	for (int i = 0; i < SATELLITE_DEVICETYPE_UNKNOWN; i++)
	{
		data->SATELLITE_LAYER_DATA[i] = calloc(SATELLITE_LAYER_UNKNOWN, sizeof * data->SATELLITE_LAYER_DATA[i]);
	}
	return data;
}

void SATELLITE_PROTOCOLDATA_FREE(NETSIM_ID d,
								 NETSIM_ID in)
{
	ptrSATELLITE_PROTODATA data = DEVICE_MACVAR(d, in);
	free(data->SATELLITE_LAYER_DATA);
	free(data);
	DEVICE_MACVAR(d, in) = NULL;
	DEVICE_PHYVAR(d, in) = NULL;
}

void* SATELLITE_LAYER_DATA_GET(NETSIM_ID d, NETSIM_ID in,
							  SATELLITE_DEVICETYPE devType,
							  SATELLITE_LAYER layer)
{
	ptrSATELLITE_PROTODATA data = check_for_data_alloc(d, in);
	if (!data)
	{
		fnNetSimError("Device %d, Interface %d is not an SATELLITE interface\n", d, in);
		return NULL;
	}

	if (data->deviceType != devType)
	{
		fnNetSimError("Device %d, Interface %d is not %s device\n",
					  d, in,
					  strSATELLITE_DEVICETYPE[devType]);
		return NULL;
	}

	if (!data->SATELLITE_LAYER_DATA[devType][layer])
	{
		fnNetSimError("%s sublayer is not initialized for device %d, Interface %d\n",
					  strSATELLITE_LAYER[layer], d, in);
		return NULL;
	}

	return data->SATELLITE_LAYER_DATA[devType][layer];
}

bool SATELLITE_LAYER_DATA_IsInitialized(NETSIM_ID d, NETSIM_ID in,
									   SATELLITE_DEVICETYPE devType,
									   SATELLITE_LAYER layer)
{
	ptrSATELLITE_PROTODATA data = check_for_data_alloc(d, in);
	if (!data)
	{
		fnNetSimError("Device %d, Interface %d is not an SATELLITE interface\n", d, in);
		return false;
	}

	if (data->deviceType != devType)
	{
		fnNetSimError("Device %d, Interface %d is not %s device\n",
					  d, in,
					  strSATELLITE_DEVICETYPE[devType]);
		return false;
	}

	if (!data->SATELLITE_LAYER_DATA[devType][layer])
	{
		return false;
	}

	return true;
}

void SATELLITE_LAYER_DATA_SET(NETSIM_ID d, NETSIM_ID in,
							  SATELLITE_DEVICETYPE devType,
							  SATELLITE_LAYER layer,
							  void* data)
{
	ptrSATELLITE_PROTODATA PD = check_for_data_alloc(d, in);
	if (!PD)
	{
		fnNetSimError("Device %d, Interface %d is not an SATELLITE interface\n", d, in);
		return;
	}

	if (PD->deviceType != devType)
	{
		fnNetSimError("Device %d, Interface %d is not %s device\n",
					  d, in,
					  strSATELLITE_DEVICETYPE[devType]);
		return;
	}

	PD->SATELLITE_LAYER_DATA[devType][layer] = data;
}

SATELLITE_DEVICETYPE SATELLITE_DEVICETYPE_GET(NETSIM_ID d, NETSIM_ID in)
{
	ptrSATELLITE_PROTODATA pd = check_for_data_alloc(d, in);
	if (!pd)
	{
		fnNetSimError("Device %d, Interface %d is not an SATELLITE interface\n", d, in);
		return SATELLITE_DEVICETYPE_UNKNOWN;
	}

	return pd->deviceType;
}
#pragma endregion

#pragma region SATELLITE_LOG
#define MAX_TAB_COUNT 100
static FILE* fplog = NULL;
static char tabs[MAX_TAB_COUNT];
static int tabCount = 0;

static bool get_satellite_log_status()
{
#ifdef SATELLITE_LOG
	return true;
#else
	return false
#endif
}

void init_satellite_log()
{
	if (get_satellite_log_status())
	{
		char str[BUFSIZ];
		sprintf(str, "%s/%s", pszIOLogPath, "satellite.log");
		fplog = fopen(str, "w");
		if (!fplog)
		{
			perror(str);
			fnSystemError("Unable to open satellite.log file");
		}
	}
}

void close_satellite_log()
{
	fclose(fplog);
}

void print_satellite_log(char* format, ...)
{
	if (fplog)
	{
		fprintf(fplog, "%s", tabs);
		va_list l;
		va_start(l, format);
		vfprintf(fplog, format, l);
		fflush(fplog);
	}
}

void satellite_log_add_tab()
{
	tabs[tabCount] = '\t';
	tabCount++;
	if (tabCount > 99) tabCount = 99;
}

void satellite_log_remove_tab()
{
	tabCount--;
	tabs[tabCount] = 0;
	if (tabCount < 0) tabCount = 0;
}
#pragma endregion
