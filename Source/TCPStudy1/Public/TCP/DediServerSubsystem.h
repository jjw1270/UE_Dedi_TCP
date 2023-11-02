// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DediServerSubsystem.generated.h"

/**
 *
 */

enum class EDediPacket
{
	None = 0,

	S2C_Ping = 1,
	C2S_Ping = 2,

	S2C_CastMessage = 3,
	//C2S_CastMessage										= 4,  //reserved

	S2C_ConnectSuccess										= 100,

	C2S_ConnectSuccess										= 4000,


	Max,
};

struct FDediPacketData
{
public:
	FDediPacketData() : PacketType(EDediPacket::None), Payload() {}
	FDediPacketData(EDediPacket NewPacketType) : PacketType(NewPacketType), Payload() {}
	FDediPacketData(EDediPacket NewPacketType, FString Payload) : PacketType(NewPacketType), Payload(Payload) {}
	FDediPacketData(uint16_t NewPacketTypeInt, FString Payload) : PacketType(static_cast<EDediPacket>(NewPacketTypeInt)), Payload(Payload) {}

	EDediPacket PacketType;
	FString Payload;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FDele_RecvPacket, const FString&, const int32&);

const int32 HeaderSize{ 4 };
UCLASS()
class TCPSTUDY1_API UDediServerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UDediServerSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

protected:
	bool Connect(const int32& PortNum, const FString& IP);

/* < Funcs You Can Use > ------------------------------*/
public:
	void ConnectToTCPDediServer();

	FORCEINLINE void SetRecvPacket(const FDediPacketData& RecvPacket) { RecvPacketData = RecvPacket; };

	void DestroySocket();

	// DONT USE THIS FUNC!!! -> Use ManageRecvPacket()
	bool Recv(FDediPacketData& OutRecvPacket);

	bool Send(const FDediPacketData& SendPacket);

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
//public:
//	FORCEINLINE const FString& GetIDPwd() const { return IDPwd; };
//	FORCEINLINE void SetIDPwd(const FString& NewIDPwd) { IDPwd = NewIDPwd; };
//
//	FORCEINLINE const FString& GetUserNickName() const { return UserNickName; };
//	FORCEINLINE void SetUserNickName(const FString& NewNickName) { UserNickName = NewNickName; };
//
//protected:
//	FString IDPwd;
//
//	FString UserNickName;
/*-----------------------------------------------------*/

protected:
	FSocket* Socket;

	class FDediServerThread* DediServerThread;
	class FRunnableThread* DediServerThreadHandle;

	FDediPacketData RecvPacketData;
	FTimerHandle ManageRecvPacketHandle;
};

class TCPSTUDY1_API FDediServerThread : public FRunnable
{
public:
	FDediServerThread(class UDediServerSubsystem* NewDediServerSubsystem);

protected:
	virtual uint32 Run() override;

public:
	void StopThread();

private:
	class UDediServerSubsystem* DediServerSubsystem;

private:
	bool bStopThread;

};
