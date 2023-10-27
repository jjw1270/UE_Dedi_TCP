// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BufferArchive.h"

/**
 * 
 */
UENUM()
enum class EClientLoginPacket
{
	None														= 0,

	S2C_Ping													= 1,
	C2S_Ping													= 2,

	S2C_CastMessage												= 3,
	//C2S_CastMessage											= 4,  //reserved

	S2C_ConnectSuccess											= 100,

	C2S_ReqSignIn												= 1000,
	S2C_ResSignIn_Success										= 1001,
	S2C_ResSignIn_Fail_InValidID								= 1002,
	S2C_ResSignIn_Fail_AlreadySignIn							= 1003,

	C2S_ReqSignUpIDPwd											= 1010,
	S2C_ResSignUpIDPwd_Success									= 1011,
	S2C_ResSignUpIDPwd_Fail_ExistID								= 1012,

	C2S_ReqSignUpNickName										= 1020,
	S2C_ResSignUpNickName_Success								= 1021,
	S2C_ResSignUpNickName_Fail_ExistNickName					= 1022,

	Max,
};

struct FClientLoginPacketData
{
	FClientLoginPacketData() : PacketType(EClientLoginPacket::None), Payload() {}
	FClientLoginPacketData(EClientLoginPacket NewPacketType) : PacketType(NewPacketType), Payload() {}
	FClientLoginPacketData(EClientLoginPacket NewPacketType, FString Payload) : PacketType(NewPacketType), Payload(Payload) {}
	FClientLoginPacketData(uint16_t NewPacketTypeInt, FString Payload) : PacketType(static_cast<EClientLoginPacket>(NewPacketTypeInt)), Payload(Payload) {}

	EClientLoginPacket PacketType;
	FString Payload;
};

class TCPSTUDY1_API PacketMaker
{
protected:
	static const int32 DefaultBufferSize = 4;

public:
	// Use this PacketMaker if does not have payload to send
	static TArray<uint8_t> MakePacket(const EClientLoginPacket& PacketType);

	// Use this PacketMaker if have payload to send
	static TArray<uint8_t> MakePacket(const EClientLoginPacket& PacketType, const FString& Payload);

protected:
	static TArray<uint8_t> MakeHeader(const EClientLoginPacket& PacketType, const uint16_t& PayloadSize);
};
