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
* Author:    Dilip Kumar Kundrapu													*
* Standard:  TR 38.901(Release 16)													*
* Updated:   04th Feb, 2020											            *
* ----------------------------------------------------------------------------------*/

#include "main.h"
#include "stdafx.h"
#include "LTENR_PHY.h"
#include "LTENR_PropagationModel.h"
#include "MobilityInterface.h"

#define c (3.0 * pow(10, 8))
#define PI 3.1415


static double calculate_distance3D(double distance2D, double height_BS, double height_UT)
{
	double distance3D;
	distance3D = pow((pow((distance2D), 2) + pow((height_BS - height_UT), 2)), 0.5);
	return distance3D;
}


static double discrete_uniform_dist(double values[], int count)
{
	int lower, upper, random;
	lower = 1;
	upper = count;
	random = (rand() % (upper - lower + 1)) + lower;

	if (random == 1)
		return values[0];
	else if (random == 2)
		return values[1];
	else if (random == 3)
		return values[2];
	else if (random == 4)
		return values[3];
	else
		return 0;
}


static double log_normal_distribution(ptrLTENR_PROPAGATIONINFO info, double std)
{
	double ldRandomNumber = 0.0;
	double fFac, fRsq, fV1, fV2;
	double st, phase, loss;

	if (info->SHADOWVAR.isConstructiveShadow == 0)
	{
		do
		{
			ldRandomNumber = NETSIM_RAND_01();
			fV1 = (double)(2.0 * ldRandomNumber - 1.0);
			//calculate the Random number from this function
			ldRandomNumber = NETSIM_RAND_01();
			fV2 = (double)(2.0 * ldRandomNumber - 1.0);
			fRsq = fV1 * fV1 + fV2 * fV2;
		} while (fRsq >= 1.0 || fRsq == 0.0);

		fFac = (double)(sqrt(-2.0 * log(fRsq) / fRsq));
		info->SHADOWVAR.Gset = fV1 * fFac;
		info->SHADOWVAR.Iset = fV2 * fFac;

		st = info->SHADOWVAR.Gset; //st = Gset || Iset;
		info->SHADOWVAR.isConstructiveShadow = 1;
	}
	else
	{
		st = info->SHADOWVAR.Iset;
		info->SHADOWVAR.isConstructiveShadow = 0;
	}

	phase = NETSIM_RAND_01();
	if (phase <= 0.5)
		loss = -1 * std * st;
	else
		loss = std * st;

	return loss;
}


/** Error inverse Function for Normal Distribution */
static long double ErrorFunc(double p)
{
	return (1.0968 * (sqrt(PI) * 0.5 * (p + (PI * pow(p, 3)) / 12 + (7 * pow(PI, 2) * pow(p, 5)) / 480 + 
									    (127 * pow(PI, 3) * pow(p, 7)) / 40320 + 
									    (4369 * pow(PI, 4) * pow(p, 9)) / 5806080 + 
									    (34807 * pow(PI, 5) * pow(p, 11)) / 182476800)));
}


static double normal_distribution(double args1, double args2)
{
	double fFirstArg, fSecondArg, fRandomNumber, fDistOut;

	fFirstArg = args1;
	fSecondArg = args2;
	fRandomNumber = NETSIM_RAND_01();
	fDistOut = (double)(fFirstArg + fSecondArg * sqrt(2) * ErrorFunc(2 * fRandomNumber - 1));
	return fDistOut;
}


static double uniform_distribution(double args1, double args2)
{
	double fFirstArg, fSecondArg, fRandomNumber, fDistOut;
	fFirstArg = args1;
	fSecondArg = args2;
	fRandomNumber = NETSIM_RAND_01();
	fDistOut = fFirstArg + (fSecondArg - fFirstArg) * fRandomNumber;
	return fDistOut;
}


static double calculate_LOS_probability(ptrLTENR_PROPAGATIONINFO info)
{
	double LOS_probability, chUT = 0;
	double hUT;
	double distance2D_out = 0;

	if (info->uePosition == LTENR_POSITION_OUTDOOR && info->gnbPosition != LTENR_POSITION_INDOOR)
		distance2D_out = info->dist2D;

	else if (info->uePosition == LTENR_POSITION_INDOOR && info->gnbPosition != LTENR_POSITION_INDOOR)
		distance2D_out = info->dist2Doutdoor;

	switch (info->currentScenario)
	{
		case LTENR_SCENARIO_RMA:
		{
			if (distance2D_out <= 10)
				LOS_probability = 1;
			else
				LOS_probability = exp(-(distance2D_out - 10) / 1000);

			return LOS_probability;
		}
		break;

		case LTENR_SCENARIO_UMA:
		{
			hUT = info->propagationConfig->UE_height;

			if (distance2D_out <= 18)
				LOS_probability = 1;
			else
			{
				if (hUT <= 13)
					chUT = 0;
				else if (13 <= hUT && hUT <= 23)
					chUT = pow(((hUT - 13) / 10), 1.5);

				LOS_probability = ((18 / distance2D_out) + 
								   (exp(-(distance2D_out / 63)) * (1 - (18 / distance2D_out)))) *
								  (1 + (chUT * (5 / 4) * pow((distance2D_out / 100), 3) * 
										exp(-(distance2D_out / 150))));
			}
			return LOS_probability;
		}
		break;

		case LTENR_SCENARIO_UMI:
		{
			if (distance2D_out <= 18)
				LOS_probability = 1;
			else
				LOS_probability = ((18 / distance2D_out) + (exp(-(distance2D_out / 36)) * 
															(1 - (18 / distance2D_out))));
			return LOS_probability;
		}
		break;

		case LTENR_SCENARIO_INH:
		{
			double distance2D_in;
			distance2D_in = info->dist2D;

			switch (info->propagationConfig->indoor_type)
			{
				case LTENR_INH_MIXED_OFFICE:
				{
					if (distance2D_in <= 1.2)
						LOS_probability = 1;
					else if (1.2 <= distance2D_in && distance2D_in <= 6.5)
						LOS_probability = exp(-(distance2D_in - 1.2) / 4.7);
					else
						LOS_probability = exp(-(distance2D_in - 6.5) / 32.6) * 0.32;

					return LOS_probability;
				}
				break;

				case LTENR_INH_OPEN_OFFICE:
				{
					if (distance2D_in <= 5)
						LOS_probability = 1;
					else if (5 <= distance2D_in  && distance2D_in <= 49)
						LOS_probability = exp(-(distance2D_in - 5) / 70.8);
					else
						LOS_probability = exp(-(distance2D_in - 49) / 211.7) * 0.54;

					return LOS_probability;
				}
				break;

				default:
				{
					fnNetSimError("Unknown INDOOR_OFFICE Type %d!\n", info->propagationConfig->indoor_type);
					return 0;
				}
				break;
			}
		}
		break;

		default:
		{
			fnNetSimError("Unknown Scenario %d in function %s\n", info->currentScenario, __FUNCTION__);
			return 0;
		}
		break;
	}
}


static LTENR_STATE check_state(ptrLTENR_PROPAGATIONINFO info)
{
	double LOS_probability;

	if (info->propagationConfig->los_mode != LTENR_LOS_MODE_USER_DEFINED)
		LOS_probability = calculate_LOS_probability(info);
	else
		LOS_probability = info->propagationConfig->los_probability;

	if (NETSIM_RAND_01() <= LOS_probability)
		return LTENR_STATE_LOS;
	else
		return LTENR_STATE_NLOS;
}


static double g_UMa(double d_2D)
{
	if (d_2D <= 18)
		return 0;
	else
		return ((5 / 4) * pow((d_2D / 100), 3) * exp(-d_2D / 150));
}


static double C_UMa(double d_2D, double hUT)
{
	if (hUT < 13)
		return 0;
	else if (13 <= hUT && hUT <= 23)
		return pow(((hUT - 13) / 10), 1.5) * g_UMa(d_2D);
	else
	{
		fnNetSimError("Height of UT beyond range!\n");
		return 0;
	}
}


static double calculate_pathloss_only(ptrLTENR_PROPAGATIONINFO info)
{
	double fc = info->frequency_gHz;
	double PL1, PL2, PL3, distance2D, distance3D, distanceBP;
	double pathloss_RMa_LOS = 0, pathloss_RMa_NLOS = 0;
	double pathloss_UMa_LOS = 0, pathloss_UMa_NLOS = 0;
	double pathloss_UMi_LOS = 0, pathloss_UMi_NLOS = 0;
	double pathloss_InH_LOS = 0, pathloss_InH_NLOS = 0;

	double hBS, hUT, W, h;
	double h_effectiveEnv, hBS_effective, hUT_effective;

	distance2D = info->dist2D;
	distance3D = info->dist3D;

	LTENR_STATE state = info->propagationConfig->state;

	switch (info->currentScenario)
	{
		case LTENR_SCENARIO_RMA:
		{
			hBS = info->propagationConfig->gNB_height;
			hUT = info->propagationConfig->UE_height;
			W = info->propagationConfig->street_width;
			h = info->propagationConfig->buildings_height;

			distanceBP = 2 * PI * hBS * hUT * ((fc * 1000000000) / c);

			PL1 = (20 * log10(40 * PI * distance3D * fc / 3)) + fmin((0.03 * pow(h, 1.72)), 10) * 
				  log10(distance3D) - fmin((0.044 * pow(h, 1.72)), 14.77) + (0.002 * log10(h) * distance3D);
			PL2 = PL1 + (40 * log10(distance3D / distanceBP)); //PL1(distanceBP) = PL1

			if (10 <= distance2D && distance2D <= distanceBP)
				pathloss_RMa_LOS = PL1;
			else if (distanceBP <= distance2D && distance2D <= 10000)
				pathloss_RMa_LOS = PL2;

			if (state == LTENR_STATE_LOS)
				return pathloss_RMa_LOS;

			else
			{
				PL3 = 161.04 - (7.1 * log10(W)) + (7.5 * log10(h)) - 
					  ((24.37 - 3.7 * pow((h / hBS), 2)) * log10(hBS)) +
					  ((43.42 - 3.1 * log10(hBS)) * (log10(distance3D) - 3)) + 
					  (20 * log10(fc)) - ((3.2 * pow((log10(11.75 * hUT)), 2) - 4.97));
				if (distance2D > 5000)
					distance2D = 5000;
				if (10 <= distance2D && distance2D <= 5000)
					pathloss_RMa_NLOS = fmax(pathloss_RMa_LOS, PL3);

				return pathloss_RMa_NLOS;
			}
		}
		break;

		case LTENR_SCENARIO_UMA:
		{
			hBS = info->propagationConfig->gNB_height;
			hUT = info->propagationConfig->UE_height;

			double prob = 1 / (1 + C_UMa(distance2D, hUT));
			double r = NETSIM_RAND_01();

			if (r <= prob)
				h_effectiveEnv = 1.0;
			else
			{
				double values[4];

				if ((hUT - 1.5) <= 12)
					h_effectiveEnv = hUT - 1.5;
				else if (12 < (hUT - 1.5) && (hUT - 1.5) <= 15)
				{
					values[0] = 12;
					values[1] = hUT - 1.5;
					h_effectiveEnv = discrete_uniform_dist(values, 2);
				}
				else if (15 < (hUT - 1.5) && (hUT - 1.5) <= 18)
				{
					values[0] = 12;
					values[1] = 15;
					values[2] = hUT - 1.5;
					h_effectiveEnv = discrete_uniform_dist(values, 3);
				}
				else
				{
					values[0] = 12;
					values[1] = 15;
					values[2] = 18;
					values[3] = hUT - 1.5;
					h_effectiveEnv = discrete_uniform_dist(values, 4);
				}
			}

			hBS_effective = hBS - h_effectiveEnv;
			hUT_effective = hUT - h_effectiveEnv;

			distanceBP = 4 * hBS_effective * hUT_effective * ((fc * 1000000000) / c);

			PL1 = 28.0 + (22 * log10(distance3D)) + (20 * log10(fc));
			PL2 = 28.0 + (40 * log10(distance3D)) + (20 * log10(fc)) - (9 * log10(pow(distanceBP, 2) + 
																		pow((hBS - hUT), 2)));

			if (10 <= distance2D && distance2D <= distanceBP)
				pathloss_UMa_LOS = PL1;
			else if (distanceBP <= distance2D && distance2D <= 5000)
				pathloss_UMa_LOS = PL2;

			if (state == LTENR_STATE_LOS)
				return pathloss_UMa_LOS;

			else
			{
				PL3 = 13.54 + (39.08 * log10(distance3D)) + (20 * log10(fc)) - (0.6 * (hUT - 1.5));

				if (10 <= distance2D && distance2D <= 5000)
					pathloss_UMa_NLOS = fmax(pathloss_UMa_LOS, PL3);

				return pathloss_UMa_NLOS;
			}
		}
		break;

		case LTENR_SCENARIO_UMI:
		{
			hBS = info->propagationConfig->gNB_height;
			hUT = info->propagationConfig->UE_height;
			h_effectiveEnv = 1.0;
			hBS_effective = hBS - h_effectiveEnv;
			hUT_effective = hUT - h_effectiveEnv;

			distanceBP = 4 * hBS_effective * hUT_effective * ((fc * 1000000000) / c);

			PL1 = 32.4 + (21 * log10(distance3D)) + (20 * log10(fc));
			PL2 = 32.4 + (40 * log10(distance3D)) + (20 * log10(fc)) - (9.5 * log10(pow(distanceBP, 2) + 
																		pow((hBS - hUT), 2)));

			if (10 <= distance2D && distance2D <= distanceBP)
				pathloss_UMi_LOS = PL1;
			else if (distanceBP <= distance2D && distance2D <= 5000)
				pathloss_UMi_LOS = PL2;

			if (state == LTENR_STATE_LOS)
				return pathloss_UMi_LOS;

			else
			{
				PL3 = (35.3 * log10(distance3D)) + 22.4 + (21.3 * log10(fc)) - (0.3 * (hUT - 1.5));

				if (10 <= distance2D && distance2D <= 5000)
					pathloss_UMi_NLOS = fmax(pathloss_UMi_LOS, PL3);

				return pathloss_UMi_NLOS;
			}
		}
		break;

		case LTENR_SCENARIO_INH:
		{
			hBS = info->propagationConfig->gNB_height;
			hUT = info->propagationConfig->UE_height;

			pathloss_InH_LOS = 32.4 + (17.3 * log10(distance3D)) + (20 * log10(fc));

			if (state == LTENR_STATE_LOS)
				return pathloss_InH_LOS;

			else
			{
				PL3 = (38.3 * log10(distance3D)) + 17.30 + (24.9 * log10(fc));
				pathloss_InH_NLOS = fmax(pathloss_InH_LOS, PL3);
				return pathloss_InH_NLOS;
			}
		}
		break;

		default:
		{
			fnNetSimError("Unknown Scenario %d in funtion %s\n", info->currentScenario, __FUNCTION__);
			return 0;
		}
		break;
	}
}


static double calculate_shadow_fading(ptrLTENR_PROPAGATIONINFO info)
{
	double fc = info->frequency_gHz;
	double hBS, hUT, distanceBP, shadowFadingStd = 0;
	double distance2D = info->dist2D;

	LTENR_STATE state = info->propagationConfig->state;

	switch (info->currentScenario)
	{
		case LTENR_SCENARIO_RMA:
		{
			hBS = info->propagationConfig->gNB_height;
			hUT = info->propagationConfig->UE_height;

			distanceBP = 2 * PI * hBS * hUT * ((fc * 1000000000) / c);

			if (state == LTENR_STATE_LOS)
			{
				if (10 <= distance2D && distance2D <= distanceBP)
					shadowFadingStd = 4;
				else if (distanceBP <= distance2D && distance2D <= 10000)
					shadowFadingStd = 6;
			}
			else
				shadowFadingStd = 8;

			return log_normal_distribution(info, shadowFadingStd);
		}
		break;

		case LTENR_SCENARIO_UMA:
		{
			if (state == LTENR_STATE_LOS)
				shadowFadingStd = 4;
			else
				shadowFadingStd = 6;

			return log_normal_distribution(info, shadowFadingStd);
		}
		break;

		case LTENR_SCENARIO_UMI:
		{
			if (state == LTENR_STATE_LOS)
				shadowFadingStd = 4;
			else
				shadowFadingStd = 7.82;

			return log_normal_distribution(info, shadowFadingStd);
		}
		break;

		case LTENR_SCENARIO_INH:
		{
			if (state == LTENR_STATE_LOS)
				shadowFadingStd = 3;
			else
				shadowFadingStd = 8.03;

			return log_normal_distribution(info, shadowFadingStd);
		}
		break;

		default:
		{
			fnNetSimError("Unknown Scenario %d in function %s\n", info->currentScenario, __FUNCTION__);
			return 0;
		}
		break;
	}
}


static double calculate_O2I_penetrationloss(ptrLTENR_PROPAGATIONINFO info)
{
	double f = info->frequency_gHz;
	double PL_tw, PL_in, std_p, O2Iloss;
	double L_glass = 2 + (0.2 * f);
	double L_iirglass = 23 + (0.3 * f);
	double L_concrete = 5 + (4 * f);

	double dist2D_in = 0.0;
	if (info->currentScenario == LTENR_SCENARIO_UMA || info->currentScenario == LTENR_SCENARIO_UMI)
		dist2D_in = fmin(uniform_distribution(0, 25), uniform_distribution(0, 25));
	else if (info->currentScenario == LTENR_SCENARIO_RMA)
		dist2D_in = fmin(uniform_distribution(0, 10), uniform_distribution(0, 10));

	switch (info->propagationConfig->O2I_model)
	{
		case LTENR_O2IMODEL_LOW_LOSS:
		{
			PL_tw = 5 - 10 * log10((0.3 * pow(10, (-L_glass / 10))) + (0.7 * pow(10, (-L_concrete / 10))));
			PL_in = 0.5 * dist2D_in;
			std_p = 4.4;

			O2Iloss = PL_tw + PL_in + abs((int)normal_distribution(0, std_p));
			return O2Iloss;
		}
		break;

		case LTENR_O2IMODEL_HIGH_LOSS:
		{
			PL_tw = 5 - 10 * log10((0.7 * pow(10, (-L_iirglass / 10))) + (0.3 * pow(10, (-L_concrete / 10))));
			PL_in = 0.5 * dist2D_in;
			std_p = 6.5;

			O2Iloss = PL_tw + PL_in + abs((int)normal_distribution(0, std_p));
			return O2Iloss;
		}
		break;

		default:
		{
			fnNetSimError("Unknown O2I model %d in function %s\n", info->propagationConfig->O2I_model, 
						  __FUNCTION__);
			return 0;
		}
		break;
	}
}

static void decideIndoorOutDoor(ptrLTENR_PROPAGATIONINFO info)
{
	if (fnMobility_isPosInsideAnyBuilding(&info->uePos) != 0)
		info->uePosition = LTENR_POSITION_INDOOR;
	else
		info->uePosition = LTENR_POSITION_OUTDOOR;

	if (fnMobility_isPosInsideAnyBuilding(&info->gnbPos))
		info->gnbPosition = LTENR_POSITION_INDOOR;
	else
		info->gnbPosition = LTENR_POSITION_OUTDOOR;
}

static void calculateDistance(ptrLTENR_PROPAGATIONINFO info)
{
	info->dist2D = fn_NetSim_Utilities_CalculateDistance(&info->gnbPos, &info->uePos);
	if (info->uePosition == LTENR_POSITION_INDOOR && info->gnbPosition != LTENR_POSITION_INDOOR)
	{
		info->dist2Dindoor = fnMobility_findIndoorDistance(&info->gnbPos, &info->uePos);
		info->dist2Doutdoor = info->dist2D - info->dist2Dindoor;
	}
}

static void recalculate_localvar(ptrLTENR_PROPAGATIONINFO info)
{
	info->frequency_gHz = info->centralFrequency_MHz / 1000.0;

	decideIndoorOutDoor(info);
	calculateDistance(info);
	
	if (info->gnbPosition != LTENR_POSITION_INDOOR)
		info->currentScenario = info->propagationConfig->outScenario;
	else
		info->currentScenario = info->propagationConfig->inScenario;

	switch (info->currentScenario)
	{
		case LTENR_SCENARIO_RMA:
		{
			if (info->dist2D < 35)
				info->dist2D = 35;

			info->dist3D = calculate_distance3D(info->dist2D, info->propagationConfig->gNB_height, 
												info->propagationConfig->UE_height);
		}
		break;
		case LTENR_SCENARIO_UMA:
		{
			if (info->dist2D < 35)
				info->dist2D = 35;

			info->dist3D = calculate_distance3D(info->dist2D, info->propagationConfig->gNB_height, 
												info->propagationConfig->UE_height);
		}
		break;
		case LTENR_SCENARIO_UMI:
		{
			if (info->dist2D < 10)
				info->dist2D = 10;
			
			info->dist3D = calculate_distance3D(info->dist2D, info->propagationConfig->gNB_height, 
												info->propagationConfig->UE_height);
		}
		break;
		case LTENR_SCENARIO_INH:
		{
			info->dist3D = calculate_distance3D(info->dist2D, info->propagationConfig->gNB_height, 
												info->propagationConfig->UE_height);
		}
		break;

		default:
			fnNetSimError("Unknown Scenario %d in function %s!\n", info->currentScenario, __FUNCTION__);
			break;
	}
}

void LTENR_Propagation_TotalLoss(ptrLTENR_PROPAGATIONINFO info)
{
	recalculate_localvar(info);

	info->propagationConfig->state = check_state(info);
	info->beamFormingGain = info->propagationConfig->beamFormingGain;
	switch (info->propagationConfig->channel_character)
	{
		case LTENR_CHANNELCHAR_NO_PATHLOSS:
		{
			info->dTotalLoss = 0;
			info->dPathLoss = 0;
			info->dO2ILoss = 0;
			info->dShadowFadingLoss = 0;
			info->beamFormingGain = 0;
		}
		break;

		case LTENR_CHANNELCHAR_PATHLOSS_ONLY:
		{
			
			info->dPathLoss = calculate_pathloss_only(info);
			info->dTotalLoss = info->dPathLoss - info->beamFormingGain;
			info->dO2ILoss = 0;
			info->dShadowFadingLoss = 0;
		}
		break;

		case LTENR_CHANNELCHAR_PATHLOSS_SHADOWFADING:
		{
			info->dPathLoss = calculate_pathloss_only(info);
			info->dShadowFadingLoss = calculate_shadow_fading(info);
			info->dTotalLoss = info->dPathLoss + info->dShadowFadingLoss - info->beamFormingGain;
			info->dO2ILoss = 0;
			
		}
		break;

		case LTENR_CHANNELCHAR_PATHLOSS_SHADOWFADING_O2ILOSS:
		{
			info->dPathLoss = calculate_pathloss_only(info);
			info->dShadowFadingLoss = calculate_shadow_fading(info);

			if (info->uePosition == LTENR_POSITION_INDOOR && info->currentScenario != LTENR_SCENARIO_INH)
				info->dO2ILoss = calculate_O2I_penetrationloss(info);
			else
				info->dO2ILoss = 0;

			info->dTotalLoss = info->dPathLoss + info->dShadowFadingLoss + info->dO2ILoss - info->beamFormingGain;
		}
		break;

		default:
			fnNetSimError("Unknown Channel Characteristic in function %s\n", 
						  info->propagationConfig->channel_character, __FUNCTION__);
			break;
	}
#ifdef LTENR_PROPAGATION_LOG
	print_ltenr_log("\tPropagation Model starts for gNB=%d and UE=%d\n", info->gnbId, info->ueId);
	print_ltenr_log("\t\t2D Distance = %lfm\n", info->dist2D);
	print_ltenr_log("\t\t3D Distance=%lfm\n", info->dist3D);
	print_ltenr_log("\t\tChannel condition = %s\n", strLTENR_STATE[info->propagationConfig->state]);
	print_ltenr_log("\t\tTotal Propagation Loss = %lfdB\n", info->dTotalLoss);
	print_ltenr_log("\t\t\tPathLoss = %lfdB\n", info->dPathLoss);
	print_ltenr_log("\t\t\tShadow Fading Loss = %lfdB\n", info->dShadowFadingLoss);
	print_ltenr_log("\t\t\tO2I Penetration Loss = %lfdB\n", info->dO2ILoss);
	print_ltenr_log("\t\t\tBeamforming Gain = %lfdB\n", info->beamFormingGain);
#endif // LTENR_PROPAGATION_LOG
}
