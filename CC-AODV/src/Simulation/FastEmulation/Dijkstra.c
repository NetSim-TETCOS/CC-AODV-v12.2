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
* Author:    Kumar Gaurav   	                                                    *
*										                                            *
* ----------------------------------------------------------------------------------*/

#pragma region HEADER_FILES
#include "main.h"
#include "FastEmulation.h"
#include "../Application/Application.h"
#pragma endregion

#pragma region STRCUT_AND_DEFINE
#define V 500
#define INT_MAX_VALUE 100000

typedef struct Path_list {
	int d;
	int in;
	struct Path_list* next;
}*ptrPath_list;
ptrPath_list list = NULL;

typedef struct stru_Link_Status_info {
	NETSIM_ID id;
	LINK_STATE state;
	NETSIM_ID src;
	NETSIM_ID dest;
	double LinkDownlinkSpeed;
	struct stru_Link_Status_info* next;
}ink_Status_info, * ptrink_Status_info;

typedef struct stru_Link_info {
	double time;
	ptrink_Status_info status;
	struct stru_Link_info* next;
}Link_info, * ptrLink_info;
ptrLink_info link_info = NULL;
#pragma endregion

#pragma region FILE_WRITING_API
static FILE* fplog = NULL;
static void init_Fast_Emulation_Route()
{
		char str[BUFSIZ];
		sprintf(str, "%s/%s", pszIOPath, "FastEmulation.route");
		fplog = fopen(str, "w");
		if (!fplog)
		{
			perror(str);
			fnSystemError("Unable to open FastEmulation.route file");
		}
}

static void close_Fast_Emulation_Route()
{
	fclose(fplog);
}

static void print_Fast_Emulation_Route(char* format, ...)
{
	if (fplog)
	{
		va_list l;
		va_start(l, format);
		vfprintf(fplog, format, l);
		fflush(fplog);
	}
}
#pragma endregion

#pragma region NODE_CREATION_ADDITION
static ptrPath_list createNode_list(int d) {
	ptrPath_list temp = calloc(1, sizeof * temp);
	temp->d = d;
	temp->in = 0;
	temp->next = NULL;
	return temp;
}

static ptrLink_info createNode_link(double time) {
	ptrLink_info temp = calloc(1, sizeof * temp);
	temp->time = time;
	temp->next = NULL;
	return temp;
}

static ptrink_Status_info createNode_Status(NETSIM_ID id, NETSIM_ID src, NETSIM_ID dest, double Linkspeed, LINK_STATE state) {
	ptrink_Status_info temp = calloc(1, sizeof * temp);
	temp->id = id;
	temp->src = src;
	temp->dest = dest;
	temp->state = state;
	temp->LinkDownlinkSpeed = Linkspeed;
	temp->next = NULL;
	return temp;
}

static void addNode_link(ptrLink_info temp) {
	if (link_info == NULL)
		link_info = temp;
	else
	{
		ptrLink_info temp1 = link_info;
		while (temp1->next)
			temp1 = temp1->next;
		temp1->next = temp;
	}
}

static void addNode(ptrPath_list temp) {
	if (list == NULL)
		list = temp;
	else
	{
		ptrPath_list temp1 = list;
		while (temp1->next)
			temp1 = temp1->next;

		temp1->next = temp;
	}
}
#pragma endregion

#pragma region PRINT_API
static int minDistance(int dist[],
	bool sptSet[])
{

	// Initialize min value 
	int min = INT_MAX, min_index = 0;

	for (int v = 0; v < V; v++)
		if (sptSet[v] == false &&
			dist[v] <= min)
			min = dist[v], min_index = v;

	return min_index;
}

static void printList() {
	ptrPath_list temp = list;
	printf("-------------\n");
	while (temp) {
		printf("%d ", temp->d);
		temp = temp->next;
	}
}

static void printPath(int parent[], int j)
{

	if (parent[j] == -1)
		return;

	printPath(parent, parent[j]);

	ptrPath_list temp = createNode_list(j + 1);
	addNode(temp);

}

static int printSolution_list(int dist[],
	int parent[], int src, int dest)
{
	if (dist[dest] != INT_MAX_VALUE) {
		ptrPath_list temp = createNode_list(src + 1);
		addNode(temp);
		printPath(parent, dest);
	}
	else {
		print_Fast_Emulation_Route("No_Path\n");
	}
	return 0;
}
#pragma endregion

#pragma region DIJKSTRA_ALGO
static void dijkstra_algo(int graph[V][V], int src, int dest)
{
	int dist[V];
	bool sptSet[V];
	int parent[V];

	for (int i = 0; i < V; i++)
	{
		parent[i] = -1;
		dist[i] = INT_MAX_VALUE;
		sptSet[i] = false;
	}

	dist[src] = 0;

	for (int count = 0; count < V - 1; count++)
	{
		int u = minDistance(dist, sptSet);
		sptSet[u] = true;
		for (int v = 0; v < V; v++)

			if (!sptSet[v] && graph[u][v] &&
				dist[u] + graph[u][v] < dist[v])
			{
				parent[v] = u;
				dist[v] = dist[u] + graph[u][v];
			}
	}

	printSolution_list(dist, parent, src, dest);
}
#pragma endregion

#pragma region ADD_INTERFACE_AND_FILE_WRITING
static NETSIM_ID Add_interface(int d, int r) {
	int src, dest;
	struct stru_NetSim_Links* links = *NETWORK->ppstruNetSimLinks;
	while (links != NULL) {
		if (links->nLinkType == LinkType_P2P) {
			src = links->puniDevList.pstruP2P.nFirstDeviceId;
			dest = links->puniDevList.pstruP2P.nSecondDeviceId;
			if (d == src && r == dest) {
				return links->puniDevList.pstruP2P.nFirstInterfaceId;
			}
			if (r == src && d == dest) {
				return links->puniDevList.pstruP2P.nSecondInterfaceId;
			}
		}
		else if (links->nLinkType == LinkType_P2MP) {
			src = links->puniDevList.pstrup2MP.nCenterDeviceId;
			for (NETSIM_ID i = 0; i < links->puniDevList.pstrup2MP.nConnectedDeviceCount - 1; i++) {
				dest = links->puniDevList.pstrup2MP.anDevIds[i];
				if (d == src && r == dest) {
					return links->puniDevList.pstrup2MP.nCenterInterfaceId;
				}
				if (r == src && d == dest) {
					return links->puniDevList.pstrup2MP.anDevInterfaceIds[i];
				}
			}
		}
		else if (links->nLinkType == LinkType_MP2MP) {
			for (NETSIM_ID i = 0; i < links->puniDevList.pstruMP2MP.nConnectedDeviceCount; i++) {
				src = links->puniDevList.pstruMP2MP.anDevIds[i];
				for (NETSIM_ID j = 0; j < links->puniDevList.pstruMP2MP.nConnectedDeviceCount; j++) {
					dest = links->puniDevList.pstruMP2MP.anDevIds[j];
					if (src != dest) {
						if (src == d && dest == r) {
							return links->puniDevList.pstruMP2MP.anDevInterfaceIds[i];
						}
						if (src == r && dest == d) {
							return links->puniDevList.pstruMP2MP.anDevInterfaceIds[j];
						}
					}

				}
			}
		}

		links = links->pstruNextLink;
	}
	return 0;
}

static bool FirstNode(ptrPath_list temp) {
	if (temp == list)
		return true;
	return false;
}

static bool LastNode(ptrPath_list temp) {
	if (temp->next == NULL)
		return true;
	return false;
}

static void Writing_file() {
	ptrPath_list temp = list;
	ptrPath_list prev = list;
	while (temp) {
		if (FirstNode(temp)) {
			print_Fast_Emulation_Route("%d:%d;", temp->d, Add_interface(temp->d, temp->next->d));
		}
		else if (LastNode(temp)) {
			print_Fast_Emulation_Route("%d:%d;\n", temp->d, Add_interface(temp->d, prev->d));
		}
		else {
			print_Fast_Emulation_Route("%d:%d;", temp->d, Add_interface(temp->d, prev->d));
			print_Fast_Emulation_Route("%d:%d;", temp->d, Add_interface(temp->d, temp->next->d));
		}
		prev = temp;
		temp = temp->next;
	}
}
#pragma endregion  

#pragma region FREE_SORT_CREATE_LIST
static void freeList() {
	ptrPath_list temp = list;
	ptrPath_list temp1 = NULL;
	while (temp) {
		temp1 = temp;
		temp = temp->next;
		temp1->next = NULL;
		free(temp1);
	}
	list = NULL;
}

static void SortingofLink_info()
{
	ptrLink_info list = link_info;
	ptrLink_info head = NULL;
	ptrLink_info tail = NULL;
	ptrLink_info t = NULL;
	while (list)
	{
		if (!head)
		{
			//first member
			head = list;
			tail = list;
			list = list->next;
			head->next = NULL;
			continue;
		}

		if (list->time <= head->time)
		{
			// Rank is less and head rank. So place at head
			t = list->next;
			list->next = head;
			head = list;
			list = t;
			continue;
		}

		if (list->time >= tail->time)
		{
			// Rank is more than tail rank, so place as tail
			t = list->next;
			tail->next = list;
			tail = list;
			list = t;
			tail->next = NULL;
			continue;
		}

		//Middle
		t = list->next;
		ptrLink_info info = head;
		while (info)
		{
			if (info->next->time > list->time)
			{
				list->next = info->next;
				info->next = list;
				break;
			}
			info = info->next;
		}
		list = t;
	}
	link_info =  head;
}

static ptrLink_info isNodeExist(double time) {
	ptrLink_info temp = link_info;
	while (temp) {
		if (temp->time == time)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

static void Make_Link() {
	struct stru_NetSim_Links* links = *NETWORK->ppstruNetSimLinks;
	ptrLink_info temp = NULL;
	ptrink_Status_info status_temp = NULL;
	while (links != NULL)
	{
		if (links->nLinkType == LinkType_P2P) {
			int src = links->puniDevList.pstruP2P.nFirstDeviceId;
			int dest = links->puniDevList.pstruP2P.nSecondDeviceId;
			double LinkSpeed = links->puniMedProp.pstruWiredLink.dDataRateDown;
			for (NETSIM_ID i = 0; i < links->struLinkFailureModel.nLinkDownCount; i++) {
				if (link_info == NULL) {
					temp = createNode_link(links->struLinkFailureModel.dLinkDownTime[i]);
					status_temp = createNode_Status(links->nLinkId, src, dest, LinkSpeed, LINKSTATE_DOWN);
					temp->status = status_temp;
					status_temp = NULL;
					addNode_link(temp);
				}
				else {
					temp = isNodeExist(links->struLinkFailureModel.dLinkDownTime[i]);
					if (temp != NULL) {
						status_temp = createNode_Status(links->nLinkId, src, dest, LinkSpeed, LINKSTATE_DOWN);
						status_temp->next = temp->status;
						temp->status = status_temp;
						status_temp = NULL;
					}
					else {
						temp = createNode_link(links->struLinkFailureModel.dLinkDownTime[i]);
						status_temp = createNode_Status(links->nLinkId, src, dest, LinkSpeed, LINKSTATE_DOWN);
						temp->status = status_temp;
						status_temp = NULL;
						addNode_link(temp);
					}
				}
			}
			for (NETSIM_ID i = 0; i < links->struLinkFailureModel.nLinkUpCount; i++) {
				if (link_info == NULL) {
					temp = createNode_link(links->struLinkFailureModel.dLinkUPTime[i]);
					status_temp = createNode_Status(links->nLinkId, src, dest, LinkSpeed, LINKSTATE_UP);
					temp->status = status_temp;
					status_temp = NULL;
					addNode_link(temp);
				}
				else {
					temp = isNodeExist(links->struLinkFailureModel.dLinkUPTime[i]);
					if (temp != NULL) {
						status_temp = createNode_Status(links->nLinkId, src, dest, LinkSpeed, LINKSTATE_UP);
						status_temp->next = temp->status;
						temp->status = status_temp;
						status_temp = NULL;
					}
					else {
						temp = createNode_link(links->struLinkFailureModel.dLinkUPTime[i]);
						status_temp = createNode_Status(links->nLinkId, src, dest, LinkSpeed, LINKSTATE_UP);
						temp->status = status_temp;
						status_temp = NULL;
						addNode_link(temp);
					}
				}
			}
		}
		links = links->pstruNextLink;
	}
	SortingofLink_info();
}

static void free_link_info() {
	ptrLink_info temp = link_info;
	ptrLink_info temp1 = NULL;
	ptrink_Status_info status = NULL;
	ptrink_Status_info status_temp = NULL;
	while (temp) {
		status = temp->status;
		while (status) {
			status_temp = status;
			status = status->next;
			status_temp->next = NULL;
			free(status_temp);
		}
		temp1 = temp;
		temp = temp->next;
		temp1->next = NULL;
		free(temp1);
	}
	link_info = NULL;
}
#pragma endregion

#pragma region MAIN_FUN_TO_CALL
int fastemulation_Route_file()
{

	init_Fast_Emulation_Route();

	int graph[V][V] = { 0 };
	int src, dest;
	struct stru_NetSim_Links* links = *NETWORK->ppstruNetSimLinks;
	Make_Link();
	while (links != NULL)
	{
		if (links->nLinkType == LinkType_P2P) {
			src = links->puniDevList.pstruP2P.nFirstDeviceId;
			dest = links->puniDevList.pstruP2P.nSecondDeviceId;
			graph[src - 1][dest - 1] = (int)links->puniMedProp.pstruWiredLink.dDataRateDown;
			graph[dest - 1][src - 1] = (int)links->puniMedProp.pstruWiredLink.dDataRateDown;
		}
		else if (links->nLinkType == LinkType_P2MP) {
			src = links->puniDevList.pstrup2MP.nCenterDeviceId;
			for (NETSIM_ID i = 0; i < links->puniDevList.pstrup2MP.nConnectedDeviceCount - 1; i++) {
				dest = links->puniDevList.pstrup2MP.anDevIds[i];
				graph[src - 1][dest - 1] = 1;
				graph[dest - 1][src - 1] = 1;
			}
		}
		else if (links->nLinkType == LinkType_MP2MP) {
			for (NETSIM_ID i = 0; i < links->puniDevList.pstruMP2MP.nConnectedDeviceCount; i++) {
				src = links->puniDevList.pstruMP2MP.anDevIds[i];
				for (NETSIM_ID j = 0; j < links->puniDevList.pstruMP2MP.nConnectedDeviceCount; j++) {
					dest = links->puniDevList.pstruMP2MP.anDevIds[j];
					if (src != dest) {
						graph[src - 1][dest - 1] = 1;
						graph[dest - 1][src - 1] = 1;
					}

				}
			}
		}

		links = links->pstruNextLink;
	}
	
	ptrLink_info temp = link_info;
	while (temp) {
		while (temp->status) {
			if (temp->status->state == LINKSTATE_UP) {
				graph[temp->status->src - 1][temp->status->dest - 1] = (int)temp->status->LinkDownlinkSpeed;
				graph[temp->status->dest - 1][temp->status->src - 1] = (int)temp->status->LinkDownlinkSpeed;
			}
			else {
				graph[temp->status->src - 1][temp->status->dest - 1] = 0;
				graph[temp->status->dest - 1][temp->status->src - 1] = 0;
			}
			temp->status = temp->status->next;
		}
		print_Fast_Emulation_Route("Time = %lf\n", temp->time);
		for (UINT i = 0; i < NETWORK->nApplicationCount; i++)
		{
			ptrAPPLICATION_INFO info = (ptrAPPLICATION_INFO)NETWORK->appInfo[i];
			dijkstra_algo(graph, info->sourceList[0] - 1, info->destList[0] - 1);
			Writing_file();
			freeList();
		}
		temp = temp->next;
	}
	free_link_info();
	close_Fast_Emulation_Route();
	return 0;
}
#pragma endregion
