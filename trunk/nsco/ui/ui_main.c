// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================
 
USER INTERFACE MAIN
 
=======================================================================
*/

// use this to get a demo build without an explicit demo build, i.e. to get the demo ui files to build
//#define PRE_RELEASE_TADEMO

#include "ui_local.h"

#define DEBUGCHARACTER 0
#define INIT_PRECACHE 1

uiInfo_t uiInfo;

static const char *MonthAbbrev[] =
    {
        "Jan","Feb","Mar",
        "Apr","May","Jun",
        "Jul","Aug","Sep",
        "Oct","Nov","Dec"
    };
  
static const char *netSources[] =
    {
        "Local",
        "Mplayer",
        "Internet",
        "Favorites"
    };
static const int numNetSources = sizeof(netSources) / sizeof(const char*);

static const serverFilter_t serverFilters[] =
    {
        {"All", ""
        },
        {"Quake 3 Arena", "" },
        {"Team Arena", "missionpack" },
        {"Rocket Arena", "arena" },
        {"Alliance", "alliance" },
    };

static const char *teamArenaGameTypes[] =
    {
        "FFA", // free for all
        "TRAINING",
        "FTP", // free team play
        "TEAMOPS"
    };

static int const numTeamArenaGameTypes = sizeof(teamArenaGameTypes) / sizeof(const char*);


static const char *teamArenaGameNames[] =
    {
        "Free For All",
        "Tournament",
        "Single Player",
        "Team Teamplay",
        "Capture the Flag",
        "One Flag CTF",
        "Overload",
        "Harvester",
        "Team Operations",
    };

static int const numTeamArenaGameNames = sizeof(teamArenaGameNames) / sizeof(const char*);


static const int numServerFilters = sizeof(serverFilters) / sizeof(serverFilter_t);

static const char *sortKeys[] =
    {
        "Server Name",
        "Map Name",
        "Open Player Spots",
        "Game Type",
        "Ping Time"
    };
static const int numSortKeys = sizeof(sortKeys) / sizeof(const char*);

static char* netnames[] = {
                              "???",
                              "UDP",
                              "IPX",
                              NULL
                          };

static char quake3worldMessage[] = "Visit www.ns-co.net for News, Community, Events, Files";

static int gamecodetoui[] = {4,2,3,0,5,1,6};
static int uitogamecode[] = {4,6,2,3,1,5,7};


static void UI_StartServerRefresh(qboolean full);
static void UI_StopServerRefresh( void );
static void UI_DoServerRefresh( void );
static void UI_FeederSelection(float feederID, int index);
static void UI_BuildServerDisplayList(qboolean force);
static void UI_BuildServerStatus(qboolean force);
static void UI_BuildFindPlayerList(qboolean force);
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 );
static int UI_MapCountByGameType(qboolean singlePlayer);
static void UI_ParseGameInfo(const char *teamFile);
static void UI_ParseTeamInfo(const char *teamFile);
static const char *UI_SelectedMap(int index, int *actual);
static int UI_GetIndexFromSelection(int actual);

int ProcessNewUI( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 );

/*
================
cvars
================
*/

typedef struct
{
    vmCvar_t	*vmCvar;
    char		*cvarName;
    char		*defaultString;
    int			cvarFlags;
}
cvarTable_t;

vmCvar_t	ui_ffa_fraglimit;
vmCvar_t	ui_ffa_timelimit;

vmCvar_t	ui_tourney_fraglimit;
vmCvar_t	ui_tourney_timelimit;

vmCvar_t	ui_team_fraglimit;
vmCvar_t	ui_team_timelimit;
vmCvar_t	ui_team_friendly;

vmCvar_t	ui_ctf_capturelimit;
vmCvar_t	ui_ctf_timelimit;
vmCvar_t	ui_ctf_friendly;

vmCvar_t	ui_arenasFile;
vmCvar_t	ui_botsFile;
vmCvar_t	ui_spScores1;
vmCvar_t	ui_spScores2;
vmCvar_t	ui_spScores3;
vmCvar_t	ui_spScores4;
vmCvar_t	ui_spScores5;
vmCvar_t	ui_spAwards;
vmCvar_t	ui_spVideos;
vmCvar_t	ui_spSkill;

vmCvar_t	ui_spSelection;

vmCvar_t	ui_browserMaster;
vmCvar_t	ui_browserGameType;
vmCvar_t	ui_browserSortKey;
vmCvar_t	ui_browserShowFull;
vmCvar_t	ui_browserShowEmpty;

vmCvar_t	ui_brassTime;
vmCvar_t	ui_drawCrosshair;
vmCvar_t	ui_drawCrosshairNames;
vmCvar_t	ui_marks;

vmCvar_t	ui_server1;
vmCvar_t	ui_server2;
vmCvar_t	ui_server3;
vmCvar_t	ui_server4;
vmCvar_t	ui_server5;
vmCvar_t	ui_server6;
vmCvar_t	ui_server7;
vmCvar_t	ui_server8;
vmCvar_t	ui_server9;
vmCvar_t	ui_server10;
vmCvar_t	ui_server11;
vmCvar_t	ui_server12;
vmCvar_t	ui_server13;
vmCvar_t	ui_server14;
vmCvar_t	ui_server15;
vmCvar_t	ui_server16;

vmCvar_t	ui_cdkeychecked;

vmCvar_t	ui_redteam;
vmCvar_t	ui_redteam1;
vmCvar_t	ui_redteam2;
vmCvar_t	ui_redteam3;
vmCvar_t	ui_redteam4;
vmCvar_t	ui_redteam5;
vmCvar_t	ui_redteam6;
vmCvar_t	ui_blueteam;
vmCvar_t	ui_blueteam1;
vmCvar_t	ui_blueteam2;
vmCvar_t	ui_blueteam3;
vmCvar_t	ui_blueteam4;
vmCvar_t	ui_blueteam5;
vmCvar_t	ui_blueteam6;
vmCvar_t	ui_teamName;
vmCvar_t	ui_dedicated;
vmCvar_t	ui_gameType;
vmCvar_t	ui_netGameType;
vmCvar_t	ui_actualNetGameType;
vmCvar_t	ui_joinGameType;
vmCvar_t	ui_netSource;
vmCvar_t	ui_serverFilterType;
vmCvar_t	ui_opponentName;
vmCvar_t	ui_menuFiles;
vmCvar_t	ui_currentTier;
vmCvar_t	ui_currentMap;
vmCvar_t	ui_currentNetMap;
vmCvar_t	ui_mapIndex;
vmCvar_t	ui_currentOpponent;
vmCvar_t	ui_selectedPlayer;
vmCvar_t	ui_selectedPlayerName;
vmCvar_t	ui_lastServerRefresh_0;
vmCvar_t	ui_lastServerRefresh_1;
vmCvar_t	ui_lastServerRefresh_2;
vmCvar_t	ui_lastServerRefresh_3;
vmCvar_t	ui_singlePlayerActive;
vmCvar_t	ui_scoreAccuracy;
vmCvar_t	ui_scoreImpressives;
vmCvar_t	ui_scoreExcellents;
vmCvar_t	ui_scoreCaptures;
vmCvar_t	ui_scoreDefends;
vmCvar_t	ui_scoreAssists;
vmCvar_t	ui_scoreGauntlets;
vmCvar_t	ui_scoreScore;
vmCvar_t	ui_scorePerfect;
vmCvar_t	ui_scoreTeam;
vmCvar_t	ui_scoreBase;
vmCvar_t	ui_scoreTimeBonus;
vmCvar_t	ui_scoreSkillBonus;
vmCvar_t	ui_scoreShutoutBonus;
vmCvar_t	ui_scoreTime;
vmCvar_t	ui_captureLimit;
vmCvar_t	ui_fragLimit;
vmCvar_t	ui_smallFont;
vmCvar_t	ui_bigFont;
vmCvar_t	ui_findPlayer;
vmCvar_t	ui_Q3Model;
vmCvar_t	ui_hudFiles;
vmCvar_t	ui_recordSPDemo;
vmCvar_t	ui_realCaptureLimit;
vmCvar_t	ui_realWarmUp;
vmCvar_t	ui_serverStatusTimeOut;
vmCvar_t	ui_t_e_eyes;
vmCvar_t	ui_t_e_head;
vmCvar_t	ui_t_e_mouth;
vmCvar_t	ui_s_e_eyes;
vmCvar_t	ui_s_e_head;
vmCvar_t	ui_s_e_mouth;
vmCvar_t	ui_new;
vmCvar_t	ui_debug;
vmCvar_t	ui_initialized;
vmCvar_t	ui_teamArenaFirstRun;

vmCvar_t	ui_s_model;
vmCvar_t	ui_s_skin;

vmCvar_t	ui_t_model;
vmCvar_t	ui_t_skin;
vmCvar_t	g_logradio;
vmCvar_t	g_allowKnifes;
vmCvar_t	g_maxTeamKill;
vmCvar_t	g_roundTime;
vmCvar_t	g_keepCharacter;
vmCvar_t	g_stopbots;
vmCvar_t	g_overrideGoals;
vmCvar_t	g_invenTime;
vmCvar_t	g_bombTime;
vmCvar_t	cg_atmosphericEffects;
vmCvar_t	cg_lowEffects;
vmCvar_t	cg_customTracerColor;
vmCvar_t	cg_gunSmokeTime;
vmCvar_t	cg_particleTime;
vmCvar_t	cg_gunSmoke;
vmCvar_t	cg_enableTimeSelect;

vmCvar_t	ui_test;

vmCvar_t  ui_character;
vmCvar_t  ui_char_xp;
vmCvar_t  ui_char_old_xp;
vmCvar_t  ui_char_accuracy;
vmCvar_t  ui_char_strength;
vmCvar_t  ui_char_stamina;
vmCvar_t  ui_char_stealth;
vmCvar_t  ui_char_technical;
vmCvar_t  ui_char_speed;
vmCvar_t  ui_gamestate;

cvarTable_t		cvarTable[] = {
                               { &ui_ffa_fraglimit, "ui_ffa_fraglimit", "20", CVAR_ARCHIVE },
                               { &ui_ffa_timelimit, "ui_ffa_timelimit", "0", CVAR_ARCHIVE },

                               { &ui_tourney_fraglimit, "ui_tourney_fraglimit", "0", CVAR_ARCHIVE },
                               { &ui_tourney_timelimit, "ui_tourney_timelimit", "15", CVAR_ARCHIVE },

                               { &ui_team_fraglimit, "ui_team_fraglimit", "0", CVAR_ARCHIVE },
                               { &ui_team_timelimit, "ui_team_timelimit", "20", CVAR_ARCHIVE },
                               { &ui_team_friendly, "ui_team_friendly",  "1", CVAR_ARCHIVE },

                               { &ui_ctf_capturelimit, "ui_ctf_capturelimit", "8", CVAR_ARCHIVE },
                               { &ui_ctf_timelimit, "ui_ctf_timelimit", "30", CVAR_ARCHIVE },
                               { &ui_ctf_friendly, "ui_ctf_friendly",  "0", CVAR_ARCHIVE },

                               { &ui_arenasFile, "g_arenasFile", "", CVAR_INIT|CVAR_ROM },
                               { &ui_botsFile, "g_botsFile", "", CVAR_INIT|CVAR_ROM },
                               { &ui_spScores1, "g_spScores1", "", CVAR_ARCHIVE | CVAR_ROM },
                               { &ui_spScores2, "g_spScores2", "", CVAR_ARCHIVE | CVAR_ROM },
                               { &ui_spScores3, "g_spScores3", "", CVAR_ARCHIVE | CVAR_ROM },
                               { &ui_spScores4, "g_spScores4", "", CVAR_ARCHIVE | CVAR_ROM },
                               { &ui_spScores5, "g_spScores5", "", CVAR_ARCHIVE | CVAR_ROM },
                               { &ui_spAwards, "g_spAwards", "", CVAR_ARCHIVE | CVAR_ROM },
                               { &ui_spVideos, "g_spVideos", "", CVAR_ARCHIVE | CVAR_ROM },
                               { &ui_spSkill, "g_spSkill", "2", CVAR_ARCHIVE },

                               { &ui_spSelection, "ui_spSelection", "", CVAR_ROM },

                               { &ui_browserMaster, "ui_browserMaster", "0", CVAR_ARCHIVE },
                               { &ui_browserGameType, "ui_browserGameType", "0", CVAR_ARCHIVE },
                               { &ui_browserSortKey, "ui_browserSortKey", "4", CVAR_ARCHIVE },
                               { &ui_browserShowFull, "ui_browserShowFull", "1", CVAR_ARCHIVE },
                               { &ui_browserShowEmpty, "ui_browserShowEmpty", "1", CVAR_ARCHIVE },

                               { &ui_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
                               { &ui_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
                               { &ui_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
                               { &ui_marks, "cg_marks", "1", CVAR_ARCHIVE },

                               { &ui_server1, "server1", "", CVAR_ARCHIVE },
                               { &ui_server2, "server2", "", CVAR_ARCHIVE },
                               { &ui_server3, "server3", "", CVAR_ARCHIVE },
                               { &ui_server4, "server4", "", CVAR_ARCHIVE },
                               { &ui_server5, "server5", "", CVAR_ARCHIVE },
                               { &ui_server6, "server6", "", CVAR_ARCHIVE },
                               { &ui_server7, "server7", "", CVAR_ARCHIVE },
                               { &ui_server8, "server8", "", CVAR_ARCHIVE },
                               { &ui_server9, "server9", "", CVAR_ARCHIVE },
                               { &ui_server10, "server10", "", CVAR_ARCHIVE },
                               { &ui_server11, "server11", "", CVAR_ARCHIVE },
                               { &ui_server12, "server12", "", CVAR_ARCHIVE },
                               { &ui_server13, "server13", "", CVAR_ARCHIVE },
                               { &ui_server14, "server14", "", CVAR_ARCHIVE },
                               { &ui_server15, "server15", "", CVAR_ARCHIVE },
                               { &ui_server16, "server16", "", CVAR_ARCHIVE },
                               { &ui_cdkeychecked, "ui_cdkeychecked", "0", CVAR_ROM },
                               { &ui_new, "ui_new", "0", CVAR_TEMP },
                               { &ui_debug, "ui_debug", "0", CVAR_TEMP },
                               { &ui_initialized, "ui_initialized", "0", CVAR_TEMP },
                               { &ui_teamName, "ui_teamName", "Pagans", CVAR_ARCHIVE },
                               { &ui_opponentName, "ui_opponentName", "Stroggs", CVAR_ARCHIVE },
                               { &ui_redteam, "ui_redteam", "Pagans", CVAR_ARCHIVE },
                               { &ui_blueteam, "ui_blueteam", "Stroggs", CVAR_ARCHIVE },
                               { &ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE },
                               { &ui_gameType, "ui_gametype", "3", CVAR_ARCHIVE },
                               { &ui_joinGameType, "ui_joinGametype", "0", CVAR_ARCHIVE },
                               { &ui_netGameType, "ui_netGametype", "3", CVAR_ARCHIVE },
                               { &ui_actualNetGameType, "ui_actualNetGametype", "0", CVAR_ARCHIVE },
                               { &ui_redteam1, "ui_redteam1", "0", CVAR_ARCHIVE },
                               { &ui_redteam2, "ui_redteam2", "0", CVAR_ARCHIVE },
                               { &ui_redteam3, "ui_redteam3", "0", CVAR_ARCHIVE },
                               { &ui_redteam4, "ui_redteam4", "0", CVAR_ARCHIVE },
                               { &ui_redteam5, "ui_redteam5", "0", CVAR_ARCHIVE },
                               { &ui_redteam6, "ui_redteam6", "0", CVAR_ARCHIVE },
                               { &ui_blueteam1, "ui_blueteam1", "0", CVAR_ARCHIVE },
                               { &ui_blueteam2, "ui_blueteam2", "0", CVAR_ARCHIVE },
                               { &ui_blueteam3, "ui_blueteam3", "0", CVAR_ARCHIVE },
                               { &ui_blueteam4, "ui_blueteam4", "0", CVAR_ARCHIVE },
                               { &ui_blueteam5, "ui_blueteam5", "0", CVAR_ARCHIVE },
                               { &ui_blueteam6, "ui_blueteam6", "0", CVAR_ARCHIVE },
                               { &ui_netSource, "ui_netSource", "0", CVAR_ARCHIVE },
                               { &ui_menuFiles, "ui_menuFiles", "ui/menus.txt", CVAR_ARCHIVE },
                               { &ui_currentTier, "ui_currentTier", "0", CVAR_ARCHIVE },
                               { &ui_currentMap, "ui_currentMap", "0", CVAR_ARCHIVE },
                               { &ui_currentNetMap, "ui_currentNetMap", "0", CVAR_ARCHIVE },
                               { &ui_mapIndex, "ui_mapIndex", "0", CVAR_ARCHIVE },
                               { &ui_currentOpponent, "ui_currentOpponent", "0", CVAR_ARCHIVE },
                               { &ui_selectedPlayer, "cg_selectedPlayer", "0", CVAR_ARCHIVE},
                               { &ui_selectedPlayerName, "cg_selectedPlayerName", "", CVAR_ARCHIVE},
                               { &ui_lastServerRefresh_0, "ui_lastServerRefresh_0", "", CVAR_ARCHIVE},
                               { &ui_lastServerRefresh_1, "ui_lastServerRefresh_1", "", CVAR_ARCHIVE},
                               { &ui_lastServerRefresh_2, "ui_lastServerRefresh_2", "", CVAR_ARCHIVE},
                               { &ui_lastServerRefresh_3, "ui_lastServerRefresh_3", "", CVAR_ARCHIVE},
                               { &ui_singlePlayerActive, "ui_singlePlayerActive", "0", 0},
                               { &ui_scoreAccuracy, "ui_scoreAccuracy", "0", CVAR_ARCHIVE},
                               { &ui_scoreImpressives, "ui_scoreImpressives", "0", CVAR_ARCHIVE},
                               { &ui_scoreExcellents, "ui_scoreExcellents", "0", CVAR_ARCHIVE},
                               { &ui_scoreCaptures, "ui_scoreCaptures", "0", CVAR_ARCHIVE},
                               { &ui_scoreDefends, "ui_scoreDefends", "0", CVAR_ARCHIVE},
                               { &ui_scoreAssists, "ui_scoreAssists", "0", CVAR_ARCHIVE},
                               { &ui_scoreGauntlets, "ui_scoreGauntlets", "0",CVAR_ARCHIVE},
                               { &ui_scoreScore, "ui_scoreScore", "0", CVAR_ARCHIVE},
                               { &ui_scorePerfect, "ui_scorePerfect", "0", CVAR_ARCHIVE},
                               { &ui_scoreTeam, "ui_scoreTeam", "0 to 0", CVAR_ARCHIVE},
                               { &ui_scoreBase, "ui_scoreBase", "0", CVAR_ARCHIVE},
                               { &ui_scoreTime, "ui_scoreTime", "00:00", CVAR_ARCHIVE},
                               { &ui_scoreTimeBonus, "ui_scoreTimeBonus", "0", CVAR_ARCHIVE},
                               { &ui_scoreSkillBonus, "ui_scoreSkillBonus", "0", CVAR_ARCHIVE},
                               { &ui_scoreShutoutBonus, "ui_scoreShutoutBonus", "0", CVAR_ARCHIVE},
                               { &ui_fragLimit, "ui_fragLimit", "10",  0},
                               { &ui_captureLimit, "ui_captureLimit", "5", 0},
                               { &ui_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
                               { &ui_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
                               { &ui_findPlayer, "ui_findPlayer", "Sarge", CVAR_ARCHIVE},
                               { &ui_Q3Model, "ui_q3model", "0", CVAR_ARCHIVE},
                               { &ui_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},
                               { &ui_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
                               { &ui_teamArenaFirstRun, "ui_teamArenaFirstRun", "0", CVAR_ARCHIVE},
                               { &ui_realWarmUp, "g_warmup", "20", CVAR_ARCHIVE},
                               { &ui_realCaptureLimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART},
                               { &ui_serverStatusTimeOut, "ui_serverStatusTimeOut", "7000", CVAR_ARCHIVE},

                               { &ui_t_e_eyes, "ui_t_e_eyes"	, "", CVAR_ARCHIVE },
                               { &ui_t_e_head , "ui_t_e_head"	, "", CVAR_ARCHIVE },
                               { &ui_t_e_mouth , "ui_t_e_mouth"	, "", CVAR_ARCHIVE },

                               { &ui_s_e_eyes, "ui_s_e_eyes"	, "", CVAR_ARCHIVE },
                               { &ui_s_e_mouth , "ui_s_e_mouth"	, "", CVAR_ARCHIVE },
                               { &ui_s_e_head , "ui_s_e_head"	, "", CVAR_ARCHIVE },


                               { &ui_s_model, "ui_s_model"	, "s_medium", CVAR_ARCHIVE },
                               { &ui_s_skin , "ui_s_skin"	, "bruce", CVAR_ARCHIVE },

                               { &ui_t_model, "ui_t_model"	, "t_medium", CVAR_ARCHIVE },
                               { &ui_t_skin , "ui_t_skin"	, "jayant", CVAR_ARCHIVE },

                               { &g_logradio, "g_logradio", "1", 0 },
                               { &g_keepCharacter, "g_keepCharacter", "1", 0 },

                               // server infos [ vmcvars that need to be visible to the client]
                               { &g_allowKnifes, "g_allowKnifes", "1", CVAR_SERVERINFO | CVAR_LATCH  | CVAR_ARCHIVE },
                               { &g_maxTeamKill, "g_maxTeamKill", "3", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE },
                               { &g_roundTime, "roundtime", "5", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE },

                               { &g_overrideGoals, "g_overridegoals", "0", CVAR_SERVERINFO | CVAR_LATCH   },
                               { &g_invenTime, "g_inventoryUpdateTime", "3", CVAR_ARCHIVE },
                               { &g_bombTime, "g_bombTime", "80", CVAR_LATCH   },
                               { &cg_gunSmoke, "cg_gunSmoke", "1", CVAR_ARCHIVE },

                               { &cg_enableTimeSelect, "cg_enabletimeselect", "1", CVAR_ARCHIVE},

                               { &cg_atmosphericEffects, "cg_enviromentFX", "1", CVAR_ARCHIVE },
                               { &cg_lowEffects, "cg_enviromentFX_quality", "0", CVAR_ARCHIVE },
                               { &cg_customTracerColor, "cg_customTracerColor", "1", CVAR_ARCHIVE },
                               { &cg_gunSmokeTime, "cg_gunSmokeTime", "1500", CVAR_ARCHIVE },
                               { &cg_particleTime, "cg_particleTime", "500", CVAR_ARCHIVE },

                               { &ui_test, "ui_test", "0", CVAR_ARCHIVE },
                               { &ui_character, "ui_character", "C1111111", CVAR_ROM},
                               { &ui_char_xp, "ui_char_xp", "-1", CVAR_ROM},
                               { &ui_char_xp, "ui_char_old_xp", "-1", CVAR_ROM},
                               { &ui_char_accuracy, "ui_char_accuracy", "1", CVAR_ROM},
                               { &ui_char_accuracy, "ui_char_strength", "1", CVAR_ROM},
                               { &ui_char_accuracy, "ui_char_stamina", "1", CVAR_ROM},
                               { &ui_char_accuracy, "ui_char_stealth", "1", CVAR_ROM},
                               { &ui_char_accuracy, "ui_char_technical", "1", CVAR_ROM},
                               { &ui_char_accuracy, "ui_char_speed", "1", CVAR_ROM},
                               { &ui_char_accuracy, "ui_gamestate", "0", CVAR_ROM}

                           };

int		cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);

/*
================
vmMain
 
This is the only way control passes into the module.
This must be the very first function compiled into the .qvm file
================
*/

void _UI_Init( qboolean );
void _UI_Shutdown( void );
void _UI_KeyEvent( int key, qboolean down );
void _UI_MouseEvent( int dx, int dy );
void _UI_Refresh( int realtime );
qboolean _UI_IsFullscreen( void );
void _UI_Precache( qboolean skipPrecache );

int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  )
{
    switch ( command )
    {
    case UI_GETAPIVERSION:
        return UI_API_VERSION;

    case UI_INIT:
        _UI_Init(arg0);
        return 0;

    case UI_SHUTDOWN:
        _UI_Shutdown();
        return 0;

    case UI_KEY_EVENT:
        _UI_KeyEvent( arg0, arg1 );
        return 0;

    case UI_MOUSE_EVENT:
        _UI_MouseEvent( arg0, arg1 );
        return 0;

    case UI_REFRESH:
        _UI_Refresh( arg0 );
        return 0;

    case UI_IS_FULLSCREEN:
        return _UI_IsFullscreen();

	case UI_PRECACHE:
		_UI_Precache( arg0 );
		return 0;

    case UI_SET_ACTIVE_MENU:
        _UI_SetActiveMenu( arg0 );
        return 0;

    case UI_CONSOLE_COMMAND:
        return UI_ConsoleCommand(arg0); 

    case UI_DRAW_CONNECT_SCREEN:
        UI_DrawConnectScreen( arg0 );
        return 0;

    case UI_HASUNIQUECDKEY:				// mod authors need to observe this
        return qtrue; 
    }

    return -1;
}

/*
======================
UI_ParseCvarFile
 
Read a cvarfile for blocked cvars
======================
*/
#define MAX_BLOCKEDCVARS	64
#define CVAR_FILENAME	"scripts/cvars.cfg"

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct
{
    char		maxvalue[MAX_CVAR_VALUE_STRING];
    char		minvalue[MAX_CVAR_VALUE_STRING];
    qboolean	restart_video;
    char		string[MAX_CVAR_VALUE_STRING];
}
vmBlockedCvar_t;

vmBlockedCvar_t			blockedCvars[MAX_BLOCKEDCVARS];
int						num_blockedCvars;



qboolean	UI_ParseCvarFile( void )
{
    char		*text_p;
    int			len;
    int			i;
    char		*token;
    char		text[20000];
    fileHandle_t	f;


    memset( &blockedCvars , 0, sizeof(blockedCvars ) );
    // load the file
    len = trap_FS_FOpenFile( CVAR_FILENAME, &f, FS_READ );
    if ( len <= 0 )
    {
        return qfalse;
    }
    if ( len >= sizeof( text ) - 1 )
    {
        Com_Printf( "File %s too long\n", CVAR_FILENAME );
        return qfalse;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;

    num_blockedCvars = 0;

    // read information
    for ( i = 0 ; i < MAX_BLOCKEDCVARS ; i++ )
    {

        // get string
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 )
        {
            break;
        }
        strcpy( blockedCvars[i].string, token );

        // get min value
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 )
        {
            break;
        }
        strcpy( blockedCvars[i].minvalue, token );

        // get max value
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 )
        {
            break;
        }
        strcpy( blockedCvars[i].maxvalue, token );

        // vid_restart
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 )
        {
            break;
        }

        if ( !Q_stricmp( "yes", token ) )
            blockedCvars[i].restart_video = qtrue;
        else
            blockedCvars[i].restart_video = qfalse;

        num_blockedCvars++;

        //	CG_Printf("parsed blocked cvar: %s min %s max %s restart video %i\n", blockedCvars[i].string, blockedCvars[i].minvalue, blockedCvars[i].maxvalue, blockedCvars[i].restart_video );
    }

    // BLUTENGEL 07.01.2004
    // removed as noone really wants to know this!
    // Com_Printf("parsed %i blocked cvars.\n", num_blockedCvars );
    return qtrue;
}

void UI_CheckForceCvar( void )
{
    int i = 0;

    char str_value[MAX_CVAR_VALUE_STRING];
    float minvalue;
    float maxvalue;
    float value;

    qboolean	vid_restart = qfalse;


    for ( i = 0; i < num_blockedCvars ; i++)
    {
        trap_Cvar_VariableStringBuffer( blockedCvars[i].string, str_value, sizeof( str_value ) );

        minvalue = atof(blockedCvars[i].minvalue);
        maxvalue = atof(blockedCvars[i].maxvalue);

        value = atof( str_value );

        if ( value < minvalue )
        {
            trap_Cvar_Set( blockedCvars[i].string , blockedCvars[i].minvalue );
            Com_Printf("forced cvar %s to %f\n", blockedCvars[i].string, minvalue );

            if ( blockedCvars[i].restart_video )
                vid_restart = qtrue;
        }
        if ( value > maxvalue )
        {
            trap_Cvar_Set( blockedCvars[i].string , blockedCvars[i].maxvalue );
            Com_Printf("forced cvar %s to %f\n", blockedCvars[i].string, maxvalue );

            if ( blockedCvars[i].restart_video )
                vid_restart = qtrue;
        }
    }

    if ( vid_restart )
    {
        trap_Cmd_ExecuteText( EXEC_NOW, "vid_restart \n" );
    }//
}

void AssetCache()
{
    //if (Assets.textFont == NULL) {
    //}
    //Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
    //Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
    uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
    uiInfo.uiDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
    uiInfo.uiDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
    uiInfo.uiDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
    uiInfo.uiDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
    uiInfo.uiDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
    uiInfo.uiDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
    uiInfo.uiDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
    uiInfo.uiDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
	uiInfo.uiDC.Assets.checkBox[0] = trap_R_RegisterShaderNoMip( ASSET_BOX_YES );
	uiInfo.uiDC.Assets.checkBox[1] = trap_R_RegisterShaderNoMip( ASSET_BOX_NO );
}

void _UI_DrawSides(float x, float y, float w, float h, float size)
{
    UI_AdjustFrom640( &x, &y, &w, &h );
    size *= uiInfo.uiDC.xscale;
    trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
    trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

void _UI_DrawTopBottom(float x, float y, float w, float h, float size)
{
    UI_AdjustFrom640( &x, &y, &w, &h );
    size *= uiInfo.uiDC.yscale;
    trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
    trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}
/*
================
UI_DrawRect
 
Coordinates are 640*480 virtual values
=================
*/
void _UI_DrawRect( float x, float y, float width, float height, float size, const float *color )
{
    trap_R_SetColor( color );

    _UI_DrawTopBottom(x, y, width, height, size);
    _UI_DrawSides(x, y, width, height, size);

    trap_R_SetColor( NULL );
}




int Text_Width(const char *text, float scale, int limit)
{
    int count,len;
    float out;
    glyphInfo_t *glyph;
    float useScale;
    const char *s = text;
    fontInfo_t *font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_smallFont.value)
    {
        font = &uiInfo.uiDC.Assets.smallFont;
    }
    else if (scale >= ui_bigFont.value)
    {
        font = &uiInfo.uiDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;
    out = 0;
    if (text)
    {
        len = strlen(text);
        if (limit > 0 && len > limit)
        {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len)
        {
            if ( Q_IsColorString(s) )
            {
                s += 2;
                continue;
            }
            else
            {
                glyph = &font->glyphs[*s];
                out += glyph->xSkip;
                s++;
                count++;
            }
        }
    }
    return out * useScale;
}

int Text_Height(const char *text, float scale, int limit)
{
    int len, count;
    float max;
    glyphInfo_t *glyph;
    float useScale;
    const char *s = text;
    fontInfo_t *font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_smallFont.value)
    {
        font = &uiInfo.uiDC.Assets.smallFont;
    }
    else if (scale >= ui_bigFont.value)
    {
        font = &uiInfo.uiDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;
    max = 0;
    if (text)
    {
        len = strlen(text);
        if (limit > 0 && len > limit)
        {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len)
        {
            if ( Q_IsColorString(s) )
            {
                s += 2;
                continue;
            }
            else
            {
                glyph = &font->glyphs[*s];
                if (max < glyph->height)
                {
                    max = glyph->height;
                }
                s++;
                count++;
            }
        }
    }
    return max * useScale;
}

void Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader)
{
    float w, h;
    w = width * scale;
    h = height * scale;
    UI_AdjustFrom640( &x, &y, &w, &h );
    trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style)
{
    int len, count;
    vec4_t newColor;
    glyphInfo_t *glyph;
    float useScale;
    fontInfo_t *font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_smallFont.value)
    {
        font = &uiInfo.uiDC.Assets.smallFont;
    }
    else if (scale >= ui_bigFont.value)
    {
        font = &uiInfo.uiDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;
    if (text)
    {
        const char *s = text;
        trap_R_SetColor( color );
        memcpy(&newColor[0], &color[0], sizeof(vec4_t));
        len = strlen(text);
        if (limit > 0 && len > limit)
        {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len)
        {
            glyph = &font->glyphs[*s];
            //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
            //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
            if ( Q_IsColorString( s ) )
            {
                memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
                newColor[3] = color[3];
                trap_R_SetColor( newColor );
                s += 2;
                continue;
            }
            else
            {
                float yadj = useScale * glyph->top;
                if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE || style == ITEM_TEXTSTYLE_OUTLINESHADOWED)
                {
                    int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

                    colorBlack[3] = newColor[3];
                    trap_R_SetColor( colorBlack );

                    Text_PaintChar(x + ofs, y - yadj + ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }
                if (style == ITEM_TEXTSTYLE_OUTLINED || style == ITEM_TEXTSTYLE_OUTLINESHADOWED )
                {

                    int ofs = 1;
                    colorBlack[3] = newColor[3] - 0.25;
                    if ( colorBlack[3] <= 0.0f)
                        colorBlack[3] = 0.0f;

                    trap_R_SetColor( colorBlack );



                    Text_PaintChar(x + ofs, y - yadj + ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);


                    Text_PaintChar(x - ofs, y - yadj + ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x + ofs, y - yadj - ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x - ofs, y - yadj - ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x - ofs, y - yadj,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x + ofs, y - yadj,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);

                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }
                Text_PaintChar(x, y - yadj,
                               glyph->imageWidth,
                               glyph->imageHeight,
                               useScale,
                               glyph->s,
                               glyph->t,
                               glyph->s2,
                               glyph->t2,
                               glyph->glyph);

                x += (glyph->xSkip * useScale) + adjust;
                s++;
                count++;
            }
        }
        trap_R_SetColor( NULL );
    }
}

void Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style)
{
    int len, count;
    vec4_t newColor;
    glyphInfo_t *glyph, *glyph2;
    float yadj;
    float useScale;
    fontInfo_t *font = &uiInfo.uiDC.Assets.textFont;
    if (scale <= ui_smallFont.value)
    {
        font = &uiInfo.uiDC.Assets.smallFont;
    }
    else if (scale >= ui_bigFont.value)
    {
        font = &uiInfo.uiDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;
    if (text)
    {
        const char *s = text;
        trap_R_SetColor( color );
        memcpy(&newColor[0], &color[0], sizeof(vec4_t));
        len = strlen(text);
        if (limit > 0 && len > limit)
        {
            len = limit;
        }
        count = 0;
        glyph2 = &font->glyphs[cursor];
        while (s && *s && count < len)
        {
            glyph = &font->glyphs[*s];
            //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
            //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
            if ( Q_IsColorString( s ) )
            {
                memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
                newColor[3] = color[3];
                trap_R_SetColor( newColor );
                s += 2;
                continue;
            }
            else
            {
                yadj = useScale * glyph->top;
                if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE || style == ITEM_TEXTSTYLE_OUTLINESHADOWED)
                {
                    int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

                    colorBlack[3] = newColor[3];
                    trap_R_SetColor( colorBlack );

                    Text_PaintChar(x + ofs, y - yadj + ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }

                if (style == ITEM_TEXTSTYLE_OUTLINED || style == ITEM_TEXTSTYLE_OUTLINESHADOWED )
                {

                    int ofs = 1;
                    colorBlack[3] = newColor[3] - 0.25;
                    if ( colorBlack[3] <= 0.0f)
                        colorBlack[3] = 0.0f;

                    trap_R_SetColor( colorBlack );



                    Text_PaintChar(x + ofs, y - yadj + ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);


                    Text_PaintChar(x - ofs, y - yadj + ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x + ofs, y - yadj - ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x - ofs, y - yadj - ofs,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x - ofs, y - yadj,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                    Text_PaintChar(x + ofs, y - yadj,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);

                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }
                Text_PaintChar(x, y - yadj,
                               glyph->imageWidth,
                               glyph->imageHeight,
                               useScale,
                               glyph->s,
                               glyph->t,
                               glyph->s2,
                               glyph->t2,
                               glyph->glyph);

                // CG_DrawPic(x, y - yadj, scale * uiDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * uiDC.Assets.textFont.glyphs[text[i]].imageHeight, uiDC.Assets.textFont.glyphs[text[i]].glyph);
                yadj = useScale * glyph2->top;
                if (count == cursorPos && !((uiInfo.uiDC.realTime/BLINK_DIVISOR) & 1))
                {
                    Text_PaintChar(x, y - yadj,
                                   glyph2->imageWidth,
                                   glyph2->imageHeight,
                                   useScale,
                                   glyph2->s,
                                   glyph2->t,
                                   glyph2->s2,
                                   glyph2->t2,
                                   glyph2->glyph);
                }

                x += (glyph->xSkip * useScale);
                s++;
                count++;
            }
        }
        // need to paint cursor at end of text
        if (cursorPos == len && !((uiInfo.uiDC.realTime/BLINK_DIVISOR) & 1))
        {
            yadj = useScale * glyph2->top;
            Text_PaintChar(x, y - yadj,
                           glyph2->imageWidth,
                           glyph2->imageHeight,
                           useScale,
                           glyph2->s,
                           glyph2->t,
                           glyph2->s2,
                           glyph2->t2,
                           glyph2->glyph);

        }


        trap_R_SetColor( NULL );
    }
}


static void Text_Paint_Limit(float *maxX, float x, float y, float scale, vec4_t color, const char* text, float adjust, int limit)
{
    int len, count;
    vec4_t newColor;
    glyphInfo_t *glyph;
    if (text)
    {
        const char *s = text;
        float max = *maxX;
        float useScale;
        fontInfo_t *font = &uiInfo.uiDC.Assets.textFont;
        if (scale <= ui_smallFont.value)
        {
            font = &uiInfo.uiDC.Assets.smallFont;
        }
        else if (scale > ui_bigFont.value)
        {
            font = &uiInfo.uiDC.Assets.bigFont;
        }
        useScale = scale * font->glyphScale;
        trap_R_SetColor( color );
        len = strlen(text);
        if (limit > 0 && len > limit)
        {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len)
        {
            glyph = &font->glyphs[*s];
            if ( Q_IsColorString( s ) )
            {
                memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
                newColor[3] = color[3];
                trap_R_SetColor( newColor );
                s += 2;
                continue;
            }
            else
            {
                float yadj = useScale * glyph->top;
                if (Text_Width(s, useScale, 1) + x > max)
                {
                    *maxX = 0;
                    break;
                }
                Text_PaintChar(x, y - yadj,
                               glyph->imageWidth,
                               glyph->imageHeight,
                               useScale,
                               glyph->s,
                               glyph->t,
                               glyph->s2,
                               glyph->t2,
                               glyph->glyph);
                x += (glyph->xSkip * useScale) + adjust;
                *maxX = x;
                count++;
                s++;
            }
        }
        trap_R_SetColor( NULL );
    }

}


void UI_ShowPostGame(qboolean newHigh)
{
    trap_Cvar_Set ("cg_cameraOrbit", "0");
    trap_Cvar_Set("cg_thirdPerson", "0");
    trap_Cvar_Set( "sv_killserver", "1" );
    uiInfo.soundHighScore = newHigh;
    _UI_SetActiveMenu(UIMENU_POSTGAME);
}
/*
=================
_UI_Refresh
=================
*/

void UI_DrawCenteredPic(qhandle_t image, int w, int h)
{
    int x, y;
    x = (SCREEN_WIDTH - w) / 2;
    y = (SCREEN_HEIGHT - h) / 2;
    UI_DrawHandlePic(x, y, w, h, image);
}

int frameCount = 0;
int startTime;

#define UI_UPDATE_TIME	1000
#define	UI_FPS_FRAMES	4

void UI_CheckCharacterCvars( void )
{ 
    int acc			= trap_Cvar_VariableValue("char_accuracy");
    int str			= trap_Cvar_VariableValue("char_strength");
    int stl			= trap_Cvar_VariableValue("char_stealth");
    int sta			= trap_Cvar_VariableValue("char_stamina");
    int tech		= trap_Cvar_VariableValue("char_technical");
    int	spd			= trap_Cvar_VariableValue("char_speed"); 
    int flash		= trap_Cvar_VariableValue("inven_ammo_flash");
    int smoke		= trap_Cvar_VariableValue("inven_ammo_smoke");
    int gren40mm	= trap_Cvar_VariableValue("inven_ammo_40mmgren");
    int mk26		= trap_Cvar_VariableValue("inven_ammo_mk26"); 
    int	xp			= trap_Cvar_VariableValue("ui_char_xp");
    int xpo			= trap_Cvar_VariableValue("char_xp");

    if ( uiInfo.raise_level_acc < acc )
        uiInfo.raise_level_acc = acc;
    if ( uiInfo.raise_level_spd < spd )
        uiInfo.raise_level_spd = spd;
    if ( uiInfo.raise_level_str < str )
        uiInfo.raise_level_str = str;
    if ( uiInfo.raise_level_stl < stl )
        uiInfo.raise_level_stl = stl;
    if ( uiInfo.raise_level_sta < sta )
        uiInfo.raise_level_sta = sta;
    if ( uiInfo.raise_level_tec < tech )
        uiInfo.raise_level_tec = tech;

    // it seems like we changed our team. so reset these settings
    if ( acc == 1 && str == 1 && stl == 1 && sta == 1 && tech == 1 && spd == 1 &&
            ( uiInfo.raise_level_tec + uiInfo.raise_level_sta + uiInfo.raise_level_stl + uiInfo.raise_level_str + uiInfo.raise_level_spd + uiInfo.raise_level_acc ) > 6
       )
    {
        uiInfo.raise_level_acc = acc;
        uiInfo.raise_level_spd = spd;
        uiInfo.raise_level_str = str;
        uiInfo.raise_level_stl = stl;
        uiInfo.raise_level_sta = sta;
        uiInfo.raise_level_tec = tech;
    }  

	if (flash + smoke + gren40mm + mk26 > SEALS_MAX_GRENADES)
    {
        trap_Cvar_Set("inven_ammo_flash", va("%i", 1) );
        trap_Cvar_Set("inven_ammo_smoke", va("%i", 1) );
        trap_Cvar_Set("inven_ammo_40mmgren", va("%i", 1) );
        trap_Cvar_Set("inven_ammo_mk26", va("%i", 1) );
    } 

	// new menu hack
    if (xp < 0)
    {	// first time on server
        if (xpo != 0)
        {
            trap_Cvar_Set("ui_char_xp", va("%i", xpo) );
            trap_Cvar_Set("ui_char_old_xp", va("%i", xpo) );
        }
    }
    else
    {	// intermission
        xpo = trap_Cvar_VariableValue("ui_char_old_xp");
        xp = trap_Cvar_VariableValue("char_xp");

        if (xp > xpo)
        {
            trap_Cvar_Set("ui_char_old_xp", va("%i", xp));

            xp -= xpo;
            xpo = trap_Cvar_VariableValue("ui_char_xp");
            xpo += xp;
            trap_Cvar_Set("ui_char_xp", va("%i", xpo));
        }
    }
}
void _UI_Refresh( int realtime )
{
    static int index;
    static int	previousTimes[UI_FPS_FRAMES];
  
    if ( !( trap_Key_GetCatcher() & KEYCATCH_UI ) ) {
    	return;
    }
   
    uiInfo.uiDC.frameTime = realtime - uiInfo.uiDC.realTime;
    uiInfo.uiDC.realTime = realtime;

    previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiDC.frameTime;
    index++;
    if ( index > UI_FPS_FRAMES )
    {
        int i, total;
        // average multiple frames together to smooth changes out a bit
        total = 0;
        for ( i = 0 ; i < UI_FPS_FRAMES ; i++ )
        {
            total += previousTimes[i];
        }
        if ( !total )
        {
            total = 1;
        }
        uiInfo.uiDC.FPS = 1000 * UI_FPS_FRAMES / total;
    }
 
    UI_UpdateCvars(); 

    if (Menu_Count() > 0)
    {
        // paint all the menus
        Menu_PaintAll();
        // refresh server browser list
        UI_DoServerRefresh();
        // refresh server status
        UI_BuildServerStatus(qfalse);
        // refresh find player list
        UI_BuildFindPlayerList(qfalse);
    } 
   
	UI_CheckCharacterCvars();
    // draw cursor
    UI_SetColor( NULL ); 
#ifdef INIT_PRECACHE
	if ( uiInfo.initDone == qfalse )
	{		
		UI_DrawHandlePic( 0, 0, 640, 480,   uiInfo.uiDC.loadingShader  );
	}
    else 
#endif	
	if (Menu_Count() > 0)
    {
        float x = uiInfo.uiDC.cursorx-4;
        float y = uiInfo.uiDC.cursory-4; 
        qhandle_t hShader = uiInfo.uiDC.Assets.cursor;

        if ( uiInfo.uiDC.cursor_lbutton_dn )
            hShader = uiInfo.uiDC.Assets.cursor_dn;
 
        UI_AdjustFrom640( &x, &y, NULL,NULL );
        trap_R_DrawStretchPic( x, y, 32, 32, 0, 0, 1, 1, hShader  );
    }

#ifndef NDEBUG
    if (uiInfo.uiDC.debug)
    {
        // cursor coordinates
        //FIXME
        //UI_DrawString( 0, 0, va("(%d,%d)",uis.cursorx,uis.cursory), UI_LEFT|UI_SMALLFONT, colorMdGrey );
    }
#endif

}

/*
=================
_UI_Shutdown
=================
*/
void _UI_Shutdown( void )
{
    trap_LAN_SaveCachedServers();
}

char *defaultMenu = NULL;

char *GetMenuBuffer(const char *filename)
{
    int	len;
    fileHandle_t	f;
    static char buf[MAX_MENUFILE];

    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( !f )
    {
        trap_Print( va( S_COLOR_RED "menu file not found: %s, using default\n", filename ) );
        return defaultMenu;
    }
    if ( len >= MAX_MENUFILE )
    {
        trap_Print( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE ) );
        trap_FS_FCloseFile( f );
        return defaultMenu;
    }

    trap_FS_Read( buf, len, f );
    buf[len] = 0;
    trap_FS_FCloseFile( f );
    //COM_Compress(buf);
    return buf;

}

qboolean Asset_Parse(int handle)
{
    pc_token_t token;
    const char *tempStr;

    if (!trap_PC_ReadToken(handle, &token))
        return qfalse;
    if (Q_stricmp(token.string, "{") != 0)
    {
        return qfalse;
    }

    while ( 1 )
    {

        memset(&token, 0, sizeof(pc_token_t));

        if (!trap_PC_ReadToken(handle, &token))
            return qfalse;

        if (Q_stricmp(token.string, "}") == 0)
        {
            return qtrue;
        }

        // font
        if (Q_stricmp(token.string, "font") == 0)
        {
            int pointSize;
            if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle,&pointSize))
            {
                return qfalse;
            }
            trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.textFont);
            uiInfo.uiDC.Assets.fontRegistered = qtrue;
            continue;
        }

        if (Q_stricmp(token.string, "smallFont") == 0)
        {
            int pointSize;
            if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle,&pointSize))
            {
                return qfalse;
            }
            trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.smallFont);
            continue;
        }

        if (Q_stricmp(token.string, "bigFont") == 0)
        {
            int pointSize;
            if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle,&pointSize))
            {
                return qfalse;
            }
            trap_R_RegisterFont(tempStr, pointSize, &uiInfo.uiDC.Assets.bigFont);
            continue;
        }


        // gradientbar
        if (Q_stricmp(token.string, "gradientbar") == 0)
        {
            if (!PC_String_Parse(handle, &tempStr))
            {
                return qfalse;
            }
            uiInfo.uiDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
            continue;
        }

        // enterMenuSound
        if (Q_stricmp(token.string, "menuEnterSound") == 0)
        {
            if (!PC_String_Parse(handle, &tempStr))
            {
                return qfalse;
            }
            uiInfo.uiDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        // exitMenuSound
        if (Q_stricmp(token.string, "menuExitSound") == 0)
        {
            if (!PC_String_Parse(handle, &tempStr))
            {
                return qfalse;
            }
            uiInfo.uiDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        // itemFocusSound
        if (Q_stricmp(token.string, "itemFocusSound") == 0)
        {
            if (!PC_String_Parse(handle, &tempStr))
            {
                return qfalse;
            } 
            uiInfo.uiDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        // menuBuzzSound
        if (Q_stricmp(token.string, "menuBuzzSound") == 0)
        {
            if (!PC_String_Parse(handle, &tempStr))
            {
                return qfalse;
            }
            uiInfo.uiDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
            continue;
        }

        if (Q_stricmp(token.string, "cursor") == 0)
        {
            if (!PC_String_Parse(handle, &uiInfo.uiDC.Assets.cursorStr))
            {
                return qfalse;
            }
            uiInfo.uiDC.Assets.cursor = trap_R_RegisterShaderNoMip( uiInfo.uiDC.Assets.cursorStr);
            continue;
        }
        if (Q_stricmp(token.string, "cursor_dn") == 0)
        {
            if (!PC_String_Parse(handle, &uiInfo.uiDC.Assets.cursorStr))
            {
                return qfalse;
            }
            uiInfo.uiDC.Assets.cursor_dn = trap_R_RegisterShaderNoMip( uiInfo.uiDC.Assets.cursorStr);
            continue;
        }

        if (Q_stricmp(token.string, "fadeClamp") == 0)
        {
            if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeClamp))
            {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "fadeCycle") == 0)
        {
            if (!PC_Int_Parse(handle, &uiInfo.uiDC.Assets.fadeCycle))
            {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "fadeAmount") == 0)
        {
            if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.fadeAmount))
            {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "shadowX") == 0)
        {
            if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowX))
            {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "shadowY") == 0)
        {
            if (!PC_Float_Parse(handle, &uiInfo.uiDC.Assets.shadowY))
            {
                return qfalse;
            }
            continue;
        }

        if (Q_stricmp(token.string, "shadowColor") == 0)
        {
            if (!PC_Color_Parse(handle, &uiInfo.uiDC.Assets.shadowColor))
            {
                return qfalse;
            }
            uiInfo.uiDC.Assets.shadowFadeClamp = uiInfo.uiDC.Assets.shadowColor[3];
            continue;
        }

    }
    return qfalse;
}

void Font_Report()
{
    int i;
    Com_Printf("Font Info\n");
    Com_Printf("=========\n");
    for ( i = 32; i < 96; i++)
    {
        Com_Printf("Glyph handle %i: %i\n", i, uiInfo.uiDC.Assets.textFont.glyphs[i].glyph);
    }
}

void UI_Report()
{
    String_Report();
    //Font_Report();

}

void UI_ParseMenu(const char *menuFile)
{
    int handle;
    pc_token_t token;

    //	Com_Printf("Parsing menu file:%s\n", menuFile);

    handle = trap_PC_LoadSource(menuFile);
    if (!handle)
    {
        return;
    }

    while ( 1 )
    {
        memset(&token, 0, sizeof(pc_token_t));
        if (!trap_PC_ReadToken( handle, &token ))
        {
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

        if ( token.string[0] == '}' )
        {
            break;
        }

        if (Q_stricmp(token.string, "assetGlobalDef") == 0)
        {
            if (Asset_Parse(handle))
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (Q_stricmp(token.string, "menudef") == 0)
        {
            // start a new menu
            Menu_New(handle);
        }
    }
    trap_PC_FreeSource(handle);
}

qboolean Load_Menu(int handle)
{
    pc_token_t token;

    if (!trap_PC_ReadToken(handle, &token))
        return qfalse;
    if (token.string[0] != '{')
    {
        return qfalse;
    }

    while ( 1 )
    {

        if (!trap_PC_ReadToken(handle, &token))
            return qfalse;

        if ( token.string[0] == 0 )
        {
            return qfalse;
        }

        if ( token.string[0] == '}' )
        {
            return qtrue;
        }

        UI_ParseMenu(token.string);
    }
    return qfalse;
}

void UI_LoadMenus(const char *menuFile, qboolean reset)
{
    pc_token_t token;
    int handle;
    int start;

    start = trap_Milliseconds();

    handle = trap_PC_LoadSource( menuFile );
    if (!handle)
    {
        trap_Error( va( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile ) );
        handle = trap_PC_LoadSource( "ui/menus.txt" );
        if (!handle)
        {
            trap_Error( va( S_COLOR_RED "default menu file not found: ui/menus.txt, unable to continue!\n", menuFile ) );
        }
    }

    ui_new.integer = 1;

    if (reset)
    {
        Menu_Reset();
    }

    while ( 1 )
    {
        if (!trap_PC_ReadToken(handle, &token))
            break;
        if( token.string[0] == 0 || token.string[0] == '}')
        {
            break;
        }

        if ( token.string[0] == '}' )
        {
            break;
        }

        if (Q_stricmp(token.string, "loadmenu") == 0)
        {
            if (Load_Menu(handle))
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }

    // BLUTENGEL 07.01.2004
    // removed as noone really wants to know that!
    // Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

    trap_PC_FreeSource( handle );
}

void UI_Load()
{
    char lastName[1024];
    menuDef_t *menu = Menu_GetFocused();
    char *menuSet = UI_Cvar_VariableString("ui_menuFiles");
    if (menu && menu->window.name)
    {
        strcpy(lastName, menu->window.name);
    }
    if (menuSet == NULL || menuSet[0] == '\0')
    {
        menuSet = "ui/menus.txt";
    }

    String_Init();

#ifdef PRE_RELEASE_TADEMO

    UI_ParseGameInfo("demogameinfo.txt");
#else

    UI_ParseGameInfo("gameinfo.txt");
    UI_LoadArenas();
#endif

    UI_LoadMenus(menuSet, qtrue);
    Menus_CloseAll();
    Menus_ActivateByName(lastName);

}

static const char *handicapValues[] =
    {"None","95","90","85","80","75","70","65","60","55","50","45","40","35","30","25","20","15","10","5",NULL
    };
static int numHandicaps = sizeof(handicapValues) / sizeof(const char*);

static void UI_DrawHandicap(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    int i, h;

    h = Com_Clamp( 5, 100, trap_Cvar_VariableValue("handicap") );
    i = 20 - h / 5;

    Text_Paint(rect->x, rect->y, scale, color, handicapValues[i], 0, 0, textStyle);
}

static void UI_DrawClanName(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    Text_Paint(rect->x, rect->y, scale, color, UI_Cvar_VariableString("ui_teamName"), 0, 0, textStyle);
}


static void UI_SetCapFragLimits(qboolean uiVars)
{
    int cap = 5;
    int frag = 10;

    if (uiVars)
    {
        trap_Cvar_Set("ui_captureLimit", va("%d", cap));
        trap_Cvar_Set("ui_fragLimit", va("%d", frag));
    }
    else
    {
        trap_Cvar_Set("capturelimit", va("%d", cap));
        trap_Cvar_Set("fraglimit", va("%d", frag));
    }
}
// ui_gameType assumes gametype 0 is -1 ALL and will not show
static void UI_DrawGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[ui_gameType.integer].gameType, 0, 0, textStyle);
}

static void UI_DrawNetGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    if (ui_netGameType.integer < 0 || ui_netGameType.integer > uiInfo.numGameTypes)
    {
        trap_Cvar_Set("ui_netGameType", "0");
        trap_Cvar_Set("ui_actualNetGameType", "0");
    }
    Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[ui_netGameType.integer].gameType , 0, 0, textStyle);
}

static void UI_DrawJoinGameType(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    if (ui_joinGameType.integer < 0 || ui_joinGameType.integer > uiInfo.numJoinGameTypes)
    {
        trap_Cvar_Set("ui_joinGameType", "0");
    }
    Text_Paint(rect->x, rect->y, scale, color, uiInfo.joinGameTypes[ui_joinGameType.integer].gameType , 0, 0, textStyle);
}



static int UI_TeamIndexFromName(const char *name)
{
    int i;

    if (name && *name)
    {
        for (i = 0; i < uiInfo.teamCount; i++)
        {
            if (Q_stricmp(name, uiInfo.teamList[i].teamName) == 0)
            {
                return i;
            }
        }
    }

    return 0;

}

static void UI_DrawClanLogo(rectDef_t *rect, float scale, vec4_t color)
{
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
    if (i >= 0 && i < uiInfo.teamCount)
    {
        trap_R_SetColor( color );

        if (uiInfo.teamList[i].teamIcon == -1)
        {
            uiInfo.teamList[i].teamIcon = trap_R_RegisterShaderNoMip(uiInfo.teamList[i].imageName);
            uiInfo.teamList[i].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
            uiInfo.teamList[i].teamIcon_Name = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
        }

        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
        trap_R_SetColor(NULL);
    }
}

static void UI_DrawClanCinematic(rectDef_t *rect, float scale, vec4_t color)
{
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
    if (i >= 0 && i < uiInfo.teamCount)
    {

        if (uiInfo.teamList[i].cinematic >= -2)
        {
            if (uiInfo.teamList[i].cinematic == -1)
            {
                uiInfo.teamList[i].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.teamList[i].imageName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
            }
            if (uiInfo.teamList[i].cinematic >= 0)
            {
                trap_CIN_RunCinematic(uiInfo.teamList[i].cinematic);
                trap_CIN_SetExtents(uiInfo.teamList[i].cinematic, rect->x, rect->y, rect->w, rect->h);
                trap_CIN_DrawCinematic(uiInfo.teamList[i].cinematic);
            }
            else
            {
                uiInfo.teamList[i].cinematic = -2;
            }
        }
        else
        {
            trap_R_SetColor( color );
            UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
            trap_R_SetColor(NULL);
        }
    }

}

static void UI_DrawPreviewCinematic(rectDef_t *rect, float scale, vec4_t color)
{
    if (uiInfo.previewMovie > -2)
    {
        uiInfo.previewMovie = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.movieList[uiInfo.movieIndex]), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
        if (uiInfo.previewMovie >= 0)
        {
            trap_CIN_RunCinematic(uiInfo.previewMovie);
            trap_CIN_SetExtents(uiInfo.previewMovie, rect->x, rect->y, rect->w, rect->h);
            trap_CIN_DrawCinematic(uiInfo.previewMovie);
        }
        else
        {
            uiInfo.previewMovie = -2;
        }
    }

}


 

static void UI_DrawTeamName(rectDef_t *rect, float scale, vec4_t color, qboolean blue, int textStyle)
{
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString((blue) ? "ui_blueTeam" : "ui_redTeam"));
    if (i >= 0 && i < uiInfo.teamCount)
    {
        Text_Paint(rect->x, rect->y, scale, color, va("%s: %s", (blue) ? "Tangos" : "Seals", uiInfo.teamList[i].teamName),0, 0, textStyle);
    }
}
 
static void UI_DrawMapPreview(rectDef_t *rect, float scale, vec4_t color, qboolean net)
{
    int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
    if (map < 0 || map > uiInfo.mapCount)
    {
        if (net)
        {
            ui_currentNetMap.integer = 0;
            trap_Cvar_Set("ui_currentNetMap", "0");
        }
        else
        {
            ui_currentMap.integer = 0;
            trap_Cvar_Set("ui_currentMap", "0");
        }
        map = 0;
    }

    if (uiInfo.mapList[map].levelShot == -1)
    {
        uiInfo.mapList[map].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[map].imageName);
    }

    if (uiInfo.mapList[map].levelShot > 0)
    {
        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.mapList[map].levelShot);
    }
    else
    {
        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("gfx/2d/unknownmap"));
    }
}


static void UI_DrawMapTimeToBeat(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    int minutes, seconds, time;
    if (ui_currentMap.integer < 0 || ui_currentMap.integer > uiInfo.mapCount)
    {
        ui_currentMap.integer = 0;
        trap_Cvar_Set("ui_currentMap", "0");
    }

    time = uiInfo.mapList[ui_currentMap.integer].timeToBeat[uiInfo.gameTypes[ui_gameType.integer].gtEnum];

    minutes = time / 60;
    seconds = time % 60;

    Text_Paint(rect->x, rect->y, scale, color, va("%02i:%02i", minutes, seconds), 0, 0, textStyle);
}



static void UI_DrawMapCinematic(rectDef_t *rect, float scale, vec4_t color, qboolean net)
{

    int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
    if (map < 0 || map > uiInfo.mapCount)
    {
        if (net)
        {
            ui_currentNetMap.integer = 0;
            trap_Cvar_Set("ui_currentNetMap", "0");
        }
        else
        {
            ui_currentMap.integer = 0;
            trap_Cvar_Set("ui_currentMap", "0");
        }
        map = 0;
    }

    if (uiInfo.mapList[map].cinematic >= -1)
    {
        if (uiInfo.mapList[map].cinematic == -1)
        {
            uiInfo.mapList[map].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[map].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
        }
        if (uiInfo.mapList[map].cinematic >= 0)
        {
            trap_CIN_RunCinematic(uiInfo.mapList[map].cinematic);
            trap_CIN_SetExtents(uiInfo.mapList[map].cinematic, rect->x, rect->y, rect->w, rect->h);
            trap_CIN_DrawCinematic(uiInfo.mapList[map].cinematic);
        }
        else
        {
            uiInfo.mapList[map].cinematic = -2;
        }
    }
    else
    {
        UI_DrawMapPreview(rect, scale, color, net);
    }
}



static qboolean updateModel = qtrue;
static qboolean q3Model = qfalse;
void UI_DrawHead( float x, float y, float w, float h, playerInfo_t *pi, int time );
extern vmCvar_t ui_s_e_mouth; 

void UI_DrawPlayerHead( float x, float y, float w, float h, playerInfo_t *pi, int time, int team );
//qboolean UI_RegisterClientStyleModels( playerInfo_t *pi, const char *mouthName, const char *eyesName, const char *headName );

/*static void UI_DrawPlayerModel2(rectDef_t *rect) {
static playerInfo_t info;
char model[MAX_QPATH];
char team[256];
char head[256];
vec3_t	viewangles;
vec3_t	moveangles;
 
strcpy(model, UI_Cvar_VariableString("model"));
strcpy(head, UI_Cvar_VariableString("headmodel"));
q3Model = qtrue;
updateModel = qtrue;
team[0] = '\0';
 
if (updateModel)
{
memset( &info, 0, sizeof(playerInfo_t) );
viewangles[YAW]   = 180 - 10  ;
viewangles[PITCH] = 0;
viewangles[ROLL]  = 0;
VectorClear( moveangles );
 
Com_sprintf(model,sizeof(model),"%s/%s",ui_s_model.string,ui_s_skin.string );
UI_RegisterClientModelname( &info, model, head, team);
 
UI_PlayerInfo_SetModel( &info, model , "fixme" , team);
 
//
 
UI_PlayerInfo_SetInfo( &info, LEGS_IDLE, TORSO_STAND_RIFLE, viewangles, vec3_origin, WP_M4, qfalse );
updateModel = qfalse;
 
UI_RegisterClientStyleModels( &info, ui_s_e_mouth.string,"goggles",ui_s_e_head.string );
 
}
 
UI_DrawPlayerHead( rect->x, rect->y, rect->w, rect->h, &info, uiInfo.uiDC.realTime / 2);
 
}
*/

qboolean UI_RegisterClientStyleModels( playerInfo_t *pi, const char *mouthName, const char *eyesName, const char *headName );
static void UI_DrawPlayerModelForTeam(rectDef_t *rect, int numteam)
{
    static playerInfo_t info;
    char model[MAX_QPATH];
    char team[256];
    char head[256];
    vec3_t	viewangles;
    vec3_t	moveangles;

    strcpy(model, UI_Cvar_VariableString("model"));
    strcpy(head, UI_Cvar_VariableString("headmodel"));
    q3Model = qtrue;
    updateModel = qtrue;
    team[0] = '\0';

    if (updateModel)
    {
        memset( &info, 0, sizeof(playerInfo_t) );
        viewangles[YAW]   = 180 - 10  ;
        viewangles[PITCH] = 0;
        viewangles[ROLL]  = 0;
        VectorClear( moveangles );


        if ( numteam == TEAM_RED )
        {
            Com_sprintf(model,sizeof(model),"%s/%s",ui_s_model.string,ui_s_skin.string );
            UI_RegisterClientModelname( &info, model, head, team);

            //
            UI_PlayerInfo_SetInfo( &info, LEGS_IDLE, TORSO_STAND_RIFLE, viewangles, vec3_origin, WP_M4, qfalse );
            UI_PlayerInfo_SetModel( &info, model , "fixme" , team);

            UI_RegisterClientModelname( &info, model, head, team);

            UI_RegisterClientStyleModels( &info, ui_s_e_mouth.string,ui_s_e_eyes.string,ui_s_e_head.string );
        }
        else if ( numteam == TEAM_BLUE )
        {
            Com_sprintf(model,sizeof(model),"%s/%s",ui_t_model.string,ui_t_skin.string );
            UI_RegisterClientModelname( &info, model, head, team);

            UI_PlayerInfo_SetInfo( &info, LEGS_IDLE, TORSO_STAND_RIFLE, viewangles, vec3_origin, WP_M4, qfalse );
            UI_PlayerInfo_SetModel( &info, model , "fixme" , team);

            //
            UI_RegisterClientStyleModels( &info, ui_t_e_mouth.string,ui_t_e_eyes.string,ui_t_e_head.string );
        }
        else
            return;


        updateModel = qfalse;
    }

    UI_DrawPlayer( rect->x, rect->y, rect->w, rect->h, &info, uiInfo.uiDC.realTime / 2);

}

void UI_DrawPlayerHead( float x, float y, float w, float h, playerInfo_t *pi, int time, int team );
static void UI_DrawHeadModel(rectDef_t *rect, int numteam )
{
    static playerInfo_t info;
    char model[MAX_QPATH];
    char team[256];
    char head[256];
    vec3_t	viewangles;
    vec3_t	moveangles;

    strcpy(model, UI_Cvar_VariableString("model"));
    strcpy(head, UI_Cvar_VariableString("headmodel"));
    q3Model = qtrue;
    updateModel = qtrue;
    team[0] = '\0';

#if 1

    if (updateModel)
    {
        memset( &info, 0, sizeof(playerInfo_t) );
        viewangles[YAW]   = 180 - 10  ;
        viewangles[PITCH] = 0;
        viewangles[ROLL]  = 0;
        VectorClear( moveangles );

        if ( numteam == TEAM_RED )
        {
            Com_sprintf(model,sizeof(model),"%s/%s",ui_s_model.string,ui_s_skin.string );
            //UI_RegisterClientModelname( &info, model, head, team);

            UI_PlayerInfo_SetModel( &info, model , head , team);

            //
            UI_PlayerInfo_SetInfo( &info, LEGS_IDLE, TORSO_STAND_RIFLE, viewangles, vec3_origin, WP_M4, qfalse );
            updateModel = qfalse;

            UI_RegisterClientStyleModels( &info, ui_s_e_mouth.string,ui_s_e_eyes.string,ui_s_e_head.string );
        }
        else if ( numteam == TEAM_BLUE )
        {
            Com_sprintf(model,sizeof(model),"%s/%s",ui_t_model.string,ui_t_skin.string );
            UI_RegisterClientModelname( &info, model, head, team);

            UI_PlayerInfo_SetModel( &info, model , "fixme" , team);

            //
            UI_PlayerInfo_SetInfo( &info, LEGS_IDLE, TORSO_STAND_RIFLE, viewangles, vec3_origin, WP_M4, qfalse );
            updateModel = qfalse;

            UI_RegisterClientStyleModels( &info, ui_t_e_mouth.string,ui_t_e_eyes.string,ui_t_e_head.string );
        }
    }

    UI_DrawPlayerHead( rect->x, rect->y, rect->w, rect->h, &info, uiInfo.uiDC.realTime / 2, numteam);
#endif
}

static void UI_DrawNetSource(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    if (ui_netSource.integer < 0 || ui_netSource.integer > numNetSources)
    {
        ui_netSource.integer = 0;
    }
    Text_Paint(rect->x, rect->y, scale, color, va("Source: %s", netSources[ui_netSource.integer]), 0, 0, textStyle);
}

static void UI_DrawNetMapPreview(rectDef_t *rect, float scale, vec4_t color)
{

    if (uiInfo.serverStatus.currentServerPreview > 0)
    {
        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.serverStatus.currentServerPreview);
    }
    else
    {
        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("menu/art/unknownmap"));
    }
}

static void UI_DrawNetMapCinematic(rectDef_t *rect, float scale, vec4_t color)
{
    if (ui_currentNetMap.integer < 0 || ui_currentNetMap.integer > uiInfo.mapCount)
    {
        ui_currentNetMap.integer = 0;
        trap_Cvar_Set("ui_currentNetMap", "0");
    }

    if (uiInfo.serverStatus.currentServerCinematic >= 0)
    {
        trap_CIN_RunCinematic(uiInfo.serverStatus.currentServerCinematic);
        trap_CIN_SetExtents(uiInfo.serverStatus.currentServerCinematic, rect->x, rect->y, rect->w, rect->h);
        trap_CIN_DrawCinematic(uiInfo.serverStatus.currentServerCinematic);
    }
    else
    {
        UI_DrawNetMapPreview(rect, scale, color);
    }
}



static void UI_DrawNetFilter(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters)
    {
        ui_serverFilterType.integer = 0;
    }
    Text_Paint(rect->x, rect->y, scale, color, va("Filter: %s", serverFilters[ui_serverFilterType.integer].description), 0, 0, textStyle);
}

 
static const char *UI_EnglishMapName(const char *map)
{
    int i;
    for (i = 0; i < uiInfo.mapCount; i++)
    {
        if (Q_stricmp(map, uiInfo.mapList[i].mapLoadName) == 0)
        {
            return uiInfo.mapList[i].mapName;
        }
    }
    return "";
}
 
static qboolean updateOpponentModel = qtrue; 

static void UI_DrawAllMapsSelection(rectDef_t *rect, float scale, vec4_t color, int textStyle, qboolean net)
{
    int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
    if (map >= 0 && map < uiInfo.mapCount)
    {
        Text_Paint(rect->x, rect->y, scale, color, uiInfo.mapList[map].mapName, 0, 0, textStyle);
    }
}

static void UI_DrawOpponentName(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    Text_Paint(rect->x, rect->y, scale, color, UI_Cvar_VariableString("ui_opponentName"), 0, 0, textStyle);
}


static int UI_OwnerDrawWidth(int ownerDraw, float scale)
{
    int i, h, value;
    const char *text;
    const char *s = NULL;

    switch (ownerDraw)
    {
    case UI_HANDICAP:
        h = Com_Clamp( 5, 100, trap_Cvar_VariableValue("handicap") );
        i = 20 - h / 5;
        s = handicapValues[i];
        break;
    case UI_CLANNAME:
        s = UI_Cvar_VariableString("ui_teamName");
        break;
    case UI_GAMETYPE:
        s = uiInfo.gameTypes[ui_gameType.integer].gameType;
        break;
    case UI_SKILL: 
        break;
    case UI_BLUETEAMNAME:
        i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_blueTeam"));
        if (i >= 0 && i < uiInfo.teamCount)
        {
            s = va("%s: %s", "Tangos", uiInfo.teamList[i].teamName);
        }
        break;
    case UI_REDTEAMNAME:
        i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_redTeam"));
        if (i >= 0 && i < uiInfo.teamCount)
        {
            s = va("%s: %s", "Seals", uiInfo.teamList[i].teamName);
        }
        break;
    case UI_BLUETEAM1:
    case UI_BLUETEAM2:
    case UI_BLUETEAM3:
    case UI_BLUETEAM4:
    case UI_BLUETEAM5:
    case UI_BLUETEAM6:
        value = trap_Cvar_VariableValue(va("ui_blueteam%i", ownerDraw-UI_BLUETEAM1 + 1));
        if (value <= 0)
        {
            text = "Closed";
        }
        else if (value == 1)
        {
            text = "Human";
        }
        else
        {
            value -= 2;
            if (value >= uiInfo.aliasCount)
            {
                value = 0;
            }
            text = uiInfo.aliasList[value].name;
        }
        s = va("%i. %s", ownerDraw-UI_BLUETEAM1 + 1, text);
        break;
    case UI_REDTEAM1:
    case UI_REDTEAM2:
    case UI_REDTEAM3:
    case UI_REDTEAM4:
    case UI_REDTEAM5:
    case UI_REDTEAM6:
        value = trap_Cvar_VariableValue(va("ui_redteam%i", ownerDraw-UI_REDTEAM1 + 1));
        if (value <= 0)
        {
            text = "Closed";
        }
        else if (value == 1)
        {
            text = "Human";
        }
        else
        {
            value -= 2;
            if (value >= uiInfo.aliasCount)
            {
                value = 0;
            }
            text = uiInfo.aliasList[value].name;
        }
        s = va("%i. %s", ownerDraw-UI_REDTEAM1 + 1, text);
        break;
    case UI_NETSOURCE:
        if (ui_netSource.integer < 0 || ui_netSource.integer > numNetSources)
        {
            ui_netSource.integer = 0;
        }
        s = va("Source: %s", netSources[ui_netSource.integer]);
        break;
    case UI_NETFILTER:
        if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters)
        {
            ui_serverFilterType.integer = 0;
        }
        s = va("Filter: %s", serverFilters[ui_serverFilterType.integer].description );
        break;
    case UI_TIER:
        break;
    case UI_TIER_MAPNAME:
        break;
    case UI_TIER_GAMETYPE:
        break;
    case UI_ALLMAPS_SELECTION:
        break;
    case UI_OPPONENT_NAME:
        break;
    case UI_KEYBINDSTATUS:
        if (Display_KeyBindPending())
        {
            s = "Waiting for new key... Press ESCAPE to cancel";
        }
        else
        {
            s = "Press ENTER or CLICK to change, Press BACKSPACE to clear";
        }
        break;
    case UI_SERVERREFRESHDATE:
        s = UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer));
        break;
    default:
        break;
    }

    if (s)
    {
        return Text_Width(s, scale, 0);
    }
    return 0;
}

static void UI_DrawRedBlue(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    Text_Paint(rect->x, rect->y, scale, color, (uiInfo.redBlue == 0) ? "Seals" : "Tangos", 0, 0, textStyle);
}


/*
===============
UI_BuildPlayerList
===============
*/
static void UI_BuildPlayerList()
{
    uiClientState_t	cs;
    int		n, count, team, team2, playerTeamNumber;
    char	info[MAX_INFO_STRING];

    trap_GetClientState( &cs );
    trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
    uiInfo.playerNumber = cs.clientNum;
    uiInfo.teamLeader = atoi(Info_ValueForKey(info, "tl"));
    team = atoi(Info_ValueForKey(info, "t"));
    trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
    count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
    uiInfo.playerCount = 0;
    uiInfo.myTeamCount = 0;
    playerTeamNumber = 0;
    for( n = 0; n < count; n++ )
    {
        trap_GetConfigString( CS_PLAYERS + n, info, MAX_INFO_STRING );

        if (info[0])
        {
            Q_strncpyz( uiInfo.playerNames[uiInfo.playerCount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
            Q_CleanStr( uiInfo.playerNames[uiInfo.playerCount] );
            uiInfo.playerCount++;
            team2 = atoi(Info_ValueForKey(info, "t"));
            if (team2 == team)
            {
                Q_strncpyz( uiInfo.teamNames[uiInfo.myTeamCount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
                Q_CleanStr( uiInfo.teamNames[uiInfo.myTeamCount] );
                uiInfo.teamClientNums[uiInfo.myTeamCount] = n;
                if (uiInfo.playerNumber == n)
                {
                    playerTeamNumber = uiInfo.myTeamCount;
                }
                uiInfo.myTeamCount++;
            }
        }
    }

    if (!uiInfo.teamLeader)
    {
        trap_Cvar_Set("cg_selectedPlayer", va("%d", playerTeamNumber));
    }

    n = trap_Cvar_VariableValue("cg_selectedPlayer");
    if (n < 0 || n > uiInfo.myTeamCount)
    {
        n = 0;
    }
    if (n < uiInfo.myTeamCount)
    {
        trap_Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[n]);
    }
}


static void UI_DrawSelectedPlayer(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    if (uiInfo.uiDC.realTime > uiInfo.playerRefresh)
    {
        uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
        UI_BuildPlayerList();
    }
    Text_Paint(rect->x, rect->y, scale, color, (uiInfo.teamLeader) ? UI_Cvar_VariableString("cg_selectedPlayerName") : UI_Cvar_VariableString("name") , 0, 0, textStyle);
}

static void UI_DrawServerRefreshDate(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    if (uiInfo.serverStatus.refreshActive)
    {
        vec4_t lowLight, newColor;
        lowLight[0] = 0.8 * color[0];
        lowLight[1] = 0.8 * color[1];
        lowLight[2] = 0.8 * color[2];
        lowLight[3] = 0.8 * color[3];
        LerpColor(color,lowLight,newColor,0.5+0.5*sin(uiInfo.uiDC.realTime / PULSE_DIVISOR));
        Text_Paint(rect->x, rect->y, scale, newColor, va("Getting info for %d servers (ESC to cancel)", trap_LAN_GetServerCount(ui_netSource.integer)), 0, 0, textStyle);
    }
    else
    {
        char buff[64];
        Q_strncpyz(buff, UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer)), 64);
        Text_Paint(rect->x, rect->y, scale, color, va("Refresh Time: %s", buff), 0, 0, textStyle);
    }
}

static void UI_DrawServerMOTD(rectDef_t *rect, float scale, vec4_t color)
{
    if (uiInfo.serverStatus.motdLen)
    {
        float maxX;

        if (uiInfo.serverStatus.motdWidth == -1)
        {
            uiInfo.serverStatus.motdWidth = 0;
            uiInfo.serverStatus.motdPaintX = rect->x + 1;
            uiInfo.serverStatus.motdPaintX2 = -1;
        }

        if (uiInfo.serverStatus.motdOffset > uiInfo.serverStatus.motdLen)
        {
            uiInfo.serverStatus.motdOffset = 0;
            uiInfo.serverStatus.motdPaintX = rect->x + 1;
            uiInfo.serverStatus.motdPaintX2 = -1;
        }

        if (uiInfo.uiDC.realTime > uiInfo.serverStatus.motdTime)
        {
            uiInfo.serverStatus.motdTime = uiInfo.uiDC.realTime + 10;
            if (uiInfo.serverStatus.motdPaintX <= rect->x + 2)
            {
                if (uiInfo.serverStatus.motdOffset < uiInfo.serverStatus.motdLen)
                {
                    uiInfo.serverStatus.motdPaintX += Text_Width(&uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], scale, 1) - 1;
                    uiInfo.serverStatus.motdOffset++;
                }
                else
                {
                    uiInfo.serverStatus.motdOffset = 0;
                    if (uiInfo.serverStatus.motdPaintX2 >= 0)
                    {
                        uiInfo.serverStatus.motdPaintX = uiInfo.serverStatus.motdPaintX2;
                    }
                    else
                    {
                        uiInfo.serverStatus.motdPaintX = rect->x + rect->w - 2;
                    }
                    uiInfo.serverStatus.motdPaintX2 = -1;
                }
            }
            else
            {
                //serverStatus.motdPaintX--;
                uiInfo.serverStatus.motdPaintX -= 2;
                if (uiInfo.serverStatus.motdPaintX2 >= 0)
                {
                    //serverStatus.motdPaintX2--;
                    uiInfo.serverStatus.motdPaintX2 -= 2;
                }
            }
        }

        maxX = rect->x + rect->w - 2;
        Text_Paint_Limit(&maxX, uiInfo.serverStatus.motdPaintX, rect->y + rect->h - 3, scale, color, &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], 0, 0);
        if (uiInfo.serverStatus.motdPaintX2 >= 0)
        {
            float maxX2 = rect->x + rect->w - 2;
            Text_Paint_Limit(&maxX2, uiInfo.serverStatus.motdPaintX2, rect->y + rect->h - 3, scale, color, uiInfo.serverStatus.motd, 0, uiInfo.serverStatus.motdOffset);
        }
        if (uiInfo.serverStatus.motdOffset && maxX > 0)
        {
            // if we have an offset ( we are skipping the first part of the string ) and we fit the string
            if (uiInfo.serverStatus.motdPaintX2 == -1)
            {
                uiInfo.serverStatus.motdPaintX2 = rect->x + rect->w - 2;
            }
        }
        else
        {
            uiInfo.serverStatus.motdPaintX2 = -1;
        }

    }
}

static void UI_DrawKeyBindStatus(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    //	int ofs = 0;
    if (Display_KeyBindPending())
    {
        Text_Paint(rect->x, rect->y, scale, color, "Choose a key for this command... Press ESCAPE to cancel", 0, 0, textStyle);
    }
    else
    {
        Text_Paint(rect->x, rect->y, scale, color, "Press ENTER or CLICK to change, Press BACKSPACE to clear", 0, 0, textStyle);
    }
}

static void UI_DrawGLInfo(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    char * eptr;
    char buff[4096];
    const char *lines[64];
    int y, numLines, i;

    Text_Paint(rect->x + 2, rect->y, scale, color, va("VENDOR: %s", uiInfo.uiDC.glconfig.vendor_string), 0, 30, textStyle);
    Text_Paint(rect->x + 2, rect->y + 15, scale, color, va("VERSION: %s: %s", uiInfo.uiDC.glconfig.version_string,uiInfo.uiDC.glconfig.renderer_string), 0, 30, textStyle);
    Text_Paint(rect->x + 2, rect->y + 30, scale, color, va ("PIXELFORMAT: color(%d-bits) Z(%d-bits) stencil(%d-bits)", uiInfo.uiDC.glconfig.colorBits, uiInfo.uiDC.glconfig.depthBits, uiInfo.uiDC.glconfig.stencilBits), 0, 30, textStyle);

    // build null terminated extension strings
    Q_strncpyz(buff, uiInfo.uiDC.glconfig.extensions_string, 4096);
    eptr = buff;
    y = rect->y + 45;
    numLines = 0;
    while ( y < rect->y + rect->h && *eptr )
    {
        while ( *eptr && *eptr == ' ' )
            *eptr++ = '\0';

        // track start of valid string
        if (*eptr && *eptr != ' ')
        {
            lines[numLines++] = eptr;
        }

        while ( *eptr && *eptr != ' ' )
            eptr++;
    }

    i = 0;
    while (i < numLines)
    {
        Text_Paint(rect->x + 2, y, scale, color, lines[i++], 0, 20, textStyle);
        if (i < numLines)
        {
            Text_Paint(rect->x + rect->w / 2, y, scale, color, lines[i++], 0, 20, textStyle);
        }
        y += 10;
        if (y > rect->y + rect->h - 11)
        {
            break;
        }
    }


}

static qboolean UI_CheckWeapon ( int weapon )
{
    int acc = trap_Cvar_VariableValue("ui_char_accuracy");
    int str = trap_Cvar_VariableValue("ui_char_strength");
    int stl = trap_Cvar_VariableValue("ui_char_stealth");
    int sta = trap_Cvar_VariableValue("ui_char_stamina");
    int tech = trap_Cvar_VariableValue("ui_char_technical");

    // to avoid compiler warnings
    stl = stl;
    sta = sta;
    tech = tech;

    if ( trap_Cvar_VariableValue("g_gametype") < GT_TEAM )
        return qtrue;

    // min char requirements for the weapons. and see if our char meets them
    switch ( weapon )
    {
    case WP_M14:
        if ( str > 5 )
            return qtrue;
        break;
    case WP_MP5:
    case WP_M4:
    case WP_MK23:
    case WP_P9S:
    case WP_870:
    case WP_MAC10:
    case WP_AK47:
    case WP_GLOCK:
    case WP_SW40T:
    case WP_M590:
        return qtrue;
        break;
    case WP_SW629:
    case WP_DEAGLE:
        if ( str > 3 )
            return qtrue;
        break;
    case WP_PDW:
        if ( str > 3 && acc > 4 )
            return qtrue;
        break;
    case WP_PSG1:
        if ( acc > 6 )
            return qtrue;
        break;
    case WP_SL8SD:
        if ( acc > 5 && stl > 5 )
            return qtrue;
        break;
    case WP_MACMILLAN:
        if ( acc > 8 && str > 4 )
            return qtrue;
        break;
    case WP_SPAS15:
        if ( str > 4 )
            return qtrue;
        break;
    case WP_M249:
        if ( str >= 4 && sta >= 5)
            return qtrue;
    }

    return qfalse;
}

static qboolean CG_HandleWeaponPrimaryAddon1( int flags, float *special, int key );
static qboolean CG_HandleWeaponPrimaryAddon2( int flags, float *special, int key );
static qboolean CG_HandleWeaponPrimaryAddon3( int flags, float *special, int key );
static qboolean CG_HandleWeaponSecondaryAddon1( int flags, float *special, int key );
static qboolean CG_HandleWeaponSecondaryAddon2( int flags, float *special, int key );

static void CG_DrawWeaponPrimaryAddon1( rectDef_t *rect, int special, float scale, vec4_t color, int textStyle )
{
    int			primary;
    int			wMods;
    int			silencer,duckbill;
    char		var[64];
    int		xp = 0;


    trap_Cvar_VariableStringBuffer("inven_duckbill", var , sizeof(var ) );
    duckbill = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_silencer", var , sizeof(var ) );
    silencer = atoi(var);

    // ---

    trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
    primary = atoi(var);

    wMods = BG_WeaponMods(primary);

    if ( silencer )
    {
        char *mode = "Suppressor";

        if ( primary == WP_M4 || primary == WP_AK47 )
            mode = "Muzzle Hider";
        xp = trap_Cvar_VariableValue("ui_char_stealth");


        if ( !(wMods & ( 1 << WM_SILENCER ) ) )
        {
            CG_HandleWeaponPrimaryAddon1( 0,0,0 );
        }
        else if ( xp < 5 )
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, va("%s (stl>=5)",mode) , 0, 30, textStyle);
        }
        else
            Text_Paint(rect->x, rect->y, scale, color, va("%s",mode), 0, 30, textStyle);
    }
    else if ( duckbill )
    {
        xp = trap_Cvar_VariableValue("ui_char_strength");

        if ( !(wMods & ( 1 << WM_DUCKBILL ) )  )
        {
            CG_HandleWeaponPrimaryAddon1( 0,0,0 );
        }
        else if ( xp < 5 )
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Duckbill (str>=5)" , 0, 30, textStyle);
        }
        else
            Text_Paint(rect->x, rect->y, scale, color, "Duckbill", 0, 30, textStyle);
    }
    else
    {
        Text_Paint(rect->x, rect->y, scale, color, "No Muzzle add-on", 0, 30, textStyle);
    }
}

static void CG_DrawWeaponPrimaryAddon2( rectDef_t *rect, int special, float scale, vec4_t color, int textStyle )
{
    int			primary;
    int			wMods;
    int			bayonet,gl,flashlight;
    char		var[64];
    int			xp;

    trap_Cvar_VariableStringBuffer("inven_grenadelauncher", var , sizeof(var ) );
    gl = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_bayonet", var , sizeof(var ) );
    bayonet = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_flashlight", var , sizeof(var ) );
    flashlight = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
    primary = atoi(var);

    wMods = BG_WeaponMods(primary);

    if ( gl )
    {
        int xp2 = trap_Cvar_VariableValue("ui_char_strength");
        xp = trap_Cvar_VariableValue("ui_char_technical");

        if ( !(wMods & ( 1 << WM_GRENADELAUNCHER ) ) )
        {
            CG_HandleWeaponPrimaryAddon2( 0,0,0 );
        }
        else if ( xp < 5 || xp2 < 3 )
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "GL (tec>=5 str>=3)" , 0, 30, textStyle);

        }
        else
            Text_Paint(rect->x, rect->y, scale, color, "Grenadelauncher", 0, 30, textStyle);
    }
    else if ( bayonet )
    {
        xp = trap_Cvar_VariableValue("ui_char_strength");

        if ( !(wMods & ( 1 << WM_BAYONET ) ) )
            CG_HandleWeaponPrimaryAddon2( 0,0,0 );
        else if ( xp < 4 )
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Bayonet (str>=4)" , 0, 30, textStyle);
        else
            Text_Paint(rect->x, rect->y, scale, color, "Bayonet", 0, 30, textStyle);
    }
    else if ( flashlight )
    {
        xp = trap_Cvar_VariableValue("ui_char_technical");

        if (!(wMods & ( 1 << WM_FLASHLIGHT ) ) )
            CG_HandleWeaponPrimaryAddon2( 0,0,0 );
        else if ( xp < 4 )
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Flashlight (tec>=4)" , 0, 30, textStyle);
        else
            Text_Paint(rect->x, rect->y, scale, color, "Flashlight", 0, 30, textStyle);
    }
    else
    {
        Text_Paint(rect->x, rect->y, scale, color, "No Underslung add-on", 0, 30, textStyle);
    }
}

static void CG_DrawWeaponPrimaryAddon3( rectDef_t *rect, int special, float scale, vec4_t color, int textStyle )
{
    int			primary;
    int			wMods;
    int			laser,scope,nvgscope;
    char		var[64];
    int			xp;

    trap_Cvar_VariableStringBuffer("inven_lasersight", var , sizeof(var ) );
    laser = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_scope", var , sizeof(var ) );
    scope = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_nvgscope", var , sizeof(var ) );
    nvgscope = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
    primary = atoi(var);

    wMods = BG_WeaponMods(primary);

    if ( laser )
    {
        xp = trap_Cvar_VariableValue("ui_char_accuracy");

        if ( !(wMods & ( 1 << WM_LASER ) ) )
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Lasersight" , 0, 30, textStyle);
            CG_HandleWeaponPrimaryAddon3( 0,0,0 );
        }
        else if ( xp < 4 )
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Lasersight (acc>=4)" , 0, 30, textStyle);
        }
        else
            Text_Paint(rect->x, rect->y, scale, color, "Lasersight", 0, 30, textStyle);
    }
    else if ( scope )
    {
        xp = trap_Cvar_VariableValue("ui_char_accuracy");

        if ( !(wMods & ( 1 << WM_SCOPE ) ) )
        {
            CG_HandleWeaponPrimaryAddon3( 0,0,0 );
        }
        else if ( xp < 6 )
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Scope (acc>=6)" , 0, 30, textStyle);
        }
        else
            Text_Paint(rect->x, rect->y, scale, color, "Scope", 0, 30, textStyle);
    }
    else if ( nvgscope )
    {
        xp = trap_Cvar_VariableValue("ui_char_stealth");

#if 1 // fixme: add flashlight

        if ( 1 )//!(wMods & ( 1 << WM_NVGSCOPE ) )
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "NV Scope" , 0, 30, textStyle);
            CG_HandleWeaponPrimaryAddon3( 0,0,0 );
        }
        else
            Text_Paint(rect->x, rect->y, scale, color, "NV Scope", 0, 30, textStyle);
#endif

    }
    else
    {
        Text_Paint(rect->x, rect->y, scale, color, "No on weapon add-on", 0, 30, textStyle);
    }
}

static void CG_DrawWeaponSecondaryAddon1( rectDef_t *rect, int special, float scale, vec4_t color, int textStyle )
{
    int			primary;
    int			wMods;
    int			silencer ;
    char		var[64];
    int			xp;


    trap_Cvar_VariableStringBuffer("inven_silencer_secondary", var , sizeof(var ) );
    silencer = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof(var ) );
    primary = atoi(var);

    wMods = BG_WeaponMods(primary);

    if ( silencer )
    {
        xp = trap_Cvar_VariableValue("ui_char_stealth");

        if ( !(wMods & ( 1 << WM_SILENCER ) ) ) // skip equipment
            CG_HandleWeaponSecondaryAddon1( 0,0,0 );
        else if ( xp < 3 ) // no enough xp. keep equipment but tell user.
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Suppressor (stl>=3)" , 0, 30, textStyle);
        else
            Text_Paint(rect->x, rect->y, scale, color, "Suppressor", 0, 30, textStyle);
    }
    else
    {
        Text_Paint(rect->x, rect->y, scale, color, "No Muzzle add-on", 0, 30, textStyle);
    }
}

static void CG_DrawWeaponSecondaryAddon2( rectDef_t *rect, int special, float scale, vec4_t color, int textStyle )
{
    int			primary;
    int			wMods;
    int			laser ;
    char		var[64];
    int			xp;

    trap_Cvar_VariableStringBuffer("inven_lasersight_secondary", var , sizeof(var ) );
    laser = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof(var ) );
    primary = atoi(var);

    wMods = BG_WeaponMods(primary);

    if ( laser )
    {
        xp = trap_Cvar_VariableValue("ui_char_accuracy");

        if ( !(wMods & ( 1 << WM_LASER ) ) )
            CG_HandleWeaponSecondaryAddon2( 0,0,0 );
        else if ( xp < 3 )
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, "Lasersight (acc>=3)" , 0, 30, textStyle);
        else
            Text_Paint(rect->x, rect->y, scale, color, "Lasersight", 0, 30, textStyle);
    }
    else
    {
        Text_Paint(rect->x, rect->y, scale, color, "No on weapon add-on", 0, 30, textStyle);
    }
}


static void CG_DrawWeaponText( rectDef_t *rect, int special, float scale, vec4_t color, int textStyle )
{
    //	qhandle_t	icon = 0;
    int			wNum = WP_NONE;

    if ( special )
        wNum = special;

    if ( wNum )
    {
        gitem_t *it = BG_FindItemForWeapon( wNum );

        if (!UI_CheckWeapon(wNum))
        {
            Text_Paint(rect->x, rect->y, scale, colorMdGrey, va("%s", it->pickup_name ), 0, 30, textStyle);
        }
        else
            Text_Paint(rect->x, rect->y, scale, color, va("%s", it->pickup_name ), 0, 30, textStyle);

    }
}

/*
=================
NSQ3 Calculate Weight
author: Defcon-X
date:
description: calculates players current weight
=================
*/
static float UI_CalcWeight ( void )
{
    float weight = 0;
    float weapon_weight = 0;

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

    char		var[MAX_TOKEN_CHARS];


    //
    // equipment
    //
    trap_Cvar_VariableStringBuffer("inven_helmet", var , sizeof(var ) );
    helmet = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_kevlar", var , sizeof(var ) );
    kevlar = atoi(var);

    //
    // grenades
    //
    trap_Cvar_VariableStringBuffer("inven_ammo_flash", var , sizeof(var ) );
    fl_grenades = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_ammo_smoke", var , sizeof(var ) );
    sm_grenades = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_ammo_mk26", var , sizeof(var ) );
    grenades = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_ammo_40mmgren", var , sizeof(var ) );
    mmgrenades = atoi(var);

    //
    // primary weapon
    //
    trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
    primary = atoi(var);

    //
    // secondary weapon
    //
    trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof(var ) );
    secondary = atoi(var);

    //
    // ammo
    //
    trap_Cvar_VariableStringBuffer("inven_ammo_primary", var , sizeof(var ) );
    priammo = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_ammo_secondary", var , sizeof(var ) );
    secammo = atoi(var);

    //
    // Weapons
    //

    switch (primary)
    {
    case WP_AK47:
        weapon_weight = 19;
        break;
    case WP_M4:
        weapon_weight = 20;
        break;
    case WP_MP5:
        weapon_weight = 15;
        break;
    case WP_PDW:
        weapon_weight = 20;
        break;
    case WP_M14:
        weapon_weight = 32;
        break;
    case WP_SPAS15:
        weapon_weight = 24;
        break;
    case WP_870:
        weapon_weight = 20;
        break;
    case WP_M249:
        weapon_weight = 21;
        break;
    case WP_M590:
        weapon_weight = 16;
        break;
    case WP_SL8SD:
        weapon_weight = 17;
        break;
    case WP_PSG1:
        weapon_weight = 18;
        break;
    case WP_MACMILLAN:
        weapon_weight = 15;
        break;
    default:
        weapon_weight = 15;
        break;
    }

    weight += weapon_weight;

    switch (secondary)
    {
    case WP_SW629:
    case WP_DEAGLE:
        weapon_weight = 9;
        break;
    case WP_SW40T:
        weapon_weight = 7;
        break;
    case WP_MK23:
        weapon_weight = 6;
        break;
    case WP_GLOCK:
        weapon_weight = 6;
        break;
    case WP_P9S:
        weapon_weight = 5;
        break;
    default:
        weapon_weight = 6;
        break;
    }

    weight += weapon_weight;

    weight += (priammo+secammo)/2;


    //
    // Kevlar Vest
    //

    if (kevlar)
        weight += 6.5;

    //
    // Helmet
    //

    if (helmet)
        weight += 6;


    if ( weight > 200)
        weight = 200;

    return weight;
}

static void UI_DrawWeight(rectDef_t *rect, float scale, vec4_t color, int textStyle)
{
    int weight = 0;

    weight = UI_CalcWeight();

    Text_Paint(rect->x, rect->y, scale, color, va("Weight: %ikg",weight), 0, 0, textStyle);
}

static void CG_DrawWeaponIcon( rectDef_t *rect, int special )
{
    qhandle_t	icon = 0;
    qboolean	weapon = qtrue;
    qboolean	item = qfalse;
    int			wNum = WP_NONE;

    if ( special )
    {

        wNum = special;
    }

    if (!weapon&&!item)
        return;

    if ( weapon )
    {
        gitem_t *it =		BG_FindItemForWeapon( wNum);


        icon = trap_R_RegisterShaderNoMip( it->icon );
    }

    if ( icon )
    {

        UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, icon );

        if (!UI_CheckWeapon(wNum))
        {
            float color[4];

            color[0] = colorRed[0];
            color[1] = colorRed[1];
            color[2] = colorRed[2];
            color[3] = colorRed[3] - 0.5;
            UI_FillRect( rect->x, rect->y, rect->w, rect->h, color );
        }

    }
}

static void CG_DrawCharacterButton( rectDef_t *rect, int special )
{
    //	qhandle_t	icon = 0;
    int		xp = 0;
    int		value;

#if DEBUGCHARACTER

    Com_Printf("DrawCharacterButton\n");
#endif

    if ( special == PC_STRENGTH )
        value = trap_Cvar_VariableValue("ui_char_strength");
    else if ( special == PC_TECHNICAL )
        value = trap_Cvar_VariableValue("ui_char_technical");
    else if ( special == PC_STAMINA )
        value = trap_Cvar_VariableValue("ui_char_stamina");
    else if ( special == PC_ACCURACY )
        value = trap_Cvar_VariableValue("ui_char_accuracy");
    else if ( special == PC_SPEED )
        value = trap_Cvar_VariableValue("ui_char_speed");
    else if ( special == PC_STEALTH )
        value = trap_Cvar_VariableValue("ui_char_stealth");
    else
        return;

    // cannot raise level
    if ( xp <= value )
    {
        UI_FillRect( rect->x, rect->y, rect->w, rect->h, colorRed );
    }
    else // possible
    {
        UI_FillRect( rect->x, rect->y, rect->w, rect->h, colorGreen );
    }
}

// FIXME: table drive
//
static void UI_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle)
{
    rectDef_t rect;

    rect.x = x + text_x;
    rect.y = y + text_y;
    rect.w = w;
    rect.h = h;
    switch (ownerDraw)
    {

    case UI_PRIMARY_ADDON1:
        CG_DrawWeaponPrimaryAddon1( &rect, special, scale, color, textStyle );
        break;
    case UI_PRIMARY_ADDON2:
        CG_DrawWeaponPrimaryAddon2( &rect, special, scale, color, textStyle );
        break;
    case UI_PRIMARY_ADDON3:
        CG_DrawWeaponPrimaryAddon3( &rect, special, scale, color, textStyle );
        break;

    case UI_SIDEARM_ADDON1:
        CG_DrawWeaponSecondaryAddon1( &rect, special, scale, color, textStyle );
        break;
    case UI_SIDEARM_ADDON2:
        CG_DrawWeaponSecondaryAddon2( &rect, special, scale, color, textStyle );
        break;
    case CG_CHARACTERBUTTON:
        CG_DrawCharacterButton( &rect, special );
        break;
    case CG_PRIMARY:
    case CG_SECONDARY:
        {
            char		var[MAX_TOKEN_CHARS];

            if (ownerDraw == CG_PRIMARY)
                trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof( var ) );
            else
                trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof( var ) );

            // set the "special to our current weapon"
            special = atoi(var);

            if ( special <= 0 || special >= WP_NUM_WEAPONS)
                return;

            CG_DrawWeaponIcon( &rect, special );
        }
        break;
    case UI_TOTALWEIGHT:
        UI_DrawWeight(&rect, scale, color, textStyle);
        break;
    case UI_TEXT_PRIMARY:
    case UI_TEXT_SIDEARM:
        {
            char		var[MAX_TOKEN_CHARS];

            if (ownerDraw == UI_TEXT_PRIMARY)
                trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof( var ) );
            else
                trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof( var ) );

            // set the "special to our current weapon"
            special = atoi(var);

            if ( special <= 0 || special >= WP_NUM_WEAPONS)
                return;

            CG_DrawWeaponText( &rect, special, scale, color, textStyle );
        }
        break;
    case CG_PICKUP:
        CG_DrawWeaponIcon( &rect, special );
        break;

    case CG_TEXT_PICKUP:
        CG_DrawWeaponText( &rect, special, scale, color, textStyle );
        break;

    case UI_HANDICAP:
        UI_DrawHandicap(&rect, scale, color, textStyle);
        break;
    case UI_EFFECTS:  
    case UI_PLAYERMODEL: 
        break;
    case UI_TANGO_HEADMODEL:
        UI_DrawHeadModel(&rect, TEAM_BLUE);//, TEAM_BLUE );
        break;
    case UI_SEAL_HEADMODEL:
        UI_DrawHeadModel(&rect, TEAM_RED );
        break;
    case UI_SEAL_PLAYERMODEL: 
        break;
    case UI_TANGO_PLAYERMODEL: 
        break;
    case UI_CLANNAME:
        UI_DrawClanName(&rect, scale, color, textStyle);
        break;
    case UI_CLANLOGO:
        UI_DrawClanLogo(&rect, scale, color);
        break;
    case UI_CLANCINEMATIC:
        UI_DrawClanCinematic(&rect, scale, color);
        break;
    case UI_PREVIEWCINEMATIC:
        UI_DrawPreviewCinematic(&rect, scale, color);
        break;
    case UI_GAMETYPE:
        UI_DrawGameType(&rect, scale, color, textStyle);
        break;
    case UI_NETGAMETYPE:
        UI_DrawNetGameType(&rect, scale, color, textStyle);
        break;
    case UI_JOINGAMETYPE:
        UI_DrawJoinGameType(&rect, scale, color, textStyle);
        break;
    case UI_MAPPREVIEW:
        UI_DrawMapPreview(&rect, scale, color, qtrue);
        break;
    case UI_MAP_TIMETOBEAT:
        UI_DrawMapTimeToBeat(&rect, scale, color, textStyle);
        break;
    case UI_MAPCINEMATIC:
        UI_DrawMapCinematic(&rect, scale, color, qfalse);
        break;
    case UI_STARTMAPCINEMATIC:
        UI_DrawMapCinematic(&rect, scale, color, qtrue);
        break;
    case UI_SKILL: 
        break;
    case UI_BLUETEAMNAME:
        UI_DrawTeamName(&rect, scale, color, qtrue, textStyle);
        break;
    case UI_REDTEAMNAME:
        UI_DrawTeamName(&rect, scale, color, qfalse, textStyle);
        break; 
    case UI_NETSOURCE:
        UI_DrawNetSource(&rect, scale, color, textStyle);
        break;
    case UI_NETMAPPREVIEW:
        UI_DrawNetMapPreview(&rect, scale, color);
        break;
    case UI_NETMAPCINEMATIC:
        UI_DrawNetMapCinematic(&rect, scale, color);
        break;
    case UI_NETFILTER:
        UI_DrawNetFilter(&rect, scale, color, textStyle);
        break; 
    case UI_ALLMAPS_SELECTION:
        UI_DrawAllMapsSelection(&rect, scale, color, textStyle, qtrue);
        break;
    case UI_MAPS_SELECTION:
        UI_DrawAllMapsSelection(&rect, scale, color, textStyle, qfalse);
        break;
    case UI_OPPONENT_NAME:
        UI_DrawOpponentName(&rect, scale, color, textStyle);
        break;
    case UI_BOTNAME:
        break;
    case UI_BOTSKILL:
        break;
    case UI_REDBLUE:
        UI_DrawRedBlue(&rect, scale, color, textStyle);
        break;
    case UI_SELECTEDPLAYER:
        UI_DrawSelectedPlayer(&rect, scale, color, textStyle);
        break;
    case UI_SERVERREFRESHDATE:
        UI_DrawServerRefreshDate(&rect, scale, color, textStyle);
        break;
    case UI_SERVERMOTD:
        UI_DrawServerMOTD(&rect, scale, color);
        break;
    case UI_GLINFO:
        UI_DrawGLInfo(&rect,scale, color, textStyle);
        break;
    case UI_KEYBINDSTATUS:
        UI_DrawKeyBindStatus(&rect,scale, color, textStyle);
        break;
    default:
        break;
    }

}

static qboolean UI_OwnerDrawVisible(int flags)
{
    qboolean vis = qtrue;

    while (flags)
    {

        if (flags & UI_SHOW_FFA)
        {
            if (trap_Cvar_VariableValue("g_gametype") != GT_FFA)
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_FFA;
        }

        if (flags & UI_SHOW_NOTFFA)
        {
            if (trap_Cvar_VariableValue("g_gametype") == GT_FFA)
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_NOTFFA;
        }

        if (flags & UI_SHOW_LEADER)
        {
            // these need to show when this client can give orders to a player or a group
            if (!uiInfo.teamLeader)
            {
                vis = qfalse;
            }
            else
            {
                // if showing yourself
                if (ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber)
                {
                    vis = qfalse;
                }
            }
            flags &= ~UI_SHOW_LEADER;
        }
        if (flags & UI_SHOW_NOTLEADER)
        {
            // these need to show when this client is assigning their own status or they are NOT the leader
            if (uiInfo.teamLeader)
            {
                // if not showing yourself
                if (!(ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber))
                {
                    vis = qfalse;
                }
            }
            flags &= ~UI_SHOW_NOTLEADER;
        }
        if (flags & UI_SHOW_FAVORITESERVERS)
        {
            // this assumes you only put this type of display flag on something showing in the proper context
            if (ui_netSource.integer != AS_FAVORITES)
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_FAVORITESERVERS;
        }
        if (flags & UI_SHOW_NOTFAVORITESERVERS)
        {
            // this assumes you only put this type of display flag on something showing in the proper context
            if (ui_netSource.integer == AS_FAVORITES)
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_NOTFAVORITESERVERS;
        }
        if (flags & UI_SHOW_ANYTEAMGAME)
        {
            if (uiInfo.gameTypes[ui_gameType.integer].gtEnum <= GT_TEAM )
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_ANYTEAMGAME;
        }
        if (flags & UI_SHOW_ANYNONTEAMGAME)
        {
            if (uiInfo.gameTypes[ui_gameType.integer].gtEnum > GT_TEAM )
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_ANYNONTEAMGAME;
        }
        if (flags & UI_SHOW_NETANYTEAMGAME)
        {
            if (uiInfo.gameTypes[ui_netGameType.integer].gtEnum <= GT_TEAM )
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_NETANYTEAMGAME;
        }
        if (flags & UI_SHOW_NETANYNONTEAMGAME)
        {
            if (uiInfo.gameTypes[ui_netGameType.integer].gtEnum > GT_TEAM )
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_NETANYNONTEAMGAME;
        }
        if (flags & UI_SHOW_DEMOAVAILABLE)
        {
            if (!uiInfo.demoAvailable)
            {
                vis = qfalse;
            }
            flags &= ~UI_SHOW_DEMOAVAILABLE;
        }
        else
        {
            flags = 0;
        }
    }
    return vis;
}

static qboolean CG_HandleWeaponPrimaryAddon1( int flags, float *special, int key )
{
    int			silencer,duckbill;
    char		var[64];
    int			primary;

    trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
    primary = atoi(var);


    trap_Cvar_VariableStringBuffer("inven_duckbill", var , sizeof(var ) );
    duckbill = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_silencer", var , sizeof(var ) );
    silencer = atoi(var);


    if ( silencer )
    {
        trap_Cvar_SetValue("inven_silencer", 0);

        if ( BG_WeaponMods( primary ) & ( 1 << WM_DUCKBILL ) )
            trap_Cvar_SetValue("inven_duckbill", 1);
        else
            trap_Cvar_SetValue("inven_duckbill", 0);
    }
    else if ( duckbill )
    {
        trap_Cvar_SetValue("inven_duckbill", 0);
        trap_Cvar_SetValue("inven_silencer", 0);
    }
    else
    {
        trap_Cvar_SetValue("inven_silencer", 0);
        trap_Cvar_SetValue("inven_duckbill", 0);

        if ( BG_WeaponMods( primary ) & ( 1 << WM_SILENCER ) )
            trap_Cvar_SetValue("inven_silencer", 1);
        else if ( BG_WeaponMods( primary ) & ( 1 << WM_DUCKBILL ) )
            trap_Cvar_SetValue("inven_duckbill", 1);
    }
    return qtrue;
}

static qboolean CG_HandleWeaponPrimaryAddon2( int flags, float *special, int key )
{
    int			bayonet,gl,flashlight;
    char		var[64];
    int			primary;

    trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
    primary = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_grenadelauncher", var , sizeof(var ) );
    gl = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_bayonet", var , sizeof(var ) );
    bayonet = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_flashlight", var , sizeof(var ) );
    flashlight = atoi(var);


    if ( gl )
    {
        trap_Cvar_SetValue("inven_grenadelauncher", 0);
        if ( BG_WeaponMods( primary ) & ( 1 << WM_BAYONET ) )
        {
            trap_Cvar_SetValue("inven_bayonet", 1);
            trap_Cvar_SetValue("inven_flashlight", 0);
        }
        else if ( 0 ) // BG_WeaponMods( primary ) & ( 1 << WM_FLASHLIGHT ) )
        {
            trap_Cvar_SetValue("inven_bayonet", 0);
            trap_Cvar_SetValue("inven_flashlight", 1);
        }
        else
        {
            trap_Cvar_SetValue("inven_flashlight", 0);
            trap_Cvar_SetValue("inven_bayonet", 0);
        }
    }
    else if ( bayonet )
    {
        trap_Cvar_SetValue("inven_grenadelauncher", 0);
        trap_Cvar_SetValue("inven_bayonet", 0 );
        if ( BG_WeaponMods( primary ) & ( 1 << WM_FLASHLIGHT ) )
            trap_Cvar_SetValue("inven_flashlight", 1);
        else
            trap_Cvar_SetValue("inven_flashlight", 0);
    }
    else if ( flashlight )
    {
        trap_Cvar_SetValue("inven_grenadelauncher", 0);
        trap_Cvar_SetValue("inven_bayonet", 0);
        trap_Cvar_SetValue("inven_flashlight", 0);
    }
    else
    {
        trap_Cvar_SetValue("inven_grenadelauncher", 0);
        trap_Cvar_SetValue("inven_bayonet", 0);
        trap_Cvar_SetValue("inven_flashlight", 0);

        if ( BG_WeaponMods( primary ) & ( 1 << WM_GRENADELAUNCHER ) )
            trap_Cvar_SetValue("inven_grenadelauncher", 1 );
        else if ( BG_WeaponMods( primary ) & ( 1 << WM_BAYONET ) )
            trap_Cvar_SetValue("inven_bayonet", 1 );
        else if ( BG_WeaponMods( primary ) & ( 1 << WM_FLASHLIGHT ) )
            trap_Cvar_SetValue("inven_flashlight", 1 );
    }
    return qtrue;
}

static qboolean CG_HandleWeaponPrimaryAddon3( int flags, float *special, int key )
{
    int			laser,scope,nvgscope;
    char		var[64];
    int			primary;

    trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
    primary = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_lasersight", var , sizeof(var ) );
    laser = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_scope", var , sizeof(var ) );
    scope = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_nvgscope", var , sizeof(var ) );
    nvgscope = atoi(var);

    if ( laser )
    {
        trap_Cvar_SetValue("inven_lasersight", 0);

        if ( BG_WeaponMods( primary ) & ( 1 << WM_SCOPE ) )
        {
            trap_Cvar_SetValue("inven_scope", 1);
            trap_Cvar_SetValue("inven_nvgscope", 0);
        }
        else  if ( 0 ) // BG_WeaponMods( primary ) & ( 1 << WM_NVGSCOPE ) )
        {
            trap_Cvar_SetValue("inven_scope", 0);
            trap_Cvar_SetValue("inven_nvgscope", 1);
        }
        else
        {
            trap_Cvar_SetValue("inven_scope", 0);
            trap_Cvar_SetValue("inven_nvgscope", 0);
        }
    }
    else if ( scope )
    {
        trap_Cvar_SetValue("inven_lasersight", 0);
        trap_Cvar_SetValue("inven_scope", 0);

        if ( 0 ) // BG_WeaponMods( primary ) & ( 1 << WM_NVGSCOPE ) )
        {
            trap_Cvar_SetValue("inven_scope", 0);
            trap_Cvar_SetValue("inven_nvgscope", 1);
        }
        else
        {
            trap_Cvar_SetValue("inven_nvgscope", 0);
            trap_Cvar_SetValue("inven_scope", 0);
        }

    }
    else if ( nvgscope )
    {
        trap_Cvar_SetValue("inven_lasersight", 0);
        trap_Cvar_SetValue("inven_scope", 0);
        trap_Cvar_SetValue("inven_nvgscope", 0);
    }
    else
    {

        trap_Cvar_SetValue("inven_lasersight", 0);
        trap_Cvar_SetValue("inven_scope", 0);
        trap_Cvar_SetValue("inven_nvgscope", 0);

        if ( BG_WeaponMods( primary ) & ( 1 << WM_SCOPE ) )
            trap_Cvar_SetValue("inven_scope", 1);
        else  if ( 0 ) // BG_WeaponMods( primary ) & ( 1 << WM_NVGSCOPE ) )
            trap_Cvar_SetValue("inven_nvgscope", 1);
        else  if ( BG_WeaponMods( primary ) & ( 1 << WM_LASER ) )
            trap_Cvar_SetValue("inven_lasersight", 1);
    }
    return qtrue;
}



static qboolean CG_HandleWeaponSecondaryAddon1( int flags, float *special, int key )
{
    int			silencer ;
    char		var[64];
    int			secondary;

    trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof(var ) );
    secondary = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_silencer_secondary", var , sizeof(var ) );
    silencer = atoi(var);


    if ( silencer )
    {
        trap_Cvar_SetValue("inven_silencer_secondary", 0);
    }
    else
    {
        trap_Cvar_SetValue("inven_silencer_secondary", 0);

        if (BG_WeaponMods(secondary) & ( 1 << WM_SILENCER ) )
            trap_Cvar_SetValue("inven_silencer_secondary", 1);
    }
    return qtrue;
}
static qboolean CG_HandleWeaponSecondaryAddon2( int flags, float *special, int key )
{
    int			laser ;
    char		var[64];
    int			secondary;

    trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof(var ) );
    secondary = atoi(var);

    trap_Cvar_VariableStringBuffer("inven_lasersight_secondary", var , sizeof(var ) );
    laser = atoi(var);

    if ( laser )
    {
        trap_Cvar_SetValue("inven_lasersight_secondary", 0);
    }
    else
    {
        trap_Cvar_SetValue("inven_lasersight_secondary", 0);

        if (BG_WeaponMods(secondary) & ( 1 << WM_LASER ) )
            trap_Cvar_SetValue("inven_lasersight_secondary", 1);
    }
    return qtrue;
}

#define TIME_FIRSTUPDATE	1000

int CG_GetTotalXPforLevel( int level )
{
    int total=0;
    int i;

    for (i=0;i<level;i++)
    {
        if ( i <= 1 )
            continue;

        total += i;
    }
    return total;
}

void CG_FreeXP( void )
{
    int accuracy, strength, stamina, stealth, technical, speed, xp;
    int i;

    xp = trap_Cvar_VariableValue("ui_gamestate");
    if (xp == STATE_LOCKED)
    {
        trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_no.wav" , qfalse ) , CHAN_LOCAL );
        return;
    }
    accuracy = trap_Cvar_VariableValue("ui_char_accuracy");
    strength = trap_Cvar_VariableValue("ui_char_strength");
    stamina = trap_Cvar_VariableValue("ui_char_stamina");
    stealth = trap_Cvar_VariableValue("ui_char_stealth");
    technical = trap_Cvar_VariableValue("ui_char_technical");
    speed = trap_Cvar_VariableValue("ui_char_speed");
    xp = trap_Cvar_VariableValue("ui_char_xp");

    for (i = accuracy; i>1; i--)
        xp += i;
    for (i = strength; i>1; i--)
        xp += i;
    for (i = stamina; i>1; i--)
        xp += i;
    for (i = stealth; i>1; i--)
        xp += i;
    for (i = technical; i>1; i--)
        xp += i;
    for (i = speed; i>1; i--)
        xp += i;

    trap_Cvar_Set("ui_char_xp", va("%i", xp));
    trap_Cvar_Set("ui_char_accuracy", va("%i", 1));
    trap_Cvar_Set("ui_char_strength", va("%i", 1));
    trap_Cvar_Set("ui_char_stamina", va("%i", 1));
    trap_Cvar_Set("ui_char_stealth", va("%i", 1));
    trap_Cvar_Set("ui_char_technical", va("%i", 1));
    trap_Cvar_Set("ui_char_speed", va("%i", 1));
}

void CG_CopyCharacterVariables( void )
{
    int acc, str, sta, ste, tec, spe, xp;

    acc = trap_Cvar_VariableValue("char_accuracy");
    str = trap_Cvar_VariableValue("char_strength");
    sta = trap_Cvar_VariableValue("char_stamina");
    ste = trap_Cvar_VariableValue("char_stealth");
    tec = trap_Cvar_VariableValue("char_technical");
    spe = trap_Cvar_VariableValue("char_speed");
    xp = trap_Cvar_VariableValue("char_xp");
    trap_Cvar_Set("ui_char_accuracy", va("%i", acc));
    trap_Cvar_Set("ui_char_strength", va("%i", str));
    trap_Cvar_Set("ui_char_stamina", va("%i", sta));
    trap_Cvar_Set("ui_char_stealth", va("%i", ste));
    trap_Cvar_Set("ui_char_technical", va("%i", tec));
    trap_Cvar_Set("ui_char_speed", va("%i", spe));
    trap_Cvar_Set("ui_char_xp", va("%i", xp));
    trap_Cvar_Set("ui_char_old_xp", va("%i", xp));
}

void CG_CopyCharacterVariablesBack( void )
{
    int acc, str, sta, ste, tec, spe ;

    acc = trap_Cvar_VariableValue("ui_char_accuracy");
    str = trap_Cvar_VariableValue("ui_char_strength");
    sta = trap_Cvar_VariableValue("ui_char_stamina");
    ste = trap_Cvar_VariableValue("ui_char_stealth");
    tec = trap_Cvar_VariableValue("ui_char_technical");
    spe = trap_Cvar_VariableValue("ui_char_speed");
    trap_Cvar_Set("char_accuracy", va("%i", acc));
    trap_Cvar_Set("char_strength", va("%i", str));
    trap_Cvar_Set("char_stamina", va("%i", sta));
    trap_Cvar_Set("char_stealth", va("%i", ste));
    trap_Cvar_Set("char_technical", va("%i", tec));
    trap_Cvar_Set("char_speed", va("%i", spe));
}

void CG_SetCharacter ( void )
{
    char buff[16];

    memset( &buff, 0, sizeof( buff ) );

    buff[0] = 'C';
    buff[PC_STRENGTH] = 48+trap_Cvar_VariableValue("char_strength");
    buff[PC_TECHNICAL] = 48+trap_Cvar_VariableValue("char_technical");
    buff[PC_STAMINA] = 48+trap_Cvar_VariableValue("char_stamina");
    buff[PC_ACCURACY] = 48+trap_Cvar_VariableValue("char_accuracy");
    buff[PC_SPEED] = 48+trap_Cvar_VariableValue("char_speed");
    buff[PC_STEALTH] = 48+trap_Cvar_VariableValue("char_stealth");
    buff[7] = '\0';

    trap_Cvar_Set("ui_character", buff);
    trap_Cvar_Set("character", buff);

}

void CG_InitialCharacterVariables( void )
{
    // reset the ui variables
    trap_Cvar_Set("ui_char_xp", va("%i", -1));
    trap_Cvar_Set("ui_char_old_xp", va("%i", -1));
    trap_Cvar_Set("char_xp", va("%i", 0));
    trap_Cvar_Set("ui_char_accuracy", va("%i", 1));
    trap_Cvar_Set("ui_char_speed", va("%i", 1));
    trap_Cvar_Set("ui_char_strength", va("%i", 1));
    trap_Cvar_Set("ui_char_stamina", va("%i", 1));
    trap_Cvar_Set("ui_char_stealth", va("%i", 1));
    trap_Cvar_Set("ui_char_technical", va("%i", 1));
    trap_Cvar_Set("ui_gamestate", va("%i", 0));
}

static qboolean CG_HandleGrenadeButton( int flags, float *special, int key )
{
    int flash, smoke, mk26, gren40mm;

    if (key != K_MOUSE1 && key != K_MOUSE2 )
        return qfalse;

    flash = trap_Cvar_VariableValue("inven_ammo_flash");

    smoke = trap_Cvar_VariableValue("inven_ammo_smoke");

    mk26 = trap_Cvar_VariableValue("inven_ammo_mk26");

    gren40mm = trap_Cvar_VariableValue("inven_ammo_40mmgren");

    switch ( flags )
    {

    case CG_INVEN_AMMO_FLASH:
        if ( key == K_MOUSE1 )
            flash++;
        else
            flash--;

        if (flash < 0)
            flash = 0;
        if (flash > 2)
            flash = 2;

        if (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES )
            flash--;

        if (flash < 0 || (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES) )
        {
            trap_Cvar_Set( "inven_ammo_flash", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_mk26", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_40mmgren", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_smoke", va("%i", 1) );
            trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_no.wav" , qfalse ) , CHAN_LOCAL );
            return qfalse;
        }

        trap_Cvar_Set( "inven_ammo_flash", va("%i", flash) );

        break;

    case CG_INVEN_AMMO_SMOKE:
        if ( key == K_MOUSE1 )
            smoke++;
        else
            smoke--;

        if (smoke < 0)
            smoke = 0;
        if (smoke > 2)
            smoke = 2;

        if (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES )
            smoke--;

        if (smoke < 0 || (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES) )
        {
            trap_Cvar_Set( "inven_ammo_flash", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_mk26", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_40mmgren", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_smoke", va("%i", 1) );
            trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_no.wav" , qfalse ) , CHAN_LOCAL );
            return qfalse;
        }

        trap_Cvar_Set( "inven_ammo_smoke", va("%i", smoke) );

        break;

    case CG_INVEN_AMMO_MK26:
        if ( key == K_MOUSE1 )
            mk26++;
        else
            mk26--;

        if (mk26 < 0)
            mk26 = 0;
        if (mk26 > 2)
            mk26 = 2;

        if (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES )
            mk26--;

        if (mk26 < 0 || (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES) )
        {
            trap_Cvar_Set( "inven_ammo_flash", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_mk26", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_40mmgren", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_smoke", va("%i", 1) );
            trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_no.wav" , qfalse ) , CHAN_LOCAL );
            return qfalse;
        }

        trap_Cvar_Set( "inven_ammo_mk26", va("%i", mk26) );

        break;

    case CG_INVEN_AMMO_40MM:
        if ( key == K_MOUSE1 )
            gren40mm++;
        else
            gren40mm--;

        if (gren40mm < 0)
            gren40mm = 0;
        if (gren40mm > 2)
            gren40mm = 2;

        if (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES )
            gren40mm--;

        if (gren40mm < 0 || (flash + smoke + mk26 + gren40mm > SEALS_MAX_GRENADES) )
        {
            trap_Cvar_Set( "inven_ammo_flash", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_mk26", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_40mmgren", va("%i", 1) );
            trap_Cvar_Set( "inven_ammo_smoke", va("%i", 1) );
            trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_no.wav" , qfalse ) , CHAN_LOCAL );
            return qfalse;
        }

        trap_Cvar_Set( "inven_ammo_40mmgren", va("%i", gren40mm) );

        break;
    }

    trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_yes.wav", qfalse ) , CHAN_LOCAL );

    return qtrue;
}

static qboolean CG_HandleCharacterButton( int flags, float *special, int key )
{
    int value = 1;
    int xp = 0;

    xp = trap_Cvar_VariableValue("ui_gamestate");
    if (xp == STATE_LOCKED)
    {
        trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_no.wav" , qfalse ) , CHAN_LOCAL );
        return qfalse;
    }

    xp = trap_Cvar_VariableValue("ui_char_xp");

    if ( key != K_MOUSE1 && key != K_MOUSE2 )
        return qfalse;

    switch (flags)
    {
    case CG_CHARACTER_STRENGTH:
        value = trap_Cvar_VariableValue("ui_char_strength");
        break;
    case CG_CHARACTER_TECHINCAL:
        value = trap_Cvar_VariableValue("ui_char_technical");
        break;
    case CG_CHARACTER_STAMINA:
        value = trap_Cvar_VariableValue("ui_char_stamina");
        break;
    case CG_CHARACTER_ACCURACY:
        value = trap_Cvar_VariableValue("ui_char_accuracy");
        break;
    case CG_CHARACTER_SPEED:
        value = trap_Cvar_VariableValue("ui_char_speed");
        break;
    case CG_CHARACTER_STEALTH:
        value = trap_Cvar_VariableValue("ui_char_stealth");
        break;
    default:
        return qfalse;
    }

    // cannot manipulate XP
    if ( ( value >= 10 && key == K_MOUSE1 ) ||
            ( value <= 1  && key == K_MOUSE2 ) ||
            ( xp <= value && key == K_MOUSE1 ) )
    {
        trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_no.wav" , qfalse ) , CHAN_LOCAL );
        return qfalse;
    }

    if (xp > value && key == K_MOUSE1)
    {
        value++;
        xp -= value;
        trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_yes.wav", qfalse ) , CHAN_LOCAL );
    }
    else if (key == K_MOUSE2)
    {
        xp += value;
        value--;
        trap_S_StartLocalSound( trap_S_RegisterSound( "ui/assets/menu_yes.wav", qfalse ) , CHAN_LOCAL );
    }

    trap_Cvar_Set("ui_char_xp", va("%i", xp));

    switch (flags)
    {
    case CG_CHARACTER_STRENGTH:
        trap_Cvar_Set("ui_char_strength", va("%i", value));
        break;
    case CG_CHARACTER_TECHINCAL:
        trap_Cvar_Set("ui_char_technical", va("%i", value));
        break;
    case CG_CHARACTER_STAMINA:
        trap_Cvar_Set("ui_char_stamina", va("%i", value));
        break;
    case CG_CHARACTER_ACCURACY:
        trap_Cvar_Set("ui_char_accuracy", va("%i", value));
        break;
    case CG_CHARACTER_SPEED:
        trap_Cvar_Set("ui_char_speed", va("%i", value));
        break;
    case CG_CHARACTER_STEALTH:
        trap_Cvar_Set("ui_char_stealth", va("%i", value));
        break;
    }

    return qtrue;
}


static qboolean UI_Handicap_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {
        int h;
        h = Com_Clamp( 5, 100, trap_Cvar_VariableValue("handicap") );
        if (key == K_MOUSE2)
        {
            h -= 5;
        }
        else
        {
            h += 5;
        }
        if (h > 100)
        {
            h = 5;
        }
        else if (h < 0)
        {
            h = 100;
        }
        trap_Cvar_Set( "handicap", va( "%i", h) );
        return qtrue;
    }
    return qfalse;
}

static qboolean UI_Effects_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {

        if (key == K_MOUSE2)
        {
            uiInfo.effectsColor--;
        }
        else
        {
            uiInfo.effectsColor++;
        }

        if( uiInfo.effectsColor > 6 )
        {
            uiInfo.effectsColor = 0;
        }
        else if (uiInfo.effectsColor < 0)
        {
            uiInfo.effectsColor = 6;
        }

        trap_Cvar_SetValue( "color", uitogamecode[uiInfo.effectsColor] );
        return qtrue;
    }
    return qfalse;
}

static qboolean UI_ClanName_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {
        int i;
        i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
        if (uiInfo.teamList[i].cinematic >= 0)
        {
            trap_CIN_StopCinematic(uiInfo.teamList[i].cinematic);
            uiInfo.teamList[i].cinematic = -1;
        }
        if (key == K_MOUSE2)
        {
            i--;
        }
        else
        {
            i++;
        }
        if (i >= uiInfo.teamCount)
        {
            i = 0;
        }
        else if (i < 0)
        {
            i = uiInfo.teamCount - 1;
        }
        trap_Cvar_Set( "ui_teamName", uiInfo.teamList[i].teamName);
        updateModel = qtrue;
        return qtrue;
    }
    return qfalse;
}

static qboolean UI_GameType_HandleKey(int flags, float *special, int key, qboolean resetMap)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {
        int oldCount = UI_MapCountByGameType(qtrue);

        // hard coded mess here
        if (key == K_MOUSE2)
        {
            ui_gameType.integer--;
            if (ui_gameType.integer == 2)
            {
                ui_gameType.integer = 1;
            }
            else if (ui_gameType.integer < 2)
            {
                ui_gameType.integer = uiInfo.numGameTypes - 1;
            }
        }
        else
        {
            ui_gameType.integer++;
            if (ui_gameType.integer >= uiInfo.numGameTypes)
            {
                ui_gameType.integer = 1;
            }
            else if (ui_gameType.integer == 2)
            {
                ui_gameType.integer = 3;
            }
        }

         trap_Cvar_Set("ui_gameType", va("%d", ui_gameType.integer));
        UI_SetCapFragLimits(qtrue);
        UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
        if (resetMap && oldCount != UI_MapCountByGameType(qtrue))
        {
            trap_Cvar_Set( "ui_currentMap", "0");
            Menu_SetFeederSelection(NULL, FEEDER_MAPS, 0, NULL);
        }
        return qtrue;
    }
    return qfalse;
}

static qboolean UI_NetGameType_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {

        if (key == K_MOUSE2)
        {
            ui_netGameType.integer--;
        }
        else
        {
            ui_netGameType.integer++;
        }

        if (ui_netGameType.integer < 0)
        {
            ui_netGameType.integer = uiInfo.numGameTypes - 1;
        }
        else if (ui_netGameType.integer >= uiInfo.numGameTypes)
        {
            ui_netGameType.integer = 0;
        }

        trap_Cvar_Set( "ui_netGameType", va("%d", ui_netGameType.integer));
        trap_Cvar_Set( "ui_actualnetGameType", va("%d", uiInfo.gameTypes[ui_netGameType.integer].gtEnum));
        trap_Cvar_Set( "ui_currentNetMap", "0");
        UI_MapCountByGameType(qfalse);
        Menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, NULL);
        return qtrue;
    }
    return qfalse;
}

static qboolean UI_JoinGameType_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {

        if (key == K_MOUSE2)
        {
            ui_joinGameType.integer--;
        }
        else
        {
            ui_joinGameType.integer++;
        }

        if (ui_joinGameType.integer < 0)
        {
            ui_joinGameType.integer = uiInfo.numJoinGameTypes - 1;
        }
        else if (ui_joinGameType.integer >= uiInfo.numJoinGameTypes)
        {
            ui_joinGameType.integer = 0;
        }

        trap_Cvar_Set( "ui_joinGameType", va("%d", ui_joinGameType.integer));
        UI_BuildServerDisplayList(qtrue);
        return qtrue;
    }
    return qfalse;
} 
 
static qboolean UI_TeamName_HandleKey(int flags, float *special, int key, qboolean blue)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {
        int i;
        i = UI_TeamIndexFromName(UI_Cvar_VariableString((blue) ? "ui_blueTeam" : "ui_redTeam"));

        if (key == K_MOUSE2)
        {
            i--;
        }
        else
        {
            i++;
        }

        if (i >= uiInfo.teamCount)
        {
            i = 0;
        }
        else if (i < 0)
        {
            i = uiInfo.teamCount - 1;
        }

        trap_Cvar_Set( (blue) ? "ui_blueTeam" : "ui_redTeam", uiInfo.teamList[i].teamName);

        return qtrue;
    }
    return qfalse;
}

static qboolean UI_TeamMember_HandleKey(int flags, float *special, int key, qboolean blue, int num)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {
        // 0 - None
        // 1 - Human
        // 2..NumCharacters - Bot
        char *cvar = va(blue ? "ui_blueteam%i" : "ui_redteam%i", num);
        int value = trap_Cvar_VariableValue(cvar);

        if (key == K_MOUSE2)
        {
            value--;
        }
        else
        {
            value++;
        }

        if (ui_actualNetGameType.integer >= GT_TEAM)
        {
            if (value >= uiInfo.characterCount + 2)
            {
                value = 0;
            }
            else if (value < 0)
            {
                value = uiInfo.characterCount + 2 - 1;
            }
        }
        else
        {}

        trap_Cvar_Set(cvar, va("%i", value));
        return qtrue;
    }
    return qfalse;
}

static qboolean UI_NetSource_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {

        if (key == K_MOUSE2)
        {
            ui_netSource.integer--;
        }
        else
        {
            ui_netSource.integer++;
        }

        if (ui_netSource.integer >= numNetSources)
        {
            ui_netSource.integer = 0;
        }
        else if (ui_netSource.integer < 0)
        {
            ui_netSource.integer = numNetSources - 1;
        }

        UI_BuildServerDisplayList(qtrue);
        if (ui_netSource.integer != AS_GLOBAL)
        {
            UI_StartServerRefresh(qtrue);
        }
        trap_Cvar_Set( "ui_netSource", va("%d", ui_netSource.integer));
        return qtrue;
    }
    return qfalse;
}

static qboolean UI_NetFilter_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {

        if (key == K_MOUSE2)
        {
            ui_serverFilterType.integer--;
        }
        else
        {
            ui_serverFilterType.integer++;
        }

        if (ui_serverFilterType.integer >= numServerFilters)
        {
            ui_serverFilterType.integer = 0;
        }
        else if (ui_serverFilterType.integer < 0)
        {
            ui_serverFilterType.integer = numServerFilters - 1;
        }
        UI_BuildServerDisplayList(qtrue);
        return qtrue;
    }
    return qfalse;
}
 
static qboolean UI_RedBlue_HandleKey(int flags, float *special, int key)
{
    if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
    {
        uiInfo.redBlue ^= 1;
        return qtrue;
    }
    return qfalse;
}
 
  

static qboolean UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key)
{
    switch (ownerDraw)
    {
    case UI_PRIMARY_ADDON1:
        CG_HandleWeaponPrimaryAddon1( flags, special, key );
        break;
    case UI_PRIMARY_ADDON2:
        CG_HandleWeaponPrimaryAddon2( flags, special, key );
        break;
    case UI_PRIMARY_ADDON3:
        CG_HandleWeaponPrimaryAddon3( flags, special, key );
        break;
    case UI_SIDEARM_ADDON1:
        CG_HandleWeaponSecondaryAddon1( flags, special, key );
        break;
    case UI_SIDEARM_ADDON2:
        CG_HandleWeaponSecondaryAddon2( flags, special, key );
        break;
    case CG_CHARACTER_STRENGTH:
    case CG_CHARACTER_TECHINCAL:
    case CG_CHARACTER_ACCURACY:
    case CG_CHARACTER_SPEED:
    case CG_CHARACTER_STAMINA:
    case CG_CHARACTER_STEALTH:
        CG_HandleCharacterButton(ownerDraw, special, key );
        break;
    case CG_INVEN_AMMO_FLASH:
    case CG_INVEN_AMMO_SMOKE:
    case CG_INVEN_AMMO_MK26:
    case CG_INVEN_AMMO_40MM:
        CG_HandleGrenadeButton( ownerDraw, special, key );
        break;
    case UI_HANDICAP:
        return UI_Handicap_HandleKey(flags, special, key);
        break;
    case UI_EFFECTS:
        return UI_Effects_HandleKey(flags, special, key);
        break;
    case UI_CLANNAME:
        return UI_ClanName_HandleKey(flags, special, key);
        break;
    case UI_GAMETYPE:
        return UI_GameType_HandleKey(flags, special, key, qtrue);
        break;
    case UI_NETGAMETYPE:
        return UI_NetGameType_HandleKey(flags, special, key);
        break;
    case UI_JOINGAMETYPE:
        return UI_JoinGameType_HandleKey(flags, special, key);
        break;
    case UI_SKILL: 
        break;
    case UI_BLUETEAMNAME:
        return UI_TeamName_HandleKey(flags, special, key, qtrue);
        break;
    case UI_REDTEAMNAME:
        return UI_TeamName_HandleKey(flags, special, key, qfalse);
        break;
    case UI_BLUETEAM1:
    case UI_BLUETEAM2:
    case UI_BLUETEAM3:
    case UI_BLUETEAM4:
    case UI_BLUETEAM5:
    case UI_BLUETEAM6:
        UI_TeamMember_HandleKey(flags, special, key, qtrue, ownerDraw - UI_BLUETEAM1 + 1);
        break;
    case UI_REDTEAM1:
    case UI_REDTEAM2:
    case UI_REDTEAM3:
    case UI_REDTEAM4:
    case UI_REDTEAM5:
    case UI_REDTEAM6:
        UI_TeamMember_HandleKey(flags, special, key, qfalse, ownerDraw - UI_REDTEAM1 + 1);
        break;
    case UI_NETSOURCE:
        UI_NetSource_HandleKey(flags, special, key);
        break;
    case UI_NETFILTER:
        UI_NetFilter_HandleKey(flags, special, key);
        break;  
    case UI_REDBLUE:
        UI_RedBlue_HandleKey(flags, special, key);
        break;
    case UI_CROSSHAIR:  
    case UI_SELECTEDPLAYER: 
        break;
    default:
        break;
    }

    return qfalse;
}


static float UI_GetValue(int ownerDraw)
{
    return 0;
}

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 )
{
    return trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey, uiInfo.serverStatus.sortDir, *(int*)arg1, *(int*)arg2);
}


/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort(int column, qboolean force)
{

    if ( !force )
    {
        if ( uiInfo.serverStatus.sortKey == column )
        {
            return;
        }
    }

    uiInfo.serverStatus.sortKey = column;
    qsort( &uiInfo.serverStatus.displayServers[0], uiInfo.serverStatus.numDisplayServers, sizeof(int), UI_ServersQsortCompare);
}
 
/*
===============
UI_LoadMods
===============
*/
static void UI_LoadMods()
{
    int		numdirs;
    char	dirlist[2048];
    char	*dirptr;
    char  *descptr;
    int		i;
    int		dirlen;

    uiInfo.modCount = 0;
    numdirs = trap_FS_GetFileList( "$modlist", "", dirlist, sizeof(dirlist) );
    dirptr  = dirlist;
    for( i = 0; i < numdirs; i++ )
    {
        dirlen = strlen( dirptr ) + 1;
        descptr = dirptr + dirlen;
        uiInfo.modList[uiInfo.modCount].modName = String_Alloc(dirptr);
        uiInfo.modList[uiInfo.modCount].modDescr = String_Alloc(descptr);
        dirptr += dirlen + strlen(descptr) + 1;
        uiInfo.modCount++;
        if (uiInfo.modCount >= MAX_MODS)
        {
            break;
        }
    }

}




/*
===============
UI_LoadMovies
===============
*/
static void UI_LoadMovies()
{
    char	movielist[4096];
    char	*moviename;
    int		i, len;

    uiInfo.movieCount = trap_FS_GetFileList( "video", "roq", movielist, 4096 );

    if (uiInfo.movieCount)
    {
        if (uiInfo.movieCount > MAX_MOVIES)
        {
            uiInfo.movieCount = MAX_MOVIES;
        }
        moviename = movielist;
        for ( i = 0; i < uiInfo.movieCount; i++ )
        {
            len = strlen( moviename );
            if (!Q_stricmp(moviename +  len - 4,".roq"))
            {
                moviename[len-4] = '\0';
            }
            Q_strupr(moviename);
            uiInfo.movieList[i] = String_Alloc(moviename);
            moviename += len + 1;
        }
    }

}



/*
===============
UI_LoadDemos
===============
*/
static void UI_LoadDemos()
{
    char	demolist[4096];
    char demoExt[32];
    char	*demoname;
    int		i, len;

    Com_sprintf(demoExt, sizeof(demoExt), "dm_%d", (int)trap_Cvar_VariableValue("protocol"));

    uiInfo.demoCount = trap_FS_GetFileList( "demos", demoExt, demolist, 4096 );

    Com_sprintf(demoExt, sizeof(demoExt), ".dm_%d", (int)trap_Cvar_VariableValue("protocol"));

    if (uiInfo.demoCount)
    {
        if (uiInfo.demoCount > MAX_DEMOS)
        {
            uiInfo.demoCount = MAX_DEMOS;
        }
        demoname = demolist;
        for ( i = 0; i < uiInfo.demoCount; i++ )
        {
            len = strlen( demoname );
            if (!Q_stricmp(demoname +  len - strlen(demoExt), demoExt))
            {
                demoname[len-strlen(demoExt)] = '\0';
            }
            Q_strupr(demoname);
            uiInfo.demoList[i] = String_Alloc(demoname);
            demoname += len + 1;
        }
    }

}
/*
===============
UI_LoadScripts
===============
*/
char scriptList[MAX_SCRIPTS][256];

static void UI_LoadScripts()
{
    char		*text_p;
    int			len;
    int			i;
    char		*token;
    char		text[20000];
    fileHandle_t	f;

    uiInfo.scriptCount = 0;
    // load the file
    len = trap_FS_FOpenFile( "scripts.cfg", &f, FS_READ );

    if ( len <= 0 )
    {
        Com_Printf( "%s not found.\n", "scripts.cfg" );
        return ;
    }
    if ( len >= sizeof( text ) - 1 )
    {
        Com_Printf( "File %s too long\n", "scripts.cfg" );
        return ;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;

    // read information
    for ( i = 0 ; i < MAX_SCRIPTS ; i++ )
    {
        // get string
        token = COM_Parse( &text_p );
        if ( !token || strlen(token) <= 0 )
        {
            break;
        }
        strcpy( scriptList[uiInfo.scriptCount], token );
        uiInfo.scriptCount++;
    }

    Com_Printf("parsed %i scripts.\n", uiInfo.scriptCount );
    return;
}


static qboolean UI_SetNextMap(int actual, int index)
{
    int i;
    for (i = actual + 1; i < uiInfo.mapCount; i++)
    {
        if (uiInfo.mapList[i].active)
        {
            Menu_SetFeederSelection(NULL, FEEDER_MAPS, index + 1, "skirmish");
            return qtrue;
        }
    }
    return qfalse;
}


static void UI_StartSkirmish(qboolean next)
{
    int		k, g, delay, temp;
    float	skill;
    char	buff[MAX_STRING_CHARS];

    if (next)
    {
        int actual;
        int index = trap_Cvar_VariableValue("ui_mapIndex");
        UI_MapCountByGameType(qtrue);
        UI_SelectedMap(index, &actual);
        if (UI_SetNextMap(actual, index))
        {}
        else
        {
            UI_GameType_HandleKey(0, 0, K_MOUSE1, qfalse);
            UI_MapCountByGameType(qtrue);
            Menu_SetFeederSelection(NULL, FEEDER_MAPS, 0, "skirmish");
        }
    }

    g = uiInfo.gameTypes[ui_gameType.integer].gtEnum;
    trap_Cvar_SetValue( "g_gametype", g );
    trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", uiInfo.mapList[ui_currentMap.integer].mapLoadName) );
    skill = trap_Cvar_VariableValue( "g_spSkill" );
    trap_Cvar_Set("ui_scoreMap", uiInfo.mapList[ui_currentMap.integer].mapName);

    k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));

    trap_Cvar_Set("ui_singlePlayerActive", "1");

    // set up sp overrides, will be replaced on postgame
    temp = trap_Cvar_VariableValue( "capturelimit" );
    trap_Cvar_Set("ui_saveCaptureLimit", va("%i", temp));
    temp = trap_Cvar_VariableValue( "fraglimit" );
    trap_Cvar_Set("ui_saveFragLimit", va("%i", temp));

    UI_SetCapFragLimits(qfalse);

    temp = trap_Cvar_VariableValue( "cg_drawTimer" );
    trap_Cvar_Set("ui_drawTimer", va("%i", temp));
    temp = trap_Cvar_VariableValue( "g_doWarmup" );
    trap_Cvar_Set("ui_doWarmup", va("%i", temp));
    temp = trap_Cvar_VariableValue( "g_friendlyFire" );
    trap_Cvar_Set("ui_friendlyFire", va("%i", temp));
    temp = trap_Cvar_VariableValue( "sv_maxClients" );
    trap_Cvar_Set("ui_maxClients", va("%i", temp));
    temp = trap_Cvar_VariableValue( "g_warmup" );
    trap_Cvar_Set("ui_Warmup", va("%i", temp));
    temp = trap_Cvar_VariableValue( "sv_pure" );
    trap_Cvar_Set("ui_pure", va("%i", temp));

    trap_Cvar_Set("cg_cameraOrbit", "0");
    trap_Cvar_Set("cg_thirdPerson", "0");
    trap_Cvar_Set("cg_drawTimer", "1");
    trap_Cvar_Set("g_doWarmup", "1");
    trap_Cvar_Set("g_warmup", "15");
    trap_Cvar_Set("sv_pure", "0");
    trap_Cvar_Set("g_friendlyFire", "0");
    trap_Cvar_Set("g_redTeam", UI_Cvar_VariableString("ui_teamName"));
    trap_Cvar_Set("g_blueTeam", UI_Cvar_VariableString("ui_opponentName"));

    if (trap_Cvar_VariableValue("ui_recordSPDemo"))
    {
        Com_sprintf(buff, MAX_STRING_CHARS, "%s_%i", uiInfo.mapList[ui_currentMap.integer].mapLoadName, g);
        trap_Cvar_Set("ui_recordSPDemoName", buff);
    }

    delay = 500;

    if (g >= GT_TEAM )
    {
        trap_Cmd_ExecuteText( EXEC_APPEND, "wait 5; team seals\n" );
    }
}

static void UI_Update(const char *name)
{
    int	val = trap_Cvar_VariableValue(name);

    if (Q_stricmp(name, "ui_SetName") == 0)
    {
        trap_Cvar_Set( "name", UI_Cvar_VariableString("ui_Name"));
    }
    else if (Q_stricmp(name, "ui_setRate") == 0)
    {
        float rate = trap_Cvar_VariableValue("rate");
        if (rate >= 5000)
        {
            trap_Cvar_Set("cl_maxpackets", "30");
            trap_Cvar_Set("cl_packetdup", "1");
        }
        else if (rate >= 4000)
        {
            trap_Cvar_Set("cl_maxpackets", "15");
            trap_Cvar_Set("cl_packetdup", "2");		// favor less prediction errors when there's packet loss
        }
        else
        {
            trap_Cvar_Set("cl_maxpackets", "15");
            trap_Cvar_Set("cl_packetdup", "1");		// favor lower bandwidth
        }
    }
    else if (Q_stricmp(name, "ui_GetName") == 0)
    {
        trap_Cvar_Set( "ui_Name", UI_Cvar_VariableString("name"));
    }
    else if (Q_stricmp(name, "r_colorbits") == 0)
    {
        switch (val)
        {
        case 0:
            trap_Cvar_SetValue( "r_depthbits", 0 );
            trap_Cvar_SetValue( "r_stencilbits", 0 );
            break;
        case 16:
            trap_Cvar_SetValue( "r_depthbits", 16 );
            trap_Cvar_SetValue( "r_stencilbits", 0 );
            break;
        case 32:
            trap_Cvar_SetValue( "r_depthbits", 24 );
            break;
        }
    }
    else if (Q_stricmp(name, "r_lodbias") == 0)
    {
        switch (val)
        {
        case 0:
            trap_Cvar_SetValue( "r_subdivisions", 2 );
            break;
        case 1:
            trap_Cvar_SetValue( "r_subdivisions", 10 );
            break;
        case 2:
            trap_Cvar_SetValue( "r_subdivisions", 20 );
            break;
        }
    }
    else if (Q_stricmp(name, "ui_glCustom") == 0)
    {
        switch (val)
        {
        case 0:	// high quality
            trap_Cmd_ExecuteText( EXEC_APPEND, "exec configs/gfx_high.cfg\n");
            break;
        case 1: // normal
            trap_Cmd_ExecuteText( EXEC_APPEND, "exec configs/gfx_normal.cfg\n");
            break;
        case 2: // fast
            trap_Cmd_ExecuteText( EXEC_APPEND, "exec configs/gfx_fast.cfg\n");
            break;
        case 3: // fastest 
            trap_Cmd_ExecuteText( EXEC_APPEND, "exec configs/gfx_fastest.cfg\n");
            break;
        }
    }
    else if (Q_stricmp(name, "ui_mousePitch") == 0)
    {
        if (val == 0)
        {
            trap_Cvar_SetValue( "m_pitch", 0.022f );
        }
        else
        {
            trap_Cvar_SetValue( "m_pitch", -0.022f );
        }
    }
}

static void UI_RunMenuScript(char **args)
{
    const char *name, *name2;
    char buff[1024];

    if (String_Parse(args, &name))
    {
        if ( Q_stricmp(name, "freexp") == 0)
        {
            //Com_Printf("freexp\n");
            CG_FreeXP();
        }
        else if ( Q_stricmp(name, "copyCharacter") == 0)
        {
            //Com_Printf("copyCharacter\n");
            CG_CopyCharacterVariables();
        }
        else if ( Q_stricmp(name, "initialCharacter") == 0)
        {
            //Com_Printf("initialCharacter\n");
            CG_InitialCharacterVariables();
        }
        else if ( Q_stricmp(name, "updateCharacter") == 0)
        {
            //Com_Printf("updateCharacter\n");
            CG_CopyCharacterVariablesBack();
            CG_SetCharacter();
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
            int silencer;
            int	duckbill;
            int	flashlight;

            int scope_secondary;
            int gl_secondary;
            int bayonet_secondary;
            int lasersight_secondary;
            int silencer_secondary;

            char		var[MAX_TOKEN_CHARS];

            gitem_t *it_primary;
            gitem_t *it_secondary;

            //Com_Printf("updateInventory\n");
            //Com_Printf(S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: ui_main.c / UI_RunMenuScript\n");

            //
            // secondary weapon items
            //
            trap_Cvar_VariableStringBuffer("inven_lasersight_secondary", var , sizeof(var ) );
            lasersight_secondary = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_bayonet_secondary", var , sizeof(var ) );
            bayonet_secondary = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_grenadelauncher_secondary", var , sizeof(var ) );
            gl_secondary = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_scope_secondary", var , sizeof(var ) );
            scope_secondary = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_silencer_secondary", var , sizeof(var ) );
            silencer_secondary = atoi(var);

            //
            // primary weapon items
            //
            trap_Cvar_VariableStringBuffer("inven_lasersight", var , sizeof(var ) );
            lasersight = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_bayonet", var , sizeof(var ) );
            bayonet = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_grenadelauncher", var , sizeof(var ) );
            gl = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_scope", var , sizeof(var ) );
            scope = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_silencer", var , sizeof(var ) );
            silencer = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_duckbill", var , sizeof(var ) );
            duckbill = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_flashlight", var , sizeof(var ) );
            flashlight = atoi(var);


            //
            // equipment
            //
            trap_Cvar_VariableStringBuffer("inven_helmet", var , sizeof(var ) );
            helmet = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_kevlar", var , sizeof(var ) );
            kevlar = atoi(var);

            //
            // grenades
            //
            trap_Cvar_VariableStringBuffer("inven_ammo_flash", var , sizeof(var ) );
            fl_grenades = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_smoke", var , sizeof(var ) );
            sm_grenades = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_mk26", var , sizeof(var ) );
            grenades = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_40mmgren", var , sizeof(var ) );
            mmgrenades = atoi(var);

            //
            // primary weapon
            //
            trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof(var ) );
            primary = atoi(var);

            //
            // secondary weapon
            //
            trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof(var ) );
            secondary = atoi(var);

            //
            // ammo
            //
            trap_Cvar_VariableStringBuffer("inven_ammo_primary", var , sizeof(var ) );
            priammo = atoi(var);

            trap_Cvar_VariableStringBuffer("inven_ammo_secondary", var , sizeof(var ) );
            secammo = atoi(var);

            //
            // character
            //
            trap_Cvar_VariableStringBuffer("character", var, sizeof(var));

            if ( primary )
                it_primary = BG_FindItemForWeapon( primary );
            if ( secondary )
                it_secondary = BG_FindItemForWeapon( secondary );

            //	<cmd>  <primary> <secondary> <PriAmmo> <SecAmmo> <40mm grenades> <Grenades>
            // <Fl Grenades> <SmokeGrenades> <kevlar> <helmet> <scope secondary > <scope secondary> <character>
            command = va("%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %s",
                         // 1        2        3       4
                         primary,secondary,priammo,secammo,
                         // 5        6        7            8
                         mmgrenades,grenades,fl_grenades,sm_grenades,
                         // 9    10     11   12  13
                         kevlar,helmet,scope,gl,bayonet,
                         // 14        15      16
                         lasersight,silencer,scope_secondary,
                         // 17         18                 19
                         gl_secondary,bayonet_secondary,lasersight_secondary,
                         // 20                21          22       23
                         silencer_secondary, duckbill, flashlight, var );

            //then send the commmand
            trap_Cmd_ExecuteText( EXEC_APPEND, va("inventory %s\n", command) );

        }
        else if ( Q_stricmp(name, "updateLooks") == 0 )
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, va("looks") );
        }
        else if ( Q_stricmp(name, "removePrimaryWeaponItems") == 0 )
        {

            /*	trap_Cvar_Set("inven_Scope", "0");
            trap_Cvar_Set("inven_GrenadeLauncher", "0");
            trap_Cvar_Set("inven_LaserSight", "0");
            trap_Cvar_Set("inven_Bayonet", "0");
            trap_Cvar_Set("inven_Silencer", "0");
            trap_Cvar_Set("inven_Duckbill", "0");
            trap_Cvar_Set("inven_Flashlight", "0");

            Com_Printf("Removed Primary Items!\n");  */
        }
        else if ( Q_stricmp(name, "removeSecondaryWeaponItems") == 0 )
        {
            /*
            trap_Cvar_Set("inven_Scope_Secondary", "0");
            trap_Cvar_Set("inven_GrenadeLauncher_Secondary", "0");
            trap_Cvar_Set("inven_LaserSight_Secondary", "0");
            trap_Cvar_Set("inven_Bayonet_Secondary", "0");
            trap_Cvar_Set("inven_Silencer_Secondary", "0");
            trap_Cvar_Set("inven_Duckbill_Secondary", "0");
            trap_Cvar_Set("inven_Flashlight_Secondary", "0");

            Com_Printf("Removed Secondary Items!\n"); */
        }
        else if (Q_stricmp(name, "StartServer") == 0)
        {
            int i, clients;
            float skill;

            trap_Cvar_Set("cg_thirdPerson", "0");
            trap_Cvar_Set("cg_cameraOrbit", "0");
            trap_Cvar_Set("ui_singlePlayerActive", "0");
            trap_Cvar_SetValue( "dedicated", Com_Clamp( 0, 2, ui_dedicated.integer ) );
            trap_Cvar_SetValue( "g_gametype", Com_Clamp( 0, 8, uiInfo.gameTypes[ui_netGameType.integer].gtEnum ) );

            trap_Cmd_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName ) );

            skill = trap_Cvar_VariableValue( "g_spSkill" );

            // set max clients based on spots
            clients = 0;
            for (i = 0; i < PLAYERS_PER_TEAM; i++)
            {
                int bot = trap_Cvar_VariableValue( va("ui_blueteam%i", i+1));
                if (bot >= 0)
                {
                    clients++;
                }
                bot = trap_Cvar_VariableValue( va("ui_redteam%i", i+1));
                if (bot >= 0)
                {
                    clients++;
                }
            }
            if (clients == 0)
            {
                clients = PLAYERS_PER_TEAM*2;
            }
            trap_Cvar_Set("sv_maxClients", va("%d",clients));
        }
        else if (Q_stricmp(name, "updateSPMenu") == 0)
        {
            UI_SetCapFragLimits(qtrue);
            UI_MapCountByGameType(qtrue);
            ui_mapIndex.integer = UI_GetIndexFromSelection(ui_currentMap.integer);
            trap_Cvar_Set("ui_mapIndex", va("%d", ui_mapIndex.integer));
            Menu_SetFeederSelection(NULL, FEEDER_MAPS, ui_mapIndex.integer, "skirmish");
            UI_GameType_HandleKey(0, 0, K_MOUSE1, qfalse);
            UI_GameType_HandleKey(0, 0, K_MOUSE2, qfalse);
        }
        else if (Q_stricmp(name, "resetDefaults") == 0)
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, "exec default.cfg\n");
            trap_Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n");
            Controls_SetDefaults();
            trap_Cvar_Set("com_introPlayed", "1" );
            trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
        }
        else if (Q_stricmp(name, "getCDKey") == 0)
        {
            char out[17];
            trap_GetCDKey(buff, 17);
            trap_Cvar_Set("cdkey1", "");
            trap_Cvar_Set("cdkey2", "");
            trap_Cvar_Set("cdkey3", "");
            trap_Cvar_Set("cdkey4", "");
            if (strlen(buff) == CDKEY_LEN)
            {
                Q_strncpyz(out, buff, 5);
                trap_Cvar_Set("cdkey1", out);
                Q_strncpyz(out, buff + 4, 5);
                trap_Cvar_Set("cdkey2", out);
                Q_strncpyz(out, buff + 8, 5);
                trap_Cvar_Set("cdkey3", out);
                Q_strncpyz(out, buff + 12, 5);
                trap_Cvar_Set("cdkey4", out);
            }

        }
        else if (Q_stricmp(name, "verifyCDKey") == 0)
        {
            buff[0] = '\0';
            Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey1"));
            Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey2"));
            Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey3"));
            Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey4"));
            trap_Cvar_Set("cdkey", buff);
            if (trap_VerifyCDKey(buff, UI_Cvar_VariableString("cdkeychecksum")))
            {
                trap_Cvar_Set("ui_cdkeyvalid", "CD Key Appears to be valid.");
                trap_SetCDKey(buff);
            }
            else
            {
                trap_Cvar_Set("ui_cdkeyvalid", "CD Key does not appear to be valid.");
            }
        }
        else if (Q_stricmp(name, "loadArenas") == 0)
        {
            UI_LoadArenas();
            UI_MapCountByGameType(qfalse);
            Menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, "createserver");
        }
        else if (Q_stricmp(name, "saveControls") == 0)
        {
            Controls_SetConfig(qtrue);
        }
        else if (Q_stricmp(name, "loadControls") == 0)
        {
            Controls_GetConfig();
        }
        else if (Q_stricmp(name, "clearError") == 0)
        {
            trap_Cvar_Set("com_errorMessage", "");
        }
        else if (Q_stricmp(name, "loadGameInfo") == 0)
        {
#ifdef PRE_RELEASE_TADEMO
            UI_ParseGameInfo("demogameinfo.txt");
#else

            UI_ParseGameInfo("gameinfo.txt");
#endif

            UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
        }
        else if (Q_stricmp(name, "resetScores") == 0)
        {
            UI_ClearScores();
        }
        else if (Q_stricmp(name, "RefreshServers") == 0)
        {
            UI_StartServerRefresh(qtrue);
            UI_BuildServerDisplayList(qtrue);
        }
        else if (Q_stricmp(name, "RefreshFilter") == 0)
        {
            UI_StartServerRefresh(qfalse);
            UI_BuildServerDisplayList(qtrue);
        }
        else if (Q_stricmp(name, "RunSPDemo") == 0)
        {
            if (uiInfo.demoAvailable)
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va("demo %s_%i", uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum));
            }
        }
        else if (Q_stricmp(name, "LoadDemos") == 0)
        {
            UI_LoadDemos();
        }
        else if (Q_stricmp(name, "LoadScripts") == 0)
        {
            UI_LoadScripts();
        }
        else if (Q_stricmp(name, "LoadMovies") == 0)
        {
            UI_LoadMovies();
        }
        else if (Q_stricmp(name, "LoadMods") == 0)
        {
            UI_LoadMods();
        }
        else if (Q_stricmp(name, "playMovie") == 0)
        {
            if (uiInfo.previewMovie >= 0)
            {
                trap_CIN_StopCinematic(uiInfo.previewMovie);
            }
            trap_Cmd_ExecuteText( EXEC_APPEND, va("cinematic %s.roq 2\n", uiInfo.movieList[uiInfo.movieIndex]));
        }
        else if (Q_stricmp(name, "RunMod") == 0)
        {
            trap_Cvar_Set( "fs_game", uiInfo.modList[uiInfo.modIndex].modName);
            trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
        }
        else if (Q_stricmp(name, "RunDemo") == 0)
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, va("demo %s\n", uiInfo.demoList[uiInfo.demoIndex]));
        }
        else if (Q_stricmp(name, "RunScript") == 0)
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, va("exec nssl/%s.cfg\n", scriptList[uiInfo.scriptIndex]));
        }
        else if (Q_stricmp(name, "Seals") == 0)
        {
            trap_Cvar_Set( "fs_game", "");
            trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart;" );
        }
        else if (Q_stricmp(name, "closeJoin") == 0)
        {
            if (uiInfo.serverStatus.refreshActive)
            {
                UI_StopServerRefresh();
                uiInfo.serverStatus.nextDisplayRefresh = 0;
                uiInfo.nextServerStatusRefresh = 0;
                uiInfo.nextFindPlayerRefresh = 0;
                UI_BuildServerDisplayList(qtrue);
            }
            else
            {
                Menus_CloseByName("joinserver");
                Menus_OpenByName("main");
            }
        }
        else if (Q_stricmp(name, "StopRefresh") == 0)
        {
            UI_StopServerRefresh();
            uiInfo.serverStatus.nextDisplayRefresh = 0;
            uiInfo.nextServerStatusRefresh = 0;
            uiInfo.nextFindPlayerRefresh = 0;
        }
        else if (Q_stricmp(name, "UpdateFilter") == 0)
        {
            if (ui_netSource.integer == AS_LOCAL)
            {
                UI_StartServerRefresh(qtrue);
            }
            UI_BuildServerDisplayList(qtrue);
            UI_FeederSelection(FEEDER_SERVERS, 0);
        }
        else if (Q_stricmp(name, "ServerStatus") == 0)
        {
            trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], uiInfo.serverStatusAddress, sizeof(uiInfo.serverStatusAddress));
            UI_BuildServerStatus(qtrue);
        }
        else if (Q_stricmp(name, "FoundPlayerServerStatus") == 0)
        {
            Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
            UI_BuildServerStatus(qtrue);
            Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
        }
        else if (Q_stricmp(name, "FindPlayer") == 0)
        {
            UI_BuildFindPlayerList(qtrue);
            // clear the displayed server status info
            uiInfo.serverStatusInfo.numLines = 0;
            Menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
        }
        else if (Q_stricmp(name, "JoinServer") == 0)
        {
            trap_Cvar_Set("cg_thirdPerson", "0");
            trap_Cvar_Set("cg_cameraOrbit", "0");
            trap_Cvar_Set("ui_singlePlayerActive", "0");
            if (uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers)
            {
                trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, 1024);
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
            }
        }
        else if (Q_stricmp(name, "FoundPlayerJoinServer") == 0)
        {
            trap_Cvar_Set("ui_singlePlayerActive", "0");
            if (uiInfo.currentFoundPlayerServer >= 0 && uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers)
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer] ) );
            }
        }
        else if (Q_stricmp(name, "Quit") == 0)
        {
            trap_Cvar_Set("ui_singlePlayerActive", "0");
            trap_Cmd_ExecuteText( EXEC_NOW, "quit");
        }
        else if (Q_stricmp(name, "Controls") == 0)
        {
            trap_Cvar_Set( "cl_paused", "0" );//1
            trap_Key_SetCatcher( KEYCATCH_UI );
            Menus_CloseAll();
            Menus_ActivateByName("setup_menu2");
        }
        else if (Q_stricmp(name, "Leave") == 0)
        {
            trap_Cmd_ExecuteText( EXEC_APPEND, "disconnect\n" );
            trap_Key_SetCatcher( KEYCATCH_UI );
            Menus_CloseAll();
            Menus_ActivateByName("main");
        }
        else if (Q_stricmp(name, "ServerSort") == 0)
        {
            int sortColumn;
            if (Int_Parse(args, &sortColumn))
            {
                // if same column we're already sorting on then flip the direction
                if (sortColumn == uiInfo.serverStatus.sortKey)
                {
                    uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
                }
                // make sure we sort again
                UI_ServersSort(sortColumn, qtrue);
            }
        }
        else if (Q_stricmp(name, "nextSkirmish") == 0)
        {
            UI_StartSkirmish(qtrue);
        }
        else if (Q_stricmp(name, "SkirmishStart") == 0)
        {
            UI_StartSkirmish(qfalse);
        }
        else if (Q_stricmp(name, "closeingame") == 0)
        {
            trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
            trap_Key_ClearStates();
            trap_Cvar_Set( "cl_paused", "0" );
            Menus_CloseAll();
        }
        else if (Q_stricmp(name, "voteMap") == 0)
        {
            if (ui_currentNetMap.integer >=0 && ui_currentNetMap.integer < uiInfo.mapCount)
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote map %s\n",uiInfo.mapList[ui_currentNetMap.integer].mapLoadName) );
            }
        }
        else if (Q_stricmp(name, "voteKick") == 0)
        {
            if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount)
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote kick %s\n",uiInfo.playerNames[uiInfo.playerIndex]) );
            }
        }
        else if (Q_stricmp(name, "voteGame") == 0)
        {
            if (ui_netGameType.integer >= 0 && ui_netGameType.integer < uiInfo.numGameTypes)
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va("callvote g_gametype %i\n",uiInfo.gameTypes[ui_netGameType.integer].gtEnum) );
            }
        }
        else if (Q_stricmp(name, "voteLeader") == 0)
        {
            if (uiInfo.teamIndex >= 0 && uiInfo.teamIndex < uiInfo.myTeamCount)
            {
                trap_Cmd_ExecuteText( EXEC_APPEND, va("callteamvote leader %s\n",uiInfo.teamNames[uiInfo.teamIndex]) );
            }
        }
        else if (Q_stricmp(name, "addFavorite") == 0)
        {
            if (ui_netSource.integer != AS_FAVORITES)
            {
                char name[MAX_NAME_LENGTH];
                char addr[MAX_NAME_LENGTH];
                int res;

                trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
                name[0] = addr[0] = '\0';
                Q_strncpyz(name, 	Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
                Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
                if (strlen(name) > 0 && strlen(addr) > 0)
                {
                    res = trap_LAN_AddServer(AS_FAVORITES, name, addr);
                    if (res == 0)
                    {
                        // server already in the list
                        Com_Printf("Favorite already in list\n");
                    }
                    else if (res == -1)
                    {
                        // list full
                        Com_Printf("Favorite list full\n");
                    }
                    else
                    {
                        // successfully added
                        Com_Printf("Added favorite server %s\n", addr);
                    }
                }
            }
        }
        else if (Q_stricmp(name, "deleteFavorite") == 0)
        {
            if (ui_netSource.integer == AS_FAVORITES)
            {
                char addr[MAX_NAME_LENGTH];
                trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
                addr[0] = '\0';
                Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
                if (strlen(addr) > 0)
                {
                    trap_LAN_RemoveServer(AS_FAVORITES, addr);
                }
            }
        }
        else if (Q_stricmp(name, "createFavorite") == 0)
        {
            if (ui_netSource.integer == AS_FAVORITES)
            {
                char name[MAX_NAME_LENGTH];
                char addr[MAX_NAME_LENGTH];
                int res;

                name[0] = addr[0] = '\0';
                Q_strncpyz(name, 	UI_Cvar_VariableString("ui_favoriteName"), MAX_NAME_LENGTH);
                Q_strncpyz(addr, 	UI_Cvar_VariableString("ui_favoriteAddress"), MAX_NAME_LENGTH);
                if (strlen(name) > 0 && strlen(addr) > 0)
                {
                    res = trap_LAN_AddServer(AS_FAVORITES, name, addr);
                    if (res == 0)
                    {
                        // server already in the list
                        Com_Printf("Favorite already in list\n");
                    }
                    else if (res == -1)
                    {
                        // list full
                        Com_Printf("Favorite list full\n");
                    }
                    else
                    {
                        // successfully added
                        Com_Printf("Added favorite server %s\n", addr);
                    }
                }
            }
        }
        else if (Q_stricmp(name, "orders") == 0)
        {
            const char *orders;
            if (String_Parse(args, &orders))
            {
                int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
                if (selectedPlayer < uiInfo.myTeamCount)
                {
                    strcpy(buff, orders);
                    trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
                    trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
                }
                else
                {
                    int i;
                    for (i = 0; i < uiInfo.myTeamCount; i++)
                    {
                        if (Q_stricmp(UI_Cvar_VariableString("name"), uiInfo.teamNames[i]) == 0)
                        {
                            continue;
                        }
                        strcpy(buff, orders);
                        trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamNames[i]) );
                        trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
                    }
                }
                trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
                trap_Key_ClearStates();
                trap_Cvar_Set( "cl_paused", "0" );
                Menus_CloseAll();
            }
        }
        else if (Q_stricmp(name, "voiceOrdersTeam") == 0)
        {
            const char *orders;
            if (String_Parse(args, &orders))
            {
                int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
                if (selectedPlayer == uiInfo.myTeamCount)
                {
                    trap_Cmd_ExecuteText( EXEC_APPEND, orders );
                    trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
                }
                trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
                trap_Key_ClearStates();
                trap_Cvar_Set( "cl_paused", "0" );
                Menus_CloseAll();
            }
        }
        else if (Q_stricmp(name, "voiceOrders") == 0)
        {
            const char *orders;
            if (String_Parse(args, &orders))
            {
                int selectedPlayer = trap_Cvar_VariableValue("cg_selectedPlayer");
                if (selectedPlayer < uiInfo.myTeamCount)
                {
                    strcpy(buff, orders);
                    trap_Cmd_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
                    trap_Cmd_ExecuteText( EXEC_APPEND, "\n" );
                }
                trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
                trap_Key_ClearStates();
                trap_Cvar_Set( "cl_paused", "0" );
                Menus_CloseAll();
            }
        }
        else if (Q_stricmp(name, "glCustom") == 0)
        {
            trap_Cvar_Set("ui_glCustom", "4");
        }
        else if (Q_stricmp(name, "update") == 0)
        {
            if (String_Parse(args, &name2))
            {
                UI_Update(name2);
            }
        }
        else
        {
            Com_Printf("unknown UI script %s\n", name);
        }
    }
}
static void UI_GetTeamColor(vec4_t *color)
{}

/*
==================
UI_MapCountByGameType
==================
*/
static int UI_MapCountByGameType(qboolean singlePlayer)
{
    int i, c, game;
    c = 0;
    game = singlePlayer ? uiInfo.gameTypes[ui_gameType.integer].gtEnum : uiInfo.gameTypes[ui_netGameType.integer].gtEnum;

    if (game == GT_TEAM)
    {
        game = GT_FFA;
    }

    for (i = 0; i < uiInfo.mapCount; i++)
    {
        uiInfo.mapList[i].active = qfalse;
        if ( uiInfo.mapList[i].typeBits & (1 << game))
        {
            c++;
            uiInfo.mapList[i].active = qtrue;
        }
    }
    return c;
}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList(int num, int position)
{
    int i;

    if (position < 0 || position > uiInfo.serverStatus.numDisplayServers )
    {
        return;
    }
    //
    uiInfo.serverStatus.numDisplayServers++;
    for (i = uiInfo.serverStatus.numDisplayServers; i > position; i--)
    {
        uiInfo.serverStatus.displayServers[i] = uiInfo.serverStatus.displayServers[i-1];
    }
    uiInfo.serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList(int num)
{
    int i, j;

    for (i = 0; i < uiInfo.serverStatus.numDisplayServers; i++)
    {
        if (uiInfo.serverStatus.displayServers[i] == num)
        {
            uiInfo.serverStatus.numDisplayServers--;
            for (j = i; j < uiInfo.serverStatus.numDisplayServers; j++)
            {
                uiInfo.serverStatus.displayServers[j] = uiInfo.serverStatus.displayServers[j+1];
            }
            return;
        }
    }
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion(int num)
{
    int mid, offset, res, len;

    // use binary search to insert server
    len = uiInfo.serverStatus.numDisplayServers;
    mid = len;
    offset = 0;
    res = 0;
    while(mid > 0)
    {
        mid = len >> 1;
        //
        res = trap_LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey,
                                       uiInfo.serverStatus.sortDir, num, uiInfo.serverStatus.displayServers[offset+mid]);
        // if equal
        if (res == 0)
        {
            UI_InsertServerIntoDisplayList(num, offset+mid);
            return;
        }
        // if larger
        else if (res == 1)
        {
            offset += mid;
            len -= mid;
        }
        // if smaller
        else
        {
            len -= mid;
        }
    }
    if (res == 1)
    {
        offset++;
    }
    UI_InsertServerIntoDisplayList(num, offset);
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList(qboolean force)
{
    int i, count, clients, maxClients, ping, game, len, visible;
    char info[MAX_STRING_CHARS];
    //	qboolean startRefresh = qtrue;
    static int numinvisible;

    if (!(force || uiInfo.uiDC.realTime > uiInfo.serverStatus.nextDisplayRefresh))
    {
        return;
    }
    // if we shouldn't reset
    if ( force == 2 )
    {
        force = 0;
    }

    // do motd updates here too
    trap_Cvar_VariableStringBuffer( "cl_motdString", uiInfo.serverStatus.motd, sizeof(uiInfo.serverStatus.motd) );
    len = strlen(uiInfo.serverStatus.motd);
    if (len == 0)
    {
        strcpy(uiInfo.serverStatus.motd, "Welcome to Navy Seals : Covert Operations.");
        len = strlen(uiInfo.serverStatus.motd);
    }
    if (len != uiInfo.serverStatus.motdLen)
    {
        uiInfo.serverStatus.motdLen = len;
        uiInfo.serverStatus.motdWidth = -1;
    }

    if (force)
    {
        numinvisible = 0;
        // clear number of displayed servers
        uiInfo.serverStatus.numDisplayServers = 0;
        uiInfo.serverStatus.numPlayersOnServers = 0;
        // set list box index to zero
        Menu_SetFeederSelection(NULL, FEEDER_SERVERS, 0, NULL);
        // mark all servers as visible so we store ping updates for them
        trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
    }

    // get the server count (comes from the master)
    count = trap_LAN_GetServerCount(ui_netSource.integer);
    if (count == -1 || (ui_netSource.integer == AS_LOCAL && count == 0) )
    {
        // still waiting on a response from the master
        uiInfo.serverStatus.numDisplayServers = 0;
        uiInfo.serverStatus.numPlayersOnServers = 0;
        uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 500;
        return;
    }

	//Com_Printf("%i server count\n", count );

    visible = qfalse;
    for (i = 0; i < count; i++)
    {
        // if we already got info for this server
        if (!trap_LAN_ServerIsVisible(ui_netSource.integer, i))
        { 
            continue;
        }
        visible = qtrue;
        // get the ping for this server
        ping = trap_LAN_GetServerPing(ui_netSource.integer, i);

        if (ping > 0 || ui_netSource.integer == AS_FAVORITES)
        { 
            trap_LAN_GetServerInfo(ui_netSource.integer, i, info, MAX_STRING_CHARS);

            clients = atoi(Info_ValueForKey(info, "clients"));
            uiInfo.serverStatus.numPlayersOnServers += clients;

            if (ui_browserShowEmpty.integer == 0)
            {
                if (clients == 0)
                {
                    trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
                    continue;
                }
            }

            if (ui_browserShowFull.integer == 0)
            {
                maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
                if (clients == maxClients)
                {
                    trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
                    continue;
                }
            }

            if (uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum != -1)
            {
                game = atoi(Info_ValueForKey(info, "gametype"));
                if (game != uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum)
                {
                    trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
                    continue;
                }
            }

            // only show ns-co servers
    /*        if (Q_stricmp(Info_ValueForKey(info, "game"), "seals") != 0)
            {
                trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
                continue;
            }*/
            /*
            //			if (ui_serverFilterType.integer > 0) {
            if (Q_stricmp(Info_ValueForKey(info, "game"), serverFilters[ui_serverFilterType.integer].basedir) != 0) {
            trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
            continue;
            }
            //			}
            */
            // make sure we never add a favorite server twice
            if (ui_netSource.integer == AS_FAVORITES)
            {
                UI_RemoveServerFromDisplayList(i);
            }
            // insert the server into the list
            UI_BinaryServerInsertion(i);
            // done with this server
            if (ping > 0)
            {
                trap_LAN_MarkServerVisible(ui_netSource.integer, i, qfalse);
                numinvisible++;
            }
        }
    }

    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime;

    // if there were no servers visible for ping updates
    if (!visible)
    {
        //		UI_StopServerRefresh();
        //		uiInfo.serverStatus.nextDisplayRefresh = 0;
    }
}

typedef struct
{
    char *name, *altName;
}
serverStatusCvar_t;

serverStatusCvar_t serverStatusCvars[] = {
            {"sv_hostname", "Name"},
            {"Address", ""},
            {"gamename", "Game name"},
            {"g_gametype", "Game type"},
            {"mapname", "Map"},
            {"version", ""},
            {"protocol", ""},
            {"timelimit", ""},
            {"fraglimit", ""},
            {NULL, NULL}
        };

/*
==================
UI_SortServerStatusInfo
==================
*/
static void UI_SortServerStatusInfo( serverStatusInfo_t *info )
{
    int i, j, index;
    char *tmp1, *tmp2;

    // FIXME: if "gamename" == "baseq3" or "missionpack" then
    // replace the gametype number by FFA, CTF etc.
    //
    index = 0;
    for (i = 0; serverStatusCvars[i].name; i++)
    {
        for (j = 0; j < info->numLines; j++)
        {
            if ( !info->lines[j][1] || info->lines[j][1][0] )
            {
                continue;
            }
            if ( !Q_stricmp(serverStatusCvars[i].name, info->lines[j][0]) )
            {
                // swap lines
                tmp1 = info->lines[index][0];
                tmp2 = info->lines[index][3];
                info->lines[index][0] = info->lines[j][0];
                info->lines[index][3] = info->lines[j][3];
                info->lines[j][0] = tmp1;
                info->lines[j][3] = tmp2;
                //
                if ( strlen(serverStatusCvars[i].altName) )
                {
                    info->lines[index][0] = serverStatusCvars[i].altName;
                }
                index++;
            }
        }
    }
}

/*
==================
UI_GetServerStatusInfo
==================
*/
static int UI_GetServerStatusInfo( const char *serverAddress, serverStatusInfo_t *info )
{
    char *p, *score, *ping, *name;
    int i, len;

    if (!info)
    {
        trap_LAN_ServerStatus( serverAddress, NULL, 0);
        return qfalse;
    }
    memset(info, 0, sizeof(*info));
    if ( trap_LAN_ServerStatus( serverAddress, info->text, sizeof(info->text)) )
    {
        Q_strncpyz(info->address, serverAddress, sizeof(info->address));
        p = info->text;
        info->numLines = 0;
        info->lines[info->numLines][0] = "Address";
        info->lines[info->numLines][1] = "";
        info->lines[info->numLines][2] = "";
        info->lines[info->numLines][3] = info->address;
        info->numLines++;
        // get the cvars
        while (p && *p)
        {
            p = strchr(p, '\\');
            if (!p)
                break;
            *p++ = '\0';
            if (*p == '\\')
                break;
            info->lines[info->numLines][0] = p;
            info->lines[info->numLines][1] = "";
            info->lines[info->numLines][2] = "";
            p = strchr(p, '\\');
            if (!p)
                break;
            *p++ = '\0';
            info->lines[info->numLines][3] = p;

            info->numLines++;
            if (info->numLines >= MAX_SERVERSTATUS_LINES)
                break;
        }
        // get the player list
        if (info->numLines < MAX_SERVERSTATUS_LINES-3)
        {
            // empty line
            info->lines[info->numLines][0] = "";
            info->lines[info->numLines][1] = "";
            info->lines[info->numLines][2] = "";
            info->lines[info->numLines][3] = "";
            info->numLines++;
            // header
            info->lines[info->numLines][0] = "num";
            info->lines[info->numLines][1] = "score";
            info->lines[info->numLines][2] = "ping";
            info->lines[info->numLines][3] = "name";
            info->numLines++;
            // parse players
            i = 0;
            len = 0;
            while (p && *p)
            {
                if (*p == '\\')
                    *p++ = '\0';
                if (!p)
                    break;
                score = p;
                p = strchr(p, ' ');
                if (!p)
                    break;
                *p++ = '\0';
                ping = p;
                p = strchr(p, ' ');
                if (!p)
                    break;
                *p++ = '\0';
                name = p;
                Com_sprintf(&info->pings[len], sizeof(info->pings)-len, "%d", i);
                info->lines[info->numLines][0] = &info->pings[len];
                len += strlen(&info->pings[len]) + 1;
                info->lines[info->numLines][1] = score;
                info->lines[info->numLines][2] = ping;
                info->lines[info->numLines][3] = name;
                info->numLines++;
                if (info->numLines >= MAX_SERVERSTATUS_LINES)
                    break;
                p = strchr(p, '\\');
                if (!p)
                    break;
                *p++ = '\0';
                //
                i++;
            }
        }
        UI_SortServerStatusInfo( info );
        return qtrue;
    }
    return qfalse;
}

/*
==================
stristr
==================
*/
static char *stristr(char *str, char *charset)
{
    int i;

    while(*str)
    {
        for (i = 0; charset[i] && str[i]; i++)
        {
            if (toupper(charset[i]) != toupper(str[i]))
                break;
        }
        if (!charset[i])
            return str;
        str++;
    }
    return NULL;
}

/*
==================
UI_BuildFindPlayerList
==================
*/
static void UI_BuildFindPlayerList(qboolean force)
{
    static numFound, numTimeOuts;
    int i, j, resend;
    serverStatusInfo_t info;
    char name[MAX_NAME_LENGTH+2];
    char infoString[MAX_STRING_CHARS];

    if (!force)
    {
        if (!uiInfo.nextFindPlayerRefresh || uiInfo.nextFindPlayerRefresh > uiInfo.uiDC.realTime)
        {
            return;
        }
    }
    else
    {
        memset(&uiInfo.pendingServerStatus, 0, sizeof(uiInfo.pendingServerStatus));
        uiInfo.numFoundPlayerServers = 0;
        uiInfo.currentFoundPlayerServer = 0;
        trap_Cvar_VariableStringBuffer( "ui_findPlayer", uiInfo.findPlayerName, sizeof(uiInfo.findPlayerName));
        Q_CleanStr(uiInfo.findPlayerName);
        // should have a string of some length
        if (!strlen(uiInfo.findPlayerName))
        {
            uiInfo.nextFindPlayerRefresh = 0;
            return;
        }
        // set resend time
        resend = ui_serverStatusTimeOut.integer / 2 - 10;
        if (resend < 50)
        {
            resend = 50;
        }
        trap_Cvar_Set("cl_serverStatusResendTime", va("%d", resend));
        // reset all server status requests
        trap_LAN_ServerStatus( NULL, NULL, 0);
        //
        uiInfo.numFoundPlayerServers = 1;
        Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
                    sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
                    "searching %d...", uiInfo.pendingServerStatus.num);
        numFound = 0;
        numTimeOuts++;
    }
    for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++)
    {
        // if this pending server is valid
        if (uiInfo.pendingServerStatus.server[i].valid)
        {
            // try to get the server status for this server
            if (UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, &info ) )
            {
                //
                numFound++;
                // parse through the server status lines
                for (j = 0; j < info.numLines; j++)
                {
                    // should have ping info
                    if ( !info.lines[j][2] || !info.lines[j][2][0] )
                    {
                        continue;
                    }
                    // clean string first
                    Q_strncpyz(name, info.lines[j][3], sizeof(name));
                    Q_CleanStr(name);
                    // if the player name is a substring
                    if (stristr(name, uiInfo.findPlayerName))
                    {
                        // add to found server list if we have space (always leave space for a line with the number found)
                        if (uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS-1)
                        {
                            //
                            Q_strncpyz(uiInfo.foundPlayerServerAddresses[uiInfo.numFoundPlayerServers-1],
                                       uiInfo.pendingServerStatus.server[i].adrstr,
                                       sizeof(uiInfo.foundPlayerServerAddresses[0]));
                            Q_strncpyz(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
                                       uiInfo.pendingServerStatus.server[i].name,
                                       sizeof(uiInfo.foundPlayerServerNames[0]));
                            uiInfo.numFoundPlayerServers++;
                        }
                        else
                        {
                            // can't add any more so we're done
                            uiInfo.pendingServerStatus.num = uiInfo.serverStatus.numDisplayServers;
                        }
                    }
                }
                Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
                            sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
                            "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
                // retrieved the server status so reuse this spot
                uiInfo.pendingServerStatus.server[i].valid = qfalse;
            }
        }
        // if empty pending slot or timed out
        if (!uiInfo.pendingServerStatus.server[i].valid ||
                uiInfo.pendingServerStatus.server[i].startTime < uiInfo.uiDC.realTime - ui_serverStatusTimeOut.integer)
        {
            if (uiInfo.pendingServerStatus.server[i].valid)
            {
                numTimeOuts++;
            }
            // reset server status request for this address
            UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, NULL );
            // reuse pending slot
            uiInfo.pendingServerStatus.server[i].valid = qfalse;
            // if we didn't try to get the status of all servers in the main browser yet
            if (uiInfo.pendingServerStatus.num < uiInfo.serverStatus.numDisplayServers)
            {
                uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiDC.realTime;
                trap_LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
                                                uiInfo.pendingServerStatus.server[i].adrstr, sizeof(uiInfo.pendingServerStatus.server[i].adrstr));
                trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num], infoString, sizeof(infoString));
                Q_strncpyz(uiInfo.pendingServerStatus.server[i].name, Info_ValueForKey(infoString, "hostname"), sizeof(uiInfo.pendingServerStatus.server[0].name));
                uiInfo.pendingServerStatus.server[i].valid = qtrue;
                uiInfo.pendingServerStatus.num++;
                Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
                            sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
                            "searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
            }
        }
    }
    for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++)
    {
        if (uiInfo.pendingServerStatus.server[i].valid)
        {
            break;
        }
    }
    // if still trying to retrieve server status info
    if (i < MAX_SERVERSTATUSREQUESTS)
    {
        uiInfo.nextFindPlayerRefresh = uiInfo.uiDC.realTime + 25;
    }
    else
    {
        // add a line that shows the number of servers found
        if (!uiInfo.numFoundPlayerServers)
        {
            Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]), "no servers found");
        }
        else
        {
            Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]),
                        "%d server%s found with player %s", uiInfo.numFoundPlayerServers-1,
                        uiInfo.numFoundPlayerServers == 2 ? "":"s", uiInfo.findPlayerName);
        }
        uiInfo.nextFindPlayerRefresh = 0;
        // show the server status info for the selected server
        UI_FeederSelection(FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer);
    }
}

/*
==================
UI_BuildServerStatus
==================
*/
static void UI_BuildServerStatus(qboolean force)
{

    if (uiInfo.nextFindPlayerRefresh)
    {
        return;
    }
    if (!force)
    {
        if (!uiInfo.nextServerStatusRefresh || uiInfo.nextServerStatusRefresh > uiInfo.uiDC.realTime)
        {
            return;
        }
    }
    else
    {
        Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
        uiInfo.serverStatusInfo.numLines = 0;
        // reset all server status requests
        trap_LAN_ServerStatus( NULL, NULL, 0);
    }
    if (uiInfo.serverStatus.currentServer < 0 || uiInfo.serverStatus.currentServer > uiInfo.serverStatus.numDisplayServers || uiInfo.serverStatus.numDisplayServers == 0)
    {
        return;
    }
    if (UI_GetServerStatusInfo( uiInfo.serverStatusAddress, &uiInfo.serverStatusInfo ) )
    {
        uiInfo.nextServerStatusRefresh = 0;
        UI_GetServerStatusInfo( uiInfo.serverStatusAddress, NULL );
    }
    else
    {
        uiInfo.nextServerStatusRefresh = uiInfo.uiDC.realTime + 500;
    }
}

/*
==================
UI_FeederCount
==================
*/
static int UI_FeederCount(float feederID)
{
    if (feederID == FEEDER_HEADS)
    {
        return uiInfo.characterCount;
    }
    else if (feederID == FEEDER_Q3HEADS)
    {
        return uiInfo.q3HeadCount;
    }
    else if (feederID == FEEDER_CINEMATICS)
    {
        return uiInfo.movieCount;
    }
    else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS)
    {
        return UI_MapCountByGameType(feederID == FEEDER_MAPS ? qtrue : qfalse);
    }
    else if (feederID == FEEDER_SERVERS)
    {
        return uiInfo.serverStatus.numDisplayServers;
    }
    else if (feederID == FEEDER_SERVERSTATUS)
    {
        return uiInfo.serverStatusInfo.numLines;
    }
    else if (feederID == FEEDER_FINDPLAYER)
    {
        return uiInfo.numFoundPlayerServers;
    }
    else if (feederID == FEEDER_PLAYER_LIST)
    {
        if (uiInfo.uiDC.realTime > uiInfo.playerRefresh)
        {
            uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
            UI_BuildPlayerList();
        }
        return uiInfo.playerCount;
    }
    else if (feederID == FEEDER_TEAM_LIST)
    {
        if (uiInfo.uiDC.realTime > uiInfo.playerRefresh)
        {
            uiInfo.playerRefresh = uiInfo.uiDC.realTime + 3000;
            UI_BuildPlayerList();
        }
        return uiInfo.myTeamCount;
    }
    else if (feederID == FEEDER_MODS)
    {
        return uiInfo.modCount;
    }
    else if (feederID == FEEDER_DEMOS)
    {
        return uiInfo.demoCount;
    }
    else if (feederID == FEEDER_SCRIPTS)
    {
        return uiInfo.scriptCount;
    }
    return 0;
}

static const char *UI_SelectedMap(int index, int *actual)
{
    int i, c;
    c = 0;
    *actual = 0;
    for (i = 0; i < uiInfo.mapCount; i++)
    {
        if (uiInfo.mapList[i].active)
        {
            if (c == index)
            {
                *actual = i;
                return uiInfo.mapList[i].mapName;
            }
            else
            {
                c++;
            }
        }
    }
    return "";
}

static int UI_GetIndexFromSelection(int actual)
{
    int i, c;
    c = 0;
    for (i = 0; i < uiInfo.mapCount; i++)
    {
        if (uiInfo.mapList[i].active)
        {
            if (i == actual)
            {
                return c;
            }
            c++;
        }
    }
    return 0;
}

static void UI_UpdatePendingPings()
{
    trap_LAN_ResetPings(ui_netSource.integer);
    uiInfo.serverStatus.refreshActive = qtrue;
    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;

}

static const char *UI_FeederItemText(float feederID, int index, int column, qhandle_t *handle)
{
    static char info[MAX_STRING_CHARS];
    static char hostname[1024];
    static char clientBuff[32];
    static lastColumn = -1;
    static lastTime = 0;
    *handle = -1;
    if (feederID == FEEDER_HEADS)
    {
        if (index >= 0 && index < uiInfo.characterCount)
        {
            return uiInfo.characterList[index].name;
        }
    }
    else if (feederID == FEEDER_Q3HEADS)
    {
        if (index >= 0 && index < uiInfo.q3HeadCount)
        {
            return uiInfo.q3HeadNames[index];
        }
    }
    else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS)
    {
        int actual;
        return UI_SelectedMap(index, &actual);
    }
    else if (feederID == FEEDER_SERVERS)
    {
        if (index >= 0 && index < uiInfo.serverStatus.numDisplayServers)
        {
            int ping, game;
            if (lastColumn != column || lastTime > uiInfo.uiDC.realTime + 5000)
            {
                trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
                lastColumn = column;
                lastTime = uiInfo.uiDC.realTime;
            }
            ping = atoi(Info_ValueForKey(info, "ping"));
            if (ping == -1)
            {
                // if we ever see a ping that is out of date, do a server refresh
                // UI_UpdatePendingPings();
            }
            switch (column)
            {
            case SORT_HOST :
                if (ping <= 0)
                {
                    return Info_ValueForKey(info, "addr");
                }
                else
                {
                    if ( ui_netSource.integer == AS_LOCAL )
                    {
                        Com_sprintf( hostname, sizeof(hostname), "%s [%s]",
                                     Info_ValueForKey(info, "hostname"),
                                     netnames[atoi(Info_ValueForKey(info, "nettype"))] );
                        return hostname;
                    }
                    else
                    {
                        return Info_ValueForKey(info, "hostname");
                    }
                }
            case SORT_MAP :
                return Info_ValueForKey(info, "mapname");
            case SORT_CLIENTS :
                Com_sprintf( clientBuff, sizeof(clientBuff), "%s (%s)", Info_ValueForKey(info, "clients"), Info_ValueForKey(info, "sv_maxclients"));
                return clientBuff;
            case SORT_GAME :
                game = atoi(Info_ValueForKey(info, "gametype"));
                if (game >= 0 && game < numTeamArenaGameTypes)
                {
                    return teamArenaGameTypes[game];
                }
                else
                {
                    return "Unknown";
                }
            case SORT_PING :
                if (ping <= 0)
                {
                    return "...";
                }
                else
                {
                    return Info_ValueForKey(info, "ping");
                }
            }
        }
    }
    else if (feederID == FEEDER_SERVERSTATUS)
    {
        if ( index >= 0 && index < uiInfo.serverStatusInfo.numLines )
        {
            if ( column >= 0 && column < 4 )
            {
                return uiInfo.serverStatusInfo.lines[index][column];
            }
        }
    }
    else if (feederID == FEEDER_FINDPLAYER)
    {
        if ( index >= 0 && index < uiInfo.numFoundPlayerServers )
        {
            //return uiInfo.foundPlayerServerAddresses[index];
            return uiInfo.foundPlayerServerNames[index];
        }
    }
    else if (feederID == FEEDER_PLAYER_LIST)
    {
        if (index >= 0 && index < uiInfo.playerCount)
        {
            return uiInfo.playerNames[index];
        }
    }
    else if (feederID == FEEDER_TEAM_LIST)
    {
        if (index >= 0 && index < uiInfo.myTeamCount)
        {
            return uiInfo.teamNames[index];
        }
    }
    else if (feederID == FEEDER_MODS)
    {
        if (index >= 0 && index < uiInfo.modCount)
        {
            if (uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr)
            {
                return uiInfo.modList[index].modDescr;
            }
            else
            {
                return uiInfo.modList[index].modName;
            }
        }
    }
    else if (feederID == FEEDER_CINEMATICS)
    {
        if (index >= 0 && index < uiInfo.movieCount)
        {
            return uiInfo.movieList[index];
        }
    }
    else if (feederID == FEEDER_DEMOS)
    {
        if (index >= 0 && index < uiInfo.demoCount)
        {
            return uiInfo.demoList[index];
        }
    }
    else if (feederID == FEEDER_SCRIPTS)
    {
        if (index >= 0 && index < uiInfo.scriptCount)
        {
            return scriptList[index];
        }
    }
    return "";
}


static qhandle_t UI_FeederItemImage(float feederID, int index)
{
    if (feederID == FEEDER_HEADS)
    {
        if (index >= 0 && index < uiInfo.characterCount)
        {
            if (uiInfo.characterList[index].headImage == -1)
            {
                uiInfo.characterList[index].headImage = trap_R_RegisterShaderNoMip(uiInfo.characterList[index].imageName);
            }
            return uiInfo.characterList[index].headImage;
        }
    }
    else if (feederID == FEEDER_Q3HEADS)
    {
        if (index >= 0 && index < uiInfo.q3HeadCount)
        {
            return uiInfo.q3HeadIcons[index];
        }
    }
    else if (feederID == FEEDER_ALLMAPS || feederID == FEEDER_MAPS)
    {
        int actual;
        UI_SelectedMap(index, &actual);
        index = actual;
        if (index >= 0 && index < uiInfo.mapCount)
        {
            if (uiInfo.mapList[index].levelShot == -1)
            {
                uiInfo.mapList[index].levelShot = trap_R_RegisterShaderNoMip(uiInfo.mapList[index].imageName);
            }
            return uiInfo.mapList[index].levelShot;
        }
    }
    return 0;
}

static void UI_FeederSelection(float feederID, int index)
{
    static char info[MAX_STRING_CHARS];

    if (feederID == FEEDER_HEADS)
    {
        if (index >= 0 && index < uiInfo.characterCount)
        {
            trap_Cvar_Set( "team_model", uiInfo.characterList[index].female ? "janet" : "james");
            trap_Cvar_Set( "team_headmodel", va("*%s", uiInfo.characterList[index].name));
            updateModel = qtrue;
        }
    }
    else if (feederID == FEEDER_Q3HEADS)
    {
        if (index >= 0 && index < uiInfo.q3HeadCount)
        {
            trap_Cvar_Set( "model", uiInfo.q3HeadNames[index]);
            trap_Cvar_Set( "headmodel", uiInfo.q3HeadNames[index]);
            updateModel = qtrue;
        }
    }
    else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS)
    {
        int actual, map;
        map = (feederID == FEEDER_ALLMAPS) ? ui_currentNetMap.integer : ui_currentMap.integer;
        if (uiInfo.mapList[map].cinematic >= 0)
        {
            trap_CIN_StopCinematic(uiInfo.mapList[map].cinematic);
            uiInfo.mapList[map].cinematic = -1;
        }
        UI_SelectedMap(index, &actual);
        trap_Cvar_Set("ui_mapIndex", va("%d", index));
        ui_mapIndex.integer = index;

        if (feederID == FEEDER_MAPS)
        {
            ui_currentMap.integer = actual;
            trap_Cvar_Set("ui_currentMap", va("%d", actual));
            uiInfo.mapList[ui_currentMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
            UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
            trap_Cvar_Set("ui_opponentModel", uiInfo.mapList[ui_currentMap.integer].opponentName);
            updateOpponentModel = qtrue;
        }
        else
        {
            ui_currentNetMap.integer = actual;
            trap_Cvar_Set("ui_currentNetMap", va("%d", actual));
            uiInfo.mapList[ui_currentNetMap.integer].cinematic = trap_CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
        }

    }
    else if (feederID == FEEDER_SERVERS)
    {
        const char *mapName = NULL;
        uiInfo.serverStatus.currentServer = index;
        trap_LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
        uiInfo.serverStatus.currentServerPreview = trap_R_RegisterShaderNoMip(va("levelshots/%s", Info_ValueForKey(info, "mapname")));
        if (uiInfo.serverStatus.currentServerCinematic >= 0)
        {
            trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
            uiInfo.serverStatus.currentServerCinematic = -1;
        }
        mapName = Info_ValueForKey(info, "mapname");
        if (mapName && *mapName)
        {
            uiInfo.serverStatus.currentServerCinematic = trap_CIN_PlayCinematic(va("%s.roq", mapName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
        }
    }
    else if (feederID == FEEDER_SERVERSTATUS)
    {
    }
    else if (feederID == FEEDER_FINDPLAYER)
    {
        uiInfo.currentFoundPlayerServer = index;
        //
        if ( index < uiInfo.numFoundPlayerServers-1)
        {
            // build a new server status for this server
            Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
            Menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
            UI_BuildServerStatus(qtrue);
        }
    }
    else if (feederID == FEEDER_PLAYER_LIST)
    {
        uiInfo.playerIndex = index;
    }
    else if (feederID == FEEDER_TEAM_LIST)
    {
        uiInfo.teamIndex = index;
    }
    else if (feederID == FEEDER_MODS)
    {
        uiInfo.modIndex = index;
    }
    else if (feederID == FEEDER_CINEMATICS)
    {
        uiInfo.movieIndex = index;
        if (uiInfo.previewMovie >= 0)
        {
            trap_CIN_StopCinematic(uiInfo.previewMovie);
        }
        uiInfo.previewMovie = -1;
    }
    else if (feederID == FEEDER_DEMOS)
    {
        uiInfo.demoIndex = index;
    }
    else if ( feederID == FEEDER_SCRIPTS )
    {
        uiInfo.scriptIndex = index;
    }
}

static qboolean Team_Parse(char **p)
{
    char *token;
    const char *tempStr;
    int i;


    token = COM_ParseExt(p, qtrue);

    if (token[0] != '{')
    {
        return qfalse;
    }

    while ( 1 )
    {

        token = COM_ParseExt(p, qtrue);

        if (Q_stricmp(token, "}") == 0)
        {
            return qtrue;
        }

        if ( !token || token[0] == 0 )
        {
            return qfalse;
        }

        if (token[0] == '{')
        {
            // seven tokens per line, team name and icon, and 5 team member names
            if (!String_Parse(p, &uiInfo.teamList[uiInfo.teamCount].teamName) || !String_Parse(p, &tempStr))
            {
                return qfalse;
            }


            uiInfo.teamList[uiInfo.teamCount].imageName = tempStr;
            uiInfo.teamList[uiInfo.teamCount].teamIcon = trap_R_RegisterShaderNoMip(uiInfo.teamList[uiInfo.teamCount].imageName);
            uiInfo.teamList[uiInfo.teamCount].teamIcon_Metal = trap_R_RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[uiInfo.teamCount].imageName));
            uiInfo.teamList[uiInfo.teamCount].teamIcon_Name = trap_R_RegisterShaderNoMip(va("%s_name", uiInfo.teamList[uiInfo.teamCount].imageName));

            uiInfo.teamList[uiInfo.teamCount].cinematic = -1;

            for (i = 0; i < TEAM_MEMBERS; i++)
            {
                uiInfo.teamList[uiInfo.teamCount].teamMembers[i] = NULL;
                if (!String_Parse(p, &uiInfo.teamList[uiInfo.teamCount].teamMembers[i]))
                {
                    return qfalse;
                }
            }

            Com_Printf("Loaded team %s with team icon %s.\n", uiInfo.teamList[uiInfo.teamCount].teamName, tempStr);
            if (uiInfo.teamCount < MAX_TEAMS)
            {
                uiInfo.teamCount++;
            }
            else
            {
                Com_Printf("Too many teams, last team replaced!\n");
            }
            token = COM_ParseExt(p, qtrue);
            if (token[0] != '}')
            {
                return qfalse;
            }
        }
    }

    return qfalse;
}

static qboolean Character_Parse(char **p)
{
    char *token;
    const char *tempStr;

    token = COM_ParseExt(p, qtrue);

    if (token[0] != '{')
    {
        return qfalse;
    }


    while ( 1 )
    {
        token = COM_ParseExt(p, qtrue);

        if (Q_stricmp(token, "}") == 0)
        {
            return qtrue;
        }

        if ( !token || token[0] == 0 )
        {
            return qfalse;
        }

        if (token[0] == '{')
        {
            // two tokens per line, character name and sex
            if (!String_Parse(p, &uiInfo.characterList[uiInfo.characterCount].name) || !String_Parse(p, &tempStr))
            {
                return qfalse;
            }

            uiInfo.characterList[uiInfo.characterCount].headImage = -1;
            uiInfo.characterList[uiInfo.characterCount].imageName = String_Alloc(va("models/players/heads/%s/icon_default.tga", uiInfo.characterList[uiInfo.characterCount].name));

            if (tempStr && (tempStr[0] == 'f' || tempStr[0] == 'F'))
            {
                uiInfo.characterList[uiInfo.characterCount].female = qtrue;
            }
            else
            {
                uiInfo.characterList[uiInfo.characterCount].female = qfalse;
            }

            //  Com_Printf("Loaded %s character %s.\n", tempStr, uiInfo.characterList[uiInfo.characterCount].name);
            if (uiInfo.characterCount < MAX_HEADS)
            {
                uiInfo.characterCount++;
            }
            else
            {
                Com_Printf("Too many characters, last character replaced!\n");
            }

            token = COM_ParseExt(p, qtrue);
            if (token[0] != '}')
            {
                return qfalse;
            }
        }
    }

    return qfalse;
}


static qboolean Alias_Parse(char **p)
{
    char *token;

    token = COM_ParseExt(p, qtrue);

    if (token[0] != '{')
    {
        return qfalse;
    }

    while ( 1 )
    {
        token = COM_ParseExt(p, qtrue);

        if (Q_stricmp(token, "}") == 0)
        {
            return qtrue;
        }

        if ( !token || token[0] == 0 )
        {
            return qfalse;
        }

        if (token[0] == '{')
        {
            // three tokens per line, character name, bot alias, and preferred action a - all purpose, d - defense, o - offense
            if (!String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].name) || !String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].ai) || !String_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].action))
            {
                return qfalse;
            }

            //  Com_Printf("Loaded character alias %s using character ai %s.\n", uiInfo.aliasList[uiInfo.aliasCount].name, uiInfo.aliasList[uiInfo.aliasCount].ai);
            if (uiInfo.aliasCount < MAX_ALIASES)
            {
                uiInfo.aliasCount++;
            }
            else
            {
                Com_Printf("Too many aliases, last alias replaced!\n");
            }

            token = COM_ParseExt(p, qtrue);
            if (token[0] != '}')
            {
                return qfalse;
            }
        }
    }

    return qfalse;
}



// mode
// 0 - high level parsing
// 1 - team parsing
// 2 - character parsing
static void UI_ParseTeamInfo(const char *teamFile)
{
    char	*token;
    char *p;
    char *buff = NULL;
    //  int mode = 0;

    buff = GetMenuBuffer(teamFile);
    if (!buff)
    {
        return;
    }

    p = buff;

    while ( 1 )
    {
        token = COM_ParseExt( &p, qtrue );
        if( !token || token[0] == 0 || token[0] == '}')
        {
            break;
        }

        if ( Q_stricmp( token, "}" ) == 0 )
        {
            break;
        }

        if (Q_stricmp(token, "teams") == 0)
        {

            if (Team_Parse(&p))
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (Q_stricmp(token, "characters") == 0)
        {
            Character_Parse(&p);
        }

        if (Q_stricmp(token, "aliases") == 0)
        {
            Alias_Parse(&p);
        }

    }

}


static qboolean GameType_Parse(char **p, qboolean join)
{
    char *token;

    token = COM_ParseExt(p, qtrue);

    if (token[0] != '{')
    {
        return qfalse;
    }

    if (join)
    {
        uiInfo.numJoinGameTypes = 0;
    }
    else
    {
        uiInfo.numGameTypes = 0;
    }

    while ( 1 )
    {
        token = COM_ParseExt(p, qtrue);

        if (Q_stricmp(token, "}") == 0)
        {
            return qtrue;
        }

        if ( !token || token[0] == 0 )
        {
            return qfalse;
        }

        if (token[0] == '{')
        {
            // two tokens per line, character name and sex
            if (join)
            {
                if (!String_Parse(p, &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gameType) || !Int_Parse(p, &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gtEnum))
                {
                    return qfalse;
                }
            }
            else
            {
                if (!String_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gameType) || !Int_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gtEnum))
                {
                    return qfalse;
                }
            }

            if (join)
            {
                if (uiInfo.numJoinGameTypes < MAX_GAMETYPES)
                {
                    uiInfo.numJoinGameTypes++;
                }
                else
                {
                    Com_Printf("Too many net game types, last one replace!\n");
                }
            }
            else
            {
                if (uiInfo.numGameTypes < MAX_GAMETYPES)
                {
                    uiInfo.numGameTypes++;
                }
                else
                {
                    Com_Printf("Too many game types, last one replace!\n");
                }
            }

            token = COM_ParseExt(p, qtrue);
            if (token[0] != '}')
            {
                return qfalse;
            }
        }
    }
    return qfalse;
}

static qboolean MapList_Parse(char **p)
{
    char *token;

    token = COM_ParseExt(p, qtrue);

    if (token[0] != '{')
    {
        return qfalse;
    }

    uiInfo.mapCount = 0;

    while ( 1 )
    {
        token = COM_ParseExt(p, qtrue);

        if (Q_stricmp(token, "}") == 0)
        {
            return qtrue;
        }

        if ( !token || token[0] == 0 )
        {
            return qfalse;
        }

        if (token[0] == '{')
        {
            if (!String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapName) || !String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapLoadName)
                    ||!Int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].teamMembers) )
            {
                return qfalse;
            }

            if (!String_Parse(p, &uiInfo.mapList[uiInfo.mapCount].opponentName))
            {
                return qfalse;
            }

            uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

            while (1)
            {
                token = COM_ParseExt(p, qtrue);
                if (token[0] >= '0' && token[0] <= '9')
                {
                    uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << (token[0] - 0x030));
                    if (!Int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].timeToBeat[token[0] - 0x30]))
                    {
                        return qfalse;
                    }
                }
                else
                {
                    break;
                }
            }

            //mapList[mapCount].imageName = String_Alloc(va("levelshots/%s", mapList[mapCount].mapLoadName));
            //if (uiInfo.mapCount == 0) {
            // only load the first cinematic, selection loads the others
            //  uiInfo.mapList[uiInfo.mapCount].cinematic = trap_CIN_PlayCinematic(va("%s.roq",uiInfo.mapList[uiInfo.mapCount].mapLoadName), qfalse, qfalse, qtrue, 0, 0, 0, 0);
            //}
            uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
            uiInfo.mapList[uiInfo.mapCount].levelShot = trap_R_RegisterShaderNoMip(va("levelshots/%s_small", uiInfo.mapList[uiInfo.mapCount].mapLoadName));

            if (uiInfo.mapCount < MAX_MAPS)
            {
                uiInfo.mapCount++;
            }
            else
            {
                Com_Printf("Too many maps, last one replaced!\n");
            }
        }
    }
    return qfalse;
}

static void UI_ParseGameInfo(const char *teamFile)
{
    char	*token;
    char *p;
    char *buff = NULL;
    //	int mode = 0;

    buff = GetMenuBuffer(teamFile);
    if (!buff)
    {
        return;
    }

    p = buff;

    while ( 1 )
    {
        token = COM_ParseExt( &p, qtrue );
        if( !token || token[0] == 0 || token[0] == '}')
        {
            break;
        }

        if ( Q_stricmp( token, "}" ) == 0 )
        {
            break;
        }

        if (Q_stricmp(token, "gametypes") == 0)
        {

            if (GameType_Parse(&p, qfalse))
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (Q_stricmp(token, "joingametypes") == 0)
        {

            if (GameType_Parse(&p, qtrue))
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (Q_stricmp(token, "maps") == 0)
        {
            // start a new menu
            MapList_Parse(&p);
        }

    }
}

static void UI_Pause(qboolean b)
{
    if (b)
    {
        // pause the game and set the ui keycatcher
        trap_Cvar_Set( "cl_paused", "0" );//1
        trap_Key_SetCatcher( KEYCATCH_UI );
    }
    else
    {
        // unpause the game and clear the ui keycatcher
        trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
        trap_Key_ClearStates();
        trap_Cvar_Set( "cl_paused", "0" );
    }
}

/*static int UI_OwnerDraw_Width(int ownerDraw) {
return 0;
}*/

static int UI_PlayCinematic(const char *name, float x, float y, float w, float h)
{
    return trap_CIN_PlayCinematic(name, x, y, w, h, (CIN_loop | CIN_silent));
}

static void UI_StopCinematic(int handle)
{
    if (handle >= 0)
    {
        trap_CIN_StopCinematic(handle);
    }
    else
    {
        handle = abs(handle);
        if (handle == UI_MAPCINEMATIC)
        {
            if (uiInfo.mapList[ui_currentMap.integer].cinematic >= 0)
            {
                trap_CIN_StopCinematic(uiInfo.mapList[ui_currentMap.integer].cinematic);
                uiInfo.mapList[ui_currentMap.integer].cinematic = -1;
            }
        }
        else if (handle == UI_NETMAPCINEMATIC)
        {
            if (uiInfo.serverStatus.currentServerCinematic >= 0)
            {
                trap_CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
                uiInfo.serverStatus.currentServerCinematic = -1;
            }
        }
        else if (handle == UI_CLANCINEMATIC)
        {
            int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
            if (i >= 0 && i < uiInfo.teamCount)
            {
                if (uiInfo.teamList[i].cinematic >= 0)
                {
                    trap_CIN_StopCinematic(uiInfo.teamList[i].cinematic);
                    uiInfo.teamList[i].cinematic = -1;
                }
            }
        }
    }
}

static void UI_DrawCinematic(int handle, float x, float y, float w, float h)
{
    trap_CIN_SetExtents(handle, x, y, w, h);
    trap_CIN_DrawCinematic(handle);
}

static void UI_RunCinematicFrame(int handle)
{
    trap_CIN_RunCinematic(handle);
}
 


void UI_RunExtendedMenuScript(char **args, int parameter )
{
    const char *name;

    //Com_Printf(S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: ui_main.c / UI_RunExtendedMenuScript\n");

    if (String_Parse(args, &name))
    {
        if (Q_stricmp(name, "setPrimaryWeapon") == 0)
        {
            if (!UI_CheckWeapon(parameter))
            {
                trap_S_StartLocalSound( (trap_S_RegisterSound("sound/misc/refuse.wav", qfalse ) ), CHAN_AUTO );
            }
            else
                trap_Cvar_Set("inven_Primary", va("%i", parameter) );
        }
        else if (Q_stricmp(name, "setSecondaryWeapon") == 0)
        {
            trap_Cvar_Set("inven_Secondary", va("%i",parameter) );
        }
        else if ( Q_stricmp(name, "setItem") == 0 )
        {
            char *cvarname;
            int	 Value;

            if ( parameter == ITEM_HELMET )
                cvarname = "inven_Helmet";
            else if ( parameter == ITEM_VEST )
                cvarname = "inven_Vest";
            else if ( parameter == ITEM_SCOPE_SECONDARY  )
                cvarname = "inven_Scope_Secondary";
            else if( parameter == ITEM_GRENADELAUNCHER_SECONDARY  )
                cvarname = "inven_GrenadeLauncher_secondary";
            else if( parameter == ITEM_LASERSIGHT_SECONDARY  )
                cvarname = "inven_LaserSight_secondary";
            else if( parameter == ITEM_BAYONET_SECONDARY  )
                cvarname = "inven_Bayonet_secondary";
            else if( parameter == ITEM_SILENCER_SECONDARY )
                cvarname = "inven_Silencer_secondary";
            else if ( parameter == ITEM_SCOPE  )
                cvarname = "inven_Scope";
            else if( parameter == ITEM_GRENADELAUNCHER  )
                cvarname = "inven_GrenadeLauncher";
            else if( parameter == ITEM_LASERSIGHT  )
                cvarname = "inven_LaserSight";
            else if( parameter == ITEM_BAYONET  )
                cvarname = "inven_Bayonet";
            else if( parameter == ITEM_SILENCER)
                cvarname = "inven_Silencer";
            else if( parameter == ITEM_FLASHLIGHT)
                cvarname = "inven_Flashlight";
            else if( parameter == ITEM_DUCKBILL )
                cvarname =" inven_Duckbill";
            else if( parameter == ITEM_FLASHLIGHT_SECONDARY )
                cvarname =" inven_Duckbill_Secondary";
            else
                cvarname = "inven_error";


            Value = (int)trap_Cvar_VariableValue(cvarname);

            // items only got activate or disable
            if ( Value == 1 ) // if it's activated, disable it
                Value = 0;
            else
                Value = 1;

            trap_Cvar_Set(cvarname, va("%i",Value) );
        }
        else if ( Q_stricmp(name, "raiseCharacter") == 0 )
        {
            if ( parameter <= 0 || parameter > PC_STEALTH )
                return;


            trap_Cmd_ExecuteText( EXEC_APPEND, va( "character %i",parameter ) );
        }

    }
}

/*
=================
UI_Init
=================
*/
void _UI_Init( qboolean inGameLoad )
{
    const char *menuSet;
    int			start;

#ifdef	INIT_PRECACHE
	uiInfo.initDone = qfalse;
#endif

    UI_RegisterCvars();
    UI_InitMemory();

    // cache redundant calulations
    trap_GetGlconfig( &uiInfo.uiDC.glconfig );

    // for 640x480 virtualized screen
    uiInfo.uiDC.yscale = uiInfo.uiDC.glconfig.vidHeight * (1.0/480.0);
    uiInfo.uiDC.xscale = uiInfo.uiDC.glconfig.vidWidth * (1.0/640.0);
    if ( uiInfo.uiDC.glconfig.vidWidth * 480 > uiInfo.uiDC.glconfig.vidHeight * 640 )
    {
        // wide screen
        uiInfo.uiDC.bias = 0.5 * ( uiInfo.uiDC.glconfig.vidWidth - ( uiInfo.uiDC.glconfig.vidHeight * (640.0/480.0) ) );
    }
    else
    {
        // no wide screen
        uiInfo.uiDC.bias = 0;
    } 

    //UI_Load();
    uiInfo.uiDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
    uiInfo.uiDC.setColor = &UI_SetColor;
    uiInfo.uiDC.drawHandlePic = &UI_DrawHandlePic;
    uiInfo.uiDC.drawStretchPic = &trap_R_DrawStretchPic;
    uiInfo.uiDC.drawText = &Text_Paint;
    uiInfo.uiDC.textWidth = &Text_Width;
    uiInfo.uiDC.textHeight = &Text_Height;
    uiInfo.uiDC.registerModel = &trap_R_RegisterModel;
    uiInfo.uiDC.modelBounds = &trap_R_ModelBounds;
    uiInfo.uiDC.fillRect = &UI_FillRect;
    uiInfo.uiDC.drawRect = &_UI_DrawRect;
    uiInfo.uiDC.drawSides = &_UI_DrawSides;
    uiInfo.uiDC.drawTopBottom = &_UI_DrawTopBottom;
    uiInfo.uiDC.clearScene = &trap_R_ClearScene;
    uiInfo.uiDC.drawSides = &_UI_DrawSides;
    uiInfo.uiDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
    uiInfo.uiDC.renderScene = &trap_R_RenderScene;
    uiInfo.uiDC.registerFont = &trap_R_RegisterFont;
    uiInfo.uiDC.ownerDrawItem = &UI_OwnerDraw;
    uiInfo.uiDC.getValue = &UI_GetValue;
    uiInfo.uiDC.ownerDrawVisible = &UI_OwnerDrawVisible;
    uiInfo.uiDC.runScript = &UI_RunMenuScript;
    uiInfo.uiDC.setWeapon = &UI_RunExtendedMenuScript;
    uiInfo.uiDC.getTeamColor = &UI_GetTeamColor;
    uiInfo.uiDC.setCVar = trap_Cvar_Set;
    uiInfo.uiDC.getCVarString = trap_Cvar_VariableStringBuffer;
    uiInfo.uiDC.getCVarValue = trap_Cvar_VariableValue;
    uiInfo.uiDC.drawTextWithCursor = &Text_PaintWithCursor;
    uiInfo.uiDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
    uiInfo.uiDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
    uiInfo.uiDC.startLocalSound = &trap_S_StartLocalSound;
    uiInfo.uiDC.ownerDrawHandleKey = &UI_OwnerDrawHandleKey;
    uiInfo.uiDC.feederCount = &UI_FeederCount;
    uiInfo.uiDC.feederItemImage = &UI_FeederItemImage;
    uiInfo.uiDC.feederItemText = &UI_FeederItemText;
    uiInfo.uiDC.feederSelection = &UI_FeederSelection;
    uiInfo.uiDC.setBinding = &trap_Key_SetBinding;
    uiInfo.uiDC.getBindingBuf = &trap_Key_GetBindingBuf;
    uiInfo.uiDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
    uiInfo.uiDC.executeText = &trap_Cmd_ExecuteText;
    uiInfo.uiDC.Error = &Com_Error;
    uiInfo.uiDC.Print = &Com_Printf;
    uiInfo.uiDC.Pause = &UI_Pause;
    uiInfo.uiDC.ownerDrawWidth = &UI_OwnerDrawWidth;
    uiInfo.uiDC.registerSound = &trap_S_RegisterSound;
    uiInfo.uiDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
    uiInfo.uiDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
    uiInfo.uiDC.playCinematic = &UI_PlayCinematic;
    uiInfo.uiDC.stopCinematic = &UI_StopCinematic;
    uiInfo.uiDC.drawCinematic = &UI_DrawCinematic;
    uiInfo.uiDC.runCinematicFrame = &UI_RunCinematicFrame;

    Init_Display(&uiInfo.uiDC);

    String_Init();

    uiInfo.uiDC.cursor	= trap_R_RegisterShaderNoMip( "menu/art/3_cursor2" );
    uiInfo.uiDC.whiteShader = trap_R_RegisterShaderNoMip( "white" );
	uiInfo.uiDC.loadingShader = trap_R_RegisterShaderNoMip( "ui/assets/menuback_world.tga" );

    AssetCache();

    start = trap_Milliseconds();

    uiInfo.teamCount = 0;
    uiInfo.characterCount = 0;
    uiInfo.aliasCount = 0;
 
    UI_ParseGameInfo("gameinfo.txt"); 

    menuSet = UI_Cvar_VariableString("ui_menuFiles");
    if (menuSet == NULL || menuSet[0] == '\0')
    {
        menuSet = "ui/menus.txt";
    }
  
    UI_LoadMenus(menuSet, qtrue);
    UI_LoadMenus("ui/ingame.txt", qfalse); 

    Menus_CloseAll();

    trap_LAN_LoadCachedServers();
    UI_LoadBestScores(uiInfo.mapList[0].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
 
    // sets defaults for ui temp cvars
    uiInfo.effectsColor = gamecodetoui[(int)trap_Cvar_VariableValue("color")-1]; 
    trap_Cvar_Set("ui_mousePitch", (trap_Cvar_VariableValue("m_pitch") >= 0) ? "0" : "1");

    uiInfo.serverStatus.currentServerCinematic = -1;
    uiInfo.previewMovie = -1;
 
    trap_Cvar_Register(NULL, "debug_protocol", "", 0 ); 
}

/*
=================
_UI_Precache
=================
*/
void UI_RegisterHeadstyle( char *mouthName, char *eyesName, char *headName );
void UI_RegisterPlayerModel( char *modelName );
void UI_RegisterWeapon( int weaponNum ); 

char *ui_headStyles[] = {
	/* seals */
	"bandana_camo", "bandana_camo2", "bandana_dark",  "bandana_red",  "band_green",  "band_brown", 
	"band_dark",  "band_red",  "barret_brown",  "barret_dark", "barret_green",   "cap_seal_eagle", 
	"cap_seal_q3",  "sealhat_desert",  "sealhat_jungle",  "sealhat_urban",  "sealhat2_brown", 
	"sealhat2_green", "sealhat2_dark",  "cap_seal2_dark", "cap_seal2_green",  "cap_seal2_white", 
	"Cap Dark", "cap_tango_dark", 
	/* tangos */
	"cap_tango_desert", "cap_tango_jungle",   "cap_tango_red",  "tangohat_brown",  "tangohat_dark",
	"tangohat_green", "turban_brown",   "turban_grey", "turban_pattern",  "turban2_brown",
	"turban2_grey",  "turban2_pattern", "ricehat_dark",  "ricehat_green",  "cap_tango2_dark", 
	"cap_tango2_green",  "cap_tango2_white", 
	/* delimiter */
	""
};
char *ui_eyeStyles[] = {
	/* seals */
	"goggles",  "glasses_seal",  "glasses_seal2",
	/* tangos */
	"glasses_tango",  "glasses_tango2",  "piercings", 
	/* delimiter */
	""
};
char *ui_mouthStyles[] = {
	/* seals */
	 "cigar", "cigarette",
	/* tangos */
	"joint", "mouthcloth", 
	/* delimiter */
	""
};

void _UI_Precache( qboolean skipPrecache )
{
#ifdef INIT_PRECACHE  
	int i = 0;
	int numModels = trap_Key_GetModelCount();
	int start;

	if ( skipPrecache == qfalse ){
		// update screen ( display loadingscreen )
	 	trap_UpdateScreen(); 

		start = trap_Milliseconds();
		
		// register weapons
		for ( i = 1 ; i < WP_NUM_WEAPONS ; i++ ) { 
			UI_RegisterWeapon( i );  
		}  
		// register playermodels
		UI_RegisterPlayerModel( "s_medium" );
		UI_RegisterPlayerModel( "t_medium" );

		// register headstuffs 
		i = 0;
		while ( strlen( ui_mouthStyles[i] ) > 0 )
		{
			UI_RegisterHeadstyle( ui_mouthStyles[i], NULL, NULL );
			i++;
		}  

		i = 0;
		while ( strlen( ui_eyeStyles[i] ) > 0 )
		{
			UI_RegisterHeadstyle( NULL, ui_eyeStyles[i], NULL );
			i++;
		}  

		i = 0;
		while ( strlen( ui_headStyles[i] ) > 0 )
		{
			UI_RegisterHeadstyle( NULL, NULL, ui_headStyles[i] );
			i++;
		} 
 	}
#endif
	uiInfo.initDone = qtrue;
	
}

/*
=================
UI_KeyEvent
=================
*/
void _UI_KeyEvent( int key, qboolean down )
{
    if ( key == K_MOUSE1 && down )
        uiInfo.uiDC.cursor_lbutton_dn = qtrue;
    else if ( key == K_MOUSE1 )
        uiInfo.uiDC.cursor_lbutton_dn = qfalse;

    if (Menu_Count() > 0)
    {
        menuDef_t *menu = Menu_GetFocused();

        if (menu)
        {
            if (key == K_ESCAPE && down && !Menus_AnyFullScreenVisible())
            {
                Menus_CloseAll();
            }
            else
            {
                Menu_HandleKey(menu, key, down );
            }
        }
        else
        {
            trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
            trap_Key_ClearStates();
            trap_Cvar_Set( "cl_paused", "0" );
        }
    }
}

/*
=================
UI_MouseEvent
=================
*/
void _UI_MouseEvent( int dx, int dy )
{
    // update mouse screen position
    uiInfo.uiDC.cursorx += dx;
    if (uiInfo.uiDC.cursorx < 0)
        uiInfo.uiDC.cursorx = 0;
    else if (uiInfo.uiDC.cursorx > SCREEN_WIDTH)
        uiInfo.uiDC.cursorx = SCREEN_WIDTH;

    uiInfo.uiDC.cursory += dy;
    if (uiInfo.uiDC.cursory < 0)
        uiInfo.uiDC.cursory = 0;
    else if (uiInfo.uiDC.cursory > SCREEN_HEIGHT)
        uiInfo.uiDC.cursory = SCREEN_HEIGHT;

    if (Menu_Count() > 0)
        Display_MouseMove(NULL, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
}

void UI_LoadNonIngame()
{
    const char *menuSet = UI_Cvar_VariableString("ui_menuFiles");
    if (menuSet == NULL || menuSet[0] == '\0')
    {
        menuSet = "ui/menus.txt";
    }
    UI_LoadMenus(menuSet, qfalse);
    uiInfo.inGameLoad = qfalse;
}

void _UI_SetActiveMenu( uiMenuCommand_t menu )
{
    char buf[256]; 

#ifdef INIT_PRECACHE	
	if (uiInfo.initDone == qfalse)
	{
		trap_Key_SetCatcher( KEYCATCH_UI ); 
		return;
	}
#endif

    // this should be the ONLY way the menu system is brought up
    // enusure minumum menu data is cached
    if (Menu_Count() > 0)
    { 
        vec3_t v;
        v[0] = v[1] = v[2] = 0;
        switch ( menu )
        {
        case UIMENU_NONE:
            trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
            trap_Key_ClearStates();
            trap_Cvar_Set( "cl_paused", "0" );
            Menus_CloseAll();

            return;
        case UIMENU_MAIN:
			uiInfo.initDone = qtrue;
            trap_Key_SetCatcher( KEYCATCH_UI );

            //trap_S_StartLocalSound( trap_S_RegisterSound("sound/misc/menu_background.wav", qfalse) , CHAN_LOCAL_SOUND );
            trap_S_StartBackgroundTrack("sound/misc/nsmt.wav", NULL);
            if (uiInfo.inGameLoad)
            {
                UI_LoadNonIngame();
            }
            Menus_CloseAll();
            Menus_ActivateByName("main");
            trap_Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));
            if (strlen(buf))
            {
                if (!ui_singlePlayerActive.integer)
                {
                    Menus_ActivateByName("error_popmenu");
                }
                else
                {
                    trap_Cvar_Set("com_errorMessage", "");
                }
            }
            return;
        case UIMENU_TEAM:
            trap_Key_SetCatcher( KEYCATCH_UI );
            Menus_ActivateByName("team");
            return;
        case UIMENU_NEED_CD:
            // no cd check in TA
            //trap_Key_SetCatcher( KEYCATCH_UI );
            //Menus_ActivateByName("needcd");
            //UI_ConfirmMenu( "Insert the CD", NULL, NeedCDAction );
            return;
        case UIMENU_BAD_CD_KEY:
            // no cd check in TA
            //trap_Key_SetCatcher( KEYCATCH_UI );
            //Menus_ActivateByName("badcd");
            //UI_ConfirmMenu( "Bad CD Key", NULL, NeedCDKeyAction );
            return;
        case UIMENU_POSTGAME:
            //trap_Cvar_Set( "sv_killserver", "1" );
            trap_Key_SetCatcher( KEYCATCH_UI );
            if (uiInfo.inGameLoad)
            {
                UI_LoadNonIngame();
            } 
            Menus_CloseAll();
            Menus_ActivateByName("endofgame");
            //UI_ConfirmMenu( "Bad CD Key", NULL, NeedCDKeyAction );
            return;
        case UIMENU_INGAME:
			uiInfo.initDone = qtrue;
            trap_Cvar_Set( "cl_paused", "0" ); //1
            trap_Key_SetCatcher( KEYCATCH_UI );
            UI_BuildPlayerList();
            Menus_CloseAll();
            Menus_ActivateByName("ingame");
            return;
        }
    }
}

qboolean _UI_IsFullscreen( void )
{
    return Menus_AnyFullScreenVisible();
}



static connstate_t	lastConnState;
static char			lastLoadingText[MAX_INFO_VALUE];

static void UI_ReadableSize ( char *buf, int bufsize, int value )
{
    if (value > 1024*1024*1024 )
    { // gigs
        Com_sprintf( buf, bufsize, "%d", value / (1024*1024*1024) );
        Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d GB",
                     (value % (1024*1024*1024))*100 / (1024*1024*1024) );
    }
    else if (value > 1024*1024 )
    { // megs
        Com_sprintf( buf, bufsize, "%d", value / (1024*1024) );
        Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d MB",
                     (value % (1024*1024))*100 / (1024*1024) );
    }
    else if (value > 1024 )
    { // kilos
        Com_sprintf( buf, bufsize, "%d KB", value / 1024 );
    }
    else
    { // bytes
        Com_sprintf( buf, bufsize, "%d bytes", value );
    }
}

// Assumes time is in msec
static void UI_PrintTime ( char *buf, int bufsize, int time )
{
    time /= 1000;  // change to seconds

    if (time > 3600)
    { // in the hours range
        Com_sprintf( buf, bufsize, "%d hr %d min", time / 3600, (time % 3600) / 60 );
    }
    else if (time > 60)
    { // mins
        Com_sprintf( buf, bufsize, "%d min %d sec", time / 60, time % 60 );
    }
    else
    { // secs
        Com_sprintf( buf, bufsize, "%d sec", time );
    }
}

void Text_PaintCenter(float x, float y, float scale, vec4_t color, const char *text, float adjust)
{
    int len = Text_Width(text, scale, 0);
    Text_Paint(x - len / 2, y, scale, color, text, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
}


static void UI_DisplayDownloadInfo( const char *downloadName, float centerPoint, float yStart, float scale )
{
    static char dlText[]	= "Downloading:";
    static char etaText[]	= "Estimated time left:";
    static char xferText[]	= "Transfer rate:";

    int downloadSize, downloadCount, downloadTime;
    char dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
    int xferRate;
    int leftWidth;
    const char *s;

    downloadSize = trap_Cvar_VariableValue( "cl_downloadSize" );
    downloadCount = trap_Cvar_VariableValue( "cl_downloadCount" );
    downloadTime = trap_Cvar_VariableValue( "cl_downloadTime" );

    leftWidth = 320;

    UI_SetColor(colorWhite);
    Text_PaintCenter(centerPoint, yStart + 112, scale, colorWhite, dlText, 0);
    Text_PaintCenter(centerPoint, yStart + 144, scale, colorWhite, etaText, 0);
    Text_PaintCenter(centerPoint, yStart + 208, scale, colorWhite, xferText, 0);

    if (downloadSize > 0)
    {
        s = va( "%s (%d%%)", downloadName, downloadCount * 100 / downloadSize );
    }
    else
    {
        s = downloadName;
    }

    Text_Paint(centerPoint, yStart + 244, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);

    UI_ReadableSize( dlSizeBuf,		sizeof dlSizeBuf,		downloadCount );
    UI_ReadableSize( totalSizeBuf,	sizeof totalSizeBuf,	downloadSize );

    if (downloadCount < 4096 || !downloadTime)
    {
        Text_PaintCenter(leftWidth, 160, 0.6f, colorWhite, "estimating", 0);
        Text_PaintCenter(leftWidth, 192, 0.6f, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
    }
    else
    {
        if ((uiInfo.uiDC.realTime - downloadTime) / 1000)
        {
            xferRate = downloadCount / ((uiInfo.uiDC.realTime - downloadTime) / 1000);
        }
        else
        {
            xferRate = 0;
        }
        UI_ReadableSize( xferRateBuf, sizeof xferRateBuf, xferRate );

        // Extrapolate estimated completion time
        if (downloadSize && xferRate)
        {
            int n = downloadSize / xferRate; // estimated time for entire d/l in secs

            // We do it in K (/1024) because we'd overflow around 4MB
            UI_PrintTime ( dlTimeBuf, sizeof dlTimeBuf,
                           (n - (((downloadCount/1024) * n) / (downloadSize/1024))) * 1000);

            Text_PaintCenter(leftWidth, 160, 0.6f, colorWhite, dlTimeBuf, 0);
            Text_PaintCenter(leftWidth, 192, 0.6f, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
        }
        else
        {
            Text_PaintCenter(leftWidth, 160, 0.6f, colorWhite, "estimating", 0);
            if (downloadSize)
            {
                Text_PaintCenter(leftWidth, 160, 0.6f, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
            }
            else
            {
                Text_PaintCenter(leftWidth, 160, 0.6f, colorWhite, va("(%s copied)", dlSizeBuf), 0);
            }
        }

        if (xferRate)
        {
            Text_PaintCenter(leftWidth, 160, 0.6f, colorWhite, va("%s/Sec", xferRateBuf), 0);
        }
    }
}

/*
========================
UI_DrawConnectScreen
 
This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen( qboolean overlay )
{
    char			*s;
    uiClientState_t	cstate;
    char			info[MAX_INFO_VALUE];
    char text[256];
    float centerPoint, yStart, scale;

    menuDef_t *menu = Menus_FindByName("Connect");


    if ( !overlay && menu )
    {
        Menu_Paint(menu, qtrue);
    }

    if (!overlay)
    {
        centerPoint = 320;
        yStart = 130;
        scale = 0.5f;
    }
    else
    {
        centerPoint = 320;
        yStart = 32;
        scale = 0.6f;
        return;
    }

    // see what information we should display
    trap_GetClientState( &cstate );

    info[0] = '\0';
    if( trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) ) )
        Text_PaintCenter(centerPoint, yStart, scale, colorWhite, va( "Loading %s", Info_ValueForKey( info, "mapname" )), 0);

    if (!Q_stricmp(cstate.servername,"localhost"))
    {
	//		Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite, va("Starting up..."), ITEM_TEXTSTYLE_SHADOWEDMORE);
    }
    else
    {
        strcpy(text, va("Connecting to %s", cstate.servername));
        Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite,text , ITEM_TEXTSTYLE_SHADOWEDMORE);
    }

    //UI_DrawProportionalString( 320, 96, "Press Esc to abort", UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, menu_text_color );

    // display global MOTD at bottom
    Text_PaintCenter(centerPoint, 600, scale, colorWhite, Info_ValueForKey( cstate.updateInfoString, "motd" ), 0);
    // print any server info (server full, bad version, etc)
    if ( cstate.connState < CA_CONNECTED )
        Text_PaintCenter(centerPoint, yStart + 176, scale, colorWhite, cstate.messageString, 0);

    if ( lastConnState > cstate.connState )
        lastLoadingText[0] = '\0';

    lastConnState = cstate.connState;

    switch ( cstate.connState )
    {
    case CA_CONNECTING:
        s = va("Awaiting connection...%i", cstate.connectPacketCount);
        break;
    case CA_CHALLENGING:
        s = va("Awaiting challenge...%i", cstate.connectPacketCount);
        break;
    case CA_CONNECTED:
        {
            char downloadName[MAX_INFO_VALUE];

            trap_Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof(downloadName) );
            if (*downloadName)
            {
                UI_DisplayDownloadInfo( downloadName, centerPoint, yStart, scale );
                return;
            }
        }
        s = "Awaiting gamestate...";
        break;
    case CA_LOADING:
        return;
    case CA_PRIMED:
        return;
    default:
        return;
    }


    if (Q_stricmp(cstate.servername,"localhost")) {
        Text_PaintCenter(centerPoint, yStart + 80, scale, colorWhite, s, 0);
    }

    // password required / connection rejected information goes here
}



/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars( void )
{
    int			i;
    cvarTable_t	*cv;

    for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ )
    {
        trap_Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
    } 
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars( void )
{
    int			i;
    cvarTable_t	*cv;

    for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ )
    {
        trap_Cvar_Update( cv->vmCvar );
    } 
}


/*
=================
ArenaServers_StopRefresh
=================
*/
static void UI_StopServerRefresh( void )
{
    int count;

    if (!uiInfo.serverStatus.refreshActive)
    {
        // not currently refreshing
        return;
    }
    uiInfo.serverStatus.refreshActive = qfalse;
    Com_Printf("%d servers listed in browser with %d players.\n",
               uiInfo.serverStatus.numDisplayServers,
               uiInfo.serverStatus.numPlayersOnServers);
    count = trap_LAN_GetServerCount(ui_netSource.integer);
    if (count - uiInfo.serverStatus.numDisplayServers > 0)
    {
        Com_Printf("%d servers not listed due to packet loss or pings higher than %d\n",
                   count - uiInfo.serverStatus.numDisplayServers,
                   (int) trap_Cvar_VariableValue("cl_maxPing"));
    }

}
 
/*
=================
UI_DoServerRefresh
=================
*/
static void UI_DoServerRefresh( void )
{
    qboolean wait = qfalse;

    if (!uiInfo.serverStatus.refreshActive)
    {
        return;
    }
    if (ui_netSource.integer != AS_FAVORITES)
    {
        if (ui_netSource.integer == AS_LOCAL)
        {
            if (!trap_LAN_GetServerCount(ui_netSource.integer))
            {
                wait = qtrue;
            }
        }
        else
        {
            if (trap_LAN_GetServerCount(ui_netSource.integer) < 0)
            {
                wait = qtrue;
            }
        }
    }

    if (uiInfo.uiDC.realTime < uiInfo.serverStatus.refreshtime)
    {
        if (wait)
        {
            return;
        }
    }

    // if still trying to retrieve pings
    if (trap_LAN_UpdateVisiblePings(ui_netSource.integer))
    {
        uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
    }
    else if (!wait)
    {
        // get the last servers in the list
        UI_BuildServerDisplayList(2);
        // stop the refresh
        UI_StopServerRefresh();
    }
    //
    UI_BuildServerDisplayList(qfalse);
}

/*
=================
UI_StartServerRefresh
=================
*/
static void UI_StartServerRefresh(qboolean full)
{
    int		i;
    char	*ptr;

    qtime_t q;
    trap_RealTime(&q);
    trap_Cvar_Set( va("ui_lastServerRefresh_%i", ui_netSource.integer), va("%s-%i, %i at %i:%i", MonthAbbrev[q.tm_mon],q.tm_mday, 1900+q.tm_year,q.tm_hour,q.tm_min));

    if (!full)
    {
        UI_UpdatePendingPings();
        return;
    }

    uiInfo.serverStatus.refreshActive = qtrue;
    uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiDC.realTime + 1000;
    // clear number of displayed servers
    uiInfo.serverStatus.numDisplayServers = 0;
    uiInfo.serverStatus.numPlayersOnServers = 0;
    // mark all servers as visible so we store ping updates for them
    trap_LAN_MarkServerVisible(ui_netSource.integer, -1, qtrue);
    // reset all the pings
    trap_LAN_ResetPings(ui_netSource.integer);
    //
    if( ui_netSource.integer == AS_LOCAL )
    {
        trap_Cmd_ExecuteText( EXEC_NOW, "localservers\n" );
        uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 1000;
        return;
    }

    uiInfo.serverStatus.refreshtime = uiInfo.uiDC.realTime + 5000;
    if( ui_netSource.integer == AS_GLOBAL || ui_netSource.integer == AS_MPLAYER )
    {
        if( ui_netSource.integer == AS_GLOBAL )
        {
            i = 0;
        }
        else
        {
            i = 1;
        }

        ptr = UI_Cvar_VariableString("debug_protocol");
        if (strlen(ptr))
        {
            trap_Cmd_ExecuteText( EXEC_NOW, va( "globalservers %d %s full empty\n", i, ptr));
        }
        else
        {
            trap_Cmd_ExecuteText( EXEC_NOW, va( "globalservers %d %d full empty\n", i, (int)trap_Cvar_VariableValue( "protocol" ) ) );
        }
    }
}

