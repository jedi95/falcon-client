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
#include "StdAfx.h"
#include "MPLobbyUI.h"
#include "Game.h"
#include "OptionsManager.h"
#include "GameNetworkProfile.h"
#include "GameCVars.h"

static const char* MPPath = "_root.Root.MainMenu.MultiPlayer.\0";

static TKeyValuePair<ESortColumn,const char*>
gSortColumnNames[] = {
					  {eSC_none,""},
					  {eSC_name,"Header_ServerName"},
					  {eSC_ping,"Header_Ping"},
					  {eSC_players,"Header_Players"},
					  {eSC_map,"Header_MapName"},
					  {eSC_mode,"Header_GameMode"},
					  {eSC_private,"Header_Lock"},
					  {eSC_favorite,"Header_Favorites"},
					  {eSC_official,"Header_Rank"},
					  {eSC_anticheat,"Header_Punkbuster"}
					 };
struct CMPLobbyUI::SServerFilter
{
  SServerFilter(CMPLobbyUI* p)
	: m_parent(p),
	  m_on(false),
	  m_minping(0),
	  m_notfull(false),
	  m_notempty(false),
	  m_notprivate(false),
	  m_notcustomized(false),
	  m_autoteambalance(false),
	  m_anticheat(false),
	  m_friendlyfire(false),
	  m_gamepadsonly(false),
	  m_novoicecomms(false),
	  m_dedicated(false),
	  m_dx10(false)
  {
		if(g_pGame->GetOptions())
		{
			g_pGame->GetOptions()->GetProfileValue("Filter.Enabled",m_on);
			g_pGame->GetOptions()->GetProfileValue("Filter.GameMode",m_gamemode);
			g_pGame->GetOptions()->GetProfileValue("Filter.MapName",m_mapname);
			g_pGame->GetOptions()->GetProfileValue("Filter.MinPing",m_minping);
			g_pGame->GetOptions()->GetProfileValue("Filter.NotFull",m_notfull);
			g_pGame->GetOptions()->GetProfileValue("Filter.NotEmpty",m_notempty);
			g_pGame->GetOptions()->GetProfileValue("Filter.NotPrivate",m_notprivate);
			g_pGame->GetOptions()->GetProfileValue("Filter.NotCustomized",m_notcustomized);
			g_pGame->GetOptions()->GetProfileValue("Filter.AutoTeam",m_autoteambalance);
			g_pGame->GetOptions()->GetProfileValue("Filter.AntiCheat",m_anticheat);
			g_pGame->GetOptions()->GetProfileValue("Filter.FriendlyFire",m_friendlyfire);
			g_pGame->GetOptions()->GetProfileValue("Filter.GamePadsOnly",m_gamepadsonly);
			g_pGame->GetOptions()->GetProfileValue("Filter.NoVoiceComm",m_novoicecomms);
			g_pGame->GetOptions()->GetProfileValue("Filter.Dedicated",m_dedicated);
			g_pGame->GetOptions()->GetProfileValue("Filter.DX10",m_dx10);
		}
  }

	~SServerFilter()
	{
		g_pGame->GetOptions()->SaveValueToProfile("Filter.Enabled",m_on);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.GameMode",m_gamemode);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.MapName",m_mapname);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.MinPing",m_minping);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.NotFull",m_notfull);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.NotEmpty",m_notempty);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.NotPrivate",m_notprivate);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.NotCustomized",m_notcustomized);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.AutoTeam",m_autoteambalance);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.AntiCheat",m_anticheat);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.FriendlyFire",m_friendlyfire);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.GamePadsOnly",m_gamepadsonly);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.NoVoiceComm",m_novoicecomms);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.Dedicated",m_dedicated);
		g_pGame->GetOptions()->SaveValueToProfile("Filter.DX10",m_dx10);
		g_pGame->GetOptions()->SaveProfile();
	}

  bool  IsEnabled()const
  {
	return m_on;
  }

  bool  Filter(const SServerInfo& i)const
  {
		if( !m_on )
			return true;
	if( !m_gamemode.empty() && i.m_gameType != m_gamemode)
	  return false;
	if( !m_mapname.empty())
	{
	  static std::string a;
	  a.resize(i.m_mapName.size());
	  for(int k=0;k<a.size();++k)
		a[k] = char(tolower(i.m_mapName[k]));
	  const char* f = strstr(a.c_str(),m_mapname.c_str());
	  if(f==0)
		return false;
	}
	if( m_minping!=0 && i.m_ping > m_minping )
	  return false;
	if( m_notfull && i.m_maxPlayers == i.m_numPlayers)
	  return false;
	if( m_notempty && i.m_numPlayers == 0)
	  return false;
	if( m_notprivate && i.m_private )
	  return false;
	if( m_anticheat && !i.m_anticheat )
	  return false;
	if( m_friendlyfire && !i.m_friendlyfire )
	  return false;
	if( m_gamepadsonly && !i.m_gamepadsonly )
	  return false;
	if( m_novoicecomms && i.m_voicecomm )
	  return false;
	if( m_dedicated && !i.m_dedicated )
	  return false;
	if( m_dx10 && !i.m_dx10 )
	  return false;
	return true;
  }

  bool HandleFSCommand(EGsUiCommand cmd, const char* pArgs);


	void UpdateUI()
	{
		m_parent->m_cmd = MPPath;
		m_parent->m_cmd += "GameLobby_M.GameLobby.LeftLobby.setUseFilters";
		SFlashVarValue arg[1] = {m_on};
		m_parent->m_player->Invoke(m_parent->m_cmd,arg,1);

		//setFilters = function(gameMode, mapName, ping, notFull, notEmpty, noPassword, autoTeam, antiCheat, friendlyFire, GamepadsOnly, noVoiceComm, dediServer, directX)
		SFlashVarValue args[] = 
		{
			SFlashVarValue(m_gamemode.empty()?"all":m_gamemode.c_str()),
			SFlashVarValue(m_mapname),
			SFlashVarValue(m_minping),
			SFlashVarValue(m_notfull),
			SFlashVarValue(m_notempty),
			SFlashVarValue(m_notprivate),
			SFlashVarValue(m_autoteambalance),
			SFlashVarValue(m_anticheat),
			SFlashVarValue(m_friendlyfire),
			SFlashVarValue(m_gamepadsonly),
			SFlashVarValue(m_novoicecomms),
			SFlashVarValue(m_dedicated),
			SFlashVarValue(m_dx10)
		};
		m_parent->m_cmd = MPPath;
		m_parent->m_cmd += "GameLobby_M.GameLobby.LeftLobby.setFilters";
		m_parent->m_player->Invoke(m_parent->m_cmd,args,sizeof(args)/sizeof(args[0]));
	}

	void EnableFilters(bool enable)
	{
		if(m_on != enable)
		{
			m_on = enable;
			m_parent->m_cmd = MPPath;
			m_parent->m_cmd += "GameLobby_M.GameLobby.LeftLobby.setUseFilters";
			SFlashVarValue arg[1] = {enable};
			m_parent->m_player->Invoke(m_parent->m_cmd,arg,1);
		}
	}

  CMPLobbyUI *m_parent;
  bool      m_on;
  string    m_gamemode;
  string    m_mapname;
  int       m_minping;
  bool      m_notfull;
  bool      m_notempty;
  bool      m_notprivate;
  bool      m_notcustomized;
  bool      m_autoteambalance;
  bool      m_anticheat;
  bool      m_friendlyfire;
  bool      m_gamepadsonly;
  bool      m_novoicecomms;
  bool      m_dedicated;
  bool      m_dx10;

};

struct SMPServerList
{
  typedef std::vector<CMPLobbyUI::SServerInfo> ServerInfoVector;
  typedef std::vector<int> DisplayedServersVector;

  SMPServerList():
	m_startIndex(0),
	m_visibleCount(13),
	m_total(0),
	m_done(0),
	m_sortcolumn(eSC_none),
	m_sorttype(eST_ascending),
	m_dirty(false),
	m_selectedServer(-1),
	m_displayMode(0)
  {

  }

  void AddToVisible(int idx)
  {
	m_all.push_back(idx);
  }

  void RemoveFromVisible(int idx)
  {
	stl::find_and_erase(m_all,idx);
  }

  void AddServer(const CMPLobbyUI::SServerInfo& srv)
  {
	int idx = m_allServers.size();
	m_allServers.push_back(srv);

	if(m_displayMode == 0)
	  AddToVisible(idx);
	m_dirty = true;
  }

  void UpdateServer(const CMPLobbyUI::SServerInfo& srv)
  {
	int idx = GetServerIdxById(srv.m_serverId);
	if(idx!=-1)
	{
	  int ping = m_allServers[idx].m_ping;
	  ping = srv.m_ping;
	  m_allServers[idx] = srv;
	  m_allServers[idx].m_ping = ping;
	}
  }

  void UpdatePing(int id, int ping)
  {
	int idx = GetServerIdxById(id);
	if(idx != -1)
	{
	  m_allServers[idx].m_ping = ping;
	}
  }

  void RemoveServer(const int id)
  {
	int idx = GetServerIdxById(id);
	if(idx != -1)
	{
	  m_allServers.erase(m_allServers.begin()+idx);
	  RemoveFromVisible(idx);
	}
  }

  int GetServerIdxById(int id)
  {
	for(uint32 i=0;i<m_allServers.size();i++)
	  if(m_allServers[i].m_serverId == id)
		return i;
	return -1;
  }

  bool    SetScrollPos(double fr)
  {
	int pos = (int)(fr*(m_all.size()-m_visibleCount));
	return SetStartIndex(pos);
  }

  bool    SetStartIndex(int i)
  {
	int set_idx = i;
	if(set_idx+m_visibleCount>m_all.size())
	  set_idx = m_all.size()-m_visibleCount;

	if(set_idx<0)
	  set_idx = 0;

	if(set_idx!= m_startIndex)
	{
	  m_startIndex = set_idx;
	  return true;
	}

	return false;
  }

  void    SetVisibleCount(int c)
  {
	if(m_startIndex + c > m_all.size())
	  m_startIndex = max(int(m_all.size())-c,0);
	m_visibleCount = c;
  }

  void Clear()
  {
	m_all.resize(0);
	m_empty.resize(0);
	m_allServers.resize(0);
	m_startIndex = 0;
	m_selectedServer = -1;
  }

  void SetDisplayMode(int mode)
  {
	m_startIndex = 0;
	switch(mode)
	{
	case 0:
	  m_all.resize(m_allServers.size());
	  for(int i=0;i<m_allServers.size();++i)
		m_all[i] = i;
	  m_dirty = true;
	  break;
	case 1:
	  m_all = m_empty;
	  m_dirty = true;
	 break;
	case 2:
	  m_all = m_empty;
	  m_dirty = true;
	  break;
	}
	m_displayMode = mode;
  }

  bool SelectServer(int id)
  {
	int idx = GetServerIdxById(id);
	if(idx != -1)
	{
	  m_selectedServer = idx;                        
	}
	return m_selectedServer != -1;
  }
  
  void ClearSelection()
  {
	m_selectedServer = -1;
  }

  bool IsVisible(int idx)
  {
	return idx>=m_startIndex && idx<min(int(m_all.size()),m_startIndex+m_visibleCount);
  }
  
  struct SSort 
  {
	SSort(const SMPServerList& sl):m_sl(sl)
	{
	}

	bool operator()(int i, int j)const//SORT!
	{
	  int less = -1;
	  switch(m_sl.m_sortcolumn)
	  {
	  case eSC_name:
				{
					bool fn = !m_sl.m_allServers[i].m_hostName.empty();
					bool sn = !m_sl.m_allServers[j].m_hostName.empty();
					if(!fn && !sn)
						less = -1;
					else if(fn && sn)
						less = (m_sl.m_allServers[i].m_hostName < m_sl.m_allServers[j].m_hostName)?1:0;
					else
						less = (fn && !sn)?1:0;
				}
				break;
	  case eSC_ping:
		less = (m_sl.m_allServers[i].m_ping < m_sl.m_allServers[j].m_ping)?1:0;
		break;
	  case eSC_players:
		less = (m_sl.m_allServers[i].m_numPlayers < m_sl.m_allServers[j].m_numPlayers)?1:0;
		break;
	  case eSC_map:
		less = (m_sl.m_allServers[i].m_mapName < m_sl.m_allServers[j].m_mapName)?1:0;
		break;
	  case eSC_mode:
		less = (m_sl.m_allServers[i].m_gameType < m_sl.m_allServers[j].m_gameType)?1:0;
		break;
	  case eSC_private:
		less = (m_sl.m_allServers[i].m_private && !m_sl.m_allServers[j].m_private)?1:0;
		break;
	  case eSC_favorite:
		less = 0;
		break;
	  case eSC_official:
		less = (m_sl.m_allServers[i].m_official && !m_sl.m_allServers[j].m_official)?1:0;
		break;
	  case eSC_anticheat:
		less = (m_sl.m_allServers[i].m_anticheat && !m_sl.m_allServers[j].m_anticheat)?1:0;
		break;
	  }
	  if(less == -1)
		less = (m_sl.m_allServers[i].m_publicIP<m_sl.m_allServers[j].m_publicIP)?1:0;

	  return m_sl.m_sorttype==eST_ascending?(less!=0):(less==0);
	}
	const SMPServerList& m_sl;
  };

  void DoFilter(CMPLobbyUI::SServerFilter* filter)
  {
	m_all.resize(0);
	switch(m_displayMode)
	{
	case 0:
	  {
		for(int i=0;i<m_allServers.size();++i)
		  if(filter->Filter(m_allServers[i]))
			m_all.push_back(i);
	  }
	  break;
	case 1:
	  break;
	case 2:
	  break;
	}
	if(m_selectedServer != -1)
	{
	  if(std::find(m_all.begin(),m_all.end(),m_selectedServer)==m_all.end())
		ClearSelection();        
	}
  }

  void DoSort()
  {
	if(m_sortcolumn == eSC_none)
	  return;
	int selected_pos = 0;
	if(m_selectedServer != -1)
	{
	  for(int i=0;i<m_all.size();++i)
		if(m_all[i] == m_selectedServer)
		{
		  selected_pos = i;
		  break;
		}
	}
	
	std::stable_sort(m_all.begin(),m_all.end(),SSort(*this));

	if(m_selectedServer != -1)
	{
	  int new_selected = -1;
	  for(int i=0;i<m_all.size();++i)
	  {
		if(m_all[i] == m_selectedServer)
		{
		  new_selected = i;
		  break;
		}
	  }
	  
	  if(new_selected!=-1)
	  {
		if(IsVisible(selected_pos))//if selected server is visible keep it's position on screen
		{
		  //finally adjust view position
		  SetStartIndex(m_startIndex + new_selected-selected_pos);
		}
	  }
	}
	m_dirty = false;
  }

  CMPLobbyUI::SServerInfo& GetSelectedServer()
  {
	static CMPLobbyUI::SServerInfo dummy;
	if(m_selectedServer == -1)
	  return dummy;
	return m_allServers[m_selectedServer];
  }

	DisplayedServersVector m_all;
	DisplayedServersVector m_empty;
  bool                m_updateCompleted;

  ServerInfoVector    m_allServers;
  //server list info
  int                 m_displayMode;
  int                 m_startIndex;
  int                 m_visibleCount;
  int                 m_selectedServer;
  int                 m_total;
  int                 m_done;
  bool                m_dirty;
  ESortColumn         m_sortcolumn;
  ESortType           m_sorttype;
};

ESortColumn         gSortColumn = eSC_none;
ESortType           gSortType = eST_ascending;

CMPLobbyUI::CMPLobbyUI(IFlashPlayer* plr):
m_player(plr),
m_serverlist(new SMPServerList()),
m_currentTab(-1)
{
  m_cmd.reserve(256);
  SetSortParams(gSortColumn, gSortType);
  m_filter.reset(new SServerFilter(this));
}

CMPLobbyUI::~CMPLobbyUI()
{
  gSortColumn = m_serverlist->m_sortcolumn;
  gSortType = m_serverlist->m_sorttype;
}

bool CMPLobbyUI::HandleFSCommand(EGsUiCommand cmd, const char* pArgs)
{
	bool handled = true;
	switch(cmd)
	{
		case eGUC_setVisibleServers:
		{
			int i=atoi(pArgs);
			m_serverlist->SetVisibleCount(i);
			DisplayServerList();
			break;
		}
		case eGUC_displayServerList:
		{
			DisplayServerList();
		}
		break;
		case eGUC_serverScrollBarPos:
		{
			double pos = atof(pArgs);
			SetServerListPos(pos);
			break;
		}
		case eGUC_serverScroll:
		{
			int d = atoi(pArgs);
			ChangeServerListPos(d);
			break;
		}
		case eGUC_userScrollBarPos:
			break;
		case eGUC_userScroll:
			break;
		case eGUC_chatScrollBarPos:
			break;
		case eGUC_chatScroll:
			break;
		case eGUC_selectServer:
		{
			int id = atoi(pArgs);
			SelectServer(id);
			break;
		}
		case eGUC_tab:
		{
			int id = atoi(pArgs);
			ChangeTab(id);
		}
		break;
		case eGUC_find:
			break;
		case eGUC_addBuddy:
			break;
		case eGUC_addBuddyFromFind:
			break;
		case eGUC_addBuddyFromInfo:
			break;
		case eGUC_addIgnoreFromInfo:
			break;
		case eGUC_addIgnore:
			break;
		case eGUC_stopIgnore:
			break;
		case eGUC_inviteBuddy:
			break;
		case eGUC_removeBuddy:
			break;
		case eGUC_chatClick:
			break;
		case eGUC_sortColumn:
		{
			ESortColumn old_mode = m_serverlist->m_sortcolumn;
			ESortType   old_type = m_serverlist->m_sorttype;
			string header = pArgs;
			const char* comma = strchr(pArgs,',');
			if(comma!=0)
			{
				header = header.substr(0,comma-pArgs);
				m_serverlist->m_sorttype = comma[1]=='1'?eST_ascending:eST_descending;
			}

			ESortColumn c = KEY_BY_VALUE(header,gSortColumnNames);
			m_serverlist->m_sortcolumn = c;

			if(old_mode!=m_serverlist->m_sortcolumn || old_type!=m_serverlist->m_sorttype)
			{
				m_serverlist->m_dirty = true;
				DisplayServerList();
			}
			break;
		}
		case eGUC_displayInfo:
			break;
		case eGUC_displayInfoInList:
			break;
		case eGUC_addFavorite:
			break;
		case eGUC_addFavoriteByIP:
			break;
		case eGUC_removeFavorite:
			break;
		case eGUC_joinPassword:
		{
			OnJoinWithPassword(atoi(pArgs));
			break;
		}
		case eGUC_chatOpen:
			break;
		default:
			handled = false;
	}

	handled = OnHandleCommand(cmd,pArgs) || handled;
	handled = m_filter->HandleFSCommand(cmd,pArgs) || handled;  

	return handled;
}

void  CMPLobbyUI::ClearServerList()
{
  m_cmd = MPPath;
  m_cmd += "ClearServerList\0";
  m_player->Invoke0(m_cmd);
  m_serverlist->Clear();
  ClearSelection();
}

void  CMPLobbyUI::AddServer(const SServerInfo& srv)
{
  m_serverlist->AddServer(srv);
}

void  CMPLobbyUI::UpdateServer(const SServerInfo& srv)
{
  m_serverlist->UpdateServer(srv);
}

void  CMPLobbyUI::UpdatePing(int id, int ping)//applies sorting
{
  m_serverlist->UpdatePing(id,ping);
}

void  CMPLobbyUI::RemoveServer(int id)
{
  if(m_serverlist->m_selectedServer!=-1 && m_serverlist->GetSelectedServer().m_serverId == id)
  {
	m_cmd = MPPath;
	m_cmd += "ClearSelectedServer\0";
	m_player->Invoke0(m_cmd);
  }
  m_serverlist->RemoveServer(id);
}

void  CMPLobbyUI::ClearSelection()
{
  m_serverlist->ClearSelection();
  m_cmd = MPPath;
  m_cmd += "ClearSelectedServer\0";
  m_player->Invoke0(m_cmd);

}

bool  CMPLobbyUI::GetSelectedServer(SServerInfo& srv)
{
  if(m_serverlist->m_selectedServer==-1)
	return false;
  srv = m_serverlist->GetSelectedServer();
  return true;
}

bool  CMPLobbyUI::GetServer(int id, SServerInfo& srv)
{
  int idx = m_serverlist->GetServerIdxById(id);
  if(idx==-1)
	return false;
  srv = m_serverlist->m_allServers[idx];
  return true;
}
void  CMPLobbyUI::FinishUpdate()
{
  m_cmd = MPPath;
  m_cmd += "GameLobby_M.GameLobby.LeftLobby.switchUpdateButton\0";
  m_player->Invoke1(m_cmd,0);
  DisplayServerList();
}

void  CMPLobbyUI::StartUpdate()
{
  m_cmd = MPPath;
  m_cmd += "GameLobby_M.GameLobby.LeftLobby.switchUpdateButton\0";
  m_player->Invoke1(m_cmd,1);
  DisplayServerList();
}

void  CMPLobbyUI::SetUpdateProgress(int done, int total)
{
  m_serverlist->m_done = done;
  m_serverlist->m_total = total;
}

void CMPLobbyUI::ChangeTab(int tab)
{
	if(m_currentTab == tab)
		return;
	if(m_currentTab != -1)
		OnDeactivateTab(m_currentTab);
	m_currentTab = tab;
	m_serverlist->SetDisplayMode(m_currentTab);
	ClearSelection();
	DisplayServerList();
	if(tab != -1)
		OnActivateTab(tab);
}

void  CMPLobbyUI::DisplayServerList()
{
  if(m_serverlist->m_dirty)
  {
		if(m_filter.get())
			m_serverlist->DoFilter(m_filter.get());
	m_serverlist->DoSort();
  }
  m_cmd = MPPath;
  m_cmd += "ClearServerList\0";
  m_player->Invoke0(m_cmd.c_str());
  
  char convert[32]      = "\0";
  char uiNumPlayers[32] = "\0";
  char uiIP[32] = "\0";

  //scrolling

  int num_servers = m_serverlist->m_all.size();

  itoa(num_servers , convert, 10);
  m_cmd = MPPath;
  m_cmd += "NumServers\0";
  m_player->SetVariable(m_cmd.c_str(), convert);

  itoa(m_serverlist->m_startIndex, convert, 10);
  m_cmd = MPPath;
  m_cmd += "DisplayServerIndex\0";
  m_player->SetVariable(m_cmd.c_str(), convert);

  m_cmd = MPPath;
  m_cmd += "ManageServerScrollbar\0";
  m_player->Invoke0(m_cmd.c_str());
  
  static string count;
  count.resize(0);

  if(m_serverlist->m_total != -1)
  {
	itoa(m_serverlist->m_done, convert, 10);
	count = convert;
	count += "/";
	itoa(m_serverlist->m_total, convert, 10);
	count += convert;
  }
  m_cmd = MPPath;
  m_cmd +="GameLobby_M.GameLobby.Texts2.Colorset.ServerCount.text";
  m_player->SetVariable(m_cmd.c_str(), count.c_str());

  for(int i=0;i<m_serverlist->m_visibleCount;i++)
  {
	int idx = i + m_serverlist->m_startIndex;
	if(idx>=num_servers)
	  break;

	const SServerInfo &server = m_serverlist->m_allServers[m_serverlist->m_all[idx]];

	itoa(server.m_numPlayers, convert, 10);
	if (server.m_numPlayers < 10)
	{
	  convert[1] = convert[0];
	  convert[0] = ' ';
	}
	convert[2] = '\0';

	strcpy(uiNumPlayers, convert);
	strcat(uiNumPlayers, " / \0");

	itoa(server.m_maxPlayers, convert, 10);
	if (server.m_maxPlayers < 10)
	{
	  convert[1] = ' ';
	}
	convert[2] = '\0';

	strcat(uiNumPlayers, convert);

	_snprintf(uiIP,32,"%d.%d.%d.%d",server.m_publicIP&0xFF,(server.m_publicIP>>8)&0xFF,(server.m_publicIP>>16)&0xFF,(server.m_publicIP>>24)&0xFF);
	uiIP[31] = 0;

	m_cmd = MPPath;
	m_cmd += "AddServer\0";

		SFlashVarValue args[] = {server.m_serverId, server.m_hostName.c_str(), server.m_ping, uiNumPlayers, server.m_mapName.c_str(), server.m_gameTypeName.c_str(), uiIP, server.m_hostPort, server.m_private, server.m_official, false, server.m_anticheat, server.m_canjoin, server.m_gameVersion.c_str()};
	m_player->Invoke(m_cmd.c_str(), args, sizeof(args)/sizeof(args[0]));

  }

  m_cmd = MPPath;
  m_cmd += "SetServerListInfo\0";
  m_player->Invoke0(m_cmd.c_str());
}

void CMPLobbyUI::SetServerListPos(double sb_pos)
{
	if(m_serverlist->SetScrollPos(sb_pos))
		DisplayServerList();
}

void CMPLobbyUI::ChangeServerListPos(int delta)
{
	if(m_serverlist->SetStartIndex(m_serverlist->m_startIndex+delta))
		DisplayServerList();
}

void CMPLobbyUI::SelectServer(int id)
{
  if(m_serverlist->SelectServer(id))
  {
	ResetServerDetails();
  }
}

void  CMPLobbyUI::ResetServerDetails()
{
  m_cmd = MPPath;
  m_cmd += "GameLobby_M.GameLobby.resetServerInfo";
  m_player->Invoke0(m_cmd);

  m_cmd = MPPath;
  m_cmd += "ClearPlayerList";
  m_player->Invoke0(m_cmd);

  m_cmd = MPPath;
  m_cmd += "SetPlayerListInfo";
  m_player->Invoke0(m_cmd);
}

void  CMPLobbyUI::SetServerDetails(const SServerDetails& sd)
{
  m_cmd = MPPath;
  m_cmd += "GameLobby_M.GameLobby.setServerInfo";
	if(sd.m_noResponce)
	{
		SFlashVarValue args[]={"@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A","@ui_menu_N_A"};
		m_player->Invoke(m_cmd,args,sizeof(args)/sizeof(args[0]));
	}
	else
	{
		static string timesting;
		if(sd.m_timelimit && sd.m_timeleft)
			timesting.Format("%02d:%02d",sd.m_timeleft/60,sd.m_timeleft%60);
		else
			timesting = "-";
		SFlashVarValue args[]={sd.m_gamever.c_str(),
													timesting.c_str(),
													0,
													sd.m_anticheat?"@ui_menu_ON":"@ui_menu_OFF",
													sd.m_friendlyfire?"@ui_menu_ON":"@ui_menu_OFF",
													"-", 
													sd.m_dedicated?"@ui_menu_YES":"@ui_menu_NO", 
													sd.m_voicecomm?"@ui_menu_ENABLED":"@ui_menu_DISABLED", 
													sd.m_gamepadsonly?"@ui_menu_YES":"@ui_menu_NO",
													sd.m_modname.c_str(),
													sd.m_modversion.c_str()};

		m_player->Invoke(m_cmd,args,sizeof(args)/sizeof(args[0]));
	}

  m_cmd = MPPath;
  m_cmd += "SetPlayerGameMode";
  m_player->Invoke1(m_cmd,sd.m_gamemode.c_str());

  if(!sd.m_players.empty())
  {
	m_cmd = MPPath;
	m_cmd += "AddPlayer";

	for(int i=0;i<sd.m_players.size();++i)
	{
	  SFlashVarValue args[]={
				SFlashVarValue(i),
				SFlashVarValue(sd.m_players[i].m_team),
				SFlashVarValue(sd.m_players[i].m_name.c_str()),
				SFlashVarValue(sd.m_players[i].m_rank),
				SFlashVarValue(sd.m_players[i].m_kills),
				SFlashVarValue(sd.m_players[i].m_deaths)
			};
	  m_player->Invoke(m_cmd,args,sizeof(args)/sizeof(args[0]));
	}
	m_cmd = MPPath;
	m_cmd += "SetPlayerListInfo";
	m_player->Invoke0(m_cmd);
  }
  DisplayServerList();
}

void  CMPLobbyUI::SetSortParams(ESortColumn sc, ESortType st)
{
  m_serverlist->m_sortcolumn = sc;
  m_serverlist->m_sorttype = st;
  m_cmd = MPPath;
  m_cmd += "SortOrder";
  m_player->SetVariable(m_cmd,st==eST_ascending?"ASC":"DSC");
  m_cmd = MPPath;
  m_cmd += "SortColumn";
  const char* col = VALUE_BY_KEY(sc,gSortColumnNames);
  m_player->SetVariable(m_cmd,col);
}

void CMPLobbyUI::EnableTabs(bool fav, bool recent, bool chat)
{
	m_cmd = MPPath;
	m_cmd += "setMPTabs";
	SFlashVarValue args[]={1,fav?1:0,recent?1:0,chat?1:0};
	m_player->Invoke(m_cmd,args,sizeof(args)/sizeof(args[0]));
}

void CMPLobbyUI::SetJoinPassword()
{
	SFlashVarValue p("");
	m_player->GetVariable("_root.MPServer_Password",&p);
	ICVar* pV = gEnv->pConsole->GetCVar("sv_password");
	if(pV)
		pV->Set(p.GetConstStrPtr());
}

void CMPLobbyUI::EnableResume(bool enable)
{
	m_player->Invoke1("setResumeEnabled",enable);
}

void  CMPLobbyUI::SetStatusString(const char* str)
{
	m_player->Invoke1("setToolTipText",str);
}

void CMPLobbyUI::SetStatusString(const char* fmt, const char* param)
{
	static wstring tmp, tmp2;
	gEnv->pSystem->GetLocalizationManager()->LocalizeLabel(fmt, tmp);

	StrToWstr(param,tmp2);

	static wstring text;
	text.resize(0);

	gEnv->pSystem->GetLocalizationManager()->FormatStringMessage(text,tmp,tmp2.c_str());
	m_cmd = "setToolTipText";
	m_player->Invoke1(m_cmd,text.c_str());
}

void  CMPLobbyUI::SetJoinButtonMode(EJoinButtonMode m)
{
	if(m == eJBM_default)
		m_player->Invoke1("_root.Root.MainMenu.MultiPlayer.setJoinServerButtonState", -1);
	else
		m_player->Invoke1("_root.Root.MainMenu.MultiPlayer.setJoinServerButtonState", int(m));
}

void  CMPLobbyUI::OpenPasswordDialog(int id)
{
	m_player->Invoke1("showEnterPassword",id);
}

bool CMPLobbyUI::SServerFilter::HandleFSCommand(EGsUiCommand cmd, const char* pArgs)
{
  switch(cmd)
  {
	case eGUC_filtersDisplay:
		UpdateUI();
		return true;
  case eGUC_filtersEnable:
	m_on = atoi(pArgs)!=0;
	break;
  case eGUC_filtersMode:
	m_gamemode = strcmp(pArgs,"all")!=0?pArgs:"";
	break;
  case eGUC_filtersMap:
	m_mapname = pArgs;
	m_mapname.MakeLowerLocale();
	break;
  case eGUC_filtersPing:
	m_minping = atoi(pArgs);
	break;
  case eGUC_filtersNotFull:
	m_notfull = atoi(pArgs)!=0;
	break;
  case eGUC_filtersNotEmpty:
	m_notempty = atoi(pArgs)!=0;
	break;
  case eGUC_filtersNoPassword:
	m_notprivate = atoi(pArgs)!=0;
	break;
  case eGUC_filtersAutoTeamBalance:
	m_autoteambalance = atoi(pArgs)!=0;
	break;
  case eGUC_filtersAntiCheat:
	m_anticheat = atoi(pArgs)!=0;
	break;
  case eGUC_filtersFriendlyFire:
	m_friendlyfire = atoi(pArgs)!=0;
	break;
  case eGUC_filtersGamepadsOnly:
	m_gamepadsonly = atoi(pArgs)!=0;
	break;
  case eGUC_filtersNoVoiceComms:
	m_novoicecomms = atoi(pArgs)!=0;
	break;
  case eGUC_filtersDedicated:
	m_dedicated = atoi(pArgs)!=0;
	break;
  case eGUC_filtersDX10:
	m_dx10 = atoi(pArgs)!=0;
	break;
  default:
	return false;
  }
	if(cmd != eGUC_filtersEnable && cmd != eGUC_filtersDisplay)//if we're changed any filters setting, enable them
	{
		EnableFilters(true);
	}
  //changed
  if(m_on || cmd==eGUC_filtersEnable)
  {
	m_parent->m_serverlist->m_dirty = true;
	m_parent->DisplayServerList();
  }
  return true;
}
