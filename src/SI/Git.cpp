// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2015 - TortoiseGit

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#include "stdafx.h"
#include "Git.h"
#include "UnicodeUtils.h"
#include "TGitPath.h"

CGit g_Git;

CGit::CGit(void)
{
	GetCurrentDirectory(MAX_PATH, m_CurrentDir.GetBuffer(MAX_PATH));
	m_CurrentDir.ReleaseBuffer();
}

CGit::~CGit(void)
{
}

/*****************************************************************************
 * Static Methods
 ****************************************************************************/
BOOL CGit::GitPathFileExists(const CString &path)
{
	if (path[0] == _T('\\') && path[1] == _T('\\'))
		//it is netshare \\server\sharefoldername
		// \\server\.git will create smb error log.
	{
		int length = path.GetLength();

		if (length<2)
			return false;

		int start = path.Find(_T('\\'), 2);
		if (start<0)
			return false;

		start = path.Find(_T('\\'), start + 1);
		if (start<0)
			return false;

		return PathFileExists(path);

	}
	else
		return PathFileExists(path);
}

BOOL CGit::GetShortName(const CString &ref, CString &shortname, CString prefix)
{
	//TRACE(_T("%s %s\r\n"),ref,prefix);
	if (ref.Left(prefix.GetLength()) == prefix)
	{
		shortname = ref.Right(ref.GetLength() - prefix.GetLength());
		if (shortname.Right(3) == _T("^{}"))
			shortname = shortname.Left(shortname.GetLength() - 3);
		return TRUE;
	}
	return FALSE;
}

CStringA CGit::GetGitPathStringA(const CString &path)
{
	return CUnicodeUtils::GetUTF8(CTGitPath(path).GetGitPathString());
}

//Must use sperate function to convert ANSI str to union code string
//Becuase A2W use stack as internal convert buffer.
void CGit::StringAppend(CString *str, const BYTE *p, int code, int length)
{
	if (str == NULL)
		return;

	int len;
	if (length<0)
		len = (int)strlen((const char*)p);
	else
		len = length;
	if (len == 0)
		return;
	int currentContentLen = str->GetLength();
	WCHAR * buf = str->GetBuffer(len * 4 + 1 + currentContentLen) + currentContentLen;
	int appendedLen = MultiByteToWideChar(code, 0, (LPCSTR)p, len, buf, len * 4);
	str->ReleaseBuffer(currentContentLen + appendedLen); // no - 1 because MultiByteToWideChar is called with a fixed length (thus no nul char included)
}

/*****************************************************************************
* Public Methods
****************************************************************************/
CString CGit::CombinePath(const CString &path) const
{
	if (path.IsEmpty())
		return m_CurrentDir;
	if (m_CurrentDir.IsEmpty())
		return path;
	return m_CurrentDir + (m_CurrentDir.Right(1) == _T("\\") ? _T("") : _T("\\")) + path;
}

CString CGit::CombinePath(const CTGitPath &path) const
{
	return CombinePath(path.GetWinPath());
}

CString CGit::CombinePath(const CTGitPath *path) const
{
	ATLASSERT(path);
	return CombinePath(path->GetWinPath());
}

int CGit::GetFileModifyTime(LPCTSTR filename, __int64* time, bool* isDir, __int64* size )
{
	WIN32_FILE_ATTRIBUTE_DATA fdata;
	if (GetFileAttributesEx(filename, GetFileExInfoStandard, &fdata))
	{
		if (time)
			*time = filetime_to_time_t(&fdata.ftLastWriteTime);

		if (size)
			*size = ((__int64)fdata.nFileSizeHigh << 32) + fdata.nFileSizeLow;

		if (isDir)
			*isDir = !!(fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

		return 0;
	}
	return -1;
}

__int64 CGit::filetime_to_time_t(const FILETIME *ft)
{
	long long winTime = ((long long)ft->dwHighDateTime << 32) + ft->dwLowDateTime;
	winTime -= 116444736000000000LL; /* Windows to Unix Epoch conversion */
	winTime /= 10000000;		 /* Nano to seconds resolution */
	return (time_t)winTime;
}

/*****************************************************************************
* Global Methods
****************************************************************************/
static LPCTSTR nextpath(const wchar_t* path, wchar_t* buf, size_t buflen)
{
	if (path == NULL || buf == NULL || buflen == 0)
		return NULL;

	const wchar_t* base = path;
	wchar_t term = (*path == L'"') ? *path++ : L';';

	for (buflen--; *path && *path != term && buflen; buflen--)
		*buf++ = *path++;

	*buf = L'\0'; /* reserved a byte via initial subtract */

	while (*path == term || *path == L';')
		++path;

	return (path != base) ? path : NULL;
}

static inline BOOL FileExists(LPCTSTR lpszFileName)
{
	struct _stat st;
	return _tstat(lpszFileName, &st) == 0;
}

static bool IsPowerShell(CString cmd)
{
	cmd.MakeLower();
	int powerShellPos = cmd.Find(_T("powershell"));
	if (powerShellPos < 0)
		return false;

	// found the word powershell, check that it is the command and not any parameter
	int end = cmd.GetLength();
	if (cmd.Find(_T('"')) == 0)
	{
		int secondDoubleQuote = cmd.Find(_T('"'), 1);
		if (secondDoubleQuote > 0)
			end = secondDoubleQuote;
	}
	else
	{
		int firstSpace = cmd.Find(_T(' '));
		if (firstSpace > 0)
			end = firstSpace;
	}

	return (end - 4 - 10 == powerShellPos || end - 10 == powerShellPos); // len(".exe")==4, len("powershell")==10
}



#define BUFSIZE 512
void GetTempPath(CString &path)
{
	TCHAR lpPathBuffer[BUFSIZE] = { 0 };
	DWORD dwRetVal;
	DWORD dwBufSize=BUFSIZE;
	dwRetVal = GetTortoiseGitTempPath(dwBufSize,		// length of the buffer
							lpPathBuffer);	// buffer for path
	if (dwRetVal > dwBufSize || (dwRetVal == 0))
	{
		path=_T("");
	}
	path.Format(_T("%s"),lpPathBuffer);
}

CString GetTempFile()
{
	TCHAR lpPathBuffer[BUFSIZE] = { 0 };
	DWORD dwRetVal;
	DWORD dwBufSize=BUFSIZE;
	TCHAR szTempName[BUFSIZE] = { 0 };
	UINT uRetVal;

	dwRetVal = GetTortoiseGitTempPath(dwBufSize,		// length of the buffer
							lpPathBuffer);	// buffer for path
	if (dwRetVal > dwBufSize || (dwRetVal == 0))
	{
		return _T("");
	}
	 // Create a temporary file.
	uRetVal = GetTempFileName(lpPathBuffer,		// directory for tmp files
								TEXT("Patch"),	// temp file name prefix
								0,				// create unique name
								szTempName);	// buffer for name


	if (uRetVal == 0)
	{
		return _T("");
	}

	return CString(szTempName);

}

DWORD GetTortoiseGitTempPath(DWORD nBufferLength, LPTSTR lpBuffer)
{
	DWORD result = ::GetTempPath(nBufferLength, lpBuffer);
	if (result == 0) return 0;
	if (lpBuffer == NULL || (result + 13 > nBufferLength))
	{
		if (lpBuffer)
			lpBuffer[0] = '\0';
		return result + 13;
	}

	_tcscat_s(lpBuffer, nBufferLength, _T("TortoiseSI\\"));
	CreateDirectory(lpBuffer, NULL);

	return result + 13;
}

