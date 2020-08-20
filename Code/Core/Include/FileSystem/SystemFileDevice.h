#pragma once
#include "FileDevice.h"

namespace Fuko
{
	class SystemFileDevice : public IFileDevice
	{
		virtual SP<IStream> Open(const Path& InPath, uint8 OpenFlag) override;

		virtual bool FileExist(const Path& InPath) override;
		virtual bool DirExist(const Path& InPath) override;

		virtual bool GetInfo(const Path& InPath, FileInfo& OutInfo) override;

		virtual bool CopyFile(const Path& DstPath, const Path& SrcPath, bool FailIfExist) override;
		virtual bool MoveFile(const Path& DstPath, const Path& SrcPath, bool FailIfExist) override;
		virtual bool DeleteFile(const Path& InPath) override;

		virtual void EachFile(TDelegate<bool(FileInfo)> InFun) override;
		virtual bool CreateDir(const Path& InPath) override;
		virtual bool RemoveDir(const Path& InPath, bool Recursive) override;
	};
}
