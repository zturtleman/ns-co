// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../ui/ui_shared.h"
#ifdef MISSIONPACK
extern menuDef_t *menuScoreboard;
#endif


void CG_TargetCommand_f( void ) {
    int		targetNum;
    char	test[4];

    targetNum = CG_CrosshairPlayer();
    if (!targetNum ) {
        return;
    }

    trap_Argv( 1, test, 4 );
    trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}


void CG_WriteRadarInfoToBRF_f( void )
{
    char filename[128];
    fileHandle_t	brfFile;
    char		string[512];
    int	i;

    if ( !cg.cheatsEnabled )
    {
        CG_Printf("Please start your game with 'devmap' to use this command.\n");
        return;
    }
    Com_sprintf(filename, sizeof( filename), "briefing/map_%s.brf", cgs.cleanMapName );

    trap_FS_FOpenFile( filename, &brfFile, FS_APPEND_SYNC );

    Com_sprintf( string, sizeof(string), "[radar]\n" );
    trap_FS_Write( string, strlen( string ), brfFile );

    for ( i = 0; i < cg.radarNumObjects; i++ )
    {
        Com_sprintf( string, sizeof(string), "%c %f %f %f\n", cg.radarObjects[i].type, cg.radarObjects[i].origin[0], cg.radarObjects[i].origin[1], cg.radarObjects[i].origin[2] );
        trap_FS_Write( string, strlen( string ), brfFile );
    }

    // todo
    //  add radar entity writeout
    Com_sprintf( string, sizeof(string), "[end]\n$EOF" );
    trap_FS_Write( string, strlen( string ), brfFile );

    CG_Printf("Wrote %i radar entities to %s.\n", cg.radarNumObjects, filename );

}

/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
    trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
    trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
    CG_Printf ("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
               (int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2],
               (int)cg.refdefViewAngles[YAW]);
}


static void CG_ScoresDown_f( void ) {

#ifdef MISSIONPACK
    CG_BuildSpectatorString();
#endif
    if ( cg.scoresRequestTime + 2000 < cg.time ) {
        // the scores are more than two seconds out of data,
        // so request new ones
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand( "score" );

        // leave the current scores up if they were already
        if ( !cg.showScores ) {
            cg.showScores = qtrue;
        }
    } else {
        // show the cached contents even if they just pressed if it
        // is within two seconds
        cg.showScores = qtrue;
    }
}

static void CG_ScoresUp_f( void ) {
    if ( cg.showScores ) {
        cg.showScores = qfalse;
        cg.scoreFadeTime = cg.time;
    }
}

static void CG_IronSightDown_f( void ) {
    // BLUTENGEL: disabled ironsights
    return;
    if (cg.ns_ironsightState == IS_NONE)
    {
        cg.ns_ironsightState = IS_PUTUP;

        if ( BG_IsRifle( cg.snap->ps.weapon ) )
            cg.ns_ironsightTimer = cg.time + IS_RIFLE_TIME;
        else
            cg.ns_ironsightTimer = cg.time + IS_TIME;

        cg.ns_ironsightDeactivate = qfalse;
    }
}
static void CG_IronSightUp_f( void ) {
    // BLUTENGEL: disabled ironsights
    return;
    cg.ns_ironsightDeactivate = qtrue;
}

static void CG_ReloadDown_f( void ) {
    if ( cg.scoresRequestTime + 500 < cg.time ) {
        // the scores are more than two seconds out of data,
        // so request new ones
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand( "reload\n" );
    }
}

static void CG_ReloadUp_f( void ) {
    cg.scoresRequestTime = 0;
}


static void CG_MapDown_f( void ) {
    if ( !cg.mapVisible ) {
        //	cg.mapVisible = qtrue;
    }
}

static void CG_MapUp_f( void ) {
    cg.mapVisible = qfalse;
}


static void CG_WeaponMode1Up_f( void ) {
    cg.scoresRequestTime = 0;
}

static void CG_WeaponMode1_f( void ) {
    if ( cg.scoresRequestTime + 500 < cg.time ) {
        // the scores are more than two seconds out of data,
        // so request new ones
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand( "weaponmode1" );
    }
}

static void CG_WeaponMode2Up_f( void ) {
    cg.scoresRequestTime = 0;
}

static void CG_WeaponMode2_f( void ) {
    if ( cg.scoresRequestTime + 500 < cg.time ) {
        // the scores are more than two seconds out of data,
        // so request new ones
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand( "weaponmode2" );
    }
}

static void CG_WeaponMode3Up_f( void ) {
    cg.scoresRequestTime = 0;
}

static void CG_WeaponMode3_f( void ) {
    if ( cg.scoresRequestTime + 500 < cg.time ) {
        cg.scoresRequestTime = cg.time;
        trap_SendClientCommand( "weaponmode3" );
    }
}

#ifdef MISSIONPACK
extern menuDef_t *menuScoreboard;
void Menu_Reset();			// FIXME: add to right include file

extern /**static*/ int strPoolIndex;
#define HASH_TABLE_SIZE 2048

extern /*static*/ char strPool[STRING_POOL_SIZE];
typedef struct stringDef_s {
    struct stringDef_s *next;
    const char *str;
} stringDef_t;
extern int strHandleCount;
extern stringDef_t *strHandle[HASH_TABLE_SIZE];
#include "../ui/ui_shared.h"
// display context for new ui stuff
extern displayContextDef_t cgDC;
//void Item_SetupKeywordHash();
//void Menu_SetupKeywordHash();

static void CG_LoadHud_f( void) {
    char buff[1024];
    const char *hudSet;


    int i;

    memset(buff, 0, sizeof(buff));

    for (i = 0; i < HASH_TABLE_SIZE; i++) {
        strHandle[i] = 0;
    }
    strHandleCount = 0;
    strPoolIndex = 0;
    //	menuCount = 0;
    //	openMenuCount = 0;
    UI_InitMemory();
    //	Item_SetupKeywordHash();
    //	Menu_SetupKeywordHash();
    if ( cgDC.getBindingBuf) {
        Controls_GetConfig();
    }

    //	String_Init();
    Menu_Reset();

    trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
    hudSet = buff;
    if (hudSet[0] == '\0') {
        hudSet = "ui/hud.txt";
    }

    CG_LoadMenus(hudSet);
    menuScoreboard = NULL;
}


static void CG_scrollScoresDown_f( void) {
    if (menuScoreboard && cg.scoreBoardShowing) {
        Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qtrue);
        Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qtrue);
        Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qtrue);
    }
}


static void CG_scrollScoresUp_f( void) {
    if (menuScoreboard && cg.scoreBoardShowing) {
        Menu_ScrollFeeder(menuScoreboard, FEEDER_SCOREBOARD, qfalse);
        Menu_ScrollFeeder(menuScoreboard, FEEDER_REDTEAM_LIST, qfalse);
        Menu_ScrollFeeder(menuScoreboard, FEEDER_BLUETEAM_LIST, qfalse);
    }
}


static void CG_spWin_f( void) {
    trap_Cvar_Set("cg_cameraOrbit", "2");
    trap_Cvar_Set("cg_cameraOrbitDelay", "35");
    trap_Cvar_Set("cg_thirdPerson", "1");
    trap_Cvar_Set("cg_thirdPersonAngle", "0");
    trap_Cvar_Set("cg_thirdPersonRange", "100");
    //trap_S_StartLocalSound(cgs.media.winnerSound, CHAN_ANNOUNCER);
    CG_CenterPrint("YOU WIN!", SCREEN_HEIGHT * .30, 0);
}

static void CG_spLose_f( void) {
    trap_Cvar_Set("cg_cameraOrbit", "2");
    trap_Cvar_Set("cg_cameraOrbitDelay", "35");
    trap_Cvar_Set("cg_thirdPerson", "1");
    trap_Cvar_Set("cg_thirdPersonAngle", "0");
    trap_Cvar_Set("cg_thirdPersonRange", "100");
    //trap_S_StartLocalSound(cgs.media.loserSound, CHAN_ANNOUNCER);
    CG_CenterPrint("YOU LOSE...", SCREEN_HEIGHT * .30, 0);
}

#endif

static void CG_TellTarget_f( void ) {
    int		clientNum;
    char	command[128];
    char	message[128];

    clientNum = CG_CrosshairPlayer();
    if ( clientNum == -1 ) {
        return;
    }

    trap_Args( message, 128 );
    Com_sprintf( command, 128, "tell %i %s", clientNum, message );
    trap_SendClientCommand( command );
}


// ASS U ME's enumeration order as far as task specific orders, OFFENSE is zero, CAMP is last
//


/*
==================
CG_TeamMenu_f
==================

static void CG_TeamMenu_f( void ) {
if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
CG_EventHandling(CGAME_EVENT_NONE);
trap_Key_SetCatcher(0);
} else {
CG_EventHandling(CGAME_EVENT_TEAMMENU);
//trap_Key_SetCatcher(KEYCATCH_CGAME);
}
}

*/

/*
==================
CG_EditHud_f
==================

static void CG_EditHud_f( void ) {
//cls.keyCatchers ^= KEYCATCH_CGAME;
//VM_Call (cgvm, CG_EVENT_HANDLING, (cls.keyCatchers & KEYCATCH_CGAME) ? CGAME_EVENT_EDITHUD : CGAME_EVENT_NONE);
}

*/



/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
    if (cg_cameraOrbit.value != 0) {
        trap_Cvar_Set ("cg_cameraOrbit", "0");
        trap_Cvar_Set("cg_thirdPerson", "0");
    } else {
        trap_Cvar_Set("cg_cameraOrbit", "5");
        trap_Cvar_Set("cg_thirdPerson", "1");
        trap_Cvar_Set("cg_thirdPersonAngle", "0");
        trap_Cvar_Set("cg_thirdPersonRange", "100");
    }
}
void CG_NewbieMessage( const char *str, int y, float charHeight );
void CG_MWheel_f( int prev );
extern vmCvar_t cg_newbeeHeight;
qboolean CG_CheckForCheats( void );

static void CG_QuitHud_f( void ) {
    CG_EventHandling(CGAME_EVENT_NONE);
    trap_Key_SetCatcher(0);
}
extern char SealBriefing[ 128 ][ 128 ]; // max chars per line
extern int	sealBriefingLines;

static void CG_ConsoleTestCmd( void ) {
    //	vec3_t forward;
    //	trace_t trace;
    //	int value;

    //trap_Cvar_Set("r_vertexlight", "0" );

    /*
    if ( sealBriefingLines > 0 ) {

    for ( value = 0; value <= sealBriefingLines; value ++ )
    {
    CG_Printf( SealBriefing[value] );//ITEM_TEXTSTYLE_SHADOWED);
    }
    }*/
    /*
    AngleVectors( cg.refdefViewAngles, forward,NULL,NULL );

    VectorMA( cg.snap->ps.origin, 10, forward, end );


    trap_CM_BoxTrace( &trace, cg.snap->ps.origin, end, NULL, NULL, 0, CONTENTS_SOLID );

    CG_Printf("Origin: %s. TR: %f %i %i\n", vtos( cg.snap->ps.origin ), trace.fraction,trace.entityNum,trace.surfaceFlags  );

    VectorCopy( cg.snap->ps.origin, cgs.mi_helpers[0] );*/
    /*
    CG_ParseHelpFile();
    CG_NewbieMessage("Loaded Newbee Messages.\n123456789012345678901234567890\n", SCREEN_HEIGHT *0.5, cg_newbeeHeight.value );

    //	CG_SpawnBloodParticle( cg.snap->ps.origin, cg.snap->ps.viewangles , 100, 0.1, 4+random()*2, 0.7,0.2,0.2,1.0f,qfalse );

    //	CG_SpawnEffect( cg.snap->ps.origin );
    //	CG_GoChase();
    */



}
extern vmCvar_t cg_qcmd_close;

static void CG_ViewQCmd_f( void ) {
    if ( cg.viewCmd )
    {
        cg.viewCmd = qfalse;
        trap_SendConsoleCommand( va("%s ;", cg_qcmd_close.string ) );
    }
    else
        cg.viewCmd = qtrue;
}
static void CG_ViewMissionInfo_f( void ) {
    if ( cg.viewMissionInfo )
        cg.viewMissionInfo = qfalse;
    else
        cg.viewMissionInfo = qtrue;
}
static void CG_EditRadar_f( void ) {
    //	CG_Printf("To set a new position for the Q-Command Menu press your Left Mouse Button.\n");
    if ( !cg.cheatsEnabled )
    {
        CG_Printf("Please start your game with 'devmap' to use this command.\n");
        return;
    }

    if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
        CG_EventHandling(CGAME_EVENT_NONE);
        trap_Key_SetCatcher(0);
    } else {
        CG_EventHandling(CGAME_EVENT_EDITRADAR);
        trap_Key_SetCatcher(KEYCATCH_CGAME);
    }


}
static void CG_EditHud_f( void ) {
    CG_Printf("Press your Left Mouse Button to set a new position for your current HUD.\nPress your Right Mouse Button to Quit.\n");

    if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
        CG_EventHandling(CGAME_EVENT_NONE);
        trap_Key_SetCatcher(0);
    } else {
        CG_EventHandling(CGAME_EVENT_EDITHUD);
        trap_Key_SetCatcher(KEYCATCH_CGAME);
    }
}
static void CG_EditRadarPos_f( void ) {
    CG_Printf("Press your Left Mouse Button to set a new position for Radar.\nPress your Right Mouse Button to Quit.\n");

    if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
        CG_EventHandling(CGAME_EVENT_NONE);
        trap_Key_SetCatcher(0);
    } else {
        CG_EventHandling(CGAME_EVENT_EDITRADARPOS);
        trap_Key_SetCatcher(KEYCATCH_CGAME);
    }
}

static void CG_EditQCmd_f( void ) {
    CG_Printf("To set a new position for the Q-Command Menu press your Left Mouse Button.\n");

    if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
        CG_EventHandling(CGAME_EVENT_NONE);
        trap_Key_SetCatcher(0);
    } else {
        CG_EventHandling(CGAME_EVENT_EDITQCMD);
        trap_Key_SetCatcher(KEYCATCH_CGAME);
    }
}
static void CG_RadioMenu_f( void ) {
    if ( cgs.gametype < GT_TEAM )
        return;

    if (trap_Key_GetCatcher() & KEYCATCH_CGAME) {
        CG_EventHandling(CGAME_EVENT_NONE);
        trap_Key_SetCatcher(0);
    } else {
        CG_EventHandling(CGAME_EVENT_RADIOMENU);
        trap_Key_SetCatcher(KEYCATCH_CGAME);
    }


}
// Navy Seals ++
static void CG_MWhellup_f ( void ) {
    CG_MWheel_f( 0 );
}
static void CG_MWhelldn_f ( void ) {
    CG_MWheel_f( 1 );
}

void CG_UpdateLooks_f ( void )
{
    playerState_t *ps;
    char var[128],model[64],skin[64];

    ps = &cg.snap->ps;

    if ( ps->persistant[PERS_TEAM] == TEAM_RED )
    {
        // update looks
        trap_Cvar_VariableStringBuffer("ui_s_e_eyes", var , sizeof(var ) );
        trap_Cvar_Set("e_eyes", var );
        trap_Cvar_VariableStringBuffer("ui_s_e_head", var , sizeof(var ) );
        trap_Cvar_Set("e_head", var );
        trap_Cvar_VariableStringBuffer("ui_s_e_mouth", var , sizeof(var ) );
        trap_Cvar_Set("e_mouth", var );

        // model
        trap_Cvar_VariableStringBuffer("ui_s_model", model , sizeof( model ) );
        trap_Cvar_VariableStringBuffer("ui_s_skin", skin , sizeof( skin ) );

        trap_Cvar_Set( "model", va("%s/%s",model,skin) );
    }
    else if ( ps->persistant[PERS_TEAM] == TEAM_BLUE )
    {
        // update looks
        trap_Cvar_VariableStringBuffer("ui_t_e_eyes", var , sizeof(var ) );
        trap_Cvar_Set("e_eyes", var );
        trap_Cvar_VariableStringBuffer("ui_t_e_head", var , sizeof(var ) );
        trap_Cvar_Set("e_head", var );
        trap_Cvar_VariableStringBuffer("ui_t_e_mouth", var , sizeof(var ) );
        trap_Cvar_Set("e_mouth", var );

        // model
        trap_Cvar_VariableStringBuffer("ui_t_model", model , sizeof( model ) );
        trap_Cvar_VariableStringBuffer("ui_t_skin", skin , sizeof( skin ) );

        trap_Cvar_Set( "model", va("%s/%s",model,skin) );
    }
}
static void CG_NSEcho_f ( void )
{
    char	message[256];

    trap_Args( message, sizeof(message) );
    CG_Printf( va("%s\n",message) );
}

static void CG_Alias_f ( void )
{
    const char *token = CG_Argv(0);

    if ( !Q_stricmp( token,"+alias") )
        token = CG_Argv(1);
    else if ( !Q_stricmp( token,"-alias") )
        token = CG_Argv(2);

    if ( !token )
    {
        CG_Printf("usage: +alias <cvar> <cvar>\n");
        return;
    }
    trap_SendConsoleCommand( va("vstr %s;",token) );
}

// Navy Seals --
void CG_ClientScript_f( void );

typedef struct {
    char	*cmd;
    void	(*function)(void);
} consoleCommand_t;


static consoleCommand_t	commands[] = {
                                         { "testgun", CG_TestGun_f },
                                         { "testmodel", CG_TestModel_f },
                                         { "nextframe", CG_TestModelNextFrame_f },
                                         { "prevframe", CG_TestModelPrevFrame_f },
                                         { "nextskin", CG_TestModelNextSkin_f },
                                         { "prevskin", CG_TestModelPrevSkin_f },
                                         { "viewpos", CG_Viewpos_f },
                                         { "+scores", CG_ScoresDown_f },
                                         { "-scores", CG_ScoresUp_f },
                                         { "+reload", CG_ReloadDown_f },
                                         { "-reload", CG_ReloadUp_f },

                                         { "-ironsight", CG_IronSightUp_f },
                                         { "+ironsight", CG_IronSightDown_f },

                                         { "+map", CG_MapDown_f },
                                         { "-map", CG_MapUp_f },

                                         { "+alias", CG_Alias_f },
                                         { "-alias", CG_Alias_f },

                                         { "+weaponmode3", CG_WeaponMode3_f },
                                         { "-weaponmode3", CG_WeaponMode3Up_f },
                                         { "+weaponmode2", CG_WeaponMode2_f },
                                         { "-weaponmode2", CG_WeaponMode2Up_f },
                                         { "+weaponmode1", CG_WeaponMode1_f },
                                         { "-weaponmode1", CG_WeaponMode1Up_f },

                                         { "sizeup", CG_SizeUp_f },
                                         { "sizedown", CG_SizeDown_f },
                                         // Navy Seals ++
                                         { "weapnext", CG_MWhellup_f },
                                         { "weapprev", CG_MWhelldn_f },
                                         // Navy Seals --
                                         { "weapon", CG_Weapon_f },
                                         { "tell_target", CG_TellTarget_f },
                                         { "tcmd", CG_TargetCommand_f },
#ifdef MISSIONPACK
                                         { "loadhud", CG_LoadHud_f },
                                         { "spWin", CG_spWin_f },
                                         { "spLose", CG_spLose_f },
                                         { "scoresDown", CG_scrollScoresDown_f },
                                         { "scoresUp", CG_scrollScoresUp_f },
#endif
                                         { "startOrbit", CG_StartOrbit_f },
                                         { "loaddeferred", CG_LoadDeferredPlayers }	,
                                         { "testcmd", CG_ConsoleTestCmd },
                                         { "looks", CG_UpdateLooks_f },
                                         { "nsecho", CG_NSEcho_f },
                                         { "radiomenu", CG_RadioMenu_f },
                                         { "quithudmenu", CG_QuitHud_f },
                                         { "nssl", CG_ClientScript_f },
                                         { "cs", CG_ClientScript_f },
                                         { "clientscript", CG_ClientScript_f },
                                         { "vqcmd", CG_ViewQCmd_f },
                                         { "missioninfo", CG_ViewMissionInfo_f },
                                         { "writeradar", CG_WriteRadarInfoToBRF_f },
                                         { "editqcmd", CG_EditQCmd_f },
                                         { "editradar", CG_EditRadar_f },
                                         { "edithud", CG_EditHud_f },
                                         { "editradarpos", CG_EditRadarPos_f },
                                         { "cgamemem", CG_GameMem_f }
                                     };

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
    const char	*cmd;
    int		i;

    cmd = CG_Argv(0);

    for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
        if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
            commands[i].function();
            return qtrue;
        }
    }

    return qfalse;
}

/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
    int		i;

    for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
        if ( !Q_stricmp( commands[i].cmd , "quithudmenu" ) )
            continue;

        trap_AddCommand( commands[i].cmd );
    }

    //
    // the game server will interpret these commands, which will be automatically
    // forwarded to the server after they are not recognized locally
    //
    trap_AddCommand ("kill");
    trap_AddCommand ("say");
    trap_AddCommand ("say_team");
    trap_AddCommand ("tell");
    trap_AddCommand ("vsay");
    trap_AddCommand ("vsay_team");
    trap_AddCommand ("vtell");
    trap_AddCommand ("vtaunt");
    trap_AddCommand ("vosay");
    trap_AddCommand ("vosay_team");
    trap_AddCommand ("votell");
    trap_AddCommand ("give");
    trap_AddCommand ("god");
    trap_AddCommand ("notarget");
    trap_AddCommand ("noclip");
    trap_AddCommand ("team");
    trap_AddCommand ("follow");
    trap_AddCommand ("levelshot");
    trap_AddCommand ("addbot");
    trap_AddCommand ("setviewpos");
    trap_AddCommand ("callvote");
    trap_AddCommand ("vote");
    trap_AddCommand ("callteamvote");
    trap_AddCommand ("teamvote");
    trap_AddCommand ("stats");
    trap_AddCommand ("teamtask");
    trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo
    trap_AddCommand ("menuselect");
    trap_AddCommand ("inventory");
    trap_AddCommand ("character");
    trap_AddCommand ("looks");
    trap_AddCommand ("reload");
    trap_AddCommand ("weaponmode1");
    trap_AddCommand ("weaponmode2");
    trap_AddCommand ("weaponmode3");
    trap_AddCommand ("resetxpdist");
    trap_AddCommand ("openxpdist");
    trap_AddCommand ("sendxpdist");
    /*	trap_AddCommand ("inc_acc");
    trap_AddCommand ("inc_sta");
    trap_AddCommand ("inc_str");
    trap_AddCommand ("inc_sth");
    trap_AddCommand ("inc_tec");
    trap_AddCommand ("inc_spd");
    trap_AddCommand ("dec_acc");
    trap_AddCommand ("dec_sta");
    trap_AddCommand ("dec_str");
    trap_AddCommand ("dec_sth");
    trap_AddCommand ("dec_tec");
    trap_AddCommand ("dec_spd");*/
}
