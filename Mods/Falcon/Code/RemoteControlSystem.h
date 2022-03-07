

#pragma once

#include <deque>

struct ICVar;

class CRemoteControlSystem;

#define RconClCmdName_Callback "Clbk"
#define RconClCmdName_Error "Err"
#define RconClCmdName_ConsoleLine "CL"
#define RconClCmdName_AdminMessage "AM"
#define RconClCmdName_ConsoleLinesRequested "CLsReq"

#define RconSvCmdName_ExecuteString "ExecStr"
#define RconSvCmdName_SendConsoleLines "ReqCL"

#define RconSvCmdName_Version "VERSION"

struct RemoteCommandArgs
{
public:
	RemoteCommandArgs() 
	{
		m_args.clear();
	}
	
	RemoteCommandArgs(const char * szName)
	{
		m_args.clear();
		AddArg(szName);
	}
	
	void AddArg(const char * szParam)
	{
		m_args.push_back(string(szParam));
	}

	void SetArg(int idx, const char *szParam)
	{
		m_args.insert(m_args.begin() + idx, string(szParam));
	}
	
	const char * GetArg(int idx)
	{
		return m_args.at(idx).c_str();
	}
	
	int GetArgCount()
	{
		return m_args.size();
	}
	
private:
	std::deque<string> m_args;
};


typedef void (*RemoteCommandClientFunc)(RemoteCommandArgs *pArgs);
typedef void (*RemoteCommandServerFunc)(int channelId, RemoteCommandArgs *pArgs);


class CRemoteControlSystem
{
public:
	CRemoteControlSystem();
	~CRemoteControlSystem();

	void ClReset();
	void ClUpdate(float frameTime);
	
	void ClOnQuery(const char * szQuery);
	void ClSendCommand(RemoteCommandArgs &args);
	void ClSendCommand(const char * szCommand);
	void ClSendCommand(const char * szCommand, const char * szParam);
	void ClSendCommand(const char * szCommand, const char * szArg1, const char * szArg2);

	void ClSendVersion();

	void ClRequestConsoleLines(int count);

	void ClProcessConsoleLine(const char *szLine, int nSeq);
	void ClDisplayConsoleLine(const char *szLine);

	void ClProcessConsoleLinesReq(const char * szLines, int nSeq);
	void ClDisplayConsoleLinesReq(const char * szLines);

private:

	bool ConvertQueryToArgs(string &query, RemoteCommandArgs *pArgs);
	bool ConvertArgsToQuery(RemoteCommandArgs *pArgs, string &dest);

	void ClOnCommand(RemoteCommandArgs &args);

	void RegisterClientCommand(const char *szName, RemoteCommandClientFunc);
	void ExecuteClientCommand(const char *szName, RemoteCommandArgs *pArgs);

	void FlushConsoleLinesQueued();
	void FlushConsoleLinesReqQueued();

	typedef std::map<string, RemoteCommandClientFunc> TMapClCommands;
	TMapClCommands m_clCommands;

	int m_nLastSeqConsoleLine;
	bool m_bQueueingConsoleLines;
	float m_fTimerQueueingConsoleLines;
	typedef std::map<int, string> TMapConsoleLineQueue;
	TMapConsoleLineQueue m_queueConsoleLines;

	int m_nLastSeqConsoleLinesReq;
	bool m_bQueueingConsoleLinesReq;
	float m_fTimerQueueingConsoleLinesReq;
	TMapConsoleLineQueue m_queueConsoleLinesReq;

	static void ClCmdOnCallback(RemoteCommandArgs *pArgs);
	static void ClCmdOnError(RemoteCommandArgs *pArgs);
	static void ClCmdOnAdminMessage(RemoteCommandArgs *pArgs);
	static void ClCmdOnConsoleLine(RemoteCommandArgs *pArgs);
	static void ClCmdOnConsoleLinesRequested(RemoteCommandArgs *pArgs);
};

