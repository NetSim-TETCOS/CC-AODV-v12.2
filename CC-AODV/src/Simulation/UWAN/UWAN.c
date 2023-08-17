/************************************************************************************
* Copyright (C) 2018                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Shashi Kant Suman                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "main.h"
#include "UWAN.h"
#include "ErrorModel.h"

/**
UWAN Init function initializes the UWAN parameters.
*/
_declspec (dllexport) int fn_NetSim_UWAN_Init(struct stru_NetSim_Network *NETWORK_Formal,
											  NetSim_EVENTDETAILS *pstruEventDetails_Formal,
											  char *pszAppPath_Formal,
											  char *pszWritePath_Formal,
											  int nVersion_Type,
											  void **fnPointer)
{
	// return fn_NetSim_UWAN_Init_F();
}

/**
This function is called by NetworkStack.dll, whenever the event gets triggered
inside the NetworkStack.dll for the UWAN protocol
*/
_declspec (dllexport) int fn_NetSim_UWAN_Run()
{
	return 0;
}

/**
This function is called by NetworkStack.dll, once simulation end to free the
allocated memory for the network.
*/
_declspec(dllexport) int fn_NetSim_UWAN_Finish()
{
	// return fn_NetSim_UWAN_Finish_F();
}

/**
This function is called by NetworkStack.dll, while writing the event trace
to get the sub event as a string.
*/
_declspec (dllexport) char* fn_NetSim_UWAN_Trace(NETSIM_ID nSubEvent)
{
	return "";
}

/**
This function is called by NetworkStack.dll, while configuring the device
for UWAN protocol.
*/
_declspec(dllexport) int fn_NetSim_UWAN_Configure(void** var)
{
	// return fn_NetSim_UWAN_Configure_F(var);
}

/**
This function is called by NetworkStack.dll, to free the UWAN protocol data.
*/
_declspec(dllexport) int fn_NetSim_UWAN_FreePacket(NetSim_PACKET* pstruPacket)
{
	// return fn_NetSim_UWAN_FreePacket_F(pstruPacket);
}

/**
This function is called by NetworkStack.dll, to copy the UWAN protocol
details from source packet to destination.
*/
_declspec(dllexport) int fn_NetSim_UWAN_CopyPacket(NetSim_PACKET* pstruDestPacket, NetSim_PACKET* pstruSrcPacket)
{
	// return fn_NetSim_UWAN_CopyPacket_F(pstruDestPacket, pstruSrcPacket);
}

/**
This function write the Metrics
*/
_declspec(dllexport) int fn_NetSim_UWAN_Metrics(PMETRICSWRITER metricsWriter)
{
	return 0;
}

/**
This function will return the string to write packet trace heading.
*/
_declspec(dllexport) char* fn_NetSim_UWAN_ConfigPacketTrace()
{
	return "";
}

/**
This function will return the string to write packet trace.
*/
_declspec(dllexport) char* fn_NetSim_UWAN_WritePacketTrace(NetSim_PACKET* pstruPacket, char** ppszTrace)
{
	return "";
}


_declspec(dllexport) double UWAN_calculate_propagation_delay(NETSIM_ID tx,
															 NETSIM_ID txi,
															 NETSIM_ID rx,
															 NETSIM_ID rxi,
															 void* propagationHandle)
{
	NetSim_LINKS* link = DEVICE_PHYLAYER(tx, txi)->pstruNetSimLinks;
	if (link->puniMedProp.pstruWirelessLink.propagation->propMedium == PROPMEDIUM_AIR)
	{
		return propagation_air_calculate_propagationDelay(tx, rx, NULL);
	}
	else if (link->puniMedProp.pstruWirelessLink.propagation->propMedium == PROPMEDIUM_ACOUSTICS)
	{
		if (!propagationHandle)
		{
			fnNetSimError("Propagation handle is passed as NULL in function %s\n",
						  __FUNCTION__);
			return 0.0;
		}
		PPROPAGATION_INFO info = get_propagation_info(propagationHandle, tx, txi, rx, rxi);
		return propagation_acoutics_calculate_propagationDelay(tx, rx, &info->propagation);
	}
	else
	{
		fnNetSimError("Unknown propagation medium for link %d\n",
					  link->nConfigLinkId);
		return 0.0;
	}
}

static double UWAN_getConstellationSize(PHY_MODULATION modulation)
{
	switch (modulation)
	{
		case Modulation_8_QAM:
			return 8.0;
		case Modulation_16_QAM:
			return 16.0;
		case Modulation_32_QAM:
			return 32.0;
		case Modulation_64_QAM:
			return 64.0;
		case Modulation_128_QAM:
			return 128.0;
		case Modulation_256_QAM:
			return 256.0;
		default:
			fnNetSimError("Unknown modulation technique %s.\n",
						  strPHY_MODULATION[modulation]);
			return 0.0;
	}
}

static double UWAN_CalculateNoise(PROPAGATION_HANDLE handle,
							 NETSIM_ID tx,
							 NETSIM_ID rx)
{
	PPROPAGATION_INFO info = get_propagation_info(handle, tx, 1, rx, 1);
	return propagation_acoutics_calculate_noise(info->txInfo.dCentralFrequency * 1000,
												info->propagation.acouticsPropVar->dShipping,
												info->propagation.acouticsPropVar->dWindSpeed);
}

static double UWAN_CalculateSNR(double power,
								double noise)
{
	return power - noise;
}
_declspec(dllexport) double UWAN_Calculate_ber(NETSIM_ID tx,
											   NETSIM_ID rx,
											   PROPAGATION_HANDLE handle,
											   double rxPower,
										PHY_MODULATION modulation,
										double dataRate /* In kbps */,
										double bandwidth /* In kHz */)
{
	if (modulation <= Modulation_Zero ||
		modulation >= Modulation_LAST)
	{
		fnNetSimError("Unknown modulation %d.\n", modulation);
		return 0.0;
	}

	NetSim_LINKS* link = DEVICE_PHYLAYER(tx, 1)->pstruNetSimLinks;
	if (link->puniMedProp.pstruWirelessLink.propagation->propMedium == PROPMEDIUM_AIR)
	{
		return calculate_BER(modulation,
							 rxPower,
							 bandwidth/1000);
	}

	double noise = UWAN_CalculateNoise(handle, tx, rx);
	double snr = UWAN_CalculateSNR(rxPower, noise);
	double EbNo = pow(10.0, snr / 10.0);
	double BER = 1.0;
	double PER = 0.0;
	
	switch (modulation)
	{
		case Modulation_BPSK:
			BER = 0.5 * erfc(sqrt(EbNo));
			break;
		case Modulation_QPSK:
			BER = 0.5 * erfc(sqrt(0.5 * EbNo));
			break;
		case Modulation_16_QAM:
		case Modulation_32_QAM:
		case Modulation_64_QAM:
		case Modulation_128_QAM:
		case Modulation_256_QAM:
		{
			// taken from Ronell B. Sicat, "Bit Error Probability Computations for M-ary Quadrature Amplitude Modulation",
			// EE 242 Digital Communications and Codings, 2009
			// generic EbNo
			EbNo *= (dataRate * 1000) / (bandwidth * 1000);

			double M = UWAN_getConstellationSize(modulation);

			// standard squared quantized QAM, even number of bits per symbol supported
			int log2sqrtM = (int)log2(sqrt(M));

			double log2M = log2(M);

			if ((int)log2M % 2)
			{
				fnNetSimError("constellation %d is not supported for QAM.", (int)M);
				return 0.0;
			}

			double sqrtM = sqrt(M);

			BER = 0.0;

			// Eq (75)
			for (int k = 0; k < log2sqrtM; k++)
			{
				int sum_items = (int)((1.0 - pow(2.0, (-1.0) * (double)k)) * sqrt(M) - 1.0);
				double pow2k = pow(2.0, (double)k - 1.0);

				double PbK = 0;

				// Eq (74)
				for (int j = 0; j < sum_items; ++j)
				{
					PbK += pow(-1.0, (double)j * pow2k / sqrtM)
						* (pow2k - floor((double)(j * pow2k / sqrtM) - 0.5))
						* erfc((2.0 * (double)j + 1.0) * sqrt(3.0 * (log2M * EbNo) / (2.0 * (M - 1.0))));

				}
				PbK *= 1.0 / sqrtM;

				BER += PbK;
			}

			BER *= 1.0 / (double)log2sqrtM;
		}
		break;
		case Modulation_FSK:
			BER = 0.5 * erfc(sqrt(0.5 * EbNo));
			break;
		default:
			fnNetSimError("Modulation %s is not supported.",
						  strPHY_MODULATION[modulation]);
			break;
	}
	return BER;
}
