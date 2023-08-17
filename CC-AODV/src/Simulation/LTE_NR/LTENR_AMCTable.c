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
#include "LTENR_PHY.h"
#include "LTENR_Spectrum.h"
#include "LTENR_AMCTable.h"

#pragma region LTENR_CQITABLE
static ptrLTENR_CQITable LTENR_GetCQITableFromStr(char* table)
{
	if (!_stricmp(table, "TABLE1"))
		return CQITable1;
	else if (!_stricmp(table, "TABLE2"))
		return CQITable2;
	else if (!_stricmp(table, "TABLE3"))
		return CQITable3;
	else
	{
		fnNetSimError("Unknown CQI table %s\n",
					  table);
		return NULL;
	}
}

void LTENR_SetCQITable(ptrLTENR_GNBPHY phy, char* table)
{
	phy->CSIReportConfig.cqiTable = LTENR_GetCQITableFromStr(table);
}

static ptrLTENR_CQITable LTENR_GetCQITable(ptrLTENR_GNBPHY phy)
{
	return phy->CSIReportConfig.cqiTable;
}

LTENR_CQITable LTENR_GetCQITableFromSpectralEfficiency(ptrLTENR_CQITable table, double efficiency)
{
	int i;
	if (efficiency < 0.0)
	{
		fnNetSimError("Spectral efficiency %ld is less than 0.0\n", efficiency);
		return table[0];
	}
	for (i = 1; i <= LTENR_MAX_CQI; i++)
	{
		if (table[i].efficiency >= efficiency)
			return table[i];
	}
	return table[LTENR_MAX_CQI];
}
#pragma endregion

#pragma region LTENR_MCSTABLE
static ptrLTENR_MCSINDEXTABLE LTENR_PDSCH_GetMCSIndexTableFromStr(char* table)
{
	if (!_stricmp(table, "QAM64"))
		return PDSCHMCSIndexTable1;
	else if (!_stricmp(table, "QAM256"))
		return PDSCHMCSIndexTable2;
	else if (!_stricmp(table, "QAM64LOWSE"))
		return PDSCHMCSIndexTable3;
	else
	{
		fnNetSimError("Unknown MCS table %s.\n", table);
		return PDSCHMCSIndexTable1;
	}
}

void LTENR_SetPDSCHMCSIndexTable(ptrLTENR_GNBPHY phy, char* table)
{
	phy->PDSCHConfig.mcsTable = LTENR_PDSCH_GetMCSIndexTableFromStr(table);
}

static ptrLTENR_MCSINDEXTABLE LTENR_PUSCH_GetMCSIndexTableFromStr(char* table, bool isTransformPrecoding)
{
	if (!isTransformPrecoding)
		return LTENR_PDSCH_GetMCSIndexTableFromStr(table);

	if (!_stricmp(table, "QAM64"))
		return PUSCHMCSIndexTable1;
	else if (!_stricmp(table, "QAM256"))
		return PDSCHMCSIndexTable2;
	else if (!_stricmp(table, "QAM64LOWSE"))
		return PUSCHMCSIndexTable2;
	else
	{
		fnNetSimError("Unknown MCS table %s.\n", table);
		return PUSCHMCSIndexTable1;
	}
}

void LTENR_SetPUSCHMCSIndexTable(ptrLTENR_GNBPHY phy, char* table)
{
	phy->PUSCHConfig.mcsTable = LTENR_PUSCH_GetMCSIndexTableFromStr(table, phy->PUSCHConfig.isTransformPrecoding);
}

LTENR_MCSINDEXTABLE LTENR_GetMCSIndexTableFromSpectralEfficiency(ptrLTENR_MCSINDEXTABLE table, double efficiency)
{
	int i;
	if (efficiency < 0.0)
	{
		fnNetSimError("Spectral efficiency %ld is less than 0.0\n", efficiency);
		return table[0];
	}
	for (i = 0; i < LTENR_MAX_MCS; i++)
	{
		if (table[i].modulation == Modulation_Zero)
			return table[i - 1];

		if (table[i].spectralEfficiency >= efficiency)
			return table[i];
	}
	return table[i-1];
}
#pragma endregion

static UINT64 LTENR_calculateTBSSize_step3(double Ninfo)
{
	UINT n = max(3, (int)floor(log2(Ninfo)) - 6);
	UINT64 pow2n = (UINT64)pow(2, n);
	double Ninfo_ = max(24, pow2n * floor((Ninfo / pow2n)));
	int i;
	int len = sizeof TBSTableForNinfoLess3824 / sizeof TBSTableForNinfoLess3824[0];
	for (i = 0; i < len; i++)
	{
		if (TBSTableForNinfoLess3824[i].TBS >= Ninfo_)
			return TBSTableForNinfoLess3824[i].TBS;
	}
	return (UINT64)TBSTableForNinfoLess3824[len - 1].TBS;
}

static UINT64 LTENR_calculateTBSSize_step4(double Ninfo, double R)
{
	int n = (int)floor(log2(Ninfo - 24)) - 5;
	UINT64 pow2n = (UINT64)pow(2, n);
	double a = ceil((Ninfo - 24) / pow2n);
	double Ninfo_ = max(3840, pow2n * a);

	UINT64 TBS;
	if (R <= 0.25)
	{
		UINT64 C = (UINT64)ceil((Ninfo_ + 24) / 3816);
		TBS = (UINT64)(8 * C * (UINT64)ceil(((Ninfo_ + 24) / (8 * C))) - 24);
	}
	else
	{
		if (Ninfo_ > 8424)
		{
			UINT64 C = (int)ceil((Ninfo_ + 24) / 8424);
			TBS = (UINT64)(8 * C * (UINT64)ceil(((Ninfo_ + 24) / (8 * C))) - 24);
		}
		else
		{
			TBS = (UINT64)(8 * (UINT64)ceil((Ninfo_ + 24) / 8)) - 24;
		}
	}
	return TBS;
}

UINT64 LTENR_calculateTBSSize(ptrLTENR_GNBPHY phy,
							  UINT allocatedPRB, bool isUplink,
							  LTENR_MCSINDEXTABLE mcsTable, int CA_ID)
{
	ptrLTENR_CA ca = phy->spectrumConfig->CA[CA_ID];
	ptrLTENR_SPECTRUMCONFIG sc = phy->spectrumConfig;
	UINT Nre_ = sc->numSubcarrierPerPRB * ca->symbolPerSlot - NUM_OF_RE_IN_DMRS - phy->PDSCHConfig.xOverhead;
	UINT Nre = min(156, Nre_) * allocatedPRB;
	UINT layerLen = isUplink ? sc->upLinkMimoLayerCount : sc->downLinkMimoLayerCount;
	double R = mcsTable.codeRate / 1024.0;
	double Ninfo = Nre * R * mcsTable.modulationOrder * layerLen;
	if (Ninfo <= 3824)
		return LTENR_calculateTBSSize_step3(Ninfo);
	else
		return LTENR_calculateTBSSize_step4(Ninfo, R);
}
