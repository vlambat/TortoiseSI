// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2014 - TortoiseGit
// Copyright (C) 2007-2009 - TortoiseSVN

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
#include "Command.h"

#include "AboutCommand.h"
#include "SICommitCommand.h"
#include "SettingsCommand.h"
#include "HelpCommand.h"
#include "MessageBox.h"

typedef enum
{
	cmdAbout,
	cmdSICommit,
	cmdHelp,
	cmdSettings
} TortoiseSICommand;

static const struct CommandInfo
{
	TortoiseSICommand command;
	LPCTSTR pCommandName;
} commandInfo[] =
{
	{	cmdAbout,			_T("about")				},
	{   cmdSICommit,        _T("sicommit")          },
	{	cmdHelp,			_T("help")				},
	{	cmdSettings,		_T("settings")			}
};

Command * CommandServer::GetCommand(const CString& sCmd)
{
	// The default command if none is specified via /command argument
	TortoiseSICommand command = cmdAbout;

	// Look up the command
	for (int nCommand = 0; nCommand < _countof(commandInfo); ++nCommand)
	{
		if (sCmd.Compare(commandInfo[nCommand].pCommandName) == 0)
		{
			// We've found the command
			command = commandInfo[nCommand].command;

			// If this fires, you've let the enum get out of sync with the commandInfo array
			ASSERT((int)command == nCommand);

			break;
		}
	}

	switch (command)
	{
	case cmdAbout:
		return new AboutCommand;
	case cmdSICommit:
		return new SICommitCommand; 
	case cmdSettings:
		return new SettingsCommand;
	case cmdHelp:
		return new HelpCommand;
	default:
		CMessageBox::Show(theApp.m_hWndExplorer, _T("Command not implemented"), _T("TortoiseSI"), MB_ICONERROR);
		return new AboutCommand;
	}
}

void Command::SetParser(const CCmdLineParser& p) 
{ 
	parser = p; 
}

void Command::SetPaths(const CTGitPathList& plist, const CTGitPath &path)
{
	orgCmdLinePath = path;
	CString WinPath = path.GetWinPath();
	if (WinPath.Left(g_Git.m_CurrentDir.GetLength()) == g_Git.m_CurrentDir)
	{
		if (g_Git.m_CurrentDir[g_Git.m_CurrentDir.GetLength() - 1] == _T('\\'))
		{
			cmdLinePath.SetFromWin(WinPath.Right(WinPath.GetLength() - g_Git.m_CurrentDir.GetLength()));
		}
		else
		{
			cmdLinePath.SetFromWin(WinPath.Right(WinPath.GetLength() - g_Git.m_CurrentDir.GetLength() - 1));
		}
	}
	orgPathList = plist;
	for (int i = 0; i < plist.GetCount(); ++i)
	{
		WinPath = plist[i].GetWinPath();
		CTGitPath p;
		if (WinPath.Left(g_Git.m_CurrentDir.GetLength()) == g_Git.m_CurrentDir)
		{
			if (g_Git.m_CurrentDir[g_Git.m_CurrentDir.GetLength() - 1] == _T('\\'))
			{
				p.SetFromWin(WinPath.Right(WinPath.GetLength() - g_Git.m_CurrentDir.GetLength()));
			}
			else
			{
				p.SetFromWin(WinPath.Right(WinPath.GetLength() - g_Git.m_CurrentDir.GetLength() - 1));
			}
		}
		else
			p = plist[i];
		pathList.AddPath(p);

	}
}

void Command::SetExplorerHwnd(HWND hWnd) 
{ 
	hwndExplorer = hWnd; 
}