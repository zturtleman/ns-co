// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_main.c -- initialization and primary entry point for cgame

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

#ifdef MISSIONPACK
#include "../ui/ui_shared.h"
// display context for new ui stuff
displayContextDef_t cgDC;
#endif

//int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );

int CG_LastAttacker();
/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

    switch ( command ) {
    case CG_INIT:
        // big thanks to apoxol! on 1.3.01
        // Apoxol: Ok, huge hack here..  This will prevent any console messages from
        // being displayed on the screen.  This is so we can move the chat window
        CG_Init( arg0, arg1, arg2 );
        return 0;
    case CG_SHUTDOWN:
        CG_Shutdown();
        return 0;
    case CG_CONSOLE_COMMAND:
        return CG_ConsoleCommand();
    case CG_DRAW_ACTIVE_FRAME:
        CG_DrawActiveFrame( arg0, arg1, arg2 );
        return 0;
    case CG_CROSSHAIR_PLAYER:
        return CG_CrosshairPlayer();
    case CG_LAST_ATTACKER:
        return CG_LastAttacker();
    case CG_KEY_EVENT:
        CG_KeyEvent(arg0, arg1);
        return 0;
    case CG_MOUSE_EVENT:
#ifdef MISSIONPACK
        cgDC.cursorx = cgs.cursorX;
        cgDC.cursory = cgs.cursorY;
#endif
        CG_MouseEvent(arg0, arg1);
        return 0;
    case CG_EVENT_HANDLING:
        CG_EventHandling(arg0);
        return 0;
    default:
        CG_Error( "vmMain: unknown command %i", command );
        break;
    }
    return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[WP_NUM_WEAPONS*2];//MAX_WEAPONS]; had to fix this
itemInfo_t			cg_items[MAX_ITEMS];


vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runyaw;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_bobyaw;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_gibs;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
vmCvar_t	cg_drawAmmoWarning;
vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_crosshairHealth;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_animSpeed;
vmCvar_t	cg_debugAnim;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_noPlayerAnims;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_footsteps;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_brassTime;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_tracerChance;
vmCvar_t	cg_tracerWidth;
vmCvar_t	cg_tracerLength;
vmCvar_t	cg_autoswitch;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_drawAttacker;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_teamChatHeight;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
//vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_blood;
//vmCvar_t	cg_predictItems;
vmCvar_t	cg_deferPlayers;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_noVoiceChats;
vmCvar_t	cg_noVoiceText;
vmCvar_t	cg_hudFiles;
vmCvar_t 	cg_scorePlum;
vmCvar_t 	cg_smoothClients;
vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_cameraOrbit;
vmCvar_t	cg_cameraOrbitDelay;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;

// Navy Seals ++
vmCvar_t	test_x;
vmCvar_t	test_y;
vmCvar_t	test_h;
vmCvar_t	test_w;
vmCvar_t	cg_gunSmoke;
vmCvar_t	pmodel_o;
vmCvar_t	cg_enableDust;
vmCvar_t	cg_enableBreath;
vmCvar_t	cg_enableTimeSelect;
vmCvar_t	cg_atmosphericEffects;
vmCvar_t	cg_lowEffects;
vmCvar_t	cg_customTracerColor;
vmCvar_t	cg_gunSmokeTime;
vmCvar_t	cg_particleTime;

vmCvar_t	cg_crosshair_r;
vmCvar_t	cg_crosshair_g;
vmCvar_t	cg_crosshair_b;

// character cvars
vmCvar_t	char_stamina;
vmCvar_t	char_stealth;
vmCvar_t	char_speed;
vmCvar_t	char_accuracy;
vmCvar_t	char_technical;
vmCvar_t	char_strength;
vmCvar_t	char_xp;

vmCvar_t	ui_sealplayers;
vmCvar_t	ui_tangoplayers;
vmCvar_t	ui_players;
vmCvar_t	ui_sealpoints;
vmCvar_t	ui_tangopoints;
vmCvar_t	ui_team;

vmCvar_t	ui_assaultstate;
vmCvar_t	ui_bombstate;

vmCvar_t	mi_viprescue;
vmCvar_t	mi_viptime;
vmCvar_t	mi_blowup;
vmCvar_t	mi_assaultfield;

vmCvar_t	ui_gotbomb;
vmCvar_t	ui_isvip;
vmCvar_t	ui_gotbriefcase;

vmCvar_t	cg_bulletTracerLength;
vmCvar_t	cg_bulletTracerWidth;

vmCvar_t	cg_wakemarkTime;
vmCvar_t	cg_wakemarkDistantTime;

vmCvar_t	cg_grenadeSparks;
vmCvar_t	cg_correctgunFov;

vmCvar_t	cg_isgun_z;
vmCvar_t	cg_isgun_y;
vmCvar_t	cg_isgun_x;
vmCvar_t	cg_isgun_roll;
vmCvar_t	cg_isgun_yaw;
vmCvar_t	cg_isgun_pitch;
vmCvar_t	cg_isgun_step;

vmCvar_t	cg_newbeeHeight;
vmCvar_t	cg_newbeeTime;
vmCvar_t	cg_viewHeight;

// for the hud icons.
vmCvar_t	ui_gotbriefcase;
vmCvar_t	ui_isvip;
vmCvar_t	ui_gotbomb;
vmCvar_t	mi_blowup;
vmCvar_t	mi_viprescue;
vmCvar_t	mi_viptime;
vmCvar_t	mi_assaultfield;
vmCvar_t	ui_assaultstate;
vmCvar_t	ui_bombstate;

vmCvar_t	ui_roundtimerworld;
vmCvar_t	ui_viptimer;
vmCvar_t	ui_assaulttimer1;
vmCvar_t	ui_assaulttimer2;
vmCvar_t	ui_assaulttimer3;
vmCvar_t	ui_assaulttimer4;
vmCvar_t	ui_bombtimer;

vmCvar_t	ui_assaultblocked1;
vmCvar_t	ui_assaultblocked2;
vmCvar_t	ui_assaultblocked3;
vmCvar_t	ui_assaultblocked4;

vmCvar_t	cg_disableHeadstuff;
vmCvar_t	cg_disableTangoHandSkin;
vmCvar_t    raise_acc;
vmCvar_t    raise_spd;
vmCvar_t    raise_str;
vmCvar_t    raise_stl;
vmCvar_t    raise_sta;
vmCvar_t    raise_tec;

vmCvar_t	ui_teampointlimit;
vmCvar_t	ui_timelimit;
vmCvar_t	ui_roundtime;
vmCvar_t	ui_friendlyfire;

vmCvar_t	cg_lightmarks;

vmCvar_t 	cg_chatTime;
vmCvar_t 	cg_chatHeight;
vmCvar_t	cg_showConsole;

vmCvar_t	cg_smallGuns;
vmCvar_t	cg_chatBeep;

vmCvar_t	cg_goreLevel;
vmCvar_t	cg_antiLag;
vmCvar_t	cg_autoReload;
vmCvar_t	cg_useBandage;
vmCvar_t	cg_crosshairWidth;


vmCvar_t	cg_qcmd_posx;
vmCvar_t	cg_qcmd_posy;
vmCvar_t	cg_qcmd_cmd1;
vmCvar_t	cg_qcmd_cmd2;
vmCvar_t	cg_qcmd_cmd3;
vmCvar_t	cg_qcmd_cmd4;
vmCvar_t	cg_qcmd_cmd5;
vmCvar_t	cg_qcmd_cmd6;
vmCvar_t	cg_qcmd_cmd7;
vmCvar_t	cg_qcmd_cmd8;
vmCvar_t	cg_qcmd_cmd9;
vmCvar_t	cg_qcmd_cmd0;
vmCvar_t	cg_qcmd_dscr1;
vmCvar_t	cg_qcmd_dscr2;
vmCvar_t	cg_qcmd_dscr3;
vmCvar_t	cg_qcmd_dscr4;
vmCvar_t	cg_qcmd_dscr5;
vmCvar_t	cg_qcmd_dscr6;
vmCvar_t	cg_qcmd_dscr7;
vmCvar_t	cg_qcmd_dscr8;
vmCvar_t	cg_qcmd_dscr9;
vmCvar_t	cg_qcmd_dscr0;
vmCvar_t	cg_qcmd_size;
vmCvar_t	cg_qcmd_r;
vmCvar_t	cg_qcmd_g;
vmCvar_t	cg_qcmd_b;
vmCvar_t	cg_qcmd_a;
vmCvar_t	cg_qcmd_close;


vmCvar_t	cg_drawRadar;
vmCvar_t	cg_radarUpdate;
vmCvar_t	cg_radarX;
vmCvar_t	cg_radarY;

vmCvar_t	cg_weaponYaw;
vmCvar_t	cg_weaponPitch;
vmCvar_t	cg_weaponRoll;

vmCvar_t	cg_hudStyle;
vmCvar_t	cg_hudScale;

vmCvar_t	cg_timerPosX;
vmCvar_t	cg_timerPosY;

vmCvar_t	cg_hud1PosX;
vmCvar_t	cg_hud1PosY;

vmCvar_t	cg_hud2PosX;
vmCvar_t	cg_hud2PosY;

vmCvar_t	cg_hudAlpha1;
vmCvar_t	cg_hudAlpha2;

vmCvar_t	cg_timerCustom;
vmCvar_t	cg_drawScriptedUI;

vmCvar_t	cg_drawScriptedUI;

vmCvar_t	r_lightVertex;

vmCvar_t	gameversion;
vmCvar_t	cg_timetocache;

// BLUTENGEL
 
vmCvar_t  cg_crosshairFade;
vmCvar_t  cg_lowAmmoWarning;

vmCvar_t		r_hdr;
vmCvar_t		r_motionblur;
vmCvar_t		r_blur;
// Navy Seals --
typedef struct {
    vmCvar_t	*vmCvar;
    char		*cvarName;
    char		*defaultString;
    int			cvarFlags;
} cvarTable_t;

cvarTable_t		cvarTable[] = {
                               { &gameversion, "gameversion", GAME_VERSION , CVAR_ROM },

                               { &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
                               { &cg_autoswitch, "cg_autoswitch", "1", CVAR_ARCHIVE },
                               { &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
                               { &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
                               { &cg_fov, "cg_fov", "80", CVAR_ROM },
                               { &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
                               { &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
                               { &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
                               { &cg_gibs, "cg_gibs", "1", CVAR_ARCHIVE  },
                               { &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE | CVAR_CHEAT },
                               { &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
                               { &cg_drawTimer, "cg_drawTimer", "0", CVAR_ARCHIVE  },
                               { &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
                               { &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
                               { &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
                               { &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
                               { &cg_drawAmmoWarning, "cg_drawAmmoWarning", "1", CVAR_ARCHIVE  },
                               { &cg_drawAttacker, "cg_drawAttacker", "1", CVAR_ARCHIVE  },
                               { &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
                               { &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
                               { &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
                               { &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
                               { &cg_crosshairHealth, "cg_crosshairHealth", "1", CVAR_ARCHIVE },
                               { &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
                               { &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
                               { &cg_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
                               { &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
                               { &cg_addMarks, "cg_marks", "120", CVAR_ARCHIVE },
                               { &cg_lagometer, "cg_lagometer", "0", CVAR_ARCHIVE },
                               // BLUTENGEL 20040206 DEMOCRITUS
                               // CHANGED TO 0 AS DEMOCRITUS REQUESTED
                               //	{ &cg_gun_x, "cg_gunX", "-1.5", CVAR_CHEAT },
                               { &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
                               { &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
                               { &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
                               { &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
                               { &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
                               { &cg_runyaw, "cg_runyaw", "0.002", CVAR_ARCHIVE},
                               { &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
                               { &cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE },
                               { &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
                               { &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
                               { &cg_bobyaw, "cg_bobyaw", "0.002", CVAR_ARCHIVE },
                               { &cg_swingSpeed, "cg_swingSpeed", "0.9", CVAR_ARCHIVE | CVAR_CHEAT },
                               { &cg_animSpeed, "cg_animspeed", "1", CVAR_CHEAT },
                               { &cg_debugAnim, "cg_debuganim", "0", CVAR_CHEAT },
                               { &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
                               { &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
                               { &cg_errorDecay, "cg_errordecay", "100", 0 },
                               { &cg_nopredict, "cg_nopredict", "0", 0 },
                               { &cg_noPlayerAnims, "cg_noplayeranims", "0", CVAR_CHEAT },
                               { &cg_showmiss, "cg_showmiss", "0", 0 },
                               { &cg_footsteps, "cg_footsteps", "1", CVAR_CHEAT },
                               { &cg_tracerChance, "cg_tracerchance", "0.1", CVAR_CHEAT },
                               { &cg_tracerWidth, "cg_tracerwidth", "0.3", CVAR_CHEAT },
                               { &cg_tracerLength, "cg_tracerlength", "100", CVAR_CHEAT },
                               { &cg_thirdPersonRange, "cg_thirdPersonRange", "40", CVAR_CHEAT },
                               { &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", CVAR_CHEAT },
                               { &cg_thirdPerson, "cg_thirdPerson", "0", CVAR_CHEAT },
                               { &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
                               { &cg_teamChatHeight, "cg_teamChatHeight", "8", CVAR_ARCHIVE  },
                               //	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
                               //	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },
#if 0
                               { &cg_deferPlayers, "cg_deferPlayers", "0", CVAR_ARCHIVE },//0
#else
                               { &cg_deferPlayers, "cg_deferPlayers", "1", CVAR_ARCHIVE },
#endif
                               { &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
                               { &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM  },
                               { &cg_stats, "cg_stats", "0", 0 },
                               { &cg_drawFriend, "cg_drawFriend", "0", CVAR_ARCHIVE },
                               { &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
                               { &cg_noVoiceChats, "cg_noVoiceChats", "0", CVAR_ARCHIVE },
                               { &cg_noVoiceText, "cg_noVoiceText", "0", CVAR_ARCHIVE },
                               // the following variables are created in other parts of the system,
                               // but we also reference them here
                               { &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
                               { &cg_paused, "cl_paused", "0", CVAR_ROM },
                               { &cg_blood, "com_blood", "1", CVAR_ARCHIVE },
                               { &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo


                               { &cg_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},

                               { &cg_cameraOrbit, "cg_cameraOrbit", "0", CVAR_CHEAT},
                               { &cg_cameraOrbitDelay, "cg_cameraOrbitDelay", "50", CVAR_ARCHIVE},
                               { &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
                               { &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
                               { &cg_timescale, "timescale", "1", 0},
                               { &cg_scorePlum, "cg_scorePlums", "0", CVAR_USERINFO | CVAR_ARCHIVE | CVAR_CHEAT },
                               { &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
                               { &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

                               { &pmove_fixed, "pmove_fixed", "0", 0},
                               { &pmove_msec, "pmove_msec", "8", 0},

                               { &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
                               { &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
                               // Navy Seals ++
                               {	&test_x, "test_x", "0", 0 },
                               {	&test_y, "test_y", "0", 0 },
                               {	&test_h, "test_h", "0", 0 },
                               {	&test_w, "test_w", "0", 0 },
                               {	&pmodel_o, "pmodel_offset", "0", CVAR_CHEAT },
                               {	&cg_gunSmoke, "cg_gunSmoke", "-1", CVAR_ARCHIVE },
                               {	&cg_enableDust, "g_enableDust", "0", CVAR_SERVERINFO},
                               {	&cg_enableBreath, "g_enableBreath", "1", CVAR_SERVERINFO},

                               {	&cg_enableTimeSelect, "cg_enabletimeselect", "1", CVAR_ARCHIVE},

                               {	&cg_atmosphericEffects, "cg_enviromentFX", "1", CVAR_ARCHIVE },
                               {	&cg_lowEffects, "cg_enviromentFX_quality", "0", CVAR_ARCHIVE },
                               {	&cg_customTracerColor, "cg_customTracerColor", "1", CVAR_ARCHIVE },
                               {	&cg_gunSmokeTime, "cg_gunSmokeTime", "1500", CVAR_ARCHIVE },
                               {	&cg_particleTime, "cg_particleTime", "500", CVAR_ARCHIVE },

                               {	&cg_crosshair_r, "cg_crosshair_r", "1", CVAR_ARCHIVE },
                               {	&cg_crosshair_g, "cg_crosshair_g", "1", CVAR_ARCHIVE },
                               {	&cg_crosshair_b, "cg_crosshair_b", "1", CVAR_ARCHIVE },

                               {	&char_strength, "char_strength", "0", CVAR_ROM },
                               {	&char_stamina, "char_stamina", "0", CVAR_ROM },
                               {	&char_stealth, "char_stealth", "0", CVAR_ROM },
                               {	&char_speed, "char_speed", "0", CVAR_ROM },
                               {	&char_accuracy, "char_accuracy", "0", CVAR_ROM },
                               {	&char_technical, "char_technical", "0", CVAR_ROM },
                               {	&char_xp,	"char_total_xp","0",CVAR_ROM },

                               {	&ui_sealplayers, "ui_sealplayers", "0", CVAR_ROM },
                               {	&ui_tangoplayers, "ui_tangoplayers", "0", CVAR_ROM },
                               {	&ui_players,	"ui_players","0",CVAR_ROM },

                               {	&ui_sealpoints, "ui_sealpoints", "0", CVAR_ROM },
                               {	&ui_tangopoints,	"ui_tangopoints","0",CVAR_ROM },

                               // game cvars
                               {	&ui_team, "ui_team", "0", CVAR_ROM },

                               {	&cg_bulletTracerLength, "cg_bulletTracerLength", "75", 0 },
                               {	&cg_bulletTracerWidth, "cg_bulletTracerWidth", "0.75", 0 },

                               {	&cg_wakemarkTime, "cg_wakemarkTime", "20", CVAR_ARCHIVE },
                               {	&cg_wakemarkDistantTime, "cg_wakemarkDistantTime", "100", CVAR_ARCHIVE } ,

                               {	&ui_assaultstate, "ui_assaultstate", "0", CVAR_ROM },
                               {	&ui_bombstate, "ui_bombstate", "0", CVAR_ROM },

                               {	&mi_viprescue, "mi_viprescue", "0", CVAR_ROM },
                               {	&mi_viptime, "mi_viptime", "0", CVAR_ROM },
                               {	&mi_blowup, "mi_blowup", "0", CVAR_ROM },
                               {	&mi_assaultfield, "mi_assaultfield", "0", CVAR_ROM } ,

                               {	&ui_gotbomb, "ui_gotbomb", "0", CVAR_ROM },
                               {	&ui_isvip, "ui_isvip", "0", CVAR_ROM },
                               {	&ui_gotbriefcase, "ui_gotbriefcase", "0", CVAR_ROM },

                               {	&cg_grenadeSparks, "cg_grenadeSparks", "1", CVAR_ARCHIVE },
                               {	&cg_correctgunFov, "cg_correctgunFov", "65", CVAR_ARCHIVE },

                               {	&cg_isgun_z, "cg_isgunz", "0", CVAR_CHEAT },
                               {	&cg_isgun_y, "cg_isguny", "0", CVAR_CHEAT },
                               {	&cg_isgun_x, "cg_isgunx", "0", CVAR_CHEAT },

                               {	&cg_isgun_roll, "cg_isgun_roll", "0", CVAR_CHEAT },
                               {	&cg_isgun_yaw, "cg_isgun_yaw", "0", CVAR_CHEAT },
                               {	&cg_isgun_pitch, "cg_isgun_pitch", "0", CVAR_CHEAT },
                               {	&cg_isgun_step, "cg_isgun_step", "0.25", CVAR_CHEAT },

                               {	&cg_newbeeHeight, "cg_newbeeHeight", "0.2", CVAR_ARCHIVE },
                               {	&cg_newbeeTime, "cg_newbeeTime", "5.5", CVAR_ARCHIVE },

                               {	&cg_viewHeight, "cg_viewHeight", "0", CVAR_CHEAT },

                               {	&ui_gotbriefcase, "ui_gotbriefcase", "0", CVAR_CHEAT },
                               {	&ui_isvip, "ui_isvip", "0", CVAR_CHEAT },
                               {	&ui_gotbomb, "ui_gotbomb", "0", CVAR_CHEAT },
                               {	&mi_blowup, "mi_blowup", "0", CVAR_CHEAT },
                               {	&mi_viprescue, "mi_viprescue", "0", CVAR_CHEAT },
                               {	&mi_viptime, "mi_viptime", "0", CVAR_CHEAT },
                               {	&mi_assaultfield, "mi_assaultfield", "0", CVAR_CHEAT },
                               {	&ui_assaultstate, "ui_assaultstate", "0", CVAR_CHEAT },
                               {	&ui_bombstate, "ui_bombstate", "0", CVAR_CHEAT },


                               {	&ui_roundtimerworld, "ui_roundtimerworld", "0", CVAR_CHEAT },
                               {	&ui_viptimer, "ui_viptimer", "0", CVAR_CHEAT },
                               {	&ui_assaulttimer1, "ui_assaulttimer1", "0", CVAR_CHEAT },
                               {	&ui_assaulttimer2, "ui_assaulttimer2", "0", CVAR_CHEAT },
                               {	&ui_assaulttimer3, "ui_assaulttimer3", "0", CVAR_CHEAT },
                               {	&ui_assaulttimer4, "ui_assaulttimer4", "0", CVAR_CHEAT },
                               {	&ui_bombtimer, "ui_bombtimer", "0", CVAR_CHEAT },
                               {	&ui_assaultblocked1, "ui_assaultblocked1", "0", CVAR_CHEAT },
                               {	&ui_assaultblocked2, "ui_assaultblocked2", "0", CVAR_CHEAT },
                               {	&ui_assaultblocked3, "ui_assaultblocked3", "0", CVAR_CHEAT },
                               {	&ui_assaultblocked4, "ui_assaultblocked4", "0", CVAR_CHEAT },

                               {	&cg_disableHeadstuff, "cg_disableHeadstuff", "0", CVAR_LATCH | CVAR_ARCHIVE },
                               {	&cg_disableTangoHandSkin, "cg_disableTangoHandSkin", "0", CVAR_LATCH  | CVAR_ARCHIVE },

                               {	&raise_acc, "raise_acc", "0", CVAR_ROM },
                               {	&raise_spd, "raise_spd", "0", CVAR_ROM },
                               {	&raise_str, "raise_str", "0", CVAR_ROM },
                               {	&raise_stl, "raise_stl", "0", CVAR_ROM },
                               {	&raise_sta, "raise_sta", "0", CVAR_ROM },
                               {	&raise_tec, "raise_tec", "0", CVAR_ROM },

                               {	&ui_teampointlimit, "ui_teampointlimit", "0", CVAR_ROM },
                               {	&ui_timelimit, "ui_timelimit", "0", CVAR_ROM },
                               {	&ui_roundtime, "ui_roundtime", "0", CVAR_ROM },
                               {	&ui_friendlyfire, "ui_friendlyfire", "0", CVAR_ROM },

                               {	&cg_chatTime, "cg_chatTime", "8000", CVAR_ARCHIVE },
                               {	&cg_chatHeight, "cg_chatHeight", "12", CVAR_ARCHIVE },
                               {	&cg_showConsole, "cg_showConsole", "1", CVAR_ARCHIVE  },

                               {	&cg_lightmarks, "cg_lightmarks", "0", CVAR_ARCHIVE },

                               {	&cg_smallGuns, "cg_smallGuns", "1", CVAR_ARCHIVE },

                               {	&cg_chatBeep, "cg_chatBeep", "0", CVAR_ARCHIVE },

                               {	&cg_goreLevel, "cg_goreLevel", "1", CVAR_ARCHIVE },

                               {	&cg_crosshairWidth, "cg_crosshairWidth", "1", CVAR_ARCHIVE },

                               {	&cg_antiLag, "cg_antiLag", "1", CVAR_ARCHIVE | CVAR_USERINFO },
                               {	&cg_autoReload, "cg_autoReload", "1", CVAR_ARCHIVE | CVAR_USERINFO },
                               {	&cg_useBandage, "cg_useBandage", "1", CVAR_ARCHIVE | CVAR_USERINFO },

                               {	&cg_qcmd_posx, "cg_qcmd_posx", "50", CVAR_ARCHIVE },
                               {	&cg_qcmd_posy, "cg_qcmd_posy", "50", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd1, "cg_qcmd_cmd1", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd2, "cg_qcmd_cmd2", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd3, "cg_qcmd_cmd3", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd4, "cg_qcmd_cmd4", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd5, "cg_qcmd_cmd5", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd6, "cg_qcmd_cmd6", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd7, "cg_qcmd_cmd7", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd8, "cg_qcmd_cmd8", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd9, "cg_qcmd_cmd9", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_cmd0, "cg_qcmd_cmd0", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_close, "cg_qcmd_close", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr1, "cg_qcmd_dscr1", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr2, "cg_qcmd_dscr2", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr3, "cg_qcmd_dscr3", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr4, "cg_qcmd_dscr4", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr5, "cg_qcmd_dscr5", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr6, "cg_qcmd_dscr6", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr7, "cg_qcmd_dscr7", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr8, "cg_qcmd_dscr8", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr9, "cg_qcmd_dscr9", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_dscr0, "cg_qcmd_dscr0", "", CVAR_ARCHIVE },
                               {	&cg_qcmd_size, "cg_qcmd_size", "0.2", CVAR_ARCHIVE },
                               {	&cg_qcmd_r, "cg_qcmd_r", "1", CVAR_ARCHIVE },
                               {	&cg_qcmd_g, "cg_qcmd_g", "1", CVAR_ARCHIVE },
                               {	&cg_qcmd_b, "cg_qcmd_b", "1", CVAR_ARCHIVE },
                               {	&cg_qcmd_a, "cg_qcmd_a", "0.75", CVAR_ARCHIVE },

                               {	&cg_drawRadar, "cg_drawRadar", "1", CVAR_ARCHIVE },
                               {	&cg_radarX, "cg_radarX", "597", CVAR_ARCHIVE },
                               {	&cg_radarY, "cg_radarY", "300", CVAR_ARCHIVE },
                               {	&cg_radarUpdate, "cg_radarUpdate", "350", CVAR_ARCHIVE | CVAR_USERINFO },

                               {	&cg_weaponYaw, "cg_weaponYaw", "1", CVAR_ARCHIVE },
                               {	&cg_weaponPitch, "cg_weaponPitch", "1", CVAR_ARCHIVE },
                               {	&cg_weaponRoll, "cg_weaponRoll", "1", CVAR_ARCHIVE },

                               {	&cg_hudStyle, "cg_hudStyle", "1", CVAR_ARCHIVE },

                               {	&cg_hud1PosX, "cg_hud1PosX", "891", CVAR_ARCHIVE },
                               {	&cg_hud1PosY, "cg_hud1PosY", "545", CVAR_ARCHIVE },
                               {	&cg_hud2PosX, "cg_hud2PosX", "830", CVAR_ARCHIVE },
                               {	&cg_hud2PosY, "cg_hud2PosY", "676", CVAR_ARCHIVE },

                               {	&cg_hudScale, "cg_hudScale", "1", CVAR_ARCHIVE },

                               {	&cg_hudAlpha1, "cg_hudAlpha1", "0.5", CVAR_ARCHIVE },
                               {	&cg_hudAlpha2, "cg_hudAlpha2", "0.4", CVAR_ARCHIVE },

                               {	&cg_timerPosX, "cg_timerPosX", "0", CVAR_ARCHIVE },
                               {	&cg_timerPosY, "cg_timerPosY", "0", CVAR_ARCHIVE },

                               {	&cg_timerCustom, "cg_timerCustom", "3", CVAR_ARCHIVE },

                               {	&cg_drawScriptedUI, "cg_drawScriptedUI", "1", CVAR_ARCHIVE },

                               {	&r_lightVertex, "r_lightVertex", "0", CVAR_ARCHIVE | CVAR_LATCH },

                               {	&cg_timetocache, "cg_timetocache", "750", CVAR_ARCHIVE | CVAR_LATCH },


                               {	&cg_debugAlloc, "cg_debugAlloc", "0", 0 }, 
                               { &cg_crosshairFade, "cg_crosshairFade", "0", CVAR_ARCHIVE},
                               { &cg_lowAmmoWarning, "cg_lowAmmoWarning", "0", CVAR_ARCHIVE},
                               { &r_hdr, "r_hdr", "0", CVAR_ARCHIVE},
							   { &r_motionblur, "r_motionblur", "0", CVAR_ARCHIVE},
							   { &r_blur, "r_blur", "0", CVAR_ARCHIVE}
							   
                               // Navy Seals --
                           };

int		cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );


/*
=================
CG_CheckForCheats
=================
*/
qboolean CG_CheckForCheats( void )
{
    int	len;
    fileHandle_t	f;

    len = trap_FS_FOpenFile( "../openGL32.dll",  &f, FS_READ );

    if ( !f ) {
        return qfalse;
    }
    trap_FS_FCloseFile( f );
    return qtrue;
}

/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
    int			i;
    cvarTable_t	*cv;
    char		var[MAX_TOKEN_CHARS];
#if 0
    int			zonemegs,hunkmegs,soundmegs;
#endif

    for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
        trap_Cvar_Register( cv->vmCvar, cv->cvarName,
                            cv->defaultString, cv->cvarFlags );
    }

    // see if we are also running the server on this machine
    trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
    cgs.localServer = atoi( var );

    //	forceModelModificationCount = cg_forceModel.modificationCount;
#if 0
    trap_Cvar_VariableStringBuffer( "com_hunkmegs", var, sizeof( var ) );
    hunkmegs = atoi( var );

    trap_Cvar_VariableStringBuffer( "com_zonemegs", var, sizeof( var ) );
    zonemegs = atoi( var );

    trap_Cvar_VariableStringBuffer( "com_soundmegs", var, sizeof( var ) );
    soundmegs = atoi( var );

    // BLUTENGEL 07.01.2004
    // removed as noone really wants to know that!
    // CG_Printf("hunkmegs: %i zonemegs: %i soundmegs: %i\n", hunkmegs,zonemegs,soundmegs );
    if ( hunkmegs < 150 )
    {
        CG_Error(
            "You can't start NS-CO because of your memorysettings."
            "You might have started NS-CO via the Quake3 mods menu."
            "Please use a shortcut, start with the required parameters or use the launcher."
            "Your 'com_hunkmegs' setting is %i, it must be atleast 150."
            "Please refer to the troubleshooting faq for more information.", hunkmegs );

        return;
    }

    if ( zonemegs < 16 )
    {
        CG_Error(
            "You can't start NS-CO because of your memorysettings."
            "You might have started NS-CO via the Quake3 mods menu."
            "Please use a shortcut, start with the required parameters or use the launcher."
            "Your 'com_zonemegs' setting is %i, it must be atleast 16."
            "Please refer to the troubleshooting faq for more information.", zonemegs );
        return;
    }

    if ( soundmegs < 16 )
    {
        CG_Error(
            "You can't start NS-CO because of your memorysettings."
            "You might have started NS-CO via the Quake3 mods menu."
            "Please use a shortcut, start with the required parameters or use the launcher."
            "Your 'com_soundmegs' setting is %i, it must be atleast 16."
            "Please refer to the troubleshooting faq for more information.", soundmegs );
        return;
    }
#endif

    trap_Cvar_Register(NULL, "model", DEFAULT_MODEL, CVAR_USERINFO | CVAR_ARCHIVE );

    trap_Cvar_Register(NULL, "e_head", "", CVAR_USERINFO | CVAR_ARCHIVE );
    trap_Cvar_Register(NULL, "e_eyes", "", CVAR_USERINFO | CVAR_ARCHIVE );
    trap_Cvar_Register(NULL, "e_mouth", "", CVAR_USERINFO | CVAR_ARCHIVE );

    trap_Cvar_Register(NULL, "quitmsg", "www.ns-co.net / join our forum", CVAR_USERINFO | CVAR_ARCHIVE );
    trap_Cvar_Register(NULL, "character", "C111111", CVAR_USERINFO );

}


static void CG_CheckForceCvar( void )
{
    int i = 0;

    char str_value[MAX_CVAR_VALUE_STRING];
    float minvalue;
    float maxvalue;
    float value;

    qboolean	vid_restart = qfalse;


    for ( i = 0; i < cg.num_blockedCvars ; i++)
    {
        trap_Cvar_VariableStringBuffer( cg.blockedCvars[i].string, str_value, sizeof( str_value ) );

        minvalue = atof(cg.blockedCvars[i].minvalue);
        maxvalue = atof(cg.blockedCvars[i].maxvalue);

        value = atof( str_value );

        if ( cg.cheatsEnabled )
        {
            if ( !Q_stricmp( cg.blockedCvars[i].string, "cg_thirdperson" ) )
                continue;
        }
        if ( value < minvalue )
        {

            if (!Q_stricmp(cg.blockedCvars[i].string, "com_maxfps") ) {
                trap_Cvar_Set( cg.blockedCvars[i].string , cg.blockedCvars[i].maxvalue );
                CG_Printf("forced cvar %s to %f\n", cg.blockedCvars[i].string, maxvalue );
            } else {
                trap_Cvar_Set( cg.blockedCvars[i].string , cg.blockedCvars[i].minvalue );
                CG_Printf("forced cvar %s to %f\n", cg.blockedCvars[i].string, minvalue );
            }

            if ( cg.blockedCvars[i].restart_video )
                vid_restart = qtrue;
        }
        if ( value > maxvalue )
        {
            trap_Cvar_Set( cg.blockedCvars[i].string , cg.blockedCvars[i].maxvalue );
            CG_Printf("forced cvar %s to %f\n", cg.blockedCvars[i].string, maxvalue );

            if ( cg.blockedCvars[i].restart_video )
                vid_restart = qtrue;
        }
    }

    if ( vid_restart )
    {
        trap_SendConsoleCommand( "vid_restart\n" );
    }//
}

/*
=================
CG_UpdateCvars
=================
*/
void ClientScript_ProcessWhileLoops( void );
void CG_UpdateCvars( void ) {
    int			i;
    cvarTable_t	*cv;

    for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
        trap_Cvar_Update( cv->vmCvar );

    }

    // check for modications here
    CG_CheckForceCvar(  );


    // If team overlay is on, ask for updates from the server.  If its off,
    // let the server know so we don't receive it
    if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
        drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

        if ( cg_drawTeamOverlay.integer > 0 ) {
            trap_Cvar_Set( "teamoverlay", "1" );
        } else {
            trap_Cvar_Set( "teamoverlay", "0" );
        }
        // FIXME E3 HACK
        trap_Cvar_Set( "teamoverlay", "1" );
    }


    //	if ( forceModelModificationCount != cg_forceModel.modificationCount )
}

int CG_CrosshairPlayer( void ) {
    if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
        return -1;
    }
    return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
    return -1; ;
}

void CG_AddToChat( const char *str );

/*
=================
CG_RemoveEscapeChar
=================
*/
static void CG_RemoveEscapeChar( char *text ) {
    int i, l;

    l = 0;
    for ( i = 0; text[i]; i++ ) {
        if (text[i] == '\x19')
            continue;
        if (text[i] == '\n' )
            continue;

        text[l++] = text[i];
    }
    text[l] = '\0';
}

void QDECL CG_Printf( const char *msg, ... ) {
    va_list		argptr;
    char		text[1024];

    va_start (argptr, msg);
    vsprintf (text, msg, argptr);
    va_end (argptr);

    if ( cg_showConsole.integer )
        trap_Print( text );

    CG_RemoveEscapeChar( text );
    //	CG_AddToChat( text );
}

void QDECL CG_Error( const char *msg, ... ) {
    va_list		argptr;
    char		text[1024];

    va_start (argptr, msg);
    vsprintf (text, msg, argptr);
    va_end (argptr);

    trap_Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... ) {
    va_list		argptr;
    char		text[1024];

    va_start (argptr, error);
    vsprintf (text, error, argptr);
    va_end (argptr);

    CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
    va_list		argptr;
    char		text[1024];

    va_start (argptr, msg);
    vsprintf (text, msg, argptr);
    va_end (argptr);

    CG_Printf ("%s", text);
}

#endif

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
    static char	buffer[MAX_STRING_CHARS];

    trap_Argv( arg, buffer, sizeof( buffer ) );

    return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
    gitem_t			*item;
    char			data[MAX_QPATH];
    char			*s, *start;
    int				len;

    item = &bg_itemlist[ itemNum ];

    if( item->pickup_sound ) {
        trap_S_RegisterSound( item->pickup_sound, qfalse );
    }

    // parse the space seperated precache string for other media
    s = item->sounds;
    if (!s || !s[0])
        return;

    while (*s) {
        start = s;
        while (*s && *s != ' ') {
            s++;
        }

        len = s-start;
        if (len >= MAX_QPATH || len < 5) {
            CG_Error( "PrecacheItem: %s has bad precache string",
                      item->classname);
            return;
        }
        memcpy (data, start, len);
        data[len] = 0;
        if ( *s ) {
            s++;
        }

        if ( !strcmp(data+len-3, "wav" )) {
            trap_S_RegisterSound( data, qfalse );
        }
    }
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
    int		i;
    char	items[MAX_ITEMS+1];
    char	name[MAX_QPATH];
    const char	*soundName;

    //	cgs.media.countFightSound = trap_S_RegisterSound( "sound/feedback/fight.wav", qfalse );
    //	cgs.media.countPrepareSound = trap_S_RegisterSound( "sound/feedback/prepare.wav", qfalse );

    if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {

        cgs.media.redLeadsSound = trap_S_RegisterSound( "sound/commentary/sls_lead.wav", qfalse );
        cgs.media.blueLeadsSound = trap_S_RegisterSound( "sound/commentary/tgs_lead.wav", qfalse );


        cgs.media.redScoredSound = trap_S_RegisterSound( "sound/commentary/sls_won.wav", qfalse );
        cgs.media.blueScoredSound = trap_S_RegisterSound( "sound/commentary/tgs_won.wav", qfalse );

        cgs.media.roundDrawSound = trap_S_RegisterSound( "sound/commentary/draw.wav", qfalse );
#ifdef IS_REDUNDANT
        cgs.media.captureYourTeamSound = trap_S_RegisterSound( "sound/commentary/bfc_stolen.wav", qfalse );
#endif
    }
    // Navy Seals ++

    for (i=0 ; i<4 ; i++) {
        Com_sprintf (name, sizeof(name), "sound/debris/wood%i.wav", i+1);
        //	CG_Printf("CACHED: %s\n",name);
        cgs.media.sfxWood[i] = trap_S_RegisterSound (name,qfalse );

        Com_sprintf (name, sizeof(name), "sound/debris/glass%i.wav", i+1);
        //	CG_Printf("CACHED: %s\n",name);
        cgs.media.sfxGlass[i] = trap_S_RegisterSound (name,qfalse );
    }
    for (i=0 ; i<4 ; i++) {
        Com_sprintf (name, sizeof(name), "sound/debris/metal%i.wav", i+1);
        //	CG_Printf("CACHED: %s\n",name);
        cgs.media.sfxMetal[i] = trap_S_RegisterSound (name,qfalse );
    }
    for (i=0;i<3;i++){
        Com_sprintf (name, sizeof(name), "sound/weapons/shellhit_%i.wav", i+1);
        //	CG_Printf("CACHED: %s\n",name);
        cgs.media.sfxShellHitWall[i] = trap_S_RegisterSound (name,qfalse );
    }
    for (i=0;i<3;i++){
        Com_sprintf (name, sizeof(name), "sound/actors/player/death%i.wav", i+1);
        //	CG_Printf("CACHED: %s\n",name);
        cgs.media.deathSounds[i] = trap_S_RegisterSound (name,qfalse );
    }

    cgs.media.mk26_explode = trap_S_RegisterSound ("sound/weapons/mk26/explode.wav",qfalse );
    cgs.media.c4_explode = trap_S_RegisterSound ("sound/weapons/c4/explode.wav",qfalse );
    cgs.media.flashbang_explode = trap_S_RegisterSound ("sound/weapons/flashbang/explode.wav",qfalse );
    cgs.media.flashbanged = trap_S_RegisterSound ("sound/weapons/flashbang/flashbanged.wav",qfalse );
    cgs.media.flash_2sec = trap_S_RegisterSound ("sound/weapons/flashbang/flash_2sec.wav",qfalse );
    cgs.media.flash_4sec = trap_S_RegisterSound ("sound/weapons/flashbang/flash_4sec.wav",qfalse );
    cgs.media.flash_6sec = trap_S_RegisterSound ("sound/weapons/flashbang/flash_6sec.wav",qfalse );
    cgs.media.flash_8sec = trap_S_RegisterSound ("sound/weapons/flashbang/flash_8sec.wav",qfalse );
    cgs.media.newbeeMsgSound = trap_S_RegisterSound ("sound/misc/newbee_msg.wav", qfalse );

    // Navy Seals --
    cgs.media.tracerSound = trap_S_RegisterSound( "sound/weapons/machinegun/buletby1.wav", qfalse );
    cgs.media.selectSound = trap_S_RegisterSound( "sound/weapons/change.wav", qfalse );
    //	cgs.media.gibSound = trap_S_RegisterSound( "sound/player/gibsplt1.wav", qfalse );
    //	cgs.media.gibBounce1Sound = trap_S_RegisterSound( "sound/player/gibimp1.wav", qfalse );
    //	cgs.media.gibBounce2Sound = trap_S_RegisterSound( "sound/player/gibimp2.wav", qfalse );
    //	cgs.media.gibBounce3Sound = trap_S_RegisterSound( "sound/player/gibimp3.wav", qfalse );



    cgs.media.teleInSound = trap_S_RegisterSound( "sound/world/telein.wav", qfalse );
    cgs.media.teleOutSound = trap_S_RegisterSound( "sound/world/teleout.wav", qfalse );

    cgs.media.noAmmoSound = trap_S_RegisterSound( "sound/weapons/noammo.wav", qfalse );

    cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav", qfalse );
    cgs.media.landSound = trap_S_RegisterSound( "sound/player/land1.wav", qfalse);


    CG_LoadingBarUpdate(25);


    cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav", qfalse);
    cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav", qfalse);
    cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav", qfalse);

    for (i=0 ; i<4 ; i++) {
        Com_sprintf (name, sizeof(name), "sound/actors/solid%i.wav", i+1);
        cgs.media.footsteps[FOOTSTEP_NORMAL][i] = trap_S_RegisterSound (name, qfalse);

        Com_sprintf (name, sizeof(name), "sound/actors/water%i.wav", i+1);
        cgs.media.footsteps[FOOTSTEP_SPLASH][i] = trap_S_RegisterSound (name, qfalse);

        Com_sprintf (name, sizeof(name), "sound/actors/metal%i.wav", i+1);
        cgs.media.footsteps[FOOTSTEP_METAL][i] = trap_S_RegisterSound (name, qfalse);

        Com_sprintf (name, sizeof(name), "sound/actors/dirt%i.wav", i+1);
        cgs.media.footsteps[FOOTSTEP_DIRT][i] = trap_S_RegisterSound (name, qfalse);

        Com_sprintf (name, sizeof(name), "sound/actors/wood%i.wav", i+1);
        cgs.media.footsteps[FOOTSTEP_WOOD][i] = trap_S_RegisterSound (name, qfalse);

        Com_sprintf (name, sizeof(name), "sound/actors/snow%i.wav", i+1);
        cgs.media.footsteps[FOOTSTEP_SNOW][i] = trap_S_RegisterSound (name, qfalse);
    }

    // only register the items that the server says we need
    strcpy( items, CG_ConfigString( CS_ITEMS ) );

    for ( i = 1 ; i < bg_numItems ; i++ ) {
        if ( items[ i ] == '1' /* || cg_buildScript.integer */ ) {
            CG_RegisterItemSounds( i );
        }
    }

    for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
        soundName = CG_ConfigString( CS_SOUNDS+i );
        if ( !soundName[0] ) {
            break;
        }
        if ( soundName[0] == '*' ) {
            continue;	// custom sound
        }
        cgs.gameSounds[i] = trap_S_RegisterSound( soundName, qfalse );
    }
 
	/*
	IMPACTSOUND_DEFAULT,
    IMPACTSOUND_METAL,
    IMPACTSOUND_DIRT,
	IMPACTSOUND_WOOD,
	IMPACTSOUND_SNOW,
	*/
    cgs.media.sfx_ric[IMPACTSOUND_DEFAULT] = trap_S_RegisterSound ("sound/weapons/impact/default.wav", qfalse);
	cgs.media.sfx_ric[IMPACTSOUND_METAL] = trap_S_RegisterSound ("sound/weapons/impact/metal.wav", qfalse);
	cgs.media.sfx_ric[IMPACTSOUND_DIRT] = trap_S_RegisterSound ("sound/weapons/impact/dirt.wav", qfalse);
	cgs.media.sfx_ric[IMPACTSOUND_WOOD] = trap_S_RegisterSound ("sound/weapons/impact/wood.wav", qfalse);
	cgs.media.sfx_ric[IMPACTSOUND_SNOW] = trap_S_RegisterSound ("sound/weapons/impact/snow.wav", qfalse);  
	cgs.media.sfx_ric[IMPACTSOUND_GLASS] = trap_S_RegisterSound ("sound/weapons/impact/glass.wav", qfalse); 
	

    for (i=0 ; i<4 ; i++) {
        Com_sprintf (name, sizeof(name), "sound/actors/fleshhit_%i.wav", i+1);
        cgs.media.bulletHitFlesh[i] = trap_S_RegisterSound (name, qfalse);

        Com_sprintf (name, sizeof(name), "sound/actors/helmethit_%i.wav", i+1);
        cgs.media.bulletHitHelmet[i] = trap_S_RegisterSound (name, qfalse);

        if ( i < 3 )
        {
            Com_sprintf (name, sizeof(name), "sound/actors/kevlarhit_%i.wav", i+1);
            cgs.media.bulletHitKevlar[i] = trap_S_RegisterSound (name, qfalse);
        }
    }



    cgs.media.hgrenb1aSound = trap_S_RegisterSound("sound/weapons/grenade_bounce1.wav", qfalse);
    cgs.media.hgrenb2aSound = trap_S_RegisterSound("sound/weapons/grenade_bounce2.wav", qfalse);

    /*	cgs.media.regenSound = trap_S_RegisterSound("sound/items/regen.wav", qfalse);
    cgs.media.protectSound = trap_S_RegisterSound("sound/items/protect3.wav", qfalse);
    cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.wav", qfalse );
    cgs.media.wstbimplSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpl.wav", qfalse);
    cgs.media.wstbimpmSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpm.wav", qfalse);
    cgs.media.wstbimpdSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbimpd.wav", qfalse);
    cgs.media.wstbactvSound = trap_S_RegisterSound("sound/weapons/proxmine/wstbactv.wav", qfalse);
    */
}


//===================================================================================

/*
=================
CG_RegisterBombGraphics

This function caches all required bomb graphics and shaders
=================
*/
static void CG_RegisterBombGraphics( void ) {
    // Navy Seals ++
    int			i;

    static char		*sb_digital_nums[10] = {
                                            "models/misc/bombcase/digit_0",
                                            "models/misc/bombcase/digit_1",
                                            "models/misc/bombcase/digit_2",
                                            "models/misc/bombcase/digit_3",
                                            "models/misc/bombcase/digit_4",
                                            "models/misc/bombcase/digit_5",
                                            "models/misc/bombcase/digit_6",
                                            "models/misc/bombcase/digit_7",
                                            "models/misc/bombcase/digit_8",
                                            "models/misc/bombcase/digit_9",
                                        };

    // precache status bar pics
    CG_LoadingString( "Bombcase Graphics" );


    for ( i = 0 ; i < 11 ; i++ ) {
        cgs.media.bombCaseDigitShaders[i] = trap_R_RegisterShader( va("models/misc/bombcase/digit_%i", i+1) );
        if ( i < 8 )
            cgs.media.bombCaseWireModels[i] = trap_R_RegisterModel( va("models/misc/bombcase/wire_%i.md3", i+1) );
        if ( i < 3 )
            cgs.media.bombCaseDigitModels[i] = trap_R_RegisterModel( va("models/misc/bombcase/digit_%i.md3", i+1) );
    }

    cgs.media.bombCaseWireShaders[0] = trap_R_RegisterShader( "models/misc/bombcase/wire_grey" );
    cgs.media.bombCaseWireShaders[1] = trap_R_RegisterShader( "models/misc/bombcase/wire_red" );
    cgs.media.bombCaseWireShaders[2] = trap_R_RegisterShader( "models/misc/bombcase/wire_green" );

    cgs.media.bombCaseModel = trap_R_RegisterModel( "models/misc/bombcase/1stperson.md3" );
    cgs.media.bombCaseTangoSkin = trap_R_RegisterSkin( "models/misc/bombcase/bombcase_t.skin");
}
/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
    int			i;
    char		items[MAX_ITEMS+1];
    // Navy Seals ++
    static char		*sb_digital_nums[11] = {
                                            "gfx/2d/hud/0",
                                            "gfx/2d/hud/1",
                                            "gfx/2d/hud/2",
                                            "gfx/2d/hud/3",
                                            "gfx/2d/hud/4",
                                            "gfx/2d/hud/5",
                                            "gfx/2d/hud/6",
                                            "gfx/2d/hud/7",
                                            "gfx/2d/hud/8",
                                            "gfx/2d/hud/9",
                                            "gfx/2d/hud/line",
                                        };
    static char		*sb_digital_nums_small[11] = {
                "gfx/2d/hud/timer/number_0",
                "gfx/2d/hud/timer/number_1",
                "gfx/2d/hud/timer/number_2",
                "gfx/2d/hud/timer/number_3",
                "gfx/2d/hud/timer/number_4",
                "gfx/2d/hud/timer/number_5",
                "gfx/2d/hud/timer/number_6",
                "gfx/2d/hud/timer/number_7",
                "gfx/2d/hud/timer/number_8",
                "gfx/2d/hud/timer/number_9",
                "gfx/2d/hud/timer/number_slash",
            };

    // Navy Seals --

    // clear any references to old media
    memset( &cg.refdef, 0, sizeof( cg.refdef ) );
    trap_R_ClearScene();

    CG_LoadingString( cgs.mapname );

    trap_R_LoadWorldMap( cgs.mapname );

    CG_LoadingBarUpdate(15);
    // Navy Seals ++
    // precache status bar pics
    CG_LoadingString( "HUD Number Graphics" );

    for ( i=0 ; i<11 ; i++) {
        // defcon-x: cache digital numbers
        cgs.media.digitalNumberShaders[i] = trap_R_RegisterShaderNoMip( sb_digital_nums[i] );
        cgs.media.smalldigitalNumberShaders[i] = trap_R_RegisterShaderNoMip( sb_digital_nums_small[i] );
        // defcon-x: end
    }
    CG_LoadingString( "HUD Graphics" );

    // defcon-x: cache hud graphics
    // cgs.media.bulletIcon = trap_R_RegisterShaderNoMip( "gfx/2d/hud/bullets.tga" );

    cgs.media.clipIcon[0] = trap_R_RegisterShaderNoMip( "gfx/2d/hud/ammo_mag-clip.tga" );
    cgs.media.clipIcon[1] = trap_R_RegisterShaderNoMip( "gfx/2d/hud/ammo_mag-shell.tga" );
    cgs.media.clipIcon[2] = trap_R_RegisterShaderNoMip( "gfx/2d/hud/ammo_mag-gren-frag.tga" );
    cgs.media.clipIcon[3] = trap_R_RegisterShaderNoMip( "gfx/2d/hud/ammo_mag-gren-flash.tga" );
    cgs.media.clipIcon[4] = trap_R_RegisterShaderNoMip( "gfx/2d/hud/ammo_mag-gren-40mm.tga" );


    //	cgs.media.vipIcon = trap_R_RegisterShaderNoMip( "ui/assets/vip.tga" );
    //	cgs.media.bombIcon = trap_R_RegisterShaderNoMip( "ui/assets/bomb_tango.tga" );
    //	cgs.media.assaultIcon = trap_R_RegisterShaderNoMip( "ui/assets/assault_active.tga" );


    cgs.media.clockIcon = trap_R_RegisterShaderNoMip( "gfx/2d/hud/clock.tga" );
    cgs.media.slashIcon = trap_R_RegisterShaderNoMip( "gfx/2d/hud/line.tga" );
    cgs.media.colonIcon = trap_R_RegisterShaderNoMip( "gfx/2d/hud/colon.tga" );
    //	cgs.media.healthIcon = trap_R_RegisterShaderNoMip( "gfx/2d/hud/health.tga" );

    CG_LoadingString( "HUD Damage Locator" );

    CG_LoadingBarUpdate(5);

    // damage locator
    cgs.media.loc_headIcon				=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/head.tga" );
    cgs.media.loc_leftArmIcon			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/larm.tga" );
    cgs.media.loc_rightArmIcon			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/rarm.tga" );
    cgs.media.loc_leftLegIcon			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/lleg.tga" );
    cgs.media.loc_rightLegIcon			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/rleg.tga" );
    cgs.media.loc_stomachIcon			=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/stomach.tga" );
    cgs.media.loc_chestIcon				=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/chest.tga" );
    cgs.media.loc_bodyLines				=	trap_R_RegisterShaderNoMip( "gfx/2d/hud/loc/loc_lines.tga" );

    CG_LoadingString( "Ingame Graphics" );

    //	cgs.media.ingameMenuTopShader		=	trap_R_RegisterShaderNoMip ( "gfx/2d/ingame_top.tga" );
    //	cgs.media.ingameMenuBottomShader	=	trap_R_RegisterShaderNoMip ( "gfx/2d/ingame_bottom.tga" );
    //	cgs.media.rocketExplosionShader		=	trap_R_RegisterShader( "rocketExplosion3D" );
    cgs.media.laserShader				=	trap_R_RegisterShader ( "gfx/misc/laserpoint" );
    cgs.media.radioIcon					=	trap_R_RegisterShaderNoMip ( "gfx/misc/radio_talking" );


    {
        /*char	colorbits[64];

        trap_Cvar_VariableStringBuffer( "r_colorbits", colorbits, sizeof(colorbits) );

        if ( atoi(colorbits) >= 32 )
        {
        cgs.media.sniperScopeShader[0]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_m4" );
        cgs.media.sniperScopeShader[1]    = trap_R_RegisterShaderNoMip ( "gfx/misc/scope_ak47" );
        cgs.media.sniperScopeShader[2]    = trap_R_RegisterShaderNoMip ( "gfx/misc/scope_m14" );
        cgs.media.sniperScopeShader[3]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_psg-1" );
        cgs.media.sniperScopeShader[4]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_macmillan" );
        cgs.media.sniperScopeShader[5]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_sl8sd" );
        }
        else
        {*/
        cgs.media.sniperScopeShader[0]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_m4_16" );
        cgs.media.sniperScopeShader[1]    = trap_R_RegisterShaderNoMip ( "gfx/misc/scope_ak47_16" );
        cgs.media.sniperScopeShader[2]    = trap_R_RegisterShaderNoMip ( "gfx/misc/scope_m14_16" );
        cgs.media.sniperScopeShader[3]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_psg-1_16" );
        cgs.media.sniperScopeShader[4]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_macmillan_16" );
        cgs.media.sniperScopeShader[5]		=	trap_R_RegisterShaderNoMip ( "gfx/misc/scope_sl8sd_16" );
        //}
    }

    cgs.media.grenadeExplosionShader	=	trap_R_RegisterShader( "grenadeExplosion3D" );
    cgs.media.waterExplosionShader		=	trap_R_RegisterShader( "grenadeExplosion3Dwater" );

    cgs.media.thermalGogglesShader		=	trap_R_RegisterShaderNoMip ( "gfx/misc/goggle_screen" );
    cgs.media.thermalGogglesNoise		=	trap_R_RegisterShader ( "gfx/misc/goggle_noise" );
    cgs.media.flashedSpot				=	trap_R_RegisterShader( "gfx/misc/ns_blindspot1");

    cgs.media.flare						=	trap_R_RegisterShader("gfx/misc/corona");
    cgs.media.smallFlare				=	trap_R_RegisterShader("gfx/misc/flare1");


    CG_LoadingBarUpdate(5);

    CG_LoadingString( "Blood Graphics" );
    //	cgs.media.bloodHitShader = trap_R_RegisterShader( "bloodHit" );

    cgs.media.bloodparticleShaders[0] = trap_R_RegisterShader( "hit_blood1" );
    cgs.media.bloodparticleShaders[1] = trap_R_RegisterShader( "hit_blood2" );
    cgs.media.bloodparticleShaders[2] = trap_R_RegisterShader( "hit_blood3" );
    cgs.media.bloodparticleShaders[3] = trap_R_RegisterShader( "hit_blood4" );
    cgs.media.bloodparticleShaders[4] = trap_R_RegisterShader( "hit_blood5" );

    //
    // lensflare
    //
    CG_LoadingString( "Lensflare Graphics" );

    cgs.media.flare_circle_blue = trap_R_RegisterShader("circle_blue");
    cgs.media.flare_circle_fadein = trap_R_RegisterShader("circle_fadein");
    cgs.media.flare_flare_green = trap_R_RegisterShader("flare_green");
    cgs.media.flare_flare_turkis = trap_R_RegisterShader("flare_turkis");
    cgs.media.flare_circle_green = trap_R_RegisterShader("circle_green");
    cgs.media.flare_circle_orange = trap_R_RegisterShader("circle_orange");
    cgs.media.flare_circle_rainbow = trap_R_RegisterShader("circle_rainbow");

    CG_LoadingString( "Graphics" );

    cgs.media.dustPuffShader = trap_R_RegisterShader("hasteSmokePuff" );

    //	cgs.media.ammoMag_back = trap_R_RegisterShader( "gfx/2d/hud/ammo_mag.tga" );

    cgs.media.ammoMag_bullet[0] = trap_R_RegisterShader( "gfx/2d/hud/ammo_bullet-pistol.tga" );
    cgs.media.ammoMag_bullet[1] = trap_R_RegisterShader( "gfx/2d/hud/ammo_bullet-rifle.tga" );
    cgs.media.ammoMag_bullet[2] = trap_R_RegisterShader( "gfx/2d/hud/ammo_bullet-shotgun.tga" );


    // bombspot in this map, so register bombcase
    if ( cgs.mi_bombSpot )
        CG_RegisterBombGraphics( );

    CG_LoadingBarUpdate(10);

    ///
    /// Bloodpool
    ///
    for ( i = 0 ; i < 15 ; i++ ) {
        cgs.media.ns_bloodStain[i] = trap_R_RegisterShader( va("gfx/damage/blood/ns_blood_stain_%i.tga", i+1) );
    }
    for ( i = 0 ; i < 5 ; i++ ) {
        cgs.media.ns_brainStain[i] = trap_R_RegisterShader( va("gfx/damage/blood/ns_brain_stain_%i.tga", i+1) );
    }
    for ( i = 0 ; i < 5 ; i++ ) {
        cgs.media.ns_bloodStainSmall[i] = trap_R_RegisterShader( va("gfx/damage/blood/ns_blood_stain_small_%i.tga", i+1) );
    }

    cgs.media.ns_bloodPool = trap_R_RegisterShader( "gfx/damage/blood/ns_blood_pool.tga" );

    //
    // Inventory
    //
    for ( i = 0 ; i < 5 ; i++ ) {
        cgs.media.weaponMenuActive[i] = trap_R_RegisterShader( va("icons/menu_%i_active.tga", i+1) );
    }
    for ( i = 0 ; i < 5 ; i++ ) {
        cgs.media.weaponMenuInactive[i] = trap_R_RegisterShader( va("icons/menu_%i_inactive.tga", i+1) );
    }
    cgs.media.weaponSelInactiveBig = trap_R_RegisterShader( "icons/menu_item_inactive.tga" );
    cgs.media.weaponSelInactiveSmall = trap_R_RegisterShader( "icons/menu_item_inactive_small.tga" );

    CG_LoadingBarUpdate(5);

    // precache status bar pics
    cgs.media.deferShader = trap_R_RegisterShaderNoMip( "gfx/2d/defer.tga" );
    cgs.media.smokePuffShader = trap_R_RegisterShader( "ns_smokePuff" );
    cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "ns_smokePuffRagePro" );
    cgs.media.shotgunSmokePuffShader = trap_R_RegisterShader( "ns_shotgunSmokePuff" );
    cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
    cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );
    cgs.media.waterBubbleShader = trap_R_RegisterShader( "waterBubble" );

    cgs.media.metalsparkShader = trap_R_RegisterShader( "gfx/misc/ns_metal_sparks" );
    cgs.media.sparkShader = trap_R_RegisterShader( "gfx/misc/ns_sparks" );
    cgs.media.tracerShader = trap_R_RegisterShader( "gfx/misc/tracer" );
    cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );
    cgs.media.teamStatusBar = trap_R_RegisterShader( "gfx/2d/colorbar.tga" );

    CG_LoadingBarUpdate(10);

    cgs.media.bloodExplosionShader = trap_R_RegisterShader( "bloodExplosion" );
    cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");


    memset( cg_items, 0, sizeof( cg_items ) );
    memset( cg_weapons, 0, sizeof( cg_weapons ) );

    // only register the items that the server says we need
    strcpy( items, CG_ConfigString( CS_ITEMS) );

	// precache weapons
    CG_PrecacheWeapons( );

    cgs.media.shadowMarkShader = trap_R_RegisterShader( "markShadow" );
    cgs.media.wakeMarkShader = trap_R_RegisterShader( "ns_wake" );

    cgs.media.woodSmall = trap_R_RegisterModel( "models/props/wood_small.md3" );
    cgs.media.woodMedium = trap_R_RegisterModel( "models/props/wood_medium.md3" );
    cgs.media.woodBig = trap_R_RegisterModel( "models/props/wood_big.md3" );
    cgs.media.glassSmall = trap_R_RegisterModel( "models/props/glass_small.md3" );
    cgs.media.glassMedium = trap_R_RegisterModel( "models/props/glass_medium.md3" );
    cgs.media.glassBig = trap_R_RegisterModel( "models/props/glass_big.md3" );
    cgs.media.metalSmall = trap_R_RegisterModel( "models/props/metal_small.md3" );
    cgs.media.metalMedium = trap_R_RegisterModel( "models/props/metal_medium.md3" );
    cgs.media.metalBig = trap_R_RegisterModel( "models/props/metal_big.md3" );
    cgs.media.stoneSmall = trap_R_RegisterModel( "models/props/stone_small.md3" );
    cgs.media.stoneMedium = trap_R_RegisterModel( "models/props/stone_medium.md3" );
    cgs.media.stoneBig = trap_R_RegisterModel( "models/props/stone_big.md3" );

    cgs.media.nullModel = trap_R_RegisterModel("models/props/null.md3");

    cgs.media.woodSplinter = trap_R_RegisterModel("models/props/wood_splinter.md3");
    cgs.media.glassSplinter = trap_R_RegisterModel("models/props/glass_splinter.md3");
    cgs.media.softSplinter = trap_R_RegisterModel("models/props/soft_chip.md3");
    //
    cgs.media.ns_bloodtrailShader = trap_R_RegisterShader( "ns_bloodTrail" );

    cgs.media.briefcaseModel =	trap_R_RegisterModel ( "models/misc/suitcase/suitcase.md3" );
    cgs.media.briefcaseModel_vweap =	trap_R_RegisterModel ( "models/misc/suitcase/suitcase_vweap.md3" );
    cgs.media.shellRifle = trap_R_RegisterModel("models/misc/shells/shell_rifle.md3");
    cgs.media.shellPistol = trap_R_RegisterModel("models/misc/shells/shell_pistol.md3");
    cgs.media.shellShotgun = trap_R_RegisterModel("models/misc/shells/shell_shotgun.md3");

    cgs.media.sphereFlashModel = trap_R_RegisterModel("models/weaphits/sphere.md3");

    CG_LoadingBarUpdate(10);

    //
    // head
    //
    cgs.media.playerSealHelmet[0] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_s_desert.md3");
    cgs.media.playerSealHelmet[1] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_s_jungle.md3");
    cgs.media.playerSealHelmet[2] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_s_arctic.md3");
    cgs.media.playerSealHelmet[3] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_s_urban.md3");
    cgs.media.playerTangoHelmet[0] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_t_desert.md3");
    cgs.media.playerTangoHelmet[1] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_t_jungle.md3");
    cgs.media.playerTangoHelmet[2] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_t_arctic.md3");
    cgs.media.playerTangoHelmet[3] = trap_R_RegisterModel("models/players/heads/accessoires/helmet_t_urban.md3");

    CG_LoadingBarUpdate(10);

    for (i=0;i<5;i++)
    {
        cgs.media.bulletholes[BHOLE_NORMAL][i] = trap_R_RegisterShader( va("gfx/damage/bhole_stone_%i",i+1) );
        cgs.media.bulletholes[BHOLE_GLASS][i] = trap_R_RegisterShader( va("gfx/damage/bhole_glass_%i",i+1) );
        cgs.media.bulletholes[BHOLE_METAL][i] = trap_R_RegisterShader( va("gfx/damage/bhole_metal_%i",i+1) ); 
		cgs.media.bulletholes[BHOLE_SOFT][i] = trap_R_RegisterShader( va("gfx/damage/bhole_soft_%i",i+1) );
        cgs.media.bulletholes[BHOLE_WOOD][i] = trap_R_RegisterShader( va("gfx/damage/bhole_wood_%i",i+1) );

        cgs.media.bulletholes[BHOLE_STONE][i] = trap_R_RegisterShader( va("gfx/damage/bhole_stone_%i",i+1) );
        cgs.media.bulletholes[BHOLE_DIRT][i] = trap_R_RegisterShader( va("gfx/damage/bhole_dirt_%i",i+1) );
    }

    for (i=0;i<3;i++)
    {
        cgs.media.burnMarkShaders[i] = trap_R_RegisterShader( va("gfx/damage/bhole_explo_%i",i+1) );
    }

    // register the inline models
    cgs.numInlineModels = trap_CM_NumInlineModels();
    for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
        char	name[10];
        vec3_t			mins, maxs;
        int				j;

        Com_sprintf( name, sizeof(name), "*%i", i );
        cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
        trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
        for ( j = 0 ; j < 3 ; j++ ) {
            cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
        }
    }
    CG_LoadingBarUpdate(10);

    // register all the server specified models
    for (i=1 ; i<MAX_MODELS ; i++) {
        const char		*modelName;

        modelName = CG_ConfigString( CS_MODELS+i );
        if ( !modelName[0] ) {
            break;
        }
        cgs.gameModels[i] = trap_R_RegisterModel( modelName );
    }

    CG_LoadingBarUpdate(10);

    // new stuff
    cgs.media.cursor = trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
    cgs.media.sizeCursor = trap_R_RegisterShaderNoMip( "ui/assets/sizecursor.tga" );
    cgs.media.selectCursor = trap_R_RegisterShaderNoMip( "ui/assets/selectcursor.tga" );

    cgs.media.hdrShader = trap_R_RegisterShader( "hdr" );
	cgs.media.motionblurShader = trap_R_RegisterShader( "motionblur" );
	cgs.media.blurShader = trap_R_RegisterShader( "blur" );

}

/*
=======================
CG_PrecacheWeapons

register all weapons (done after loading map and finished connecting)
=======================
*/
int ticks = 0;

void CG_PrecacheWeapons( ) {
    int i,start;
  	
    start = trap_Milliseconds();

    for ( i = 1 ; i < WP_NUM_WEAPONS ; i++ ) { 
        CG_RegisterWeapon( i );
    }

	CG_Printf("Time to register weapons %i msec\n", trap_Milliseconds() - start );
}

/*
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString() {
    int i;
    cg.spectatorList[0] = 0;
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR ) {
            Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s%s%s     ",S_COLOR_WHITE, cgs.clientinfo[i].name,S_COLOR_WHITE));
        }
    }
    i = strlen(cg.spectatorList);
    if (i != cg.spectatorLen) {
        cg.spectatorLen = i;
        cg.spectatorWidth = -1;
    }
}


/*
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
    int		i;

    CG_LoadingClient(cg.clientNum);
    CG_NewClientInfo(cg.clientNum);

    for (i=0 ; i<MAX_CLIENTS ; i++) {
        const char		*clientInfo;

        if (cg.clientNum == i) {
            continue;
        }

        clientInfo = CG_ConfigString( CS_PLAYERS+i );
        if ( !clientInfo[0]) {
            continue;
        }
        CG_LoadingClient( i );
        CG_NewClientInfo( i );
    }
    CG_BuildSpectatorString();
}

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
    if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
        CG_Error( "CG_ConfigString: bad index: %i", index );
    }
    return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
    char	*s;
    char	parm1[MAX_QPATH], parm2[MAX_QPATH];

    // start the background music
    s = (char *)CG_ConfigString( CS_MUSIC );
    Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
    Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

    trap_S_StartBackgroundTrack( parm1, parm2 );
}
#ifdef MISSIONPACK
char *CG_GetMenuBuffer(const char *filename) {
    int	len;
    fileHandle_t	f;
    static char buf[MAX_MENUFILE];

    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( !f ) {
        trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
        return NULL;
    }
    if ( len >= MAX_MENUFILE ) {
        trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
        trap_FS_FCloseFile( f );
        return NULL;
    }

    trap_FS_Read( buf, len, f );
    buf[len] = 0;
    trap_FS_FCloseFile( f );

    return buf;
}

//
// ==============================
// new hud stuff ( mission pack )
// ==============================
//
qboolean CG_Asset_Parse(int handle) {
    pc_token_t token;
    const char *tempStr;

    if (!trap_PC_ReadToken(handle, &token))
        return qfalse;
    if (Q_stricmp(token.string, "{") != 0) {
        return qfalse;
    }

    while ( 1 ) {
        if (!trap_PC_ReadToken(handle, &token))
            return qfalse;

        if (Q_stricmp(token.string, "}") == 0) {
            return qtrue;
        }

        // font
        if (Q_stricmp(token.string, "font") == 0) {
            int pointSize;
            if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
                return qfalse;
            }
            cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.textFont);
            continue;
        }

        // smallFont
        if (Q_stricmp(token.string, "smallFont") == 0) {
            int pointSize;
            if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
                return qfalse;
            }
            cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.smallFont);
            continue;
        }

        // font
        if (Q_stricmp(token.string, "bigfont") == 0) {
            int pointSize;
            if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
                return qfalse;
            }
            cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.bigFont);
            continue;
        }

        // gradientbar
        if (Q_stricmp(token.string, "gradientbar") == 0) {
            if (!PC_String_Parse(handle, &tempStr)) {
                return qfalse;
            }
            cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
            continue;
        }

        // enterMenuSound
        if (Q_stricmp(token.string, "menuEnterSound") == 0) {
            if (!PC_String_Parse(handle, &tempStr)) {
                return qfalse;
            }
            cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        // exitMenuSound
        if (Q_stricmp(token.string, "menuExitSound") == 0) {
            if (!PC_String_Parse(handle, &tempStr)) {
                return qfalse;
            }
            cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        // itemFocusSound
        if (Q_stricmp(token.string, "itemFocusSound") == 0) {
            if (!PC_String_Parse(handle, &tempStr)) {
                return qfalse;
            }
            cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        // menuBuzzSound
        if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
            if (!PC_String_Parse(handle, &tempStr)) {
                return qfalse;
            }
            cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        if (Q_stricmp(token.string, "cursor") == 0) {
            if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
                return qfalse;
            }
            cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
            continue;
        }

        if (Q_stricmp(token.string, "fadeClamp") == 0) {
            if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "fadeCycle") == 0) {
            if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "fadeAmount") == 0) {
            if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "shadowX") == 0) {
            if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "shadowY") == 0) {
            if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "shadowColor") == 0) {
            if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
                return qfalse;
            }
            cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
            continue;
        }
#if 0
        if (Q_stricmp(token.string, "sealscolor") == 0) {
            if (!PC_Color_Parse(handle, &cgDC.Assets.team1color)) {
                return qfalse;
            }
            continue;
        }
        if (Q_stricmp(token.string, "tangoscolor") == 0) {
            if (!PC_Color_Parse(handle, &cgDC.Assets.team2color)) {
                return qfalse;
            }
            continue;
        }
#endif
        if (Q_stricmp(token.string, "noteamcolor") == 0) {
            if (!PC_Color_Parse(handle, &cgDC.Assets.dmcolor)) {
                return qfalse;
            }
            continue;
        }
    }
    return qfalse;
}

void CG_ParseMenu(const char *menuFile) {
    pc_token_t token;
    int handle;

    handle = trap_PC_LoadSource(menuFile);
    if (!handle)
        handle = trap_PC_LoadSource("ui/testhud.menu");
    if (!handle)
        return;

    while ( 1 ) {
        if (!trap_PC_ReadToken( handle, &token )) {
            break;
        }

        //if ( Q_stricmp( token, "{" ) ) {
        //	Com_Printf( "Missing { in menu file\n" );
        //	break;
        //}

        //if ( menuCount == MAX_MENUS ) {
        //	Com_Printf( "Too many menus!\n" );
        //	break;
        //}

        if ( token.string[0] == '}' ) {
            break;
        }

        if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
            if (CG_Asset_Parse(handle)) {
                continue;
            } else {
                break;
            }
        }


        if (Q_stricmp(token.string, "menudef") == 0) {
            // start a new menu
            Menu_New(handle);
        }
    }
    trap_PC_FreeSource(handle);
}

qboolean CG_Load_Menu(char **p) {
    char *token;

    token = COM_ParseExt(p, qtrue);

    if (token[0] != '{') {
        return qfalse;
    }

    while ( 1 ) {

        token = COM_ParseExt(p, qtrue);

        if (Q_stricmp(token, "}") == 0) {
            return qtrue;
        }

        if ( !token || token[0] == 0 ) {
            return qfalse;
        }

        CG_ParseMenu(token);
    }
    return qfalse;
}



void CG_LoadMenus(const char *menuFile) {
    char	*token;
    char *p;
    int	len, start;
    fileHandle_t	f;
    static char buf[MAX_MENUDEFFILE];

    start = trap_Milliseconds();

    len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
    if ( !f ) {
        trap_Error( va( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile ) );
        len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
        if (!f) {
            trap_Error( va( S_COLOR_RED "default HUD scriptfile not found: ui/hud.txt, unable to continue!\n", menuFile ) );
        }
    }

    if ( len >= MAX_MENUDEFFILE ) {
        trap_Error( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
        trap_FS_FCloseFile( f );
        return;
    }

    trap_FS_Read( buf, len, f );
    buf[len] = 0;
    trap_FS_FCloseFile( f );

    COM_Compress(buf);

    Menu_Reset();

    p = buf;

    while ( 1 ) {
        token = COM_ParseExt( &p, qtrue );
        if( !token || token[0] == 0 || token[0] == '}') {
            break;
        }

        //if ( Q_stricmp( token, "{" ) ) {
        //	Com_Printf( "Missing { in menu file\n" );
        //	break;
        //}

        //if ( menuCount == MAX_MENUS ) {
        //	Com_Printf( "Too many menus!\n" );
        //	break;
        //}

        if ( Q_stricmp( token, "}" ) == 0 ) {
            break;
        }

        if (Q_stricmp(token, "loadmenu") == 0) {
            if (CG_Load_Menu(&p)) {
                continue;
            } else {
                break;
            }
        }
    }

    // BLUTENGEL: 07.01.2004
    // removed this message as noone really needs it!
    // Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

}



static qboolean CG_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) {
    return qfalse;
}


static int CG_FeederCount(float feederID) {
    int i, count;
    count = 0;
    if (feederID == FEEDER_REDTEAM_LIST) {
        for (i = 0; i < cg.numScores; i++) {
            if (cg.scores[i].team == TEAM_RED) {
                count++;
            }
        }
    } else if (feederID == FEEDER_BLUETEAM_LIST) {
        for (i = 0; i < cg.numScores; i++) {
            if (cg.scores[i].team == TEAM_BLUE) {
                count++;
            }
        }
    } else if (feederID == FEEDER_SCOREBOARD) {
        return cg.numScores;
    }
    return count;
}


void CG_SetScoreSelection(void *p) {
    menuDef_t *menu = (menuDef_t*)p;
    playerState_t *ps = &cg.snap->ps;
    int i, red, blue;
    red = blue = 0;
    for (i = 0; i < cg.numScores; i++) {
        if (cg.scores[i].team == TEAM_RED) {
            red++;
        } else if (cg.scores[i].team == TEAM_BLUE) {
            blue++;
        }
        if (ps->clientNum == cg.scores[i].client) {
            cg.selectedScore = i;
        }
    }

    if (menu == NULL) {
        // just interested in setting the selected score
        return;
    }

    if ( cgs.gametype >= GT_TEAM ) {
        int feeder = FEEDER_REDTEAM_LIST;
        i = red;
        if (cg.scores[cg.selectedScore].team == TEAM_BLUE) {
            feeder = FEEDER_BLUETEAM_LIST;
            i = blue;
        }
        Menu_SetFeederSelection(menu, feeder, i, NULL);
    } else {
        Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
    }
}

// FIXME: might need to cache this info
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
    int i, count;
    if ( cgs.gametype >= GT_TEAM ) {
        count = 0;
        for (i = 0; i < cg.numScores; i++) {
            if (cg.scores[i].team == team) {
                if (count == index) {
                    *scoreIndex = i;
                    return &cgs.clientinfo[cg.scores[i].client];
                }
                count++;
            }
        }
    }
    *scoreIndex = index;
    return &cgs.clientinfo[ cg.scores[index].client ];
}

char *xp_to_rank(int xp,int team)
{
    switch (team)
    {
    case TEAM_BLUE:
        if ( xp >= 0 && xp <= 10 )
            return "Pfc";
        else if ( xp >= 11 && xp <= 15 )
            return "Cpl";
        else if ( xp >= 16 && xp <= 20 )
            return "Sgt";
        else if ( xp >= 21 && xp <= 30 )
            return "Msgt";
        else if ( xp >= 31 && xp <= 40 )
            return "2ndLt";
        else if ( xp >= 41 && xp <= 50 )
            return "1stLt";
        else if ( xp >= 51 && xp <= 60 )
            return "Capt";
        else if ( xp >=  61 && xp <= 70 )
            return "Maj";
        else if ( xp >= 71 && xp <= 80 )
            return "LtCol";
        else if ( xp >= 81 && xp <= 99 )
            return "Col";
        else if ( xp >= 100 )
            return "Gen";
        else
            return "";
        break;
    case TEAM_RED:
    default:
        if ( xp >= 0 && xp <= 10 )
            return "Seam";
        else if ( xp >= 11 && xp <= 15 )
            return "POf";
        else if ( xp >= 16 && xp <= 20 )
            return "CPof";
        else if ( xp >= 21 && xp <= 30 )
            return "MCPOf";
        else if ( xp >= 31 && xp <= 40 )
            return "Ens";
        else if ( xp >= 41 && xp <= 50 )
            return "Lt,j";
        else if ( xp >= 51 && xp <= 60 )
            return "Lt";
        else if ( xp >=  61 && xp <= 70 )
            return "LtCom";
        else if ( xp >= 71 && xp <= 80 )
            return "Com";
        else if ( xp >= 81 && xp <= 99 )
            return "Capt";
        else if ( xp >= 100 )
            return "Adm";
        else
            return "";
        break;
    }
}

char *status_to_string( int status )
{
    switch (status) {
    case MS_HEALTH5:
        return "Excellent";
    case MS_HEALTH4:
        return "Good";
    case MS_HEALTH3:
        return "Wounded";
    case MS_HEALTH2:
        return "Injured";
    case MS_HEALTH1:
        return "Poor";
    case MS_DEAD:
        return "Dead";
    case MS_BOMB:
        return "Got Bomb";
    default:
        return "Unknown";
    }
}
static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle) {
    //	gitem_t *item;
    int scoreIndex = 0;
    clientInfo_t *info = NULL;
    int team = -1;
    score_t *sp = NULL;

    *handle = -1;

    if (feederID == FEEDER_REDTEAM_LIST) {
        team = TEAM_RED;
    } else if (feederID == FEEDER_BLUETEAM_LIST) {
        team = TEAM_BLUE;
    }

    info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
    sp = &cg.scores[scoreIndex];

    if (info && info->infoValid) {
        switch (column) {
        case 0:
            if ( cgs.gametype >= GT_TEAM )
                return va("%s. %s",xp_to_rank(info->score, team),info->name);
            else
                return va("%s",info->name);
            break;
        case 1:
            if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) ) {
                return "Ready";
            }
            if (team == -1) {
                if (info->infoValid && info->team == TEAM_SPECTATOR ) {
                    return "Spectator";
                } else {
                    return "";
                }
            }
            else
            {
                return va("%s %s", ( !Q_stricmp( info->modelName, "vip_male") )?"V.I.P.":"", status_to_string( cg.playerStatus[ sp->client ] ) ) ;
            }
            break;
        case 2:
            if ( sp->ping == -1 ) {
                return "Connecting";
            }
            return va("%4i", sp->ping);
            /*

            */
            break;
        case 3:
            return va("%4i", sp->time);

            break;
        case 4:
            return va("%i", info->score);
            break;
        case 5:
            return "";//return info->name;
            break;
        case 6:
            return "";
            break;
        }
    }

    return "";
}

static qhandle_t CG_FeederItemImage(float feederID, int index) {
    return 0;
}

static void CG_FeederSelection(float feederID, int index) {
    if ( cgs.gametype >= GT_TEAM ) {
        int i, count;
        int team = (feederID == FEEDER_REDTEAM_LIST) ? TEAM_RED : TEAM_BLUE;
        count = 0;
        for (i = 0; i < cg.numScores; i++) {
            if (cg.scores[i].team == team) {
                if (index == count) {
                    cg.selectedScore = i;
                }
                count++;
            }
        }
    } else {
        cg.selectedScore = index;
    }
}
#endif
#if 1
static float CG_Cvar_Get(const char *cvar) {
    char buff[128];
    memset(buff, 0, sizeof(buff));
    trap_Cvar_VariableStringBuffer(cvar, buff, sizeof(buff));
    return atof(buff);
}
#endif

#ifdef MISSIONPACK
void CG_Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style) {
    CG_Text_Paint(x, y, scale, color, text, 0, limit, style);
}

static int CG_OwnerDrawWidth(int ownerDraw, float scale) {
    switch (ownerDraw) {
    case CG_GAME_TYPE:
        return CG_Text_Width(CG_GameTypeString(), scale, 0);
    case CG_GAME_STATUS:
        return CG_Text_Width(CG_GetGameStatusText(), scale, 0);
        break;
    case CG_KILLER:
        return CG_Text_Width(CG_GetKillerText(), scale, 0);
        break;


    }
    return 0;
}

static int CG_PlayCinematic(const char *name, float x, float y, float w, float h) {
    return trap_CIN_PlayCinematic(name, x, y, w, h, CIN_loop);
}

static void CG_StopCinematic(int handle) {
    trap_CIN_StopCinematic(handle);
}

static void CG_DrawCinematic(int handle, float x, float y, float w, float h) {
    trap_CIN_SetExtents(handle, x, y, w, h);
    trap_CIN_DrawCinematic(handle);
}

static void CG_RunCinematicFrame(int handle) {
    trap_CIN_RunCinematic(handle);
}


/*
================
configure the client weapon, set the current cvar
================
*/
void CG_DoWeaponConfig(char **args, int parameter )
{
    const char *name;

    //CG_Printf(S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: cg_main.c / CG_DoWeaponConfig\n");

    if (String_Parse(args, &name))
    {
        if (Q_stricmp(name, "setPrimaryWeapon") == 0)
        {

            trap_Cvar_Set("inven_Primary", va("%i", parameter) );
        }
        else if (Q_stricmp(name, "setSecondaryWeapon") == 0)
        {
            trap_Cvar_Set("inven_Secondary", va("%i",parameter) );
        }
        else if ( Q_stricmp(name, "setItem") == 0 )
        {
            char *cvarname = "cvarnone";
            char		var[MAX_TOKEN_CHARS];
            int	 Value;

            if ( parameter == ITEM_HELMET )
                cvarname = "inven_Helmet";
            else if ( parameter == ITEM_VEST )
                cvarname = "inven_Vest";
            else if ( parameter == ITEM_SCOPE )
                cvarname = "inven_Scope";
            else if( parameter == ITEM_GRENADELAUNCHER )
                cvarname = "inven_GrenadeLauncher";
            else if( parameter == ITEM_LASERSIGHT )
                cvarname = "inven_LaserSight";
            else if( parameter == ITEM_BAYONET )
                cvarname = "inven_Bayonet";
            else if( parameter == ITEM_SILENCER )
                cvarname = "inven_Silencer";

            trap_Cvar_VariableStringBuffer(cvarname, var , sizeof(var ) );
            Value = atoi(var );

            // items only got activate or disable
            if ( Value == 1 ) // if it's activated, disable it
                Value = 0;
            else
                Value = 1;

            trap_Cvar_Set(cvarname, va("%i",Value) );
        }
        else if ( Q_stricmp(name, "removeAllSecondary") == 0 )
        {
            Com_Printf("removed all Secondary\n");
        }
        else if ( Q_stricmp(name, "quitCGmenu") == 0 )
        {
            CG_EventHandling(CGAME_EVENT_NONE);
            trap_Key_SetCatcher(0);
        }
        else if ( Q_stricmp(name, "updateInventory") == 0 )
        {
            char *command;

            int primary ;
            int secondary;
            int priammo;
            int secammo;
            int mmgrenades;

            int grenades;
            int fl_grenades;
            int sm_grenades;
            int kevlar;
            int helmet;
            int scope;

            int gl;
            int bayonet;
            int lasersight;

            char		var[MAX_TOKEN_CHARS];

            gitem_t *it_primary;
            gitem_t *it_secondary;


            trap_Cvar_VariableStringBuffer("inven_lasersight", var , sizeof(var ) );
            lasersight = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_bayonet", var , sizeof(var ) );
            bayonet = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_grenadelauncher", var , sizeof(var ) );
            gl = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_scope", var , sizeof(var ) );
            scope = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_helmet", var , sizeof(var ) );
            helmet = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_kevlar", var , sizeof(var ) );
            kevlar = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_flash", var , sizeof(var ) );
            fl_grenades = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_smoke", var , sizeof(var ) );
            sm_grenades = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_mk26", var , sizeof(var ) );
            grenades = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_40mmgren", var , sizeof(var ) );
            mmgrenades = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
            primary = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof(var ) );
            secondary = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_primary", var , sizeof(var ) );
            priammo = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_secondary", var , sizeof(var ) );
            secammo = atoi(var);

            if ( primary )
                it_primary = BG_FindItemForWeapon( primary );
            if ( secondary )
                it_secondary = BG_FindItemForWeapon( secondary );

            //	<cmd>  <primary> <secondary> <PriAmmo> <SecAmmo> <40mm grenades> <Grenades> <Fl Grenades> <kevlar> <helmet> <scope> <gl> <bayonet> <lasersight>

            command = va("%i %i %i %i %i %i %i %i %i %i %i %i %i %i\n", primary,secondary,priammo,secammo,
                         mmgrenades,grenades,fl_grenades,sm_grenades, kevlar,helmet,scope,gl,bayonet,lasersight );

            CG_Printf("sended inventory: %s\n",command );
            //then send the commmand
            trap_SendClientCommand( va("inventory %s",command) );

        }
    }
}

void CG_Text( int when, const char *text )
{
    trap_SendConsoleCommand(text );
}

void CG_AssetCache( void )
{
    //if (Assets.textFont == NULL) {
    //  trap_R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
    //}
    //Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
    //Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
    cgDC.Assets.gradientBar			= trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgDC.Assets.scrollBar			= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
    cgDC.Assets.scrollBarArrowDown  = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
    cgDC.Assets.scrollBarArrowUp	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
    cgDC.Assets.scrollBarArrowLeft	= trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
    cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
    cgDC.Assets.scrollBarThumb		= trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
    cgDC.Assets.sliderBar			= trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
    cgDC.Assets.sliderThumb			= trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB ); 
	cgDC.Assets.checkBox[0]			= trap_R_RegisterShaderNoMip( ASSET_BOX_YES );
	cgDC.Assets.checkBox[1]			= trap_R_RegisterShaderNoMip( ASSET_BOX_NO );
}


static void CG_RunMenuScript(char **args) {
    const char *name;

    CG_Printf("uiscript \n" );

    if (String_Parse(args, &name))
    {
        if (Q_stricmp(name, "quitCGmenu") == 0) {
            CG_EventHandling(CGAME_EVENT_NONE);
            trap_Key_SetCatcher(0);
        }
    }

}
/*
=================
CG_LoadHudMenu();

=================
*/
void CG_LoadHudMenu() {
    char buff[1024];
    const char *hudSet;

    cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
    cgDC.setColor = &trap_R_SetColor;
    cgDC.drawHandlePic = &CG_DrawPic;
    cgDC.drawStretchPic = &trap_R_DrawStretchPic;
    cgDC.drawText = &CG_Text_Paint;
    cgDC.textWidth = &CG_Text_Width;
    cgDC.textHeight = &CG_Text_Height;
    cgDC.registerModel = &trap_R_RegisterModel;
    cgDC.modelBounds = &trap_R_ModelBounds;
    cgDC.fillRect = &CG_FillRect;
    cgDC.drawRect = &CG_DrawRect;
    cgDC.drawSides = &CG_DrawSides;
    cgDC.drawTopBottom = &CG_DrawTopBottom;
    cgDC.clearScene = &trap_R_ClearScene;
    cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
    cgDC.renderScene = &trap_R_RenderScene;
    cgDC.registerFont = &trap_R_RegisterFont;
    cgDC.ownerDrawItem = &CG_OwnerDraw;
    cgDC.getValue = &CG_GetValue;
    cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
    cgDC.runScript = &CG_RunMenuScript;
    cgDC.setWeapon = &CG_DoWeaponConfig; // ns!
    cgDC.getTeamColor = &CG_GetTeamColor;
    cgDC.setCVar = trap_Cvar_Set;
    cgDC.getCVarString = trap_Cvar_VariableStringBuffer;
    cgDC.getCVarValue = CG_Cvar_Get;
    cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
    //cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
    //cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
    cgDC.startLocalSound = &trap_S_StartLocalSound;
    cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
    cgDC.feederCount = &CG_FeederCount;
    cgDC.feederItemImage = &CG_FeederItemImage;
    cgDC.feederItemText = &CG_FeederItemText;
    cgDC.feederSelection = &CG_FeederSelection;
    //cgDC.setBinding = &trap_Key_SetBinding;
    //cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
    //cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
    cgDC.executeText = &CG_Text;
    cgDC.Error = &Com_Error;
    cgDC.Print = &Com_Printf;
    cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
    //cgDC.Pause = &CG_Pause;
    cgDC.registerSound = &trap_S_RegisterSound;
    cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
    cgDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
    cgDC.playCinematic = &CG_PlayCinematic;
    cgDC.stopCinematic = &CG_StopCinematic;
    cgDC.drawCinematic = &CG_DrawCinematic;
    cgDC.runCinematicFrame = &CG_RunCinematicFrame;

    Init_Display(&cgDC);

    //	String_Init();

    cgDC.cursor	= trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
    cgDC.whiteShader = trap_R_RegisterShaderNoMip( "white" );

    CG_AssetCache();

    Menu_Reset();

    trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
    hudSet = buff;
    if (hudSet[0] == '\0') {
        hudSet = "ui/hud.txt";
    }

    CG_LoadMenus(hudSet);

    // parse the hardcoded menu
    CG_ParseMenu("ui/hud_radio.menu");
}


#endif

/*
======================
CG_ParseCvarFile

Read a cvarfile for blocked cvars
======================
*/
#define CVAR_FILENAME	"scripts/cvars.cfg"

static  qboolean	CG_ParseCvarFile( void ) {
    char		*text_p;
    int			len;
    int			i;
    char		*token;
    char		text[20000];
    fileHandle_t	f;


    memset( &cg.blockedCvars , 0, sizeof(cg.blockedCvars ) );
    // load the file
    len = trap_FS_FOpenFile( CVAR_FILENAME, &f, FS_READ );
    if ( len <= 0 ) {
        return qfalse;
    }
    if ( len >= sizeof( text ) - 1 ) {
        CG_Printf( "File %s too long\n", CVAR_FILENAME );
        return qfalse;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;

    cg.num_blockedCvars = 0;

    // read information
    for ( i = 0 ; i < MAX_BLOCKEDCVARS ; i++ ) {

        // get string
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 ) {
            break;
        }
        strcpy( cg.blockedCvars[i].string, token );

        // get min value
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 ) {
            break;
        }
        strcpy( cg.blockedCvars[i].minvalue, token );

        // get max value
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 ) {
            break;
        }
        strcpy( cg.blockedCvars[i].maxvalue, token );

        // vid_restart
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 ) {
            break;
        }

        if ( !Q_stricmp( "yes", token ) )
            cg.blockedCvars[i].restart_video = qtrue;
        else
            cg.blockedCvars[i].restart_video = qfalse;

        cg.num_blockedCvars++;

        //	CG_Printf("parsed blocked cvar: %s min %s max %s restart video %i\n", cg.blockedCvars[i].string, cg.blockedCvars[i].minvalue, cg.blockedCvars[i].maxvalue, cg.blockedCvars[i].restart_video );
    }

    // BLUTENGEL 07.01.2004
    // removes as noone really wants to know that!
    // CG_Printf("parsed %i blocked cvars.\n", cg.num_blockedCvars );
    return qtrue;
}

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum )
{
    const char	*s;
    // clear everything
    memset( &cgs, 0, sizeof( cgs ) );
    memset( &cg, 0, sizeof( cg ) );
    memset( cg_entities, 0, sizeof(cg_entities) );
    memset( cg_weapons, 0, sizeof(cg_weapons) );
    memset( cg_items, 0, sizeof(cg_items) );

    cg.clientNum = clientNum;

    trap_SendConsoleCommand( "bind f1 \"vote yes\"\n" );
    trap_SendConsoleCommand( "bind f2 \"vote no\"\n" );

    // init loading bar.
    CG_LoadBarInit();

    CG_LoadingBarSetMax( 216 );

    cgs.processedSnapshotNum = serverMessageNum;
    cgs.serverCommandSequence = serverCommandSequence;

    // load a few needed things before we do any screen updates
    cgs.media.charsetShader		= trap_R_RegisterShader( "gfx/2d/bigchars" );
    cgs.media.whiteShader		= trap_R_RegisterShader( "white" );
    cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
    cgs.media.charsetPropGlow	= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
    cgs.media.charsetPropB		= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

    // init crosshair mod
    cg.crosshairMod = 0;
    cg.ladderWeaponTime = 0;
    cg.crosshairFadeIn = qtrue;
    cg.crosshairTime = cg.time;
    cg.crosshairFinishedChange = qtrue;
    cg.cameraActive = qfalse;

    CG_LoadingBarUpdate(5);

    CG_RegisterCvars();


    if ( r_lightVertex.integer )
        trap_Cvar_Set( "r_vertexlight", "1" );
    else
        trap_Cvar_Set( "r_vertexlight", "0" );

    CG_LoadingBarUpdate(3);

    CG_InitConsoleCommands();

    CG_LoadingBarUpdate(2);

    cg.weaponSelect = WP_KHURKURI;

    cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
    cgs.flagStatus = -1;
    // old servers

    // get the rendering configuration from the client system
    trap_GetGlconfig( &cgs.glconfig );
    cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
    cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

    // get the gamestate from the client system
    trap_GetGameState( &cgs.gameState );
    //
    CG_LoadingBarUpdate(2);

    // check version
    s = CG_ConfigString( CS_GAME_VERSION );
    if ( strcmp( s, GAME_VERSION ) ) {
        CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
    }
    else {
        // BLUTENGEL 07.01.2004
        // removed as noone really wants to know that!
        // CG_Printf("Connecting to Server:%s Client: %s\n", s, GAME_VERSION );
    }

    s = CG_ConfigString( CS_LEVEL_START_TIME );
    cgs.levelStartTime = atoi( s );

    CG_ParseServerinfo();

    CG_LoadingBarUpdate(5);


    // load the new map
    CG_LoadingString( "collision map" );

    CG_LoadingBarUpdate(2);

    trap_CM_LoadMap( cgs.mapname );


    CG_LoadingBarUpdate(5);
    //	String_Init();

    //	CG_AssetCache();
    CG_LoadHudMenu();      // load new hud stuff

    CG_LoadingBarUpdate(5);

    {
        const char	*info;
        const char	*s;

        strcpy( cgs.cleanLastMap, "unknown");

        info = CG_ConfigString( CS_SEALINFO );
        s = Info_ValueForKey( info, "lastmap" );

        strcpy( cgs.cleanLastMap, s );

        if ( !CG_ParseBriefingFile( cgs.cleanMapName ) )
            CG_Printf("Couldn't load briefing file for : %s\n", cgs.mapname);
        else
            cg.viewMissionInfo = qtrue; // show the misson info on first connect

        CG_ParseHelpFile( );
    }

    CG_LoadingBarUpdate(2);

    cg.loading = qtrue;		// force players to load instead of defer

    CG_LoadingString( "sounds" );

    CG_RegisterSounds();

    CG_LoadingBarUpdate(5);

    CG_LoadingString( "graphics" );

    CG_RegisterGraphics();

    CG_LoadingBarUpdate(30);

    CG_LoadingString( "Playermodels" );

    CG_CacheAllModels( );

    CG_RegisterClients();		// if low on memory, some clients will be deferred

    CG_LoadingBarUpdate(10);

    cg.loading = qfalse;	// future players will be deferred

    CG_InitLocalEntities();

    CG_LoadingBarUpdate(5);

    CG_InitMarkPolys();

    CG_LoadingBarUpdate(5);

    // remove the last loading update
    cg.infoScreenText[0] = 0;

    // Make sure we have update values (scores)
    CG_SetConfigValues();

    CG_StartMusic();

    CG_LoadingString( "" );

    CG_LoadingBarUpdate(5);

#ifdef MISSIONPACK
    CG_InitTeamChat();
#endif

    CG_ShaderStateChanged();

    trap_S_ClearLoopingSounds( qtrue );
    // Navy Seals ++
    CG_ParseWeaponAnimationFile("scripts/weapon_mp5.cfg", WP_MP5);
    CG_ParseWeaponAnimationFile("scripts/weapon_pdw.cfg", WP_PDW);
    CG_ParseWeaponAnimationFile("scripts/weapon_ak47-bg15.cfg", WP_AK47 );
    CG_ParseWeaponAnimationFile("scripts/weapon_mk23.cfg", WP_MK23);
    CG_ParseWeaponAnimationFile("scripts/weapon_mk23.cfg", WP_P9S);
    CG_ParseWeaponAnimationFile("scripts/weapon_glock30.cfg", WP_GLOCK);
    CG_ParseWeaponAnimationFile("scripts/weapon_psg1.cfg", WP_PSG1);
    CG_ParseWeaponAnimationFile("scripts/weapon_macmillan.cfg", WP_MACMILLAN);
    CG_ParseWeaponAnimationFile("scripts/weapon_m4-m203.cfg", WP_M4 );
    CG_ParseWeaponAnimationFile("scripts/weapon_knife_t.cfg", WP_KHURKURI);
    CG_ParseWeaponAnimationFile("scripts/weapon_knife_t.cfg", WP_SEALKNIFE);
    CG_ParseWeaponAnimationFile("scripts/weapon_870.cfg", WP_870);
    CG_ParseWeaponAnimationFile("scripts/weapon_m590.cfg", WP_M590);
    CG_ParseWeaponAnimationFile("scripts/weapon_g_mk26.cfg", WP_GRENADE);
    CG_ParseWeaponAnimationFile("scripts/weapon_c4.cfg", WP_C4);
    CG_ParseWeaponAnimationFile("scripts/weapon_mac10.cfg", WP_MAC10);
    CG_ParseWeaponAnimationFile("scripts/weapon_sw40t.cfg", WP_SW40T);
    CG_ParseWeaponAnimationFile("scripts/weapon_spas15.cfg", WP_SPAS15);
    CG_ParseWeaponAnimationFile("scripts/weapon_g_flashbang.cfg", WP_FLASHBANG);
    CG_ParseWeaponAnimationFile("scripts/weapon_m14.cfg", WP_M14 );
    CG_ParseWeaponAnimationFile("scripts/weapon_deagle.cfg", WP_DEAGLE );
    CG_ParseWeaponAnimationFile("scripts/weapon_sw629.cfg", WP_SW629 );
    CG_ParseWeaponAnimationFile("scripts/weapon_g_smoke.cfg", WP_SMOKE );
    CG_ParseWeaponAnimationFile("scripts/weapon_m249.cfg", WP_M249 );
    CG_ParseWeaponAnimationFile("scripts/weapon_sl8sd.cfg", WP_SL8SD );


    CG_ParseWeaponAnimationFile("scripts/weapon_bombcase.cfg", 28 );

    cgs.infoPicLeft = trap_R_RegisterShader( va("briefing/%s_left", cgs.cleanMapName ) );
    cgs.infoPicMiddle = trap_R_RegisterShader( va("briefing/%s_middle", cgs.cleanMapName ) );
    cgs.infoPicRight = trap_R_RegisterShader( va("briefing/%s_right", cgs.cleanMapName ) );


    CG_LoadingBarUpdate(10);

    // starting Anim In Idle [ keep in loop ]
    cg.predictedPlayerEntity.pe.weapAnim = WANIM_IDLE;
    cg.predictedPlayerEntity.pe.nextweapAnim = WANIM_IDLE;

    // init us w/ 0 XP points
    cg.xpPoints = 0;

    CG_ParseCvarFile();

    // clientscript
    CG_InitMemory();
    ClientScript_Init( );


    if ( r_lightVertex.integer )
    {
        trap_Cvar_Set( "r_vertexlight", "0" );
    }

    trap_SendConsoleCommand( ";exec nssl/autoexec.cfg;" );
    // Navy Seals --

}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
    // some mods may need to do cleanup work here,
    // like closing files or archiving session data
}


/*
==================
CG_EventHandling
==================
type 0 - no event handling
1 - team menu
2 - hud editor

*/
#ifndef MISSIONPACK
void CG_EventHandling(int type) {
}


#include "..\q3_ui\keycodes.h"

void CG_KeyEvent(int key, qboolean down) {

}

void CG_MouseEvent(int x, int y) {
}
#endif

