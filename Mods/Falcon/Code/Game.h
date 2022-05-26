/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$
  Description: 
  
 -------------------------------------------------------------------------
  History:
  - 3:8:2004   11:23 : Created by M�rcio Martins

*************************************************************************/
#ifndef __GAME_H__
#define __GAME_H__

#if _MSC_VER > 1000
# pragma once
#endif

#include <IGame.h>
#include <IGameFramework.h>
#include <IGameObjectSystem.h>
#include <IGameObject.h>
#include <IActorSystem.h>
#include <StlUtils.h>
#include "ClientSynchedStorage.h"
#include "ServerSynchedStorage.h"
#include "Cry_Camera.h"
#include "ILauncher.h"

#define GAME_NAME				"Crysis Wars"
#define GAME_LONGNAME		"Crysis Wars"


struct ISystem;
struct IConsole;
struct ILCD;

class	CScriptBind_Actor;
class CScriptBind_Item;
class CScriptBind_Weapon;
class CScriptBind_GameRules;
class CScriptBind_Game;
class CScriptBind_HUD;
class CWeaponSystem;
class CFlashMenuObject;
class COptionsManager;

struct IActionMap;
struct IActionFilter;
class  CGameActions;
class CGameRules;
class CBulletTime;
class CHUD;
class CSynchedStorage;
class CClientSynchedStorage;
class CServerSynchedStorage;
struct SCVars;
struct SItemStrings;
class CItemSharedParamsList;
class CSoundMoods;
class CLCDWrapper;
class CDownloadTask;

class CRemoteControlSystem;


// when you add stuff here, also update in CGame::RegisterGameObjectEvents
enum ECryGameEvent
{
	eCGE_PreFreeze = eGFE_PreFreeze,	// this is really bad and must be fixed
	eCGE_PreShatter = eGFE_PreShatter,
	eCGE_PostFreeze = 256,
	eCGE_PostShatter,
	eCGE_OnShoot,
	eCGE_Recoil, 
	eCGE_BeginReloadLoop,
	eCGE_EndReloadLoop,
	eCGE_ActorRevive,
	eCGE_VehicleDestroyed,
	eCGE_TurnRagdoll,
	eCGE_EnableFallAndPlay,
	eCGE_DisableFallAndPlay,
	eCGE_VehicleTransitionEnter,
	eCGE_VehicleTransitionExit,
	eCGE_HUD_PDAMessage,
	eCGE_HUD_TextMessage,
	eCGE_TextArea,
	eCGE_HUD_Break,
	eCGE_HUD_Reboot,
	eCGE_InitiateAutoDestruction,
	eCGE_Event_Collapsing,
	eCGE_Event_Collapsed,
	eCGE_MultiplayerChatMessage,
	eCGE_ResetMovementController,
	eCGE_AnimateHands,
	eCGE_Ragdoll,
	eCGE_EnablePhysicalCollider,
	eCGE_DisablePhysicalCollider,
	eCGE_RebindAnimGraphInputs,
	eCGE_OpenParachute,
  eCGE_Turret_LockedTarget,
  eCGE_Turret_LostTarget,
};

static const int GLOBAL_SERVER_IP_KEY						=	1000;
static const int GLOBAL_SERVER_PUBLIC_PORT_KEY	= 1001;
static const int GLOBAL_SERVER_NAME_KEY					=	1002;


class CGame :
  public IGame, public IGameFrameworkListener
{
public:
  typedef bool (*BlockingConditionFunction)();
	CGame();
	virtual ~CGame();

	// IGame
	virtual bool  Init(IGameFramework *pFramework);
	virtual bool  CompleteInit();
	virtual void  Shutdown();
	virtual int   Update(bool haveFocus, unsigned int updateFlags);
	virtual void  ConfigureGameChannel(bool isServer, IProtocolBuilder *pBuilder);
	virtual void  EditorResetGame(bool bStart);
	virtual void  PlayerIdSet(EntityId playerId);
	virtual string  InitMapReloading();
	virtual bool IsReloading() { return m_bReload; }
	virtual IGameFramework *GetIGameFramework() { return m_pFramework; }

	virtual const char *GetLongName();
	virtual const char *GetName();

	virtual void GetMemoryStatistics(ICrySizer * s);

	virtual void OnClearPlayerIds();
	//auto-generated save game file name
	virtual const char* CreateSaveGameName();
	//level names were renamed without changing the file/directory
	virtual const char* GetMappedLevelName(const char *levelName) const;
	// ~IGame

  // IGameFrameworkListener
  virtual void OnPostUpdate(float fDeltaTime);
  virtual void OnSaveGame(ISaveGame* pSaveGame);
  virtual void OnLoadGame(ILoadGame* pLoadGame);
	virtual void OnLevelEnd(const char* nextLevel) {};
  virtual void OnActionEvent(const SActionEvent& event);
  // ~IGameFrameworkListener

  void BlockingProcess(BlockingConditionFunction f);
  void GameChannelDestroyed(bool isServer);
  void DestroyHUD();

	virtual CScriptBind_Actor *GetActorScriptBind() { return m_pScriptBindActor; }
	virtual CScriptBind_Item *GetItemScriptBind() { return m_pScriptBindItem; }
	virtual CScriptBind_Weapon *GetWeaponScriptBind() { return m_pScriptBindWeapon; }
	virtual CScriptBind_GameRules *GetGameRulesScriptBind() { return m_pScriptBindGameRules; }
	virtual CScriptBind_HUD *GetHUDScriptBind() { return m_pScriptBindHUD; }

	virtual CWeaponSystem *GetWeaponSystem() { return m_pWeaponSystem; };
	virtual CItemSharedParamsList *GetItemSharedParamsList() { return m_pItemSharedParamsList; };

	CGameActions&	Actions() const {	return *m_pGameActions;	};

	CGameRules *GetGameRules() const;
	CSoundMoods *GetSoundMoods() const;
	CHUD *GetHUD() const;
	CFlashMenuObject *GetMenu() const;
	COptionsManager *GetOptions() const;

	ILINE CSynchedStorage *GetSynchedStorage() const
	{
		if (m_pServerSynchedStorage && gEnv->bServer)
			return m_pServerSynchedStorage;

		return m_pClientSynchedStorage;
	}

	ILINE CServerSynchedStorage *GetServerSynchedStorage() const
	{
		return m_pServerSynchedStorage;
	}

	const string& GetLastSaveGame(string &levelName);
	const string& GetLastSaveGame() { string tmp; return GetLastSaveGame(tmp); }

	ILINE SCVars *GetCVars() {return m_pCVars;}

	CDownloadTask* GetDownloadTask() const { return m_pDownloadTask; }

	virtual void SetDX10Fix(int state);
	virtual int GetDX10Fix();

protected:
	virtual void LoadActionMaps(const char* filename = "libs/config/defaultProfile.xml");

	virtual void ReleaseActionMaps();

	virtual void InitScriptBinds();
	virtual void ReleaseScriptBinds();

	virtual void CheckReloadLevel();

	// These funcs live in GameCVars.cpp
	virtual void RegisterConsoleVars();
	virtual void RegisterConsoleCommands();
	virtual void UnregisterConsoleCommands();

	virtual void RegisterGameObjectEvents();

	// marcok: this is bad and evil ... should be removed soon
	static void CmdRestartGame(IConsoleCmdArgs *pArgs);

	static void CmdLastInv(IConsoleCmdArgs *pArgs);
	static void CmdTeam(IConsoleCmdArgs *pArgs);
	static void CmdSpectator(IConsoleCmdArgs *pArgs);
	static void CmdJoinGame(IConsoleCmdArgs *pArgs);
	static void CmdKill(IConsoleCmdArgs *pArgs);
	static void CmdRestart(IConsoleCmdArgs *pArgs);
	static void CmdSay(IConsoleCmdArgs *pArgs);
	static void CmdReloadItems(IConsoleCmdArgs *pArgs);
	static void CmdLoadActionmap(IConsoleCmdArgs *pArgs);
  static void CmdNextLevel(IConsoleCmdArgs* pArgs);

  static void CmdQuickGame(IConsoleCmdArgs* pArgs);
  static void CmdQuickGameStop(IConsoleCmdArgs* pArgs);
  static void CmdBattleDustReload(IConsoleCmdArgs* pArgs);
  static void CmdLogin(IConsoleCmdArgs* pArgs);
	static void CmdLoginProfile(IConsoleCmdArgs* pArgs);
	static void CmdRegisterNick(IConsoleCmdArgs* pArgs);
  static void CmdCryNetConnect(IConsoleCmdArgs* pArgs);

  static void CmdOnCustomCCommandExecuted(IConsoleCmdArgs* pArgs);
	static void CmdChat(IConsoleCmdArgs* pArgs);
	static void CmdChatTeam(IConsoleCmdArgs* pArgs);
	static void CmdRconSc(IConsoleCmdArgs* pArgs);
	static void CmdRconScl(IConsoleCmdArgs* pArgs);
	//vader mod
	static void CmdFnThirdPerson(IConsoleCmdArgs* pArgs);

	IGameFramework			*m_pFramework;
	IConsole						*m_pConsole;

	CWeaponSystem				*m_pWeaponSystem;

	bool								m_bReload;
	int m_dx10Fix;

	// script binds
	CScriptBind_Actor		*m_pScriptBindActor;
	CScriptBind_Item		*m_pScriptBindItem;
	CScriptBind_Weapon	*m_pScriptBindWeapon;
	CScriptBind_GameRules*m_pScriptBindGameRules;
	CScriptBind_Game    *m_pScriptBindGame;
	CScriptBind_HUD     *m_pScriptBindHUD;

	//menus
	CFlashMenuObject		*m_pFlashMenuObject;
	COptionsManager			*m_pOptionsManager;

	IActionMap					*m_pDefaultAM;
	IActionMap					*m_pMultiplayerAM;
	CGameActions				*m_pGameActions;	
	IPlayerProfileManager* m_pPlayerProfileManager;
	CHUD								*m_pHUD;

	CServerSynchedStorage	*m_pServerSynchedStorage;
	CClientSynchedStorage	*m_pClientSynchedStorage;
	bool								m_inDevMode;

	EntityId m_uiPlayerID;

	SCVars*	m_pCVars;
	SItemStrings					*m_pItemStrings;
	CItemSharedParamsList *m_pItemSharedParamsList;
	string                 m_lastSaveGame;
	string								 m_newSaveGame;

	CSoundMoods						*m_pSoundMoods;

	typedef std::map<string, string, stl::less_stricmp<string> > TLevelMapMap;
	TLevelMapMap m_mapNames;

	CDownloadTask					*m_pDownloadTask;

	ILauncher* m_pLauncher;
	CRemoteControlSystem *m_pRemoteControlSystem;

public:
	//  IGame.h
	CRemoteControlSystem *GetRemoteControlSystem() const { return m_pRemoteControlSystem; }
	ILauncher* GetLauncher() const { return m_pLauncher; }
};

extern CGame *g_pGame;

#define SAFE_HARDWARE_MOUSE_FUNC(func)\
	if(gEnv->pHardwareMouse)\
		gEnv->pHardwareMouse->func

#define SAFE_MENU_FUNC(func)\
	{	if(g_pGame && g_pGame->GetMenu()) g_pGame->GetMenu()->func; }

#define SAFE_MENU_FUNC_RET(func)\
	((g_pGame && g_pGame->GetMenu()) ? g_pGame->GetMenu()->func : 0)

#define SAFE_HUD_FUNC(func)\
	{	if(g_pGame && g_pGame->GetHUD()) g_pGame->GetHUD()->func; }

#define SAFE_HUD_FUNC_RET(func)\
	((g_pGame && g_pGame->GetHUD()) ? g_pGame->GetHUD()->func : 0)

#define SAFE_SOUNDMOODS_FUNC(func)\
	{	if(g_pGame && g_pGame->GetSoundMoods()) g_pGame->GetSoundMoods()->func; }

#define CRAPDOLLS

#endif //__GAME_H__