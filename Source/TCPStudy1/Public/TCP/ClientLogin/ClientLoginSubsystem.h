// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ClientLoginSubsystem.generated.h"

/**
 * 
 */

const int32 HeaderSize{ 4 };

UCLASS()
class TCPSTUDY1_API UClientLoginSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UClientLoginSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
	
public:
	bool Connect(const int32& PortNum, const FString& IP);

	void DestroySocket();

	bool Recv(struct FLoginPacketData& OutRecvPacket);

	bool Send(const struct FLoginPacketData& SendPacket);

	bool IsConnect();

protected:
	void PrintSocketError(const FString& Text);

private:
	FSocket* Socket;

};

enum class ELoginPacket
{
	None											= 0,

	S2C_Ping										= 1,
	C2S_Ping										= 2,

	S2C_CastMessage									= 3,
	//C2S_CastMessage								= 4,  //reserved

	S2C_ConnectSuccess								= 100,

	C2S_ReqSignIn									= 1000,
	S2C_ResSignIn_Success							= 1001,
	S2C_ResSignIn_Fail_InValidID					= 1002,
	S2C_ResSignIn_Fail_AlreadySignIn				= 1003,

	C2S_ReqSignUpIDPwd								= 1010,
	S2C_ResSignUpIDPwd_Success						= 1011,
	S2C_ResSignUpIDPwd_Fail_ExistID					= 1012,

	C2S_ReqSignUpNickName							= 1020,
	S2C_ResSignUpNickName_Success					= 1021,
	S2C_ResSignUpNickName_Fail_ExistNickName		= 1022,
	
	Max,
};

struct FLoginPacketData
{
public:
	FLoginPacketData() : PacketType(ELoginPacket::None), Payload() {}
	FLoginPacketData(ELoginPacket NewPacketType) : PacketType(NewPacketType), Payload() {}
	FLoginPacketData(ELoginPacket NewPacketType, FString Payload) : PacketType(NewPacketType), Payload(Payload) {}
	FLoginPacketData(uint16_t NewPacketTypeInt, FString Payload) : PacketType(static_cast<ELoginPacket>(NewPacketTypeInt)), Payload(Payload) {}

	ELoginPacket PacketType;
	FString Payload;
};
