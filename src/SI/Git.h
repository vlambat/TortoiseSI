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

#pragma once

#include <functional>

class CTGitPath;

class CGit
{
public:
	CString m_CurrentDir;

	CGit(void);
	~CGit(void);

	static BOOL GitPathFileExists(const CString &path);
	static BOOL GetShortName(const CString &ref, CString &shortname, CString prefix);
	static CStringA GetGitPathStringA(const CString &path);
	static void StringAppend(CString *str, const BYTE *p, int code = CP_UTF8, int length = -1);

	CString GetHomeDirectory() const;
	CString CombinePath(const CString &path) const;
	CString CombinePath(const CTGitPath &path) const;
	CString CombinePath(const CTGitPath *path) const;
	int GetFileModifyTime(LPCTSTR filename, __int64* time, bool* isDir = nullptr, __int64* size = nullptr);
	__int64 filetime_to_time_t(const FILETIME *ft);
};

extern void GetTempPath(CString &path);
extern CString GetTempFile();
extern DWORD GetTortoiseGitTempPath(DWORD nBufferLength, LPTSTR lpBuffer);

extern CGit g_Git;