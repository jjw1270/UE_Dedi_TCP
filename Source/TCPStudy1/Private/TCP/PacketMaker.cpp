// Fill out your copyright notice in the Description page of Project Settings.


#include "PacketMaker.h"
#include "terse/utils/Endianness.h"

TArray<uint8_t> PacketMaker::MakePacket(const EClientLoginPacket& PacketType)
{
	// Header      Payload
	//[][][][] [Variable data]
	TArray<uint8_t> PacketBuffer = MakeHeader(PacketType, 0);

	return PacketBuffer;
}

TArray<uint8_t> PacketMaker::MakePacket(const EClientLoginPacket& PacketType, const FString& Payload)
{
	// Header      Payload
	//[][][][] [Variable data]

	//FString to Utf8
	TArray<uint8_t> PayloadBuffer;
	//FTCHARToUTF8 MyConverter(*Payload);
	//PayloadBuffer.Append(reinterpret_cast<const uint8_t*>(MyConverter.Get()), MyConverter.Length());
	StringToBytes(Payload, (uint8_t*)&PayloadBuffer, Payload.Len());

	uint16_t PayloadSize = PayloadBuffer.Num();
	TArray<uint8_t> PacketBuffer = MakeHeader(PacketType, PayloadSize);

	PacketBuffer.Append(PayloadBuffer);

	return PacketBuffer;
}

TArray<uint8_t> PacketMaker::MakeHeader(const EClientLoginPacket& PacketType, const uint16_t& PayloadSize)
{
	// Header
	//size Type
	//[][] [][]

	uint16_t Size = hton(PayloadSize);
	uint16_t Type = hton(StaticCast<uint16_t>(PacketType));

	TArray<uint8_t> Header;
	Header.AddZeroed(4);

	memcpy(&Header, &PayloadSize, 2);
	memcpy(&Header, (uint16_t*)&PacketType, 2);


	//memcpy(&Header, &Size, 2);
	//memcpy(&Header + 2, &Type, 2);

	return Header;
}
