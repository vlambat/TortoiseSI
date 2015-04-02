// TortoiseSI - a Windows shell extension for easy version control

// Copyright (C) 2003-2008, 2014 - TortoiseSVN
// Copyright (C) 2008-2014 - TortoiseGit
// Copyright (C) 2015 - TortoiseSI

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

#define MAX_BUFFER_LENGTH   1024

// Define reference to list view
HWND m_list;

// Nonmember function prototypes
BOOL CALLBACK PageProc (HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK PropPageCallbackProc ( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
UINT CALLBACK ListViewCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

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
	// Delete the page before closing
	if (PSPCB_RELEASE == uMsg)
	{
		CSIPropertyPage* sheetpage = (CSIPropertyPage*) ppsp->lParam;
		if (sheetpage != NULL)
			delete sheetpage;
	}
	return 1;
}

// Function to compare strings used for sort operation 
UINT CALLBACK ListViewCompare(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	bool isAsc = (lParamSort > 0);
	int column = abs(lParamSort) - 1;
	
	UINT result = 0;

	WCHAR buf1[MAX_BUFFER_LENGTH];
	WCHAR buf2[MAX_BUFFER_LENGTH];

	// Retrieve text from items
	ListView_GetItemText(m_list, lParam1, column, buf1, sizeof(buf1) / sizeof(WCHAR));
	ListView_GetItemText(m_list, lParam2, column, buf2, sizeof(buf2) / sizeof(WCHAR));

	wcslen(buf1);
	// Perform string compare depending on sort type
	if (isAsc) 
	{
		result = CompareStringEx(LOCALE_NAME_SYSTEM_DEFAULT,
			0,
			buf1,
			wcslen(buf1),
			buf2,
			wcslen(buf2),
			NULL, NULL, 0);
	}
	else 
	{	
		result = CompareStringEx(LOCALE_NAME_SYSTEM_DEFAULT,
			0,
			buf2,
			wcslen(buf2),
			buf1,
			wcslen(buf1),
			NULL, NULL, 0);
	}

	// Make return result consistent with C runtime
	return result - 2;
}


CSIPropertyPage::CSIPropertyPage(const std::vector<stdstring> &newFilenames, FileStatusFlags flags)
	:m_filenames(newFilenames)
	,m_hwnd(NULL)
	,m_flags(flags)
{
}

CSIPropertyPage::~CSIPropertyPage(void)
{
}

void CSIPropertyPage::SetHwnd(HWND newHwnd)
{
	m_hwnd = newHwnd;
}

// Set the sort icons on the column header
// Sort order can be defined by: 0 (neither), 1 (ascending), 2 (descending)
void CSIPropertyPage::SetSortIcon(HWND listView, int col, int sortOrder)
{
	int numColumns = 0;
	int curCol = 0;
	wchar_t headerText[MAX_BUFFER_LENGTH];
	HWND headerRef;
	HDITEM itemHdr;

	// Retrieve reference to header control
	headerRef = ListView_GetHeader(listView);

	// Retrieve number of list view columns
	numColumns = Header_GetItemCount(headerRef);

	for (curCol = 0; curCol<numColumns; curCol++)
	{
		itemHdr.mask = HDI_FORMAT | HDI_TEXT;
		itemHdr.pszText = headerText;
		itemHdr.cchTextMax = MAX_BUFFER_LENGTH - 1;

		// Get the column
		SendMessage(headerRef, HDM_GETITEM, curCol, (LPARAM)&itemHdr);

		// Check the sort order and set the sort icon
		if ((sortOrder != 0) && (curCol == col))
		{
			switch (sortOrder)
			{
			case 1:
				itemHdr.fmt &= !HDF_SORTUP;
				itemHdr.fmt |= HDF_SORTDOWN;
				break;
			case 2:
				itemHdr.fmt &= !HDF_SORTDOWN;
				itemHdr.fmt |= HDF_SORTUP;
				break;
			}
		}
		else
		{
			itemHdr.fmt &= !HDF_SORTUP & !HDF_SORTDOWN;
		}
		itemHdr.fmt |= HDF_STRING;
		itemHdr.mask = HDI_FORMAT | HDI_TEXT;

		SendMessage(headerRef, HDM_SETITEM, curCol, (LPARAM)&itemHdr);
	}
}

// Used to call the sort operation
void CSIPropertyPage::OnColumnClick(LPNMLISTVIEW pLVInfo)
{
	static int sortColumn = 0;
	static BOOL sortAscend = TRUE;

	LPARAM lParamSort;

	if (pLVInfo->iSubItem == sortColumn)
	{
		sortAscend = !sortAscend;
	}
	else
	{
		sortColumn = pLVInfo->iSubItem;
		sortAscend = TRUE;
	}

	// Combine values to pass to sort function
	lParamSort = 1 + sortColumn;
	if (!sortAscend)
	{
		lParamSort = -lParamSort;
	}

	// Sort list view
	ListView_SortItemsEx(pLVInfo->hdr.hwndFrom, ListViewCompare, lParamSort);

	// Toggle sort icon
	SetSortIcon(pLVInfo->hdr.hwndFrom, sortColumn, sortAscend + 1);
}

BOOL CSIPropertyPage::PageProc(HWND /*hwnd*/, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	// Catch events and handle
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

		// Respond to tooltip notification
		if (code == LVN_GETINFOTIP) {

			LPNMLVGETINFOTIP pGetInfoTip = (LPNMLVGETINFOTIP)lParam;

			if (pGetInfoTip != NULL)
			{
				std::wstring buffer;

				// Ensure buffer is the correct size
				buffer.resize(pGetInfoTip->cchTextMax);

				// Retrieve the property value that will appear in the tool tip
				ListView_GetItemText(m_list, pGetInfoTip->iItem, 1, &buffer[0], buffer.size());

				// Copy the value into the buffer
				wcsncpy(pGetInfoTip->pszText, buffer.c_str(), buffer.length());
			}
		}
		else if (code == LVN_COLUMNCLICK) 
		{
			// Respond to column click for sorting
			OnColumnClick((LPNMLISTVIEW)lParam);
		}

		return TRUE;
	}
	case WM_DESTROY:
		return TRUE;
	}

	return FALSE;
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
	int iItemIndex = 0;
	int iGroupId = 100;

	LVCOLUMN lvCol;
	LVGROUP lvGroup;
	LVITEM lvitem;
	
	// Return if no files found or multiple
	if (m_filenames.empty() || m_filenames.size() != 1)
		return;

	// Get a reference to the list view
	m_list = GetDlgItem(m_hwnd, IDC_LIST2);

	// Macro enables group view
	ListView_EnableGroupView(m_list, TRUE);

	// Macro enables tool tips
	ListView_SetExtendedListViewStyle(m_list, LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT);

	// Initialize columns appearing in list view
	SecureZeroMemory(&lvCol, sizeof(LVCOLUMN));

	std::wstring columnProp = getTortoiseSIString(IDS_PROP_NAME_COLUMN);
	std::wstring valueProp = getTortoiseSIString(IDS_PROP_VALUE_COLUMN);

	// Define the columns appearing in the list view
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvCol.cx = 100;
	lvCol.pszText = (LPWSTR)columnProp.c_str();
	SendMessage(m_list, LVM_INSERTCOLUMN, 0, (LPARAM)&lvCol);
	lvCol.cx = 225;
	lvCol.pszText = (LPWSTR)valueProp.c_str();
	SendMessage(m_list, LVM_INSERTCOLUMN, 1, (LPARAM)&lvCol);

	// Initialize entries in the list view
	SecureZeroMemory(&lvitem, sizeof(LVITEM));

	// Specify member functions that contain data
	lvitem.mask = LVIF_TEXT | LVIF_GROUPID | LVIF_COLUMNS;

	// Retrieve group name
	std::wstring lockGrpName = getTortoiseSIString(IDS_PROP_LOCK_GROUP_NAME);

	// Retrieve property names
	std::wstring lockPropName = getTortoiseSIString(IDS_PROP_LOCK_NAME);
	std::wstring lockCpIdPropName = getTortoiseSIString(IDS_PROP_LOCK_CPID);
	std::wstring lockTypePropName = getTortoiseSIString(IDS_PROP_LOCK_TYPE);

	// Retrieve highlighted file name
	stdstring fileName = m_filenames.front();

	// Retrieve file properties from Integrity
	std::shared_ptr<IntegrityActions::MemberProperties> memProp =
		IntegrityActions::getMemberInfo(IStatusCache::getInstance().getIntegritySession(), fileName.c_str());

	// Extract required properties
	std::wstring memRev = memProp->getMemberRev();
	std::wstring workingRev = memProp->getWorkingRev();
	std::wstring sandboxName = memProp->getSandboxName();
	std::wstring cpId = memProp->getChangePackageId();

	// Retrieve the file locks and related info 
	for (std::shared_ptr<IntegrityActions::LockProperties> lock : memProp->getLockers()) {

		LVGROUP lvLockGroup;

		// Increment the group id
		iGroupId++;

		// Extract lock info
		std::wstring lockName = lock->getLockName();
		std::wstring lockType = lock->getLockType();
		std::wstring lockCpId = lock->getLockChangePackageId();

		SecureZeroMemory(&lvLockGroup, sizeof(LVGROUP));

		// Insert the lock group - one for each lock
		lvLockGroup.cbSize = sizeof(LVGROUP);
		lvLockGroup.mask = LVGF_HEADER | LVGF_GROUPID;
		lvLockGroup.pszHeader = (LPWSTR)lockGrpName.c_str();
		lvLockGroup.iGroupId = iGroupId;
		SendMessage(m_list, LVM_INSERTGROUP, 0, (LPARAM)&lvLockGroup);

		if (!lockName.empty()) {

			lvitem.iItem = iItemIndex;
			lvitem.iGroupId = iGroupId;
			lvitem.iSubItem = 0;
			lvitem.pszText = (LPWSTR)lockPropName.c_str();
			SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
			lvitem.iSubItem = 1;
			lvitem.pszText = (LPWSTR)lockName.c_str();
			SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
		}

		if (!lockCpId.empty()) {

			lvitem.iItem = iItemIndex;
			lvitem.iGroupId = iGroupId;
			lvitem.iSubItem = 0;
			lvitem.pszText = (LPWSTR)lockCpIdPropName.c_str();
			SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
			lvitem.iSubItem = 1;
			lvitem.pszText = (LPWSTR)lockCpId.c_str();
			SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
		}

		if (!lockType.empty()) {

			lvitem.iItem = iItemIndex;
			lvitem.iGroupId = iGroupId;
			lvitem.iSubItem = 0;
			lvitem.pszText = (LPWSTR)lockTypePropName.c_str();
			SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
			lvitem.iSubItem = 1;
			lvitem.pszText = (LPWSTR)lockType.c_str();
			SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
		}
	}

	// Retrieve group file name
	std::wstring fileGrpName = getTortoiseSIString(IDS_PROP_FILE_GROUP_NAME);

	SecureZeroMemory(&lvGroup, sizeof(LVGROUP));

	// Insert group containing file properties
	lvGroup.cbSize = sizeof(LVGROUP);
	lvGroup.mask = LVGF_HEADER | LVGF_GROUPID;
	lvGroup.pszHeader = (LPWSTR)fileGrpName.c_str();
	lvGroup.iGroupId = iGroupId;
	SendMessage(m_list, LVM_INSERTGROUP, 0, (LPARAM)&lvGroup);

	if (!memRev.empty()) {

		std::wstring memberRevPropName = getTortoiseSIString(IDS_PROP_MEMBER_REV);

		lvitem.iItem = iItemIndex;
		lvitem.iGroupId = iGroupId;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPWSTR)memberRevPropName.c_str();
		SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
		lvitem.iSubItem = 1;
		lvitem.pszText = (LPWSTR)memRev.c_str();
		SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
	}

	if (!workingRev.empty()) {

		std::wstring workRevPropName = getTortoiseSIString(IDS_PROP_WORKING_REV);

		lvitem.iItem = iItemIndex;
		lvitem.iGroupId = iGroupId;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPWSTR)workRevPropName.c_str();
		SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
		lvitem.iSubItem = 1;
		lvitem.pszText = (LPWSTR)workingRev.c_str();
		SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
	}

	if (!cpId.empty()) {

		std::wstring cpIdPropName = getTortoiseSIString(IDS_PROP_WORKING_CPID);

		lvitem.iItem = iItemIndex;
		lvitem.iGroupId = iGroupId;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPWSTR)cpIdPropName.c_str();
		SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
		lvitem.iSubItem = 1;
		lvitem.pszText = (LPWSTR)cpId.c_str();
		SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
	}

	if (!sandboxName.empty()) {

		std::wstring sandboxNamePropName = getTortoiseSIString(IDS_PROP_SANDBOX_NAME);

		lvitem.iItem = iItemIndex;
		lvitem.iGroupId = iGroupId;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPWSTR)sandboxNamePropName.c_str();
		SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
		lvitem.iSubItem = 1;
		lvitem.pszText = (LPWSTR)sandboxName.c_str();
		SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
	}

	// Holds file status
	std::wstring status;

	if (m_flags != 0) {

		if (hasFileStatus(m_flags, FileStatus::Member)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Member");
		}

		if (hasFileStatus(m_flags, FileStatus::Locked)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Locked");
		}

		if (hasFileStatus(m_flags, FileStatus::Incoming)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Incoming");
		}

		if (hasFileStatus(m_flags, FileStatus::FormerMember)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("FormerMember");
		}

		if (hasFileStatus(m_flags, FileStatus::Modified)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Modified");
		}

		if (hasFileStatus(m_flags, FileStatus::Moved)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Moved");
		}

		if (hasFileStatus(m_flags, FileStatus::Renamed)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Renamed");
		}

		if (hasFileStatus(m_flags, FileStatus::MergeNeeded)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("MergeNeeded");
		}

		if (hasFileStatus(m_flags, FileStatus::Drop)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Drop");
		}

		if (hasFileStatus(m_flags, FileStatus::Add)) {
			if (!status.empty()) {
				status += _T(" | ");
			}
			status += _T("Add");
		}

	}

	if (!status.empty()) {

		std::wstring statusPropName = getTortoiseSIString(IDS_PROP_STATUS);

		lvitem.iItem = iItemIndex;
		lvitem.iGroupId = iGroupId;
		lvitem.iSubItem = 0;
		lvitem.pszText = (LPWSTR)statusPropName.c_str();
		SendMessage(m_list, LVM_INSERTITEM, 0, (LPARAM)&lvitem);
		lvitem.iSubItem = 1;
		lvitem.pszText = (LPWSTR)status.c_str();
		SendMessage(m_list, LVM_SETITEMTEXT, iItemIndex++, (LPARAM)&lvitem);
	}
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
	FileStatusFlags flags;

	if ((selectedItems.empty() && currentExplorerWindowFolder.empty()) || selectedItems.size() > 1)
		return S_OK;

	if (IsIllegalFolder(currentExplorerWindowFolder))
		return S_OK;

	// folder is empty, but maybe files are selected
	if (currentExplorerWindowFolder.empty())
	{
		// nothing selected - we don't have a menu to show
		if (selectedItems.empty())
			return S_OK;	

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

	// Only display properties tab for member files
	for (std::vector<stdstring>::iterator I = selectedItems.begin(); I != selectedItems.end(); ++I)
	{
		flags = getPathStatus(I->c_str());

		if (!hasFileStatus(flags, FileStatus::Member)) {
			return S_OK;
		}
	}

	LoadLangDll();
	PROPSHEETPAGE psp;
	SecureZeroMemory(&psp, sizeof(PROPSHEETPAGE));
	HPROPSHEETPAGE hPage;
	CSIPropertyPage *sheetpage = new (std::nothrow) CSIPropertyPage(selectedItems, flags);

	// Name the tab the same as the context menu
	std::wstring propTabTitle = getTortoiseSIString(IDS_MENU);

	// Initialize the property page
	psp.dwSize = sizeof (psp);
	psp.dwFlags = PSP_USEREFPARENT | PSP_USETITLE | PSP_USEICONID | PSP_USECALLBACK;
	psp.hInstance = g_hResInst;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE);
	psp.pszIcon = MAKEINTRESOURCE(IDI_APP);
	psp.pszTitle = (LPWSTR)propTabTitle.c_str();
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
