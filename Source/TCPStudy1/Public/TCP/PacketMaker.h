// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BufferArchive.h"

/**
 * 
 */
UENUM()
enum class EPacket
{
	None = 0,

	S2C_Ping = 1,
	C2S_Ping = 2,

	S2C_CastMessage = 3,
	//C2S_CastMessage = 4,  //reserved


	S2C_Login_UserIDReq = 100,
	C2S_Login_UserIDAck = 101,

	S2C_Login_UserIDFailureReq = 102,
	C2S_Login_UserIDReq = 103,
	C2S_Login_MakeNewUserReq = 110,

	S2C_Login_NewUserNickNameReq = 111,
	C2S_Login_NewUserNickNameAck = 112,

	S2C_Login_NewUserPwdReq = 113,
	C2S_Login_NewUserPwdAck = 114,

	S2C_Login_UserPwdReq = 120,
	C2S_Login_UserPwdAck = 121,

	S2C_Login_UserPwdFailureReq = 122,
	C2S_Login_UserPwdReq = 123,

	S2C_LoginSuccess = 150,
	S2C_AlreadyLoginOnServer = 151,

	S2C_CanChat = 200,
	C2S_Chat = 201,
	S2C_Chat = 202,

	Max,
};

struct FPacketData
{
	FPacketData() : PacketType(EPacket::None), Payload() {}
	FPacketData(EPacket NewPacketType) : PacketType(NewPacketType), Payload() {}
	FPacketData(EPacket NewPacketType, FString Payload) : PacketType(NewPacketType), Payload(Payload) {}
	FPacketData(uint16_t NewPacketTypeInt, FString Payload) : PacketType(static_cast<EPacket>(NewPacketTypeInt)), Payload(Payload) {}

	EPacket PacketType;
	FString Payload;
};

class TCPSTUDY1_API PacketMaker
{
protected:
	static const int32 DefaultBufferSize = 4;

public:
	// Use this PacketMaker if does not have payload to send
	static TArray<uint8_t> MakePacket(const EPacket& PacketType);

	// Use this PacketMaker if have payload to send
	static TArray<uint8_t> MakePacket(const EPacket& PacketType, const FString& Payload);

protected:
	static TArray<uint8_t> MakeHeader(const EPacket& PacketType, const uint16_t& PayloadSize);
};
