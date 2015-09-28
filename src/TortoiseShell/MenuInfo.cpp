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
#include "CreateProcessHelper.h"
#include "PathUtils.h"

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

	IStatusCache::getInstance().clear();

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

		if (startsWith(path, rootFolder)) {

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
*  Finds registered sandboxes - possibly the top level sandbox if specified by the
*  boolean returnTopLevel argument
*/
std::wstring getSandbox(std::wstring path, HWND parentWindow, boolean returnTopLevel) {

	std::wstring message;
	std::wstring topLevel;

	if (path.empty())
		return topLevel;

	// Update to lower case for comparison
	std::transform(path.begin(), path.end(), path.begin(), ::tolower);

	for (std::wstring sandboxFolder : IntegrityActions::getSandboxList(getIntegritySession())) {

		// Strip off the .pj file at the end of the path before we do comparison
		std::wstring subDir = sandboxFolder.substr(0, sandboxFolder.find_last_of('\\'));

		if (returnTopLevel) {
			// Re-append slash
			if (subDir.at(subDir.size() - 1) != '\\') {
				subDir += L"\\";
			}
		}

		if ((returnTopLevel && startsWith(path, subDir)) || 
				(!returnTopLevel && path == subDir)) {

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

/**
*  Find the top level sandbox directory. Display an error if
*  nested sandboxes are found.
*/
std::wstring getTopLevelSandbox(std::wstring path, HWND parentWindow) {
	return getSandbox(path, parentWindow, true);
}

/**
* Find a registered sandbox regardless of whether it is a top
* level sandbox
*/
std::wstring getRegisteredSandbox(std::wstring path, HWND parentWindow) {
	return getSandbox(path, parentWindow, false);
}

/**
* Get current contents of the exclude filter, add new pattern (if it is not already in the filter), and update the exclude filter
*/
void updateExcludeFileFilter(const std::vector<std::wstring>& selectedItems, const std::wstring newExclude) {
	// assume newExclude of the form "filename.extension" or "*.extension" 
	if (selectedItems.empty()) {
		EventLog::writeDebug(L"selected items list empty for ignore operations");
		return;
	}

	std::wstring file = selectedItems.front();

	// Extract folder name from file path
	std::wstring folder = file.substr(0, file.find_last_of('\\'));

	// retrieve current exclude contents
	std::vector<std::wstring> excludeContents = IntegrityActions::getExcludeFilterContents(getIntegritySession());

	// append it to this list
	std::wstring newExcludeOption = L"file:" + newExclude;

	//Remove elements from the exclude Filter	
	std::vector<std::wstring>::iterator it;
	it = std::find(excludeContents.begin(), excludeContents.end(), newExcludeOption);
	if (it != excludeContents.end()) 
		excludeContents.erase(it);
	//Adding elements into the Exclude Filter
	else
		excludeContents.push_back(newExcludeOption);

	// set prefs and refresh folder
	IntegrityActions::setExcludeFileFilter(getIntegritySession(), excludeContents, [folder] { refreshFolder(folder); });
}

std::vector<MenuInfo> menuInfo =
{
	menuSeperator,
	{ MenuItem::ViewSandbox, IDI_REPOBROWSE, IDS_VIEW_SANDBOX, IDS_VIEW_SANDBOX_DESC,
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
	{ MenuItem::CreateSandbox, IDI_CREATEREPOS, IDS_CREATE_SANDBOX, IDS_CREATE_SANDBOX_DESC,
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

	{ MenuItem::ViewRelatedChangePackages, IDI_REPOBROWSE, IDS_VIEW_RELATEDCP, IDS_VIEW_RELATEDCP_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::viewRelatedChangePackages(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return selectedItems.size() == 1 &&
			hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
			hasFileStatus(selectedItemsStatus, FileStatus::Member);
	}
	},

	{ MenuItem::ViewMyProjectHistory, IDI_REVISIONGRAPH, IDS_VIEW_PROJECTHISTORY, IDS_VIEW_PROJECTHISTORY_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::viewMyProjectHistory(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return selectedItems.size() == 1 &&
			hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
			hasFileStatus(selectedItemsStatus, FileStatus::Member);
	}
	},


	{ MenuItem::ViewMyProjectDifferences, IDI_REPOBROWSE, IDS_VIEW_PROJECTDIFFERENCES, IDS_VIEW_PROJECTDIFFERENCES_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::viewMyProjectDifferences(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return selectedItems.size() == 1 &&
			hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
			hasFileStatus(selectedItemsStatus, FileStatus::Member);
	}
	},

	menuSeperator,
	{ MenuItem::ViewMyChangePackages, IDI_REPOBROWSE, IDS_VIEW_CHANGEPACKAGES, IDS_VIEW_CHANGEPACKAGES_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::viewMyChangePackages(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return true;
	}
	},	

	{ MenuItem::ViewMyReviews, IDI_REPOBROWSE, IDS_VIEW_MYREVIEWS, IDS_VIEW_MYREVIEWS_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::viewMyReviews(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return true;
	}
	},

	{ MenuItem::ViewMyLocks, IDI_REPOBROWSE, IDS_VIEW_LOCKS, IDS_VIEW_LOCKS_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::viewMyLocks(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return true;
	}
	},

	{ MenuItem::MergeConflicts, IDI_MERGE, IDS_MERGE_CONFLICTS, IDS_MERGE_CONFLICTS_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::mergeConflicts(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return selectedItems.size() == 1 &&
			hasFileStatus(selectedItemsStatus, FileStatus::File) &&
			hasFileStatus(selectedItemsStatus, FileStatus::Member) &&
			hasFileStatus(selectedItemsStatus, FileStatus::MergeNeeded);
	}
	},

	{ MenuItem::WorkingFileChangesView, IDI_SHOWCHANGED, IDS_VIEW_WORKINGFILECHANGES, IDS_VIEW_WORKINGFILECHANGES_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND)
	{
		IntegrityActions::launchWorkingFileChangesView(getIntegritySession(), selectedItems.front());
	},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
	{
		return selectedItems.size() == 1 &&
			hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
			hasFileStatus(selectedItemsStatus, FileStatus::Member);
	}
	},

	menuSeperator,
	{ MenuItem::Resync, IDI_PULL, IDS_RESYNC, IDS_RESYNC_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring sandboxName;
			std::wstring folder;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for resync operation");
				return;
			}

			// Error will be displayed if multiple sandboxes found
			sandboxName = getRegisteredSandbox(selectedItems.front(), parentWindow);
			if (sandboxName.empty())
				return;

			// Extract folder name from file path
			folder = sandboxName.substr(0, sandboxName.find_last_of('\\'));

			IntegrityActions::resyncSandbox(getIntegritySession(), sandboxName,
				[folder]{ refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::Resync, IDI_PULL, IDS_RESYNC, IDS_RESYNC_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring sandboxName;
			std::wstring file;
			std::wstring folder;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for resync operation");
				return;
			}

			// Retrieve selection
			file = selectedItems.front();

			// Extract folder name from file path
			folder = file.substr(0, file.find_last_of('\\'));

			IntegrityActions::resync(getIntegritySession(), selectedItems.front(),
				[folder]{ refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::ResyncByCp, IDI_PULL, IDS_RESYNC_BYCP, IDS_RESYNC_BYCP_DESC,
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
			IntegrityActions::resyncByCp(getIntegritySession(), selectedItems,
				[folder] { refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				!hasFileStatus(selectedItemsStatus, FileStatus::Folder) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::DropSandbox, IDI_DELETE, IDS_DROP_SANDBOX, IDS_DROP_SANDBOX_DESC,
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
	{ MenuItem::RetargetSandbox, IDI_RELOCATE, IDS_RETARGET_SANDBOX, IDS_RETARGET_SANDBOX_DESC,
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
	menuSeperator,
	/*{ MenuItem::SubmitCP, IDI_COMMIT, IDS_SUBMIT_CP, IDS_SUBMIT_CP_DESC,
	    [](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring tortoiseSIProcPath = CPathUtils::GetAppDirectory(g_hmodThisDll) + L"TortoiseSIProc.exe";

			// Leading space is required
			std::wstring cmd = L" /command:sicommit /hwnd:";
			TCHAR buf[30] = { 0 };
			_stprintf_s(buf, _T("%p"), parentWindow);
			cmd += buf;

			if (CCreateProcessHelper::CreateProcessDetached(const_cast<TCHAR*>(tortoiseSIProcPath.c_str()), const_cast<TCHAR*>(cmd.c_str()))) {
				// Process started
				return;
			}

			MessageBox(parentWindow, CFormatMessageWrapper(), L"TortoiseSIProc launch failed", MB_OK | MB_ICONINFORMATION);
		},
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return true;
		}
	},*/
	menuSeperator,
	{ MenuItem::LocalChangesDiff, IDI_DIFF, IDS_LOCAL_CHANGES_DIFF, IDS_LOCAL_CHANGES_DIFF_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;

			// First selected file
			file = selectedItems.front();

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for diff operation");
				return;
			}

			IntegrityActions::launchLocalChangesDiffView(getIntegritySession(), file);

		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	menuSeperator,
	{ MenuItem::ViewHistory, IDI_REVISIONGRAPH, IDS_VIEW_HISTORY, IDS_VIEW_HISTORY_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;

			// First selected file
			file = selectedItems.front();

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for view history operation");
				return;
			}

			IntegrityActions::launchMemberHistoryView(getIntegritySession(), file);

		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::ViewAnnotatedRevision, IDI_BLAME, IDS_VIEW_ANNOTATED_REVISION, IDS_VIEW_ANNOTATED_REVISION_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;

			// First selected file
			file = selectedItems.front();

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for view annotated revision operation");
				return;
			}

			IntegrityActions::launchAnnotatedRevisionView(getIntegritySession(), file);

		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::ViewMemberInfo, IDI_PROPERTIES, IDS_VIEW_MEMBER_INFO, IDS_VIEW_MEMBER_INFO_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;

			// First selected file
			file = selectedItems.front();

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for view member info operation");
				return;
			}

			IntegrityActions::launchMemberInfoView(getIntegritySession(), file);

		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	menuSeperator,
	{ MenuItem::Add, IDI_ADD, IDS_ADD_NONMEMBER, IDS_ADD_NONMEMBER_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;
			std::wstring sandboxName;
			
			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for add operation");
				return;
			}
			
			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			sandboxName = IntegrityActions::getSandboxName(getIntegritySession(), folder);

			IntegrityActions::addFiles(getIntegritySession(), sandboxName, selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Add);
		}
	},
	{ MenuItem::CheckOut, IDI_CHECKOUT, IDS_CHECKOUT, IDS_CHECKOUT_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;
			std::wstring sandboxName;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for checkout operation");
				return;
			}

			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			sandboxName = IntegrityActions::getSandboxName(getIntegritySession(), folder);

			IntegrityActions::checkoutFiles(getIntegritySession(), sandboxName, selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::CheckIn, IDI_IMPORT, IDS_CHECKIN, IDS_CHECKIN_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;
			std::wstring sandboxName;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for checkin operation");
				return;
			}

			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			sandboxName = IntegrityActions::getSandboxName(getIntegritySession(), folder);

			IntegrityActions::checkinFiles(getIntegritySession(), sandboxName, selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::Drop, IDI_DELETE, IDS_DROP, IDS_DROP_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;
			std::wstring sandboxName;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for drop operation");
				return;
			}

			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			sandboxName = IntegrityActions::getSandboxName(getIntegritySession(), folder);

			IntegrityActions::dropPaths(getIntegritySession(), sandboxName, selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::Lock, IDI_LOCK, IDS_LOCK, IDS_LOCK_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for lock operation");
				return;
			}

			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			IntegrityActions::lockFiles(getIntegritySession(), selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::Revert, IDI_REVERT, IDS_REVERT, IDS_REVERT_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for revert operation");
				return;
			}

			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			IntegrityActions::revertFiles(getIntegritySession(), selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::Move, IDI_RELOCATE, IDS_MOVE, IDS_MOVE_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for move operation");
				return;
			}

			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			IntegrityActions::moveFiles(getIntegritySession(), selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	{ MenuItem::Rename, IDI_RENAME, IDS_RENAME, IDS_RENAME_DESC,
	[](const std::vector<std::wstring>& selectedItems, HWND parentWindow)
		{
			std::wstring file;
			std::wstring folder;

			if (selectedItems.empty()) {
				EventLog::writeDebug(L"selected items list empty for rename operation");
				return;
			}

			file = selectedItems.front();

			folder = file.substr(0, file.find_last_of('\\'));

			IntegrityActions::renameFiles(getIntegritySession(), selectedItems,
				[folder] {refreshFolder(folder); });
		},
			[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Member);
		}
	},
	menuSeperator,
	{ MenuItem::IgnoreSubMenu, 0, IDS_IGNORE_SUBMENU, IDS_IGNORE_SUBMENU_DESC,
		nullptr, // CShellExt::InsertIgnoreSubmenus define the actions associated with menu
		[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) && 
				!hasFileStatus(selectedItemsStatus, FileStatus::Ignored);
		}
	},

	{ MenuItem::UnIgnoreSubMenu, 0, IDS_REMOVE_SUBMENU, IDS_REMOVE_SUBMENU_DESC,
	nullptr, // CShellExt::InsertIgnoreSubmenus define the actions associated with menu
	[](const std::vector<std::wstring>& selectedItems, FileStatusFlags selectedItemsStatus)
		{
			return selectedItems.size() == 1 &&
				hasFileStatus(selectedItemsStatus, FileStatus::File) &&
				hasFileStatus(selectedItemsStatus, FileStatus::Ignored);
		}
	},
};
