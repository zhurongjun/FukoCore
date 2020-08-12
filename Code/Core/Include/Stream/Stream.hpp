#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include <atomic>

namespace Fuko
{
	enum class ESeekMode
	{
		Begin ,
		Now ,
		End ,
	};

	class Stream
	{
	public:
		FORCEINLINE Stream()
			: m_bReadable(false)
			, m_bWritable(false)
			, m_bSeekable(false)
			, m_bResizable(false)
			, m_bBuffered(false)
			, m_bEOF(false)
			, m_bFailed(false)
			, m_bCrashed(false)
			, m_bWriting(0)
			, m_bReading(0)
		{}
		virtual ~Stream() {}

		// Get info 
		FORCEINLINE bool IsReadable() { return m_bReadable; }
		FORCEINLINE bool IsWriteable() { return m_bWritable; }
		FORCEINLINE bool IsSeekable() { return m_bSeekable; }
		FORCEINLINE bool IsResizable() { return m_bResizable; }
		FORCEINLINE bool IsBuffered() { return m_bBuffered; }
		
		// Get state 
		FORCEINLINE bool IsEOF() { return m_bEOF; }
		FORCEINLINE bool IsFailed() { return m_bFailed; }
		FORCEINLINE bool IsCrashed() { return m_bCrashed; }
		FORCEINLINE bool IsReading() { return m_bReading; }
		FORCEINLINE bool IsWriting() { return m_bWriting; }
		FORCEINLINE bool IsBusy() { return m_bReading || m_bWriting; }
		FORCEINLINE bool IsFree() { return !IsBusy(); }

		// Read write 
		virtual uint32	Read(void* Buffer, uint32 Size) = 0;
		virtual uint32	Write(void* Buffer, uint32 Size) = 0;

		// Size 
		virtual uint32	Size() = 0;
		virtual bool	Reserve(uint32 NewSize) { return false; }
		
		// Position 
		virtual uint32	Tell() = 0;
		virtual bool	Seek(int32 Offset, ESeekMode Mode = ESeekMode::Begin) = 0;

		// Device operator 
		virtual bool	Flush() { return true; }
		virtual bool	Close() { return true; }

	protected:
		// Options 
		uint8	m_bReadable : 1;	// 可读 
		uint8	m_bWritable : 1;	// 可写 
		uint8	m_bSeekable : 1;	// 可跳转 
		uint8	m_bResizable : 1;	// 可重置大小 
		uint8	m_bBuffered : 1;	// 拥有缓冲区 

		// States 
		uint8	m_bEOF : 1;			// 是否结束
		uint8	m_bFailed : 1;		// 失败(可恢复) 
		uint8	m_bCrashed : 1;		// 错误(不可恢复) 
		
		// for async 
		std::atomic<uint8>	m_bWriting;		// 正在读取 
		std::atomic<uint8>	m_bReading;		// 正在写入 
	};
}
