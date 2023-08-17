#pragma once
/************************************************************************************
 * Copyright (C) 2018
 * TETCOS, Bangalore. India															*

 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all  *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 * Author:	Shashi Kant Suman														*
 * ---------------------------------------------------------------------------------*/
#ifndef _NETSIM_LTENR_BUFFER_H_
#define _NETSIM_LTENR_BUFFER_H_

#ifdef  __cplusplus
extern "C" {
#endif

	typedef void* ptrLTENR_BUFFERITERATOR;
#define INFINITE_BUFFER 0xFFFFFFFFFFFFFFFF

	typedef struct stru_LTENR_BufferPacket
	{
		NetSim_PACKET* packet;
		UINT64 size;
		double time;
		void* optionalInfo;

		bool isMarked;
		struct stru_LTENR_BufferPacket* next;
	}LTENR_BUFFEREDPACKET, *ptrLTENR_BUFFEREDPACKET;

	typedef struct stru_LTENR_BUFFER
	{
		UINT64 maxSize;
		UINT64 currentSize;
		ptrLTENR_BUFFEREDPACKET headPacket;
		ptrLTENR_BUFFEREDPACKET tailPacket;

		void* iterator;
	}LTENR_BUFFER, *ptrLTENR_BUFFER;

#define LTENR_BUFFER_ISEMPTY(buffer) (buffer != NULL && buffer->headPacket == NULL)
#define LTENR_BUFFER_HASPACKET(buffer) (buffer != NULL && buffer->headPacket != NULL)

	//Function  prototype
	ptrLTENR_BUFFER LTENR_BUFFER_INIT(UINT64 maxSize/*In Bytes*/);
	void LTENR_BUFFER_DELETE(ptrLTENR_BUFFER buffer);

	bool LTENR_BUFFER_ADDPACKET(ptrLTENR_BUFFER buffer,
								NetSim_PACKET* packet,
								UINT64 size,
								double time,
								void* optional);
	bool LTENR_BUFFER_ADDPACKET_SORTED(ptrLTENR_BUFFER buffer,
									   NetSim_PACKET* packet,
									   UINT64 size,
									   double time,
									   void* optional,
									   bool(*fnSort)(ptrLTENR_BUFFEREDPACKET currBuffered,
													 ptrLTENR_BUFFEREDPACKET newBuffered));
	NetSim_PACKET* LTENR_BUFFER_GETPACKET(ptrLTENR_BUFFER buffer,
										  UINT64* size /* optional*/,
										  double* time /* optional*/,
										  void** optional /* optional*/);
	NetSim_PACKET* LTENR_BUFFER_HEADPACKET(ptrLTENR_BUFFER buffer,
										   UINT64* size /* optional*/,
										   double* time /* optional*/,
										   void** optional /* optional*/);

	//Buffer Iterator
	ptrLTENR_BUFFERITERATOR _LTENR_BUFFER_NEWITERATOR_(ptrLTENR_BUFFER buffer, int line, char* file);
#define LTENR_BUFFER_NEWITERATOR(buffer) (_LTENR_BUFFER_NEWITERATOR_((buffer),__LINE__,__FILE__))

	void LTENR_BUFFER_DELETEITERATOR(ptrLTENR_BUFFERITERATOR* it);
	NetSim_PACKET* LTENR_BUFFER_ITERATOR_NEXT(ptrLTENR_BUFFERITERATOR it);
	void LTENR_BUFFER_ITERATOR_REMOVE(ptrLTENR_BUFFERITERATOR it);
	NetSim_PACKET* LTENR_BUFFER_ITERATOR_CURRENT(ptrLTENR_BUFFERITERATOR it);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif //_NETSIM_LTENR_BUFFER_H_