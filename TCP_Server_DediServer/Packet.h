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

	///////////////////////////////////////////////////////////////////////////

	C2S_ReqDediTCPConnect									= 3000,

	S2C_ReqDediServer										= 3100,
	C2S_ResDediServer										= 3101,

	///////////////////////////////////////////////////////////////////////////

	C2S_ConnectSuccess										= 4000,

	///////////////////////////////////////////////////////////////////////////

	Max,
};

#pragma pack(1)
#pragma pack()

#endif