// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame
// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"
#include "../../ui/menudef.h"



/*
=================
CG_ParseMenu

=================
*/


menuState_t i_Menu[MAX_MENU_LINES];
static void CG_ParseMenuString (int pos)
{
    int i;

    for (i=0;i<63;i++)
    {
        i_Menu[pos].Line[i] = i_Menu[pos].Line[i+1];
    }

    i_Menu[pos].Line[64] = ' ';
}
static void CG_ParseMenuOld( void ) {

    int i = 0;

#if 0
    CG_Printf("Recieved Menu Command!\n");
#endif

    cg.num_menuLines = 0;

    //	Com_sprintf(i_Menu[0].Line,sizeof(i_Menu[0].Line), "     Navy Seals: Covert Operations");
    Com_sprintf(i_Menu[1].Line,sizeof(i_Menu[1].Line), "------------------------------------(%s)", NS_VERSION);

    i_Menu[0].l_color = colorLtBlue;
    i_Menu[1].l_color = colorWhite;

    for (i=2; i<= MAX_MENU_LINES; i++ ) {
        // clear Array

        Com_sprintf(i_Menu[i].Line,sizeof(i_Menu[i].Line) ,"%s", CG_Argv(i) );

        switch (i_Menu[i].Line [0]) {
        case '#':
            i_Menu[i].l_color = colorLtBlue;
            CG_ParseMenuString(i);
            break;
        case '§':
            i_Menu[i].l_color = colorRed;
            CG_ParseMenuString(i);
            break;
        case '$':
            i_Menu[i].l_color = colorGreen;
            CG_ParseMenuString(i);
            break;
        case '&':
            i_Menu[i].l_color = colorLtGrey;
            CG_ParseMenuString(i);
            break;
        case '?':
            Com_sprintf(i_Menu[i].Line,sizeof(i_Menu[i].Line), "------------------------------------------");
            i_Menu[i].l_color = colorWhite;
            break;
        default:
            i_Menu[i].l_color = colorWhite;
            break;
        }
    }


    cg.menuValidSlots = atoi(CG_Argv(1));
    cg.inMenu = qtrue;
}

/*
=================
CG_ParseXP

=================
*/
static void CG_ParseXP( void ) {
    int oldXP;
    int dif;
    sfxHandle_t	Sound;
    char	soundname[128];

    oldXP = cg.xpPoints;

    cg.xpPoints = atoi( CG_Argv( 1 ) );
    cg.xpTime = cg.time; // tell user that we've updated

    if ( oldXP > cg.xpPoints )
        return; // we gained nothing
    else
        dif = cg.xpPoints - oldXP;

    if ( dif <= 0 )
        return;

    if ( dif > 10 )
        Com_sprintf (soundname, sizeof(soundname), "sound/commentary/xp_10+.wav" );
    else
        Com_sprintf (soundname, sizeof(soundname), "sound/commentary/xp_%i.wav", dif );

    Sound = trap_S_RegisterSound(soundname, qfalse );

    // add sound to sound buffer
    CG_AddBufferedSound( Sound );
}
extern vmCvar_t ui_sealplayers,ui_tangoplayers,ui_players,ui_sealpoints,ui_tangopoints;

static void CG_UpdateTeamCvars( void )
{
    int seal_players  = 0;
    int tango_players = 0;
    int total_players = 0;
    int i;

    for ( i=0;i<cg.numScores;i++ )
    {
        if ( cg.scores[i].team == TEAM_RED )
            seal_players++;
        else if ( cg.scores[i].team == TEAM_BLUE )
            tango_players++;

        total_players++;
    }

    // team points
    if ( cg.teamScores[0] != ui_sealpoints.integer )
        trap_Cvar_Set("ui_sealpoints", va("%i", cg.teamScores[0] ) );
    if ( cg.teamScores[1] != ui_tangopoints.integer )
        trap_Cvar_Set("ui_tangopoints", va("%i", cg.teamScores[1] ) );

    // cvar updates
    if ( ui_sealplayers.integer != seal_players )
        trap_Cvar_Set("ui_sealplayers", va("%i", seal_players ) );
    if ( ui_tangoplayers.integer != tango_players )
        trap_Cvar_Set("ui_tangoplayers", va("%i", tango_players ) );
    if ( ui_players.integer != total_players )
        trap_Cvar_Set("ui_players", va("%i", total_players ) );
}

#define LEN_RADAR_STRING 4

static void CG_ParseRadar( void ) {
    // <type> <x> <y> <up/down>
    int		i;

    cg.radarNumEntities = atoi( CG_Argv( 1 ) );

    //	CG_Printf("recieved %i radar entities\n", cg.radarNumEntities );

    for ( i = 0; i<cg.radarNumEntities ; i++ )
    {
        cg.radarEntities[i].type = CG_Argv( i * LEN_RADAR_STRING + 2 )[0];
        //		CG_Printf("recived entity type: %c\n",cg.radarEntities[i].type);
        cg.radarEntities[i].origin[0] = atof( CG_Argv( i * LEN_RADAR_STRING + 3 ) );
        cg.radarEntities[i].origin[1] = atof( CG_Argv( i * LEN_RADAR_STRING + 4 ) );
        cg.radarEntities[i].origin[2] = atof( CG_Argv( i * LEN_RADAR_STRING + 5 ) );
    }

}
/*
=================
CG_ParseScores

=================
*/

static void CG_ParseScores( void ) {
    int		i, powerups;

    cg.numScores = atoi( CG_Argv( 1 ) );
    if ( cg.numScores > MAX_CLIENTS ) {
        cg.numScores = MAX_CLIENTS;
    }

    cg.teamScores[0] = atoi( CG_Argv( 2 ) );
    cg.teamScores[1] = atoi( CG_Argv( 3 ) );

    //comment: had to do this since the status is also saved in this struct!
    //	memset( cg.scores, 0, sizeof( cg.scores ) );

    for ( i = 0 ; i < cg.numScores ; i++ ) {
        //
        cg.scores[i].client = atoi( CG_Argv( i * LEN_SCORE_STRING + 4 ) );
        cg.scores[i].score = atoi( CG_Argv( i * LEN_SCORE_STRING + 5 ) );
        cg.scores[i].ping = atoi( CG_Argv( i * LEN_SCORE_STRING + 6 ) );
        cg.scores[i].time = atoi( CG_Argv( i * LEN_SCORE_STRING + 7 ) );
        cg.scores[i].scoreFlags = atoi( CG_Argv( i * LEN_SCORE_STRING + 8 ) );
        powerups = atoi( CG_Argv( i * LEN_SCORE_STRING + 9 ) );

        if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS ) {
            cg.scores[i].client = 0;
        }
        cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;
        cgs.clientinfo[ cg.scores[i].client ].powerups = powerups;

        cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;
    }

    // update cvars.
    CG_UpdateTeamCvars();

#ifdef MISSIONPACK
    CG_SetScoreSelection(NULL);
#endif

}

/*
=================
CG_ParseStatus

=================
*/
static void CG_ParseTeamKill( void ) {
    char text[128];
    int time = atoi(CG_Argv(2));

    Q_strncpyz( text, CG_Argv(1),sizeof(text) );

    if ( !Q_stricmp( text, "done" ) )
    {
        cgs.voteTime = 0;
        return;
    }

    cgs.voteTime = time;
    cgs.voteModified = qtrue;

    Q_strncpyz( cgs.voteString, va("Forgive %s", cgs.clientinfo[atoi(text)].name), sizeof( cgs.voteString ) );
}

/*
=================
CG_ParseStatus

=================
*/
static void CG_ParseStatus( void ) {
    int client = atoi(CG_Argv(1));
    int status = atoi(CG_Argv(2));

    // -1 resets all status to health1
    if ( client < 0 )
    {
        int i = 0;

        for ( i = 0; i < cg.numScores ; i++ )
        {

            cg.playerStatus[i] = status;
            //			CG_Printf("setted status for client %i to %i\n", i,cg.playerStatus[i] );
        }
        return;
    }
    cg.playerStatus[client] = status;
    //	CG_Printf("setted status for client %i to %i\n", client,cg.scores[client].status );
}




/*
=================
CG_ParseTeamInfo

=================
*/
static void CG_ParseTeamInfo( void ) {
    int		i;
    int		client;

    numSortedTeamPlayers = atoi( CG_Argv( 1 ) );

    for ( i = 0 ; i < numSortedTeamPlayers ; i++ ) {
        client = atoi( CG_Argv( i * 6 + 2 ) );

        sortedTeamPlayers[i] = client;

        cgs.clientinfo[ client ].location = atoi( CG_Argv( i * 6 + 3 ) );
        cgs.clientinfo[ client ].health = atoi( CG_Argv( i * 6 + 4 ) );
        cgs.clientinfo[ client ].armor = atoi( CG_Argv( i * 6 + 5 ) );
        cgs.clientinfo[ client ].curWeapon = atoi( CG_Argv( i * 6 + 6 ) );
        cgs.clientinfo[ client ].powerups = atoi( CG_Argv( i * 6 + 7 ) );
    }
}


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void CG_ParseServerinfo( void ) {
    const char	*info;
    char	*mapname;

    info = CG_ConfigString( CS_SERVERINFO );
    cgs.gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );
    trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));
    cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
    cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
    //	cgs.fraglimit = atoi( Info_ValueForKey( info, "fraglimit" ) );
    //	cgs.capturelimit = atoi( Info_ValueForKey( info, "capturelimit" ) );
    cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
    cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
    cgs.matchlockMode = atoi( Info_ValueForKey( info, "g_matchLockXP" ) );
    // navy seals +
    cgs.squadAssault = atoi ( Info_ValueForKey( info, "g_squadAssault" ) ) ;
    cgs.teampointlimit = atoi ( Info_ValueForKey( info, "teampointlimit" ) ) ;
    cgs.teamRespawn = atoi ( Info_ValueForKey( info, "g_teamRespawn" ) ) ;
    cgs.roundtime = atoi ( Info_ValueForKey( info, "roundtime" ) );

    // set cvars for scoreboard
    CG_ForceCvar( "ui_teampointlimit", cgs.teampointlimit );
    CG_ForceCvar( "ui_timelimit", cgs.timelimit );
    CG_ForceCvar( "ui_roundtime", cgs.roundtime );
    CG_ForceCvar( "ui_friendlyfire", atoi( Info_ValueForKey( info, "g_friendlyfire") ) );

    // navy seals -
    mapname = Info_ValueForKey( info, "mapname" );
    Q_strncpyz( cgs.cleanMapName, Info_ValueForKey( info, "mapname" ), sizeof(cgs.cleanMapName) );
    // fixme: do this for campaingsystem
    // Q_strncpyz( cgs.cleanLastMapName, Info_ValueForKey( info, "l_mapname" ), sizeof(cgs.cleanLastMapName) );
    Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );

    trap_Cvar_Set( "mapname", cgs.cleanMapName );
}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void ) {
    const char	*info;
    int			warmup;

    info = CG_ConfigString( CS_WARMUP );

    warmup = atoi( info );
    cg.warmupCount = -1;

    if ( warmup == 0 && cg.warmup ) {

    } else if ( warmup > 0 && cg.warmup <= 0 ) {
        {
            //			trap_S_StartLocalSound( cgs.media.countPrepareSound, CHAN_ANNOUNCER );
        }
    }

    cg.warmup = warmup;
}

/*
==================
CG_ParseBombcaseWires
==================
*/
static void CG_ParseBombcaseWires( void ) {
    int wire;
    int state;

    wire = atoi( CG_Argv( 1 ) );

    if ( wire > 8 || wire < 0 )
        return;

    state = atoi( CG_Argv( 2 ) );

    if ( state > 1 || state < -1 )
        return;

    cg.bombcaseWires[wire-1] = state;

    //	CG_Printf("changed bombwire %i to state %i\n",wire,state);
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void ) {
    //	const char *s;

    cgs.scores1 = atoi( CG_ConfigString( CS_SCORES1 ) );
    cgs.scores2 = atoi( CG_ConfigString( CS_SCORES2 ) );
    cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );

    cg.warmup = atoi( CG_ConfigString( CS_WARMUP ) );
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void) {
    char originalShader[MAX_QPATH];
    char newShader[MAX_QPATH];
    char timeOffset[16];
    const char *o;
    char *n,*t;

    o = CG_ConfigString( CS_SHADERSTATE );
    while (o && *o) {
        n = strstr(o, "=");
        if (n && *n) {
            strncpy(originalShader, o, n-o);
            originalShader[n-o] = 0;
            n++;
            t = strstr(n, ":");
            if (t && *t) {
                strncpy(newShader, n, t-n);
                newShader[t-n] = 0;
            } else {
                break;
            }
            t++;
            o = strstr(t, "@");
            if (o) {
                strncpy(timeOffset, t, o-t);
                timeOffset[o-t] = 0;
                o++;
                trap_R_RemapShader( originalShader, newShader, timeOffset );
            }
        } else {
            break;
        }
    }
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void ) {
    const char	*str;
    int		num;

    num = atoi( CG_Argv( 1 ) );

    // get the gamestate from the client system, which will have the
    // new configstring already integrated
    trap_GetGameState( &cgs.gameState );

    // look up the individual string that was modified
    str = CG_ConfigString( num );

    // do something with it if necessary
    if ( num == CS_MUSIC ) {
        CG_StartMusic();
    } else if ( num == CS_SERVERINFO ) {
        CG_ParseServerinfo();
    } else if ( num == CS_WARMUP ) {
        CG_ParseWarmup();
    } else if ( num == CS_SCORES1 ) {
        cgs.scores1 = atoi( str );
    } else if ( num == CS_SCORES2 ) {
        cgs.scores2 = atoi( str );
    } else if ( num == CS_LEVEL_START_TIME ) {
        cgs.levelStartTime = atoi( str );
    } else if ( num == CS_VOTE_TIME ) {
        cgs.voteTime = atoi( str );
        cgs.voteModified = qtrue;
    } else if ( num == CS_VOTE_YES ) {
        cgs.voteYes = atoi( str );
        cgs.voteModified = qtrue;
    } else if ( num == CS_VOTE_NO ) {
        cgs.voteNo = atoi( str );
        cgs.voteModified = qtrue;
    } else if ( num == CS_VOTE_STRING ) {
        Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
#ifdef MISSIONPACK
        //		trap_S_StartLocalSound( cgs.media.voteNow, CHAN_ANNOUNCER );
#endif //MISSIONPACK
    } else if ( num >= CS_TEAMVOTE_TIME && num <= CS_TEAMVOTE_TIME + 1) {
        cgs.teamVoteTime[num-CS_TEAMVOTE_TIME] = atoi( str );
        cgs.teamVoteModified[num-CS_TEAMVOTE_TIME] = qtrue;
    } else if ( num >= CS_TEAMVOTE_YES && num <= CS_TEAMVOTE_YES + 1) {
        cgs.teamVoteYes[num-CS_TEAMVOTE_YES] = atoi( str );
        cgs.teamVoteModified[num-CS_TEAMVOTE_YES] = qtrue;
    } else if ( num >= CS_TEAMVOTE_NO && num <= CS_TEAMVOTE_NO + 1) {
        cgs.teamVoteNo[num-CS_TEAMVOTE_NO] = atoi( str );
        cgs.teamVoteModified[num-CS_TEAMVOTE_NO] = qtrue;
    } else if ( num >= CS_TEAMVOTE_STRING && num <= CS_TEAMVOTE_STRING + 1) {
        Q_strncpyz( cgs.teamVoteString[num-CS_TEAMVOTE_STRING], str, sizeof( cgs.teamVoteString ) );
#ifdef MISSIONPACK
        //		trap_S_StartLocalSound( cgs.media.voteNow, CHAN_ANNOUNCER );
#endif
    } else if ( num == CS_INTERMISSION ) {
        cg.intermissionStarted = atoi( str );
    } else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
        cgs.gameModels[ num-CS_MODELS ] = trap_R_RegisterModel( str );
    } else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
        if ( str[0] != '*' ) {	// player specific sounds don't register here
            cgs.gameSounds[ num-CS_SOUNDS] = trap_S_RegisterSound( str, qfalse );
        }
    } else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {
        CG_NewClientInfo( num - CS_PLAYERS );
        CG_BuildSpectatorString();
    }
    else if ( num == CS_SHADERSTATE ) {
        CG_ShaderStateChanged();
    } else if ( num == CS_ROUND_START_TIME ) {
        cgs.levelRoundStartTime = atoi( str );
    } else if ( num == CS_VIP_START_TIME ) {
        cgs.levelVipStartTime = atoi( str );
    }
    else if ( num == CS_ASSAULT_START_TIME )
    {
        if ( !Q_stricmp( str, "taken") )
        {
            cgs.assaultFieldsCaptured[0] = qtrue;
        }
        else
        {
            cgs.levelAssaultStartTime[0] = atoi( str );
        }
    }
    else if ( num == CS_ASSAULT2_START_TIME )
    {
        if (!Q_stricmp( str, "taken") )
        {
            cgs.assaultFieldsCaptured[1] = qtrue;
        }
        else
        {
            cgs.levelAssaultStartTime[1] = atoi( str );
        }

    } else if ( num == CS_ASSAULT3_START_TIME )
    {
        if (!Q_stricmp( str, "taken") )
        {
            cgs.assaultFieldsCaptured[2] = qtrue;
        }
        else
        {
            cgs.levelAssaultStartTime[2] = atoi( str );
        }
    }
    else if ( num == CS_ASSAULT4_START_TIME )
    {
        if (!Q_stricmp( str, "taken") )
        {
            cgs.assaultFieldsCaptured[3] = qtrue;
        }
        else
        {
            cgs.levelAssaultStartTime[3] = atoi( str );
        }
    }
    else if ( num == CS_BOMB_START_TIME )
    {

        cgs.levelBombStartTime = atoi( str );
    }
}

/*
=======================
CG_AddToChat

=======================
*/
void CG_AddToChat( const char *str )
{
    int len;
    char *p, *ls;
    int lastcolor;
    int chatHeight;

    if (cg_chatHeight.integer < TEAMCHAT_HEIGHT) {
        chatHeight = cg_chatHeight.integer;
    } else {
        chatHeight = TEAMCHAT_HEIGHT;
    }

    if (chatHeight <= 0 || cg_chatTime.integer <= 0) {
        // team chat disabled, dump into normal chat
        cgs.ChatPos = cgs.LastChatPos = 0;
        return;
    }

    len = 0;

    p = cgs.ChatMsgs[cgs.ChatPos % chatHeight];
    *p = 0;

    lastcolor = '7';

    ls = NULL;
    while (*str) {
        if (len > TEAMCHAT_WIDTH - 1) {
            if (ls) {
                str -= (p - ls);
                str++;
                p -= (p - ls);
            }
            *p = 0;

            cgs.ChatMsgTimes[cgs.ChatPos % chatHeight] = cg.time;

            cgs.ChatPos++;
            p = cgs.ChatMsgs[cgs.ChatPos % chatHeight];
            *p = 0;
            *p++ = Q_COLOR_ESCAPE;
            *p++ = lastcolor;
            len = 0;
            ls = NULL;
        }

        if ( Q_IsColorString( str ) ) {
            *p++ = *str++;
            lastcolor = *str;
            *p++ = *str++;
            continue;
        }
        if (*str == ' ') {
            ls = p;
        }
        *p++ = *str++;
        len++;
    }
    *p = 0;

    cgs.ChatMsgTimes[cgs.ChatPos % chatHeight] = cg.time;
    cgs.ChatPos++;

    if (cgs.ChatPos - cgs.LastChatPos > chatHeight)
        cgs.LastChatPos = cgs.ChatPos - chatHeight;
}


/*
=======================
CG_AddToTeamChat

=======================
*/
static void CG_AddToTeamChat( const char *str ) {
    int len;
    char *p, *ls;
    int lastcolor;
    int chatHeight;

    if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT) {
        chatHeight = cg_teamChatHeight.integer;
    } else {
        chatHeight = TEAMCHAT_HEIGHT;
    }

    if (chatHeight <= 0 || cg_teamChatTime.integer <= 0) {
        // team chat disabled, dump into normal chat
        cgs.teamChatPos = cgs.teamLastChatPos = 0;
        return;
    }

    len = 0;

    p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
    *p = 0;

    lastcolor = '7';

    ls = NULL;
    while (*str) {
        if (len > TEAMCHAT_WIDTH - 1) {
            if (ls) {
                str -= (p - ls);
                str++;
                p -= (p - ls);
            }
            *p = 0;

            cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;

            cgs.teamChatPos++;
            p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
            *p = 0;
            *p++ = Q_COLOR_ESCAPE;
            *p++ = lastcolor;
            len = 0;
            ls = NULL;
        }

        if ( Q_IsColorString( str ) ) {
            *p++ = *str++;
            lastcolor = *str;
            *p++ = *str++;
            continue;
        }
        if (*str == ' ') {
            ls = p;
        }
        *p++ = *str++;
        len++;
    }
    *p = 0;

    cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
    cgs.teamChatPos++;

    if (cgs.teamChatPos - cgs.teamLastChatPos > chatHeight)
        cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
}



/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void ) {
    if ( cg_showmiss.integer ) {
        CG_Printf( "CG_MapRestart\n" );
    }

    CG_InitLocalEntities();
    CG_InitMarkPolys();

    // make sure the "3 frags left" warnings play again
    cg.fraglimitWarnings = 0;

    cg.timelimitWarnings = 0;

    cg.roundlimitWarnings = 0;

    cg.intermissionStarted = qfalse;

    cgs.voteTime = 0;

    cg.mapRestart = qtrue;

    CG_StartMusic();

    trap_S_ClearLoopingSounds(qtrue);

    // we really should clear more parts of cg here and stop sounds

    // play the "fight" sound if this is a restart without warmup
    /*	if ( cg.warmup == 0 /* && cgs.gametype == GT_TOURNAMENT *//*) {
    trap_S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
    CG_CenterPrint( "FIGHT!", 120, GIANTCHAR_WIDTH*2 );
    }*/
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text ) {
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

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
    const char	*cmd;
    char		text[MAX_SAY_TEXT];

    cmd = CG_Argv(0);

    if ( !cmd[0] ) {
        // server claimed the command
        return;
    }

    if ( !strcmp( cmd, "cp" ) ) {
        CG_CenterPrint( CG_Argv(1), SCREEN_HEIGHT * 0.30, cg_newbeeHeight.value );
        return;
    }

    if ( !strcmp( cmd, "cs" ) ) {
        CG_ConfigStringModified();
        return;
    }

    if ( !strcmp( cmd, "print" ) ) {
        if ( strstr( CG_Argv(1), "disconnected\n" ) )
            ;
        else
            CG_Printf( "%s", CG_Argv(1) );
        /*
        #ifdef MISSIONPACK
        cmd = CG_Argv(1);			// yes, this is obviously a hack, but so is the way we hear about
        // votes passing or failing
        if ( !Q_stricmpn( cmd, "vote failed", 11 ) || !Q_stricmpn( cmd, "team vote failed", 16 )) {
        trap_S_StartLocalSound( cgs.media.voteFailed, CHAN_ANNOUNCER );
        } else if ( !Q_stricmpn( cmd, "vote passed", 11 ) || !Q_stricmpn( cmd, "team vote passed", 16 ) ) {
        trap_S_StartLocalSound( cgs.media.votePassed, CHAN_ANNOUNCER );
        }
        #endif  */
        return;
    }

    if ( !strcmp( cmd, "chat" ) ) {
        if ( !cg_teamChatsOnly.integer ) {
            if ( cg_chatBeep.integer )
                trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );

            Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
            CG_Printf( "%s\n", text );
        }
        return;
    }

    if ( !strcmp( cmd, "tchat" ) ) {
        if ( cg_chatBeep.integer )
            trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );

        Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
        CG_RemoveChatEscapeChar( text );
        CG_AddToTeamChat( text );
        CG_Printf("%s\n",text);
        return;
    }
    if ( !strcmp( cmd, "scores" ) ) {
        CG_ParseScores();
        return;
    }

    if ( !strcmp( cmd, "radar" ) )  {
        CG_ParseRadar();
        return;
    }
    if ( !strcmp( cmd, "tinfo" ) ) {
        CG_ParseTeamInfo();
        return;
    }

    if ( !strcmp( cmd, "map_restart" ) ) {
        CG_MapRestart();
        return;
    }

    if ( Q_stricmp (cmd, "remapShader") == 0 ) {
        if (trap_Argc() == 4) {
            trap_R_RemapShader(CG_Argv(1), CG_Argv(2), CG_Argv(3));
        }
    }

    // Navy Seals ++
    if ( !strcmp( cmd, "menu" ) ) {
        CG_ParseMenuOld();
        return;
    }
    if ( !strcmp( cmd, "uxp" ) ) {
        CG_ParseXP();
        return;
    }
    if ( !strcmp( cmd, "mstatus" ) ) {
        CG_ParseStatus();
        return;
    }
    if ( !strcmp( cmd, "tk" ) ) {
        CG_ParseTeamKill();
        return;
    }
    if (!strcmp(cmd,"bombwire")){
        CG_ParseBombcaseWires();
        return;
    }
    if (!strcmp(cmd, "roundst") ) { // remove marks / brass
        int i;

        for ( i=0;i<8;i++)
            cg.bombcaseWires[i] = 0; // -1 red 1 green 0 grey

        // reset fieldcapture status
        for ( i=0;i<4;i++)
            cgs.assaultFieldsCaptured[i] = qfalse;

        cg.gunSmokeTime = 0;
        CG_RemoveAllFragments( );
        // reset some values on every round restart here.
        cg.roundStarted = qtrue; // so the marks will be removed
        cg.roundlimitWarnings = 0;

        if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ||
                cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
            trap_SendConsoleCommand( "vstr entry_startround" );
        // reset blocked entities
        memset( &cg.noMarkEntities,0,sizeof(cg.noMarkEntities) );
        return;
    }

    // Navy Seals --
    // loaddeferred can be both a servercmd and a consolecmd
    if ( !strcmp( cmd, "loaddefered" ) ) {	// FIXME: spelled wrong, but not changing for demo
        CG_LoadDeferredPlayers();
        return;
    }

    // clientLevelShot is sent before taking a special screenshot for
    // the menu system during development
    if ( !strcmp( cmd, "clientLevelShot" ) ) {
        cg.levelShot = qtrue;
        return;
    }

    CG_Printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
    while ( cgs.serverCommandSequence < latestSequence ) {
        if ( trap_GetServerCommand( ++cgs.serverCommandSequence ) ) {
            CG_ServerCommand();
        }
    }
}
