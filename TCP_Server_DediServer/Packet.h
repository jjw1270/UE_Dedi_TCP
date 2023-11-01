#pragma once

#ifndef __PACKET_H__ 
#define __PACKET_H__

enum class EPacket
{
	None													= 0,

	S2C_Ping												= 1,
	C2S_Ping												= 2,

	S2C_CastMessage											= 3,
	//C2S_CastMessage										= 4,  //reserved

	S2C_ConnectSuccess										= 100,

	S2C_ConnectDediSuccess									= 3000,

	C2S_ReqDediTCPConnect									= 8000,

	S2C_ReqAvailableDediServer								= 8100,
	C2S_ResAvailableDediServer								= 8101,

	Max,
};

#pragma pack(1)
#pragma pack()

#endif