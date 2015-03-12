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

#undef min
#undef max

IMPLEMENT_DYNAMIC(CSICommitDlg, CResizableStandAloneDialog)
CSICommitDlg::CSICommitDlg(CWnd* pParent /*=NULL*/)	: 
    CResizableStandAloneDialog(CSICommitDlg::IDD, pParent),
	m_hAccelerator(nullptr)
{
}

CSICommitDlg::~CSICommitDlg()
{
}

int CSICommitDlg::getIntegrationPort()
{
	CRegStdDWORD integrationPointKey(L"Software\\TortoiseSI\\IntegrationPoint", -1);
	integrationPointKey.read();

	int port = (DWORD)integrationPointKey;
	return port;
}

void CSICommitDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableStandAloneDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SUBMIT_CP_COMBOBOX, m_ctrlChangePackageComboBox)
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
	//TODO: GORD: Add resource strings for tool tips
	//TODO: GORD: Register tool tips against controls
	//m_tooltips.AddTool(IDC_EXTERNALWARNING, IDS_COMMITDLG_EXTERNALS);

	//TODO: GORD: Set any dynamic control text, e.g. labels
	//SetDlgItemText(ID_RESOURCEID, _T(""));

	//TODO: GORD: Initially enable or diable any controls
	//GetDlgItem(IDC_RESOURCEID)->EnableWindow(TRUE);
	//GetDlgItem(IDC_RESOURCEID)->EnableWindow(FALSE);

	//TODO: GORD: Initially hide or show any controls
	//GetDlgItem(IDC_RESOURCEID)->ShowWindow(SW_SHOW);
	//GetDlgItem(IDC_RESORUCEID)->ShowWindow(SW_HIDE);

	//TODO: GORD: Set initial focus
	//GetDlgItem(IDC_RESOURCEID)->SetFocus();

	GetWindowText(m_sWindowTitle);

	// TODO: GORD: Adjust the size of radio buttoms and check boxes to fit translated text
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

	//TODO: GORD: Anchor controls using resizeable lib
	AddAnchor(IDC_SUBMIT_TO_CP_LABEL, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_SUBMIT_CP_COMBOBOX, TOP_RIGHT);
	AddAnchor(IDC_CREATE_CP_BUTTON, TOP_RIGHT);
	AddAnchor(IDC_SUBMIT_CP_BUTTON, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);

	// Set restrictions on the width and height of the dialog
	//GetClientRect(m_DlgOrigRect);
	//SetMinTrackSize( CSize( mDlgOriginRect.Width(), m_DlgOriginRect.Height())  );

	// Loads window rect that was saved allowing user to resize and persist
	EnableSaveRestore(_T("SICommitDlg"));

	// Center dialog in Windows explorer (if its window handle was pass as parameter)
	if (hWndExplorer)
		CenterWindow(CWnd::FromHandle(hWndExplorer));

	/***************************************************************************
	 * Establish Integrity server connection
	 ***************************************************************************/
	int port = getIntegrationPort();

	if (port > 0 && port <= std::numeric_limits<unsigned short>::max()) {
		EventLog::writeInformation(L"SICommitDLg connecting to Integrity client via localhost:" + std::to_wstring(port));

		m_integritySession = std::unique_ptr<IntegritySession>(new IntegritySession("localhost", port));

	}  else {
		EventLog::writeInformation(L"SICommitDlg connecting to Integrity client via localintegration point");

		m_integritySession = std::unique_ptr<IntegritySession>(new IntegritySession());
	}

	m_serverConnectionsCache = std::unique_ptr<ServerConnections>(new ServerConnections(*m_integritySession));

	//TODO: GORD: Determine the users active change packages and add them to the combobox
	// Should store cp number as additional data to comboxbox items

	// If there are no active change packages then disable submit cp button 
	if( m_ctrlChangePackageComboBox.GetCount() == 0 ) {
	    GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(FALSE);
	} else {
		GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(TRUE);
	}

	return FALSE;  // return TRUE unless you set the focus to a control
}

BOOL CSICommitDlg::PreTranslateMessage(MSG* pMsg)
{
	return CResizableStandAloneDialog::PreTranslateMessage(pMsg);
}

LRESULT CSICommitDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return __super::DefWindowProc(message, wParam, lParam);
}

void CSICommitDlg::DoSize(int delta)
{
	RemoveAnchor(IDC_SUBMIT_TO_CP_LABEL);
	RemoveAnchor(IDC_SUBMIT_CP_COMBOBOX);
	RemoveAnchor(IDC_CREATE_CP_BUTTON);
	RemoveAnchor(IDC_SUBMIT_CP_BUTTON);
	RemoveAnchor(IDCANCEL);

	//AddAnchor(IDC_MESSAGEGROUP, TOP_LEFT, TOP_RIGHT);
	//AddAnchor(IDC_LOGMESSAGE, TOP_LEFT, TOP_RIGHT);
	//AddAnchor(IDC_SPLITTER, TOP_LEFT, TOP_RIGHT);
	//AddAnchor(IDC_LISTGROUP, TOP_LEFT, BOTTOM_RIGHT);
	//AddAnchor(IDC_FILELIST, TOP_LEFT, BOTTOM_RIGHT);
	//AddAnchor(IDC_SIGNOFF,TOP_RIGHT);
	//AddAnchor(IDC_COMMIT_AMEND,TOP_LEFT);
	//AddAnchor(IDC_COMMIT_AMENDDIFF,TOP_LEFT);
	//AddAnchor(IDC_COMMIT_SETDATETIME,TOP_LEFT);
	//AddAnchor(IDC_COMMIT_DATEPICKER,TOP_LEFT);
	//AddAnchor(IDC_COMMIT_TIMEPICKER,TOP_LEFT);
	//AddAnchor(IDC_COMMIT_SETAUTHOR, TOP_LEFT);
	//AddAnchor(IDC_COMMIT_AUTHORDATA, TOP_LEFT, TOP_RIGHT);
	//AddAnchor(IDC_TEXT_INFO,TOP_RIGHT);
	//AddAnchor(IDC_SELECTLABEL, TOP_LEFT);
	//AddAnchor(IDC_CHECKALL, TOP_LEFT);
	//AddAnchor(IDC_CHECKNONE, TOP_LEFT);
	//AddAnchor(IDC_CHECKUNVERSIONED, TOP_LEFT);
	//AddAnchor(IDC_CHECKVERSIONED, TOP_LEFT);
	//AddAnchor(IDC_CHECKADDED, TOP_LEFT);
	//AddAnchor(IDC_CHECKDELETED, TOP_LEFT);
	//AddAnchor(IDC_CHECKMODIFIED, TOP_LEFT);
	//AddAnchor(IDC_CHECKFILES, TOP_LEFT);
	//AddAnchor(IDC_CHECKSUBMODULES, TOP_LEFT);
	//ArrangeLayout();
	//// adjust the minimum size of the dialog to prevent the resizing from
	//// moving the list control too far down.
	//CRect rcLogMsg;
	//m_cLogMessage.GetClientRect(rcLogMsg);
	//SetMinTrackSize(CSize(m_DlgOrigRect.Width(), m_DlgOrigRect.Height()-m_LogMsgOrigRect.Height()+rcLogMsg.Height()));

	//GetDlgItem(IDC_LOGMESSAGE)->Invalidate();
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
	// TODO: Add your control notification handler code here
	UpdateData();

	// Hide the tool tips
	m_tooltips.Pop();
}


void CSICommitDlg::OnBnClickedSubmitCpButton()
{
	// TODO: Add your control notification handler code here

	// Retrieve data for dialog controls
	UpdateData();

	// Hide the tool tips
	m_tooltips.Pop();

}


void CSICommitDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here

	// Hide the tool tips
	m_tooltips.Pop();

}


void CSICommitDlg::OnCbnSelchangeSubmitCpCombobox()
{
	// TODO: Add your control notification handler code here

	// Retrieve data for dialog control
	UpdateData();

	// Hide the tooltips
	m_tooltips.Pop();
}
