
#include "StdAfx.h"

#include "RemoteControlSystem.h"
#include "GameRules.h"

#include <INetwork.h>
#include <INetworkService.h>
#include <IGameObject.h>
#include <IActorSystem.h>
#include <IItemSystem.h>
#include <IConsole.h>
#include <ILog.h>

#include "Actor.h"
#include "HUD/HUD.h"
#include "Falcon.h"

#define ConsoleLineQueueTimerDelay 0.75f

// Cl stands for client side
// Sv stands for server side

//------------------------------------------------------------------------------

CRemoteControlSystem::CRemoteControlSystem()
: 
m_nLastSeqConsoleLine(0)
, m_nLastSeqConsoleLinesReq(0)
, m_bQueueingConsoleLines(false)
, m_bQueueingConsoleLinesReq(false)
, m_fTimerQueueingConsoleLines(0.f)
, m_fTimerQueueingConsoleLinesReq(0.f)

{
	m_clCommands.clear();
	m_queueConsoleLines.clear();
	m_queueConsoleLinesReq.clear();

	RegisterClientCommand(RconClCmdName_Callback, ClCmdOnCallback);
	RegisterClientCommand(RconClCmdName_Error, ClCmdOnError);
	RegisterClientCommand(RconClCmdName_AdminMessage, ClCmdOnAdminMessage);
	RegisterClientCommand(RconClCmdName_ConsoleLine, ClCmdOnConsoleLine);
	RegisterClientCommand(RconClCmdName_ConsoleLinesRequested, ClCmdOnConsoleLinesRequested);
}

CRemoteControlSystem::~CRemoteControlSystem()
{
}

void CRemoteControlSystem::ClUpdate(float fFrameTime)
{
	if (m_bQueueingConsoleLines)
	{
		m_fTimerQueueingConsoleLines -= fFrameTime;
		if (m_fTimerQueueingConsoleLines < 1.0e-3)
		{
			m_bQueueingConsoleLines = false;
			m_fTimerQueueingConsoleLines = 0.f;
			
			FlushConsoleLinesQueued();
		}
	}
	
	if (m_bQueueingConsoleLinesReq)
	{
		m_fTimerQueueingConsoleLinesReq -= fFrameTime;
		if (m_fTimerQueueingConsoleLinesReq < 1.0e-3)
		{
			m_bQueueingConsoleLinesReq = false;
			m_fTimerQueueingConsoleLinesReq = 0.f;
			
			FlushConsoleLinesReqQueued();
		}
	}
}


void CRemoteControlSystem::ClReset()
{
	m_nLastSeqConsoleLine = 0;
	m_nLastSeqConsoleLinesReq = 0;
	m_bQueueingConsoleLines = false;
	m_bQueueingConsoleLinesReq = false;
	m_fTimerQueueingConsoleLines = 0.f;
	m_fTimerQueueingConsoleLinesReq = 0.f;
	m_queueConsoleLines.clear();
	m_queueConsoleLinesReq.clear();
}



void CRemoteControlSystem::RegisterClientCommand(const char *szName, RemoteCommandClientFunc pCallback)
{
	m_clCommands.insert(TMapClCommands::value_type(string(szName), pCallback));
}



void CRemoteControlSystem::ExecuteClientCommand(const char *szName, RemoteCommandArgs *pArgs)
{
	TMapClCommands::iterator it = m_clCommands.find(szName);
	if (it != m_clCommands.end())
	{
		it->second(pArgs);
	}
}



inline bool CRemoteControlSystem::ConvertQueryToArgs(string &query, RemoteCommandArgs *pArgs)
{
	if (query.empty())
		return false;

	std::string s(query.c_str());
	std::vector<std::string> vec;
	size_t start;
	size_t end = 0;
	const char delim = '\x0c';
	while ((start = s.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = s.find(delim, start);
		vec.push_back(s.substr(start, end - start).c_str());
	}

	int nSize = vec.size();
	if (nSize < 1)
	{
		return false;
	}

	string cmdName(vec.at(0).c_str());
	pArgs->AddArg(cmdName);

	int i;
	for (i=1; i<nSize; i++)
	{
		pArgs->AddArg(string(vec.at(i).c_str()).c_str());
	}

	return true;
}



inline bool CRemoteControlSystem::ConvertArgsToQuery(RemoteCommandArgs *pArgs, string &dest)
{
	int nCount = pArgs->GetArgCount();
	if (nCount < 1)
		return false;

	std::vector<std::string> vecQuery;
	vecQuery.push_back(std::string(pArgs->GetArg(0)));

	if (1 < nCount)
	{
		int i;
		for (i=1; i<nCount; i++)
		{
			vecQuery.push_back(std::string(pArgs->GetArg(i)));
		}
	}

	string output;
	int nSize = vecQuery.size();
	for (int i=0; i < nSize; i++)
	{
		if (i > 0) {
			output += '\x0c';
		}
		output += vecQuery.at(i).c_str();
	}

	dest.swap(output);
	return true;
}

void CRemoteControlSystem::ClSendCommand(RemoteCommandArgs &args)
{
	IActor *pActor = g_pGame->GetIGameFramework()->GetClientActor();
	if (!pActor)
		return;

	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;
	
	string query;
	if (!ConvertArgsToQuery(&args, query))
		return;
	
	EntityId src(pActor->GetEntityId());
	pGameRules->GetGameObject()->InvokeRMI(CGameRules::SvRequestChatMessage(), CGameRules::ChatMessageParams(eChatToTarget, src, 0, query.c_str(), true), eRMI_ToServer);
}



void CRemoteControlSystem::ClSendCommand(const char * szCommand)
{
	RemoteCommandArgs args(szCommand);
	ClSendCommand(args);
}



void CRemoteControlSystem::ClSendCommand(const char * szCommand, const char * szParam)
{
	RemoteCommandArgs args(szCommand);
	args.AddArg(szParam);
	ClSendCommand(args);
}



void CRemoteControlSystem::ClSendCommand(const char * szCommand, const char * szArg1, const char * szArg2)
{
	RemoteCommandArgs args(szCommand);
	args.AddArg(szArg1);
	args.AddArg(szArg2);
	ClSendCommand(args);
}

void CRemoteControlSystem::ClOnQuery(const char * szQuery)
{
	string query(szQuery);
	RemoteCommandArgs args;
	if (ConvertQueryToArgs(query, &args))
	{
		ClOnCommand(args);
	}
}

void CRemoteControlSystem::ClOnCommand(RemoteCommandArgs &args)
{
	ExecuteClientCommand(args.GetArg(0), &args);
}


// --------------------------------------------------

void CRemoteControlSystem::ClSendVersion()
{
	CryLogAlways("Sending version information...");
	ClSendCommand(RconSvCmdName_Version, g_pGameCVars->fn_version->GetString());
}

void CRemoteControlSystem::ClRequestConsoleLines(int count)
{
	string sParam;
	sParam.Format("%d", count);
	ClSendCommand(RconSvCmdName_SendConsoleLines, sParam.c_str());
}


// --------------------------------------------------

void CRemoteControlSystem::ClCmdOnCallback(RemoteCommandArgs *pArgs)
{
	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;
	
	string cmdName(pArgs->GetArg(1));
	if (cmdName == RconSvCmdName_ExecuteString)
	{
		pGameRules->OnTextMessage(eTextMessageConsole, FalconConsoleText("Command executed successfully."));
		SAFE_HUD_FUNC(BattleLogEvent(eBLE_Information, "[Falcon] Command executed."));
		return;
	}
}

void CRemoteControlSystem::ClCmdOnError(RemoteCommandArgs *pArgs)
{
	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;

	pGameRules->OnTextMessage(eTextMessageConsole, FalconConsoleText("Access Denied. You are not authorized to execute commands."));
	SAFE_HUD_FUNC(BattleLogEvent(eBLE_Warning, "[Falcon] Access Denied."));
}

void CRemoteControlSystem::ClCmdOnAdminMessage(RemoteCommandArgs *pArgs)
{
	if (pArgs->GetArgCount() < 2)
		return;
	
	string msg(pArgs->GetArg(1));
	string::iterator it, itEnd, itEscaping;
	it = msg.begin();
	itEnd = msg.end();
	itEscaping = itEnd;
	
	for (; it!=itEnd; ++it)
	{
		if (itEscaping != itEnd)
		{
			if ('0' <= *it && *it <= '9')
			{
				// number char
				*it = 0x1b;
				*itEscaping = 0x1b;
			}
			
			itEscaping = itEnd;
			continue;
		}

		if (*it == '$')
		{
			itEscaping = it;
		}
	}
	
	msg.replace("\x1b", "");
	SAFE_HUD_FUNC(BattleLogEvent(eBLE_Information, msg));
}


inline void CRemoteControlSystem::FlushConsoleLinesQueued()
{
	TMapConsoleLineQueue::iterator it, itEnd;
	it = m_queueConsoleLines.begin();
	itEnd = m_queueConsoleLines.end();
	for (; it != itEnd; ++it)
	{
		ClDisplayConsoleLine(it->second.c_str());
	}
	
	TMapConsoleLineQueue::reverse_iterator rit = m_queueConsoleLines.rbegin();
	if (rit != m_queueConsoleLines.rend())
	{
		m_nLastSeqConsoleLine = rit->first;
	}
	
	m_queueConsoleLines.clear();
}



inline void CRemoteControlSystem::FlushConsoleLinesReqQueued()
{
	TMapConsoleLineQueue::iterator it, itEnd;
	it = m_queueConsoleLinesReq.begin();
	itEnd = m_queueConsoleLinesReq.end();
	for (; it != itEnd; ++it)
	{
		ClDisplayConsoleLinesReq(it->second.c_str());
	}
	
	TMapConsoleLineQueue::reverse_iterator rit = m_queueConsoleLinesReq.rbegin();
	if (rit != m_queueConsoleLinesReq.rend())
	{
		m_nLastSeqConsoleLinesReq = rit->first;
	}
	
	m_queueConsoleLinesReq.clear();
}



void CRemoteControlSystem::ClDisplayConsoleLine(const char *szLine)
{
	CGameRules *pGameRules = g_pGame->GetGameRules();
	if (!pGameRules)
		return;

	const char * szFormat = "  %0";
	if (g_pGameCVars && g_pGameCVars->fn_rconClientConsoleLineFormat)
	{
		szFormat = g_pGameCVars->fn_rconClientConsoleLineFormat->GetString();
	}
	
	string msg(szFormat);
	msg.replace("%0", szLine);
	pGameRules->OnTextMessage(eTextMessageConsole, msg);
}



void CRemoteControlSystem::ClProcessConsoleLine(const char *szLine, int nSeq)
{
	if (m_bQueueingConsoleLines)
	{
		m_queueConsoleLines.insert(TMapConsoleLineQueue::value_type(nSeq, string(szLine)));
		return;
	}
	
	if (0 == m_nLastSeqConsoleLine || nSeq == (m_nLastSeqConsoleLine + 1))
	{
		m_nLastSeqConsoleLine = nSeq;
		ClDisplayConsoleLine(szLine);
	}
	else
	{
		m_bQueueingConsoleLines = true;
		m_nLastSeqConsoleLine = 0;
		m_fTimerQueueingConsoleLines = ConsoleLineQueueTimerDelay;
		m_queueConsoleLines.insert(TMapConsoleLineQueue::value_type(nSeq, string(szLine)));
	}
}


void CRemoteControlSystem::ClProcessConsoleLinesReq(const char * szLines, int nSeq)
{
	if (m_bQueueingConsoleLinesReq)
	{
		m_queueConsoleLinesReq.insert(TMapConsoleLineQueue::value_type(nSeq, string(szLines)));
		return;
	}
	
	if (0 == m_nLastSeqConsoleLinesReq || nSeq == (m_nLastSeqConsoleLinesReq + 1))
	{
		m_nLastSeqConsoleLinesReq = nSeq;
		ClDisplayConsoleLinesReq(szLines);
	}
	else
	{
		m_bQueueingConsoleLinesReq = true;
		m_nLastSeqConsoleLinesReq = 0;
		m_fTimerQueueingConsoleLinesReq = ConsoleLineQueueTimerDelay;
		m_queueConsoleLinesReq.insert(TMapConsoleLineQueue::value_type(nSeq, string(szLines)));
	}
}



void CRemoteControlSystem::ClDisplayConsoleLinesReq(const char * szLines)
{
	std::string s(szLines);
	size_t start;
	size_t end = 0;
	const char delim = '\n';
	while ((start = s.find_first_not_of(delim, end)) != std::string::npos)
	{
		end = s.find(delim, start);
		ClDisplayConsoleLine(s.substr(start, end - start).c_str());
	}
}



void CRemoteControlSystem::ClCmdOnConsoleLine(RemoteCommandArgs *pArgs)
{
	CRemoteControlSystem *pRcs = g_pGame->GetRemoteControlSystem();
	if (!pRcs)
		return;

	if (pArgs->GetArgCount() < 3)
		return;
	
	const char * szLine = pArgs->GetArg(1);
	int nSeq = atoi(pArgs->GetArg(2));
	if (nSeq <= 0)
		return;
	
	pRcs->ClProcessConsoleLine(szLine, nSeq);
}



void CRemoteControlSystem::ClCmdOnConsoleLinesRequested(RemoteCommandArgs *pArgs)
{
	CRemoteControlSystem *pRcs = g_pGame->GetRemoteControlSystem();
	if (!pRcs)
		return;

	if (pArgs->GetArgCount() < 3)
		return;
	
	const char * szLinesReq = pArgs->GetArg(1);
	int nSeq = atoi(pArgs->GetArg(2));
	if (nSeq <= 0)
		return;
	
	pRcs->ClProcessConsoleLinesReq(szLinesReq, nSeq);
}
