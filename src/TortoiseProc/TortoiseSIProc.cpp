// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2014 - TortoiseGit
// Copyright (C) 2003-2008, 2012-2014 - TortoiseSVN

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
#include "TortoiseSIProc.h"
#include "CmdLineParser.h"
#include "Common\AppUtils.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include "UnicodeUtils.h"
#include "MessageBox.h"
#include "DirFileEnum.h"
#include "SmartHandle.h"
#include "Commands\Command.h"
#include "..\version.h"
#include "Common\SinglePropSheetDlg.h"
#include "commands\setmainpage.h"
#include "Libraries.h"
#include "TaskbarUUID.h"
#include <math.h>

#define STRUCT_IOVEC_DEFINED

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

/*****************************************************************************
 * Globals
 ****************************************************************************/
// The only CTortoiseSIProcApp object
CTortoiseSIProcApp theApp;

CString g_sGroupingIcon;
bool g_bGroupingRemoveIcon = false;

BEGIN_MESSAGE_MAP(CTortoiseSIProcApp, CWinAppEx)
	ON_COMMAND(ID_HELP, CWinAppEx::OnHelp)
END_MESSAGE_MAP()

CTortoiseSIProcApp::CTortoiseSIProcApp()
{
	CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": CTortoiseSIProcApp()\n"));

	// Remove the current directory from the default DLL search order.
	// Search in System32, System, Windows and the PATH.
	SetDllDirectory(L"");

	// Use HTML help for this application
	EnableHtmlHelp();

	m_bLoadUserToolbars = FALSE;
	m_bSaveState = FALSE;
	m_bRetSuccess = false;
	m_gdiplusToken = NULL;

	// Programs compiled with 64-bit VS 2013 C++ compiler crash with "0xC000001D: Illegal Instruction" on
	// Windows 7 x64 SP1 if support for the AVX2 is not available on the CPU.  This is the work-around.
#if defined (_WIN64) && _MSC_VER >= 1800
	_set_FMA3_enable(0);
#endif
}

CTortoiseSIProcApp::~CTortoiseSIProcApp()
{

}

BOOL CTortoiseSIProcApp::InitInstance()
{
	CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": InitInstance()\n"));

	// Set the visual manage that is used to redraw the application
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Specifies that the style of the button border corresponds to the current Windows theme
	CMFCButton::EnableWindowsTheming();
	
	// Initialize Windows GDI+ (must be called before any GDI+ call)
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	LoadLanguageDLL();

	SetHelpFile();

	setlocale(LC_ALL, "");

    // TODO: Check that TortoiseSI exists? For now we assume it does

	// TODO: Check that we have the minimum required version of TortoiseSI

	InitializeUIComponents();

	CWinAppEx::InitInstance();

	// Application settings to be stored in registry instead of INI files.
	// This also sets m_pszRegistryKey
	SetRegistryKey(_T("TortoiseSIProc"));


	if (!ProcessCommandLine()) {
		return FALSE;
	}



	CAutoGeneralHandle TGitMutex = ::CreateMutex(NULL, FALSE, _T("TortoiseSIProc.exe"));
	if (!g_Git.SetCurrentDir(cmdLinePath.GetWinPathString(), parser.HasKey(_T("submodule")) == TRUE))
	{
		for (int i = 0; i < pathList.GetCount(); ++i)
			if(g_Git.SetCurrentDir(pathList[i].GetWinPath()))
				break;
	}

	if(!g_Git.m_CurrentDir.IsEmpty())
	{
		sOrigCWD = g_Git.m_CurrentDir;
		SetCurrentDirectory(g_Git.m_CurrentDir);
	}

	if (m_sGroupingUUID.IsEmpty())
	{
		CRegStdDWORD groupSetting = CRegStdDWORD(_T("Software\\TortoiseGit\\GroupTaskbarIconsPerRepo"), 3);
		switch (DWORD(groupSetting))
		{
		case 1:
		case 2:
			// implemented differently to TortoiseSVN atm
			break;
		case 3:
		case 4:
			{
				CString wcroot;
				if (g_GitAdminDir.HasAdminDir(g_Git.m_CurrentDir, true, &wcroot))
				{
					git_oid oid;
					CStringA wcRootA(wcroot + CPathUtils::GetAppDirectory());
					if (!git_odb_hash(&oid, wcRootA, wcRootA.GetLength(), GIT_OBJ_BLOB))
					{
						CStringA hash;
						git_oid_tostr(hash.GetBufferSetLength(GIT_OID_HEXSZ + 1), GIT_OID_HEXSZ + 1, &oid);
						hash.ReleaseBuffer();
						g_sGroupingUUID = hash;
					}
					ProjectProperties pp;
					pp.ReadProps();
					CString icon = pp.sIcon;
					icon.Replace('/', '\\');
					if (icon.IsEmpty())
						g_bGroupingRemoveIcon = true;
					g_sGroupingIcon = icon;
				}
			}
		}
	}

	CString sAppID = GetTaskIDPerUUID(g_sGroupingUUID).c_str();
	EnsureGitLibrary(false);

	{
		CString err;
		try
		{
			// requires CWD to be set
			CGit::m_LogEncode = CAppUtils::GetLogOutputEncode();

			// make sure all config files are read in order to check that none contains an error
			g_Git.GetConfigValue(_T("doesnot.exist"));
		}
		catch (char* msg)
		{
			err = CString(msg);
		}

		if (!err.IsEmpty())
		{
			UINT choice = CMessageBox::Show(hWndExplorer, err, _T("TortoiseGit"), 1, IDI_ERROR, CString(MAKEINTRESOURCE(IDS_PROC_EDITLOCALGITCONFIG)), CString(MAKEINTRESOURCE(IDS_PROC_EDITGLOBALGITCONFIG)), CString(MAKEINTRESOURCE(IDS_ABORTBUTTON)));
			if (choice == 1)
			{
				// open the config file with alternative editor
				CAppUtils::LaunchAlternativeEditor(g_Git.GetGitLocalConfig());
			}
			else if (choice == 2)
			{
				// open the global config file with alternative editor
				CAppUtils::LaunchAlternativeEditor(g_Git.GetGitGlobalConfig());
			}
			return FALSE;
		}
	}

	// execute the requested command
	CommandServer server;
	Command * cmd = server.GetCommand(parser.GetVal(_T("command")));
	if (cmd)
	{
		cmd->SetExplorerHwnd(hWndExplorer);

		cmd->SetParser(parser);
		cmd->SetPaths(pathList, cmdLinePath);

		m_bRetSuccess = cmd->Execute();
		delete cmd;
	}

	// Look for temporary files left around by TortoiseSVN and
	// remove them. But only delete 'old' files because some
	// apps might still be needing the recent ones.
	{
		DWORD len = GetTortoiseGitTempPath(0, NULL);
		std::unique_ptr<TCHAR[]> path(new TCHAR[len + 100]);
		len = GetTortoiseGitTempPath (len + 100, path.get());
		if (len != 0)
		{
			CDirFileEnum finder(path.get());
			FILETIME systime_;
			::GetSystemTimeAsFileTime(&systime_);
			__int64 systime = (((_int64)systime_.dwHighDateTime)<<32) | ((__int64)systime_.dwLowDateTime);
			bool isDir;
			CString filepath;
			while (finder.NextFile(filepath, &isDir))
			{
				HANDLE hFile = ::CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, isDir ? FILE_FLAG_BACKUP_SEMANTICS : NULL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					FILETIME createtime_;
					if (::GetFileTime(hFile, &createtime_, NULL, NULL))
					{
						::CloseHandle(hFile);
						__int64 createtime = (((_int64)createtime_.dwHighDateTime)<<32) | ((__int64)createtime_.dwLowDateTime);
						if ((createtime + 864000000000) < systime)		//only delete files older than a day
						{
							::SetFileAttributes(filepath, FILE_ATTRIBUTE_NORMAL);
							if (isDir)
								::RemoveDirectory(filepath);
							else
								::DeleteFile(filepath);
						}
					}
					else
						::CloseHandle(hFile);
				}
			}
		}
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	// application, rather than start the application's message pump.
	return FALSE;
}

int CTortoiseSIProcApp::ExitInstance()
{
	CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": ExitInstance()\n"));

	// Finished using GDI+
	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	CWinAppEx::ExitInstance();

	if (m_bRetSuccess)
		return 0;
	return -1;
}

void CTortoiseSIProcApp::LoadLanguageDLL()
{
	CRegDWORD regLangId = CRegDWORD(_T("Software\\TortoiseSI\\LanguageID"), 1033);
	long langId = regLangId;

	CString langDll;

	do
	{
		langDll.Format(_T("%sLanguages\\TortoiseProc%ld.dll"), (LPCTSTR)CPathUtils::GetAppParentDirectory(), langId);

		CString sVer = _T(STRPRODUCTVER);
		CString sFileVer = CPathUtils::GetVersionFromFile(langDll);

		if (sFileVer == sVer)
		{
			// Language DLL version matches TortoiseSIProc version, so load language DLL
			HINSTANCE hInst = LoadLibrary(langDll);

			if (hInst != NULL)
			{
				CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": Loaded Language DLL %s\n"), langDll);

				// Tell the application that this is the default resources for the application
				AfxSetResourceHandle(hInst);
				break;
			}

		} 

		DWORD lid = SUBLANGID(langId) - 1;

		if (lid > 0) {
			langId = MAKELANGID(PRIMARYLANGID(langId), lid);
		} else {
			langId = 0;
		}

	} while (langId != 0);

}

void CTortoiseSIProcApp::SetHelpFile()
{
	CRegDWORD regLangId = CRegDWORD(_T("Software\\TortoiseSI\\LanguageID"), 1033);
	TCHAR buf[6] = { 0 };
	long langId;

	_tcscpy_s(buf, _T("en"));
	langId = regLangId;

	// MFC uses a help file with the same name as the application by default,
	// which means we have to change that default to our language specific help files
	CString sHelpPath = CPathUtils::GetAppDirectory() + _T("TortoiseSI_en.chm");

	free((void*)m_pszHelpFilePath);

	m_pszHelpFilePath = _tcsdup(sHelpPath);

	sHelpPath = CPathUtils::GetAppParentDirectory() + _T("Languages\\TortoiseSI_en.chm");

	do
	{
		CString sLang = _T("_");

		if (GetLocaleInfo(MAKELCID(langId, SORT_DEFAULT), LOCALE_SISO639LANGNAME, buf, _countof(buf)))
		{
			sLang += buf;
			sHelpPath.Replace(_T("_en"), sLang);
			if (PathFileExists(sHelpPath))
			{
				free((void*)m_pszHelpFilePath);
				m_pszHelpFilePath = _tcsdup(sHelpPath);
				break;
			}
		}

		sHelpPath.Replace(sLang, _T("_en"));

		if (GetLocaleInfo(MAKELCID(langId, SORT_DEFAULT), LOCALE_SISO3166CTRYNAME, buf, _countof(buf)))
		{
			sLang += _T("_");
			sLang += buf;
			sHelpPath.Replace(_T("_en"), sLang);
			if (PathFileExists(sHelpPath))
			{
				free((void*)m_pszHelpFilePath);
				m_pszHelpFilePath = _tcsdup(sHelpPath);
				break;
			}
		}

		sHelpPath.Replace(sLang, _T("_en"));

		DWORD lid = SUBLANGID(langId) - 1;

		if (lid > 0) {
			langId = MAKELANGID(PRIMARYLANGID(langId), lid);
		} else {
			langId = 0;
		}

	} while (langId);

	CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": Set Help Filename %s\n"), m_pszHelpFilePath);
}

void CTortoiseSIProcApp::InitializeUIComponents()
{
	CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": Initializing UI components ...\n"));

	// InitCommonControls() is required on Windows XP if an application manifest 
	// specifies use of ComCtl32.dll version 6 or later to enable visual styles.
	// Otherwise, any window creation will fail.

	INITCOMMONCONTROLSEX used = {
		sizeof(INITCOMMONCONTROLSEX),
		ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_DATE_CLASSES |
		ICC_HOTKEY_CLASS | ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES |
		ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS |
		ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES | ICC_UPDOWN_CLASS |
		ICC_USEREX_CLASSES | ICC_WIN95_CLASSES
	};

	// Ensures that the common control DLL (Comctl32.dll) is loaded and registers
	// the specified common control classes from the DLL.  This must be called
	// before creating a common control.
	InitCommonControlsEx(&used);

	// Initialize OLE support
	AfxOleInit();

	// Initialize support for containment of OLE controls
	AfxEnableControlContainer();

	// Initialize Rich Edit Controls version 5.0 and later
	AfxInitRichEdit5();
}

BOOL CTortoiseSIProcApp::ProcessCommandLine()
{
	CCmdLineParser parser(AfxGetApp()->m_lpCmdLine);

	CString sVal = parser.GetVal(_T("hwnd"));

	if (!sVal.IsEmpty()) {
		m_hWndExplorer = (HWND)_wcstoui64(sVal, nullptr, 16);
	}

	while (GetParent(m_hWndExplorer) != NULL) {
		m_hWndExplorer = GetParent(m_hWndExplorer);
	}

	if (!IsWindow(m_hWndExplorer))
	{
		m_hWndExplorer = NULL;
	}

#if _DEBUG
	if (CRegDWORD(_T("Software\\TortoiseSI\\Debug"), FALSE) == TRUE) {
		AfxMessageBox(AfxGetApp()->m_lpCmdLine, MB_OK | MB_ICONINFORMATION);
	}
#endif

	// The path and pathfile arguments are mutually exclusive
	if (parser.HasKey(_T("path")) && parser.HasKey(_T("pathfile")))
	{
		CMessageBox::Show(NULL, IDS_ERR_INVALIDPATH, IDS_APPNAME, MB_ICONERROR);
		return FALSE;
	}

	if (m_sGroupingUUID.IsEmpty()) {
		m_sGroupingUUID = parser.GetVal(L"groupuuid");
	}

	CTGitPath cmdLinePath;
	CTGitPathList pathList;

	if (parser.HasKey(_T("pathfile")))
	{
		// Process /pathfile argument

		CString sPathFileArgument = CPathUtils::GetLongPathname(parser.GetVal(_T("pathfile")));

		cmdLinePath.SetFromUnknown(sPathFileArgument);

		if (pathList.LoadFromFile(cmdLinePath) == false) {
			// no path specified!
			return FALSE;		
	    }

		if (parser.HasKey(_T("deletepathfile")))
		{
			// We can delete the temporary path file, now that we've loaded it
			::DeleteFile(cmdLinePath.GetWinPath());
		}

		// This was a path to a temporary file - it's got no meaning now, and
		// anybody who uses it again is in for a problem...
		cmdLinePath.Reset();

	} else {

		// Process /path and /expaths argument
		// Build-uo /path value as /path:<value>*<arg1>*<arg2>...
		// Where <arg1>, <arg2> etc are extracted from /expath arguments.
		// Since all arguments are passed in the form of /parameter:<argument>
		// We process only those arguments which are not preceded by a /parameter:
		// This only includes arguments specified following /expaths (See comments below)

		CString sPathArgument = CPathUtils::GetLongPathname(parser.GetVal(_T("path")));

		if (parser.HasKey(_T("expaths")))
		{
			// An /expaths param means we are started via the buttons in our Win7 library
			// and that means the value of /expaths is the current directory and
			// the selected paths are then added as additional parameters but without a key, only a value
			//
			//     e.g. /expaths:"D:\" "D:\Utils"
			//
			// Because of the "strange treatment of quotation marks and backslashes by CommandLineToArgvW",
			// we have to escape the backslashes first.  Since we're only dealing with paths here, that's
			// a safe bet.  Without this, a command line like:
			//
			//     /command:commit /expaths:"D:\" "D:\Utils"
			// 
			// would fail because the "D:\" is treated as the backslash being the escape char for the quotation
			// mark and we'd end up with:
			//
			//     argv[1] = /command:commit
			//     argv[2] = /expaths:D:" D:\Utils
			//
			// See here for more details: http://blogs.msdn.com/b/oldnewthing/archive/2010/09/17/10063629.aspx

			CString cmdLine = GetCommandLineW();

			// Escape backslashes
			cmdLine.Replace(L"\\", L"\\\\");

			int nArgs = 0;
			LPWSTR *szArgList = CommandLineToArgvW(cmdLine, &nArgs);

			if (szArgList)
			{
				// Argument 0 is the process path, so start with 1
				for (int i = 1; i < nArgs; i++)
				{
					if (szArgList[i][0] != '/')
					{
						if (!sPathArgument.IsEmpty()) {
							sPathArgument += '*';
						}
						sPathArgument += szArgList[i];
					}
				}

				sPathArgument.Replace(L"\\\\", L"\\");
			}

			LocalFree(szArgList);
		}

		if (sPathArgument.IsEmpty() && parser.HasKey(L"path"))
		{
			CMessageBox::Show(m_hWndExplorer, IDS_ERR_INVALIDPATH, IDS_APPNAME, MB_ICONERROR);
			return FALSE;
		}

		int asterisk = sPathArgument.Find('*');
		cmdLinePath.SetFromUnknown(asterisk >= 0 ? sPathArgument.Left(asterisk) : sPathArgument);
		pathList.LoadFromAsteriskSeparatedString(sPathArgument);
	}

	if (pathList.IsEmpty()) {
		pathList.AddPath(CTGitPath::CTGitPath(g_Git.m_CurrentDir));
	}

	// Set CWD to temporary dir, and restore it later
	{
		DWORD len = GetCurrentDirectory(0, NULL);

		if (len)
		{
			std::unique_ptr<TCHAR[]> originalCurrentDirectory(new TCHAR[len]);

			if (GetCurrentDirectory(len, originalCurrentDirectory.get()))
			{
				m_sOrigCWD = originalCurrentDirectory.get();
				m_sOrigCWD = CPathUtils::GetLongPathname(m_sOrigCWD);
			}
		}

		TCHAR pathbuf[MAX_PATH] = { 0 };
		GetTortoiseGitTempPath(MAX_PATH, pathbuf);
		SetCurrentDirectory(pathbuf);
	}

	return TRUE;
}
