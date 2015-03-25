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
#include "IntegritySession.h"
#include "IntegrityResponse.h"

IntegritySession::IntegritySession()
{
	ip = NULL;
	session = NULL;

	initAPI();

	mksCreateLocalAPIConnector(&ip, 4, 15, TRUE);
	if (ip != NULL) {
		mksGetCommonSession(&session, ip);
	}
}

IntegritySession::IntegritySession(std::string hostname, int port) 
{
	ip = NULL;
	session = NULL;

	initAPI();

	mksCreateAPIConnector(&ip, hostname.c_str(), port, FALSE, 4, 15);
	if (ip != NULL) {
		mksGetCommonSession(&session, ip);
	}
}

void IntegritySession::initAPI()
{
	mksAPIInitialize(NULL); // TODO allow API loging
	//mksLogConfigure(MKS_LOG_ERROR, MKS_LOG_MEDIUM);
}

IntegritySession::~IntegritySession()
{
	if (ip != NULL) {
		mksReleaseIntegrationPoint(ip);
		mksAPITerminate();

		ip = NULL;
		session = NULL;
	}
}

std::wstring IntegrityCommand::Option::getAsString() {
	std::wstring optionString = L"";
	if (name.size() == 1) {
		optionString = L"-" + name;
		if (value.size() > 0) {
			optionString += L" " + value;
		}
	} else {
		optionString = L"--" + name;
		if (value.size() > 0) {
			optionString += L"=" + value;
		}
	}
	return optionString;
}

class NativeIntegrityCommand {
public:
	NativeIntegrityCommand(const IntegrityCommand& command) {
		nativeCommand = mksCreateCommand();
		if (nativeCommand != NULL) {
			nativeCommand->appName = command.getApp();
			nativeCommand->cmdName = command.getName();

			for (IntegrityCommand::Option option : command.options) {
				if (option.value.size() > 0) {
					mksOptionListAdd(nativeCommand->optionList, option.name.c_str(), option.value.c_str());
				} else {
					mksOptionListAdd(nativeCommand->optionList, option.name.c_str(), NULL);
				}
			}

			for (std::wstring selectionItem : command.selection) {
				mksSelectionListAdd(nativeCommand->selectionList, selectionItem.c_str());
			}
		}
	};
	~NativeIntegrityCommand() {
		if (nativeCommand != NULL) {
			mksReleaseCommand(nativeCommand);
		}
	};

	mksCommand nativeCommand;
};

std::unique_ptr<IntegrityResponse> IntegritySession::execute(const IntegrityCommand& command) const
{
	if (session != NULL) {
		mksCmdRunner commandRunner;
		NativeIntegrityCommand nativeIntegrityCommand(command);

		mksCreateCmdRunner(&commandRunner, session);

		if (commandRunner != NULL) {
			mksResponse response = mksCmdRunnerExecCmd(commandRunner, nativeIntegrityCommand.nativeCommand, NO_INTERIM);

			std::unique_ptr<IntegrityResponse> responseWrapper 
				= std::unique_ptr<IntegrityResponse>(new IntegrityResponse(commandRunner, response, command));

			if (responseWrapper->getException() != NULL) {
				for (auto errorHandler : errorHandlers) {
					errorHandler(responseWrapper->getException());
				}
			}

			return responseWrapper;
		}
	}
	return NULL;
};