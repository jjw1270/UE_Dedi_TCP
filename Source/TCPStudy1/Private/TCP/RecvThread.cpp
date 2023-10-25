// Fill out your copyright notice in the Description page of Project Settings.


#include "RecvThread.h"
#include "TCPStudy1.h"
#include "SocketManager.h"

RecvThread::RecvThread(SocketManager* NewSocketManager)
{
	pSocketManager = NewSocketManager;

	Thread = FRunnableThread::Create(this, TEXT("RecvThread"));
}

RecvThread::~RecvThread()
{
	if (Thread)
	{
		Thread->WaitForCompletion();
		Thread->Kill();
		delete Thread;
	}
}

bool RecvThread::Init()
{
	return true;
}

uint32 RecvThread::Run()
{
	ABLOG(Warning, TEXT("Start Recv Thread"));

	//pSocketManager->Recv();
	while (true)
	{
		pSocketManager->Recv();
	}

	return 0;
}

void RecvThread::Exit()
{
	pSocketManager = nullptr;
	delete pSocketManager;
}
