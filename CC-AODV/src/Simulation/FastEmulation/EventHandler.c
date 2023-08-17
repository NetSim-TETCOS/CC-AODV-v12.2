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
* Author:    Shashi Kant Suman	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/
#include "main.h"
#include "NetSim_utility.h"
#include "FastEmulation.h"

#pragma region EVENT_HANDLER
HANDLE eventMutex;
double dSimulationEndTime;
#pragma endregion

#pragma region EVENTHANDLER
struct stru_fel
{
	ptrEVENTDETAILS head;
	ptrEVENTDETAILS tail;
}FEL;

void init_eventhandler()
{
	isContinueEmulation = true;
	eventMutex = CreateMutexA(NULL, false, "FASTEMULATION_EVENTHANDLER");
	dSimulationEndTime = NETWORK->pstruSimulationParameter->dVal;
}

void add_event(ptrEVENTDETAILS ev)
{
	ptrEVENTDETAILS e = calloc(1, sizeof * e);
	memcpy(e, ev, sizeof * e);

	WaitForSingleObject(eventMutex, INFINITE);
	if (FEL.head == NULL)
	{
		FEL.head = e;
		FEL.tail = e;
		ReleaseMutex(eventMutex);
		return;
	}

	if (FEL.head->time > e->time)
	{
		e->next = FEL.head;
		FEL.head = e;
		ReleaseMutex(eventMutex);
		return;
	}

	if (FEL.tail->time <= e->time)
	{
		FEL.tail->next = e;
		FEL.tail = e;
		ReleaseMutex(eventMutex);
		return;
	}

	ptrEVENTDETAILS t = FEL.head;
	while (t->next)
	{
		if (t->next->time > e->time)
		{
			e->next = t->next;
			t->next = e;
			break;
		}
		t = t->next;
	}
	ReleaseMutex(eventMutex);
}

static ptrEVENTDETAILS get_event()
{
	WaitForSingleObject(eventMutex, INFINITE);
	if (FEL.head == NULL)
	{
		ReleaseMutex(eventMutex);
		return NULL;
	}

	if (em_current_time() < FEL.head->time)
	{
		ReleaseMutex(eventMutex);
		return NULL;
	}

	ptrEVENTDETAILS t = FEL.head;
	if (FEL.head == FEL.tail)
	{
		FEL.head = NULL;
		FEL.tail = NULL;
		ReleaseMutex(eventMutex);
		return t;
	}

	FEL.head = t->next;
	t->next = NULL;
	ReleaseMutex(eventMutex);
	return t;
}

void start_event_execution()
{
	while (isContinueEmulation)
	{
		if (em_current_time() >= dSimulationEndTime)
		{
			isContinueEmulation = false;
			break;
		}

		ptrEVENTDETAILS t = get_event();
		if (t)
		{
			switch (t->type)
			{
				case EVENTTYPE_APPSTART:
					start_application(t->appMap);
					break;
				case EVENTTYPE_APPEND:
					close_application(t->appMap);
					break;
				default:
					fnNetSimError("Event handler is not implemented for %d event type\n", t->type);
					break;
			}
			free(t);
		}
		else
		{
			Sleep(100);
		}
	}

	fprintf(stderr, "\rEmulation completed.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\n");
}

#pragma endregion
