// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketMaker.h"

class FSocket;

/**
 * It should be created in the GameInstance.
 */
class TCPSTUDY1_API FSocketManager
{

private:
	int32 HeaderSize{ 4 };

public:
	bool Connect(const int32& Port);

	bool Recv(FClientLoginPacketData& OutRecvPacket);

	bool Send(const FClientLoginPacketData& ToSendPacket);

	void DestroySocket();

private:
	void PrintSocketError(const FString& Text);

private:
	FSocket* Socket;

};
