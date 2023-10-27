// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "TCPStudy1.h"
#include "RecvThread.h"
#include "SendThread.h"

void UMyGameInstance::Shutdown()
{
    SocketManager->DestroySocket();
    SocketManager.Reset();
    Super::Shutdown();
}

void UMyGameInstance::InitSocketManager()
{
    SocketManager = MakeShared<FSocketManager>();
}

//FString UMyGameInstance::RecvClientLoginSocket()
//{
//    SocketManager->Recv()
//
//    return FString();
//}

//TSharedPtr<FSocketManager> UMyGameInstance::GetSocketManager()
//{
//    //UWorld* World = GetWorld();
//    //if (World)
//    //{
//    //    if (World->GetNetMode() == ENetMode::NM_DedicatedServer)
//    //    {
//    //        return SocketManager;
//    //    }
//    //}
//
//    //return nullptr;
//
//    return SocketManager;
//}
