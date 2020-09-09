//========= Copyright Valve Corporation ============//

#include "PathTools.h"
#include "StringUtils.h"
#include "Platform.h"

#if defined( _WIN32)
#include <Windows.h>
#include <direct.h>
#include <Shobjidl.h>
#include <KnownFolders.h>
#include <Shlobj.h>
#include <share.h>

#undef GetEnvironmentVariable
#else
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#endif

#ifdef __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
    #elif TARGET_IPHONE_SIMULATOR
    #elif TARGET_OS_MAC
        #define OSX
        #define POSIX
    #else
    #endif
#elif TC_OS == TC_OS_LINUX
    #define POSIX
    #define LINUX

    #include <strings.h>
    #include <string.h>
    #define _stricmp strcasecmp
    #define _strnicmp strncasecmp
#endif

#if defined OSX
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <mach-o/dyld.h>
#define _S_IFDIR S_IFDIR     // really from tier0/platform.h which we dont have yet
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
#endif

#include <sys/stat.h>

#include <algorithm>

namespace tc
{

/** Returns the path (including filename) to the current executable */
std::string FPathTools::GetExecutablePath()
{
#if defined( _WIN32 )
	wchar_t *pwchPath = new wchar_t[MAX_UNICODE_PATH];
	char *pchPath = new char[MAX_UNICODE_PATH_IN_UTF8];
	::GetModuleFileNameW(NULL, pwchPath, MAX_UNICODE_PATH);
	WideCharToMultiByte(CP_UTF8, 0, pwchPath, -1, pchPath, MAX_UNICODE_PATH_IN_UTF8, NULL, NULL);
	delete[] pwchPath;

	std::string sPath = pchPath;
	delete[] pchPath;
	return sPath;
#elif defined( OSX )
	char rchPath[1024];
	uint32_t nBuff = sizeof(rchPath);
	bool bSuccess = _NSGetExecutablePath(rchPath, &nBuff) == 0;
	rchPath[nBuff - 1] = '\0';
	if (bSuccess)
		return rchPath;
	else
		return "";
#elif defined LINUX
	char rchPath[1024];
	size_t nBuff = sizeof(rchPath);
	ssize_t nRead = readlink("/proc/self/exe", rchPath, nBuff - 1);
	if (nRead != -1)
	{
		rchPath[nRead] = 0;
		return rchPath;
	}
	else
	{
		return "";
	}
#else
	AssertMsg(false, "Implement Plat_GetExecutablePath");
	return "";
#endif

}

/** Returns the path of the current working directory */
std::string FPathTools::GetWorkingDirectory()
{
	std::string sPath;
#if defined( _WIN32 )
	wchar_t buf[MAX_UNICODE_PATH];
	sPath = FStringUtils::UTF16to8(_wgetcwd(buf, MAX_UNICODE_PATH));
#else
	char buf[1024];
	sPath = getcwd(buf, sizeof(buf));
#endif
	return sPath;
}

/** Sets the path of the current working directory. Returns true if this was successful. */
bool FPathTools::SetWorkingDirectory(const std::string & sPath)
{
	bool bSuccess;
#if defined( _WIN32 )
	std::wstring wsPath = FStringUtils::UTF8to16(sPath.c_str());
	bSuccess = 0 == _wchdir(wsPath.c_str());
#else
	bSuccess = 0 == chdir(sPath.c_str());
#endif
	return bSuccess;
}

/** Returns the specified path without its filename */
std::string FPathTools::StripFilename(const std::string & sPath, char slash)
{
	if (slash == 0)
		slash = FPathTools::GetSlash();

	std::string::size_type n = sPath.find_last_of(slash);
	if (n == std::string::npos)
		return sPath;
	else
		return std::string(sPath.begin(), sPath.begin() + n);
}

/** returns just the filename from the provided full or relative path. */
std::string FPathTools::StripDirectory(const std::string & sPath, char slash)
{
	if (slash == 0)
		slash = FPathTools::GetSlash();

	std::string::size_type n = sPath.find_last_of(slash);
	if (n == std::string::npos)
		return sPath;
	else
		return std::string(sPath.begin() + n + 1, sPath.end());
}

/** returns just the filename with no extension of the provided filename.
* If there is a path the path is left intact. */
std::string FPathTools::StripExtension(const std::string & sPath)
{
	for (std::string::const_reverse_iterator i = sPath.rbegin(); i != sPath.rend(); i++)
	{
		if (*i == '.')
		{
			return std::string(sPath.begin(), i.base() - 1);
		}

		// if we find a slash there is no extension
		if (*i == '\\' || *i == '/')
			break;
	}

	// we didn't find an extension
	return sPath;
}

/** returns just extension of the provided filename (if any). */
std::string FPathTools::GetExtension(const std::string & sPath)
{
	for (std::string::const_reverse_iterator i = sPath.rbegin(); i != sPath.rend(); i++)
	{
		if (*i == '.')
		{
			return std::string(i.base(), sPath.end());
		}

		// if we find a slash there is no extension
		if (*i == '\\' || *i == '/')
			break;
	}

	// we didn't find an extension
	return "";
}

bool FPathTools::IsAbsolute(const std::string & sPath)
{
	if (sPath.empty())
		return false;

#if defined( WIN32 )
	if (sPath.size() < 3) // must be c:\x or \\x at least
		return false;

	if (sPath[1] == ':') // drive letter plus slash, but must test both slash cases
	{
		if (sPath[2] == '\\' || sPath[2] == '/')
			return true;
	}
	else if (sPath[0] == '\\' && sPath[1] == '\\') // UNC path
		return true;
#else
	if (sPath[0] == '\\' || sPath[0] == '/') // any leading slash
		return true;
#endif

	return false;
}


/** Makes an absolute path from a relative path and a base path */
std::string FPathTools::MakeAbsolute(const std::string & sRelativePath, const std::string & sBasePath, char slash)
{
	if (slash == 0)
		slash = FPathTools::GetSlash();

	if (FPathTools::IsAbsolute(sRelativePath))
		return sRelativePath;
	else
	{
		if (!FPathTools::IsAbsolute(sBasePath))
			return "";

		std::string sCompacted = FPathTools::Compact(FPathTools::Join(sBasePath, sRelativePath, slash), slash);
		if (FPathTools::IsAbsolute(sCompacted))
			return sCompacted;
		else
			return "";
	}
}


/** Fixes the directory separators for the current platform */
std::string FPathTools::FixSlashes(const std::string & sPath, char slash)
{
	if (slash == 0)
		slash = FPathTools::GetSlash();

	std::string sFixed = sPath;
	for (std::string::iterator i = sFixed.begin(); i != sFixed.end(); i++)
	{
		if (*i == '/' || *i == '\\')
			*i = slash;
	}

	return sFixed;
}


char FPathTools::GetSlash()
{
#if defined(_WIN32)
	return '\\';
#else
	return '/';
#endif
}

/** Jams two paths together with the right kind of slash */
std::string FPathTools::Join(const std::string & first, const std::string & second, char slash)
{
	if (slash == 0)
		slash = FPathTools::GetSlash();

	// only insert a slash if we don't already have one
	std::string::size_type nLen = first.length();
	if (!nLen)
		return second;
#if defined(_WIN32)
	if (first.back() == '\\' || first.back() == '/')
		nLen--;
#else
	char last_char = first[first.length() - 1];
	if (last_char == '\\' || last_char == '/')
		nLen--;
#endif

	return first.substr(0, nLen) + std::string(1, slash) + second;
}


std::string FPathTools::Join(const std::string & first, const std::string & second, const std::string & third, char slash)
{
	return FPathTools::Join(FPathTools::Join(first, second, slash), third, slash);
}

std::string FPathTools::Join(const std::string & first, const std::string & second, const std::string & third, const std::string &fourth, char slash)
{
	return FPathTools::Join(FPathTools::Join(FPathTools::Join(first, second, slash), third, slash), fourth, slash);
}

std::string FPathTools::Join(
	const std::string & first,
	const std::string & second,
	const std::string & third,
	const std::string & fourth,
	const std::string & fifth,
	char slash)
{
	return FPathTools::Join(FPathTools::Join(FPathTools::Join(FPathTools::Join(first, second, slash), third, slash), fourth, slash), fifth, slash);
}


std::string FPathTools::RemoveTrailingSlash(const std::string & sRawPath, char slash)
{
	if (slash == 0)
		slash = FPathTools::GetSlash();

	std::string sPath = sRawPath;
	std::string::size_type nCurrent = sRawPath.length();
	if (nCurrent == 0)
		return sPath;

	int nLastFound = -1;
	nCurrent--;
	while (nCurrent != 0)
	{
		if (sRawPath[nCurrent] == slash)
		{
			nLastFound = (int)nCurrent;
			nCurrent--;
		}
		else
		{
			break;
		}
	}

	if (nLastFound >= 0)
	{
		sPath.erase(nLastFound, std::string::npos);
	}

	return sPath;
}


/** Removes redundant <dir>/.. elements in the path. Returns an empty path if the
* specified path has a broken number of directories for its number of ..s */
std::string FPathTools::Compact(const std::string & sRawPath, char slash)
{
	if (slash == 0)
		slash = FPathTools::GetSlash();

	std::string sPath = FPathTools::FixSlashes(sRawPath, slash);
	std::string sSlashString(1, slash);

	// strip out all /./
	for (std::string::size_type i = 0; (i + 3) < sPath.length(); )
	{
		if (sPath[i] == slash && sPath[i + 1] == '.' && sPath[i + 2] == slash)
		{
			sPath.replace(i, 3, sSlashString);
		}
		else
		{
			++i;
		}
	}


	// get rid of trailing /. but leave the path separator
	if (sPath.length() > 2)
	{
		std::string::size_type len = sPath.length();
		if (sPath[len - 1] == '.'  && sPath[len - 2] == slash)
		{
			// sPath.pop_back();
			sPath[len - 1] = 0;  // for now, at least
		}
	}

	// get rid of leading ./ 
	if (sPath.length() > 2)
	{
		if (sPath[0] == '.'  && sPath[1] == slash)
		{
			sPath.replace(0, 2, "");
		}
	}

	// each time we encounter .. back up until we've found the previous directory name
	// then get rid of both
	std::string::size_type i = 0;
	while (i < sPath.length())
	{
		if (i > 0 && sPath.length() - i >= 2
			&& sPath[i] == '.'
			&& sPath[i + 1] == '.'
			&& (i + 2 == sPath.length() || sPath[i + 2] == slash)
			&& sPath[i - 1] == slash)
		{
			// check if we've hit the start of the string and have a bogus path
			if (i == 1)
				return "";

			// find the separator before i-1
			std::string::size_type iDirStart = i - 2;
			while (iDirStart > 0 && sPath[iDirStart - 1] != slash)
				--iDirStart;

			// remove everything from iDirStart to i+2
			sPath.replace(iDirStart, (i - iDirStart) + 3, "");

			// start over
			i = 0;
		}
		else
		{
			++i;
		}
	}

	return sPath;
}


/** Returns the path to the current DLL or exe */
std::string FPathTools::GetThisModulePath()
{
	// gets the path of vrclient.dll itself
#ifdef WIN32
	HMODULE hmodule = NULL;

	::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCTSTR>(FPathTools::GetThisModulePath), &hmodule);

	wchar_t *pwchPath = new wchar_t[MAX_UNICODE_PATH];
	char *pchPath = new char[MAX_UNICODE_PATH_IN_UTF8];
	::GetModuleFileNameW(hmodule, pwchPath, MAX_UNICODE_PATH);
	WideCharToMultiByte(CP_UTF8, 0, pwchPath, -1, pchPath, MAX_UNICODE_PATH_IN_UTF8, NULL, NULL);
	delete[] pwchPath;

	std::string sPath = pchPath;
	delete[] pchPath;
	return sPath;

#elif defined( OSX ) || defined( LINUX )
	// get the addr of a function in vrclient.so and then ask the dlopen system about it
	Dl_info info;
	dladdr((void *)FPathTools::GetThisModulePath, &info);
	return info.dli_fname;
#endif

}


/** returns true if the specified path exists and is a directory */
bool FPathTools::IsDirectory(const std::string & sPath)
{
	std::string sFixedPath = FPathTools::FixSlashes(sPath);
	if (sFixedPath.empty())
		return false;
	char cLast = sFixedPath[sFixedPath.length() - 1];
	if (cLast == '/' || cLast == '\\')
		sFixedPath.erase(sFixedPath.end() - 1, sFixedPath.end());

	// see if the specified path actually exists.

#if defined(POSIX)
	struct	stat	buf;
	if (stat(sFixedPath.c_str(), &buf) == -1)
	{
		return false;
	}

#if defined( LINUX ) || defined( OSX )
	return S_ISDIR(buf.st_mode);
#else
	return (buf.st_mode & _S_IFDIR) != 0;
#endif

#else
	struct	_stat	buf;
	std::wstring wsFixedPath = FStringUtils::UTF8to16(sFixedPath.c_str());
	if (_wstat(wsFixedPath.c_str(), &buf) == -1)
	{
		return false;
	}

	return (buf.st_mode & _S_IFDIR) != 0;
#endif
}

/** returns true if the specified path represents an app bundle */
bool FPathTools::IsAppBundle(const std::string & sPath)
{
#if defined(OSX)
	NSBundle *bundle = [NSBundle bundleWithPath : [NSString stringWithUTF8String : sPath.c_str()]];
	bool bisAppBundle = (nullptr != bundle);
	[bundle release];
	return bisAppBundle;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the the path exists
//-----------------------------------------------------------------------------
bool FPathTools::Exists(const std::string & sPath)
{
	std::string sFixedPath = FPathTools::FixSlashes(sPath);
	if (sFixedPath.empty())
		return false;

#if defined( WIN32 )
	struct	_stat	buf;
	std::wstring wsFixedPath = FStringUtils::UTF8to16(sFixedPath.c_str());
	if (_wstat(wsFixedPath.c_str(), &buf) == -1)
	{
		return false;
	}
#else
	struct stat buf;
	if (stat(sFixedPath.c_str(), &buf) == -1)
	{
		return false;
	}
#endif

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: helper to find a directory upstream from a given path
//-----------------------------------------------------------------------------
std::string FPathTools::FindParentDirectoryRecursively(const std::string &strStartDirectory, const std::string &strDirectoryName)
{
	std::string strFoundPath = "";
	std::string strCurrentPath = FPathTools::FixSlashes(strStartDirectory);
	if (strCurrentPath.length() == 0)
		return "";

	bool bExists = FPathTools::Exists(strCurrentPath);
	std::string strCurrentDirectoryName = FPathTools::StripDirectory(strCurrentPath);
	if (bExists && _stricmp(strCurrentDirectoryName.c_str(), strDirectoryName.c_str()) == 0)
		return strCurrentPath;

	while (bExists && strCurrentPath.length() != 0)
	{
		strCurrentPath = FPathTools::StripFilename(strCurrentPath);
		strCurrentDirectoryName = FPathTools::StripDirectory(strCurrentPath);
		bExists = FPathTools::Exists(strCurrentPath);
		if (bExists && _stricmp(strCurrentDirectoryName.c_str(), strDirectoryName.c_str()) == 0)
			return strCurrentPath;
	}

	return "";
}


//-----------------------------------------------------------------------------
// Purpose: helper to find a subdirectory upstream from a given path
//-----------------------------------------------------------------------------
std::string FPathTools::FindParentSubDirectoryRecursively(const std::string &strStartDirectory, const std::string &strDirectoryName)
{
	std::string strFoundPath = "";
	std::string strCurrentPath = FPathTools::FixSlashes(strStartDirectory);
	if (strCurrentPath.length() == 0)
		return "";

	bool bExists = FPathTools::Exists(strCurrentPath);
	while (bExists && strCurrentPath.length() != 0)
	{
		strCurrentPath = FPathTools::StripFilename(strCurrentPath);
		bExists = FPathTools::Exists(strCurrentPath);

		if (FPathTools::Exists(FPathTools::Join(strCurrentPath, strDirectoryName)))
		{
			strFoundPath = FPathTools::Join(strCurrentPath, strDirectoryName);
			break;
		}
	}
	return strFoundPath;
}


//-----------------------------------------------------------------------------
// Purpose: reading and writing files in the vortex directory
//-----------------------------------------------------------------------------
unsigned char * FPathTools::ReadBinaryFile(const std::string &strFilename, int *pSize)
{
	FILE *f;
#if defined( POSIX )
	f = fopen(strFilename.c_str(), "rb");
#else
	std::wstring wstrFilename = FStringUtils::UTF8to16(strFilename.c_str());
	// the open operation needs to be sharable, therefore use of _wfsopen instead of _wfopen_s
    f = _wfsopen(wstrFilename.c_str(), L"rb", _SH_DENYNO);
#endif

	unsigned char* buf = NULL;

	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);

		buf = new unsigned char[size];
		if (buf && fread(buf, size, 1, f) == 1)
		{
			if (pSize)
				*pSize = size;
		}
		else
		{
			delete[] buf;
			buf = 0;
		}

		fclose(f);
	}

	return buf;
}

uint32_t  FPathTools::ReadBinaryFile(const std::string &strFilename, unsigned char *pBuffer, uint32_t unSize)
{
	FILE *f;
#if defined( POSIX )
	f = fopen(strFilename.c_str(), "rb");
#else
	std::wstring wstrFilename = FStringUtils::UTF8to16(strFilename.c_str());
	errno_t err = _wfopen_s(&f, wstrFilename.c_str(), L"rb");
	if (err != 0)
	{
		f = NULL;
	}
#endif

	uint32_t unSizeToReturn = 0;

	if (f != NULL)
	{
		fseek(f, 0, SEEK_END);
		uint32_t size = (uint32_t)ftell(f);
		fseek(f, 0, SEEK_SET);

		if (size > unSize || !pBuffer)
		{
			unSizeToReturn = (uint32_t)size;
		}
		else
		{
			if (fread(pBuffer, size, 1, f) == 1)
			{
				unSizeToReturn = (uint32_t)size;
			}
		}

		fclose(f);
	}

	return unSizeToReturn;
}

bool FPathTools::WriteBinaryFile(const std::string &strFilename, unsigned char *pData, unsigned nSize)
{
	FILE *f;
#if defined( POSIX )
	f = fopen(strFilename.c_str(), "wb");
#else
	std::wstring wstrFilename = FStringUtils::UTF8to16(strFilename.c_str());
	errno_t err = _wfopen_s(&f, wstrFilename.c_str(), L"wb");
	if (err != 0)
	{
		f = NULL;
	}
#endif

	size_t written = 0;
	if (f != NULL) {
		written = fwrite(pData, sizeof(unsigned char), nSize, f);
		fclose(f);
	}

    return (written = nSize ? true : false);
}

std::string FPathTools::ReadTextFile(const std::string &strFilename)
{
	// doing it this way seems backwards, but I don't
	// see an easy way to do this with C/C++ style IO
	// that isn't worse...
	int size;
	unsigned char* buf = FPathTools::ReadBinaryFile(strFilename, &size);
	if (!buf)
		return "";

	// convert CRLF -> LF
	size_t outsize = 1;
	for (int i = 1; i < size; i++)
	{
		if (buf[i] == '\n' && buf[i - 1] == '\r') // CRLF
			buf[outsize - 1] = '\n'; // ->LF
		else
			buf[outsize++] = buf[i]; // just copy
	}

	std::string ret((char *)buf, outsize);
	delete[] buf;
	return ret;
}


bool FPathTools::WriteStringToTextFile(const std::string &strFilename, const char *pchData)
{
	FILE *f;
#if defined( POSIX )
	f = fopen(strFilename.c_str(), "w");
#else
	std::wstring wstrFilename = FStringUtils::UTF8to16(strFilename.c_str());
	errno_t err = _wfopen_s(&f, wstrFilename.c_str(), L"w");
	if (err != 0)
	{
		f = NULL;
	}
#endif

	bool ok = false;

	if (f != NULL)
	{
		ok = fputs(pchData, f) >= 0;
		fclose(f);
	}

	return ok;
}

bool FPathTools::WriteStringToTextFileAtomic(const std::string &strFilename, const char *pchData)
{
	std::string strTmpFilename = strFilename + ".tmp";

	if (!FPathTools::WriteStringToTextFile(strTmpFilename, pchData))
		return false;

	// Platform specific atomic file replacement
#if defined( _WIN32 )
	std::wstring wsFilename = FStringUtils::UTF8to16(strFilename.c_str());
	std::wstring wsTmpFilename = FStringUtils::UTF8to16(strTmpFilename.c_str());
	if (!::ReplaceFileW(wsFilename.c_str(), wsTmpFilename.c_str(), nullptr, 0, 0, 0))
	{
		// if we couldn't ReplaceFile, try a non-atomic write as a fallback
		if (!FPathTools::WriteStringToTextFile(strFilename, pchData))
			return false;
	}
#elif defined( POSIX )
	if (rename(strTmpFilename.c_str(), strFilename.c_str()) == -1)
		return false;
#else
#error Do not know how to write atomic file
#endif

	return true;
}


#if defined(WIN32)
#define FILE_URL_PREFIX "file:///"
#else
#define FILE_URL_PREFIX "file://"
#endif

// ----------------------------------------------------------------------------------------------------------------------------
// Purpose: Turns a path to a file on disk into a URL (or just returns the value if it's already a URL)
// ----------------------------------------------------------------------------------------------------------------------------
std::string FPathTools::FilePathToUrl(const std::string & sRelativePath, const std::string & sBasePath)
{
	if (!_strnicmp(sRelativePath.c_str(), "http://", 7)
		|| !_strnicmp(sRelativePath.c_str(), "https://", 8)
		|| !_strnicmp(sRelativePath.c_str(), "file://", 7))
	{
		return sRelativePath;
	}
	else
	{
		std::string sAbsolute = FPathTools::MakeAbsolute(sRelativePath, sBasePath);
		if (sAbsolute.empty())
			return sAbsolute;
		return std::string(FILE_URL_PREFIX) + sAbsolute;
	}
}

// -----------------------------------------------------------------------------------------------------
// Purpose: Strips off file:// off a URL and returns the path. For other kinds of URLs an empty string is returned
// -----------------------------------------------------------------------------------------------------
std::string FPathTools::UrlToFilePath(const std::string & sFileUrl)
{
	if (!_strnicmp(sFileUrl.c_str(), FILE_URL_PREFIX, strlen(FILE_URL_PREFIX)))
	{
		return sFileUrl.c_str() + strlen(FILE_URL_PREFIX);
	}
	else
	{
		return "";
	}
}


// -----------------------------------------------------------------------------------------------------
// Purpose: Returns the root of the directory the system wants us to store user documents in
// -----------------------------------------------------------------------------------------------------
std::string FPathTools::GetUserDocumentsPath()
{
#if defined( WIN32 )
	WCHAR rwchPath[MAX_PATH];

	if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS | CSIDL_FLAG_CREATE, NULL, 0, rwchPath)))
	{
		return "";
	}

	// Convert the path to UTF-8 and store in the output
	std::string sUserPath = FStringUtils::UTF16to8(rwchPath);

	return sUserPath;
#elif defined( OSX )
	@autoreleasepool {
		NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		if ([paths count] == 0)
		{
			return "";
		}

		return[[paths objectAtIndex : 0] UTF8String];
	}
#elif defined( LINUX )
	// @todo: not solved/changed as part of OSX - still not real - just removed old class based steam cut and paste
	const char *pchHome = getenv("HOME");
	if (pchHome == NULL)
	{
		return "";
	}
	return pchHome;
#endif
}

std::string FPathTools::GetUserAppDataPath()
{
#if defined( WIN32 )
	WCHAR rwchPath[MAX_PATH];

	if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, rwchPath)))
	{
		return "";
	}

	// Convert the path to UTF-8 and store in the output
	std::string sUserPath = FStringUtils::UTF16to8(rwchPath);

	return sUserPath;
#elif defined( OSX )
    @autoreleasepool {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        if ([paths count] == 0)
        {
            return "";
        }

        return[[paths objectAtIndex : 0] UTF8String];
    }
#elif defined( LINUX )
    const char *pchHome = getenv("HOME");
    if (pchHome == NULL)
    {
        return "";
    }
    std::string home(pchHome);
    return home + "/.local/share/";
#endif
}

}
