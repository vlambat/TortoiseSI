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

#include "stdafx.h"
#include "IntegrityActions.h"
#include "IntegrityResponse.h"
#include "EventLog.h"
#include "CrashReport.h"
#include "DebugEventLog.h"
#include <sstream>
#include <chrono>
#include <future>

namespace IntegrityActions {
	void displayException(const IntegrityResponse& response);
	void logAnyExceptions(const IntegrityResponse& response);
	void executeUserCommand(const IntegritySession& session, const IntegrityCommand& command, std::function<void()>);

	void launchSandboxView(const IntegritySession& session, std::wstring path)
	{
		IntegrityCommand command(L"si", L"viewsandbox");
		command.addOption(L"cwd", path);
		command.addOption(L"g");

		executeUserCommand(session, command, nullptr);
	}

	void createSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone)
	{
		IntegrityCommand command(L"si", L"createsandbox");
		command.addOption(L"g");
		command.addSelection(path);

		executeUserCommand(session, command, onDone);
	}

	void resyncFiles(const IntegritySession& session, std::vector<std::wstring> paths, std::function<void()> onDone)
	{
		IntegrityCommand command(L"si", L"resync");

		command.addOption(L"g");
		command.addOption(L"byCP");

		for (std::wstring path : paths) {
			command.addSelection(path);
		}

		// Issue found with wf execute command accepting options - don't use it for now
		// executeUserCommand(session, initializeWFExecute(command), onDone);
		executeUserCommand(session, command, onDone);
	}

	void resyncEntireSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone)
	{
		IntegrityCommand command(L"si", L"resync");
		command.addOption(L"g");
		command.addOption(L"S", path);

		executeUserCommand(session, command, onDone);
	}

	void dropSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone)
	{
		IntegrityCommand command(L"si", L"dropsandbox");
		command.addOption(L"g");
		command.addSelection(path);

		executeUserCommand(session, command, onDone);
	}

	void retargetSandbox(const IntegritySession& session, std::wstring path, std::function<void()> onDone)
	{
		IntegrityCommand command(L"si", L"retargetsandbox");
		command.addOption(L"g");
		command.addSelection(path);

		executeUserCommand(session, command, onDone);
	}

	bool submitCP(const IntegritySession &session, std::wstring cpid)
	{
		IntegrityCommand command(L"si", L"submitcp");
		command.addOption(L"confirmcloseCP"); 
		command.addOption(L"confirmdeferredIsError");
		command.addOption(L"confirmignoreNoDeferred");
		command.addOption(L"g");
		command.addSelection(cpid);

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			return false;
		}

		if (response->getExitCode() != 0) {
			return false;
		}

		return true;
	}

	void setExcludeFileFilter(const IntegritySession& session, std::vector<std::wstring> patterns, std::function<void()> onDone)
	{
		IntegrityCommand command(L"si", L"setprefs");
		command.addOption(L"command",L"viewnonmembers");

		std::wstring patternString;
		if (!patterns.empty()) {
			std::wostringstream ss;

			// build list of filter patterns, separating with commas. Add last one manually so we don't have trailing comma
			std::copy(patterns.begin(), patterns.end() - 1, std::ostream_iterator<std::wstring, wchar_t>(ss, L","));
			ss << patterns.back();

			patternString = ss.str();
		}

		command.addSelection(L"excludeFilter=" + patternString);

		executeUserCommand(session, command, onDone);
	}

	const FileStatusFlags NO_STATUS = 0;

	// get status flags for a set of files...
	FileStatusFlags fileInfo(const IntegritySession& session, const std::wstring& file) 
	{
		IntegrityCommand command(L"wf", L"fileinfo");
		command.addSelection(file);

		std::future<std::unique_ptr<IntegrityResponse>> responseFuture =
			std::async(std::launch::async, [command, &session]() { return session.execute(command); });

		std::future_status status = responseFuture.wait_for(std::chrono::seconds(10));
		if (status != std::future_status::ready) {
			EventLog::writeWarning(L"get status timeout for " + file);

			// need to call get so that the unique_ptr will exisit somewhere and go out of 
			// scope and delete the command runner / response
			std::async(std::launch::async, [&]{ 
					std::unique_ptr<IntegrityResponse> response = responseFuture.get(); 
				});

			return (FileStatusFlags)FileStatus::TimeoutError;
		}

		// TODO check future's exception... ?
		std::unique_ptr<IntegrityResponse> response = responseFuture.get();
		if (response == NULL) {
			EventLog::writeWarning(L"get status failed to return response");
			return NO_STATUS;
		}

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			return NO_STATUS;
		} 

		if (response->getExitCode() != 0) { 
			EventLog::writeWarning(L"get status has non zero exit code");
		}

		for (mksWorkItem item : *response) {
			int status = getIntegerFieldValue(item, L"status", NO_STATUS);

			EventLog::writeInformation(std::wstring(L"wf fileInfo ") + file + L" has status " +  std::to_wstring(status));
			return status;
		}
		return NO_STATUS;
	}


	/**
	*  Retrieve the name of the sandbox using the 'sandboxinfo' command
	*/
	std::wstring getSandboxName(const IntegritySession& session, std::wstring path) {

		std::wstring sandboxName;

		IntegrityCommand command(L"si", L"sandboxinfo");
		command.addOption(L"cwd", path);
		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			displayException(*response);
			return sandboxName;
		}

		for (mksWorkItem item : *response) {
			return getId(item);
		}

		return sandboxName;
	}
	

	/**
	*  Retrieve a list of all sandboxes using the 'sandboxes' command
	*/
	std::vector<std::wstring> getSandboxList(const IntegritySession& session) {

		std::vector<std::wstring> sandboxPaths;
		IntegrityCommand command(L"si", L"sandboxes");

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			displayException(*response);
			return sandboxPaths;
		}

		for (mksWorkItem item : *response) {

			// Retrieve sandbox name - sandbox directory including
			// associated .pj file
			std::wstring id = getId(item);

			// Convert all to lowercase
			std::transform(id.begin(), id.end(), id.begin(), ::tolower);

			sandboxPaths.push_back(id);
		}

		return sandboxPaths;
	}

	/**
	*  Retrieve current excludeFilter contents using 'viewprefs' command
	*/
	std::vector<std::wstring> getExcludeFilterContents(const IntegritySession& session) {
		
		std::vector<std::wstring> filterContents;
		IntegrityCommand command(L"si", L"viewprefs");
		command.addOption(L"command", L"viewnonmembers");

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			displayException(*response);
			return filterContents;
		}


		std::wstring contents;
		for (mksWorkItem item : *response) {
			std::wstring id = getId(item);

			mksItem setting = getItemFieldValue(item, L"excludeFilter");
			if (setting != NULL) {
				contents = getStringFieldValue(setting, L"default");
			}

		}

		if (!contents.empty()) {
			wchar_t* token = wcstok((wchar_t*)contents.c_str(), L",");
			while (token != NULL) {
				filterContents.push_back(token);
				token = wcstok(NULL, L",");
			}
		}

		return filterContents;


	}

	/**
	 * Retrieve list of change packages
	 */
	std::vector<std::shared_ptr<IntegrityActions::ChangePackage> *> getChangePackageList(const IntegritySession& session) {
		std::vector<std::shared_ptr<IntegrityActions::ChangePackage> *> cps;
		IntegrityCommand command(L"si", L"viewcps");
		command.addOption(L"filter", L"state:open");
		command.addOption(L"fields", L"id,summary,description,cptype,issue");

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			displayException(*response);
			return cps;
		}

		for (mksWorkItem item : *response) {
			std::wstring id = getId(item);
			std::wstring summary = getStringFieldValue(item, L"summary");
			std::wstring description = getStringFieldValue(item, L"description");
			std::wstring cptype = getStringFieldValue(item, L"cptype");
			std::wstring issueId = getStringFieldValue(item, L"issue");

			// Create change package object
			std::shared_ptr<IntegrityActions::ChangePackage> *cp = new std::shared_ptr<IntegrityActions::ChangePackage>(IntegrityActions::ChangePackage::ChangePackageBuilder()
				.setID(id)
				.setSummary(summary)
				.setDescription(description)
				.setType(cptype)
				.setIssueId(issueId)
				.build());

			cps.push_back(cp);
		}

		return cps;
	}

	/**
	 * Launch Create Change Package View
	 */
	bool launchCreateCPView(const IntegritySession& session) {
		IntegrityCommand command(L"si", L"createcp");
		command.addOption(L"g");

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			return false;
		}

		if (response->getExitCode() != 0) {
			return false;
		}

		return true;
	}

	/**
	*  Launch Member History View
	*/
	void launchMemberHistoryView(const IntegritySession& session, std::wstring path) {
		IntegrityCommand command(L"si", L"viewhistory");
		command.addOption(L"g");
		command.addSelection(path);

		IntegrityCommand wfCommand = initializeWFExecute(command);

		executeUserCommand(session, wfCommand, nullptr);
	}

	/**
	*  Launch Annotated Revision View
	*/
	void launchAnnotatedRevisionView(const IntegritySession& session, std::wstring path) {
		IntegrityCommand command(L"si", L"annotate");
		command.addOption(L"g");
		command.addSelection(path);

		IntegrityCommand wfCommand = initializeWFExecute(command);

		executeUserCommand(session, wfCommand, nullptr);
	}

	/**
	*  Launch Member Information View
	*/
	void launchMemberInfoView(const IntegritySession& session, std::wstring path) {
		IntegrityCommand command(L"si", L"memberinfo");
		command.addOption(L"g");
		command.addSelection(path);

		IntegrityCommand wfCommand = initializeWFExecute(command);

		executeUserCommand(session, wfCommand, nullptr);
	}

	/**
	*  Launch Differences View on local member
	*/
	void launchLocalChangesDiffView(const IntegritySession& session, std::wstring path) {
		IntegrityCommand command(L"si", L"diff");
		command.addOption(L"g");
		command.addSelection(path);

		IntegrityCommand wfCommand = initializeWFExecute(command);

		executeUserCommand(session, wfCommand, nullptr);
	}

	std::vector<std::wstring> folders(const IntegritySession& session)
	{
		IntegrityCommand command(L"wf", L"folders");
		std::vector<std::wstring> rootPaths;

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			return rootPaths;
		}

		// Retrieve the id of the returned sandbox
		for (mksWorkItem item : *response) {
			rootPaths.push_back(getId(item));
		}

		return rootPaths;
	}

	std::vector<std::wstring> servers(const IntegritySession& session)
	{
		IntegrityCommand command(L"si", L"servers");
		std::vector<std::wstring> servers;

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			return servers;
		}

		for (mksWorkItem item : *response) {
			servers.push_back(getId(item));
		}

		return servers;
	}

	bool connect(const IntegritySession& session) 
	{
		IntegrityCommand command(L"si", L"connect");
		command.addOption(L"g");

		std::unique_ptr<IntegrityResponse> response = session.execute(command);

		if (response->getException() != NULL) {
			logAnyExceptions(*response);
			return false;
		}

		if (response->getExitCode() != 0) {
			return false;
		}

		return true;
	}

	void logAnyExceptions(const IntegrityResponse& response) {
		if (response.getException() != NULL) {
			std::wstring message = std::wstring(L"Error encountered running command '")
				+ response.getCommand().getApp() + L" " + response.getCommand().getName()
				+ L"': \n\terror id = "
				+ getId(response.getException()) + L"\n\t message = '"
				+ getExceptionMessage(response.getException()) + L"'";

			for (mksWorkItem item : response) {
				mksAPIException workItemException = mksWorkItemGetAPIException(item);

				if (workItemException != NULL) {
					message = std::wstring(L"\nError processing item '")
						+ getId(item) + L"' with type '" + getModelType(item)
						+ L"': \n\terror id = "
						+ getId(response.getException()) + L"\n\t message = '"
						+ getExceptionMessage(response.getException()) + L"'";
				}
			}

			EventLog::writeError(message);
		}
	}

	void displayException(const IntegrityCommand& command, std::wstring errorId, std::wstring msg) {
		std::wstring message = std::wstring(L"Error encountered running command '")  
			+ command.getApp() + L" " + command.getName()
			+ L"': \n\terror id = " + errorId + L"\n\t message = '" + msg + L"'";

		MessageBoxW(NULL, message.c_str(), L"Error", MB_OK | MB_ICONEXCLAMATION);
	}

	void executeUserCommand(const IntegritySession& session, const IntegrityCommand& command, std::function<void()> onDone) {
		std::async(std::launch::async, [&](IntegrityCommand command, std::function<void()> onDone){
			try {
				std::unique_ptr<IntegrityResponse> response = session.execute(command);

				logAnyExceptions(*response);

				if (onDone != nullptr) {
					onDone();
				}
			}
			catch (std::exception)
			{
			}
		}, command, onDone);
	}

	void displayException(const IntegrityResponse& response) {
		mksAPIException exception = response.getException();
		if (exception != NULL) {
			for (mksWorkItem item : response) {
				mksAPIException workItemException = mksWorkItemGetAPIException(item);

				if (workItemException != NULL) {
					displayException(response.getCommand(), getId(workItemException),
						getExceptionMessage(workItemException));
					return;
				}
			}
			
			displayException(response.getCommand(), getId(exception),
				getExceptionMessage(exception));
		}
	}

	IntegrityCommand initializeWFExecute(const IntegrityCommand& command) {
		IntegrityCommand wfcommand(L"wf", L"execute");

		wfcommand.addSelection(command.getName());

		for (IntegrityCommand::Option option : command.options) {
			wfcommand.addSelection(option.getAsString());
		}

		// explicitly mark end of options and start of selection 
		wfcommand.addSelection(L"--");

		for (std::wstring selectionItem : command.selection) {
			wfcommand.addSelection(selectionItem);
		}
		return wfcommand;
	}
}