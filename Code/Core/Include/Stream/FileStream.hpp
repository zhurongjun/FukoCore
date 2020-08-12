#pragma once
#include "Stream.hpp"
#include <fileapi.h>
#include <stdio.h>

// FileStream 
namespace Fuko
{
	class FileStream : public Stream final
	{
		HANDLE	m_File;
	public:
		FileStream(HANDLE FileHandle, bool Writable, bool Readable)
			: m_File(FileHandle)
			, m_bWritable(Writable)
			, m_bReadable(Readable)
			, m_bSeekable(true)
			, m_bResizable(true)
		{}
		~FileStream() { Close(); }

		// Read write 
		virtual uint32	Read(void* Buffer, uint32 Size) override
		{
			if (!m_bReadable || m_bEOF || m_bCrashed) return 0;
			DWORD ReadByte;
			BOOL Done = ::ReadFile(m_File, Buffer, (DWORD)Size, &ReadByte, nullptr);
			
			m_bEOF = Done && ReadByte < Size;
			m_bFailed = !Done;

			return ReadByte;
		}
		virtual uint32	Write(void* Buffer, uint32 Size) override
		{
			if (!m_bWritable || m_bEOF || m_bCrashed) return 0;
			DWORD WriteByte;
			BOOL Done = ::WriteFile(m_File, Buffer, (DWORD)Size, &WriteByte, nullptr);

			m_bEOF = Done && ReadByte < Size;
			m_bFailed = !Done;

			return ReadByte;
		}

		// Size 
		virtual uint32	Size() override
		{
			if (m_bCrashed) return 0;
			LARGE_INTEGER Size;
			if (::GetFileSizeEx(m_File, &Size))
			{
				return uint32(Size.QuadPart);
			}
			return 0;
		}
		virtual bool	Reserve(uint32 NewSize) override
		{
			if (!m_bReadable || m_bCrashed) return false;
			LARGE_INTEGER LastPos;
			LARGE_INTEGER Pos;
			LARGE_INTEGER EndSize;
			EndSize.QuadPart = NewSize;
			if (m_bFailed = !::GetFileSizeEx(m_File, LastPos)) return false;
			if (m_bFailed = !::SetFilePointerEx(m_File, EndSize, &Pos, FILE_BEGIN)) return false;
			if (m_bFailed = !::SetEndOfFile(m_File)) return false;
			if (m_bFailed = !::SetFilePointerEx(m_File, LastPos, &Pos, FILE_BEGIN)) return false;
			return true;
		}

		// Position 
		virtual uint32	Tell() override
		{
			if (m_bCrashed) return 0;
			LARGE_INTEGER Pos;
			LARGE_INTEGER Movement;
			Movement.QuadPart = 0;
			if (m_bFailed = ::SetFilePointerEx(m_File, Movement, &Pos, FILE_CURRENT) == 0) return 0;
			return (uint32)Pos.QuadPart;
		}
		virtual bool	Seek(int32 Offset, ESeekMode Mode = ESeekMode::Begin) override
		{
			if (m_bCrashed) return false;
			LARGE_INTEGER Pos;
			LARGE_INTEGER Movement;
			Movement.QuadPart = 0;
			DWORD Method;
			switch (Mode)
			{
			case Fuko::ESeekMode::Begin:
				Method = FILE_BEGIN;
				break;
			case Fuko::ESeekMode::Now:
				Method = FILE_CURRENT;
				break;
			case Fuko::ESeekMode::End:
				Method = FILE_END;
				break;
			default:
				break;
			}
			if (m_bFailed = ::SetFilePointerEx(m_File, Movement, &Pos, Method) == 0) return false;
			return true;
		}

		// Device operator 
		virtual bool	Flush() override
		{
			if (m_bCrashed) return false;
			m_bFailed = ::FlushFileBuffers(m_File);
			return m_bFailed;
		}
		virtual bool	Close() override
		{
			if (m_bCrashed) return false;
			BOOL Done = ::CloseHandle(m_File);
			m_File == INVALID_HANDLE_VALUE;
			m_bCrashed = true;
			return Done;
		}
	};
}

// BufferedFileStream 
namespace Fuko
{
	class BufferedFileStream : public Stream final
	{
		FILE*		m_File;
	public:
		BufferedFileStream(FILE* FileHandle, bool Writable, bool Readable)
			: m_File(FileHandle)
			, m_bWritable(Writable)
			, m_bReadable(Readable)
			, m_bSeekable(true)
			, m_bResizable(true)
			, m_bBuffered(true)
		{}
		// Read write 
		virtual uint32	Read(void* Buffer, uint32 Size) override
		{
			if (!m_bReadable || m_bEOF || m_bCrashed) return 0;

			uint32 ReadSize = (uint32)_fread_nolock(Buffer, Size, 1, m_File);

			if (ReadSize != Size)
			{
				if (feof(m_File))
				{
					m_bEOF = true;
					clearerr(m_File);
					return ReadSize;
				}
				m_bFailed = true;
				clearerr(m_File);
				return ReadSize;
			}

			return ReadSize;
		}
		virtual uint32	Write(void* Buffer, uint32 Size) override
		{
			if (!m_bWritable || m_bEOF || m_bCrashed) return 0;

			uint32 WriteSize = (uint32)_fwrite_nolock(Buffer, Size, 1, m_File);

			if (WriteSize != Size)
			{
				if (feof(m_File))
				{
					m_bEOF = true;
					clearerr(m_File);
					return WriteSize;
				}
				m_bFailed = true;
				clearerr(m_File);
				return WriteSize;
			}

			return WriteSize;
		}

		// Size 
		virtual uint32	Size() override
		{
			if (m_bCrashed) return 0;
			HANDLE H = (HANDLE)_get_osfhandle(_fileno(m_File));
			LARGE_INTEGER Size;
			if (::GetFileSizeEx(H, &Size))
			{
				return uint32(Size.QuadPart);
			}
			return 0;
		}
		virtual bool	Reserve(uint32 NewSize) override
		{
			if (!m_bReadable || m_bCrashed) return false;
			HANDLE H = (HANDLE)_get_osfhandle(_fileno(m_File));
			LARGE_INTEGER LastPos;
			LARGE_INTEGER Pos;
			LARGE_INTEGER EndSize;
			EndSize.QuadPart = NewSize;
			if (m_bFailed = !::GetFileSizeEx(H, LastPos)) return false;
			if (m_bFailed = !::SetFilePointerEx(H, EndSize, &Pos, FILE_BEGIN)) return false;
			if (m_bFailed = !::SetEndOfFile(H)) return false;
			if (m_bFailed = !::SetFilePointerEx(H, LastPos, &Pos, FILE_BEGIN)) return false;
			return true;
		}

		// Position 
		virtual uint32	Tell() override
		{
			if (m_bCrashed) return 0;
			auto CurSize = _ftelli64_nolock(m_File);
			if (CurSize < 0)
			{
				m_bFailed = true;
				clearerr(m_File);
				return 0;
			}
			return (uint32)CurSize;
		}
		virtual bool	Seek(int32 Offset, ESeekMode Mode = ESeekMode::Begin) override
		{
			if (m_bCrashed) return false;
			int Origin;
			switch (Mode)
			{
			case ESeekMode::Begin:
				Origin = SEEK_SET;
				break;
			case ESeekMode::Now:
				Origin = SEEK_CUR;
				break;
			case ESeekMode::End:
				Origin = SEEK_END;
				break;
			default:
				break;
			}
			if (_fseeki64_nolock(m_File, Offset, Origin))
			{
				m_bFailed = true;
				clearerr(m_File);
				return false;
			}
			return true;
		}

		// Device operator 
		virtual bool	Flush() override
		{
			if (m_bCrashed) return false;
			if (_fflush_nolock(m_File))
			{
				m_bFailed = true;
				clearerr(m_File);
				return false;
			}
			return true;
		}
		virtual bool	Close() override
		{
			if (m_bCrashed) return false;
			fclose(m_File);
			m_File = nullptr;
			m_bCrashed = true;
		}
	};
}

// SubFileStream 



// BufferedSubFileStream


