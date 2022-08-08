/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2007.
-------------------------------------------------------------------------
$Id$
$DateTime$
Description: Game-side part of Player's network profile

-------------------------------------------------------------------------
History:
- 03/2007: Created by Stas Spivakov

*************************************************************************/
#include "StdAfx.h"
#include "INetwork.h"
#include "GameNetworkProfile.h"
#include "MPHub.h"

#include "Game.h"
#include "GameCVars.h"
#include "HUD/HUD.h"
#include "HUD/HUDRadar.h"
#include "HUD/HUDTextChat.h"

static TKeyValuePair<ENetworkProfileError,const char*>
gProfileErrors[] = {	{eNPE_ok,""},
											{eNPE_connectFailed,"@ui_menu_connectFailed"},
											{eNPE_disconnected,"@ui_menu_disconnected"},
											{eNPE_loginFailed,"@ui_menu_loginFailed"},
											{eNPE_loginTimeout,"@ui_menu_loginTimeout"},
											{eNPE_anotherLogin,"@ui_menu_anotherLogin"},
											{eNPE_nickTaken,"@ui_menu_nickTaken"},
											{eNPE_registerAccountError,"@ui_menu_registerAccountError"},
											{eNPE_registerGeneric,"@ui_menu_registerGeneric"},
											{eNPE_nickTooLong,"@ui_menu_nickTooLong"},
											{eNPE_nickFirstNumber,"@ui_menu_nickFirstNumber"},
											{eNPE_nickSlash,"@ui_menu_nickSlash"},
											{eNPE_nickFirstSpecial,"@ui_menu_nickFirstSpecial"},
											{eNPE_nickNoSpaces,"@ui_menu_nickNoSpaces"},
											{eNPE_nickEmpty,"@ui_menu_nickEmpty"},
											{eNPE_profileEmpty,"@ui_menu_profileEmpty"},
											{eNPE_passTooLong,"@ui_menu_passTooLong"},
											{eNPE_passEmpty,"@ui_menu_passEmpty"},
											{eNPE_mailTooLong,"@ui_menu_mailTooLong"},
											{eNPE_mailEmpty,"@ui_menu_mailEmpty"}
};

struct CGameNetworkProfile::SProfileListener : public INetworkProfileListener
{
	SProfileListener(CGameNetworkProfile* p):m_parent(p),m_ui(0)
	{}

	void LoginResult(ENetworkProfileError res, const char* descr, int id, const char* nick)
	{
		if(res == eNPE_ok)
		{
			m_parent->OnLoggedIn(id, nick);
		}
		else
		{
			m_parent->m_loggingIn = false;
			const char* err = VALUE_BY_KEY(res,gProfileErrors);
			m_parent->m_hub->OnLoginFailed(err);
		}
	}

	void OnError(ENetworkProfileError res, const char* descr)
	{
		bool logoff = false;
		switch(res)
		{
			case eNPE_connectFailed:
			case eNPE_disconnected:
			case eNPE_anotherLogin:
				{
					const char* err = VALUE_BY_KEY(res,gProfileErrors);
					m_parent->m_hub->ShowError(err,true);
					logoff = true;
				}
				break;
		}

		if(m_parent->m_loggingIn && m_parent->m_hub)
			m_parent->m_hub->CloseLoadingDlg();

		if(logoff && m_parent->m_hub)
			m_parent->m_hub->DoLogoff();
	}

	// Need these to satisfy the interface, but they do nothing
	void OnProfileInfo(int id, const char* key, const char* value) {}
	virtual void AddNick(const char* nick) {}
	virtual void UpdateFriend(int id, const char* nick, EUserStatus, const char* status, bool foreign) {}
	virtual void RemoveFriend(int id) {}
	virtual void OnFriendRequest(int id, const char* message) {}
	virtual void OnMessage(int id, const char* message) {}
	virtual void RetrievePasswordResult(bool ok) {}
	virtual void OnProfileComplete(int id) {}
	virtual void OnSearchResult(int id, const char* nick) {}
	virtual void OnSearchComplete() {}
	virtual void OnUserId(const char* nick, int id) {}
	virtual void OnUserNick(int id, const char* nick, bool foreign_name) {}

	void UIActivated(INProfileUI* ui)
	{
		m_ui = ui;
		if(!m_ui)
			return;
	}

	CGameNetworkProfile* m_parent;
	INProfileUI* m_ui;
};

CGameNetworkProfile::CGameNetworkProfile(CMPHub* hub):
m_hub(hub),m_loggingIn(false),m_profileId(-1)
{
	INetworkService *serv = gEnv->pNetwork->GetService("GameSpy");
	if(serv)
	{
		m_profile = serv->GetNetworkProfile();
	}
	else
	{
		m_profile = 0;
	}
}

CGameNetworkProfile::~CGameNetworkProfile()
{
  if(m_profile)
	 m_profile->RemoveListener(m_profileListener.get());
}

void CGameNetworkProfile::Login(const char* login, const char* password)
{
	m_login = login;
	m_password = password;
	m_loggingIn = true;
	m_profileListener.reset(new SProfileListener(this));
	m_profile->AddListener(m_profileListener.get());
	m_profile->Login(login,password);
}

void CGameNetworkProfile::LoginProfile(const char* email, const char* password, const char* profile)
{
	m_login = profile;
	m_password = password;
	m_loggingIn = true;
	m_profileListener.reset(new SProfileListener(this));
	m_profile->AddListener(m_profileListener.get());
	m_profile->LoginProfile(email,password,profile);
}

void CGameNetworkProfile::Register(const char* login, const char* email, const char* pass, const char* country, SRegisterDayOfBirth dob)
{
	m_login = login;
	m_password = pass;
	m_loggingIn = true;
	m_profileListener.reset(new SProfileListener(this));
	m_profile->AddListener(m_profileListener.get());
	m_profile->Register(login,email,pass,country, dob);
}

void CGameNetworkProfile::Logoff()
{
	m_loggingIn = false;
	m_profile->Logoff();
	m_profile->RemoveListener(m_profileListener.get());
	m_profileListener.reset(0);
}

void CGameNetworkProfile::InitUI(INProfileUI* a)
{
	if(m_profileListener.get())
		m_profileListener->UIActivated(a);
}

void CGameNetworkProfile::DestroyUI()
{
	if(m_profileListener.get())
		m_profileListener->m_ui = 0;
}

bool CGameNetworkProfile::IsLoggedIn()const
{
  return m_profile->IsLoggedIn() && !m_loggingIn;
}

bool CGameNetworkProfile::IsLoggingIn()const
{
	return m_loggingIn;
}

const char* CGameNetworkProfile::GetLogin()const
{
	return m_login.c_str();
}

const char* CGameNetworkProfile::GetPassword()const
{
	return m_password.c_str();
}

const char* CGameNetworkProfile::GetCountry()const
{
	return m_country.c_str();
}

const int CGameNetworkProfile::GetProfileId()const
{
	return m_profileId;
}

void CGameNetworkProfile::OnLoggedIn(int id, const char* nick)
{
	m_profileId = id;
	m_login = nick;
	m_loggingIn = false;
	m_hub->OnLoginSuccess(m_login);
}

bool CGameNetworkProfile::IsDead() const
{
	return m_profile == 0;
}