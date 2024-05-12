/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description:  Base-class functionality for downloading files
-------------------------------------------------------------------------
History:
- 16/02/2007   : Steve Humphreys, Created
*************************************************************************/

#include "StdAfx.h"

#include "DownloadTask.h"

#include "INetworkService.h"

#include "Game.h"

static bool s_enableDLLogging = true;

CDownloadTask::CDownloadTask()
{
	m_currentDownload.destFilename.clear();
	m_currentDownload.md5 = 0;
	m_currentDownload.sourceFilename.clear();

	m_downloadAttempt = 0;
	m_maxDownloadAttempts = 1;
	m_downloadState = eDS_None;

	m_downloadFolderPath.clear();

	m_pListener = NULL;
}

CDownloadTask::~CDownloadTask()
{
	StopDownloadTask();
}

bool CDownloadTask::StartMapDownload(SFileDownloadParameters& dl, int attempts, IDownloadTaskListener* pListener)
{
	if(IsDownloadTaskInProgress())
		return false;

	if(s_enableDLLogging)
		CryLog("Starting map download from %s", dl.sourceFilename.c_str());

	// download to %USER%/levels/<download>.zip
	//	 and then extract it from there.
	GetUserDataFolder(m_downloadFolderPath);
	m_downloadFolderPath += "Levels/" + dl.destPath;
	CreateDestinationFolder(m_downloadFolderPath);
	m_currentDownload = dl;
	m_downloadList.push_back(dl);
	m_downloadAttempt = 0;
	m_maxDownloadAttempts = (attempts >= 1) ? attempts : 1;

	m_pListener = pListener;

	return true;
}

void CDownloadTask::Update()
{
	if(m_downloadState == eDS_None && m_downloadList.empty())
		return;

	INetworkService* pserv = g_pGame->GetIGameFramework()->GetISystem()->GetINetwork()->GetService("GameSpy");
	if(pserv)
	{
		IFileDownloader* pfd = pserv->GetFileDownloader();
		if(pfd && pfd->IsAvailable())
		{
			switch(m_downloadState)
			{
				default:
				case eDS_None:
					// no file downloading. Start one now.
					DownloadNextFile();
					break;

				case eDS_Downloading:
					// update listener
					if(m_pListener)
						m_pListener->OnDownloadProgress(pfd->GetDownloadProgress());

					// wait for download to finish
					if(!pfd->IsDownloading())
					{
						m_downloadState = eDS_Done;
					}
					break;

				case eDS_Done:
				{
					// validate will restart the download if it failed, and call the listeners OnDownloadFinished()
					ValidateDownload();
					m_downloadState = eDS_None;
					break;
				}
			}
		}
	}
}

void CDownloadTask::StopDownloadTask()
{
	if(s_enableDLLogging)
		CryLog("Stopping all downloads");

	// update listener then forget about it.
	if(m_pListener)
		m_pListener->OnDownloadFinished(eFDE_RequestCancelled, NULL);
	m_pListener = NULL;

	// cancel pending downloads
	INetworkService* pserv = g_pGame->GetIGameFramework()->GetISystem()->GetINetwork()->GetService("GameSpy");
	if(pserv)
	{
		IFileDownloader* pfd = pserv->GetFileDownloader();
		if(pfd && pfd->IsAvailable())
		{
			pfd->Stop();
			m_downloadState = eDS_None;
			m_downloadList.clear();
			m_currentDownload.sourceFilename.clear();
			m_downloadAttempt = 0;
		}
	}
}

bool CDownloadTask::IsDownloadTaskInProgress() const
{
	return (m_downloadState != eDS_None);
}

int CDownloadTask::GetNumberOfFilesRemaining() const
{
	return m_downloadList.size();
}

float CDownloadTask::GetCurrentFileProgress() const
{
 	INetworkService* pserv = g_pGame->GetIGameFramework()->GetISystem()->GetINetwork()->GetService("GameSpy");
 	if(pserv)
 	{
 		IFileDownloader* pfd = pserv->GetFileDownloader();
 		if(pfd && pfd->IsAvailable())
 		{
 			return pfd->GetDownloadProgress();
 		}
 	}

	return 0.0f;
}

void CDownloadTask::DownloadNextFile()
{
	INetworkService* pserv = g_pGame->GetIGameFramework()->GetISystem()->GetINetwork()->GetService("GameSpy");
	if(pserv)
	{
		IFileDownloader* pfd = pserv->GetFileDownloader();
		if(pfd && pfd->IsAvailable())
		{
			if(!m_downloadList.empty())
			{
				// first check to see if the file exists already (except the index file - always DL this)
				m_currentDownload = m_downloadList.front();
				m_downloadList.pop_front();
				m_currentDownload.destPath = m_downloadFolderPath;
				if(!FileExists(m_currentDownload.destPath + "/" + m_currentDownload.destFilename, m_currentDownload.fileSize))
				{
					if(s_enableDLLogging)
						CryLog("Downloading file: %s", m_currentDownload.destFilename.c_str());
					m_downloadAttempt = 0;
					pfd->DownloadFile(m_currentDownload);
					m_downloadState = eDS_Downloading;
				}
				else
				{
					if(s_enableDLLogging)
						CryLog("File exists (skipping): %s", m_currentDownload.destFilename.c_str());
					m_downloadState = eDS_Done;	
				}
			}
			else
			{
				// no more files. 
				m_downloadState = eDS_None;
				m_downloadList.clear();
				m_currentDownload.sourceFilename.clear();
				m_downloadAttempt = 0;
			}
		}
	}
}

int CDownloadTask::ValidateDownload()
{
	int state = eDS_Done;

	INetworkService* pserv = g_pGame->GetIGameFramework()->GetISystem()->GetINetwork()->GetService("GameSpy");
	if(pserv)
	{
		IFileDownloader* pfd = pserv->GetFileDownloader();
		if(pfd && pfd->IsAvailable())
		{
			int error = pfd->GetDownloadError();
			if(error != eFDE_NoError)
			{
				state = error;
			}
		}
	}

	// first check we have a filename to validate...
	if(state == eDS_Done && m_currentDownload.destFilename.empty())
		state = eDS_Error_FileNotFound;
	
	// check the file exists and is of the expected size
	string fileName = m_downloadFolderPath + "/" + m_currentDownload.destFilename;
	int expectedSize = m_currentDownload.fileSize;
	if(expectedSize == 0)
		expectedSize = -1;
	if(state == eDS_Done && !FileExists(fileName, expectedSize))
		state = eDS_Error_FileNotFound;

	if(state != eDS_Done)
	{
		++m_downloadAttempt;
		if(m_downloadAttempt < m_maxDownloadAttempts)
		{
			if(s_enableDLLogging)
				CryLog("Download failed, retrying: %s", m_currentDownload.destFilename.c_str());
			m_downloadList.push_front(m_currentDownload);
			m_downloadState = eDS_Downloading;
		}
		else
		{
			if(s_enableDLLogging)
				CryLog("Download failed, giving up: %s", m_currentDownload.destFilename.c_str());

			// update listener
			if(m_pListener)
				m_pListener->OnDownloadFinished(state, NULL);
		}
	}

	if(state == eDS_Done)
	{
		if(s_enableDLLogging)
			CryLog("File downloaded ok");

		if(m_pListener)
			m_pListener->OnDownloadFinished(state, fileName);
	}

	return state;
}

bool CDownloadTask::FileExists(string file, int expectedSize)
{
	// if we don't know how big a file to expect, we should always redownload it.
	if(expectedSize == 0)
		return false;

	bool ok = true;
	ICryPak* pPak = gEnv->pCryPak;
	if(pPak)
	{
		FILE* pFile = pPak->FOpen(file, "r", ICryPak::FLAGS_NO_FULL_PATH);
		if(!pFile)
			ok = false;
		else if(expectedSize != -1)	// -1 means 'don't care'
		{
			if(pPak->FGetSize(pFile) != expectedSize)
				ok = false;
		}
		pPak->FClose(pFile);
	}

	return ok;
}

bool CDownloadTask::GetUserDataFolder(string& path)
{
	// can't save into the game folder, so we must use the My Games folder instead.
	path = gEnv->pCryPak->GetAlias("%USER%");
	path += "/Downloads/";
	return true;
}

bool CDownloadTask::CreateDestinationFolder(string& folder)
{
	ICryPak* pPak = gEnv->pCryPak;
	if(pPak)
		pPak->MakeDir(folder.c_str());

	return true;
}