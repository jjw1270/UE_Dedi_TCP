// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TCP/Packets.h"

// < For profiling >
// Declare performance statistics data for a specific group
// Used for performance profiling and statistics collection
// Params : Name, Identifier, Category
DECLARE_STATS_GROUP(TEXT("SocketManager"), STATGROUP_SocketManager, STATCAT_Advanced);
// Declare performance statistics data
// Used to measure the execution time of a particular task or function
// Parmas : Name, Identifier, Group
DECLARE_CYCLE_STAT(TEXT("Send"), STAT_Send, STATGROUP_SocketManager);
DECLARE_CYCLE_STAT(TEXT("Recv"), STAT_Recv, STATGROUP_SocketManager);

class FSocket;

/**
 * 
 */
class TCPSTUDY1_API SocketManager
{

private:
	static bool Connect(FSocket* Socket, FString IPAddress, int PortNumber);

	static bool Receive(FSocket* Socket, uint8* Results, int32 Size);

	static bool Send(FSocket* Socket, const uint8* Buffer, int32 Size);

	static bool SendPacket(FSocket* Socket, const EPacket& PacketType, const uint8* Payload, const int32& PayloadSize);

	static bool ReceivePacket(FSocket* Socket, TArray<uint8>& OutPayload);

	static void PrintSocketError(const FString& Text);

public:
	void Connect();

	void DestroySocket();

	void Send(const EPacket& PacketType, const FString& MessageToSend);

	void Recv();

private:
	FSocket* Socket;

};
