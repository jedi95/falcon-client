#ifndef __GAMECVARS_H__
#define __GAMECVARS_H__

enum EExplosiveType
{
	eET_All = -1,
	eET_Claymore,
	eET_AVMine,
	eET_C4,
	eET_LaunchedGrenade,
	eET_NumTypes,
};

struct SCVars
{
	// Falcon system CVARs
	ICVar* fn_build;
	ICVar* fn_version;
	int fn_svFalcon;

	// RCON
	ICVar* fn_rconClientConsoleLineFormat;

	// Falcon server controlled
	float fn_c4ThrowVelocityMultiplier;
	int fn_circleJump;
	int fn_disableFreefall;
	int fn_fastWeaponMenu;
	int fn_fastWeaponSwitch;
	int fn_playerLeaning;
	int fn_radarClearOnDeath;
	int fn_simpleGameMechanics;
	float fn_wallJumpMultiplier;
	float fn_weaponMassMultiplier;

	// Falcon client options
	int fn_constantMouseSensitivity;
	int fn_crouchToggle;
	int fn_disableShootZoom;
	int fn_enableFpBody;
	int fn_fixExplosivePlant;
	int fn_fixHUD;
	float fn_fov;
	int fn_zoomToggle;
	int sys_MaxFps;

	// State only
	int fn_fpBody;

	static const float v_altitudeLimitDefault()
	{
		return 600.0f;
	}

	float cl_fov;
	float cl_bob;
	float cl_headBob;
	float cl_headBobLimit;
	float cl_tpvDist;
	float cl_tpvYaw;
	float cl_sprintShake;
	float cl_sensitivityZeroG;
	float cl_sensitivity;
	float cl_controllersensitivity;
	float cl_strengthscale;
	int		cl_invertMouse;
	int		cl_invertController;
	int		cl_crouchToggle;
	int		cl_fpBody;
	int   cl_hud;

	ICVar* 	ca_GameControlledStrafingPtr;
	float pl_curvingSlowdownSpeedScale;
	float ac_enableProceduralLeaning;

	float cl_shallowWaterSpeedMulPlayer;
	float cl_shallowWaterSpeedMulAI;
	float cl_shallowWaterDepthLo;
	float cl_shallowWaterDepthHi;

	float cl_frozenSteps;
	float cl_frozenSensMin;
	float cl_frozenSensMax;
	float cl_frozenAngleMin;
	float cl_frozenAngleMax;
	float cl_frozenMouseMult;
	float cl_frozenKeyMult;
	float cl_frozenSoundDelta;
	int		goc_enable;
	int		goc_tpcrosshair;
	float goc_targetx;
	float goc_targety;
	float goc_targetz;

	int		dt_enable;
	float dt_time;
	float dt_meleeTime;

	int   sv_input_timeout;

	float hr_rotateFactor;
	float hr_rotateTime;
	float hr_dotAngle;
	float hr_fovAmt;
	float hr_fovTime;

	int		i_staticfiresounds;
	int		i_soundeffects;
	int		i_lighteffects;
	int		i_particleeffects;
	int		i_rejecteffects;
	float i_offset_front;
	float i_offset_up;
	float i_offset_right;
	int		i_unlimitedammo;
	int   i_iceeffects;
	int		i_lighteffectsShadows;
  
	float int_zoomAmount;
	float int_zoomInTime;
	float int_moveZoomTime;
	float int_zoomOutTime;

	float pl_inputAccel;

	float g_tentacle_joint_limit;
	int		g_detachCamera;
	int		g_suicideDelay;
	int		g_enableSpeedLean;
	int   g_difficultyLevel;
	int		g_difficultyHintSystem;
	float g_difficultyRadius;
	int		g_difficultyRadiusThreshold;
	int		g_difficultySaveThreshold;
	int		g_playerHealthValue;
	float g_walkMultiplier;
	float g_suitRecoilEnergyCost;
	float g_suitSpeedMult;
	float g_suitSpeedMultMultiplayer;
	float g_suitArmorHealthValue;
	float g_suitSpeedEnergyConsumption;
	float g_suitSpeedEnergyConsumptionMultiplayer;
	float g_suitCloakEnergyDrainAdjuster;
	float g_AiSuitEnergyRechargeTime;
	float g_AiSuitHealthRegenTime;
	float g_AiSuitArmorModeHealthRegenTime;
	float g_playerSuitEnergyRechargeTime;
	float g_playerSuitEnergyRechargeTimeArmor;
	float g_playerSuitEnergyRechargeTimeArmorMoving;
	float g_playerSuitEnergyRechargeTimeMultiplayer;
	float g_playerSuitEnergyRechargeDelay;
	float g_playerSuitHealthRegenTime;
	float g_playerSuitHealthRegenTimeMoving;
	float g_playerSuitArmorModeHealthRegenTime;
	float g_playerSuitArmorModeHealthRegenTimeMoving;
	float g_playerSuitHealthRegenDelay;
	float g_frostDecay;
	float g_stanceTransitionSpeed;
	float g_stanceTransitionSpeedSecondary;
	int		g_playerRespawns;
	float g_playerLowHealthThreshold;
	float g_playerLowHealthThreshold2;
	float g_playerLowHealthThresholdMultiplayer;
	float g_playerLowHealthThreshold2Multiplayer;
	int		g_punishFriendlyDeaths;
	int		g_enableMPStealthOMeter;
	int   g_meleeWhileSprinting;
	float g_AiSuitStrengthMeleeMult;
	float g_fallAndPlayThreshold;
	int		g_useHitSoundFeedback;

	int		g_meleeDamage;
	int		g_meleeStrenghtDamage;

	int sv_pacifist;

	int g_empStyle;
	float g_empNanosuitDowntime;

	float g_pp_scale_income;
	float g_pp_scale_price;
	float g_energy_scale_income;
	float g_energy_scale_price;

	float g_dofset_minScale;
	float g_dofset_maxScale;
	float g_dofset_limitScale;

	float g_dof_minHitScale;
	float g_dof_maxHitScale;
	float g_dof_sampleAngle;
	float g_dof_minAdjustSpeed;
	float g_dof_maxAdjustSpeed;
	float g_dof_averageAdjustSpeed;
	float g_dof_distAppart;
	int		g_dof_ironsight;

	// explosion culling
	int		g_ec_enable;
	float g_ec_radiusScale;
	float g_ec_volume;
	float g_ec_extent;
	int		g_ec_removeThreshold;

	float g_radialBlur;
	int		g_playerFallAndPlay;

	float g_timelimit;
	int		g_teamlock;
	float g_roundtime;
	int		g_preroundtime;
	int		g_suddendeathtime;
	int		g_roundlimit;
	int		g_fraglimit;
  int		g_fraglead;
	int		g_scorelimit;
	int		g_scorelead;
	int		g_spawn_force_timeout;
	int		g_spawnteamdist;
	float	g_spawnenemydist;
	float	g_spawndeathdist;

  float g_friendlyfireratio;
	float g_friendlyVehicleCollisionRatio;
  int   g_revivetime; 
  int   g_autoteambalance;
	int		g_autoteambalance_threshold;
	int   g_minplayerlimit;
	int   g_minteamlimit;
	int		g_mpSpeedRechargeDelay;

	int   g_tk_punish;
	int		g_tk_punish_limit;

	float g_trooperProneMinDistance;
	float g_trooperTentacleAnimBlend;
	float g_trooperBankingMultiplier;
	float g_alienPhysicsAnimRatio;  

	int		g_resetActionmapOnStart;
	int		g_useProfile;
	int		g_startFirstTime;
	int		g_enableAutoSave;

	int   g_enableTracers;
	int		g_enableAlternateIronSight;

	float	g_ragdollMinTime;
	float	g_ragdollUnseenTime;
	float	g_ragdollPollTime;
	float	g_ragdollDistance;

	int   v_pa_surface;
	int   v_invertPitchControl;
	float v_wind_minspeed;
	float v_sprintSpeed;
	int   v_rockBoats;
	float v_altitudeLimit;
	ICVar* pAltitudeLimitCVar;
	float v_altitudeLimitLowerOffset;
	ICVar* pAltitudeLimitLowerOffsetCVar;
	float v_airControlSensivity;
	float v_stabilizeVTOL;
	int   v_help_tank_steering;
	ICVar* pVehicleQuality;
	int		v_newBrakingFriction;
	int		v_newBoost;

	float pl_swimBaseSpeed;
	float pl_swimBackSpeedMul;
	float pl_swimSideSpeedMul;
	float pl_swimVertSpeedMul;
	float pl_swimNormalSprintSpeedMul;
	float pl_swimSpeedSprintSpeedMul;
	float pl_swimUpSprintSpeedMul;
	float pl_swimJumpStrengthCost;
	float pl_swimJumpStrengthSprintMul;
	float pl_swimJumpStrengthBaseMul;
	float pl_swimJumpSpeedCost;
	float pl_swimJumpSpeedSprintMul;
	float pl_swimJumpSpeedBaseMul;

	float pl_fallDamage_Normal_SpeedSafe;
	float pl_fallDamage_Normal_SpeedFatal;
	float pl_fallDamage_Strength_SpeedSafe;
	float pl_fallDamage_Strength_SpeedFatal;
	float pl_fallDamage_SpeedBias;
	
	float pl_zeroGSpeedMultSpeed;
	float pl_zeroGSpeedMultSpeedSprint;
	float pl_zeroGSpeedMultNormal;
	float pl_zeroGSpeedMultNormalSprint;
	float pl_zeroGUpDown;
	float pl_zeroGBaseSpeed;
	float pl_zeroGSpeedMaxSpeed;
	float pl_zeroGSpeedModeEnergyConsumption;
	float	pl_zeroGDashEnergyConsumption;
	int		pl_zeroGSwitchableGyro;
	int		pl_zeroGEnableGBoots;
	float pl_zeroGThrusterResponsiveness;
	float pl_zeroGFloatDuration;
	int		pl_zeroGParticleTrail;
	int		pl_zeroGEnableGyroFade;
	float pl_zeroGGyroFadeAngleInner;
	float pl_zeroGGyroFadeAngleOuter;
	float pl_zeroGGyroFadeExp;
	float pl_zeroGGyroStrength;
	float pl_zeroGAimResponsiveness;

	int		hud_mpNamesDuration;
	int		hud_mpNamesNearDistance;
	int		hud_mpNamesFarDistance;
	int		hud_onScreenNearDistance;
	int		hud_onScreenFarDistance;
	float	hud_onScreenNearSize;
	float	hud_onScreenFarSize;
	int		hud_colorLine;
	int		hud_colorOver;
	int		hud_colorText;
	int		hud_voicemode;
	int		hud_enableAlienInterference;
	float	hud_alienInterferenceStrength;
	int		hud_crosshair_enable;
	int		hud_crosshair;
	int		hud_chDamageIndicator;
	int		hud_showAllObjectives;
	int		hud_showObjectiveMessages;
	int		hud_showTeamIcons;
	int		hud_showRadarObjectiveIcons;
	int		hud_showGrenadeIcons;
	int		hud_showFunMessages;
	int		hud_showRoundMessages;
	int		hud_showTeamMessages;
	int		hud_showKillMessages;
	int		hud_subtitles;
	int   hud_subtitlesRenderMode;
	int   hud_subtitlesHeight;
	int   hud_subtitlesFontSize;
	int   hud_subtitlesShowCharName;
	int   hud_subtitlesQueueCount;
	int   hud_subtitlesVisibleCount;
	int		hud_radarBackground;
	float	hud_radarJammingThreshold;
	float	hud_radarJammingEffectScale;
	int		hud_aspectCorrection;
	float hud_ctrl_Curve_X;
	float hud_ctrl_Curve_Z;
	float hud_ctrl_Coeff_X;
	float hud_ctrl_Coeff_Z;
	int		hud_ctrlZoomMode;
	int		hud_attachBoughtEquipment;
	float hud_nightVisionRecharge;
	float hud_nightVisionConsumption;
	int		hud_showBigVehicleReload;
	float hud_binocsScanningDelay;
	float hud_binocsScanningWidth;
	//new crosshair spread code (Julien)
	float hud_fAlternateCrosshairSpreadCrouch;
	float hud_fAlternateCrosshairSpreadNeutral;
	int hud_iAlternateCrosshairSpread;
	int hud_creategame_pb_server;

	float aim_assistMaxDistance;
	float aim_assistVerticalScale;

	float g_combatFadeTime;
	float g_combatFadeTimeDelay;
	float g_battleRange;

	float	tracer_min_distance;
	float	tracer_max_distance;
	float	tracer_min_scale;
	float	tracer_max_scale;
	int		tracer_max_count;
	float	tracer_player_radiusSqr;
	int		i_auto_turret_target;
	int		i_auto_turret_target_tacshells;
  
	float h_turnSpeed;
	int		h_useIK;

  ICVar*  g_quickGame_map;
  ICVar*  g_quickGame_mode;
  int     g_quickGame_min_players;
  int     g_quickGame_prefer_lan;
  int     g_quickGame_prefer_favorites;
  int     g_quickGame_prefer_my_country;
  int     g_quickGame_ping1_level;
  int     g_quickGame_ping2_level;
	int			g_skip_tutorial;

  int     g_displayIgnoreList;
  int     g_buddyMessagesIngame;

  int			g_battleDust_enable;
	ICVar*  g_battleDust_effect;

	int			g_PSTutorial_Enabled;

	int			g_proneAimAngleRestrict_Enable;
	int			g_enableFriendlyFallAndPlay;

	int			g_spectate_TeamOnly;
	int			g_spectate_FixedOrientation;
	float		g_spectate_FreeLookMoveSpeed;
	int			g_explosiveLimits[eET_NumTypes];
	int			g_deathCam;
	float		g_deathCamMaxZoomFOV;
	int			g_deathCamMinZoomDistance;
	int			g_deathCamMaxZoomDistance;
	int			g_deathEffects;

	ICVar*	i_restrictItems;	// list of item names which aren't allowed in this MP game.
	int			g_spawnProtectionTime;
	int			g_roundRestartTime;

	ICVar*  net_mapDownloadURL;

	float		g_painSoundGap;
	float		g_explosionScreenShakeMultiplier;

	SCVars()
	{
		memset(this,0,sizeof(SCVars));
	}

	~SCVars() { ReleaseCVars(); }

	void InitCVars(IConsole *pConsole);
	void ReleaseCVars();

protected:
	static void RestrictedItemsChanged( ICVar* var );

	static void SetupAdmins(ICVar *pVar);
	static void SetupModerators(ICVar *pVar);
	static void SetupCVarsToUnlock(ICVar *pVar);

};

#endif //__GAMECVARS_H__
