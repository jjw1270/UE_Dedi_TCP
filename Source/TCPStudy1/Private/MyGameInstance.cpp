// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "TCPStudy1.h"
#include "RecvThread.h"
#include "SendThread.h"

void UMyGameInstance::Shutdown()
{
    delete RecvThread;
    delete SendThread;

    SocketManager->DestroySocket();
    SocketManager.Reset();
    
    Super::Shutdown();
}

void UMyGameInstance::InitSocketManager()
{
    SocketManager = MakeShared<FSocketManager>();
}

TSharedPtr<FSocketManager> UMyGameInstance::GetSocketManager()
{
    UWorld* World = GetWorld();
    if (World)
    {
        if (World->GetNetMode() == ENetMode::NM_DedicatedServer)
        {
            return SocketManager;
        }
    }

    return nullptr;
}

void UMyGameInstance::StartRecvThread()
{
    RecvThread = new FRecvThread(SocketManager);
}

void UMyGameInstance::StartSendThread()
{
    SendThread = new FSendThread(SocketManager);
}

void UMyGameInstance::SendPacket(const EPacket& PacketType, const FString& Payload)
{
    SendThread->SetSendPacket(PacketType, Payload);
}

void UMyGameInstance::SendPacket(const EPacket& PacketType)
{
    SendThread->SetSendPacket(PacketType, FString());
}
