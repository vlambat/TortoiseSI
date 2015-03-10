// TortoiseSI - a Windows shell extension for easy version control

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

#pragma once

#include "FileStatus.h"
#include "IntegritySession.h"

// methods to execute integrity commands

namespace IntegrityActions {
	// get status flags for a set of files...
	FileStatusFlags fileInfo(const IntegritySession& session, const std::wstring& files);

	// list of controlled folders (ie with a sandbox) 
	std::vector<std::wstring> folders(const IntegritySession& session);

	// list of server connections the client has
	std::vector<std::wstring> servers(const IntegritySession& session);

	// list of all sandboxes
	std::vector<std::wstring> getSandboxList(const IntegritySession& session);

	// name of specific sandbox (includes .pj file)
	std::wstring getSandboxName(const IntegritySession& session, std::wstring path);

	// list of patterns in exclude filter
	std::vector<std::wstring> getExcludeFilterContents(const IntegritySession& session);

	bool connect(const IntegritySession& session);

	void launchSandboxView(const IntegritySession& session, std::wstring path);
	void launchMemberHistoryView(const IntegritySession& session, std::wstring path);
	void launchLocalChangesDiffView(const IntegritySession& session, std::wstring path);
	void launchIncomingChangesDiffView(const IntegritySession& session, std::wstring path);
	void launchAnnotatedRevisionView(const IntegritySession& session, std::wstring path);
	void launchSubmitChangesView(const IntegritySession& session, std::wstring path);
	void launchMemberInfoView(const IntegritySession& session, std::wstring path);
	void launchChangePackageView(const IntegritySession& session);
	void launchMyChangePackageReviewsView(const IntegritySession& session);
	void launchPreferencesView(const IntegritySession& session);
	void launchIntegrityGUI(const IntegritySession& session);

	void lockFiles(const IntegritySession& session, std::vector<std::wstring> paths, std::function<void()> onDone);
	void unlockFiles(const IntegritySession& session, std::vector<std::wstring> paths, std::function<void()> onDone);
	void addFiles(const IntegritySession& session, std::wstring sandbox, std::vector<std::wstring> paths, std::function<void()> onDone);
	void dropPaths(const IntegritySession& session, std::wstring sandbox, std::vector<std::wstring> paths, std::function<void()> onDone);
	void moveFiles(const IntegritySession& session, std::vector<std::wstring> paths, std::function<void()> onDone);
	void renameFiles(const IntegritySession& session, std::vector<std::wstring> paths, std::function<void()> onDone);
	void revertFiles(const IntegritySession& session, std::vector<std::wstring> paths, std::function<void()> onDone);
	void checkoutFiles(const IntegritySession& session, std::wstring sandbox, std::vector<std::wstring> paths, std::function<void()> onDone);
	void checkinFiles(const IntegritySession& session, std::wstring sandbox, std::vector<std::wstring> paths, std::function<void()> onDone);

	void createSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone);
	void dropSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone);
	void resyncFiles(const IntegritySession& session, std::vector<std::wstring> paths, std::function<void()> onDone);
	void resyncEntireSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone);
	void retargetSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone);

	void setExcludeFileFilter(const IntegritySession& session, std::vector<std::wstring> patterns, std::function<void()> onDone);

	IntegrityCommand initializeWFExecute(const IntegrityCommand& command);

}