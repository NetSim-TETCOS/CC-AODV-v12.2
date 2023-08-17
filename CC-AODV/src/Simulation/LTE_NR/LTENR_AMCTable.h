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
#ifndef _NETSIM_LTENR_AMCTABLE_H_
#define _NETSIM_LTENR_AMCTABLE_H_
#ifdef  __cplusplus
extern "C" {
#endif
#include "LTENR_Spectrum.h"

	//3GPP 38.214
	//Table 5.2.2.1-2: 4-bit CQI Table 1
	static LTENR_CQITable CQITable1[] =
	{
		{0,		Modulation_Zero,	0,		0}, //out of range
		{1,		Modulation_QPSK,	78,		0.1523},
		{2,		Modulation_QPSK,	120,	0.2344},
		{3,		Modulation_QPSK,	193,	0.3770},
		{4,		Modulation_QPSK,	308,	0.6016},
		{5,		Modulation_QPSK,	449,	0.8770},
		{6,		Modulation_QPSK,	602,	1.1758},
		{7,		Modulation_16_QAM,	378,	1.4766},
		{8,		Modulation_16_QAM,	490,	1.9141},
		{9,		Modulation_16_QAM,	616,	2.4063},
		{10,	Modulation_64_QAM,	466,	2.7305},
		{11,	Modulation_64_QAM,	567,	3.3223},
		{12,	Modulation_64_QAM,	666,	3.9023},
		{13,	Modulation_64_QAM,	772,	4.5234},
		{14,	Modulation_64_QAM,	873,	5.1152},
		{15,	Modulation_64_QAM,	948,	5.5547},
	};

	//3GPP 38.214
	//Table 5.2.2.1-3: 4-bit CQI Table 2
	static LTENR_CQITable CQITable2[] =
	{
		{0,		Modulation_Zero,	0,		0}, //out of range
		{1,		Modulation_QPSK,	78,		0.1523},
		{2,		Modulation_QPSK,	193,	0.3770},
		{3,		Modulation_QPSK,	449,	0.8770},
		{4,		Modulation_16_QAM,	378,	1.4766},
		{5,		Modulation_16_QAM,	490,	1.9141},
		{6,		Modulation_16_QAM,	616,	2.4063},
		{7,		Modulation_64_QAM,	466,	2.7305},
		{8,		Modulation_64_QAM,	567,	3.3223},
		{9,		Modulation_64_QAM,	666,	3.9023},
		{10,	Modulation_64_QAM,	772,	4.5234},
		{11,	Modulation_64_QAM,	873,	5.1152},
		{12,	Modulation_256_QAM,	711,	5.5547},
		{13,	Modulation_256_QAM,	797,	6.2266},
		{14,	Modulation_256_QAM,	885,	6.9141},
		{15,	Modulation_256_QAM,	948,	7.4063},
	};

	//3GPP 38.214
	//Table 5.2.2.1-4: 4-bit CQI Table 3
	static LTENR_CQITable CQITable3[] =
	{
		{0,		Modulation_Zero,	0,		0}, //out of range
		{1,		Modulation_QPSK,	30,		0.0586},
		{2,		Modulation_QPSK,	50,		0.0977},
		{3,		Modulation_QPSK,	78,		0.1523},
		{4,		Modulation_QPSK,	120,	0.2344},
		{5,		Modulation_QPSK,	193,	0.3770},
		{6,		Modulation_QPSK,	308,	0.6016},
		{7,		Modulation_QPSK,	449,	0.8770},
		{8,		Modulation_QPSK,	602,	1.1758},
		{9,		Modulation_16_QAM,	378,	1.4766},
		{10,	Modulation_16_QAM,	490,	1.9141},
		{11,	Modulation_16_QAM,	616,	2.4063},
		{12,	Modulation_64_QAM,	466,	2.7305},
		{13,	Modulation_64_QAM,	567,	3.3223},
		{14,	Modulation_64_QAM,	666,	3.9023},
		{15,	Modulation_64_QAM,	772,	4.5234},
	};


	//3GPPP 38.214
	//Table 5.1.3.1-1: MCS index table 1 for PDSCH
	static LTENR_MCSINDEXTABLE PDSCHMCSIndexTable1[] =
	{
		{0,		2,	Modulation_QPSK,	120,	0.2344},
		{1,		2,	Modulation_QPSK,	157,	0.3066},
		{2,		2,	Modulation_QPSK,	193,	0.3770},
		{3,		2,	Modulation_QPSK,	251,	0.4902},
		{4,		2,	Modulation_QPSK,	308,	0.6016},
		{5,		2,	Modulation_QPSK,	379,	0.7402},
		{6,		2,	Modulation_QPSK,	449,	0.8770},
		{7,		2,	Modulation_QPSK,	526,	1.0273},
		{8,		2,	Modulation_QPSK,	602,	1.1758},
		{9,		2,	Modulation_QPSK,	679,	1.3262},
		{10,	4,	Modulation_16_QAM,	340,	1.3281},
		{11,	4,	Modulation_16_QAM,	378,	1.4766},
		{12,	4,	Modulation_16_QAM,	434,	1.6953},
		{13,	4,	Modulation_16_QAM,	490,	1.9141},
		{14,	4,	Modulation_16_QAM,	553,	2.1602},
		{15,	4,	Modulation_16_QAM,	616,	2.4063},
		{16,	4,	Modulation_64_QAM,	658,	2.5703},
		{17,	6,	Modulation_64_QAM,	438,	2.5664},
		{18,	6,	Modulation_64_QAM,	466,	2.7305},
		{19,	6,	Modulation_64_QAM,	517,	3.0293},
		{20,	6,	Modulation_64_QAM,	567,	3.3223},
		{21,	6,	Modulation_64_QAM,	616,	3.6094},
		{22,	6,	Modulation_64_QAM,	666,	3.9023},
		{23,	6,	Modulation_64_QAM,	719,	4.2129},
		{24,	6,	Modulation_64_QAM,	772,	4.5234},
		{25,	6,	Modulation_64_QAM,	822,	4.8164},
		{26,	6,	Modulation_64_QAM,	873,	5.1152},
		{27,	6,	Modulation_64_QAM,	910,	5.3320},
		{28,	6,	Modulation_64_QAM,	948,	5.5547},
		{29,	2,	Modulation_Zero,	0,		0.0}, //reserved
		{30,	4,	Modulation_Zero,	0,		0.0}, //reserved
		{31,	6,	Modulation_Zero,	0,		0.0}, //reserved
	};

	//3GPPP 38.214
	//Table 5.1.3.1-2: MCS index table 2 for PDSCH
	static LTENR_MCSINDEXTABLE PDSCHMCSIndexTable2[] =
	{
		{0,		2,	Modulation_QPSK,	120,	0.2344},
		{1,		2,	Modulation_QPSK,	193,	0.3770},
		{2,		2,	Modulation_QPSK,	308,	0.6016},
		{3,		2,	Modulation_QPSK,	449,	0.8770},
		{4,		2,	Modulation_QPSK,	602,	1.1758},
		{5,		4,	Modulation_16_QAM,	378,	1.4766},
		{6,		4,	Modulation_16_QAM,	434,	1.6953},
		{7,		4,	Modulation_16_QAM,	490,	1.9141},
		{8,		4,	Modulation_16_QAM,	553,	2.1602},
		{9,		4,	Modulation_16_QAM,	616,	2.4063},
		{10,	4,	Modulation_16_QAM,	658,	2.5703},
		{11,	6,	Modulation_64_QAM,	466,	2.7305},
		{12,	6,	Modulation_64_QAM,	517,	3.0293},
		{13,	6,	Modulation_64_QAM,	567,	3.3223},
		{14,	6,	Modulation_64_QAM,	616,	3.6094},
		{15,	6,	Modulation_64_QAM,	666,	3.9023},
		{16,	6,	Modulation_64_QAM,	719,	4.2129},
		{17,	6,	Modulation_64_QAM,	772,	4.5234},
		{18,	6,	Modulation_64_QAM,	822,	4.8164},
		{19,	6,	Modulation_64_QAM,	873,	5.1152},
		{20,	8,	Modulation_256_QAM,	682.5,	5.3320},
		{21,	8,	Modulation_256_QAM,	711,	5.5547},
		{22,	8,	Modulation_256_QAM,	754,	5.8906},
		{23,	8,	Modulation_256_QAM,	797,	6.2266},
		{24,	8,	Modulation_256_QAM,	841,	6.5703},
		{25,	8,	Modulation_256_QAM,	885,	6.9141},
		{26,	8,	Modulation_256_QAM,	916.5,	7.1602},
		{27,	8,	Modulation_256_QAM,	948,	7.4063},
		{28,	2,	Modulation_Zero,	0,		0.0}, //reserved
		{29,	4,	Modulation_Zero,	0,		0.0}, //reserved
		{30,	6,	Modulation_Zero,	0,		0.0}, //reserved
		{31,	8,	Modulation_Zero,	0,		0.0}, //reserved
	};

	//3GPPP 38.214
	//Table 5.1.3.1-3: MCS index table 3 for PDSCH
	static LTENR_MCSINDEXTABLE PDSCHMCSIndexTable3[] =
	{
		{0,		2,	Modulation_QPSK,	30,		0.0586},
		{1,		2,	Modulation_QPSK,	40,		0.0781},
		{2,		2,	Modulation_QPSK,	50,		0.0977},
		{3,		2,	Modulation_QPSK,	64,		0.1250},
		{4,		2,	Modulation_QPSK,	78,		0.1523},
		{5,		2,	Modulation_QPSK,	99,		0.1934},
		{6,		2,	Modulation_QPSK,	120,	0.2344},
		{7,		2,	Modulation_QPSK,	157,	0.3066},
		{8,		2,	Modulation_QPSK,	193,	0.3770},
		{9,		2,	Modulation_QPSK,	251,	0.4902},
		{10,	2,	Modulation_QPSK,	308,	0.6016},
		{11,	2,	Modulation_QPSK,	379,	0.7402},
		{12,	2,	Modulation_QPSK,	449,	0.8770},
		{13,	2,	Modulation_QPSK,	526,	1.0273},
		{14,	2,	Modulation_QPSK,	602,	1.1758},
		{15,	4,	Modulation_16_QAM,	340,	1.3281},
		{16,	4,	Modulation_16_QAM,	378,	1.4766},
		{17,	4,	Modulation_16_QAM,	434,	1.6953},
		{18,	4,	Modulation_16_QAM,	490,	1.9141},
		{19,	4,	Modulation_16_QAM,	553,	2.1602},
		{20,	4,	Modulation_16_QAM,	616,	2.4063},
		{21,	6,	Modulation_64_QAM,	438,	2.5664},
		{22,	6,	Modulation_64_QAM,	466,	2.7305},
		{23,	6,	Modulation_64_QAM,	517,	3.0293},
		{24,	6,	Modulation_64_QAM,	567,	3.3223},
		{25,	6,	Modulation_64_QAM,	616,	3.6094},
		{26,	6,	Modulation_64_QAM,	666,	3.9023},
		{27,	6,	Modulation_64_QAM,	719,	4.2129},
		{28,	6,	Modulation_64_QAM,	772,	4.5234},
		{29,	2,	Modulation_Zero,	0,		0.0}, //reserved
		{30,	4,	Modulation_Zero,	0,		0.0}, //reserved
		{31,	6,	Modulation_Zero,	0,		0.0}, //reserved
	};

	//3GPPP 38.214
	//Table 6.1.4.1-1: MCS index table for PUSCH with transform precoding and 64QAM
	static LTENR_MCSINDEXTABLE PUSCHMCSIndexTable1[] =
	{
		{0,		2,	Modulation_QPSK,	120,	0.2344},
		{1,		2,	Modulation_QPSK,	157,	0.3066},
		{2,		2,	Modulation_QPSK,	193,	0.3770},
		{3,		2,	Modulation_QPSK,	251,	0.4902},
		{4,		2,	Modulation_QPSK,	308,	0.6016},
		{5,		2,	Modulation_QPSK,	379,	0.7402},
		{6,		2,	Modulation_QPSK,	449,	0.8770},
		{7,		2,	Modulation_QPSK,	526,	1.0273},
		{8,		2,	Modulation_QPSK,	602,	1.1758},
		{9,		2,	Modulation_QPSK,	679,	1.3262},
		{10,	4,	Modulation_16_QAM,	340,	1.3281},
		{11,	4,	Modulation_16_QAM,	378,	1.4766},
		{12,	4,	Modulation_16_QAM,	434,	1.6953},
		{13,	4,	Modulation_16_QAM,	490,	1.9141},
		{14,	4,	Modulation_16_QAM,	553,	2.1602},
		{15,	4,	Modulation_16_QAM,	616,	2.4063},
		{16,	4,	Modulation_16_QAM,	658,	2.5703},
		{17,	6,	Modulation_64_QAM,	466,	2.7305},
		{18,	6,	Modulation_64_QAM,	517,	3.0293},
		{19,	6,	Modulation_64_QAM,	567,	3.3223},
		{20,	6,	Modulation_64_QAM,	616,	3.6094},
		{21,	6,	Modulation_64_QAM,	666,	3.9023},
		{22,	6,	Modulation_64_QAM,	719,	4.2129},
		{23,	6,	Modulation_64_QAM,	772,	4.5234},
		{24,	6,	Modulation_64_QAM,	822,	4.8164},
		{25,	6,	Modulation_64_QAM,	873,	5.1152},
		{26,	6,	Modulation_64_QAM,	910,	5.3320},
		{27,	6,	Modulation_64_QAM,	948,	5.5547},
		{28,	2,	Modulation_Zero,	0,		0.0}, //reserved
		{29,	2,	Modulation_Zero,	0,		0.0}, //reserved
		{30,	4,	Modulation_Zero,	0,		0.0}, //reserved
		{31,	6,	Modulation_Zero,	0,		0.0}, //reserved
	};

	//3GPPP 38.214
	//Table 6.1.4.1-2: MCS index table 2 for PUSCH with transform precoding and 64QAM
	static LTENR_MCSINDEXTABLE PUSCHMCSIndexTable2[] =
	{
		{0,		2,	Modulation_QPSK,	30,		0.0586},
		{1,		2,	Modulation_QPSK,	40,		0.0781},
		{2,		2,	Modulation_QPSK,	50,		0.0977},
		{3,		2,	Modulation_QPSK,	64,		0.1250},
		{4,		2,	Modulation_QPSK,	78,		0.1523},
		{5,		2,	Modulation_QPSK,	99,		0.1934},
		{6,		2,	Modulation_QPSK,	120,	0.2344},
		{7,		2,	Modulation_QPSK,	157,	0.3066},
		{8,		2,	Modulation_QPSK,	193,	0.3770},
		{9,		2,	Modulation_QPSK,	251,	0.4902},
		{10,	2,	Modulation_QPSK,	308,	0.6016},
		{11,	2,	Modulation_QPSK,	379,	0.7402},
		{12,	2,	Modulation_QPSK,	449,	0.8770},
		{13,	2,	Modulation_QPSK,	526,	1.0273},
		{14,	2,	Modulation_QPSK,	602,	1.1758},
		{15,	2,	Modulation_QPSK,	679,	1.3262},
		{16,	4,	Modulation_16_QAM,	378,	1.4766},
		{17,	4,	Modulation_16_QAM,	434,	1.6953},
		{18,	4,	Modulation_16_QAM,	490,	1.9141},
		{19,	4,	Modulation_16_QAM,	553,	2.1602},
		{20,	4,	Modulation_16_QAM,	616,	2.4063},
		{21,	6,	Modulation_16_QAM,	658,	2.5703},
		{22,	6,	Modulation_16_QAM,	699,	2.7305},
		{23,	6,	Modulation_16_QAM,	772,	3.0156},
		{24,	6,	Modulation_64_QAM,	567,	3.3223},
		{25,	6,	Modulation_64_QAM,	616,	3.6094},
		{26,	6,	Modulation_64_QAM,	666,	3.9023},
		{27,	6,	Modulation_64_QAM,	772,	4.5234},
		{28,	2,	Modulation_Zero,	0,		0.0}, //reserved
		{29,	2,	Modulation_Zero,	0,		0.0}, //reserved
		{30,	4,	Modulation_Zero,	0,		0.0}, //reserved
		{31,	6,	Modulation_Zero,	0,		0.0}, //reserved
	};

	//3GPP 38.214
	//Table 5.1.3.2-1: TBS for  Ninfo <= 3824
	static LTENR_TBSTABLE TBSTableForNinfoLess3824[] =
	{
		{1,24},
		{2,32},
		{3,40},
		{4,48},
		{5,56},
		{6,64},
		{7,72},
		{8,80},
		{9,88},
		{10,96},
		{11,104},
		{12,112},
		{13,120},
		{14,128},
		{15,136},
		{16,144},
		{17,152},
		{18,160},
		{19,168},
		{20,176},
		{21,184},
		{22,192},
		{23,208},
		{24,224},
		{25,240},
		{26,256},
		{27,272},
		{28,288},
		{29,304},
		{30,320},
		{31,336},
		{32,352},
		{33,368},
		{34,384},
		{35,408},
		{36,432},
		{37,456},
		{38,480},
		{39,504},
		{40,528},
		{41,552},
		{42,576},
		{43,608},
		{44,640},
		{45,672},
		{46,704},
		{47,736},
		{48,768},
		{49,808},
		{50,848},
		{51,888},
		{52,928},
		{53,984},
		{54,1032},
		{55,1064},
		{56,1128},
		{57,1160},
		{58,1192},
		{59,1224},
		{60,1256},
		{61,1288},
		{62,1320},
		{63,1352},
		{64,1416},
		{65,1480},
		{66,1544},
		{67,1608},
		{68,1672},
		{69,1736},
		{70,1800},
		{71,1864},
		{72,1928},
		{73,2024},
		{74,2088},
		{75,2152},
		{76,2216},
		{77,2280},
		{78,2408},
		{79,2472},
		{80,2536},
		{81,2600},
		{82,2664},
		{83,2728},
		{84,2792},
		{85,2856},
		{86,2976},
		{87,3104},
		{88,3240},
		{89,3368},
		{90,3496},
		{91,3624},
		{92,3752},
		{93,3824},
	};

#ifdef  __cplusplus
}
#endif
#endif /* _NETSIM_LTENR_AMCTABLE_H_ */
