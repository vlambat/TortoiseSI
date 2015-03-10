// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2003-2008 - TortoiseSVN
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
#pragma once

#include "StandAloneDlg.h"
#include "registry.h"
#include "SciEdit.h"
#include "Tooltip.h"
#include "Git.h"

#include <regex>

#define ENDDIALOGTIMER	100
#define REFRESHTIMER	101
#define FILLPATCHVTIMER	102

typedef enum
{
	GIT_POSTCOMMIT_CMD_NOTHING,
	GIT_POSTCOMMIT_CMD_RECOMMIT,
	GIT_POSTCOMMIT_CMD_PUSH,
	GIT_POSTCOMMIT_CMD_DCOMMIT,
	GIT_POSTCOMMIT_CMD_PULL,
	GIT_POSTCOMMIT_CMD_CREATETAG,
} GIT_POSTCOMMIT_CMD;


/**
 * \ingroup TortoiseSIProc
 * Dialog to commit change package
 */
class CSICommitDlg : public CResizableStandAloneDialog, public CSciEditContextMenuInterface
{
	DECLARE_DYNAMIC(CSICommitDlg)

public:
	CSICommitDlg(CWnd* pParent = NULL); 
	virtual ~CSICommitDlg();

private:
	static UINT StatusThreadEntry(LPVOID pVoid);
	UINT StatusThread();
	void SetDlgTitle();

	enum { IDD = IDD_SICOMMITDLG };

protected:
	CComboBox     m_ctrlChangePackageComboBox;

private:
	CRect         m_DlgOrigRect;

	volatile LONG m_bThreadRunning;
	volatile LONG m_bRunThread;
	CToolTips     m_tooltips;
	CString	      m_sWindowTitle;
	static UINT   WM_AUTOLISTREADY;
	static UINT   WM_UPDATEOKBUTTON;
	static UINT   WM_UPDATEDATAFALSE;
	HACCEL        m_hAccelerator;

protected:
	// DDX/DDV support
	virtual void    DoDataExchange(CDataExchange* pDX);

	virtual BOOL    OnInitDialog();
	virtual void    OnOK();
	virtual void    OnCancel();
	virtual BOOL    PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	// CSciEditContextMenuInterface
	virtual void InsertMenuItems(CMenu& mPopup, int& nCmd);
	virtual bool HandleMenuItemClick(int cmd, CSciEdit * pSciEdit);

	void DoSize(int delta);
	void RunStartCommitHook();

public:
	afx_msg void    OnScnUpdateUI(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void    OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void    OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void    OnFocusMessage();
	afx_msg LRESULT OnUpdateOKButton(WPARAM, LPARAM);
	afx_msg void    OnSize(UINT nType, int cx, int cy);

	afx_msg void OnBnClickedCreateCpButton();
	afx_msg void OnBnClickedSubmitCpButton();
	afx_msg void OnCbnSelchangeSubmitToCpCombobox();
	afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnCbnSelchangeSubmitCpCombobox();
};
