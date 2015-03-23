// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2015 - TortoiseGit
// Copyright (C) 2003-2011, 2013-2014 - TortoiseSVN

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
#include <propkey.h>
#include "TortoiseSIProc.h"
#include "PathUtils.h"
#include "AppUtils.h"
#include "StringUtils.h"
#include "MessageBox.h"
#include "registry.h"
#include "TGitPath.h"
#include "Git.h"
#include "UnicodeUtils.h"
#include "GitAdminDir.h"
#include "BrowseFolder.h"
#include "DirFileEnum.h"
#include "MessageBox.h"
#include "SICommitDlg.h"
#include "SmartHandle.h"
#include "Globals.h"

CAppUtils::CAppUtils(void)
{
}

CAppUtils::~CAppUtils(void)
{
}

BOOL CAppUtils::StartTextViewer(CString file)
{
	CString viewer;
	CRegString txt = CRegString(_T(".txt\\"), _T(""), FALSE, HKEY_CLASSES_ROOT);
	viewer = txt;
	viewer = viewer + _T("\\Shell\\Open\\Command\\");
	CRegString txtexe = CRegString(viewer, _T(""), FALSE, HKEY_CLASSES_ROOT);
	viewer = txtexe;

	DWORD len = ExpandEnvironmentStrings(viewer, NULL, 0);
	std::unique_ptr<TCHAR[]> buf(new TCHAR[len + 1]);
	ExpandEnvironmentStrings(viewer, buf.get(), len);
	viewer = buf.get();
	len = ExpandEnvironmentStrings(file, NULL, 0);
	std::unique_ptr<TCHAR[]> buf2(new TCHAR[len + 1]);
	ExpandEnvironmentStrings(file, buf2.get(), len);
	file = buf2.get();
	file = _T("\"")+file+_T("\"");
	if (viewer.IsEmpty())
	{
		return CAppUtils::ShowOpenWithDialog(file) ? TRUE : FALSE;
	}
	if (viewer.Find(_T("\"%1\"")) >= 0)
	{
		viewer.Replace(_T("\"%1\""), file);
	}
	else if (viewer.Find(_T("%1")) >= 0)
	{
		viewer.Replace(_T("%1"),  file);
	}
	else
	{
		viewer += _T(" ");
		viewer += file;
	}

	if(!LaunchApplication(viewer, IDS_ERR_TEXTVIEWSTART, false))
	{
		return FALSE;
	}
	return TRUE;
}

bool CAppUtils::LaunchAlternativeEditor(const CString& filename, bool uac)
{
	CString editTool = CRegString(_T("Software\\TortoiseGit\\AlternativeEditor"));
	if (editTool.IsEmpty() || (editTool.Left(1).Compare(_T("#"))==0)) {
		editTool = CPathUtils::GetAppDirectory() + _T("notepad2.exe");
	}

	CString sCmd;
	sCmd.Format(_T("\"%s\" \"%s\""), editTool, filename);

	LaunchApplication(sCmd, NULL, false, NULL, uac);
	return true;
}

bool CAppUtils::FormatTextInRichEditControl(CWnd * pWnd)
{
	CString sText;
	if (pWnd == NULL)
		return false;
	bool bStyled = false;
	pWnd->GetWindowText(sText);
	// the rich edit control doesn't count the CR char!
	// to be exact: CRLF is treated as one char.
	sText.Remove(_T('\r'));

	// style each line separately
	int offset = 0;
	int nNewlinePos;
	do
	{
		nNewlinePos = sText.Find('\n', offset);
		CString sLine = nNewlinePos >= 0 ? sText.Mid(offset, nNewlinePos - offset) : sText.Mid(offset);

		int start = 0;
		int end = 0;
		while (FindStyleChars(sLine, '*', start, end))
		{
			CHARRANGE range = {(LONG)start+offset, (LONG)end+offset};
			pWnd->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&range);
			SetCharFormat(pWnd, CFM_BOLD, CFE_BOLD);
			bStyled = true;
			start = end;
		}
		start = 0;
		end = 0;
		while (FindStyleChars(sLine, '^', start, end))
		{
			CHARRANGE range = {(LONG)start+offset, (LONG)end+offset};
			pWnd->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&range);
			SetCharFormat(pWnd, CFM_ITALIC, CFE_ITALIC);
			bStyled = true;
			start = end;
		}
		start = 0;
		end = 0;
		while (FindStyleChars(sLine, '_', start, end))
		{
			CHARRANGE range = {(LONG)start+offset, (LONG)end+offset};
			pWnd->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&range);
			SetCharFormat(pWnd, CFM_UNDERLINE, CFE_UNDERLINE);
			bStyled = true;
			start = end;
		}
		offset = nNewlinePos+1;
	} while(nNewlinePos>=0);
	return bStyled;
}

bool CAppUtils::FindStyleChars(const CString& sText, TCHAR stylechar, int& start, int& end)
{
	int i=start;
	int last = sText.GetLength() - 1;
	bool bFoundMarker = false;
	TCHAR c = i == 0 ? _T('\0') : sText[i - 1];
	TCHAR nextChar = i >= last ? _T('\0') : sText[i + 1];

	// find a starting marker
	while (i < last)
	{
		TCHAR prevChar = c;
		c = nextChar;
		nextChar = sText[i + 1];

		// IsCharAlphaNumeric can be somewhat expensive.
		// Long lines of "*****" or "----" will be pre-empted efficiently
		// by the (c != nextChar) condition.

		if ((c == stylechar) && (c != nextChar))
		{
			if (IsCharAlphaNumeric(nextChar) && !IsCharAlphaNumeric(prevChar))
			{
				start = ++i;
				bFoundMarker = true;
				break;
			}
		}
		++i;
	}
	if (!bFoundMarker)
		return false;

	// find ending marker
	// c == sText[i - 1]

	bFoundMarker = false;
	while (i <= last)
	{
		TCHAR prevChar = c;
		c = sText[i];
		if (c == stylechar)
		{
			if ((i == last) || (!IsCharAlphaNumeric(sText[i + 1]) && IsCharAlphaNumeric(prevChar)))
			{
				end = i;
				++i;
				bFoundMarker = true;
				break;
			}
		}
		++i;
	}
	return bFoundMarker;
}

// from CSciEdit
namespace {
	bool IsValidURLChar(wchar_t ch)
	{
		return iswalnum(ch) ||
			ch == L'_' || ch == L'/' || ch == L';' || ch == L'?' || ch == L'&' || ch == L'=' ||
			ch == L'%' || ch == L':' || ch == L'.' || ch == L'#' || ch == L'-' || ch == L'+' ||
			ch == L'|' || ch == L'>' || ch == L'<';
	}

	bool IsUrl(const CString& sText)
	{
		if (!PathIsURLW(sText))
			return false;
		if (sText.Find(L"://") >= 0)
			return true;
		return false;
	}
}

BOOL CAppUtils::StyleURLs(const CString& msg, CWnd* pWnd)
{
	std::vector<CHARRANGE> positions = FindURLMatches(msg);
	CAppUtils::SetCharFormat(pWnd, CFM_LINK, CFE_LINK, positions);

	return positions.empty() ? FALSE : TRUE;
}

/**
* implements URL searching with the same logic as CSciEdit::StyleURLs
*/
std::vector<CHARRANGE> CAppUtils::FindURLMatches(const CString& msg)
{
	std::vector<CHARRANGE> result;

	int len = msg.GetLength();
	int starturl = -1;

	for (int i = 0; i <= msg.GetLength(); ++i)
	{
		if ((i < len) && IsValidURLChar(msg[i]))
		{
			if (starturl < 0)
				starturl = i;
		}
		else
		{
			if (starturl >= 0)
			{
				bool strip = true;
				if (msg[starturl] == '<' && i < len) // try to detect and do not strip URLs put within <>
				{
					while (starturl <= i && msg[starturl] == '<') // strip leading '<'
						++starturl;
					strip = false;
					i = starturl;
					while (i < len && msg[i] != '\r' && msg[i] != '\n' && msg[i] != '>') // find first '>' or new line after resetting i to start position
						++i;
				}
				if (!IsUrl(msg.Mid(starturl, i - starturl)))
				{
					starturl = -1;
					continue;
				}

				int skipTrailing = 0;
				while (strip && i - skipTrailing - 1 > starturl && (msg[i - skipTrailing - 1] == '.' || msg[i - skipTrailing - 1] == '-' || msg[i - skipTrailing - 1] == '?' || msg[i - skipTrailing - 1] == ';' || msg[i - skipTrailing - 1] == ':' || msg[i - skipTrailing - 1] == '>' || msg[i - skipTrailing - 1] == '<'))
					++skipTrailing;
				
				CHARRANGE range = { starturl, i - skipTrailing };
				result.push_back(range);
			}
			starturl = -1;
		}
	}

	return result;
}

CString CAppUtils::GetClipboardLink(const CString &skipGitPrefix, int paramsCount)
{
	if (!OpenClipboard(NULL))
		return CString();

	CString sClipboardText;
	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if (hglb)
	{
		LPCSTR lpstr = (LPCSTR)GlobalLock(hglb);
		sClipboardText = CString(lpstr);
		GlobalUnlock(hglb);
	}
	hglb = GetClipboardData(CF_UNICODETEXT);
	if (hglb)
	{
		LPCTSTR lpstr = (LPCTSTR)GlobalLock(hglb);
		sClipboardText = lpstr;
		GlobalUnlock(hglb);
	}
	CloseClipboard();

	if(!sClipboardText.IsEmpty())
	{
		if(sClipboardText[0] == _T('\"') && sClipboardText[sClipboardText.GetLength()-1] == _T('\"'))
			sClipboardText=sClipboardText.Mid(1,sClipboardText.GetLength()-2);

		if(sClipboardText.Find( _T("http://")) == 0)
			return sClipboardText;

		if(sClipboardText.Find( _T("https://")) == 0)
			return sClipboardText;

		if(sClipboardText.Find( _T("git://")) == 0)
			return sClipboardText;

		if(sClipboardText.Find( _T("ssh://")) == 0)
			return sClipboardText;

		if (sClipboardText.Find(_T("git@")) == 0)
			return sClipboardText;

		if(sClipboardText.GetLength()>=2)
			if( sClipboardText[1] == _T(':') )
				if( (sClipboardText[0] >= 'A' &&  sClipboardText[0] <= 'Z')
					|| (sClipboardText[0] >= 'a' &&  sClipboardText[0] <= 'z') )
					return sClipboardText;

		// trim prefixes like "git clone "
		if (!skipGitPrefix.IsEmpty() && sClipboardText.Find(skipGitPrefix) == 0)
		{
			sClipboardText = sClipboardText.Mid(skipGitPrefix.GetLength()).Trim();
			int spacePos = -1;
			while (paramsCount >= 0)
			{
				--paramsCount;
				spacePos = sClipboardText.Find(_T(' '), spacePos + 1);
				if (spacePos == -1)
					break;
			}
			if (spacePos > 0 && paramsCount < 0)
				sClipboardText = sClipboardText.Left(spacePos);
			return sClipboardText;
		}
	}

	return CString(_T(""));
}

bool CAppUtils::CreateMultipleDirectory(const CString& szPath)
{
	CString strDir(szPath);
	if (strDir.GetAt(strDir.GetLength()-1)!=_T('\\'))
	{
		strDir.AppendChar(_T('\\'));
	}
	std::vector<CString> vPath;
	CString strTemp;
	bool bSuccess = false;

	for (int i=0;i<strDir.GetLength();++i)
	{
		if (strDir.GetAt(i) != _T('\\'))
		{
			strTemp.AppendChar(strDir.GetAt(i));
		}
		else
		{
			vPath.push_back(strTemp);
			strTemp.AppendChar(_T('\\'));
		}
	}

	for (auto vIter = vPath.begin(); vIter != vPath.end(); ++vIter)
	{
		bSuccess = CreateDirectory(*vIter, NULL) ? true : false;
	}

	return bSuccess;
}

void CAppUtils::RemoveTrailSlash(CString &path)
{
	if(path.IsEmpty())
		return ;

	// For URL, do not trim the slash just after the host name component.
	int index = path.Find(_T("://"));
	if (index >= 0)
	{
		index += 4;
		index = path.Find(_T('/'), index);
		if (index == path.GetLength() - 1)
			return;
	}

	while(path[path.GetLength()-1] == _T('\\') || path[path.GetLength()-1] == _T('/' ) )
	{
		path=path.Left(path.GetLength()-1);
		if(path.IsEmpty())
			return;
	}
}

BOOL CAppUtils::SICommit()
{
	bool bFailed = true;

	while (bFailed)
	{
		bFailed = false;
		CSICommitDlg dlg;

		if (dlg.DoModal() == IDOK)
		{
		}
	}
	return true;
}

void CAppUtils::MarkWindowAsUnpinnable(HWND hWnd)
{
	typedef HRESULT (WINAPI *SHGPSFW) (HWND hwnd,REFIID riid,void** ppv);

	CAutoLibrary hShell = AtlLoadSystemLibraryUsingFullPath(_T("Shell32.dll"));

	if (hShell.IsValid()) {
		SHGPSFW pfnSHGPSFW = (SHGPSFW)::GetProcAddress(hShell, "SHGetPropertyStoreForWindow");
		if (pfnSHGPSFW) {
			IPropertyStore *pps;
			HRESULT hr = pfnSHGPSFW(hWnd, IID_PPV_ARGS(&pps));
			if (SUCCEEDED(hr)) {
				PROPVARIANT var;
				var.vt = VT_BOOL;
				var.boolVal = VARIANT_TRUE;
				pps->SetValue(PKEY_AppUserModel_PreventPinning, var);
				pps->Release();
			}
		}
	}
}

void CAppUtils::SetWindowTitle(HWND hWnd, const CString& urlorpath, const CString& dialogname)
{
	ASSERT(dialogname.GetLength() < 70);
	ASSERT(urlorpath.GetLength() < MAX_PATH);
	WCHAR pathbuf[MAX_PATH] = {0};

	PathCompactPathEx(pathbuf, urlorpath, 70 - dialogname.GetLength(), 0);

	wcscat_s(pathbuf, L" - ");
	wcscat_s(pathbuf, dialogname);
	wcscat_s(pathbuf, L" - ");
	wcscat_s(pathbuf, CString(MAKEINTRESOURCE(IDS_APPNAME)));
	SetWindowText(hWnd, pathbuf);
}

void CAppUtils::ExploreTo(HWND hwnd, CString path)
{
	if (PathFileExists(path))
	{
		ITEMIDLIST __unaligned * pidl = ILCreateFromPath(path);
		if (pidl)
		{
			SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
			ILFree(pidl);
		}
		return;
	}
	// if filepath does not exist any more, navigate to closest matching folder
	do
	{
		int pos = path.ReverseFind(_T('\\'));
		if (pos <= 3)
			break;
		path = path.Left(pos);
	} while (!PathFileExists(path));
	ShellExecute(hwnd, _T("explore"), path, nullptr, nullptr, SW_SHOW);
}

bool CAppUtils::ShellOpen(const CString& file, HWND hwnd /*= nullptr */)
{
	if ((int)ShellExecute(hwnd, NULL, file, NULL, NULL, SW_SHOW) > HINSTANCE_ERROR)
		return true;

	return ShowOpenWithDialog(file, hwnd);
}

bool CAppUtils::ShowOpenWithDialog(const CString& file, HWND hwnd /*= nullptr */)
{
	CAutoLibrary hShell = AtlLoadSystemLibraryUsingFullPath(_T("shell32.dll"));
	if (hShell)
	{
		typedef HRESULT STDAPICALLTYPE SHOpenWithDialoFN(_In_opt_ HWND hwndParent, _In_ const OPENASINFO *poainfo);
		SHOpenWithDialoFN *pfnSHOpenWithDialog = (SHOpenWithDialoFN*)GetProcAddress(hShell, "SHOpenWithDialog");
		if (pfnSHOpenWithDialog)
		{
			OPENASINFO oi = { 0 };
			oi.pcszFile = file;
			oi.oaifInFlags = OAIF_EXEC;
			return SUCCEEDED(pfnSHOpenWithDialog(hwnd, &oi));
		}
	}
	CString cmd = _T("RUNDLL32 Shell32,OpenAs_RunDLL ");
	cmd += file;
	return CAppUtils::LaunchApplication(cmd, NULL, false);
}
