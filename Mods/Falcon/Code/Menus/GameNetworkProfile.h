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

#ifndef __GAMENETWORKPROFILE_H__
#define __GAMENETWORKPROFILE_H__

#pragma once

#include "INetworkService.h"

class CMPHub;

struct INProfileUI
{
	virtual void ShowError(const char* descr) = 0;
};

class CGameNetworkProfile
{
private:
	struct SProfileListener;

public:
	CGameNetworkProfile(CMPHub* hub);
	~CGameNetworkProfile();

	void Login(const char* login, const char* password);
	void LoginProfile(const char* email, const char* password, const char* profile);
	void Register(const char* login, const char* email, const char* pass, const char* country, SRegisterDayOfBirth dob);
	void Logoff();

	void InitUI(INProfileUI* a);
	void DestroyUI();

	bool IsLoggedIn()const;
	bool IsLoggingIn()const;
	const char* GetLogin()const;
	const char* GetPassword()const;
	const char* GetCountry()const;
	const int GetProfileId()const;
	bool IsDead()const;

private:
	void OnLoggedIn(int id, const char* nick);

private:
	CMPHub* m_hub;
	INetworkProfile* m_profile;
	string m_login;
	string m_password;
	string m_country;
	int m_profileId;
	bool m_loggingIn;

	std::auto_ptr<SProfileListener> m_profileListener;
};

#endif //__GAMENETWORKPROFILE_H__
