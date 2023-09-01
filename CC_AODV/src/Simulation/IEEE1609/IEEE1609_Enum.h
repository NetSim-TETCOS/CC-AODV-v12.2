/************************************************************************************
* Copyright (C) 2016
* TETCOS, Bangalore. India															*

* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
* Author:	Shashi Kant Suman														*
* ---------------------------------------------------------------------------------*/

#include "EnumString.h"
#include "main.h"
BEGIN_ENUM(IEEE1609_Subevent)
{
	DECL_ENUM_ELEMENT_WITH_VAL(CHANNEL_SWITCH_START, MAC_PROTOCOL_IEEE1609 * 100),
		DECL_ENUM_ELEMENT(CHANNEL_SWITCH_END),
}
END_ENUM(IEEE1609_Subevent);
