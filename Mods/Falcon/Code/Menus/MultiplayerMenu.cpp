/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Multiplayer lobby

-------------------------------------------------------------------------
History:
- 12/2006: Created by Stas Spivakov

*************************************************************************/
#include "StdAfx.h"
#include "MultiplayerMenu.h"
#include "IGameFramework.h"
#include "MPLobbyUI.h"
#include "INetwork.h"
#include "INetworkService.h"
#include "GameCVars.h"
#include "HUD/GameFlashLogic.h"
#include "CreateGame.h"
#include "Game.h"
#include "GameNetworkProfile.h"

enum EServerInfoKey
{
  eSIK_unknown,
  eSIK_hostname,
  eSIK_mapname,
  eSIK_numplayers,
  eSIK_maxplayers,
  eSIK_gametype,
  eSIK_dedicated,
  eSIK_official,
  eSIK_frinedlyFire,
  eSIK_gamever,
	eSIK_timeleft,
  eSIK_timelimit,
  eSIK_anticheat,
  eSIK_voicecomm,
	eSIK_gamepadsonly,
  eSIK_private,
	eSIK_dx10,

  //player,
  eSIK_playerName,
  eSIK_playerTeam,
  eSIK_playerRank,
  eSIK_playerKills,
	eSIK_playerDeaths,

	//mods
	eSIK_modName,
	eSIK_modVersion
};


static TKeyValuePair<EServerInfoKey,const char*>
gServerKeyNames[] = { 
					  {eSIK_unknown,""},
					  {eSIK_hostname,"hostname"},
					  {eSIK_mapname,"mapname"},
					  {eSIK_numplayers,"numplayers"},
					  {eSIK_maxplayers,"maxplayers"},
					  {eSIK_gametype,"gametype"},
					  {eSIK_dedicated,"dedicated"},
					  {eSIK_official,"official"},
					  {eSIK_frinedlyFire,"friendlyfire"},
											{eSIK_timeleft,"timeleft"},
					  {eSIK_timelimit,"timelimit"},
					  {eSIK_gamever,"gamever"}, 
					  {eSIK_anticheat,"anticheat"},
											{eSIK_voicecomm,"voicecomm"},
											{eSIK_gamepadsonly,"gamepadsonly"},
					  {eSIK_private,"password"},
											{eSIK_dx10,"dx10"},
					  {eSIK_playerName,"player"},
					  {eSIK_playerTeam,"team"},
					  {eSIK_playerRank,"rank"},
											{eSIK_playerKills,"kills"},
											{eSIK_playerDeaths,"deaths"},
											{eSIK_modName,"modname"},
											{eSIK_modVersion,"modversion"}
					};


enum EChannelKey
{
	eCK_None,
	eCK_Profile,
	eCK_Country,
	eCK_Played,
	eCK_GameMode,
	eCK_Map,
	eCK_Weapon,
	eCK_Vehicle,
	eCK_SuitMode,
	eCK_Accuracy,
	eCK_KillsPerMinute,
	eCK_Kills,
	eCK_Deaths,
};

static TKeyValuePair<EChannelKey,const char*>
gChannelKeyNames[] = {
											{eCK_None,""},
											{eCK_Profile,"Profile"},
											{eCK_Played,"TimePlayed"},
											{eCK_GameMode,"FavoriteGameMode"},
											{eCK_Map,"FavoriteMap"},
											{eCK_Weapon,"FavoriteWeapon"},
											{eCK_Vehicle,"FavoriteVehicle"},
											{eCK_SuitMode,"FavoriteSuitMode"},
											{eCK_Accuracy,"Accuracy"},
											{eCK_KillsPerMinute,"KillsPerMinute"},
											{eCK_Kills,"Kills"},
											{eCK_Deaths,"Deaths"}
										};

class  CMultiPlayerMenu::CUI : public CMPLobbyUI
{
public:
	CUI(IFlashPlayer* plr):CMPLobbyUI(plr),m_menu(0),m_curTab(0)
	{
	}

	void SetMenu(CMultiPlayerMenu* m)
	{
		m_menu = m;
	}
	int GetCurTab()const
	{
		return m_curTab;
	}
	void JoinSelectedServer();

protected:
	virtual void OnActivateTab(int tab);
	virtual void OnDeactivateTab(int tab);
	virtual bool OnHandleCommand(EGsUiCommand cmd, const char* pArgs);
	virtual void OnJoinWithPassword(int id);
	void RefreshServer();

private:
	CMultiPlayerMenu* m_menu;
	int m_curTab;

	string m_joinIpCmd;
	string m_joinCmd;
	string m_joinServerName;
};

struct CMultiPlayerMenu::SGSBrowser : public IServerListener
{
	SGSBrowser():
	m_menu(0),
	m_pendingUpdate(0)
	{
		char strProductVersion[256];
		gEnv->pSystem->GetProductVersion().ToString(strProductVersion);
		m_version = strProductVersion;

		//get current mod info
		SModInfo info;
		if(g_pGame->GetIGameFramework()->GetModInfo(&info))
		{
			m_modName = info.m_name;
			m_modVersion = info.m_version;
		}
	}

	void  SetMenu(CMultiPlayerMenu* menu)
	{
		m_menu = menu;
	}

	virtual void UpdateServer(const int id,const SBasicServerInfo* info)
	{
		OnServer(id,info,true);
	 }

	virtual void NewServer(const int id,const SBasicServerInfo* info)
	{
		OnServer(id,info,false);
	}

	void OnServer(const int id,const SBasicServerInfo* info, bool update)
	{
		CMPLobbyUI::SServerInfo si;
		si.m_numPlayers		= info->m_numPlayers;
		si.m_maxPlayers		= info->m_maxPlayers;
		si.m_private		= info->m_private;
		si.m_hostName		= info->m_hostName;
		si.m_publicIP		= info->m_publicIP;
		si.m_publicPort		= info->m_publicPort;
		si.m_hostPort		= info->m_hostPort;
		si.m_privateIP		= info->m_privateIP;
		si.m_hostName		= info->m_hostName;
		si.m_mapName		= info->m_mapName;
		si.m_gameVersion	= info->m_gameVersion;
		si.m_gameTypeName	= GetGameType(info->m_gameType);
		si.m_gameType		= info->m_gameType;
		si.m_official		= false;
		si.m_anticheat		= info->m_anticheat;
		si.m_voicecomm		= info->m_voicecomm; 
		si.m_friendlyfire	= info->m_friendlyfire;
		si.m_dx10			= info->m_dx10;
		si.m_dedicated		= info->m_dedicated;
		si.m_gamepadsonly	= info->m_gamepadsonly;
		si.m_serverId		= id;
		si.m_ping			= 10000;
		si.m_modName		= info->m_modName;
		si.m_modVersion		= info->m_modVersion;

		if((!m_version.empty() && m_version != si.m_gameVersion) || (m_modName!=si.m_modName) || (m_modVersion!=si.m_modVersion))
		{
			return;
		}

		//check for ghost servers
		if(si.m_gameType.empty() && si.m_mapName.empty() && si.m_hostName.empty())
		{
			return;
		}

		si.m_canjoin = (m_version.empty() || m_version == si.m_gameVersion) && (m_modName==si.m_modName) && (m_modVersion==si.m_modVersion);

		if(update)
			m_menu->m_ui->UpdateServer(si);
		else
			m_menu->m_ui->AddServer(si);
	}

	virtual void RemoveServer(const int id)
	{
		m_menu->m_ui->RemoveServer(id);
	}

	virtual void UpdatePing(const int id, const int ping)
	{
		m_menu->m_ui->UpdatePing(id,ping);
	}

	virtual void UpdateValue(const int id,const char* name,const char* value)
	{
	EServerInfoKey key = KEY_BY_VALUE(CONST_TEMP_STRING(name),gServerKeyNames);

	CMPLobbyUI::SServerInfo si;
	if(m_menu->m_ui->GetServer(id,si))
	{
	  bool basic = true;
	  switch(key)
	  {
	  case eSIK_hostname:
		si.m_hostName = value;
		break;
	  case eSIK_mapname:
		si.m_mapName = value;
		break;
	  case eSIK_numplayers:
				si.m_numPlayers = atoi(value);
				if(si.m_numPlayers)
					m_details.m_players.resize(si.m_numPlayers);
		break;
	  case eSIK_maxplayers:
		si.m_maxPlayers = atoi(value);
		break;
	  case eSIK_gametype:
		si.m_gameTypeName = GetGameType(value);
		si.m_gameType = value;
		break;
	  case eSIK_official:
		break;
	  case eSIK_anticheat:
		si.m_anticheat = atoi(value)!=0;
		break;
			case eSIK_gamepadsonly:
				si.m_gamepadsonly = atoi(value)!=0;
				break;
	  case eSIK_private:
		si.m_private = atoi(value)!=0;
		break;
			case eSIK_dx10:
				si.m_dx10 = atoi(value)!=0;
				break;
			case eSIK_modName:
				si.m_modName = value;
				break;
			case eSIK_modVersion:
				si.m_modVersion = value;
				break;
	  default:
		basic = false;
	  }

	  if(basic)
	  {
		m_menu->m_ui->UpdateServer(si);
	  }
	}

	if(id != m_pendingUpdate)
	  return;

	switch(key)
	{
	case eSIK_dedicated:
	  m_details.m_dedicated = IsTrue(value);
	  break;
	case eSIK_frinedlyFire:
	  m_details.m_friendlyfire = IsTrue(value);
	  break;
	case eSIK_timelimit:
	  m_details.m_timelimit = atoi(value);
	  break;
		case eSIK_timeleft:
			m_details.m_timeleft = atoi(value);
			break;
	case eSIK_gamever:
	  m_details.m_gamever = value;
	  break;
	case eSIK_gametype:
	  m_details.m_gamemode = value;
	  break;
	case eSIK_voicecomm:
	  m_details.m_voicecomm = IsTrue(value);
	  break;
	case eSIK_anticheat:
	  m_details.m_anticheat = IsTrue(value);
	  break;
		case eSIK_gamepadsonly:
			m_details.m_gamepadsonly = IsTrue(value);
			break;
		case eSIK_modName:
			m_details.m_modname = value;
			break;
		case eSIK_modVersion:
			m_details.m_modversion = value;
			break;
	default:
	  return;
	} 
  }

  virtual void UpdatePlayerValue(const int id,const int playerNum,const char* name,const char* value)
  {
	if(id != m_pendingUpdate)
	  return;
	if(playerNum>=m_details.m_players.size())
	  return;
	CMPLobbyUI::SServerDetails::SPlayerDetails& pl = m_details.m_players[playerNum];
	string val(name);
	EServerInfoKey key = KEY_BY_VALUE(CONST_TEMP_STRING(val),gServerKeyNames);
	switch(key)
	{
	case eSIK_playerName:
	  pl.m_name = value;
	  break;
	case eSIK_playerTeam:
	  pl.m_team = atoi(value);
	  break;
	case eSIK_playerRank:
	  {
		int rank = atoi(value);
		if(rank)
		{
		  pl.m_rank.Format("@ui_short_rank_%d",rank);
		}
		else
		{
		  pl.m_rank="";
		}
				pl.m_iRank = rank;
	  }
	  break;
	case eSIK_playerKills:
	  pl.m_kills = atoi(value);
	  break;
	case eSIK_playerDeaths:
	  pl.m_deaths = atoi(value);
	  break;
	}
  }

  virtual void UpdateTeamValue(const int id,const int teamNum,const char *name,const char* value)
  {
  }

  virtual void OnError(const EServerBrowserError)
  {
  }

  virtual void UpdateComplete(bool cancel)
  {
		m_menu->m_ui->SetUpdateProgress(m_menu->m_browser->GetPendingQueryCount(),m_menu->m_browser->GetServerCount());
		m_menu->m_ui->FinishUpdate();
  }

  virtual void ServerUpdateFailed(const int id)
  {
		if(id == m_pendingUpdate)
		{
			CMPLobbyUI::SServerInfo svr;
			if(m_menu->m_ui->GetServer(id,svr))
			{
				svr.m_ping = 9999;
				m_menu->m_ui->UpdateServer(svr);
				CMPLobbyUI::SServerDetails details;
				details.m_noResponce = true;
				m_menu->m_ui->SetServerDetails(details);
			}
			m_menu->OnRefreshComplete(false);
			m_pendingUpdate = -1;
		}
  }

	static bool CompareScores(const CMPLobbyUI::SServerDetails::SPlayerDetails &a, const CMPLobbyUI::SServerDetails::SPlayerDetails &b)
	{
		if(a.m_iRank > b.m_iRank)
		{
			return true;
		}
		else if (a.m_iRank == b.m_iRank)
		{
			if(a.m_kills>b.m_kills)
			{
				return true;
			}
			else if(a.m_kills == b.m_kills)
			{
				if(a.m_deaths == b.m_deaths)
					return a.m_name < b.m_name;
				else
					return a.m_deaths<b.m_deaths;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

  virtual void ServerUpdateComplete(const int id)
  {
	if(id == m_pendingUpdate)
	{
			std::sort(m_details.m_players.begin(),m_details.m_players.end(),&CompareScores);
	  m_menu->m_ui->SetServerDetails(m_details);
	  m_pendingUpdate = -1;
			m_menu->OnRefreshComplete(true);
	}
  }

  virtual void ServerDirectConnect(bool neednat, uint ip, ushort port)
  {
	string connect;
	if(neednat)
	{
	  int cookie = rand() + (rand()<<16);
			connect.Format("connect <nat>%d|%d.%d.%d.%d:%d",cookie,ip&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);
	  m_menu->m_browser->SendNatCookie(ip,port,cookie);
	}
	else
	{
	  connect.Format("connect %d.%d.%d.%d:%d",ip&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);
	}
	g_pGame->GetIGameFramework()->ExecuteCommandNextFrame(connect.c_str());
  }

  const wstring& GetGameType(const char* gt)
  {
	std::map<string,wstring>::const_iterator it = m_gameTypes.find(CONST_TEMP_STRING(gt));
	if(it==m_gameTypes.end())
	{
	  CryFixedStringT<128>  cType;
	  cType = "@ui_rules_";
	  cType += gt;
	  if(gEnv->pSystem->GetLocalizationManager()->LocalizeLabel(cType,m_tempWString) == false)
			{
				SUIWideString mode(gt);
				m_tempWString = mode.c_str();
			}
	  it = m_gameTypes.insert(std::make_pair(gt,m_tempWString)).first;
	}
	return it->second;
  }

  std::map<string,wstring>     m_gameTypes;

  CMultiPlayerMenu*           m_menu;
  int                         m_pendingUpdate;
  CMPLobbyUI::SServerDetails  m_details;

	string											m_version;//client version
	string											m_modName;
	string											m_modVersion;
	wstring                     m_tempWString;
};

CMultiPlayerMenu::CMultiPlayerMenu(bool lan, IFlashPlayer* plr, CMPHub* hub):
m_browser(0),
m_profile(0),
m_serverlist(new SGSBrowser()),
m_ui(new CUI(plr)),
m_creategame(new SCreateGame(plr,hub,lan)),
m_lan(lan),
m_hub(hub),
m_selectedId(),
m_joiningServer(false)
{
  m_ui->SetMenu(this);
  m_ui->EnableTabs(!lan,!lan,!lan);

  m_profile = m_hub->GetProfile();
 
  INetworkService *serv=GetISystem()->GetINetwork()->GetService("GameSpy");
  if(serv)
  {
	m_browser = serv->GetServerBrowser();
	if(m_browser->IsAvailable())
	{
		m_browser->SetListener(m_serverlist.get());
		m_browser->Start(m_lan);
		m_browser->Update();
	}
	m_serverlist->SetMenu(this);
  }

  m_ui->SetJoinButtonMode(m_hub->IsIngame()?eJBM_disconnect:eJBM_default);
}

CMultiPlayerMenu::~CMultiPlayerMenu()
{
	m_ui->SetStatusString("");
	if(m_hub && m_hub->GetProfile())
		m_hub->GetProfile()->DestroyUI();

	if(m_browser)
	{
		m_browser->SetListener(0);
		m_browser->Stop();
	}
}

bool CMultiPlayerMenu::HandleFSCommand(EGsUiCommand cmd, const char* pArgs)
{
	if(!m_browser)
		return false;

  bool handled = false;

  handled = m_ui->HandleFSCommand(cmd,pArgs) || handled;

  if(m_creategame.get())
	handled = m_creategame->HandleFSCommand(cmd,pArgs) || handled;

  return handled;
}

void CMultiPlayerMenu::OnUIEvent(const SUIEvent& event)
{
  if(event.event == eUIE_connectFailed)
  {
	m_hub->CloseLoadingDlg();
		m_hub->DisconnectError(EDisconnectionCause(event.param),true);
  }
	else if(event.event == eUIE_disconnect)
	{
		if(m_hub->IsInLobby())
			m_ui->EnableResume(false);
		CMPLobbyUI::SServerInfo serv;
		if(m_ui->GetSelectedServer(serv))
		{
			m_ui->SetJoinButtonMode(eJBM_join);
			m_ui->SetJoinButtonMode(eJBM_default);
		}
		else
		{
			m_ui->SetJoinButtonMode(eJBM_hidden);
			m_ui->SetJoinButtonMode(eJBM_default);
		}
	}
}

void CMultiPlayerMenu::UpdateServerList()
{
	switch(m_ui->GetCurTab())
	{
	case 0:
		m_ui->ClearServerList();
		m_ui->StartUpdate();
		m_ui->SetUpdateProgress(0,-1);
		m_browser->Update();
		break;
	case 1:
		break;
	case 2:
		break;
	}
}

void CMultiPlayerMenu::StopServerListUpdate()
{
	m_browser->Stop();
	m_ui->FinishUpdate();
}
  
void CMultiPlayerMenu::SelectServer(int id)
{
	//do feed UI with players&teams info
	m_browser->UpdateServerInfo(id);
}

void CMultiPlayerMenu::JoinServer()
{
	CMPLobbyUI::SServerInfo serv;
	if(m_ui->GetSelectedServer(serv))
	{
		if (m_lan)
		{
			uint ip = serv.m_publicIP;
			string connect;
			connect.Format("connect %d.%d.%d.%d:%d", ip & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF, serv.m_hostPort);
			g_pGame->GetIGameFramework()->ExecuteCommandNextFrame(connect.c_str());
		}
		else
			m_browser->CheckDirectConnect(serv.m_serverId,serv.m_hostPort);
		m_ui->UpdateServer(serv);
		m_hub->ShowLoadingDlg("@ui_connecting_to",serv.m_hostName.c_str());
	}
}

bool CMultiPlayerMenu::CUI::OnHandleCommand(EGsUiCommand cmd, const char* pArgs)
{
  bool handled = true;
  switch(cmd)
  {
  case eGUC_update:
	m_menu->UpdateServerList();
	break;
  case eGUC_stop:
	m_menu->StopServerListUpdate();
	break;
  case eGUC_acceptBuddy:
	break;
  case eGUC_declineBuddy:
	break;
  case eGUC_join:
	{
			if(m_menu->m_hub->IsIngame())
			{
				m_menu->m_hub->ShowYesNoDialog("@ui_disconnect_warning","disconnect_join");
			}
			else
			{
				m_menu->m_joiningServer = true;
				SServerInfo si;
				if(GetSelectedServer(si))
				{
					m_menu->m_hub->ShowLoadingDlg("@ui_connecting_to",si.m_hostName.c_str());
					ResetServerDetails();
					RefreshServer();
				}
			}
	}
	break;
  case eGUC_joinIP:
	{
			m_joinIpCmd.Format("connect %s",pArgs);
			m_joinServerName = pArgs;
			if(m_joinIpCmd[m_joinIpCmd.size()-1]==':')
				m_joinIpCmd = m_joinIpCmd.substr(0,m_joinIpCmd.size()-1);

			if(m_menu->m_hub->IsIngame())
			{
				m_menu->m_hub->ShowYesNoDialog("@ui_disconnect_warning","disconnect_joinIp");
			}
			else
			{
				SetJoinPassword();
				m_menu->m_hub->ShowLoadingDlg("@ui_connecting_to",pArgs);
				g_pGame->GetIGameFramework()->ExecuteCommandNextFrame(m_joinIpCmd.c_str());
			}
	}
	break;
	case eGUC_dialogYes:
		if (!strcmp(pArgs,"disconnect_joinIp"))
		{
			gEnv->pConsole->ExecuteString("disconnect");
			SetJoinPassword();
			m_menu->m_hub->ShowLoadingDlg("@ui_connecting_to", m_joinServerName.c_str());
			g_pGame->GetIGameFramework()->ExecuteCommandNextFrame(m_joinIpCmd.c_str());
		}
		else if (!strcmp(pArgs,"disconnect_join"))
		{
			gEnv->pConsole->ExecuteString("disconnect");
			JoinSelectedServer();
		}
		else
			handled = false;
		break;
  case eGUC_joinBuddy:
	break;
  case eGUC_disconnect:
		g_pGame->GetIGameFramework()->ExecuteCommandNextFrame("disconnect");
	break;
  case eGUC_displayServerList:
	SetUpdateProgress(m_menu->m_browser->GetPendingQueryCount(),m_menu->m_browser->GetServerCount());
	break;
  case eGUC_chat:
	break;
  case eGUC_find:
	break;
  case eGUC_selectServer:

	//break;
  case eGUC_refreshServer:
	{
	  ResetServerDetails();
			m_menu->m_joiningServer = false;
			RefreshServer();
	}
	break;
  default:
	handled = false;
  }
  return handled;
}

void CMultiPlayerMenu::CUI::OnActivateTab(int tab)
{
  switch(tab)
  {
  case 0:
	break;
  case 1:
	break;
  case 2:
	break;
  case 3:
	break;
  }
  m_curTab = tab;
}

void CMultiPlayerMenu::CUI::OnDeactivateTab(int tab)
{
  switch(tab)
  {
  case 0:
	break;
  case 1:
	break;
  case 2:
	break;
  case 3:
	break;
  }  
}

void CMultiPlayerMenu::CUI::OnJoinWithPassword(int id)
{
  SetJoinPassword();
  m_menu->JoinServer();
}

void CMultiPlayerMenu::CUI::JoinSelectedServer()
{
	SServerInfo si;
	if(GetSelectedServer(si))
	{
		if(si.m_private)
		{
			m_menu->m_hub->CloseLoadingDlg();
			OpenPasswordDialog(si.m_serverId);
		}
		else
		{
			ICVar* pV = gEnv->pConsole->GetCVar("sv_password");
			if(pV)
				pV->Set("");
			m_menu->JoinServer();
		}
	}
}

void CMultiPlayerMenu::CUI::RefreshServer()
{
	SServerInfo si;
	if(GetSelectedServer(si))
	{
		m_menu->m_serverlist->m_pendingUpdate = si.m_serverId;
		m_menu->m_serverlist->m_details = CMPLobbyUI::SServerDetails();
		m_menu->m_browser->UpdateServerInfo(si.m_serverId);
	}
}

void CMultiPlayerMenu::OnRefreshComplete(bool ok)
{
	if(m_joiningServer)
	{
		if(ok)
			m_ui->JoinSelectedServer();
		else
			m_hub->CloseLoadingDlg();

		m_joiningServer = false;
	}	
}