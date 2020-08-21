#pragma once
#include <CoreConfig.h>
#include <CoreType.h>
#include "Path.h"
#include <Misc/SmartPtr.h>

namespace Fuko
{
	class IStream;
	enum EFileOpenFlag
	{
		Read	= 1 << 0,	// 读模式 
		Write	= 1 << 1,	// 写模式 
		Append	= 1 << 4,	// 追加模式 
		Create	= 1 << 2,	// 如不存在则创建 
	};

	struct FileInfo
	{
		uint8		Writable : 1;			// 可写 
		uint8		Hidden : 1;				// 隐藏 
		uint8		Directory : 1;			// 目录 
		uint8		CharacterSpecial : 1;	// 字符文件特殊格式(UNIX/Linux) 

		uint64		FileSize;		// 文件大小
		uint64		CreateTime;		// 创建时间
		uint64		LastAccessTime;	// 最后访问时间
		uint64		LastWriteTime;	// 最后写入时间
	};

	struct IFileDevice
	{
		virtual ~IFileDevice() {}
		
		virtual SP<IStream> Open(const Path& InPath, uint8 OpenFlag) = 0;

		virtual bool FileExist(const Path& InPath) = 0;
		virtual bool DirExist(const Path& InPath) = 0;

		virtual bool GetInfo(const Path& InPath, FileInfo& OutInfo) = 0;
		
		virtual bool CopyFile(const Path& DstPath, const Path& SrcPath, bool FailIfExist) = 0;
		virtual bool MoveFile(const Path& DstPath, const Path& SrcPath, bool FailIfExist) = 0;
		virtual bool DeleteFile(const Path& InPath) = 0;

		virtual void EachFile(TDelegate<bool(FileInfo)> InFun) = 0;
		virtual bool CreateDir(const Path& InPath) = 0;
		virtual bool RemoveDir(const Path& InPath, bool Recursive) = 0;
	};
}
