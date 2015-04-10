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
#include "PathUtils.h"
#include "MessageBox.h"
#include "DirFileEnum.h"
#include "SmartHandle.h"
#include "Commands\Command.h"
#include "EventLog.h"
#include "Git.h"
#include "..\version.h"
#include <math.h>

#define STRUCT_IOVEC_DEFINED

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#undef min
#undef max

/*****************************************************************************
 * Globals
 ****************************************************************************/
// The only CTortoiseSIProcApp object
CTortoiseSIProcApp theApp;

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

	/* 
	 * Create a named mutex to limit application to a single instance.
	 * TODO: Create a randomly named mutex and store the name so that it can only be obtained
	 *       by an authorized user to prevent a malicious application from creating the
	 *       named mutex before us and purposely preventing our application from running.
	 *       Alternatively, create a locked file in the users profile directory.
	 */
	CAutoGeneralHandle TGitMutex = ::CreateMutex(NULL, FALSE, _T("TortoiseSIProc.exe"));

	/* Establish Integrity server connection */
	int port = getIntegrationPort();

	if (port > 0 && port <= std::numeric_limits<unsigned short>::max()) {
		EventLog::writeInformation(L"TortoiseSIProcApp connecting to Integrity client via localhost:" + std::to_wstring(port));
		m_integritySession = std::unique_ptr<IntegritySession>(new IntegritySession("localhost", port));

	}
	else {
		EventLog::writeInformation(L"TortoiseSIProcApp connecting to Integrity client via localintegration point");
		m_integritySession = std::unique_ptr<IntegritySession>(new IntegritySession());
	}

	m_serverConnectionsCache = std::unique_ptr<ServerConnections>(new ServerConnections(*m_integritySession));

	//if (!m_serverConnectionsCache->isOnline()) {
	//	EventLog::writeInformation(L"TortoiseSIProcApp::InitInstance() bailing out, unable to connected to server");
	//	return FALSE;
	//}

	if (!ProcessCommandLine()) {
		return FALSE;
	}

	return TRUE;
}

int CTortoiseSIProcApp::ExitInstance()
{
	CTraceToOutputDebugString::Instance()(_T(__FUNCTION__) _T(": ExitInstance()\n"));

	// Finished using GDI+
	Gdiplus::GdiplusShutdown(m_gdiplusToken);

	CWinAppEx::ExitInstance();

	m_serverConnectionsCache = NULL;
	m_integritySession = NULL;

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

	if (!g_Git.m_CurrentDir.IsEmpty())
	{
		m_sOrigCWD = g_Git.m_CurrentDir;
		SetCurrentDirectory(m_sOrigCWD);
	}

	// Execute the requested command
	CommandServer server;

	Command *cmd = server.GetCommand(parser.GetVal(_T("command")));

	if (cmd)
	{
		cmd->SetExplorerHwnd(m_hWndExplorer);
		cmd->SetParser(parser);
		cmd->SetPaths(pathList, cmdLinePath);
		m_bRetSuccess = cmd->Execute();
		delete cmd;
	}

	return TRUE;
}

int CTortoiseSIProcApp::getIntegrationPort()
{
	CRegStdDWORD integrationPointKey(L"Software\\TortoiseSI\\IntegrationPoint", (DWORD)-1);
	integrationPointKey.read();

	int port = (DWORD)integrationPointKey;
	return port;
}