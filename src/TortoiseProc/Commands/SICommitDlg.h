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
#include "ServerConnections.h"
#include "IntegritySession.h"
#include "TGitPath.h"
#include "registry.h"
#include "Tooltip.h"

#include <regex>

/**
 * \ingroup TortoiseSIProc
 * Dialog to commit change package
 */
class CSICommitDlg : public CResizableStandAloneDialog
{
	DECLARE_DYNAMIC(CSICommitDlg)

public:
	CTGitPath     m_Path;
	CTGitPathList m_pathList;

	CSICommitDlg(CWnd* pParent = NULL); 
	virtual ~CSICommitDlg();

private:
	void SetDlgTitle();

	enum { IDD = IDD_SICOMMITDLG };

protected:
	CComboBox     m_ctrlChangePackageComboBox;

private:
	CRect         m_DlgOrigRect;
	CToolTips     m_tooltips;
	CString	      m_sWindowTitle;
	static UINT   WM_AUTOLISTREADY;
	static UINT   WM_UPDATEOKBUTTON;
	static UINT   WM_UPDATEDATAFALSE;
	HACCEL        m_hAccelerator;

	int getIntegrationPort();

protected:
	virtual void    DoDataExchange(CDataExchange* pDX);
	virtual BOOL    OnInitDialog();
	virtual BOOL    PreTranslateMessage(MSG* pMsg);
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void DoSize(int delta);
	void ClearChangePackageList();
	void UpdateChangePackageList();

public:
	afx_msg void    OnSize(UINT nType, int cx, int cy);
	afx_msg void    OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void    OnMove(int x, int y);
	afx_msg void    OnMoving(UINT fwSide, LPRECT pRect);

	afx_msg void    OnCbnSelchangeSubmitCpCombobox();

	afx_msg void    OnBnClickedCreateCpButton();
	afx_msg void    OnBnClickedSubmitCpButton();
	afx_msg void    OnBnClickedCancel();


	DECLARE_MESSAGE_MAP()
};
