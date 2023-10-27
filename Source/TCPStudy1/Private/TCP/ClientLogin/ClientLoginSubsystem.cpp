// Fill out your copyright notice in the Description page of Project Settings.


#include "TCP/ClientLogin/ClientLoginSubsystem.h"
#include "TCPStudy1.h"
#include "SocketSubsystem.h"

UClientLoginSubsystem::UClientLoginSubsystem()
{

}

bool UClientLoginSubsystem::Connect(const int32 PortNum)
{
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("TCPClientLoginSocket"), false);

	FString IP = TEXT("127.0.0.1");			//Temp IP
	FIPv4Address IPv4Address;
	FIPv4Address::Parse(IP, IPv4Address);

	TSharedPtr<FInternetAddr> SocketAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	SocketAddress->SetPort(Port);
	SocketAddress->SetIp(IPv4Address.Value);

	if (Socket->Connect(*SocketAddress))
	{
		ABLOG(Warning, TEXT("Connect TCP Success!"));

		return true;
	}
	else
	{
		ABLOG(Error, TEXT("Connect TCP Fail."));
		PrintSocketError(TEXT("Connect"));

		DestroySocket();

		return false;
	}
}
