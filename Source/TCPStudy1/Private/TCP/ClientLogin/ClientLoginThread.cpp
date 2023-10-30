// Fill out your copyright notice in the Description page of Project Settings.


#include "TCP/ClientLogin/ClientLoginThread.h"
#include "TCPStudy1.h"
#include "ClientLogin/ClientLoginSubsystem.h"

FClientLoginThread::FClientLoginThread(UClientLoginSubsystem*& NewClientLoginSubsystem)
	: ClientLoginSubsystem(NewClientLoginSubsystem)
{
	bStopThread = false;
}

uint32 FClientLoginThread::Run()
{
    while (!bStopThread)
    {
		FLoginPacketData PacketData;
		bool RecvByte = ClientLoginSubsystem->Recv(PacketData);
		if (!RecvByte)
		{
			ABLOG(Error, TEXT("Recv Error, Stop Thread"));
			break;
		}

		if (PacketData.PacketType != ELoginPacket::None)
		{
			ABLOG_S(Warning);
			ClientLoginSubsystem->ProcessPacket(PacketData);
		}
    }

    return 0;
}

void FClientLoginThread::StopThread()
{
	bStopThread = true;
}
