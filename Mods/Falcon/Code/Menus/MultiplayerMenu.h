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
#ifndef __MULTIPLAYERMENU_H__
#define __MULTIPLAYERMENU_H__

#pragma once

#include "IFlashPlayer.h"
#include "MPHub.h"

struct  IServerBrowser;
struct  INetworkChat;
class   CGameNetworkProfile;
#include "MPLobbyUI.h"

class CMultiPlayerMenu
{
private:
	struct SGSBrowser;
	struct SCreateGame;
	class CUI;

public:
	CMultiPlayerMenu(bool lan, IFlashPlayer* plr, CMPHub* hub);
	~CMultiPlayerMenu();
	bool HandleFSCommand(EGsUiCommand cmd, const char* pArgs);
	void OnUIEvent(const SUIEvent& event);

private:
	void UpdateServerList();
	void StopServerListUpdate();
	void SelectServer(int id);
	void JoinServer();
	void OnRefreshComplete(bool ok);

	IServerBrowser* m_browser;
	CGameNetworkProfile* m_profile;
	std::auto_ptr<CUI> m_ui;
	std::auto_ptr<SGSBrowser> m_serverlist;
	std::auto_ptr<SCreateGame> m_creategame;
	bool m_lan;
	CMPHub* m_hub;
	bool m_joiningServer;
	int m_selectedId;
};
#endif /*__MULTIPLAYERMENU_H__*/
