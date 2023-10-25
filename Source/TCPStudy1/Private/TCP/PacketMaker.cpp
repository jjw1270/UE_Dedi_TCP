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

FBufferArchive PacketMaker::MakePacket(const EPacket& PacketType, const uint8_t* Payload, const uint16_t& PayloadSize)
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

FString PacketMaker::MakeHeader(const EPacket& PacketType, const uint16_t& PayloadSize)
{
	// Header
	//size Type
	//[][] [][]

	uint16_t Size = hton(PayloadSize);
	uint16_t Type = hton(StaticCast<uint16_t>(PacketType));

	FString Header;

	Header.Append((const TCHAR*)&Size, sizeof(uint16_t));
	Header.Append((const TCHAR*)&Type, sizeof(uint16_t));

	UE_LOG(LogTemp, Warning, TEXT("Header Size : %d %d"), Size, Type);

	return Header;
}
