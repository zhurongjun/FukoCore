#pragma once
#include "Stream.hpp"

namespace Fuko
{
	class MemoryStream : public Stream
	{
		void*		m_Buffer;
		uint32		m_Pos;
		uint32		m_Size;
	public:
		FORCEINLINE MemoryStream(void* InBuffer, uint32 InSize, bool Writable = true, bool Readable = true)
			: m_Buffer(InBuffer)
			, m_Size(InSize)
			, m_Pos(0)
			, m_bWritable(Writable)
			, m_bReadable(Readable)
			, m_bSeekable(true)
		{}

		virtual uint32 Write(void * Buffer, uint32 Size) override
		{
			m_bWriting = true;
			uint32 WriteSize = Size;
			uint32 NewPos = m_Pos + Size;
			if (NewPos >= m_Size)
			{
				NewPos = m_Size;
				WriteSize = m_Size - m_Pos;
				m_bEOF = true;
			}

			m_bWriting = false;
			return WriteSize;
		}

		virtual uint32 Peek(void * Buffer, uint32 Size) override;

		virtual uint32 Size() override;

		virtual uint32 Tell() override;

		virtual bool Seek(uint32 Offset, ESeekMode Mode = ESeekMode::Begin) override;

	};
}