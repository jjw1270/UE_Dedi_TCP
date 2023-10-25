// Fill out your copyright notice in the Description page of Project Settings.


#include "PacketMaker.h"
#include "terse/utils/Endianness.h"

FBufferArchive PacketMaker::MakePacket(const EPacket& PacketType)
{
	// Header      Payload
	//[][][][] [Variable data]

	FString Header = MakeHeader(PacketType, 0);

	// serialize out the header
	FBufferArchive BufferArchive;
	BufferArchive << Header;

	return BufferArchive;
}

FBufferArchive PacketMaker::MakePacket(const EPacket& PacketType, const uint8* Payload, const int32& PayloadSize)
{
	// Header      Payload
	//[][][][] [Variable data]

	FString Header = MakeHeader(PacketType, PayloadSize);

	// serialize out the header
	FBufferArchive BufferArchive;
	BufferArchive << Header;

	// Append Payload
	BufferArchive.Append(Payload, PayloadSize);

	return BufferArchive;
}

FString PacketMaker::MakeHeader(const EPacket& PacketType, const uint16& PayloadSize)
{
	// Header
	//size Type
	//[][] [][]

	uint16 Size = hton(PayloadSize);
	uint16 Type = hton(StaticCast<uint16>(PacketType));

	FString Header;

	Header.Append((const TCHAR*)&Size, sizeof(uint16));
	Header.Append((const TCHAR*)&Type, sizeof(uint16));

	UE_LOG(LogTemp, Warning, TEXT("Header Size : %d"), Header.Len());

	return Header;
}
