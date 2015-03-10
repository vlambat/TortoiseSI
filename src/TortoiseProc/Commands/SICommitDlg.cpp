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
#include "TortoiseProc.h"
#include "SICommitDlg.h"


#include "DirFileEnum.h"
#include "MessageBox.h"
#include "AppUtils.h"
#include "PathUtils.h"
#include "Git.h"
#include "registry.h"
#include "GitStatus.h"
#include "HistoryDlg.h"
#include "Hooks.h"
#include "UnicodeUtils.h"
#include "../TGitCache/CacheInterface.h"
#include "ProgressDlg.h"
#include "ShellUpdater.h"
#include "Commands/PushCommand.h"
#include "COMError.h"
#include "Globals.h"
#include "SysProgressDlg.h"
#include "MassiveGitTask.h"
#include "LogDlg.h"
#include "BstrSafeVector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT CSICommitDlg::WM_AUTOLISTREADY = RegisterWindowMessage(_T("TORTOISEGIT_AUTOLISTREADY_MSG"));
UINT CSICommitDlg::WM_UPDATEOKBUTTON = RegisterWindowMessage(_T("TORTOISEGIT_COMMIT_UPDATEOKBUTTON"));
UINT CSICommitDlg::WM_UPDATEDATAFALSE = RegisterWindowMessage(_T("TORTOISEGIT_COMMIT_UPDATEDATAFALSE"));

IMPLEMENT_DYNAMIC(CSICommitDlg, CResizableStandAloneDialog)
CSICommitDlg::CSICommitDlg(CWnd* pParent /*=NULL*/)
	: CResizableStandAloneDialog(CSICommitDlg::IDD, pParent)
	, m_bThreadRunning(FALSE)
	, m_bRunThread(FALSE)
	, m_hAccelerator(nullptr)
{
}

CSICommitDlg::~CSICommitDlg()
{
}

void CSICommitDlg::DoDataExchange(CDataExchange* pDX)
{
	CResizableStandAloneDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SUBMIT_CP_COMBOBOX, m_ctrlChangePackageComboBox)

	//DDX_Control(pDX, IDC_FILELIST, m_ListCtrl);
	//DDX_Control(pDX, IDC_LOGMESSAGE, m_cLogMessage);
	//DDX_Check(pDX, IDC_SHOWUNVERSIONED, m_bShowUnversioned);
	//DDX_Check(pDX, IDC_COMMIT_SETDATETIME, m_bSetCommitDateTime);
	//DDX_Check(pDX, IDC_CHECK_NEWBRANCH, m_bCreateNewBranch);
	//DDX_Text(pDX, IDC_NEWBRANCH, m_sCreateNewBranch);
	//DDX_Text(pDX, IDC_BUGID, m_sBugID);
	//DDX_Text(pDX, IDC_COMMIT_AUTHORDATA, m_sAuthor);
	//DDX_Check(pDX, IDC_WHOLE_PROJECT, m_bWholeProject);
	//DDX_Control(pDX, IDC_SPLITTER, m_wndSplitter);
	//DDX_Check(pDX, IDC_KEEPLISTS, m_bKeepChangeList);
	//DDX_Check(pDX, IDC_NOAUTOSELECTSUBMODULES, m_bDoNotAutoselectSubmodules);
	//DDX_Check(pDX,IDC_COMMIT_AMEND,m_bCommitAmend);
	//DDX_Check(pDX, IDC_COMMIT_MESSAGEONLY, m_bCommitMessageOnly);
	//DDX_Check(pDX,IDC_COMMIT_AMENDDIFF,m_bAmendDiffToLastCommit);
	//DDX_Check(pDX, IDC_COMMIT_SETAUTHOR, m_bSetAuthor);
	//DDX_Control(pDX,IDC_VIEW_PATCH,m_ctrlShowPatch);
	//DDX_Control(pDX, IDC_COMMIT_DATEPICKER, m_CommitDate);
	//DDX_Control(pDX, IDC_COMMIT_TIMEPICKER, m_CommitTime);
}

BEGIN_MESSAGE_MAP(CSICommitDlg, CResizableStandAloneDialog)
	//ON_BN_CLICKED(IDC_SHOWUNVERSIONED, OnBnClickedShowunversioned)
	//ON_NOTIFY(SCN_UPDATEUI, IDC_LOGMESSAGE, OnScnUpdateUI)
    //ON_BN_CLICKED(IDC_HISTORY, OnBnClickedHistory)
	//ON_BN_CLICKED(IDC_BUGTRAQBUTTON, OnBnClickedBugtraqbutton)
	//ON_EN_CHANGE(IDC_LOGMESSAGE, OnEnChangeLogmessage)
	//ON_REGISTERED_MESSAGE(CGitStatusListCtrl::GITSLNM_ITEMCOUNTCHANGED, OnGitStatusListCtrlItemCountChanged)
	//ON_REGISTERED_MESSAGE(CGitStatusListCtrl::GITSLNM_NEEDSREFRESH, OnGitStatusListCtrlNeedsRefresh)
	//ON_REGISTERED_MESSAGE(CGitStatusListCtrl::GITSLNM_ADDFILE, OnFileDropped)
	//ON_REGISTERED_MESSAGE(CGitStatusListCtrl::GITSLNM_CHECKCHANGED, &CCommitDlg::OnGitStatusListCtrlCheckChanged)
	//ON_REGISTERED_MESSAGE(CGitStatusListCtrl::GITSLNM_ITEMCHANGED, &CCommitDlg::OnGitStatusListCtrlItemChanged)
	//ON_REGISTERED_MESSAGE(CLinkControl::LK_LINKITEMCLICKED, &CCommitDlg::OnCheck)
	//ON_REGISTERED_MESSAGE(WM_AUTOLISTREADY, OnAutoListReady)
	//ON_REGISTERED_MESSAGE(WM_UPDATEOKBUTTON, OnUpdateOKButton)
	//ON_REGISTERED_MESSAGE(WM_UPDATEDATAFALSE, OnUpdateDataFalse)
	//ON_WM_TIMER()
	//ON_WM_SIZE()
	//ON_STN_CLICKED(IDC_EXTERNALWARNING, &CCommitDlg::OnStnClickedExternalwarning)
	//ON_BN_CLICKED(IDC_SIGNOFF, &CCommitDlg::OnBnClickedSignOff)
	//ON_BN_CLICKED(IDC_COMMIT_AMEND, &CCommitDlg::OnBnClickedCommitAmend)
	//ON_BN_CLICKED(IDC_COMMIT_MESSAGEONLY, &CCommitDlg::OnBnClickedCommitMessageOnly)
	//ON_BN_CLICKED(IDC_WHOLE_PROJECT, &CCommitDlg::OnBnClickedWholeProject)
	//ON_COMMAND(ID_FOCUS_MESSAGE,&CCommitDlg::OnFocusMessage)
	//ON_COMMAND(ID_FOCUS_FILELIST, OnFocusFileList)
	//ON_STN_CLICKED(IDC_VIEW_PATCH, &CCommitDlg::OnStnClickedViewPatch)
	//ON_WM_MOVE()
	//ON_WM_MOVING()
	//ON_WM_SIZING()
	//ON_NOTIFY(HDN_ITEMCHANGED, 0, &CCommitDlg::OnHdnItemchangedFilelist)
	//ON_BN_CLICKED(IDC_COMMIT_AMENDDIFF, &CCommitDlg::OnBnClickedCommitAmenddiff)
	//ON_BN_CLICKED(IDC_NOAUTOSELECTSUBMODULES, &CCommitDlg::OnBnClickedNoautoselectsubmodules)
	//ON_BN_CLICKED(IDC_COMMIT_SETDATETIME, &CCommitDlg::OnBnClickedCommitSetDateTime)
	//ON_BN_CLICKED(IDC_CHECK_NEWBRANCH, &CCommitDlg::OnBnClickedCheckNewBranch)
	//ON_BN_CLICKED(IDC_COMMIT_SETAUTHOR, &CCommitDlg::OnBnClickedCommitSetauthor)
	ON_BN_CLICKED(IDC_CREATE_CP_BUTTON, &CSICommitDlg::OnBnClickedCreateCpButton)
	ON_BN_CLICKED(IDC_SUBMIT_CP_BUTTON, &CSICommitDlg::OnBnClickedSubmitCpButton)
	ON_BN_CLICKED(IDCANCEL, &CSICommitDlg::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_SUBMIT_CP_COMBOBOX, &CSICommitDlg::OnCbnSelchangeSubmitCpCombobox)
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

	//TODO: GORD: Set any control text, e.g. labels
	//SetDlgItemText(ID_RESOURCEID, _T(""));

	//TODO: GORD: Initially enable or diable any controls
	//GetDlgItem(IDC_RESOURCEID)->EnableWindow(TRUE);
	//GetDlgItem(IDC_RESOURCEID)->EnableWindow(FALSE);

	//TODO: GORD: Initially hid or show any controls
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

	// Get the bounding rect of the dialog
	GetClientRect(m_DlgOrigRect);

	//TODO: GORD: Anchor controls using resizeable lib
	//AddAnchor(IDC_COMMITLABEL, TOP_LEFT, TOP_RIGHT);
	//AddAnchor(IDC_BUGIDLABEL, TOP_RIGHT);
	//AddAnchor(IDC_VIEW_PATCH, BOTTOM_RIGHT);
	//AddAnchor(IDC_LISTGROUP, TOP_LEFT, BOTTOM_RIGHT);
	//AddAnchor(IDC_SHOWUNVERSIONED, BOTTOM_LEFT);
	//AddAnchor(IDC_EXTERNALWARNING, BOTTOM_RIGHT);
	//AddAnchor(IDC_STATISTICS, BOTTOM_LEFT, BOTTOM_RIGHT);

	// Center dialog in Windows explorer (if its window handle was pass as parameter)
	if (hWndExplorer)
		CenterWindow(CWnd::FromHandle(hWndExplorer));

	// Loads window rect that was saved allowing user to resize and persist
	EnableSaveRestore(_T("SICommitDlg"));
	
	//TODO: GORD: Determine the users active change packages and add them to the combobox

	// If there are no active change packages then disable submit cp button 
	if( m_ctrlChangePackageComboBox.GetCount() == 0 ) {
	    GetDlgItem(IDC_SUBMIT_CP_BUTTON)->EnableWindow(FALSE);
	}

	return FALSE;  // return TRUE unless you set the focus to a control
}



void CSICommitDlg::OnOK()
{
    if (m_bBlock)
		return;
	if (m_bThreadRunning)
	{
		m_bCancelled = true;
		InterlockedExchange(&m_bRunThread, FALSE);
		WaitForSingleObject(m_pThread->m_hThread, 1000);
		if (m_bThreadRunning)
		{
			// we gave the thread a chance to quit. Since the thread didn't
			// listen to us we have to kill it.
			TerminateThread(m_pThread->m_hThread, (DWORD)-1);
			InterlockedExchange(&m_bThreadRunning, FALSE);
		}
	}
	this->UpdateData();

	if (m_bCreateNewBranch)
	{
		if (!g_Git.IsBranchNameValid(m_sCreateNewBranch))
		{
			ShowEditBalloon(IDC_NEWBRANCH, IDS_B_T_NOTEMPTY, IDS_ERR_ERROR, TTI_ERROR);
			return;
		}
		if (g_Git.BranchTagExists(m_sCreateNewBranch))
		{
			// branch already exists
			CString msg;
			msg.LoadString(IDS_B_EXISTS);
			msg += _T(" ") + CString(MAKEINTRESOURCE(IDS_B_DELETEORDIFFERENTNAME));
			ShowEditBalloon(IDC_NEWBRANCH, msg, CString(MAKEINTRESOURCE(IDS_WARN_WARNING)));
			return;
		}
		if (g_Git.BranchTagExists(m_sCreateNewBranch, false))
		{
			// tag with the same name exists -> shortref is ambiguous
			if (CMessageBox::Show(m_hWnd, IDS_B_SAMETAGNAMEEXISTS, IDS_APPNAME, 2, IDI_EXCLAMATION, IDS_CONTINUEBUTTON, IDS_ABORTBUTTON) == 2)
				return;
		}
	}

	CString id;
	GetDlgItemText(IDC_BUGID, id);
	if (!m_ProjectProperties.CheckBugID(id))
	{
		ShowEditBalloon(IDC_BUGID, IDS_COMMITDLG_ONLYNUMBERS, IDS_ERR_ERROR, TTI_ERROR);
		return;
	}
	m_sLogMessage = m_cLogMessage.GetText();
	if ( m_sLogMessage.IsEmpty() )
	{
		// no message entered, go round again
		CMessageBox::Show(this->m_hWnd, IDS_COMMITDLG_NOMESSAGE, IDS_APPNAME, MB_OK | MB_ICONERROR);
		return;
	}
	if ((m_ProjectProperties.bWarnIfNoIssue) && (id.IsEmpty() && !m_ProjectProperties.HasBugID(m_sLogMessage)))
	{
		if (CMessageBox::Show(this->m_hWnd, IDS_COMMITDLG_NOISSUEWARNING, IDS_APPNAME, MB_YESNO | MB_ICONWARNING)!=IDYES)
			return;
	}

	if (m_ProjectProperties.bWarnNoSignedOffBy == TRUE && m_cLogMessage.GetText().Find(GetSignedOffByLine()) == -1)
	{
		UINT retval = CMessageBox::Show(this->m_hWnd, IDS_PROC_COMMIT_NOSIGNOFFLINE, IDS_APPNAME, 1, IDI_WARNING, IDS_PROC_COMMIT_ADDSIGNOFFBUTTON, IDS_PROC_COMMIT_NOADDSIGNOFFBUTTON, IDS_ABORTBUTTON);
		if (retval == 1)
		{
			OnBnClickedSignOff();
			m_sLogMessage = m_cLogMessage.GetText();
		}
		else if (retval == 3)
			return;
	}

	if (CHooks::Instance().IsHookPresent(pre_commit_hook, g_Git.m_CurrentDir)) {
		DWORD exitcode = 0xFFFFFFFF;
		CString error;
		CTGitPathList list;
		m_ListCtrl.WriteCheckedNamesToPathList(list);
		if (CHooks::Instance().PreCommit(g_Git.m_CurrentDir, list, m_sLogMessage, exitcode, error))
		{
			if (exitcode)
			{
				CString temp;
				temp.Format(IDS_ERR_HOOKFAILED, (LPCTSTR)error);
				MessageBox(temp, _T("TortoiseGit"), MB_ICONERROR);
				return;
			}
		}
	}

	int nListItems = m_ListCtrl.GetItemCount();
	bool needResetIndex = false;
	for (int i = 0; i < nListItems && !m_bCommitMessageOnly; ++i)
	{
		CTGitPath *entry = (CTGitPath *)m_ListCtrl.GetItemData(i);
		if (!entry->m_Checked && !(entry->m_Action & CTGitPath::LOGACTIONS_UNVER))
			needResetIndex = true;
		if (!entry->m_Checked || !entry->IsDirectory())
			continue;

		bool dirty = false;
		if (entry->m_Action & CTGitPath::LOGACTIONS_UNVER)
		{
			CGit subgit;
			subgit.m_CurrentDir = g_Git.CombinePath(entry);
			CString subcmdout;
			subgit.Run(_T("git.exe status --porcelain"), &subcmdout, CP_UTF8);
			dirty = !subcmdout.IsEmpty();
		}
		else
		{
			CString cmd, cmdout;
			cmd.Format(_T("git.exe diff -- \"%s\""), entry->GetWinPathString());
			g_Git.Run(cmd, &cmdout, CP_UTF8);
			dirty = cmdout.Right(7) == _T("-dirty\n");
		}

		if (dirty)
		{
			CString message;
			message.Format(CString(MAKEINTRESOURCE(IDS_COMMITDLG_SUBMODULEDIRTY)), entry->GetGitPathString());
			int result = CMessageBox::Show(m_hWnd, message, _T("TortoiseGit"), 1, IDI_QUESTION, CString(MAKEINTRESOURCE(IDS_PROGRS_CMD_COMMIT)), CString(MAKEINTRESOURCE(IDS_MSGBOX_IGNORE)), CString(MAKEINTRESOURCE(IDS_MSGBOX_CANCEL)));
			if (result == 1)
			{
				CString cmdCommit;
				cmdCommit.Format(_T("/command:commit /path:\"%s\\%s\""), g_Git.m_CurrentDir, entry->GetWinPathString());
				CAppUtils::RunTortoiseGitProc(cmdCommit);
				return;
			}
			else if (result == 2)
				continue;
			else
				return;
		}
	}

	if (!m_bCommitMessageOnly)
		m_ListCtrl.WriteCheckedNamesToPathList(m_selectedPathList);
	m_pathwatcher.Stop();
	InterlockedExchange(&m_bBlock, TRUE);
	//first add all the unversioned files the user selected
	//and check if all versioned files are selected
	int nchecked = 0;

	CMassiveGitTask mgtReAddAfterCommit(_T("add --ignore-errors -f"));

	CString cmd;
	CString out;

	bool bAddSuccess=true;
	bool bCloseCommitDlg=false;

	CSysProgressDlg sysProgressDlg;
	if (nListItems >= 25)
	{
		sysProgressDlg.SetTitle(CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_PREPARECOMMIT)));
		sysProgressDlg.SetLine(1, CString(MAKEINTRESOURCE(IDS_PROC_COMMIT_UPDATEINDEX)));
		sysProgressDlg.SetTime(true);
		sysProgressDlg.SetShowProgressBar(true);
		sysProgressDlg.ShowModal(this, true);
	}

	CBlockCacheForPath cacheBlock(g_Git.m_CurrentDir);
	DWORD currentTicks = GetTickCount();

	if (g_Git.UsingLibGit2(CGit::GIT_CMD_COMMIT_UPDATE_INDEX))
	{
		/*
			Do not use the libgit2 implementation right now, since it has several flaws:
			* https://code.google.com/p/tortoisegit/issues/detail?id=1690: possible access denied problem
			* https://code.google.com/p/tortoisegit/issues/detail?id=2224: filters not correctly applied
			* changes to x-bit are not correctly committed, since we reset the index
		*/
		bAddSuccess = false;
		do
		{
			CAutoRepository repository(g_Git.GetGitRepository());
			if (!repository)
			{
				CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not open repository.")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
				break;
			}

			CGitHash revHash;
			CString revRef = _T("HEAD");
			if (m_bCommitAmend && !m_bAmendDiffToLastCommit)
				revRef = _T("HEAD~1");
			if (CGit::GetHash(repository, revHash, revRef))
			{
				MessageBox(g_Git.GetLibGit2LastErr(_T("Could not get HEAD hash after committing.")), _T("TortoiseGit"), MB_ICONERROR);
				break;
			}

			CAutoCommit commit;
			if (!revHash.IsEmpty() && needResetIndex && git_commit_lookup(commit.GetPointer(), repository, (const git_oid*)revHash.m_hash))
			{
				CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not get last commit.")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
				break;
			}

			CAutoTree tree;
			if (!revHash.IsEmpty() && needResetIndex && git_commit_tree(tree.GetPointer(), commit))
			{
				CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not read tree of commit.")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
				break;
			}

			CAutoIndex index;
			if (git_repository_index(index.GetPointer(), repository))
			{
				CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not get the repository index.")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
				break;
			}

			// reset index to the one of the reference commit (HEAD or HEAD~1)
			if (!revHash.IsEmpty() && needResetIndex && git_index_read_tree(index, tree))
			{
				CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not read the tree into the index.")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
				break;
			}
			else if (!revHash.IsEmpty() && !needResetIndex)
			{
				git_index_read(index, true);
			}

			bAddSuccess = true;

			for (int j = 0; j < nListItems; ++j)
			{
				CTGitPath *entry = (CTGitPath*)m_ListCtrl.GetItemData(j);

				if (sysProgressDlg.IsVisible())
				{
					if (GetTickCount() - currentTicks > 1000 || j == nListItems - 1 || j == 0)
					{
						sysProgressDlg.SetLine(2, entry->GetGitPathString(), true);
						sysProgressDlg.SetProgress(j, nListItems);
						AfxGetThread()->PumpMessage(); // process messages, in order to avoid freezing; do not call this too often: this takes time!
						currentTicks = GetTickCount();
					}
				}

				CStringA filePathA = CUnicodeUtils::GetMulti(entry->GetGitPathString(), CP_UTF8);

				if (entry->m_Checked && !m_bCommitMessageOnly)
				{
					if (entry->IsDirectory())
					{
						git_submodule *submodule = nullptr;
						if (git_submodule_lookup(&submodule, repository, filePathA))
						{
							bAddSuccess = false;
							CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not open submodule \"") + entry->GetGitPathString() + _T("\".")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
							break;
						}
						if (git_submodule_add_to_index(submodule, FALSE))
						{
							bAddSuccess = false;
							CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not add submodule \"") + entry->GetGitPathString() + _T("\" to index.")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
							break;
						}
					}
					else if (entry->m_Action & CTGitPath::LOGACTIONS_DELETED)
					{
						git_index_remove_bypath(index, filePathA); // ignore error
					}
					else
					{
						if (git_index_add_bypath(index, filePathA))
						{
							bAddSuccess = false;
							CMessageBox::Show(m_hWnd, CGit::GetLibGit2LastErr(_T("Could not add \"") + entry->GetGitPathString() + _T("\" to index.")), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
							break;
						}
					}

					if (entry->m_Action & CTGitPath::LOGACTIONS_REPLACED)
					{
						git_index_remove_bypath(index, filePathA); // ignore error
					}

					++nchecked;
				}
				else if (entry->m_Action & CTGitPath::LOGACTIONS_ADDED || entry->m_Action & CTGitPath::LOGACTIONS_REPLACED)
					mgtReAddAfterCommit.AddFile(*entry);

				if (sysProgressDlg.HasUserCancelled())
				{
					bAddSuccess = false;
					break;
				}

				CShellUpdater::Instance().AddPathForUpdate(*entry);
			}
			if (bAddSuccess && git_index_write(index))
				bAddSuccess = false;
		} while (0);
	}
	else
	{
		// ***************************************************
		// ATTENTION: Similar code in RebaseDlg.cpp!!!
		// ***************************************************
		CMassiveGitTask mgtAdd(_T("add -f"));
		CMassiveGitTask mgtUpdateIndexForceRemove(_T("update-index --force-remove"));
		CMassiveGitTask mgtUpdateIndex(_T("update-index"));
		CMassiveGitTask mgtRm(_T("rm --ignore-unmatch"));
		CMassiveGitTask mgtRmFCache(_T("rm -f --cache"));
		CString resetCmd = _T("reset");
		if (m_bCommitAmend && !m_bAmendDiffToLastCommit)
			resetCmd += _T(" HEAD~1");;
		CMassiveGitTask mgtReset(resetCmd, TRUE, true);
		for (int j = 0; j < nListItems; ++j)
		{
			CTGitPath *entry = (CTGitPath*)m_ListCtrl.GetItemData(j);

			if (entry->m_Checked && !m_bCommitMessageOnly)
			{
				if (entry->m_Action & CTGitPath::LOGACTIONS_UNVER)
					mgtAdd.AddFile(entry->GetGitPathString());
				else if (entry->m_Action & CTGitPath::LOGACTIONS_DELETED)
					mgtUpdateIndexForceRemove.AddFile(entry->GetGitPathString());
				else
					mgtUpdateIndex.AddFile(entry->GetGitPathString());

				if ((entry->m_Action & CTGitPath::LOGACTIONS_REPLACED) && !entry->GetGitOldPathString().IsEmpty())
					mgtRm.AddFile(entry->GetGitOldPathString());

				++nchecked;
			}
			else
			{
				if (entry->m_Action & CTGitPath::LOGACTIONS_ADDED || entry->m_Action & CTGitPath::LOGACTIONS_REPLACED)
				{	//To init git repository, there are not HEAD, so we can use git reset command
					mgtRmFCache.AddFile(entry->GetGitPathString());
					mgtReAddAfterCommit.AddFile(*entry);

					if (entry->m_Action & CTGitPath::LOGACTIONS_REPLACED && !entry->GetGitOldPathString().IsEmpty())
						mgtReset.AddFile(entry->GetGitOldPathString());
				}
				else if (!(entry->m_Action & CTGitPath::LOGACTIONS_UNVER))
					mgtReset.AddFile(entry->GetGitPathString());
			}

			if (sysProgressDlg.HasUserCancelled())
			{
				bAddSuccess = false;
				break;
			}
		}

		CMassiveGitTask tasks[] = { mgtAdd, mgtUpdateIndexForceRemove, mgtUpdateIndex, mgtRm, mgtRmFCache, mgtReset };
		int progress = 0, maxProgress = 0;
		for (int j = 0; j < _countof(tasks); ++j)
			maxProgress += tasks[j].GetListCount();
		for (int j = 0; j < _countof(tasks); ++j)
			bAddSuccess = bAddSuccess && UpdateIndex(tasks[j], sysProgressDlg, progress += tasks[j].GetListCount(), maxProgress);

		if (sysProgressDlg.HasUserCancelled())
			bAddSuccess = false;

		for (int j = 0; bAddSuccess && j < nListItems; ++j)
			CShellUpdater::Instance().AddPathForUpdate(*(CTGitPath*)m_ListCtrl.GetItemData(j));
	}

	if (sysProgressDlg.HasUserCancelled())
		bAddSuccess = false;

	//sysProgressDlg.Stop();

	if (bAddSuccess && m_bCreateNewBranch)
	{
		if (g_Git.Run(_T("git.exe branch ") + m_sCreateNewBranch, &out, CP_UTF8))
		{
			MessageBox(_T("Creating new branch failed:\n") + out, _T("TortoiseGit"), MB_OK | MB_ICONERROR);
			bAddSuccess = false;
		}
		if (g_Git.Run(_T("git.exe checkout ") + m_sCreateNewBranch + _T(" --"), &out, CP_UTF8))
		{
			MessageBox(_T("Switching to new branch failed:\n") + out, _T("TortoiseGit"), MB_OK | MB_ICONERROR);
			bAddSuccess = false;
		}
	}

	if (bAddSuccess && m_bWarnDetachedHead && CheckHeadDetach())
		bAddSuccess = false;

	m_sBugID.Trim();
	CString sExistingBugID = m_ProjectProperties.FindBugID(m_sLogMessage);
	sExistingBugID.Trim();
	if (!m_sBugID.IsEmpty() && m_sBugID.Compare(sExistingBugID))
	{
		m_sBugID.Replace(_T(", "), _T(","));
		m_sBugID.Replace(_T(" ,"), _T(","));
		CString sBugID = m_ProjectProperties.sMessage;
		sBugID.Replace(_T("%BUGID%"), m_sBugID);
		if (m_ProjectProperties.bAppend)
			m_sLogMessage += _T("\n") + sBugID + _T("\n");
		else
			m_sLogMessage = sBugID + _T("\n") + m_sLogMessage;
	}

	// now let the bugtraq plugin check the commit message
	CComPtr<IBugTraqProvider2> pProvider2 = NULL;
	if (m_BugTraqProvider)
	{
		HRESULT hr = m_BugTraqProvider.QueryInterface(&pProvider2);
		if (SUCCEEDED(hr))
		{
			ATL::CComBSTR temp;
			ATL::CComBSTR repositoryRoot(g_Git.m_CurrentDir);
			ATL::CComBSTR parameters(m_bugtraq_association.GetParameters());
			ATL::CComBSTR commonRoot(m_pathList.GetCommonRoot().GetDirectory().GetWinPath());
			ATL::CComBSTR commitMessage(m_sLogMessage);
			CBstrSafeVector pathList(m_selectedPathList.GetCount());

			for (LONG index = 0; index < m_selectedPathList.GetCount(); ++index)
				pathList.PutElement(index, m_selectedPathList[index].GetGitPathString());

			if (FAILED(hr = pProvider2->CheckCommit(GetSafeHwnd(), parameters, repositoryRoot, commonRoot, pathList, commitMessage, &temp)))
			{
				COMError ce(hr);
				CString sErr;
				sErr.Format(IDS_ERR_FAILEDISSUETRACKERCOM, m_bugtraq_association.GetProviderName(), ce.GetMessageAndDescription().c_str());
				CMessageBox::Show(m_hWnd, sErr, _T("TortoiseGit"), MB_ICONERROR);
			}
			else
			{
				CString sError = temp;
				if (!sError.IsEmpty())
				{
					CMessageBox::Show(m_hWnd, sError, _T("TortoiseGit"), MB_ICONERROR);
					return;
				}
			}
		}
	}

	if (m_bCommitMessageOnly || bAddSuccess && (nchecked || m_bCommitAmend ||  CTGitPath(g_Git.m_CurrentDir).IsMergeActive()))
	{
		bCloseCommitDlg = true;

		CString tempfile=::GetTempFile();

		if (CAppUtils::SaveCommitUnicodeFile(tempfile, m_sLogMessage))
		{
			CMessageBox::Show(nullptr, _T("Could not save commit message"), _T("TortoiseGit"), MB_OK | MB_ICONERROR);
			return;
		}

		CTGitPath path=g_Git.m_CurrentDir;

		BOOL IsGitSVN = path.GetAdminDirMask() & ITEMIS_GITSVN;

		out =_T("");
		CString amend;
		if(this->m_bCommitAmend)
		{
			amend=_T("--amend");
		}
		CString dateTime;
		if (m_bSetCommitDateTime)
		{
			CTime date, time;
			m_CommitDate.GetTime(date);
			m_CommitTime.GetTime(time);
			dateTime.Format(_T("--date=%sT%s"), date.Format(_T("%Y-%m-%d")), time.Format(_T("%H:%M:%S")));
		}
		CString author;
		if (m_bSetAuthor)
			author.Format(_T("--author=\"%s\""), m_sAuthor);
		CString allowEmpty = m_bCommitMessageOnly ? _T("--allow-empty") : _T("");
		cmd.Format(_T("git.exe commit %s %s %s %s -F \"%s\""), author, dateTime, amend, allowEmpty, tempfile);

		CCommitProgressDlg progress;
		progress.m_bBufferAll=true; // improve show speed when there are many file added.
		progress.m_GitCmd=cmd;
		progress.m_bShowCommand = FALSE;	// don't show the commit command
		progress.m_PreText = out;			// show any output already generated in log window

		progress.m_PostCmdCallback = [&](DWORD status, PostCmdList& postCmdList)
		{
			if (status || m_bNoPostActions || m_bAutoClose)
				return;

			if (IsGitSVN)
				postCmdList.push_back(PostCmd(IDI_COMMIT, IDS_MENUSVNDCOMMIT, [&]{ m_PostCmd = GIT_POSTCOMMIT_CMD_DCOMMIT; }));

			postCmdList.push_back(PostCmd(IDI_PUSH, IDS_MENUPUSH, [&]{ m_PostCmd = GIT_POSTCOMMIT_CMD_PUSH; }));
			postCmdList.push_back(PostCmd(IDI_PULL, IDS_MENUPULL, [&]{ m_PostCmd = GIT_POSTCOMMIT_CMD_PULL; }));
			postCmdList.push_back(PostCmd(IDI_COMMIT, IDS_PROC_COMMIT_RECOMMIT, [&]{ m_PostCmd = GIT_POSTCOMMIT_CMD_RECOMMIT; }));
			postCmdList.push_back(PostCmd(IDI_TAG, IDS_MENUTAG, [&]{ m_PostCmd = GIT_POSTCOMMIT_CMD_CREATETAG; }));
		};

		m_PostCmd = GIT_POSTCOMMIT_CMD_NOTHING;
		progress.DoModal();

		if (progress.m_GitStatus || m_PostCmd == GIT_POSTCOMMIT_CMD_RECOMMIT)
		{
			bCloseCommitDlg = false;
			if (m_PostCmd == GIT_POSTCOMMIT_CMD_RECOMMIT)
			{
				if (!m_sLogMessage.IsEmpty())
				{
					m_History.AddEntry(m_sLogMessage);
					m_History.Save();
				}

				this->m_sLogMessage.Empty();
				GetCommitTemplate(m_sLogMessage);
				RunStartCommitHook();
				m_cLogMessage.SetText(m_sLogMessage);
				if (m_bCreateNewBranch)
				{
					GetDlgItem(IDC_COMMIT_TO)->ShowWindow(SW_SHOW);
					GetDlgItem(IDC_NEWBRANCH)->ShowWindow(SW_HIDE);
				}
				m_bCreateNewBranch = FALSE;
			}

			if (!progress.m_GitStatus)
			{
				m_AmendStr.Empty();
				m_bCommitAmend = FALSE;
			}

			UpdateData(FALSE);
			this->Refresh();
			this->BringWindowToTop();
		}

		::DeleteFile(tempfile);

		if (m_BugTraqProvider && progress.m_GitStatus == 0)
		{
			CComPtr<IBugTraqProvider2> pProvider = NULL;
			HRESULT hr = m_BugTraqProvider.QueryInterface(&pProvider);
			if (SUCCEEDED(hr))
			{
				ATL::CComBSTR commonRoot(g_Git.m_CurrentDir);
				CBstrSafeVector pathList(m_selectedPathList.GetCount());

				for (LONG index = 0; index < m_selectedPathList.GetCount(); ++index)
					pathList.PutElement(index, m_selectedPathList[index].GetGitPathString());

				ATL::CComBSTR logMessage(m_sLogMessage);

				CGitHash hash;
				if (g_Git.GetHash(hash, _T("HEAD")))
					MessageBox(g_Git.GetGitLastErr(_T("Could not get HEAD hash after committing.")), _T("TortoiseGit"), MB_ICONERROR);
				LONG version = g_Git.Hash2int(hash);

				ATL::CComBSTR temp;
				if (FAILED(hr = pProvider->OnCommitFinished(GetSafeHwnd(),
					commonRoot,
					pathList,
					logMessage,
					(LONG)version,
					&temp)))
				{
					CString sErr = temp;
					if (!sErr.IsEmpty())
						CMessageBox::Show(NULL,(sErr),_T("TortoiseGit"),MB_OK|MB_ICONERROR);
					else
					{
						COMError ce(hr);
						sErr.Format(IDS_ERR_FAILEDISSUETRACKERCOM, ce.GetSource().c_str(), ce.GetMessageAndDescription().c_str());
						CMessageBox::Show(NULL,(sErr),_T("TortoiseGit"),MB_OK|MB_ICONERROR);
					}
				}
			}
		}
		RestoreFiles(progress.m_GitStatus == 0);
		if (((DWORD)CRegStdDWORD(_T("Software\\TortoiseGit\\ReaddUnselectedAddedFilesAfterCommit"), TRUE)) == TRUE)
		{
			BOOL cancel = FALSE;
			mgtReAddAfterCommit.Execute(cancel);
		}
	}
	else if(bAddSuccess)
	{
		CMessageBox::Show(this->m_hWnd, IDS_ERROR_NOTHING_COMMIT, IDS_COMMIT_FINISH, MB_OK | MB_ICONINFORMATION);
		bCloseCommitDlg=false;
	}

	UpdateData();
	m_regAddBeforeCommit = m_bShowUnversioned;
	m_regKeepChangelists = m_bKeepChangeList;
	m_regDoNotAutoselectSubmodules = m_bDoNotAutoselectSubmodules;
	if (!GetDlgItem(IDC_KEEPLISTS)->IsWindowEnabled())
		m_bKeepChangeList = FALSE;
	InterlockedExchange(&m_bBlock, FALSE);

	if (!m_sLogMessage.IsEmpty())
	{
		m_History.AddEntry(m_sLogMessage);
		m_History.Save();
	}

	SaveSplitterPos();

	if( bCloseCommitDlg )
		CResizableStandAloneDialog::OnOK();

	CShellUpdater::Instance().Flush();
}



UINT CSICommitDlg::StatusThreadEntry(LPVOID pVoid)
{
	return ((CCommitDlg*)pVoid)->StatusThread();
}

UINT CCommitDlg::StatusThread()
{
	//get the status of all selected file/folders recursively
	//and show the ones which have to be committed to the user
	//in a list control.
	InterlockedExchange(&m_bBlock, TRUE);
	InterlockedExchange(&m_bThreadRunning, TRUE);// so the main thread knows that this thread is still running
	InterlockedExchange(&m_bRunThread, TRUE);	// if this is set to FALSE, the thread should stop

	m_pathwatcher.Stop();

	m_ListCtrl.SetBusy(true);
	g_Git.RefreshGitIndex();

	m_bCancelled = false;

	DialogEnableWindow(IDOK, false);
	DialogEnableWindow(IDC_SHOWUNVERSIONED, false);
	DialogEnableWindow(IDC_WHOLE_PROJECT, false);
	DialogEnableWindow(IDC_NOAUTOSELECTSUBMODULES, false);
	GetDlgItem(IDC_EXTERNALWARNING)->ShowWindow(SW_HIDE);
	DialogEnableWindow(IDC_EXTERNALWARNING, false);
	DialogEnableWindow(IDC_COMMIT_AMEND, FALSE);
	DialogEnableWindow(IDC_COMMIT_AMENDDIFF, FALSE);
	// read the list of recent log entries before querying the WC for status
	// -> the user may select one and modify / update it while we are crawling the WC

	DialogEnableWindow(IDC_CHECKALL, false);
	DialogEnableWindow(IDC_CHECKNONE, false);
	DialogEnableWindow(IDC_CHECKUNVERSIONED, false);
	DialogEnableWindow(IDC_CHECKVERSIONED, false);
	DialogEnableWindow(IDC_CHECKADDED, false);
	DialogEnableWindow(IDC_CHECKDELETED, false);
	DialogEnableWindow(IDC_CHECKMODIFIED, false);
	DialogEnableWindow(IDC_CHECKFILES, false);
	DialogEnableWindow(IDC_CHECKSUBMODULES, false);

	if (m_History.IsEmpty())
	{
		CString reg;
		reg.Format(_T("Software\\TortoiseGit\\History\\commit%s"), (LPCTSTR)m_ListCtrl.m_sUUID);
		reg.Replace(_T(':'),_T('_'));
		m_History.Load(reg, _T("logmsgs"));
	}

	CString dotGitPath;
	g_GitAdminDir.GetAdminDirPath(g_Git.m_CurrentDir, dotGitPath);
	if (PathFileExists(dotGitPath + _T("CHERRY_PICK_HEAD")))
	{
		GetDlgItem(IDC_COMMIT_AMENDDIFF)->ShowWindow(SW_HIDE);
		m_bCommitAmend = FALSE;
		SendMessage(WM_UPDATEDATAFALSE);
	}

	// Initialise the list control with the status of the files/folders below us
	m_ListCtrl.Clear();
	BOOL success;
	CTGitPathList *pList;
	m_ListCtrl.m_amend = (m_bCommitAmend==TRUE || m_bForceCommitAmend) && (m_bAmendDiffToLastCommit==FALSE);
	m_ListCtrl.m_bDoNotAutoselectSubmodules = (m_bDoNotAutoselectSubmodules == TRUE);

	if(m_bWholeProject || m_bWholeProject2)
		pList=NULL;
	else
		pList = &m_pathList;

	success=m_ListCtrl.GetStatus(pList);

	//m_ListCtrl.UpdateFileList(git_revnum_t(GIT_REV_ZERO));
	if(this->m_bShowUnversioned)
		m_ListCtrl.UpdateFileList(CGitStatusListCtrl::FILELIST_UNVER,true,pList);

	m_ListCtrl.CheckIfChangelistsArePresent(false);

	DWORD dwShow = GITSLC_SHOWVERSIONEDBUTNORMALANDEXTERNALSFROMDIFFERENTREPOS | GITSLC_SHOWLOCKS | GITSLC_SHOWINCHANGELIST | GITSLC_SHOWDIRECTFILES;
	dwShow |= DWORD(m_regAddBeforeCommit) ? GITSLC_SHOWUNVERSIONED : 0;
	if (success)
	{
		if (!m_checkedPathList.IsEmpty())
			m_ListCtrl.Show(dwShow, m_checkedPathList);
		else
		{
			DWORD dwCheck = m_bSelectFilesForCommit ? dwShow : 0;
			dwCheck &=~(CTGitPath::LOGACTIONS_UNVER); //don't check unversion file default.
			m_ListCtrl.Show(dwShow, dwCheck);
			m_bSelectFilesForCommit = true;
		}

		if (m_ListCtrl.HasExternalsFromDifferentRepos())
		{
			GetDlgItem(IDC_EXTERNALWARNING)->ShowWindow(SW_SHOW);
			DialogEnableWindow(IDC_EXTERNALWARNING, TRUE);
		}

		SetDlgItemText(IDC_COMMIT_TO, g_Git.GetCurrentBranch());
		m_tooltips.AddTool(GetDlgItem(IDC_STATISTICS), m_ListCtrl.GetStatisticsString());
	}
	if (!success)
	{
		if (!m_ListCtrl.GetLastErrorMessage().IsEmpty())
			m_ListCtrl.SetEmptyString(m_ListCtrl.GetLastErrorMessage());
		m_ListCtrl.Show(dwShow);
	}

	if ((m_ListCtrl.GetItemCount()==0)&&(m_ListCtrl.HasUnversionedItems())
		 && !PathFileExists(dotGitPath + _T("MERGE_HEAD")))
	{
		CString temp;
		temp.LoadString(IDS_COMMITDLG_NOTHINGTOCOMMITUNVERSIONED);
		if (CMessageBox::ShowCheck(m_hWnd, temp, _T("TortoiseGit"), MB_ICONINFORMATION | MB_YESNO, _T("NothingToCommitShowUnversioned"), NULL)==IDYES)
		{
			m_bShowUnversioned = TRUE;
			GetDlgItem(IDC_SHOWUNVERSIONED)->SendMessage(BM_SETCHECK, BST_CHECKED);
			DWORD dwShow = (DWORD)(GITSLC_SHOWVERSIONEDBUTNORMALANDEXTERNALSFROMDIFFERENTREPOS | GITSLC_SHOWUNVERSIONED | GITSLC_SHOWLOCKS);
			m_ListCtrl.UpdateFileList(CGitStatusListCtrl::FILELIST_UNVER);
			m_ListCtrl.Show(dwShow,dwShow&(~CTGitPath::LOGACTIONS_UNVER));
		}
	}

	SetDlgTitle();

	m_autolist.clear();
	// we don't have to block the commit dialog while we fetch the
	// auto completion list.
	m_pathwatcher.ClearChangedPaths();
	InterlockedExchange(&m_bBlock, FALSE);
	if ((DWORD)CRegDWORD(_T("Software\\TortoiseGit\\Autocompletion"), TRUE)==TRUE)
	{
		m_ListCtrl.Block(TRUE, TRUE);
		GetAutocompletionList();
		m_ListCtrl.Block(FALSE, FALSE);
	}
	SendMessage(WM_UPDATEOKBUTTON);
	if (m_bRunThread)
	{
		DialogEnableWindow(IDC_SHOWUNVERSIONED, true);
		DialogEnableWindow(IDC_WHOLE_PROJECT, !m_bWholeProject2);
		DialogEnableWindow(IDC_NOAUTOSELECTSUBMODULES, true);
		if (m_ListCtrl.HasChangeLists())
			DialogEnableWindow(IDC_KEEPLISTS, true);

		// activate amend checkbox (if necessary)
		if (g_Git.IsInitRepos())
		{
			m_bCommitAmend = FALSE;
			SendMessage(WM_UPDATEDATAFALSE);
		}
		else
		{
			if (m_bForceCommitAmend)
			{
				m_bCommitAmend = TRUE;
				SendMessage(WM_UPDATEDATAFALSE);
			}
			else
				GetDlgItem(IDC_COMMIT_AMEND)->EnableWindow(!PathFileExists(dotGitPath + _T("CHERRY_PICK_HEAD")));

			CGitHash hash;
			if (g_Git.GetHash(hash, _T("HEAD")))
			{
				MessageBox(g_Git.GetGitLastErr(_T("Could not get HEAD hash.")), _T("TortoiseGit"), MB_ICONERROR);
			}
			if (!hash.IsEmpty())
			{
				GitRev headRevision;
				try
				{
					headRevision.GetParentFromHash(hash);
				}
				catch (char* msg)
				{
					CString err(msg);
					MessageBox(_T("Could not get parent from HEAD.\nlibgit reports:\n") + err, _T("TortoiseGit"), MB_ICONERROR);
				}
				// do not allow to show diff to "last" revision if it has more that one parent
				if (headRevision.ParentsCount() != 1)
				{
					m_bAmendDiffToLastCommit = TRUE;
					SendMessage(WM_UPDATEDATAFALSE);
				}
				else
					GetDlgItem(IDC_COMMIT_AMENDDIFF)->EnableWindow(TRUE);
			}
		}

		UpdateCheckLinks();

		// we have the list, now signal the main thread about it
		SendMessage(WM_AUTOLISTREADY);	// only send the message if the thread wasn't told to quit!
	}

	InterlockedExchange(&m_bRunThread, FALSE);
	InterlockedExchange(&m_bThreadRunning, FALSE);
	// force the cursor to normal
	RefreshCursor();

	return 0;
}

void CSICommitDlg::OnCancel()
{
	//m_bCancelled = true;
	//m_pathwatcher.Stop();

	//if (m_bThreadRunning)
	//{
	//	InterlockedExchange(&m_bRunThread, FALSE);
	//	WaitForSingleObject(m_pThread->m_hThread, 1000);
	//	if (m_bThreadRunning)
	//	{
	//		// we gave the thread a chance to quit. Since the thread didn't
	//		// listen to us we have to kill it.
	//		TerminateThread(m_pThread->m_hThread, (DWORD)-1);
	//		InterlockedExchange(&m_bThreadRunning, FALSE);
	//	}
	//}
	//UpdateData();
	//m_sBugID.Trim();
	//m_sLogMessage = m_cLogMessage.GetText();
	//if (!m_sBugID.IsEmpty())
	//{
	//	m_sBugID.Replace(_T(", "), _T(","));
	//	m_sBugID.Replace(_T(" ,"), _T(","));
	//	CString sBugID = m_ProjectProperties.sMessage;
	//	sBugID.Replace(_T("%BUGID%"), m_sBugID);
	//	if (m_ProjectProperties.bAppend)
	//		m_sLogMessage += _T("\n") + sBugID + _T("\n");
	//	else
	//		m_sLogMessage = sBugID + _T("\n") + m_sLogMessage;
	//}
	//if ((m_sLogTemplate.Compare(m_sLogMessage) != 0) && !m_sLogMessage.IsEmpty())
	//{
	//	m_History.AddEntry(m_sLogMessage);
	//	m_History.Save();
	//}
	//RestoreFiles();
	//SaveSplitterPos();
	//CResizableStandAloneDialog::OnCancel();
}

BOOL CSICommitDlg::PreTranslateMessage(MSG* pMsg)
{
	if (!m_bBlock)
		m_tooltips.RelayEvent(pMsg);

	if (m_hAccel)
	{
		int ret = TranslateAccelerator(m_hWnd, m_hAccel, pMsg);
		if (ret)
			return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN)
	{
		switch (pMsg->wParam)
		{
		case VK_F5:
			{
				if (m_bBlock)
					return CResizableStandAloneDialog::PreTranslateMessage(pMsg);
				Refresh();
			}
			break;
		case VK_RETURN:
			{
				if (GetAsyncKeyState(VK_CONTROL)&0x8000)
				{
					if ( GetDlgItem(IDOK)->IsWindowEnabled() )
					{
						PostMessage(WM_COMMAND, IDOK);
					}
					return TRUE;
				}
				if ( GetFocus()==GetDlgItem(IDC_BUGID) )
				{
					// Pressing RETURN in the bug id control
					// moves the focus to the message
					GetDlgItem(IDC_LOGMESSAGE)->SetFocus();
					return TRUE;
				}
			}
			break;
		}
	}

	return CResizableStandAloneDialog::PreTranslateMessage(pMsg);
}

void CSICommitDlg::Refresh()
{
	if (m_bThreadRunning)
		return;

	InterlockedExchange(&m_bBlock, TRUE);
	m_pThread = AfxBeginThread(StatusThreadEntry, this, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
	if (m_pThread==NULL)
	{
		CMessageBox::Show(this->m_hWnd, IDS_ERR_THREADSTARTFAILED, IDS_APPNAME, MB_OK | MB_ICONERROR);
		InterlockedExchange(&m_bBlock, FALSE);
	}
	else
	{
		m_pThread->m_bAutoDelete = FALSE;
		m_pThread->ResumeThread();
	}
}



void CSICommitDlg::OnStnClickedExternalwarning()
{
	//m_tooltips.Popup();
}

void CSICommitDlg::OnEnChangeLogmessage()
{
	//SendMessage(WM_UPDATEOKBUTTON);
}

//////////////////////////////////////////////////////////////////////////
// functions which run in the status thread
//////////////////////////////////////////////////////////////////////////



// CSciEditContextMenuInterface
void CSICommitDlg::InsertMenuItems(CMenu& mPopup, int& nCmd)
{
	//CString sMenuItemText;
	//sMenuItemText.LoadString(IDS_COMMITDLG_POPUP_PICKCOMMITHASH);
	//m_nPopupPickCommitHash = nCmd++;
	//mPopup.AppendMenu(MF_STRING | MF_ENABLED, m_nPopupPickCommitHash, sMenuItemText);

	//sMenuItemText.LoadString(IDS_COMMITDLG_POPUP_PICKCOMMITMESSAGE);
	//m_nPopupPickCommitMessage = nCmd++;
	//mPopup.AppendMenu(MF_STRING | MF_ENABLED, m_nPopupPickCommitMessage, sMenuItemText);

	//sMenuItemText.LoadString(IDS_COMMITDLG_POPUP_PASTEFILELIST);
	//m_nPopupPasteListCmd = nCmd++;
	//mPopup.AppendMenu(MF_STRING | MF_ENABLED, m_nPopupPasteListCmd, sMenuItemText);

	//if (!m_History.IsEmpty())
	//{
	//	sMenuItemText.LoadString(IDS_COMMITDLG_POPUP_PASTELASTMESSAGE);
	//	m_nPopupPasteLastMessage = nCmd++;
	//	mPopup.AppendMenu(MF_STRING | MF_ENABLED, m_nPopupPasteLastMessage, sMenuItemText);

	//	sMenuItemText.LoadString(IDS_COMMITDLG_POPUP_LOGHISTORY);
	//	m_nPopupRecentMessage = nCmd++;
	//	mPopup.AppendMenu(MF_STRING | MF_ENABLED, m_nPopupRecentMessage, sMenuItemText);
	//}
}

bool CSICommitDlg::HandleMenuItemClick(int cmd, CSciEdit * pSciEdit)
{
	//if (cmd == m_nPopupPickCommitHash)
	//{
	//	// use the git log to allow selection of a version
	//	CLogDlg dlg;
	//	// tell the dialog to use mode for selecting revisions
	//	dlg.SetSelect(true);
	//	// only one revision must be selected however
	//	dlg.SingleSelection(true);
	//	if (dlg.DoModal() == IDOK)
	//	{
	//		// get selected hash if any
	//		CString selectedHash = dlg.GetSelectedHash();
	//		pSciEdit->InsertText(selectedHash);
	//	}
	//	return true;
	//}

	//if (cmd == m_nPopupPickCommitMessage)
	//{
	//	// use the git log to allow selection of a version
	//	CLogDlg dlg;
	//	// tell the dialog to use mode for selecting revisions
	//	dlg.SetSelect(true);
	//	// only one revision must be selected however
	//	dlg.SingleSelection(true);
	//	if (dlg.DoModal() == IDOK)
	//	{
	//		// get selected hash if any
	//		CString selectedHash = dlg.GetSelectedHash();
	//		GitRev rev;
	//		rev.GetCommit(selectedHash);
	//		CString message = rev.GetSubject() + _T("\r\n") + rev.GetBody();
	//		pSciEdit->InsertText(message);
	//	}
	//	return true;
	//}

	//if (cmd == m_nPopupPasteListCmd)
	//{
	//	CString logmsg;
	//	int nListItems = m_ListCtrl.GetItemCount();
	//	for (int i=0; i<nListItems; ++i)
	//	{
	//		CTGitPath * entry = (CTGitPath*)m_ListCtrl.GetItemData(i);
	//		if (entry&&entry->m_Checked)
	//		{
	//			CString line;
	//			CString status = entry->GetActionName();
	//			if(entry->m_Action & CTGitPath::LOGACTIONS_UNVER)
	//				status = _T("Add"); // I18N TODO

	//			//git_wc_status_kind status = entry->status;
	//			WORD langID = (WORD)CRegStdDWORD(_T("Software\\TortoiseGit\\LanguageID"), GetUserDefaultLangID());
	//			if (m_ProjectProperties.bFileListInEnglish)
	//				langID = 1033;

	//			line.Format(_T("%-10s %s\r\n"),status , (LPCTSTR)m_ListCtrl.GetItemText(i,0));
	//			logmsg += line;
	//		}
	//	}
	//	pSciEdit->InsertText(logmsg);
	//	return true;
	//}

	//if(cmd == m_nPopupPasteLastMessage)
	//{
	//	if (m_History.IsEmpty())
	//		return false;

	//	CString logmsg;
	//	logmsg +=m_History.GetEntry(0);
	//	pSciEdit->InsertText(logmsg);
	//	return true;
	//}

	//if(cmd == m_nPopupRecentMessage )
	//{
	//	OnBnClickedHistory();
	//	return true;
	//}
	return false;
}

LRESULT CSICommitDlg::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	//switch (message) {
	//case WM_NOTIFY:
	//	if (wParam == IDC_SPLITTER)
	//	{
	//		SPC_NMHDR* pHdr = (SPC_NMHDR*) lParam;
	//		DoSize(pHdr->delta);
	//	}
	//	break;
	//}

	return __super::DefWindowProc(message, wParam, lParam);
}

void CSICommitDlg::DoSize(int delta)
{
	//RemoveAnchor(IDC_MESSAGEGROUP);
	//RemoveAnchor(IDC_LOGMESSAGE);
	//RemoveAnchor(IDC_SPLITTER);
	//RemoveAnchor(IDC_SIGNOFF);
	//RemoveAnchor(IDC_COMMIT_AMEND);
	//RemoveAnchor(IDC_COMMIT_AMENDDIFF);
	//RemoveAnchor(IDC_COMMIT_SETDATETIME);
	//RemoveAnchor(IDC_COMMIT_DATEPICKER);
	//RemoveAnchor(IDC_COMMIT_TIMEPICKER);
	//RemoveAnchor(IDC_COMMIT_SETAUTHOR);
	//RemoveAnchor(IDC_COMMIT_AUTHORDATA);
	//RemoveAnchor(IDC_LISTGROUP);
	//RemoveAnchor(IDC_FILELIST);
	//RemoveAnchor(IDC_TEXT_INFO);
	//RemoveAnchor(IDC_SELECTLABEL);
	//RemoveAnchor(IDC_CHECKALL);
	//RemoveAnchor(IDC_CHECKNONE);
	//RemoveAnchor(IDC_CHECKUNVERSIONED);
	//RemoveAnchor(IDC_CHECKVERSIONED);
	//RemoveAnchor(IDC_CHECKADDED);
	//RemoveAnchor(IDC_CHECKDELETED);
	//RemoveAnchor(IDC_CHECKMODIFIED);
	//RemoveAnchor(IDC_CHECKFILES);
	//RemoveAnchor(IDC_CHECKSUBMODULES);

	//CSplitterControl::ChangeHeight(&m_cLogMessage, delta, CW_TOPALIGN);
	//CSplitterControl::ChangeHeight(GetDlgItem(IDC_MESSAGEGROUP), delta, CW_TOPALIGN);
	//CSplitterControl::ChangeHeight(&m_ListCtrl, -delta, CW_BOTTOMALIGN);
	//CSplitterControl::ChangeHeight(GetDlgItem(IDC_LISTGROUP), -delta, CW_BOTTOMALIGN);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_SIGNOFF),0,delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_COMMIT_AMEND),0,delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_COMMIT_AMENDDIFF),0,delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_COMMIT_SETDATETIME),0,delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_COMMIT_DATEPICKER),0,delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_COMMIT_TIMEPICKER),0,delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_COMMIT_SETAUTHOR), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_COMMIT_AUTHORDATA), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_TEXT_INFO),0,delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_SELECTLABEL), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKALL), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKNONE), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKUNVERSIONED), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKVERSIONED), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKADDED), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKDELETED), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKMODIFIED), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKFILES), 0, delta);
	//CSplitterControl::ChangePos(GetDlgItem(IDC_CHECKSUBMODULES), 0, delta);

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

	//SetSplitterRange();
	//m_cLogMessage.Invalidate();
	//GetDlgItem(IDC_LOGMESSAGE)->Invalidate();
}

void CSICommitDlg::OnSize(UINT nType, int cx, int cy)
{
	//// first, let the resizing take place
	//__super::OnSize(nType, cx, cy);

	////set range
	//SetSplitterRange();
}



void CSICommitDlg::OnFocusMessage()
{
	//m_cLogMessage.SetFocus();
}

void CSICommitDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	//__super::OnMoving(fwSide, pRect);

	//if (::IsWindow(m_patchViewdlg.m_hWnd))
	//{
	//	RECT patchrect;
	//	m_patchViewdlg.GetWindowRect(&patchrect);
	//	if (::IsWindow(m_hWnd))
	//	{
	//		RECT thisrect;
	//		GetWindowRect(&thisrect);
	//		if (patchrect.left == thisrect.right)
	//		{
	//			m_patchViewdlg.SetWindowPos(NULL, patchrect.left - (thisrect.left - pRect->left), patchrect.top - (thisrect.top - pRect->top),
	//				0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
	//		}
	//	}
	//}

}

void CSICommitDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	__super::OnSizing(fwSide, pRect);

	//if(::IsWindow(this->m_patchViewdlg.m_hWnd))
	//{
	//	CRect thisrect, patchrect;
	//	this->GetWindowRect(thisrect);
	//	this->m_patchViewdlg.GetWindowRect(patchrect);
	//	if(thisrect.right==patchrect.left)
	//	{
	//		patchrect.left -= (thisrect.right - pRect->right);
	//		patchrect.right-= (thisrect.right - pRect->right);

	//		if(	patchrect.bottom == thisrect.bottom)
	//		{
	//			patchrect.bottom -= (thisrect.bottom - pRect->bottom);
	//		}
	//		if(	patchrect.top == thisrect.top)
	//		{
	//			patchrect.top -=  thisrect.top-pRect->top;
	//		}
	//		m_patchViewdlg.MoveWindow(patchrect);
	//	}
	//}
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


void CSICommitDlg::OnCbnSelchangeSubmitToCpCombobox()
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
}
