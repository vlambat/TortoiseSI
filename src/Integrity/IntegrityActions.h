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
#include <exception>

// methods to execute integrity commands

namespace IntegrityActions {

	class ChangePackage {
	public:
		// Use this class to construct a valid ChangePackage
		class ChangePackageBuilder;

	private:
		// Fields of a change package, builder enforces required fields
		std::wstring m_id;
		std::wstring m_summary;
		std::wstring m_description;
		std::wstring m_cptype;
		std::wstring m_issueId;

		// Private constructor can only be used by ChangePackageBuilder
		ChangePackage( std::wstring id, std::wstring summary, std::wstring description, std::wstring cptype, std::wstring issueId ) :
			m_id(id), m_summary(summary), m_description(description), m_cptype(cptype), m_issueId(issueId) {}

	public:
		// ChangePackage specific functionality
		std::wstring getId() const { return m_id; } 
		std::wstring getSumamry() const { return m_summary; } 
		std::wstring getDescription() const { return m_description; } 
		std::wstring getType() const { return m_cptype; }
		std::wstring getIssueId() const { return m_issueId; }
	};

	class ChangePackage::ChangePackageBuilder {
	private:
		// Fields that can be initialized by the ChangePackageBuilder
		std::wstring m_id;
		std::wstring m_summary;
		std::wstring m_description;
		std::wstring m_cptype;
		std::wstring m_issueId;

	public:
		ChangePackageBuilder& setID(const std::wstring id) { m_id = id; return *this; }
		ChangePackageBuilder& setSummary(const std::wstring summary) { m_summary = summary; return *this; }
		ChangePackageBuilder& setDescription(const std::wstring description) { m_description = description; return *this; }
		ChangePackageBuilder& setType(const std::wstring cptype) { m_cptype = cptype; return *this; }
		ChangePackageBuilder& setIssueId(const std::wstring issueId) { m_issueId = issueId; return *this; }

		ChangePackage* build() {
			if (m_id.empty() && m_summary.empty()) {
				throw new std::exception("Attempt to construct Change Package that does not have valid id and sumamry fields");
			}
			else {
				return new ChangePackage(m_id, m_summary, m_description, m_cptype, m_issueId);
			}
		}
	};

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

	// list of change packages
	std::vector<std::shared_ptr<IntegrityActions::ChangePackage> *> getChangePackageList(const IntegritySession& session);

	bool connect(const IntegritySession& session);

	void launchSandboxView(const IntegritySession& session, std::wstring path);
	void launchMemberHistoryView(const IntegritySession& session, std::wstring path);
	void launchLocalChangesDiffView(const IntegritySession& session, std::wstring path);
	void launchIncomingChangesDiffView(const IntegritySession& session, std::wstring path);
	void launchAnnotatedRevisionView(const IntegritySession& session, std::wstring path);
	void launchSubmitChangesView(const IntegritySession& session, std::wstring path);
	void launchMemberInfoView(const IntegritySession& session, std::wstring path);
	void launchWorkingFileChangesView(const IntegritySession& session, std::wstring path);
	void launchChangePackageView(const IntegritySession& session);
	void launchMyChangePackageReviewsView(const IntegritySession& session);
	void launchPreferencesView(const IntegritySession& session);
	void launchIntegrityGUI(const IntegritySession& session);
	bool launchCreateCPView(const IntegritySession& session);

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

	bool submitCP(const IntegritySession &session, std::wstring cpid);

	IntegrityCommand initializeWFExecute(const IntegrityCommand& command);

}