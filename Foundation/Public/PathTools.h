//========= Copyright Valve Corporation ============//
// This file is part of Valve's VR SDK

#pragma once
#include "FoundationAPI.h"
#include <string>

namespace tc
{

class FOUNDATION_API FPathTools
{
public:
	/** Returns the path (including filename) to the current executable */
	static std::string GetExecutablePath();

	/** Returns the path of the current working directory */
	static std::string GetWorkingDirectory();

	/** Sets the path of the current working directory. Returns true if this was successful. */
	static bool SetWorkingDirectory(const std::string & sPath);

	/** returns the path (including filename) of the current shared lib or DLL */
	static std::string GetThisModulePath();

	/** Returns the specified path without its filename.
	* If slash is unspecified the native path separator of the current platform
	* will be used. */
	static std::string StripFilename(const std::string & sPath, char slash = 0);

	/** returns just the filename from the provided full or relative path. */
	static std::string StripDirectory(const std::string & sPath, char slash = 0);

	/** returns just the filename with no extension of the provided filename.
	* If there is a path the path is left intact. */
	static std::string StripExtension(const std::string & sPath);

	/** returns just extension of the provided filename (if any). */
	static std::string GetExtension(const std::string & sPath);

	/** Returns true if the path is absolute */
	static bool IsAbsolute(const std::string & sPath);

	/** Makes an absolute path from a relative path and a base path */
	static std::string MakeAbsolute(const std::string & sRelativePath, const std::string & sBasePath, char slash = 0);

	/** Fixes the directory separators for the current platform.
	* If slash is unspecified the native path separator of the current platform
	* will be used. */
	static std::string FixSlashes(const std::string & sPath, char slash = 0);

	/** Returns the path separator for the current platform */
	static char GetSlash();

	/** Jams two paths together with the right kind of slash */
	static std::string Join(const std::string & first, const std::string & second, char slash = 0);
	static std::string Join(const std::string & first, const std::string & second, const std::string & third, char slash = 0);
	static std::string Join(const std::string & first, const std::string & second, const std::string & third, const std::string &fourth, char slash = 0);
	static std::string Join(
		const std::string & first,
		const std::string & second,
		const std::string & third,
		const std::string & fourth,
		const std::string & fifth,
		char slash = 0);


	/** Removes redundant <dir>/.. elements in the path. Returns an empty path if the
	* specified path has a broken number of directories for its number of ..s.
	* If slash is unspecified the native path separator of the current platform
	* will be used. */
	static std::string Compact(const std::string & sRawPath, char slash = 0);

	//** Removed trailing slashes */
	static std::string RemoveTrailingSlash(const std::string & sRawPath, char slash = 0);

	/** returns true if the specified path exists and is a directory */
	static bool IsDirectory(const std::string & sPath);

	/** returns true if the specified path represents an app bundle */
	static bool IsAppBundle(const std::string & sPath);

	/** returns true if the the path exists */
	static bool Exists(const std::string & sPath);

	/** Helper functions to find parent directories or subdirectories of parent directories */
	static std::string FindParentDirectoryRecursively(const std::string &strStartDirectory, const std::string &strDirectoryName);
	static std::string FindParentSubDirectoryRecursively(const std::string &strStartDirectory, const std::string &strDirectoryName);

	/** Path operations to read or write text/binary files */
	static unsigned char * ReadBinaryFile(const std::string &strFilename, int *pSize);
	static uint32_t  ReadBinaryFile(const std::string &strFilename, unsigned char *pBuffer, uint32_t unSize);
	static bool WriteBinaryFile(const std::string &strFilename, unsigned char *pData, unsigned nSize);
	static std::string ReadTextFile(const std::string &strFilename);
	static bool WriteStringToTextFile(const std::string &strFilename, const char *pchData);
	static bool WriteStringToTextFileAtomic(const std::string &strFilename, const char *pchData);

	/** Returns a file:// url for paths, or an http or https url if that's what was provided */
	static std::string FilePathToUrl(const std::string & sRelativePath, const std::string & sBasePath);

	/** Strips off file:// off a URL and returns the path. For other kinds of URLs an empty string is returned */
	static std::string UrlToFilePath(const std::string & sFileUrl);

	/** Returns the root of the directory the system wants us to store user documents in */
	static std::string GetUserDocumentsPath();

    /** Returns the root of the directory the system wants us to store app data in */
    static std::string GetUserAppDataPath();
};

} /* namespace tc */

#ifndef MAX_UNICODE_PATH
#define MAX_UNICODE_PATH 32767
#endif

#ifndef MAX_UNICODE_PATH_IN_UTF8
#define MAX_UNICODE_PATH_IN_UTF8 (MAX_UNICODE_PATH * 4)
#endif
