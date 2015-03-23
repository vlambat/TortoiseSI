// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2015 - TortoiseGit
// Copyright (C) 2003-2008 - TortoiseSVN

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
#include "CommonAppUtils.h"

class CTGitPathList;

/**
 * \ingroup TortoiseSIProc
 * A utility class with static functions.
 */
class CAppUtils : public CCommonAppUtils
{
public:
	CAppUtils(void);
	~CAppUtils(void);

	/**
	 * Launches the standard text viewer/editor application which is associated
	 * with txt files.
	 * \return TRUE if the program could be started.
	 */
	static BOOL StartTextViewer(CString file);

	/**
	* Launch alternative editor
	*/
	static bool LaunchAlternativeEditor(const CString& filename, bool uac = false);

	/**
	* Sets the title of a dialog
	*/
	static void SetWindowTitle(HWND hWnd, const CString& urlorpath, const CString& dialogname);

	/**
	 * Formats text in a rich edit control (version 2).
	 * text in between * chars is formatted bold
	 * text in between ^ chars is formatted italic
	 * text in between _ chars is underlined
	 */
	static bool FormatTextInRichEditControl(CWnd * pWnd);
	static bool FindStyleChars(const CString& sText, TCHAR stylechar, int& start, int& end);
	
	/**
	* implements URL searching with the same logic as CSciEdit::StyleURLs
	*/
	static std::vector<CHARRANGE> FindURLMatches(const CString& msg);
	static BOOL StyleURLs(const CString& msg, CWnd* pWnd);


	static bool ShellOpen(const CString& file, HWND hwnd = nullptr);
	static bool ShowOpenWithDialog(const CString& file, HWND hwnd = nullptr);

	static CString GetClipboardLink(const CString &skipGitPrefix = _T(""), int paramsCount = 0);

	static bool CreateMultipleDirectory(const CString &dir);

	static void RemoveTrailSlash(CString &path);

	static BOOL SICommit();

	static void MarkWindowAsUnpinnable(HWND hWnd);

	static void ExploreTo(HWND hwnd, CString path);

};
