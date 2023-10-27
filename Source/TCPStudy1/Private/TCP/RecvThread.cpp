// Fill out your copyright notice in the Description page of Project Settings.


#include "RecvThread.h"
#include "TCPStudy1.h"
#include "SocketManager.h"

//FRecvThread::FRecvThread(TSharedPtr<FSocketManager> SocketManager)
//{
//	this->SocketManager = SocketManager;
//
//	Thread = FRunnableThread::Create(this, TEXT("RecvThread"));
//}
//
//FRecvThread::~FRecvThread()
//{
//	if (Thread)
//	{
//		Thread->WaitForCompletion();
//		Thread->Kill();
//		delete Thread;
//	}
//}
//
//bool FRecvThread::Init()
//{
//	ABLOG(Warning, TEXT("Start Recv Thread"));
//
//	return true;
//}
//
//uint32 FRecvThread::Run()
//{
//	while (true)
//	{
//		FPacketData RecvPacket;
//		if (SocketManager->Recv(RecvPacket))
//		{
//			ABLOG(Warning, TEXT("Recv Type : %d, Recv Payload : %s"), static_cast<int32>(RecvPacket.PacketType), *RecvPacket.Payload);
//		}
//
//		// Make Logic here
//
//	}
//
//	return 0;
//}
//
//void FRecvThread::Exit()
//{
//	SocketManager.Reset();
//}
