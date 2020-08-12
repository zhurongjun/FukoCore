#pragma once
#include "Stream.hpp"
#include <Containers/Allocator.h>

namespace Fuko
{
	template<typename TAlloc = PmrAlloc>
	class DynamicMemoryStream : public Stream, private TAlloc final
	{
		void*		m_Buffer;
		uint32		m_Pos;
		uint32		m_Size;
		FORCEINLINE void* _NowPos() { return (char*)m_Buffer.Get() + m_Pos; }
		FORCEINLINE void _Reserve(uint32 Size)
		{
			if (m_Size != Size)
			{
				m_Size = TAlloc::ReserveRaw(m_Buffer, Size, 16);
			}
		}
	public:
		DynamicMemoryStream(uint32 InitSize = 0, bool Writable = true, bool Readable = true, const TAlloc& InAlloc = TAlloc())
			: TAlloc(InAlloc)
			, m_Buffer(nullptr)
			, m_Pos(0)
			, m_Size(0)
			, m_bWritable(Writable)
			, m_bReadable(Readable)
			, m_bSeekable(true)
			, m_bResizable(true)
		{
			if (InitSize) _Reserve(InitSize);
		}

		// non copyable
		DynamicMemoryStream(const DynamicMemoryStream&) = delete;
		DynamicMemoryStream(DynamicMemoryStream&&) = delete;
		DynamicMemoryStream& operator=(const DynamicMemoryStream&) = delete;
		DynamicMemoryStream& operator=(DynamicMemoryStream&&) = delete;

		// Read write 
		virtual uint32	Read(void* Buffer, uint32 Size) override
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
			Memcpy(Buffer, _NowPos(), ReadSize);
			m_Pos = NewPos;
			m_bReading = false;

			return ReadSize;
		}
		virtual uint32	Write(void* Buffer, uint32 Size) override
		{
			if (!m_bWritable) return 0;
			m_bEOF = false;

			m_bWriting = true;
			// calculate index 
			uint32 WriteSize = Size;
			uint32 NewPos = m_Pos + Size;
			if (NewPos >= m_Size)
			{
				uint32 NewSize = TAlloc::GetGrow(NewPos, m_Size);
				_Reserve(NewSize);
			}

			// copy and seek 
			Memcpy(_NowPos(), Buffer, WriteSize);
			m_Pos = NewPos;
			m_bWriting = false;

			return WriteSize;
		}

		// Size 
		virtual uint32	Size() override { return m_Size; }
		virtual bool	Reserve(uint32 NewSize) override { _Reserve(NewSize); return true; }

		// Position 
		virtual uint32	Tell() override { return m_Pos; }
		virtual bool	Seek(int32 Offset, ESeekMode Mode = ESeekMode::Begin) override
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
	};
}