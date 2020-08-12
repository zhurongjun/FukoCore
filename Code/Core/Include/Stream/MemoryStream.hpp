#pragma once
#include <CoreMinimal/SmartPtr.h>
#include "Stream.hpp"

namespace Fuko
{
	class MemoryStream : public Stream final
	{
		SP<void*>	m_Buffer;
		uint32		m_Pos;
		uint32		m_Size;
		FORCEINLINE void* _NowPos() { return (char*)m_Buffer.Get() + m_Pos; }
	public:
		FORCEINLINE MemoryStream(SP<void*> InBuffer, uint32 InSize, bool Writable = true, bool Readable = true);

		// non copyable
		MemoryStream(const MemoryStream&) = delete;
		MemoryStream(MemoryStream&&) = delete;
		MemoryStream& operator=(const MemoryStream&) = delete;
		MemoryStream& operator=(MemoryStream&&) = delete;

		virtual uint32 Read(void* Buffer, uint32 Size) override;
		virtual uint32 Write(void * Buffer, uint32 Size) override;

		virtual uint32 Size() override { return m_Size; }
		virtual uint32 Tell() override { return m_Pos; }

		virtual bool Seek(int32 Offset, ESeekMode Mode = ESeekMode::Begin) override;
	};
}

// Impl MemoryStream 
namespace Fuko
{
	FORCEINLINE MemoryStream::MemoryStream(SP<void *> InBuffer, uint32 InSize, bool Writable, bool Readable)
		: m_Buffer(InBuffer)
		, m_Size(InSize)
		, m_Pos(0)
		, m_bWritable(Writable)
		, m_bReadable(Readable)
		, m_bSeekable(true)
	{}

	FORCEINLINE uint32 MemoryStream::Read(void* Buffer, uint32 Size)
	{
		if (!m_bReadable || m_bEOF) return 0;

		m_bReading = true;
		// calculate index 
		uint32 ReadSize = Size;
		uint32 NewPos = m_Pos + Size;
		if (NewPos >= m_Size)
		{
			NewPos = m_Size;
			ReadSize = m_Size - m_Pos;
			m_bEOF = true;
		}

		// copy and seek 
		Memcpy(Buffer,_NowPos() , ReadSize);
		m_Pos = NewPos;
		m_bReading = false;

		return ReadSize;
	}

	FORCEINLINE uint32 MemoryStream::Write(void * Buffer, uint32 Size)
	{
		if (!m_bWritable || m_bEOF) return 0;

		m_bWriting = true;
		// calculate index 
		uint32 WriteSize = Size;
		uint32 NewPos = m_Pos + Size;
		if (NewPos >= m_Size)
		{
			NewPos = m_Size;
			WriteSize = m_Size - m_Pos;
			m_bEOF = true;
		}

		// copy and seek 
		Memcpy(_NowPos(), Buffer, WriteSize);
		m_Pos = NewPos;
		m_bWriting = false;

		return WriteSize;
	}

	FORCEINLINE bool MemoryStream::Seek(int32 Offset, ESeekMode Mode)
	{
		uint32 PosAfterSeek;
		
		switch (Mode)
		{
		case ESeekMode::Begin:
			PosAfterSeek = Offset;
			break;
		case ESeekMode::Now:
			PosAfterSeek = m_Pos + Offset;
			break;
		case ESeekMode::End:
			PosAfterSeek = m_Size - Offset;
			break;
		default: 
			check(false);
			break;
		}

		if (PosAfterSeek < 0 || PosAfterSeek > m_Size) return false;

		m_Pos = PosAfterSeek;
		m_bEOF = (PosAfterSeek == m_Size);
		
		return true;
	}
}