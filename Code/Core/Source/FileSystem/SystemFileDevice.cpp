// #include <FileSystem/SystemFileDevice.h>
// #include <stdlib.h>

namespace Fuko
{
// 	SP<IStream> SystemFileDevice::Open(const Path& InPath, uint8 OpenFlag)
// 	{
// 		String PathStr = InPath.Encode(0, true);
// 		TCHAR Flag[5] = {};
// 		uint32 FlagIndex = 0;
// 		
// 		// combine flags 
// 		Flag[FlagIndex++] = TSTR('b');
// 		if (OpenFlag & EFileOpenFlag::Read) Flag[FlagIndex++] = TSTR('r');
// 		if (OpenFlag & EFileOpenFlag::Write) Flag[FlagIndex++] = TSTR('w');
// 		if (OpenFlag & EFileOpenFlag::Append) Flag[FlagIndex++] = TSTR('a');
// 		if ((OpenFlag & EFileOpenFlag::Create) == 0) Flag[FlagIndex++] = TSTR('+');
// 
// 		// open file 
// 		FILE* File = _wfopen(PathStr.GetData(), Flag);
// 		if (!File) return nullptr;
// 		return MakeSP<BufferedFileStream>(File, OpenFlag & EFileOpenFlag::Write, OpenFlag & EFileOpenFlag::Read);
// 	}
// 
// 	bool SystemFileDevice::FileExist(const Path& InPath)
// 	{
// 		String PathStr = InPath.Encode(0, true);
// 
// 		WIN32_FIND_DATA FindData;
// 		HANDLE Handle = ::FindFirstFileW(PathStr.GetData(), &FindData);
// 		if (Handle != INVALID_HANDLE_VALUE)
// 		{
// 			::FindClose(Handle);
// 			return true;
// 		}
// 		return false;
// 	}
// 
// 	bool SystemFileDevice::DirExist(const Path& InPath)
// 	{
// 		String PathStr = InPath.Encode(0, true);
// 		
// 		WIN32_FIND_DATA FindData;
// 		HANDLE Handle = ::FindFirstFileW(PathStr.GetData(), &FindData);
// 		if (Handle != INVALID_HANDLE_VALUE)
// 		{
// 			if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
// 			{
// 				::FindClose(Handle);
// 				return true;
// 			}
// 			::FindClose(Handle);
// 			return false;
// 		}
// 		return false;
// 	}
// 
// 	bool SystemFileDevice::GetInfo(const Path& InPath, FileInfo& OutInfo)
// 	{
// 		String PathStr = InPath.Encode(0, true);
// 
// 		WIN32_FIND_DATA FindData;
// 		HANDLE Handle = ::FindFirstFileW(PathStr.GetData(), &FindData);
// 		if (Handle != INVALID_HANDLE_VALUE)
// 		{
// 			OutInfo.Writable = !(FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY);
// 			OutInfo.Hidden = FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
// 			OutInfo.Directory = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
// 			OutInfo.FileSize = FindData.nFileSizeLow;
// 			OutInfo.FileSize += FindData.nFileSizeHigh << sizeof(DWORD);
// 
// 			OutInfo.CreateTime = FindData.ftCreationTime.dwLowDateTime;
// 			OutInfo.CreateTime += FindData.ftCreationTime.dwHighDateTime << sizeof(DWORD);
// 			
// 			OutInfo.LastAccessTime = FindData.ftLastAccessTime.dwLowDateTime;
// 			OutInfo.LastAccessTime += FindData.ftLastAccessTime.dwHighDateTime << sizeof(DWORD);
// 			
// 			OutInfo.LastWriteTime = FindData.ftLastWriteTime.dwLowDateTime;
// 			OutInfo.LastWriteTime += FindData.ftLastWriteTime.dwHighDateTime << sizeof(DWORD);
// 
// 			::FindClose(Handle);
// 			return true;
// 		}
// 		return false;
// 	}



}

