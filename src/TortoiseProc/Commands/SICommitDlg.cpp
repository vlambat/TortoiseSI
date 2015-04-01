// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2003-2013 - TortoiseSVN
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
#include "TortoiseSIProc.h"
#include "ServerConnections.h"
#include "IntegrityActions.h"
#include "SICommitDlg.h"
#include "EventLog.h"
#include "MessageBox.h"
#include "AppUtils.h"
#include "Git.h"
#include "registry.h"
#include "UnicodeUtils.h"
#include "COMError.h"
#include "Globals.h"
#include "BstrSafeVector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CSICommitDlg, CResizableStandAloneDialog)
CSICommitDlg::CSICommitDlg(CWnd* pParent /*=NULL*/)	: 
    CResizableStandAloneDialog(CSICommitDlg::IDD, pParent),
	m_hAccelerator(nullptr)
{
}


CSICommitDlg::~CSICommitDlg()
{
}


void CSICommitDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableStandAloneDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SUBMIT_CP_COMBOBOX, m_ctrlChangePackageComboBox);
}


BEGIN_MESSAGE_MAP(CSICommitDlg, CResizableStandAloneDialog)
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_MOVE()
	ON_WM_MOVING()
	ON_CBN_SELCHANGE(IDC_SUBMIT_CP_COMBOBOX, &CSICommitDlg::OnCbnSelchangeSubmitCpCombobox)
	ON_BN_CLICKED(IDC_CREATE_CP_BUTTON, &CSICommitDlg::OnBnClickedCreateCpButton)
	ON_BN_CLICKED(IDC_SUBMIT_CP_BUTTON, &CSICommitDlg::OnBnClickedSubmitCpButton)
	ON_BN_CLICKED(IDCANCEL, &CSICommitDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


BOOL CSICommitDlg::OnInitDialog()
{
	// Call the super class OnInitDialog()
	CResizableStandAloneDialog::OnInitDialog();

	// Make the dialog unpinnable
	CAppUtils::MarkWindowAsUnpinnable(m_hWnd);

	// Load keyboard accelerator resource
	m_hAccelerator = LoadAccelerators(AfxGetResourceHandle(),MAKEINTRESOURCE(IDR_ACC_SICOMMITDLG));

	// Initialize control data in dialog box
	UpdateData(FALSE);

	m_tooltips.Create(this);
	//TODO: Add resource strings for tool tips
	//TODO: Register tool tips against controls
	//m_tooltips.AddTool(IDC_EXTERNALWARNING, IDS_COMMITDLG_EXTERNALS);

	//TODO: Set any dynamic control text, e.g. dynamic labels
	//SetDlgItemText(ID_RESOURCEID, _T(""));

	m_ctrlChangePackageComboBox.SetFocus();
	
	GetWindowText(m_sWindowTitle);

	// TODO: Adjust the size of radio buttoms and check boxes to fit translated text
	//AdjustControlSize(IDC_SHOWUNVERSIONED);

	// line up all controls and adjust their sizes.
    //#define LINKSPACING 9
	//RECT rc = AdjustControlSize(IDC_SELECTLABEL);
	//rc.right -= 15;	// AdjustControlSize() adds 20 pixels for the checkbox/radio button bitmap, but this is a label...
	//rc = AdjustStaticSize(IDC_CHECKALL, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKNONE, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKUNVERSIONED, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKVERSIONED, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKADDED, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKDELETED, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKMODIFIED, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKFILES, rc, LINKSPACING);
	//rc = AdjustStaticSize(IDC_CHECKSUBMODULES, rc, LINKSPACING);

	// Anchor controls using resizeable lib
	AddAnchor(IDC_SUBMIT_TO_CP_LABEL, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_SUBMIT_CP_COMBOBOX, TOP_RIGHT);
	AddAnchor(IDC_CREATE_CP_BUTTON, TOP_RIGHT);
	AddAnchor(IDC_SUBMIT_CP_BUTTON, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);

	// Set restrictions on the width and height of the dialog
	//GetClientRect(m_DlgOrigRect);
	//SetMinTrackSize( CSize( mDlgOriginRect.Width(), m_DlgOriginRect.Height())  );

	// Center dialog in Windows explorer (if its window handle was pass as parameter)
	if (theApp.m_hWndExplorer) {
		CenterWindow(CWnd::FromHandle(theApp.m_hWndExplorer));
	}

	if (!theApp.m_serverConnectionsCache->isOnline()) {
		EventLog::writeInformation(L"SICommitDlg create cp bailing out, unable to connect to integrity server");
		return FALSE;
	}

	UpdateChangePackageList();

	// If there are no active change packages then disable submit cp button 
	if( m_ctrlChangePackageComboBox.GetCount() == 0 ) {
		GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(TRUE);
		m_ctrlChangePackageComboBox.SetCurSel(m_ctrlChangePackageComboBox.GetCount() - 1);
	}

	// Loads window rect that was saved allowing user to resize and persist
	EnableSaveRestore(_T("SICommitDlg"));

	// return TRUE unless you set the focus to a control
	return FALSE;  
}


BOOL CSICommitDlg::PreTranslateMessage(MSG* pMsg)
{
	return CResizableStandAloneDialog::PreTranslateMessage(pMsg);
}


LRESULT CSICommitDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return __super::DefWindowProc(message, wParam, lParam);
}


void CSICommitDlg::DoSize(int /*delta*/)
{
	RemoveAnchor(IDC_SUBMIT_TO_CP_LABEL);
	RemoveAnchor(IDC_SUBMIT_CP_COMBOBOX);
	RemoveAnchor(IDC_CREATE_CP_BUTTON);
	RemoveAnchor(IDC_SUBMIT_CP_BUTTON);
	RemoveAnchor(IDCANCEL);
	ArrangeLayout();
}


// ON_WM_SIZE
// Called after the windows size has changed
void CSICommitDlg::OnSize(UINT nType, int cx, int cy)
{
    // first, let the resizing take place
    __super::OnSize(nType, cx, cy);
}


// ON_WM_SIZING
// Called while the window size is changing
void CSICommitDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	__super::OnSizing(fwSide, pRect);
}


// ON_WM_MOVE
// Called after the window has been moved
void CSICommitDlg::OnMove(int x, int y)
{
    // first, let the move take place
	__super::OnMove(x,y);

}


// ON_WM_MOVING
// Called while window is moving
void CSICommitDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	__super::OnMoving(fwSide, pRect);
}


void CSICommitDlg::OnBnClickedCreateCpButton()
{
	UpdateData();

	// Hide the tool tips
	m_tooltips.Pop();

	if (!theApp.m_serverConnectionsCache->isOnline()) {
		EventLog::writeInformation(L"SICommitDlg create cp bailing out, unable to connect to integrity server");
		return;
	}

	std::wstring cpid;

	// Launch the create cp view
	if (IntegrityActions::launchCreateCPView(*(theApp.m_integritySession), cpid)) {

		ClearChangePackageList();

		// Update CP combobox to include newly created CP
		UpdateChangePackageList();

		// Enable the submit CP button if there are no CPs, disable otherwise
		if (m_ctrlChangePackageComboBox.GetCount() == 0) {
			GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(TRUE);

			// Select the newly created CP in combobox
			int idx = m_ctrlChangePackageComboBox.FindString(-1, cpid.c_str());

			if (idx != CB_ERR) {
				m_ctrlChangePackageComboBox.SetCurSel(idx);
			}
		}
	}
}


void CSICommitDlg::OnBnClickedSubmitCpButton()
{
	// Retrieve data for dialog controls
	UpdateData();

	// Hide the tool tips
	m_tooltips.Pop();

	// Check client connection and connect if not connected
	if (!theApp.m_serverConnectionsCache->isOnline()) {
		EventLog::writeInformation(L"SICommitDlg submit cp bailing out, unable to connect to server");
		return;
	}

	// Get selected CP data
	int idx = m_ctrlChangePackageComboBox.GetCurSel();
	std::shared_ptr<IntegrityActions::ChangePackage> *cp = (std::shared_ptr<IntegrityActions::ChangePackage> *) m_ctrlChangePackageComboBox.GetItemDataPtr(idx);
	
	// Submit CP
	if(IntegrityActions::submitCP(*(theApp.m_integritySession), (*cp)->getId())) {

		ClearChangePackageList();

		// Update CP combobox to remove submitted CP (if it was closed)
		UpdateChangePackageList();
		
		// Disable the submit CP button if there are no CPs, disable otherwise
		if (m_ctrlChangePackageComboBox.GetCount() == 0) {
			GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(TRUE);
			m_ctrlChangePackageComboBox.SetCurSel( m_ctrlChangePackageComboBox.GetCount() - 1);
		}
	}
}


void CSICommitDlg::OnBnClickedCancel()
{
	UpdateData();

	// Hide the tool tips
	m_tooltips.Pop();

	ClearChangePackageList();

	__super::OnCancel();
}


void CSICommitDlg::OnCbnSelchangeSubmitCpCombobox()
{
	// TODO: Add your control notification handler code here

	// Retrieve data for dialog control
	UpdateData();

	// Hide the tooltips
	m_tooltips.Pop();

}


void CSICommitDlg::ClearChangePackageList()
{
	// Delete all combobox strings and corresponding cp item data from combo box
	for (int idx = m_ctrlChangePackageComboBox.GetCount() - 1; idx >= 0; idx--) {
		std::shared_ptr<IntegrityActions::ChangePackage> *cp =
			(std::shared_ptr<IntegrityActions::ChangePackage> *) m_ctrlChangePackageComboBox.GetItemDataPtr(idx);
		m_ctrlChangePackageComboBox.SetItemDataPtr(idx, NULL);
		m_ctrlChangePackageComboBox.DeleteString(idx);
		delete cp;
	}

	m_ctrlChangePackageComboBox.ResetContent();
}


void CSICommitDlg::UpdateChangePackageList()
{
	// Add active change package ids and summary to combobox.
	// Store ChangePackage information as item data for later reference
	for (std::shared_ptr<IntegrityActions::ChangePackage> *cp : IntegrityActions::getChangePackageList(*(theApp.m_integritySession))) {
		std::wstring cpText = (*cp)->getId() + L" " + (*cp)->getSummary();
		int idx = m_ctrlChangePackageComboBox.AddString(cpText.c_str());
		m_ctrlChangePackageComboBox.SetItemDataPtr(idx, cp);
	}
}

