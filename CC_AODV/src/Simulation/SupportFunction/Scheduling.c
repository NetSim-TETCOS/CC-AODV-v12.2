#include "main.h"
#include "Scheduling.h"
#include "NetSim_Plot.h"
#pragma comment(lib,"Metrics.lib")

enum enum_Buffer_Manipulation_Type
{
	ADD = 1,
	GET,
};
#define fnGetList(pstruPacketlist,Priority) (pstruPacketlist+Priority/(Priority_High-Priority_Medium))

//Queuing functions
enum enum_BUFFER fn_NetSim_DropTail(NetSim_BUFFER*, NetSim_PACKET*);
enum enum_BUFFER fn_NetSim_RED(NetSim_BUFFER*, NetSim_PACKET*);
enum enum_BUFFER fn_NetSim_WRED(NetSim_BUFFER*, NetSim_PACKET*);

//Scheduling functions
NetSim_PACKET* fn_NetSim_Priority(NetSim_BUFFER*, NetSim_PACKET*, int, int);
NetSim_PACKET* fn_NetSim_FIFO(NetSim_BUFFER*, NetSim_PACKET*, int, int);
NetSim_PACKET* fn_NetSim_RoundRobin(NetSim_BUFFER*, NetSim_PACKET*, int, int);
NetSim_PACKET* fn_NetSim_WFQ(NetSim_BUFFER*, NetSim_PACKET*, int, int);
NetSim_PACKET* fn_NetSim_EDF(NetSim_BUFFER*, NetSim_PACKET*, int, int);


UINT64 fn_NetSim_GetPosition_MaximumNumber(UINT64* nCount);
int fn_NetSim_GetBufferStatus(NetSim_BUFFER*);
enum enum_BUFFER fn_NetSim_CheckBuffer(NetSim_BUFFER*, NetSim_PACKET*);
static void fn_NetSim_DropPackets(NetSim_BUFFER*, NetSim_PACKET*);

static void* init_buffer_plot(NETSIM_ID d, NETSIM_ID in)
{
	char heading[BUFSIZ];
	sprintf(heading, "Buffer_Occupancy_%s_Interface_%d", DEVICE_NAME(d), DEVICE_INTERFACE_CONFIGID(d, in));
	return  fn_NetSim_Install_Metrics_Plot(Plot_Buffer,
										   "Buffer_Occupancy",
										   heading);
}

/** This function is to check whether buffer list has any packet or not **/
_declspec(dllexport) int fn_NetSim_GetBufferStatus(NetSim_BUFFER* pstruBuffer)
{
	SCHEDULING_TYPE Scheduling_Technique;
	NetSim_PACKET* pstruTempList;
	int nFlag = 0;

	Scheduling_Technique = pstruBuffer->nSchedulingType;
	switch (Scheduling_Technique)
	{
	case 0:
	case SCHEDULING_PRIORITY:
	case SCHEDULING_EDF:
	case SCHEDULING_FIFO:
		if (!pstruBuffer->pstruPacketlist)
			return 0;
		else
			return 1;
		break;
	case SCHEDULING_ROUNDROBIN:
	case SCHEDULING_WFQ:


		if (pstruBuffer->pstruPacketlist)
		{
		Retry:
			nFlag++;
			if (nFlag == 5)
				return 0;
			else
			{
				pstruBuffer->pstruPacketlist->nPacketPriority -= Priority_High - Priority_Medium;
				if (pstruBuffer->pstruPacketlist->nPacketPriority <= 0)
					pstruBuffer->pstruPacketlist->nPacketPriority = Priority_High;

				pstruTempList = fnGetList(pstruBuffer->pstruPacketlist, pstruBuffer->pstruPacketlist->nPacketPriority);
				if (pstruTempList->pstruNextPacket != NULL)
					return 1;
				else
					goto Retry;
			}
			break;
		}
		else
			return 0;
	default:
		return 0;
		break;
	}
}

/** This function is to return queuing status based on given algorithm **/
enum enum_Buffer fn_NetSim_Queuing(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	QUEUINGTECHNIQUE Queuing_Technique;
	int queuing_result;
	Queuing_Technique = pstruBuffer->queuingTechnique;
	switch (Queuing_Technique)
	{
	case QUEUING_DROPTAIL:
		queuing_result = fn_NetSim_DropTail(pstruBuffer, pstruData);
		break;
	case QUEUING_RED:
		queuing_result = fn_NetSim_RED(pstruBuffer, pstruData);
		break;
	case QUEUING_WRED:
		queuing_result = fn_NetSim_WRED(pstruBuffer, pstruData);
		break;
	default:
		queuing_result = fn_NetSim_DropTail(pstruBuffer, pstruData);
		break;
	}

	if (queuing_result == Buffer_Underflow)
		return Buffer_Underflow;
	else
		return Buffer_Overflow;
}

/** This function is to add packet to the buffer **/
_declspec(dllexport)int fn_NetSim_Packet_AddPacketToBuffer(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	int nBufferFlag;
	int nType = 1;
	SCHEDULING_TYPE Scheduling_Technique;

	if (pstruBuffer->isPlotEnable && pstruBuffer->netsimPlotVar == NULL)
		pstruBuffer->netsimPlotVar = init_buffer_plot(pstruBuffer->deviceId, pstruBuffer->interfaceId);

	nBufferFlag = fn_NetSim_Queuing(pstruBuffer, pstruData);
	if (nBufferFlag == Buffer_Underflow)
	{
		if (pstruBuffer->isPlotEnable)
			add_plot_data_formatted(pstruBuffer->netsimPlotVar, "%lf,%d,%s,",
									pstruEventDetails->dEventTime,
									(UINT)pstruData->pstruNetworkData->dPacketSize,
									"Enqueue");
		pstruBuffer->nQueuedPacket++;
		Scheduling_Technique = pstruBuffer->nSchedulingType;
		switch (Scheduling_Technique)
		{
		case SCHEDULING_PRIORITY:
			fn_NetSim_Priority(pstruBuffer, pstruData, nType, 1);
			break;
		case SCHEDULING_FIFO:
			fn_NetSim_FIFO(pstruBuffer, pstruData, nType, 1);
			break;
		case SCHEDULING_ROUNDROBIN:
			fn_NetSim_RoundRobin(pstruBuffer, pstruData, nType, 1);
			break;
		case SCHEDULING_WFQ:
			fn_NetSim_WFQ(pstruBuffer, pstruData, nType, 1);
			break;
		case SCHEDULING_EDF:
			fn_NetSim_EDF(pstruBuffer, pstruData, nType, 1);
			break;
		default:
			fn_NetSim_FIFO(pstruBuffer, pstruData, nType, 1);
			break;
		}
	}
	else if (nBufferFlag == Buffer_Overflow)
	{
		if (pstruBuffer->isPlotEnable)
			add_plot_data_formatted(pstruBuffer->netsimPlotVar, "%lf,%d,%s,",
									pstruEventDetails->dEventTime,
									(UINT)pstruData->pstruNetworkData->dPacketSize,
									"Drop");

		pstruBuffer->nDroppedPacket++;
		pstruData->nPacketStatus = PacketStatus_Buffer_Dropped;
		pstruData->nReceiverId = pstruEventDetails->nDeviceId;
		fn_NetSim_WritePacketTrace(pstruData);
		fn_NetSim_Packet_FreePacket(pstruData);
		pstruData = NULL;
	}
	else
	{
		assert(false);
	}
	return 0;
}

/** This function is to get the packet from the buffer **/
_declspec(dllexport) NetSim_PACKET* fn_NetSim_GetPacketFromBuffer(NetSim_BUFFER* pstruBuffer, int nFlag)
{

	int nType = 2;
	int nScheduling_Technique;
	NetSim_PACKET* pstruData = NULL;
	if (pstruBuffer->pstruPacketlist)
	{
		if (nFlag)
			pstruBuffer->nDequeuedPacket++;
		nScheduling_Technique = pstruBuffer->nSchedulingType;
		switch (nScheduling_Technique)
		{
		case SCHEDULING_PRIORITY:
			pstruData = fn_NetSim_Priority(pstruBuffer, pstruData, nType, nFlag);
			break;
		case SCHEDULING_FIFO:
			pstruData = fn_NetSim_FIFO(pstruBuffer, pstruData, nType, nFlag);
			break;
		case SCHEDULING_ROUNDROBIN:
			pstruData = fn_NetSim_RoundRobin(pstruBuffer, pstruData, nType, nFlag);
			break;
		case SCHEDULING_WFQ:
			pstruData = fn_NetSim_WFQ(pstruBuffer, pstruData, nType, nFlag);
			break;
		case SCHEDULING_EDF:
			pstruData = fn_NetSim_EDF(pstruBuffer, pstruData, nType, nFlag);
			break;
		default:
			pstruData = fn_NetSim_FIFO(pstruBuffer, pstruData, nType, nFlag);
			break;
		}
	}
	
	if (pstruData == NULL)
		return NULL;

	if (nFlag &&
		pstruBuffer->dMaxBufferSize)
	{
		double d_NN_DataSize;
		if (pstruData->pstruNetworkData)
			d_NN_DataSize = pstruData->pstruNetworkData->dPacketSize;
		else
			d_NN_DataSize = pstruData->pstruMacData->dPacketSize;
		pstruBuffer->dCurrentBufferSize -= (double)((int)d_NN_DataSize);
		if (pstruBuffer->dCurrentBufferSize < 0)
		{
			assert(false);
			pstruBuffer->dCurrentBufferSize = 0;
			fnNetSimError("Assert failure in scheduling. Buffer size is negative.\n");
		}
	}

	if (pstruBuffer->queuingTechnique == QUEUING_RED && pstruBuffer->dCurrentBufferSize == 0)
	{
		ptrQUEUING_RED_VAR queuing_var = pstruBuffer->queuingParam;
		queuing_var->q_time = pstruEventDetails->dEventTime;
	}

	if (pstruBuffer->queuingTechnique == QUEUING_WRED && pstruBuffer->dCurrentBufferSize == 0)
	{
		ptrQUEUING_WRED_VAR queuing_var = pstruBuffer->queuingParam;
		queuing_var->q_time = pstruEventDetails->dEventTime;
	}

	if (pstruBuffer->isPlotEnable && nFlag)
		add_plot_data_formatted(pstruBuffer->netsimPlotVar, "%lf,%d,%s,",
								pstruEventDetails->dEventTime,
								(UINT)pstruData->pstruNetworkData->dPacketSize,
								"Dequeue");
	return pstruData;
}

/** The function will get the array and it will return the Maximum number's position **/
UINT64 fn_NetSim_GetPosition_MaximumNumber(UINT64* nCount)
{
	unsigned long long int nMax;
	UINT nLoop;
	UINT64 nFlag = 0;
	nMax = nCount[0];
	for (nLoop = 1; nLoop < 4; nLoop++)
	{
		if (nCount[nLoop] >= nMax)
		{
			nMax = nCount[nLoop];
			nFlag = nLoop;
		}
	}

	return nFlag + 1;
}

/** Function for dropping packets **/
static void fn_NetSim_DropPackets(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	pstruBuffer->nDroppedPacket++;
	pstruData->nPacketStatus = PacketStatus_Buffer_Dropped;
	pstruData->nReceiverId = pstruData->nTransmitterId;
	fn_NetSim_WritePacketTrace(pstruData);
	fn_NetSim_Packet_FreePacket(pstruData);
	pstruData = NULL;
}

/**
If the Current buffer size is greater than the Maximum buffer size --> Buffer Overflow
Otherwise Buffer underflows.
*/
enum enum_BUFFER fn_NetSim_CheckBuffer(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	double d_NN_DataSize = 0;
	double ldMaximumBufferSize;
	double ldCurrentBufferSize;
	NetSim_PACKET* p = pstruData;

	while (p)
	{
		if (pstruData->pstruNetworkData)
			d_NN_DataSize += pstruData->pstruNetworkData->dPacketSize;
		else
			d_NN_DataSize += pstruData->pstruMacData->dPacketSize;
		p = p->pstruNextPacket;
	}
	if (!pstruBuffer->dMaxBufferSize)
		pstruBuffer->dMaxBufferSize = (double)0xFFFFFFFFFFFFFFFF;
	ldMaximumBufferSize = pstruBuffer->dMaxBufferSize * 1024 * 1024;
	ldCurrentBufferSize = pstruBuffer->dCurrentBufferSize + d_NN_DataSize;
	if (ldCurrentBufferSize <= ldMaximumBufferSize)
	{
		//buffer underflow
		pstruBuffer->dCurrentBufferSize = (double)((int)ldCurrentBufferSize);
		return Buffer_Underflow;
	}
	else
		//buffer overflow
		return Buffer_Overflow;
}

/** Queuing - Packets are dropped only when buffer is full **/
enum enum_BUFFER fn_NetSim_DropTail(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	enum enum_BUFFER checkBuffer = fn_NetSim_CheckBuffer(pstruBuffer, pstruData);
	if (checkBuffer == Buffer_Underflow)
		return Buffer_Underflow;
	else
		return Buffer_Overflow;
}

/** Function is used to add packet size to of buffer **/
static void add_current_size_buffer(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	double d_NN_DataSize = 0;
	double ldCurrentBufferSize;
	NetSim_PACKET* p = pstruData;

	while (p)
	{
		if (pstruData->pstruNetworkData)
			d_NN_DataSize += pstruData->pstruNetworkData->dPacketSize;
		else
			d_NN_DataSize += pstruData->pstruMacData->dPacketSize;
		p = p->pstruNextPacket;
	}
	ldCurrentBufferSize = pstruBuffer->dCurrentBufferSize + d_NN_DataSize;
	pstruBuffer->dCurrentBufferSize = (double)((int)ldCurrentBufferSize);
}

/** Function is used to reduce size of buffer(added while checkBuffer) in case of packet drop **/
static void reduce_current_size_buffer(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	double d_NN_DataSize;
	if (pstruData->pstruNetworkData)
		d_NN_DataSize = pstruData->pstruNetworkData->dPacketSize;
	else
		d_NN_DataSize = pstruData->pstruMacData->dPacketSize;
	pstruBuffer->dCurrentBufferSize -= (double)((int)d_NN_DataSize);
	if (pstruBuffer->dCurrentBufferSize < 0)
	{
		assert(false);
		pstruBuffer->dCurrentBufferSize = 0;
		fnNetSimError("Scheduling--- Buffer size negative. how?????\n");
	}
}

/** Helper function for RED Queuing Technique **/
static enum enum_BUFFER red_algorithm(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	ptrQUEUING_RED_VAR queuing_var = pstruBuffer->queuingParam;

	double pb;

	if (queuing_var->min_th <= queuing_var->avg_queue_size &&
		queuing_var->avg_queue_size < queuing_var->max_th)
	{
		queuing_var->count++;
		pb = queuing_var->max_p * (queuing_var->avg_queue_size - queuing_var->min_th) /
			(queuing_var->max_th - queuing_var->min_th);
		if (queuing_var->count > 0 && queuing_var->count > queuing_var->random_value / pb)
		{
			queuing_var->count = 0;
			queuing_var->random_value = NETSIM_RAND_01();

			return Buffer_Overflow;
		}
		return Buffer_Underflow;
	}
	else if (queuing_var->max_th <= queuing_var->avg_queue_size)
	{
		queuing_var->count = -1;

		return Buffer_Overflow;
	}
	else
	{
		queuing_var->count = -1;
		return Buffer_Underflow;
	}
}

/** Queuing - RED **/
enum enum_BUFFER fn_NetSim_RED(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	enum enum_BUFFER checkBuffer = fn_NetSim_CheckBuffer(pstruBuffer, pstruData);
	if (checkBuffer == Buffer_Underflow)
	{
		reduce_current_size_buffer(pstruBuffer, pstruData);
		ptrQUEUING_RED_VAR queuing_var = pstruBuffer->queuingParam;
		double curr_buff_size_in_MB = pstruBuffer->dCurrentBufferSize / 1048576;

		if (curr_buff_size_in_MB == 0)
		{
			queuing_var->avg_queue_size = pow((1 - queuing_var->wq), (pstruEventDetails->dEventTime - queuing_var->q_time))
				* queuing_var->avg_queue_size;
		}
		else
		{
			queuing_var->avg_queue_size = queuing_var->avg_queue_size +
				queuing_var->wq * (curr_buff_size_in_MB - queuing_var->avg_queue_size);
		}

		checkBuffer = red_algorithm(pstruBuffer, pstruData);
		if (checkBuffer == Buffer_Underflow)
			add_current_size_buffer(pstruBuffer, pstruData); // Increment current buffer size

		return checkBuffer;
	}
	else
		return Buffer_Overflow;
}

/** Helper function for WRED Queuing Technique **/
static enum enum_BUFFER wred_algorithm(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData,
	double max_th, double min_th, double max_p, int* count, double* random)
{
	ptrQUEUING_WRED_VAR queuing_var = pstruBuffer->queuingParam;

	double pb;

	if (min_th <= queuing_var->avg_queue_size &&
		queuing_var->avg_queue_size < max_th)
	{
		*count = *count + 1;
		pb = max_p * (queuing_var->avg_queue_size - min_th) /
			(max_th - min_th);
		if (*count > 0 && *count > * random / pb)
		{
			*count = 0;
			*random = NETSIM_RAND_01();

			return Buffer_Overflow;
		}
		return Buffer_Underflow;
	}
	else if (max_th <= queuing_var->avg_queue_size)
	{
		*count = -1;
		return Buffer_Overflow;
	}
	else
	{
		*count = -1;
		return Buffer_Underflow;
	}
}

/** Queuing - WRED **/
enum enum_BUFFER fn_NetSim_WRED(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData)
{
	enum enum_BUFFER checkBuffer = fn_NetSim_CheckBuffer(pstruBuffer, pstruData);
	if (checkBuffer == Buffer_Underflow)
	{
		reduce_current_size_buffer(pstruBuffer, pstruData);
		ptrQUEUING_WRED_VAR queuing_var = pstruBuffer->queuingParam;
		int index;
		double curr_max_th;
		double curr_min_th;
		double curr_max_p;
		int* curr_count;
		double* curr_random;
		double curr_buff_size_in_MB = pstruBuffer->dCurrentBufferSize / 1048576;
		int priority = pstruData->nPacketPriority;

		if (curr_buff_size_in_MB == 0)
		{
			queuing_var->avg_queue_size = pow((1 - queuing_var->wq), (pstruEventDetails->dEventTime - queuing_var->q_time))
				* queuing_var->avg_queue_size;
		}
		else
		{
			queuing_var->avg_queue_size = queuing_var->avg_queue_size +
				queuing_var->wq * (curr_buff_size_in_MB - queuing_var->avg_queue_size);
		}

		index = (priority / 2) - 1;

		//adding control packets in queue
		if (index == -1)
		{
			checkBuffer = Buffer_Underflow;
		}
		else
		{
			curr_max_th = *(queuing_var->max_th + index);
			curr_min_th = *(queuing_var->min_th + index);
			curr_max_p = *(queuing_var->max_p + index);
			curr_count = (queuing_var->count + index);
			curr_random = (queuing_var->random_value + index);

			checkBuffer = wred_algorithm(pstruBuffer, pstruData, curr_max_th, curr_min_th, curr_max_p, curr_count, curr_random);
		}

		if (checkBuffer == Buffer_Underflow)
			add_current_size_buffer(pstruBuffer, pstruData); // Increment current buffer size

		return checkBuffer;
	}
	else
		return Buffer_Overflow;
}

/**
If the type is ADD, then all the packets will be added in the buffer based on packet priority
If the type is GET, then the packets to be retrieved based on the priority
*/
NetSim_PACKET* fn_NetSim_Priority(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData, int nType, int nFlag)
{
	switch (nType)
	{
	case ADD:
		if (pstruBuffer->pstruPacketlist)
		{
			NetSim_PACKET* pstruTmpBufferPacketList;
			NetSim_PACKET* pstruPrevPacketList;

			pstruTmpBufferPacketList = pstruBuffer->pstruPacketlist;
			pstruPrevPacketList = pstruBuffer->pstruPacketlist;
			if (pstruTmpBufferPacketList->nPacketPriority < pstruData->nPacketPriority)
			{
				pstruData->pstruNextPacket = pstruTmpBufferPacketList;
				pstruBuffer->pstruPacketlist = pstruData;
				return 0;
			}
			while (pstruTmpBufferPacketList)
			{
				if (pstruTmpBufferPacketList->nPacketPriority < pstruData->nPacketPriority)
				{
					pstruData->pstruNextPacket = pstruTmpBufferPacketList;
					pstruPrevPacketList->pstruNextPacket = pstruData;
					break;
				}
				else if (!pstruTmpBufferPacketList->pstruNextPacket)
				{
					pstruTmpBufferPacketList->pstruNextPacket = pstruData;
					break;
				}
				else
				{
					pstruPrevPacketList = pstruTmpBufferPacketList;
					pstruTmpBufferPacketList = pstruTmpBufferPacketList->pstruNextPacket;
				}
			}

		}
		else
			pstruBuffer->pstruPacketlist = pstruData;
		break;
	case GET:
	{
		NetSim_PACKET* pstruPacket;
		pstruPacket = pstruBuffer->pstruPacketlist;
		if (nFlag)
		{
			pstruBuffer->pstruPacketlist = pstruPacket->pstruNextPacket;
			pstruPacket->pstruNextPacket = NULL;
		}
		return pstruPacket;
		break;
	}
	default:
		printf("\nInvalid Selection in Buffer\n");
		break;
	}
	return NULL;

}
/**
If the type is ADD, then all the packets will be added in the buffer based on the arrival
If the type is GET, then the packets to be retrieved depends on the arrangement in the queue
*/
NetSim_PACKET* fn_NetSim_FIFO(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData, int nType, int nFlag)
{
	NetSim_PACKET* p = pstruData;
	while (p && p->pstruNextPacket)
		p = p->pstruNextPacket;
	switch (nType)
	{
	case ADD:
		if (pstruBuffer->pstruPacketlist)
		{
			pstruBuffer->last->pstruNextPacket = pstruData;
			pstruBuffer->last = p;
		}
		else
		{
			pstruBuffer->pstruPacketlist = pstruData;
			pstruBuffer->last = p;
		}
		break;
	case GET:
	{
		NetSim_PACKET* pstruPacket;
		pstruPacket = pstruBuffer->pstruPacketlist;
		if (nFlag)
		{
			pstruBuffer->pstruPacketlist = pstruPacket->pstruNextPacket;
			if (pstruBuffer->pstruPacketlist == NULL)
				pstruBuffer->last = NULL;
			pstruPacket->pstruNextPacket = NULL;
		}
		return pstruPacket;
		break;
	}
	default:
		printf("\nInvalid selection in Buffer\n");
		break;
	}
	return 0;
}
/**
If the type is ADD, then all the packets will be added in the buffer based on packet priority in the corresponding list
If the type is GET, then the packets to be retrieved in the order of one packet from each list(high priority list,Medium,Normal,Low)
*/
NetSim_PACKET* fn_NetSim_RoundRobin(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData, int nType, int nFlag)
{
	switch (nType)
	{
	case ADD:
	{
		NetSim_PACKET* pstruTmpBufferPacketList;
		NetSim_PACKET* pstruTempTraversePacketList;

		if (pstruBuffer->pstruPacketlist == NULL)
		{
			pstruBuffer->pstruPacketlist = fnpAllocateMemory(5, sizeof * pstruData);
		}
		pstruTmpBufferPacketList = pstruBuffer->pstruPacketlist;
		if (pstruData->nPacketPriority)
		{
			pstruTempTraversePacketList = fnGetList(pstruTmpBufferPacketList, pstruData->nPacketPriority);

			while (pstruTempTraversePacketList->pstruNextPacket != NULL)
				pstruTempTraversePacketList = pstruTempTraversePacketList->pstruNextPacket;
			pstruTempTraversePacketList->pstruNextPacket = pstruData;
		}
		else
		{
			pstruTempTraversePacketList = fnGetList(pstruTmpBufferPacketList, Priority_Low);
			while (pstruTempTraversePacketList->pstruNextPacket != NULL)
				pstruTempTraversePacketList = pstruTempTraversePacketList->pstruNextPacket;
			pstruTempTraversePacketList->pstruNextPacket = pstruData;
		}
		break;
	}
	case GET:
	{
		NetSim_PACKET* pstruTemp;
		NetSim_PACKET* pstruTempList;
		int nFlag1 = 0;
		PACKET_PRIORITY nPacketPriority;
		if (pstruBuffer->pstruPacketlist)
		{
			nPacketPriority = pstruBuffer->pstruPacketlist->nPacketPriority;
		Retry:
			nFlag1++;
			if (nFlag1 == 5)
				return NULL;
			else
			{
				pstruBuffer->pstruPacketlist->nPacketPriority -= Priority_High - Priority_Medium;
				if (pstruBuffer->pstruPacketlist->nPacketPriority <= 0)
					pstruBuffer->pstruPacketlist->nPacketPriority = Priority_High;

				pstruTempList = fnGetList(pstruBuffer->pstruPacketlist, pstruBuffer->pstruPacketlist->nPacketPriority);
				if (pstruTempList->pstruNextPacket != NULL)
				{
					pstruTemp = pstruTempList->pstruNextPacket;
					if (nFlag)
					{
						pstruTempList->pstruNextPacket = pstruTemp->pstruNextPacket;
						pstruTemp->pstruNextPacket = NULL;
					}
					else
						pstruBuffer->pstruPacketlist->nPacketPriority = nPacketPriority;
					return pstruTemp;
				}
				else
					goto Retry;
			}
		}
		else
			return NULL;
	}
	default:
		printf("\nInvalid Selection in Buffer\n");
		break;
	}
	return 0;
}

/**
If the type is ADD, then all the packets will be added in the buffer based on packet priority in the corresponding list
If the type is GET, then the packets to be retrieved in the order of maximum weight priority list(High,Medium,Normal,Low)
*/
NetSim_PACKET* fn_NetSim_WFQ(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData, int nType, int nFlag)
{
	switch (nType)
	{
	case ADD:
	{
		NetSim_PACKET* pstruTmpBufferPacketList;
		NetSim_PACKET* pstruTempTraversePacketList;

		if (pstruBuffer->pstruPacketlist == NULL)
		{
			pstruBuffer->pstruPacketlist = fnpAllocateMemory(5, sizeof * pstruData);
		}
		pstruTmpBufferPacketList = pstruBuffer->pstruPacketlist;
		if (pstruData->nPacketPriority)
		{
			pstruTempTraversePacketList = fnGetList(pstruTmpBufferPacketList, pstruData->nPacketPriority);

			pstruTempTraversePacketList->nPacketId++;

			while (pstruTempTraversePacketList->pstruNextPacket != NULL)
				pstruTempTraversePacketList = pstruTempTraversePacketList->pstruNextPacket;
			pstruTempTraversePacketList->pstruNextPacket = pstruData;
		}
		else
		{
			pstruTempTraversePacketList = fnGetList(pstruTmpBufferPacketList, Priority_Low);

			pstruTempTraversePacketList->nPacketId++;

			while (pstruTempTraversePacketList->pstruNextPacket != NULL)
				pstruTempTraversePacketList = pstruTempTraversePacketList->pstruNextPacket;
			pstruTempTraversePacketList->pstruNextPacket = pstruData;
		}

		break;
	}
	case GET:
	{
		UINT64 nCount[4], nPosition;
		NetSim_PACKET* pstruTempList;
		NetSim_PACKET* pstruTemp;
		NetSim_PACKET* pstruHighPriorityList;
		NetSim_PACKET* pstruMediumPriorityList;
		NetSim_PACKET* pstruNormalPriorityList;
		NetSim_PACKET* pstruLowPriorityList;

		pstruLowPriorityList = fnGetList(pstruBuffer->pstruPacketlist, Priority_Low);
		nCount[0] = pstruLowPriorityList->nPacketId * 1;

		pstruNormalPriorityList = fnGetList(pstruBuffer->pstruPacketlist, Priority_Normal);
		nCount[1] = pstruNormalPriorityList->nPacketId * 2;

		pstruMediumPriorityList = fnGetList(pstruBuffer->pstruPacketlist, Priority_Medium);
		nCount[2] = pstruMediumPriorityList->nPacketId * 3;

		pstruHighPriorityList = fnGetList(pstruBuffer->pstruPacketlist, Priority_High);
		nCount[3] = pstruHighPriorityList->nPacketId * 4;

		nPosition = fn_NetSim_GetPosition_MaximumNumber(nCount);

		pstruTempList = pstruBuffer->pstruPacketlist + nPosition;

		if (pstruTempList && pstruTempList->pstruNextPacket)
		{
			pstruTempList->nPacketId--;
			pstruTemp = pstruTempList->pstruNextPacket;
			if (nFlag)
			{
				pstruTempList->pstruNextPacket = pstruTemp->pstruNextPacket;
				pstruTemp->pstruNextPacket = NULL;
			}
			else
				pstruTempList->nPacketId++;
			return pstruTemp;
		}
		else
			return NULL;
	}
	default:
		printf("\nInvalid Selection in Buffer\n");
		break;
	}
	return 0;
}

/** Helper function for EDF Scheduling Technique **/
static void edf_algorithm_drop_expired_packets(NetSim_BUFFER* pstruBuffer)
{
	ptrSCHEDULING_EDF_VAR scheduling_var = pstruBuffer->schedulingParam;

	NetSim_PACKET* pstruPacket_prev;
	NetSim_PACKET* pstruPacket_itr;
	NetSim_PACKET* pstruPacket_drop;
	int index;
	double curr_max_latency;
	double curr_deadline;

	pstruPacket_itr = pstruBuffer->pstruPacketlist;
	pstruPacket_prev = pstruBuffer->pstruPacketlist;

	while (pstruPacket_itr)
	{
		index = pstruPacket_itr->nQOS - 1;
		if (index != -1)
		{
			curr_max_latency = *(scheduling_var->max_latency + index) * 1000;
			curr_deadline = curr_max_latency - (pstruEventDetails->dEventTime -
				pstruPacket_itr->dEventTime);
			if (curr_deadline < 0)
			{
				//drop packet from buffer
				pstruPacket_drop = pstruPacket_itr;
				if (pstruBuffer->pstruPacketlist == pstruPacket_itr)
				{
					pstruPacket_itr = pstruPacket_itr->pstruNextPacket;
					pstruPacket_prev = pstruPacket_itr;
					pstruBuffer->pstruPacketlist = pstruPacket_itr;
					if (pstruPacket_itr == NULL)
						pstruBuffer->last = NULL;
				}
				else
				{
					pstruPacket_itr = pstruPacket_itr->pstruNextPacket;
					pstruPacket_prev->pstruNextPacket = pstruPacket_itr;
					if (pstruPacket_itr == NULL)
						pstruBuffer->last = pstruPacket_prev;
				}
				fn_NetSim_DropPackets(pstruBuffer, pstruPacket_drop);
				continue;
			}
		}
		pstruPacket_prev = pstruPacket_itr;
		pstruPacket_itr = pstruPacket_itr->pstruNextPacket;
	}
}

/** Helper function for EDF Scheduling Technique **/
static NetSim_PACKET* edf_algorithm_get_earliest_deadline_packet(NetSim_BUFFER* pstruBuffer)
{
	ptrSCHEDULING_EDF_VAR scheduling_var = pstruBuffer->schedulingParam;

	NetSim_PACKET* pstruPacket_itr;
	NetSim_PACKET* pstruPacket = NULL;
	int index;
	double curr_max_latency;
	double curr_deadline;
	double earliest_deadline = (double)0xFFFFFFFFFFFFFFFF;

	pstruPacket_itr = pstruBuffer->pstruPacketlist;
	while (pstruPacket_itr)
	{
		index = pstruPacket_itr->nQOS - 1;
		if (index == -1)
			return pstruPacket_itr;

		curr_max_latency = *(scheduling_var->max_latency + index) * 1000;
		curr_deadline = curr_max_latency - (pstruEventDetails->dEventTime -
			pstruPacket_itr->dEventTime);
		if (curr_deadline < earliest_deadline)
		{
			pstruPacket = pstruPacket_itr;
			earliest_deadline = curr_deadline;
		}
		pstruPacket_itr = pstruPacket_itr->pstruNextPacket;
	}
	return pstruPacket;
}

/**
If the type is ADD, then all the packets will be added in the buffer based on arrival
If the type is GET, then the expired packets are dropped from buffer and earliest deadline packet is served from the buffer
*/
NetSim_PACKET* fn_NetSim_EDF(NetSim_BUFFER* pstruBuffer, NetSim_PACKET* pstruData,
	int nType, int nFlag)
{
	ptrSCHEDULING_EDF_VAR scheduling_var = pstruBuffer->schedulingParam;
	NetSim_PACKET* p = pstruData;
	while (p && p->pstruNextPacket)
		p = p->pstruNextPacket;

	switch (nType)
	{
	case ADD:
		if (pstruBuffer->pstruPacketlist)
		{
			pstruBuffer->last->pstruNextPacket = pstruData;
			pstruBuffer->last = p;
		}
		else
		{
			pstruBuffer->pstruPacketlist = pstruData;
			pstruBuffer->last = p;
		}
		break;
	case GET:
	{
		NetSim_PACKET* pstruPacket;
		edf_algorithm_drop_expired_packets(pstruBuffer);
		pstruPacket = edf_algorithm_get_earliest_deadline_packet(pstruBuffer);

		if (nFlag && pstruPacket != NULL)
		{
			NetSim_PACKET* pstruPacket_itr = pstruBuffer->pstruPacketlist;

			if (pstruPacket_itr == pstruPacket)
			{
				pstruBuffer->pstruPacketlist = pstruPacket->pstruNextPacket;
				if (pstruBuffer->pstruPacketlist == NULL)
					pstruBuffer->last = NULL;
			}
			else
			{
				while (pstruPacket_itr->pstruNextPacket != pstruPacket)
					pstruPacket_itr = pstruPacket_itr->pstruNextPacket;
				pstruPacket_itr->pstruNextPacket = pstruPacket->pstruNextPacket;
				if (pstruPacket_itr->pstruNextPacket == NULL)
					pstruBuffer->last = pstruPacket_itr;
			}
			pstruPacket->pstruNextPacket = NULL;
		}
		return pstruPacket;
		break;
	}
	default:
		printf("\nInvalid selection in Buffer\n");
		break;
	}
	return 0;
}
