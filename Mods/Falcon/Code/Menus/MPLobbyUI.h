/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Multiplayer lobby

-------------------------------------------------------------------------
History:
- 02/6/2006: Created by Stas Spivakov

*************************************************************************/
#ifndef __MPLOBBYUI_H__
#define __MPLOBBYUI_H__

#pragma once

#include "IFlashPlayer.h"
#include "INetwork.h"
#include "INetworkService.h"
#include "MPHub.h"

enum ESortColumn
{
	eSC_none,
	eSC_name,
	eSC_ping,
	eSC_players,
	eSC_map,
	eSC_mode,
	eSC_private,
	eSC_favorite,
	eSC_official,
	eSC_anticheat,
};

enum ESortType
{
	eST_ascending,
	eST_descending
};

enum EJoinButtonMode
{
	eJBM_hidden = 0,
	eJBM_join,
	eJBM_disconnect,
	eJBM_joinBuddy,
	eJBM_default,
};

struct SMPServerList;

class CMPLobbyUI
{
public:
	struct SServerInfo
	{
		SServerInfo():
		m_publicIP(0),
		m_publicPort(0),
		m_hostPort(0),
		m_privateIP(0),
		m_numPlayers(0),
		m_maxPlayers(0),
		m_ping(-1),
		m_serverId(-1),
		m_private(false),
		m_official(false),
		m_anticheat(false),
		m_gamepadsonly(false),
		m_canjoin(true)
	{}

		string m_hostName;
		string m_mapName;
		string m_gameType;
		wstring m_gameTypeName;
		string m_gameVersion;

		string m_modName;
		string m_modVersion;

		uint32 m_publicIP;
		ushort m_publicPort;
		ushort m_hostPort;
		uint32 m_privateIP;
		int m_numPlayers;
		int m_maxPlayers;
		int m_ping;

		int m_serverId;

		bool m_private;
		bool m_official;
		bool m_anticheat;
		bool m_voicecomm;
		bool m_friendlyfire;
		bool m_dx10;
		bool m_dedicated;
		bool m_gamepadsonly;

		bool m_canjoin;//determined on client side
	};

	struct SServerDetails
	{
		SServerDetails():
		m_noResponce(false),
		m_friendlyfire(false),
		m_dedicated(false),
		m_timelimit(0),
		m_timeleft(0),
		m_voicecomm(false),
		m_anticheat(false),
		m_gamepadsonly(false),
		m_dx10(false)
	{
	}

		bool m_noResponce;
		bool m_friendlyfire;
		bool m_dedicated;
		int m_timelimit;
		int m_timeleft;
		bool m_voicecomm;
		bool m_anticheat;
		bool m_gamepadsonly;
		bool m_dx10;

		string m_gamever;
		string m_gamemode;
		string m_modname;
		string m_modversion;

		struct SPlayerDetails
		{
			SPlayerDetails():m_team(0),m_kills(0),m_deaths(0),m_iRank(0){}
			string m_name;
			string m_rank;
			int m_team;
			int m_kills;
			int m_iRank;
			int m_deaths;
		};
		std::vector<SPlayerDetails> m_players;
	};

	struct SServerFilter;

	CMPLobbyUI(IFlashPlayer* plr);
	virtual ~CMPLobbyUI();
	bool HandleFSCommand(EGsUiCommand cmd, const char* pArgs);
	void ClearServerList();
	void AddServer(const SServerInfo& srv);
	void UpdateServer(const SServerInfo& srv);
	void UpdatePing(int id, int ping);//applies sorting
	void RemoveServer(int id);
	void ClearSelection();
	bool GetSelectedServer(SServerInfo& srv);
	bool GetServer(int id, SServerInfo& srv);
	void FinishUpdate();
	void StartUpdate();
	void SetUpdateProgress(int done, int total);
	void ResetServerDetails();
	void SetServerDetails(const SServerDetails&);
	void ChangeTab(int tab);
	void EnableTabs(bool fav, bool recent, bool chat);
	void SetStatusString(const char* str);
	void SetStatusString(const char* fmt, const char* param);
	void SetJoinButtonMode(EJoinButtonMode m);
	void OpenPasswordDialog(int );
	void SetJoinPassword();
	void EnableResume(bool enable);

protected:
	virtual void OnActivateTab(int tab){}
	virtual void OnDeactivateTab(int tab){}
	virtual bool OnHandleCommand(EGsUiCommand cmd, const char* pArgs){return false;}
	virtual void OnJoinWithPassword(int id){}

private:
	void DisplayServerList();
	void SetServerListPos(double sb_pos);
	void ChangeServerListPos(int delta);
	void SelectServer(int id);
	void SetSortParams(ESortColumn, ESortType);

	IFlashPlayer* m_player;
	std::auto_ptr<SMPServerList> m_serverlist;
	std::auto_ptr<SServerFilter> m_filter;

	string m_cmd;
	int m_currentTab;
};

inline bool IsTrue(const char* str)
{
	if(*str == 'T' || *str == 't')//true
		return true;
	if(*str == 'O' || *str == 'o')//on
		return true;
	return atoi(str)!=0;
}

#endif /*__MPLOBBYUI_H__*/