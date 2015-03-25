// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2003-2008, 2014 - TortoiseSVN
// Copyright (C) 2008-2014 - TortoiseGit

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

#include "StatusCache.h"
#include "ShellExt.h"
#include "SIPropertyPage.h"
#include "UnicodeUtils.h"
#include "PathUtils.h"
#include "UnicodeUtils.h"
#include "CreateProcessHelper.h"
#include "FormatMessageWrapper.h"
#include "IntegrityActions.h"
#include "IntegrityResponse.h"

typedef CComCritSecLock<CComCriticalSection> CAutoLocker;

#define MAX_STRING_LENGTH	4096	//should be big enough

// Nonmember function prototypes
BOOL CALLBACK PageProc (HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK PropPageCallbackProc ( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );

static const IntegritySession& getIntegritySession() {
	return IStatusCache::getInstance().getIntegritySession();
}

/////////////////////////////////////////////////////////////////////////////
// Dialog procedures and other callback functions

BOOL CALLBACK PageProc (HWND hwnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	CSIPropertyPage * sheetpage;

	if (uMessage == WM_INITDIALOG)
	{
		sheetpage = (CSIPropertyPage*) ((LPPROPSHEETPAGE) lParam)->lParam;
		SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) sheetpage);
		sheetpage->SetHwnd(hwnd);
	}
	else
	{
		sheetpage = (CSIPropertyPage*) GetWindowLongPtr (hwnd, GWLP_USERDATA);
	}

	if (sheetpage != 0L)
		return sheetpage->PageProc(hwnd, uMessage, wParam, lParam);
	else
		return FALSE;
}

UINT CALLBACK PropPageCallbackProc ( HWND /*hwnd*/, UINT uMsg, LPPROPSHEETPAGE ppsp )
{
	// Delete the page before closing.
	if (PSPCB_RELEASE == uMsg)
	{
		CSIPropertyPage* sheetpage = (CSIPropertyPage*) ppsp->lParam;
		if (sheetpage != NULL)
			delete sheetpage;
	}
	return 1;
}

// *********************** CGitPropertyPage *************************
const UINT CSIPropertyPage::m_UpdateLastCommit = RegisterWindowMessage(_T("TORTOISEGIT_PROP_UPDATELASTCOMMIT"));

CSIPropertyPage::CSIPropertyPage(const std::vector<stdstring> &newFilenames)
	:filenames(newFilenames)
	,m_bChanged(false)
	, m_hwnd(NULL)
{
}

CSIPropertyPage::~CSIPropertyPage(void)
{
}

void CSIPropertyPage::SetHwnd(HWND newHwnd)
{
	m_hwnd = newHwnd;
}

BOOL CSIPropertyPage::PageProc (HWND /*hwnd*/, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		{
			InitWorkfileView();
			return TRUE;
		}
	case WM_NOTIFY:
		{
			LPNMHDR point = (LPNMHDR)lParam;
			int code = point->code;
			//
			// Respond to notifications.
			//
			if (code == PSN_APPLY && m_bChanged)
			{

			}
			SetWindowLongPtr (m_hwnd, DWLP_MSGRESULT, FALSE);
			return TRUE;
		}
		case WM_DESTROY:
			return TRUE;

		case WM_COMMAND:
		PageProcOnCommand(wParam);
		break;
	} // switch (uMessage)

	if (uMessage == m_UpdateLastCommit)
	{
		//DisplayCommit((git_commit *)lParam, IDC_LAST_HASH, IDC_LAST_SUBJECT, IDC_LAST_AUTHOR, IDC_LAST_DATE);
		return TRUE;
	}

	return FALSE;
}
void CSIPropertyPage::PageProcOnCommand(WPARAM wParam)
{
	if(HIWORD(wParam) != BN_CLICKED)
		return;

	switch (LOWORD(wParam))
	{
	case IDC_SHOWLOG:
		{
			tstring gitCmd = _T(" /command:");
			gitCmd += _T("log /path:\"");
			gitCmd += filenames.front().c_str();
			gitCmd += _T("\"");
	//		RunCommand(gitCmd);
		}
		break;
	case IDC_SHOWSETTINGS:
		{
			
		}
		break;
	case IDC_ASSUMEVALID:
	case IDC_SKIPWORKTREE:
	case IDC_EXECUTABLE:
	case IDC_SYMLINK:
		BOOL executable = (BOOL)SendMessage(GetDlgItem(m_hwnd, IDC_EXECUTABLE), BM_GETCHECK, 0, 0);
		BOOL symlink = (BOOL)SendMessage(GetDlgItem(m_hwnd, IDC_SYMLINK), BM_GETCHECK, 0, 0);
		if (executable == BST_CHECKED)
		{
			EnableWindow(GetDlgItem(m_hwnd, IDC_SYMLINK), FALSE);
			SendMessage(GetDlgItem(m_hwnd, IDC_SYMLINK), BM_SETCHECK, BST_UNCHECKED, 0);
		}
		else
			EnableWindow(GetDlgItem(m_hwnd, IDC_SYMLINK), TRUE);
		if (symlink == BST_CHECKED)
		{
			EnableWindow(GetDlgItem(m_hwnd, IDC_EXECUTABLE), FALSE);
			SendMessage(GetDlgItem(m_hwnd, IDC_EXECUTABLE), BM_SETCHECK, BST_UNCHECKED, 0);
		}
		else
			EnableWindow(GetDlgItem(m_hwnd, IDC_EXECUTABLE), TRUE);
		m_bChanged = true;
		SendMessage(GetParent(m_hwnd), PSM_CHANGED, (WPARAM)m_hwnd, 0);
		break;
	}
}

void CSIPropertyPage::RunCommand(const tstring& command)
{
//	tstring tortoiseProcPath = CPathUtils::GetAppDirectory(g_hmodThisDll) + _T("TortoiseGitProc.exe");
//	if (CCreateProcessHelper::CreateProcessDetached(tortoiseProcPath.c_str(), const_cast<TCHAR*>(command.c_str())))
//	{
//		// process started - exit
//		return;
//	}

//	MessageBox(NULL, CFormatMessageWrapper(), _T("TortoiseGitProc launch failed"), MB_OK | MB_ICONINFORMATION);
}

int CSIPropertyPage::LogThread()
{

	return 0;
}

void CSIPropertyPage::LogThreadEntry(void *param)
{
	((CSIPropertyPage*)param)->LogThread();
}

void CSIPropertyPage::InitWorkfileView()
{
	LVCOLUMN lvcol;
	LVITEM lvitem;
	HWND list;

	if (filenames.empty())
		return;

	list = GetDlgItem(m_hwnd, IDC_LIST2);

	SecureZeroMemory(&lvcol, sizeof(LVCOLUMN));

	// Define the columns appearing in the list view
	lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;                                                           
	lvcol.cx = 100;

	// Insert columns
	lvcol.pszText = _T("Property");
	SendMessage(list, LVM_INSERTCOLUMN, 0, (LPARAM)&lvcol);
	lvcol.cx = 200;
	lvcol.pszText = _T("Value");                          
	SendMessage(list, LVM_INSERTCOLUMN, 1, (LPARAM)&lvcol); 

	// Insert entries into properties table
	SecureZeroMemory(&lvitem, sizeof(LVITEM));

	lvitem.mask = LVIF_TEXT;   // Text Style
	lvitem.cchTextMax = 256; // Max size of test

	// 
	for (std::vector<stdstring>::iterator I = filenames.begin(); I != filenames.end(); ++I)
	{
		MemberProperties memProp;
		std::unique_ptr<IntegrityResponse> response;

		response = IntegrityActions::getMemberInfo(getIntegritySession(), I->c_str());
		for (mksWorkItem workItem : *response) {

			// revision = getStringFieldValue(item, L"memberrevision");
			mksItem revItem = getItemFieldValue(workItem, L"memberrev");
			memProp.memberRev = getId(revItem);

			// Retrieve working item rev
			mksItem workRevItem = getItemFieldValue(workItem, L"workingrev");
			memProp.workRev = getId(workRevItem);

			// Retrieve lock items
			mksItemList lockItemList = getItemListFieldValue(workItem, L"lockrecord");

			// 	
			for (mksItem lockItem = mksItemListGetFirst(lockItemList); lockItem != NULL; lockItem = mksItemListGetNext(lockItemList)) {

				LockProperties lockProp;
				mksItem locker;
				mksItem lockcpid;
			
				lockcpid = getItemFieldValue(lockItem, L"lockcpid");
				lockProp.lockcpid = getId(lockcpid);

				locker = getItemFieldValue(lockItem, L"locker");
				lockProp.locker = getId(locker);

				lockProp.locktype = getStringFieldValue(lockItem, L"locktype");
				
				// Push the locker properties onto the vector
				memProp.lockers.push_back(lockProp);
			}

			memProp.sandboxName = IntegrityActions::getSandboxName(getIntegritySession(), I->c_str());
		}

		lvitem.iItem = 0;          // choose item  
		lvitem.iSubItem = 0;       // Put in first coluom
		lvitem.pszText = _T("Member Revision"); // Text to display (can be from a char variable) (Items)
		SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
		lvitem.iSubItem = 1;
		lvitem.pszText = memProp.memberRev.empty() ? _T("") : (LPWSTR)memProp.memberRev.c_str();
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems

		lvitem.iItem = 1;          // choose item  
		lvitem.iSubItem = 0;       // Put in first coluom
		lvitem.pszText = _T("Working Revision"); // Text to display (can be from a char variable) (Items)
		SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
		lvitem.iSubItem = 1;
		lvitem.pszText = memProp.workRev.empty() ? _T("") : (LPWSTR) memProp.workRev.c_str();
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems

		lvitem.iItem = 2;          // choose item  
		lvitem.iSubItem = 0;       // Put in first coluom
		lvitem.pszText = _T("Working CP ID"); // Text to display (can be from a char variable) (Items)
		SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
		lvitem.iSubItem = 1;
		lvitem.pszText = memProp.cpid.empty() ? _T("") : (LPWSTR) memProp.cpid.c_str();
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems

		lvitem.iItem = 3;          // choose item  
		lvitem.iSubItem = 0;       // Put in first coluom
		lvitem.pszText = _T("Sandbox Name"); // Text to display (can be from a char variable) (Items)
		SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
		lvitem.iSubItem = 1;
		lvitem.pszText = memProp.sandboxName.empty() ? _T("") : (LPWSTR) memProp.sandboxName.c_str();
		SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems

		FileStatusFlags flags = getPathStatus(I->c_str());

		if (flags != 0) {

			if (hasFileStatus(flags, FileStatus::Locked)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Locked");
			}

			if (hasFileStatus(flags, FileStatus::Incoming)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Incoming");
			}

			if (hasFileStatus(flags, FileStatus::FormerMember)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("FormerMember");
			}

			if (hasFileStatus(flags, FileStatus::Modified)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Modified");
			}

			if (hasFileStatus(flags, FileStatus::Moved)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Moved");
			}

			if (hasFileStatus(flags, FileStatus::Renamed)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Renamed");
			}

			if (hasFileStatus(flags, FileStatus::MergeNeeded)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("MergeNeeded");
			}

			if (hasFileStatus(flags, FileStatus::Drop)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Drop");
			}

			if (hasFileStatus(flags, FileStatus::Add)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Add");
			}
			
			if (hasFileStatus(flags, FileStatus::Member)) {
				if (!memProp.status.empty()) {
					memProp.status += _T(" | ");
				}
				memProp.status += _T("Member");
			}
		}

		if (!memProp.status.empty()) {
			lvitem.iItem = 4;          // choose item  
			lvitem.iSubItem = 0;       // Put in first coluom
			lvitem.pszText = _T("Status"); // Text to display (can be from a char variable) (Items)
			SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
			lvitem.iSubItem = 1;
			lvitem.pszText = memProp.status.empty() ? _T("") : (LPWSTR)memProp.status.c_str();
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
		}

		for (std::vector<LockProperties>::iterator it = memProp.lockers.begin(); it != memProp.lockers.end(); ++it) {

			lvitem.iItem = 5;          // choose item  
			lvitem.iSubItem = 0;       // Put in first coluom
			lvitem.pszText = _T("Locker"); // Text to display (can be from a char variable) (Items)
			SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
			lvitem.iSubItem = 1;
			lvitem.pszText = it->locker.empty() ? _T("") : (LPWSTR)it->locker.c_str();
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems

			lvitem.iItem = 6;          // choose item  
			lvitem.iSubItem = 0;       // Put in first coluom
			lvitem.pszText = _T("Lock cpid"); // Text to display (can be from a char variable) (Items)
			SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
			lvitem.iSubItem = 1;
			lvitem.pszText = it->lockcpid.empty() ? _T("") : (LPWSTR)it->lockcpid.c_str();
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems

			lvitem.iItem = 7;          // choose item  
			lvitem.iSubItem = 0;       // Put in first coluom
			lvitem.pszText = _T("Lock type"); // Text to display (can be from a char variable) (Items)
			SendMessage(list, LVM_INSERTITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
			lvitem.iSubItem = 1;
			lvitem.pszText = it->locktype.empty() ? _T("") : (LPWSTR)it->locktype.c_str();
			SendMessage(list, LVM_SETITEM, 0, (LPARAM)&lvitem); // Enter text to SubItems
		}
	}

}

FileStatusFlags	CSIPropertyPage::getPathStatus(std::wstring path)
{
	if (IsPathAllowed(path)){
		if (IStatusCache::getInstance().getRootFolderCache().isPathControlled(path)) {
			if (PathIsDirectoryW(path.c_str())) {
				return FileStatus::Folder | FileStatus::Member;
			}
			else {
				FileStatusFlags status = IStatusCache::getInstance().getFileStatus(path);
				if (status == 0) {
					return FileStatus::Ignored | FileStatus::File;
				}
				return status | FileStatus::File;
			}
		}
		else {
			if (PathIsDirectoryW(path.c_str())) {
				return (FileStatusFlags)FileStatus::Folder;
			}
			else {
				return (FileStatusFlags)FileStatus::File;
			}
		}
	}
	return (FileStatusFlags)FileStatus::None;
}


// CShellExt member functions (needed for IShellPropSheetExt)
STDMETHODIMP CShellExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
	__try
	{
		return AddPages_Wrap(lpfnAddPage, lParam);
	}
	__except (HandleException(GetExceptionInformation()))
	{
	}
	return E_FAIL;
}

STDMETHODIMP CShellExt::AddPages_Wrap(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
	CString ProjectTopDir;

	if ((selectedItems.empty() && currentExplorerWindowFolder.empty()) || selectedItems.size() > 1)
		return S_OK;

	if (IsIllegalFolder(currentExplorerWindowFolder))
		return S_OK;

	if (currentExplorerWindowFolder.empty())
	{
		// folder is empty, but maybe files are selected
		if (selectedItems.empty())
			return S_OK;	// nothing selected - we don't have a menu to show
		// check whether a selected entry is an UID - those are namespace extensions
		// which we can't handle
		for (const std::wstring& path : selectedItems)
		{
			if (_tcsncmp(path.c_str(), _T("::{"), 3) == 0)
				return S_OK;
		}
	}
	else
	{
		if (_tcsncmp(currentExplorerWindowFolder.c_str(), _T("::{"), 3) == 0)
			return S_OK;
	}

	// Check if selections are under version control
	for (std::vector<stdstring>::iterator I = selectedItems.begin(); I != selectedItems.end(); ++I)
	{
		FileStatusFlags flags = getPathStatus(I->c_str());

		if (!hasFileStatus(flags, FileStatus::Member)) {
			return S_OK;
		}
	}

	LoadLangDll();
	PROPSHEETPAGE psp;
	SecureZeroMemory(&psp, sizeof(PROPSHEETPAGE));
	HPROPSHEETPAGE hPage;
	CSIPropertyPage *sheetpage = new (std::nothrow) CSIPropertyPage(selectedItems);

	psp.dwSize = sizeof (psp);
	psp.dwFlags = PSP_USEREFPARENT | PSP_USETITLE | PSP_USEICONID | PSP_USECALLBACK;
	psp.hInstance = g_hResInst;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE);
	psp.pszIcon = MAKEINTRESOURCE(IDI_APP);
	psp.pszTitle = _T("SI");
	psp.pfnDlgProc = (DLGPROC) PageProc;
	psp.lParam = (LPARAM) sheetpage;
	psp.pfnCallback = PropPageCallbackProc;
	psp.pcRefParent = (UINT*)&g_cRefThisDll;

	hPage = CreatePropertySheetPage (&psp);

	if (hPage != NULL)
	{
		if (!lpfnAddPage (hPage, lParam))
		{
			delete sheetpage;
			DestroyPropertySheetPage (hPage);
		}
	}

	return S_OK;
}

STDMETHODIMP CShellExt::ReplacePage (UINT /*uPageID*/, LPFNADDPROPSHEETPAGE /*lpfnReplaceWith*/, LPARAM /*lParam*/)
{
	return E_FAIL;
}
