// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ClientLoginSubsystem.generated.h"

/**
 * 
 */

enum class ELoginPacket
{
	None													= 0,

	S2C_Ping												= 1,
	C2S_Ping												= 2,

	S2C_CastMessage											= 3,
	//C2S_CastMessage										= 4,	//reserved

	S2C_ConnectSuccess										= 100,

	C2S_ReqSignIn											= 1000,
	S2C_ResSignIn_Success									= 1001,
	S2C_ResSignIn_Fail_InValidID							= 1002,
	S2C_ResSignIn_Fail_InValidPassword						= 1003,
	//S2C_ResSignIn_Fail_AlreadySignIn						= 1004,  //reserved

	C2S_ReqSignUpIDPwd										= 1010,
	S2C_ResSignUpIDPwd_Success								= 1011,
	S2C_ResSignUpIDPwd_Fail_ExistID							= 1012,

	C2S_ReqSignUpNickName									= 1020,
	S2C_ResSignUpNickName_Success							= 1021,
	S2C_ResSignUpNickName_Fail_ExistNickName				= 1022,

	C2S_ReqMatchMaking										= 1100,
	S2C_ResMatchMaking_DediIP								= 1101,

	C2S_ReqCancelMatchMaking								= 1110,

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

DECLARE_MULTICAST_DELEGATE_ThreeParams(FDele_RecvPacket, const FString&, const int32&, bool);

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

protected:
	bool Connect(const int32& PortNum, const FString& IP);

/* < Funcs You Can Use > ------------------------------*/
public:
	void ConnectToLoginServer();

	FORCEINLINE void SetRecvPacket(const FLoginPacketData& RecvPacket) { RecvPacketData = RecvPacket; };

	void DestroySocket();

	// DONT USE THIS FUNC!!! -> Use ManageRecvPacket()
	bool Recv(FLoginPacketData& OutRecvPacket);

	bool Send(const FLoginPacketData& SendPacket);

	bool IsConnect();
/*-----------------------------------------------------*/

/* < MultiCast Delegates You Can Use > ----------------*/
public:
	FDele_RecvPacket RecvPacketDelegate;
/*-----------------------------------------------------*/

protected:
	// For Socket Error Log
	void PrintSocketError(const FString& Text);

protected:
/* Your Custom Codes In this Func----------------------*/
	UFUNCTION()
	void ManageRecvPacket();
/*-----------------------------------------------------*/

/* Your Custom Vars In Here----------------------------*/
public:
	FORCEINLINE const FString& GetIDPwd() const { return IDPwd; };
	FORCEINLINE void SetIDPwd(const FString& NewIDPwd) { IDPwd = NewIDPwd; };

	FORCEINLINE const FString& GetUserNickName() const { return UserNickName; };
	FORCEINLINE void SetUserNickName(const FString& NewNickName) { UserNickName = NewNickName; };

protected:
	FString IDPwd;

	FString UserNickName;
/*-----------------------------------------------------*/

protected:
	FSocket* Socket;

	class FClientLoginThread* ClientLoginThread;
	class FRunnableThread* ClientLoginThreadHandle;

	FLoginPacketData RecvPacketData;
	FTimerHandle ManageRecvPacketHandle;
};

class TCPSTUDY1_API FClientLoginThread : public FRunnable
{
public:
	FClientLoginThread(class UClientLoginSubsystem* NewClientLoginSubsystem);

protected:
	virtual uint32 Run() override;

public:
	void StopThread();

private:
	class UClientLoginSubsystem* ClientLoginSubsystem;

private:
	bool bStopThread;

};
