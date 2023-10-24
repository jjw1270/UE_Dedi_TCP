// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TCP/Packets.h"
#include "Serialization/BufferArchive.h"

/**
 * 
 */

class TCPSTUDY1_API PacketMaker
{
protected:
	static const int32 DefaultBufferSize = 4;

public:
	// Use this PacketMaker if does not have payload to send
	static FBufferArchive MakeBuffer(const EPacket& PacketType);

	// Use this PacketMaker if have payload to send
	static FBufferArchive MakeBuffer(const EPacket& PacketType, const uint8* Payload, const int32& PayloadSize);

protected:
	static FString MakeHeader(const EPacket& PacketType, const uint16& PayloadSize);
};
