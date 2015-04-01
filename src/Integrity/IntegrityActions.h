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
		std::wstring getId() { return m_id; }
		std::wstring getSumamry() { return m_summary; }
		std::wstring getDescription() { return m_description; }
		std::wstring getType() { return m_cptype; }
		std::wstring getIssueId() { return m_issueId; }
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

	class LockProperties {
	private:
		std::wstring m_name;
		std::wstring m_lockcpid;
		std::wstring m_lockType;

	public:
		std::wstring getLockName() { return m_name; }
		std::wstring getLockChangePackageId() { return m_lockcpid; }
		std::wstring getLockType() { return m_lockType; }
		LockProperties() {};
		LockProperties(std::wstring name, std::wstring lockcpid, std::wstring lockType) :
			m_name(name),
			m_lockcpid(lockcpid),
			m_lockType(lockType)
		{}

	};

	class MemberProperties {

	private:
		std::wstring m_memberRev;
		std::wstring m_sandboxName;
		std::wstring m_cpid;
		std::wstring m_workRev;
		std::wstring m_status;
		std::vector<std::shared_ptr<LockProperties>> m_lockers;

	public:

		std::wstring getMemberRev() { return m_memberRev; }
		std::wstring getSandboxName() { return m_sandboxName; }
		std::wstring getChangePackageId() { return m_cpid; }
		std::wstring getWorkingRev() { return m_workRev; }
		std::wstring getStatus() { return m_status; }
		std::vector <std::shared_ptr<LockProperties>> getLockers() { return m_lockers; };

		void setMemberRev(std::wstring memberRev) { m_memberRev = memberRev; }
		void setSandboxName(std::wstring sandboxName) { m_sandboxName = sandboxName; }
		void setChangePackageId(std::wstring cpid) { m_cpid = cpid; }
		void setWorkingRev(std::wstring workRev) { m_workRev = workRev; }
		void setLockerProperties(std::vector<std::shared_ptr<LockProperties>> lockProps) { m_lockers = lockProps; }
		void addLockerProperties(std::shared_ptr<LockProperties> lockProps) { m_lockers.push_back(lockProps); }

		MemberProperties() {};
		MemberProperties(std::wstring memberRev, std::wstring sandboxName, std::wstring cpid, std::wstring workRev, std::wstring status):
			m_memberRev(memberRev),
			m_sandboxName(sandboxName),
			m_cpid(cpid),
			m_workRev(workRev),
			m_status(status)
			{}

	};


	// get status flags for a set of files...
	FileStatusFlags fileInfo(const IntegritySession& session, const std::wstring& files);

	// get member info
	std::shared_ptr<IntegrityActions::MemberProperties> getMemberInfo(const IntegritySession& session, const std::wstring& file);

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
	std::vector<std::shared_ptr<IntegrityActions::ChangePackage>> getChangePackageList(const IntegritySession& session);

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

	IntegrityCommand initializeWFExecute(const IntegrityCommand& command);

}