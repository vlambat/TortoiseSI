// TortoiseSI- a Windows shell extension for easy version control

// Copyright (C) 2015 - TortoiseSI
// Copyright (C) 2008-2014 - TortoiseGit
// Copyright (C) 2003-2008 - TortoiseSVN

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
#include "menuinfo.h"
#include "resource.h"
#include "Globals.h"
#include "ShellExt.h"
#include "IntegrityActions.h"
#include "StatusCache.h"

// defaults are specified in ShellCache.h

MenuInfo menuSeperator = { MenuItem::Seperator, 0, 0, 0, [](const std::vector<std::wstring>&, HWND) {}, [](const std::vector<std::wstring>&, FileStatusFlags) { return true; } };

static const IntegritySession& getIntegritySession() {
	return IStatusCache::getInstance().getIntegritySession();
}


/**
*  Takes a path to a folder as an argument and initiates an explorer refresh to refresh any icons
*  that may have changes as a result of an update of file status
*/
void refreshFolder(std::wstring folder) {
	EventLog::writeDebug(L"sending update notification for " + folder);

	SHChangeNotify(SHCNE_ATTRIBUTES, SHCNF_PATH | SHCNF_FLUSH, (LPCVOID)folder.c_str(), NULL);
}

/**
*  return true if there was a decendant path that was controlled
*/
bool warnIfPathHasControlledDecendantFolders(std::wstring path, HWND parentWindow) {
	std::vector<std::wstring> folders = IStatusCache::getInstance().getRootFolderCache().getRootFolders();
	const int maxPaths = 10;
	int nMatchingPaths = 0;

	if (path.at(path.size() - 1) != '\\') {
		path += L"\\";
	}

	std::transform(path.begin(), path.end(), path.begin(), ::tolower);

	// show the user where the sandboxes when they are decandants of the current folder since
	// it may not be obvious to the user why they can't create a sandbox here
	std::wstring message;
	for (std::wstring rootFolder : folders) {

		if (startsWith(rootFolder, path)) {

			nMatchingPaths++;

			if (message.empty()) {
				message = getTortoiseSIString(IDS_SANDBOX_NOTALLOWED1);
			}

			if (nMatchingPaths <= maxPaths) {
				message += L"\n\t '" + rootFolder + L"'";
		    }

		}
	}

	if (nMatchingPaths > maxPaths) {
		// Add message indicating we are showing only partial results
		message += L"\n\t  " + getFormattedTortoiseSIString(IDS_SHOWING_PARTIAL_RESULTS, maxPaths, nMatchingPaths);
		message += L"\n\t  " + getTortoiseSIString(IDS_ETC);
	}

	if (!message.empty()) {
		MessageBoxW(parentWindow, message.c_str(), NULL, MB_ICONERROR);
		return true;
	}
	else {
		return false;
	}
}

/**
*  Find the top level sandbox directory. Display an error if
*  nested sandboxes are found.
*/
std::wstring getTopLevelSandbox(std::wstring path, HWND parentWindow) {

	std::wstring message;
	std::wstring topLevel;

	if (path.empty())
		return topLevel;

	// Update to lower case for comparison
	std::transform(path.begin(), path.end(), path.begin(), ::tolower);

	for (std::wstring sandboxFolder : IntegrityActions::getSandboxList(getIntegritySession())) {

		// Strip off the .pj file at the end of the path before we do comparison
		std::wstring subDir = sandboxFolder.substr(0, sandboxFolder.find_last_of('\\'));

		if (startsWith(path, subDir)) {

			// Found another top level sandbox folder
			if (!topLevel.empty()) {
				if (message.empty()) {
					message = getTortoiseSIString(IDS_MULTIPLE_SANDBOXES);
					message += L"\n\t '" + topLevel + L"'";
				}
				message += L"\n\t '" + sandboxFolder + L"'";
			}
			else {
				topLevel = sandboxFolder;
			}
		}
	}

	// Display error message if one exists
	if (!message.empty()) {
		MessageBoxW(parentWindow, message.c_str(), NULL, MB_ICONERROR);
		topLevel.clear();
	}

	return topLevel;
}

std::vector<MenuInfo> menuInfo =
{
	{ MenuItem::ViewSandbox, 0, IDS_VIEW_SANDBOX, IDS_VIEW_SANDBOX,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::launchSandboxView(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return selectedItems.size() == 1 &&
			hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
			hasFileStatus(selectedItemsStatus, FileStatus::Member);
	}
	},
	{ MenuItem::CreateSandbox, 0, IDS_CREATE_SANDBOX, IDS_CREATE_SANDBOX,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
	{
		if (warnIfPathHasControlledDecendantFolders(selectedItems.front(), parentWindow)) {
			return;
		}

		IntegrityActions::createSandbox(getIntegritySession(), selectedItems.front(),
			[]{ IStatusCache::getInstance().getRootFolderCache().forceRefresh(); });

	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return selectedItems.size() == 1 &&
			hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
			!hasFileStatus(selectedItemsStatus, FileStatus::Member);
	}
	},
	menuSeperator,
	{ MenuItem::ResyncFile, 0, IDS_RESYNC, IDS_RESYNC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
		{
			std::wstring folder;
			std::wstring file;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for resync file operation");
				return;
			}

			// First selected file
			file = selectedItems.front();

			// Extract folder name from file path
			folder = file.substr(0, file.find_last_of('\\'));

			// Pass list of selections for resync operation
			IntegrityActions::resyncFiles(getIntegritySession(), selectedItems,
				[folder] { refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				!hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::ResyncEntireSandbox, 0, IDS_RESYNC_ENTIRE_SANDBOX, IDS_RESYNC_ENTIRE_SANDBOX,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring sandboxName;
			std::wstring folder;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for resync entire sandbox operation");
				return;
			}

			// Error will be displayed if multiple sandboxes found
			sandboxName = getTopLevelSandbox(selectedItems.front(), parentWindow);
			if (sandboxName.empty())
				return;

			// Extract folder name from file path
			folder = sandboxName.substr(0, sandboxName.find_last_of('\\'));

			IntegrityActions::resyncEntireSandbox(getIntegritySession(), sandboxName,
				[folder]{ refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::DropSandbox, 0, IDS_DROP_SANDBOX, IDS_DROP_SANDBOX,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring sandboxName;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for drop sandbox operation");
				return;
			}

			// Error will be displayed if multiple sandboxes found
			sandboxName = getTopLevelSandbox(selectedItems.front(), parentWindow);
			if (sandboxName.empty())
				return;

			IntegrityActions::dropSandbox(getIntegritySession(), sandboxName,
				[]{ IStatusCache::getInstance().getRootFolderCache().forceRefresh(); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::RetargetSandbox, 0, IDS_RETARGET_SANDBOX, IDS_RETARGET_SANDBOX,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring folder;
			std::wstring sandboxName;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for retarget sandbox operation");
				return;
			}

			// Error will be displayed if multiple sandboxes found
			sandboxName = getTopLevelSandbox(selectedItems.front(), parentWindow);
			if (sandboxName.empty())
				return;

			// Extract folder name from file path
			folder = sandboxName.substr(0, sandboxName.find_last_of('\\'));

			IntegrityActions::retargetSandbox(getIntegritySession(), sandboxName,
				[folder]{ refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::IgnoreSubMenu, 0, IDS_IGNORE_SUBMENU, IDS_IGNORE_SUBMENU,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for ignore operations");
				return;
			}

		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) && 
				!hasFileStatus(selectedItemsStatus, FileStatus::Ignored);
		}
	},

	{ MenuItem::Test, 0, NULL, NULL,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for ignore operations");
				return;
			}

			std::vector<std::wstring> contents = IntegrityActions::getExcludeFilterContents(getIntegritySession());

			std::wstring msg;
			for (std::wstring content : contents) {
				msg += content + L" ";
			}
			EventLog::writeDebug(msg);
			
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				!hasFileStatus(selectedItemsStatus, FileStatus::Ignored);
		}
	},

	
};
