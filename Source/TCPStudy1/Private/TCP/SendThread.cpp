// Fill out your copyright notice in the Description page of Project Settings.


#include "TCP/SendThread.h"
#include "TCPStudy1.h"
#include "SocketManager.h"

FSendThread::FSendThread(TSharedPtr<FSocketManager> SocketManager)
{
	this->SocketManager = SocketManager;

	Thread = FRunnableThread::Create(this, TEXT("SendThread"));
}

FSendThread::~FSendThread()
{
	if (Thread)
	{
		Thread->WaitForCompletion();
		Thread->Kill();
		delete Thread;
	}
}

void FSendThread::SetSendPacket(const EPacket& PacketType, const FString& Payload)
{
	if (Payload.IsEmpty())
	{
		SendPacket = FPacketData(PacketType);
	}
	else
	{
		SendPacket = FPacketData(PacketType, Payload);
	}
}

bool FSendThread::Init()
{
	ABLOG(Warning, TEXT("Start Send Thread"));

	return true;
}

uint32 FSendThread::Run()
{
	while (true)
	{
		if (SendPacket.PacketType != EPacket::None)
		{
			if (SocketManager->Send(SendPacket))
			{
				ABLOG(Warning, TEXT("Send Type : %d, Send Payload : %s"), static_cast<int32>(SendPacket.PacketType), *SendPacket.Payload);
				SendPacket = FPacketData();
			}
		}
	}

	return 0;
}

void FSendThread::Exit()
{
	SocketManager.Reset();
}
