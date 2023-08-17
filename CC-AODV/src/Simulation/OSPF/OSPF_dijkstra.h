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
* Author:    Ramsaran Giri					                                        *
*										                                            *
* ----------------------------------------------------------------------------------*/
#ifndef _NETSIM_APP_ROUTING_H_
#define _NETSIM_APP_ROUTING_H_
#ifdef  __cplusplus
extern "C" {
#endif

	// A structure to represent a node in adjacency list
	struct AdjListNode
	{
		int dest;
		int weight;
		int interfaceId;
		struct AdjListNode* next;
	};

	// A structure to represent an adjacency list
	struct AdjList
	{
		struct AdjListNode* head; // pointer to head node of list
	};

	// A structure to represent a graph. A graph is an array of adjacency lists.
	// Size of array will be V (number of vertices in graph)
	struct Graph
	{
		int V;
		struct AdjList* array;
	};

	//A structure to store the shortest path from source to destination
	typedef struct Path
	{
		int node;
		struct Path* next;
	}PATH, * ptrPATH;

	// Structure to represent a min heap node
	typedef struct MinHeapNode
	{
		int v;
		int dist;
	}MINHEAP, * ptrMINHEAP;

	// Structure to represent a min heap
	struct MinHeap
	{
		int size;	 // Number of heap nodes present currently
		int capacity; // Capacity of min heap
		int* pos;	 // This is needed for decreaseKey()
		struct MinHeapNode** array;
	};

#ifdef  __cplusplus
}
#endif
#endif //_NETSIM_APP_ROUTING_H_
