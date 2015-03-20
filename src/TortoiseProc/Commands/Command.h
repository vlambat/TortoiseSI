// TortoiseGit - a Windows shell extension for easy version control

// Copyright (C) 2008-2009,2011,2013-2014 - TortoiseGit
// Copyright (C) 2007 - TortoiseSVN

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
#include "TortoiseSIProc.h"
#include "CmdLineParser.h"
#include "TGitPath.h"
#include "Git.h"

/**
 * \ingroup TortoiseSIProc
 * Interface for all command line commands TortoiseSIProc can execute.
 */
class Command
{
public:
	/**
	 * Executes the command.
	 */
	virtual bool Execute() = 0;

	// Allow sub-classes to execute code during destruction
	virtual ~Command() {};

	void SetParser(const CCmdLineParser& p);
	void SetPaths(const CTGitPathList& plist, const CTGitPath &path);
	void SetExplorerHwnd(HWND hWnd);

protected:
	CCmdLineParser	parser;
	CTGitPathList	pathList;
	CTGitPathList	orgPathList;
	CTGitPath		cmdLinePath;
	CTGitPath		orgCmdLinePath;
	HWND			hwndExplorer;
};

/**
 * \ingroup TortoiseSIProc
 * Factory for the different commands which TortoiseProc executes from the
 * command line.
 */
class CommandServer
{
public:
	Command *GetCommand(const CString& sCmd);
};