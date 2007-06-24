// Copyright (C) 1999-2000 Id Software, Inc.
//

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

#include "../../ui/menudef.h"			// for the voice chats

/* Q3CAM - BEGIN */
// removed as defconx said 07.01.2004
// #include "camclient.h"
/* Q3CAM - END */

void Cmd_RadioChannel_f ( gentity_t *ent );
gentity_t *fire_ball( gentity_t *self, vec3_t start, vec3_t dir );

void NS_Cmd_Mapinfo (gentity_t *ent );

/*
==================
DeathmatchtMessage

==================
*/
void DeathmatchScoreboardMessage( gentity_t *ent ) {
    char		entry[1024];
    char		string[1400];
    int			stringlength;
    int			i, j;
    gclient_t	*cl;
    int			numSorted, scoreFlags, accuracy, perfect;

    // send the latest information on all clients
    string[0] = 0;
    stringlength = 0;
    scoreFlags = 0;

    numSorted = level.numConnectedClients;

    for (i=0 ; i < numSorted ; i++) {
        int		ping;

        cl = &level.clients[level.sortedClients[i]];

        if ( cl->pers.connected == CON_CONNECTING ) {
            ping = -1;
        } else {
            ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
        }

        if( cl->accuracy_shots ) {
            accuracy = cl->accuracy_hits * 100 / cl->accuracy_shots;
        }
        else {
            accuracy = 0;
        }
        perfect =  0;
        if ( g_gametype.integer == GT_LTS )
            Com_sprintf (entry, sizeof(entry),
                         " %i %i %i %i %i %i",
                         level.sortedClients[i],
                         cl->pers.nsPC.entire_xp,
                         ping,
                         (level.time - cl->pers.enterTime)/60000,
                         scoreFlags,
                         g_entities[level.sortedClients[i]].s.powerups  );//cl->ps.persistant[PERS_CAPTURES]);
        else
            Com_sprintf (entry, sizeof(entry),
                         " %i %i %i %i %i %i",
                         level.sortedClients[i],
                         cl->ps.persistant[PERS_SCORE],
                         ping,
                         (level.time - cl->pers.enterTime)/60000,
                         scoreFlags,
                         g_entities[level.sortedClients[i]].s.powerups );//cl->ps.persistant[PERS_CAPTURES]);

        j = strlen(entry);
        if (stringlength + j > 1024)
            break;
        strcpy (string + stringlength, entry);
        stringlength += j;
    }

    trap_SendServerCommand( ent-g_entities, va("scores %i %i %i%s", i,
                            level.teamScores[TEAM_RED], level.teamScores[TEAM_BLUE],
                            string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( gentity_t *ent ) {
    DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
qboolean	CheatsOk( gentity_t *ent ) {
    if ( !g_cheats.integer ) {
        trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
        return qfalse;
    }
    if ( ent->health <= 0 ) {
        trap_SendServerCommand( ent-g_entities, va("print \"You must be alive to use this command.\n\""));
        return qfalse;
    }
    return qtrue;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) {
    int		i, c, tlen;
    static char	line[MAX_STRING_CHARS];
    int		len;
    char	arg[MAX_STRING_CHARS];

    len = 0;
    c = trap_Argc();
    for ( i = start ; i < c ; i++ ) {
        trap_Argv( i, arg, sizeof( arg ) );
        tlen = strlen( arg );
        if ( len + tlen >= MAX_STRING_CHARS - 1 ) {
            break;
        }
        memcpy( line + len, arg, tlen );
        len += tlen;
        if ( i != c - 1 ) {
            line[len] = ' ';
            len++;
        }
    }

    line[len] = 0;

    return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) {
    while ( *in ) {
        if ( *in == 27 ) {
            in += 2;		// skip color code
            continue;
        }
        if ( *in < 32 ) {
            in++;
            continue;
        }
        *out++ = tolower( *in++ );
    }

    *out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( gentity_t *to, char *s ) {
    gclient_t	*cl;
    int			idnum;
    char		s2[MAX_STRING_CHARS];
    char		n2[MAX_STRING_CHARS];

    // numeric values are just slot numbers
    if (s[0] >= '0' && s[0] <= '9') {
        idnum = atoi( s );
        if ( idnum < 0 || idnum >= level.maxclients ) {
            trap_SendServerCommand( to-g_entities, va("print \"Bad client slot: %i\n\"", idnum));
            return -1;
        }

        cl = &level.clients[idnum];
        if ( cl->pers.connected != CON_CONNECTED ) {
            trap_SendServerCommand( to-g_entities, va("print \"Client %i is not active\n\"", idnum));
            return -1;
        }
        return idnum;
    }

    // check for a name match
    SanitizeString( s, s2 );
    for ( idnum=0,cl=level.clients ; idnum < level.maxclients ; idnum++,cl++ ) {
        if ( cl->pers.connected != CON_CONNECTED ) {
            continue;
        }
        SanitizeString( cl->pers.netname, n2 );
        if ( !strcmp( n2, s2 ) ) {
            return idnum;
        }
    }

    trap_SendServerCommand( to-g_entities, va("print \"User %s is not on the server\n\"", s));
    return -1;
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (gentity_t *ent)
{
    char		*name;
    gitem_t		*it;
    int			i;
    qboolean	give_all;
    gentity_t		*it_ent;
    trace_t		trace;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    name = ConcatArgs( 1 );

    if (Q_stricmp(name, "all") == 0)
        give_all = qtrue;
    else
        give_all = qfalse;

    // Navy Seals ++
    if ( Q_stricmp(name,"legdamage") == 0)
    {
        if (ent->client->ps.stats[STAT_LEG_DAMAGE] >= 100)
            ent->client->ps.stats[STAT_LEG_DAMAGE] = 0;
        else
            ent->client->ps.stats[STAT_LEG_DAMAGE] += 10;
    }
    if ( Q_stricmp(name,"armdamage") == 0)
    {
        if (ent->client->ps.stats[STAT_ARM_DAMAGE] >= 100)
            ent->client->ps.stats[STAT_ARM_DAMAGE] = 0;
        else
            ent->client->ps.stats[STAT_ARM_DAMAGE] += 10;
    }
    if ( Q_stricmp(name, "xp") == 0)
    {
        NS_GiveXP( ent, 5, qfalse );
    }

    if ( Q_stricmp(name, "gl") == 0 )
    {
        ent->client->ns.weaponmode[ ent->s.weapon ] |= ( 1 << WM_GRENADELAUNCHER );
        ent->client->pers.nsInven.weapon_mods[ ent->s.weapon].gl = 1;
    }
    if ( Q_stricmp(name, "bayonet") == 0)
    {

        ent->client->ns.weaponmode[ ent->s.weapon ] |= ( 1 << WM_BAYONET);
        ent->client->pers.nsInven.weapon_mods[ ent->s.weapon].bayonet = 1;
    }
    if ( Q_stricmp(name, "silencer") == 0)
    {
        ent->client->ns.weaponmode[ ent->s.weapon ] |= ( 1 << WM_SILENCER );
        ent->client->pers.nsInven.weapon_mods[ ent->s.weapon].silencer = 1;
    }
    if ( Q_stricmp(name, "scope") == 0)
    {
        ent->client->ns.weaponmode[ ent->s.weapon ] |= ( 1 << WM_SCOPE);
        ent->client->pers.nsInven.weapon_mods[ ent->s.weapon].scope = 1;
    }
    if ( Q_stricmp(name, "lasersight") == 0)
    {
        ent->client->ns.weaponmode[ ent->s.weapon ] |= ( 1 << WM_LASER );
        ent->client->pers.nsInven.weapon_mods[ ent->s.weapon].lasersight = 1;
    }
    if ( Q_stricmp(name, "duckbill") == 0)
    {
        ent->client->ns.weaponmode[ ent->s.weapon ] |= ( 1 << WM_DUCKBILL);
        ent->client->pers.nsInven.weapon_mods[ ent->s.weapon].duckbill = 1;
    }
    if ( Q_stricmp(name, "flashlight") == 0)
    {
        ent->client->ns.weaponmode[ ent->s.weapon ] |= ( 1 << WM_FLASHLIGHT );
        ent->client->pers.nsInven.weapon_mods[ ent->s.weapon].flashlight = 1;
    }
    if ( Q_stricmp(name, "flare") == 0)
    {
        it_ent = G_Spawn();
        NS_SpawnFlare(it_ent);
        it_ent->s.frame = 20;

        it_ent->s.generic1 = 100;
        it_ent->s.otherEntityNum  = 100;
        it_ent->s.otherEntityNum2 = 100;
        it_ent->s.eventParm = 1;

        G_SetOrigin( it_ent, ent->client->ps.origin );
        VectorCopy( ent->client->ps.origin, it_ent->s.origin );
        VectorCopy(ent->client->ps.origin, it_ent->r.currentOrigin );
        trap_LinkEntity( it_ent );
        return;
    }

    if ( Q_stricmp(name, "redflare") == 0)
    {
        it_ent = G_Spawn();
        NS_SpawnFlare(it_ent);
        it_ent->s.frame = 20;

        it_ent->s.generic1 = 100;
        it_ent->s.otherEntityNum  = 0;
        it_ent->s.otherEntityNum2 = 0;
        it_ent->s.eventParm = 1;

        G_SetOrigin( it_ent, ent->client->ps.origin );
        VectorCopy( ent->client->ps.origin, it_ent->s.origin );
        VectorCopy(ent->client->ps.origin, it_ent->r.currentOrigin );
        trap_LinkEntity( it_ent );
        return;
    }
    if ( Q_stricmp(name, "blueflare") == 0)
    {
        it_ent = G_Spawn();
        NS_SpawnFlare(it_ent);
        it_ent->s.frame = 20;

        it_ent->s.generic1 = 0;
        it_ent->s.otherEntityNum  = 0;
        it_ent->s.otherEntityNum2 = 100;
        it_ent->s.eventParm = 1;

        G_SetOrigin( it_ent, ent->client->ps.origin );
        VectorCopy( ent->client->ps.origin, it_ent->s.origin );
        VectorCopy(ent->client->ps.origin, it_ent->r.currentOrigin );
        trap_LinkEntity( it_ent );
        return;
    }
    if ( Q_stricmp(name, "greenflare") == 0)
    {
        it_ent = G_Spawn();
        NS_SpawnFlare(it_ent);
        it_ent->s.frame = 20;

        it_ent->s.generic1 = 0;
        it_ent->s.otherEntityNum  = 100;
        it_ent->s.otherEntityNum2 = 0;
        it_ent->s.eventParm = 1;

        G_SetOrigin( it_ent, ent->client->ps.origin );
        VectorCopy( ent->client->ps.origin, it_ent->s.origin );
        VectorCopy(ent->client->ps.origin, it_ent->r.currentOrigin );
        trap_LinkEntity( it_ent );
        return;
    }
    if ( Q_stricmp(name, "ball") == 0)
    {


        it_ent = fire_ball( ent, ent->client->ps.origin, ent->client->ps.velocity );

        return;
    }

    if ( Q_stricmp(name, "botroam") == 0)
    {


        it = BG_FindItem ("Bot Roam");
        if (!it) {
            return;
        }

        it_ent = G_Spawn();
        VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
        it_ent->classname = "item_botroam";
        G_SpawnItem (it_ent, it);
        FinishSpawningItem( it_ent );

        it_ent->s.eFlags &=~EF_NODRAW;

        G_Printf("Added item_botroam at %s\n", vtos(ent->r.currentOrigin ) );
        return;
    }


    // Navy Seals --

    if (give_all || Q_stricmp( name, "health") == 0)
    {
        ent->health = 100;
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "weapons") == 0)
    {
        int x;

        G_Printf("Please stand by, precaching might take a while!\n");
        for ( x = WP_NUM_WEAPONS - 1; x > 0; x -- ) {
            // this is the fucked up spot
            // if ( x == WP_NUTSHELL) continue;

            BG_PackWeapon( x, ent->client->ps.stats ); // add all weapons!
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "ammo") == 0)
    {
        for ( i = 0 ; i < AM_NUM_AMMO ; i++ ) {
            ent->client->ps.ammo[i] = 10;
        }
        if (!give_all)
            return;
    }

    if (give_all || Q_stricmp(name, "armor") == 0)
    {
        return;
    }

    // spawn a specific item right on the player
    if ( !give_all )
    {
        it = BG_FindItem (name);
        if (!it) {
            return;
        }

        it_ent = G_Spawn();
        VectorCopy( ent->r.currentOrigin, it_ent->s.origin );
        it_ent->classname = it->classname;
        G_SpawnItem (it_ent, it);
        FinishSpawningItem(it_ent );
        memset( &trace, 0, sizeof( trace ) );
        Pick_Item (it_ent, ent, &trace );
        if (it_ent->inuse) {
            G_FreeEntity( it_ent );
        }
    }
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (gentity_t *ent)
{
    char	*msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    ent->flags ^= FL_GODMODE;
    if (!(ent->flags & FL_GODMODE) )
        msg = "godmode OFF\n";
    else
        msg = "godmode ON\n";

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}

/*
==================
Cmd_DebugMission_f

==================
*/
void Cmd_DebugMission_f (gentity_t *ent)
{
    if ( !CheatsOk( ent ) ) {
        return;
    }

    trap_SendServerCommand( ent-g_entities, va("print \"SEALS: NUM %i DONE %i\n TANGOS: NUM %i DONE %i\n\"",
                            level.num_objectives[TEAM_RED], level.done_objectives[TEAM_RED],
                            level.num_objectives[TEAM_BLUE], level.done_objectives[TEAM_BLUE]));
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f( gentity_t *ent ) {
    char	*msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    ent->flags ^= FL_NOTARGET;
    if (!(ent->flags & FL_NOTARGET) )
        msg = "notarget OFF\n";
    else
        msg = "notarget ON\n";

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( gentity_t *ent ) {
    char	*msg;

    if ( !CheatsOk( ent ) ) {
        return;
    }

    if ( ent->client->noclip ) {
        msg = "noclip OFF\n";
    } else {
        msg = "noclip ON\n";
    }
    ent->client->noclip = !ent->client->noclip;

    trap_SendServerCommand( ent-g_entities, va("print \"%s\"", msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( gentity_t *ent ) {
    if ( !CheatsOk( ent ) ) {
        return;
    }

    // doesn't work in single player
    if ( g_gametype.integer != 0 ) {
        trap_SendServerCommand( ent-g_entities,
                                "print \"Must be in g_gametype 0 for levelshot\n\"" );
        return;
    }

    BeginIntermission();
    trap_SendServerCommand( ent-g_entities, "clientLevelShot" );
}





/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( gentity_t *ent ) {
    if ( Is_Spectator( ent ) ) {
        return;
    }
    if (ent->health <= 0) {
        return;
    }
    ent->flags &= ~FL_GODMODE;
    ent->client->ps.stats[STAT_HEALTH] = ent->health = -999;
    player_die (ent, ent, ent, 500, MOD_SUICIDE);
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( gclient_t *client, int oldTeam )
{
    // Navy Seals ++
    if ( client->sess.sessionTeam == TEAM_RED ) {
        PrintMsg( NULL, "%s" S_COLOR_WHITE " joined the seals team.\n ", client->pers.netname );
    } else if ( client->sess.sessionTeam == TEAM_BLUE ) {
        PrintMsg( NULL, "%s" S_COLOR_WHITE " joined the tangos team.\n ", client->pers.netname );
    }
    // Navy Seals --
    else if ( client->sess.sessionTeam == TEAM_SPECTATOR && oldTeam != TEAM_SPECTATOR ) {
        PrintMsg( NULL, "%s" S_COLOR_WHITE " joined the spectators.\n ", client->pers.netname );
    } else if ( client->sess.sessionTeam == TEAM_FREE ) {
        PrintMsg( NULL, "%s" S_COLOR_WHITE " joined the battle.\n ", client->pers.netname );
    }
}

/*
=================
SetTeam
=================
*/
// #define TA_LEADERCODE

void SetTeam( gentity_t *ent, char *s ) {
    int					team, oldTeam;
    gclient_t			*client;
    int					clientNum;
    spectatorState_t	specState;
    int					specClient;
#ifdef TA_LEADERCODE
    int					teamLeader;
#endif
    qboolean			movewaiting = qtrue;

    //PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: game/c_cmds.c / SetTeam called!\n");

    //
    // see what change is requested
    //
    client = ent->client;

    clientNum = client - level.clients;
    specClient = 0;
    specState = SPECTATOR_NOT;
    if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  ) {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_SCOREBOARD;
    } else if ( !Q_stricmp( s, "follow1" ) ) {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FOLLOW;
        specClient = -1;
    } else if ( !Q_stricmp( s, "follow2" ) ) {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FOLLOW;
        specClient = -2;
    } else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) ) {
        team = TEAM_SPECTATOR;
        specState = SPECTATOR_FREE;
    } else if ( g_gametype.integer >= GT_TEAM ) {
        // if running a team game, assign player to one of the teams
        specState = SPECTATOR_NOT;
        if ( !Q_stricmp( s, "seals" ) ) {
            team = TEAM_RED;
        } else if ( !Q_stricmp( s, "tangos" ) ) {
            team = TEAM_BLUE;
        } else {
            // pick the team with the least number of players
            team = PickTeam( clientNum );
        }

        if ( g_teamForceBalance.integer  ) {
            int		counts[TEAM_NUM_TEAMS];

            counts[TEAM_BLUE] = TeamCount( ent->client->ps.clientNum, TEAM_BLUE );
            counts[TEAM_RED] = TeamCount( ent->client->ps.clientNum, TEAM_RED );

            // We allow a spread of two
            if ( team == TEAM_RED && counts[TEAM_RED] - counts[TEAM_BLUE] > 1 ) {
                trap_SendServerCommand( ent->client->ps.clientNum,
                                        "cp \"Seal team has too many players.\n\"" );
                return; // ignore the request
            }
            if ( team == TEAM_BLUE && counts[TEAM_BLUE] - counts[TEAM_RED] > 1 ) {
                trap_SendServerCommand( ent->client->ps.clientNum,
                                        "cp \"Tango team has too many players.\n\"" );
                return; // ignore the request
            }

            // It's ok, the team we are switching to has less or same number of players
        }

    } else {
        // force them to spectators if there aren't any spots free
        team = TEAM_FREE;
    }

    // override decision if limiting the players
    if ( g_maxGameClients.integer > 0 &&
            level.numNonSpectatorClients >= g_maxGameClients.integer ) {
        team = TEAM_SPECTATOR;
    }

    //
    // decide if we will allow the change
    //
    oldTeam = client->sess.sessionTeam;
    if ( team == oldTeam && team != TEAM_SPECTATOR ) {
        return;
    }

    //
    // execute the team change
    //

    // he starts at 'base'
    client->pers.teamState.state = TEAM_BEGIN;

    if ( g_gametype.integer == GT_TEAM && team != TEAM_SPECTATOR )
    {
        ent->flags &= ~FL_GODMODE;
        movewaiting = qfalse;

        if ( ent->client->ps.pm_type == PM_NORMAL &&
                ent->health > 0 )
            Cmd_Kill_f( ent );

        ent->client->sess.waiting = qfalse;
        client->sess.sessionTeam = team;
        client->sess.spectatorState = specState;
        client->sess.spectatorClient = specClient;
        //PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: game/g_cmds.c SetTeam / client with number %i / changing team and weapons\n", clientNum);
        client->ps.persistant[PERS_TEAM] = team;

        client->sess.teamLeader = qfalse;

        { // When playing SA, first round EVERYONE got a mp5 (tango's too)

            gitem_t *pri,*sec;
            // primary weapon
            if ( team == TEAM_BLUE )
                ent->client->pers.nsInven.primaryweapon = WP_MAC10;
            else
                ent->client->pers.nsInven.primaryweapon = WP_MP5;

            // secondary
            if ( team == TEAM_BLUE)
                ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
            else
                ent->client->pers.nsInven.secondaryweapon = WP_MK23;
            ent->client->pers.nsPC.style = 0;

            pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
            sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

            ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 5;
            ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 5;

            // powerups
            ent->client->pers.nsInven.powerups[PW_VEST] = 1;
            ent->client->pers.nsInven.powerups[PW_HELMET] = 0;
        }

        if ( oldTeam == TEAM_SPECTATOR )
            ClientSpawn( ent );
    }
    else if ( oldTeam != TEAM_SPECTATOR && !client->sess.waiting ) {
        // Kill him (makes sure he loses flags, server knows he's dead,etc.)
        ent->flags &= ~FL_GODMODE;
        movewaiting = qfalse;
        Cmd_Kill_f (ent );

    }

    client->sess.sessionTeam = team;
    client->sess.spectatorState = specState;
    client->sess.spectatorClient = specClient;
    //PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: game/g_cmds.c SetTeam / lower setting\n");
    client->ps.persistant[PERS_TEAM] = team;

    client->sess.teamLeader = qfalse;

    BroadcastTeamChange( client, oldTeam );

    // get and distribute relevent paramters
    ClientUserinfoChanged( clientNum );

    if ( g_gametype.integer == GT_LTS && movewaiting )// && GameState != STATE_OPEN )
        client->sess.waiting = qtrue;
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( gentity_t *ent ) {
    ent->client->sess.spectatorState = SPECTATOR_FREE;
    ent->client->ps.pm_flags &= ~PMF_FOLLOW;
    ent->r.svFlags &= ~SVF_BOT;
    ent->client->ps.stats[STAT_HEALTH] = ent->health = 100;
    ent->client->ps.pm_type = PM_SPECTATOR;
    ent->client->ps.eFlags &= ~EF_TELEPORT_BIT;
    ent->client->ps.clientNum = ent - g_entities;
    ent->client->ps.stats[STAT_LEG_DAMAGE] = 0;
    ent->client->ps.stats[STAT_ARM_DAMAGE] = 0;
    ent->client->ps.stats[STAT_STOMACH_DAMAGE] = 0;
    ent->client->ps.stats[STAT_CHEST_DAMAGE] = 0;
    ent->client->ps.stats[STAT_HEAD_DAMAGE] = 0;
    ent->client->ps.stats[STAT_STAMINA] = 300;
    ent->client->ps.stats[STAT_WEAPONMODE] = 0; // reset zoom
    // reset team to original value
    ent->client->ps.persistant[PERS_TEAM] = ent->client->sess.sessionTeam;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( gentity_t *ent ) {
    int			oldTeam;
    char		s[MAX_TOKEN_CHARS];


    if ( ent->client->ps.pm_flags & PMF_FOLLOW  )
        StopFollowing(ent);

    oldTeam = ent->client->sess.sessionTeam;

    if ( trap_Argc() != 2 ) {
        switch ( oldTeam ) {
        case TEAM_BLUE:// Navy Seals ++
            trap_SendServerCommand( ent-g_entities, "print \"Tangos team\n\"" );
            break;
        case TEAM_RED:
            trap_SendServerCommand( ent-g_entities, "print \"Seals team\n\"" );
            break;	// Navy Seals --
        case TEAM_FREE:
            trap_SendServerCommand( ent-g_entities, "print \"Free team\n\"" );
            break;
        case TEAM_SPECTATOR:
            trap_SendServerCommand( ent-g_entities, "print \"Spectator team\n\"" );
            break;
        }
        return;
    }


    if ( ent->client->switchTeamTime >= level.time ) {
        trap_SendServerCommand( ent-g_entities, "print \"May not switch teams more than once per 3 seconds.\n\"" );
        return;
    }

    trap_Argv( 1, s, sizeof( s ) );

    SetTeam( ent, s );

    // set class
    if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR && oldTeam != ent->client->sess.sessionTeam )
        SetClass( ent, CLASS_CUSTOM );


    // give equipment on deathmatch games.
    if ( g_gametype.integer == GT_FFA )
    {

        if ( ent->client->sess.sessionTeam == TEAM_FREE )
        {
            ent->client->sess.waiting = qfalse;
            ClientSpawn( ent );
        }
    }

    ent->client->switchTeamTime = level.time + 3000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( gentity_t *ent ) {
    int		i;
    char	arg[MAX_TOKEN_CHARS];

    if ( trap_Argc() != 2 ) {
        if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
            StopFollowing( ent );
        }
        return;
    }

    trap_Argv( 1, arg, sizeof( arg ) );
    i = ClientNumberFromString( ent, arg );
    if ( i == -1 ) {
        return;
    }

    // can't follow self
    if ( &level.clients[ i ] == ent->client ) {
        return;
    }

    // can't follow another spectator
    if ( level.clients[ i ].sess.sessionTeam == TEAM_SPECTATOR ) {
        return;
    }

    if ( level.clients[i].sess.waiting )
        return;

    // first set them to spectator
    //	if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
    //		SetTeam( ent, "spectator" );
    //	}


    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;
    ent->client->sess.spectatorClient = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
int AliveTeamCount( int ignoreClientNum, team_t team );

void Cmd_FollowCycle_f( gentity_t *ent, int dir ) {
    int		clientnum;
    int		original;
    qboolean	trynodir = qfalse;

    // if they are playing a tournement game, count as a loss
    /*	if ( (g_gametype.integer == GT_TOURNAMENT )
    && ent->client->sess.sessionTeam == TEAM_FREE ) {
    ent->client->sess.losses++;
    }*/
    // first set them to spectator
    if ( dir != 1 && dir != -1 ) {
        G_Error( "Cmd_FollowCycle_f: bad dir %i", dir );
    }

    clientnum = ent->client->sess.spectatorClient;
    original = clientnum;

    if ( AliveTeamCount( -1, TEAM_RED ) + AliveTeamCount( -1, TEAM_BLUE ) < 1 )
    {
        if ( !g_teamlockcamera.integer )
            PrintMsg( ent, "There are no players to chase.\n");
        return;
    }
    if ( !ent->client->sess.waiting )
        return;

    if ( ent->client->sess.spectatorState == SPECTATOR_FREE )
    {
        trynodir = qfalse;

        if ( !level.clients[clientnum].sess.waiting )
            trynodir = qtrue;
    }

    ent->client->sess.spectatorState = SPECTATOR_FOLLOW;

    do {
        if ( !trynodir )
            clientnum += dir;

        trynodir = qfalse;

        if ( clientnum >= level.maxclients ) {
            clientnum = 0;
        }
        if ( clientnum < 0 ) {
            clientnum = level.maxclients - 1;
        }

        // can only follow connected clients
        if ( level.clients[ clientnum ].pers.connected != CON_CONNECTED ) {
            continue;
        }

        // can't follow another spectator
        if ( level.clients[ clientnum ].sess.sessionTeam == TEAM_SPECTATOR ) {
            continue;
        }

        if ( g_teamlockcamera.integer )
        {
            if ( ent->client->sess.sessionTeam != TEAM_FREE &&
                    AliveTeamCount( -1, ent->client->sess.sessionTeam ) > 0 )
            {
                if ( level.clients[clientnum].sess.sessionTeam !=
                        ent->client->sess.sessionTeam )
                    continue;
            }
        }
        // can't follow freezed clients
        if ( level.clients[ clientnum ].ps.pm_type == PM_FREEZE )
            continue;

        // Navy Seals ++
        // can't follow waiting clients
        if ( level.clients[ clientnum ].sess.waiting )
            continue;
        // Navy Seals --

        // this is good, we can use it
        ent->client->sess.spectatorClient = clientnum;
        return;
    } while ( clientnum != original );

    // leave it where it was
}

void NS_RespondToChatString ( const char *chatText );

/*
==================
G_Say
==================
*/

static void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message ) {
    if (!other) {
        return;
    }
    if (!other->inuse) {
        return;
    }
    if (!other->client) {
        return;
    }
    if ( ( mode == SAY_RADIO || mode == SAY_TEAM ) && !OnSameTeam(ent, other) ) {
        return;
    }

    // Navy Seals ++
    if (g_gametype.integer >= GT_LTS)
    {
        // deads and spectators cannot talk to alive
        if ( (ent->client->sess.waiting == qtrue  ||
                ent->client->sess.sessionTeam == TEAM_SPECTATOR ) &&
                other->client->sess.waiting == qfalse )
            return;
        // but alives can talk to deads, but not send radiomsgs.
        if ( ent->client->sess.waiting == qfalse &&
                other->client->sess.waiting == qtrue &&
                mode == SAY_RADIO )
            return;
    }
    // Navy Seals --



    trap_SendServerCommand( other-g_entities, va("%s \"%s%c%c%s\"",
                            ( mode == SAY_RADIO || mode == SAY_TEAM )  ? "tchat" : "chat",
                            name, Q_COLOR_ESCAPE, color, message));
}

#define EC		"\x19"

qboolean NS_IsBandaging( gentity_t *ent );

void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText ) {
    int			j;
    gentity_t	*other;
    int			color;
    char		name[64];
    // don't let text be too long for malicious reasons
    char		text[MAX_SAY_TEXT];
    char		location[128];

    // only chat if the client is not flooding
    if (ent->client->lastChatTime >= level.time) return;
    ent->client->lastChatTime = level.time + SEALS_CHAT_COMMAND_TIME;

    if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) {
        mode = SAY_ALL;
    }


    switch ( mode ) {
    default:
    case SAY_ALL:
        G_LogPrintf( "say: [%i] \"%s\": %s\n",
                     ent->client->ps.clientNum, ent->client->pers.netname, chatText );
        Com_sprintf (name, sizeof(name), "%s%s%s: ", ent->client->pers.netname,(ent->client->sess.waiting)?"*ghost*":"", S_COLOR_WHITE );
        color = COLOR_GREEN;
        break;
    case SAY_TEAM:
        G_LogPrintf( "sayteam: [%i] \"%s\": %s\n",
                     ent->client->ps.clientNum, ent->client->pers.netname, chatText );
        if (Team_GetLocationMsg(ent, location, sizeof(location)))
            Com_sprintf (name, sizeof(name), EC"(%s%s%s%s%c%c"EC") (%s)"EC": ",
                         ent->client->pers.netname,NS_IsBandaging(ent)?S_COLOR_RED"*Bandaging*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->ns.is_vip)?S_COLOR_GREEN"*VIP*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->sess.waiting)?S_COLOR_WHITE"*ghost*":"", Q_COLOR_ESCAPE, COLOR_WHITE, location);
        else
            Com_sprintf (name, sizeof(name), EC"(%s%s%s%s%c%c"EC")"EC": ",
                         ent->client->pers.netname,NS_IsBandaging(ent)?S_COLOR_RED"*Bandaging*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->ns.is_vip)?S_COLOR_GREEN"*VIP*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->sess.waiting)?S_COLOR_WHITE"*ghost*":"", Q_COLOR_ESCAPE, COLOR_WHITE );
        color = COLOR_CYAN;
        break;
    case SAY_RADIO:
        G_LogPrintf( "sayradio: [%i] \"%s\": %s\n",
                     ent->client->ps.clientNum, ent->client->pers.netname, chatText );
        if (Team_GetLocationMsg(ent, location, sizeof(location)))
            Com_sprintf (name, sizeof(name), EC"{%s%s%s%s%c%c"EC"} (%s)"EC": ",
                         ent->client->pers.netname,NS_IsBandaging(ent)?S_COLOR_RED"*Bandaging*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->ns.is_vip)?S_COLOR_GREEN"*VIP*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->sess.waiting)?S_COLOR_WHITE"*ghost*":"", Q_COLOR_ESCAPE, COLOR_WHITE, location);
        else
            Com_sprintf (name, sizeof(name), EC"{%s%s%s%s%c%c"EC"}"EC": ",
                         ent->client->pers.netname,NS_IsBandaging(ent)?S_COLOR_RED"*Bandaging*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->ns.is_vip)?S_COLOR_GREEN"*VIP*"S_COLOR_WHITE:""S_COLOR_WHITE,(ent->client->sess.waiting)?S_COLOR_WHITE"*ghost*":"", Q_COLOR_ESCAPE, COLOR_WHITE );
        color = COLOR_YELLOW;
        break;
    case SAY_TELL:
        if (target && g_gametype.integer >= GT_TEAM &&
                target->client->sess.sessionTeam == ent->client->sess.sessionTeam &&
                Team_GetLocationMsg(ent, location, sizeof(location)))
            Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"] (%s)"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE, location );
        else
            Com_sprintf (name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client->pers.netname, Q_COLOR_ESCAPE, COLOR_WHITE );
        color = COLOR_MAGENTA;
        break;
    }

    Q_strncpyz( text, chatText, sizeof(text) );

    if ( target ) {
        G_SayTo( ent, target, mode, color, name, text );
        return;
    }

    // echo the text to the console
    if ( g_dedicated.integer ) {
        //	G_Printf( "%s%s\n", name, text);
    }

    // send it to all the apropriate clients
    for (j = 0; j < level.maxclients; j++) {
        other = &g_entities[j];
        G_SayTo( ent, other, mode, color, name, text );
    }

    NS_RespondToChatString( chatText );
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( gentity_t *ent, int mode, qboolean arg0 ) {
    char		*p;

    if ( trap_Argc () < 2 && !arg0 ) {
        return;
    }

    if (arg0)
    {
        p = ConcatArgs( 0 );
    }
    else
    {
        p = ConcatArgs( 1 );
    }

    G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( gentity_t *ent ) {
    int			targetNum;
    gentity_t	*target;
    char		*p;
    char		arg[MAX_TOKEN_CHARS];

    if ( trap_Argc () < 2 ) {
        return;
    }

    trap_Argv( 1, arg, sizeof( arg ) );
    targetNum = atoi( arg );
    if ( targetNum < 0 || targetNum >= level.maxclients ) {
        return;
    }

    target = &g_entities[targetNum];
    if ( !target || !target->inuse || !target->client ) {
        return;
    }

    p = ConcatArgs( 2 );

    G_LogPrintf( "tell: [%i] \"%s\" to [%i] \"%s\": %s\n",
                 ent->client->ps.clientNum, ent->client->pers.netname,
                 target->client->ps.clientNum, target->client->pers.netname, p );
    G_Say( ent, target, SAY_TELL, p );
    // don't tell to the player self if it was already directed to this player
    // also don't send the chat back to a bot
    if ( ent != target && !(ent->r.svFlags & SVF_BOT)) {
        G_Say( ent, ent, SAY_TELL, p );
    }
}



/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( gentity_t *ent ) {
    trap_SendServerCommand( ent-g_entities, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

static const char *gameNames[] = {
                                     "Free For All",
                                     "Training",
                                     "Free Teamplay",
                                     "Navy Seals"
                                 };

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( gentity_t *ent ) {
    int		i;
    char	arg1[MAX_STRING_TOKENS];
    char	arg2[MAX_STRING_TOKENS];

    if ( !g_allowVote.integer ) {
        trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
        return;
    }

    if ( level.voteTime ) {
        trap_SendServerCommand( ent-g_entities, "print \"A vote is already in progress.\n\"" );
        return;
    }
    if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
        trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of votes.\n\"" );
        return;
    }
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
        trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
        return;
    }
    if ( g_gametype.integer == GT_LTS && LTS_Rounds <= 2 ) {
        trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote in the first two rounds.\n\"" );
        return;
    }

    ent->client->pers.voteCount++;

    // make sure it is a valid command to vote on
    trap_Argv( 1, arg1, sizeof( arg1 ) );
    trap_Argv( 2, arg2, sizeof( arg2 ) );

    if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        return;
    }

    if ( !Q_stricmp( arg1, "map_restart" ) ) {
        if ( !g_allowMapVote.integer )
        {
            PrintMsg( ent, "Map voting not allowed on this server.\n");
            return;
        }
    } else if ( !Q_stricmp( arg1, "nextmap" ) ) {
        if ( !g_allowMapVote.integer )
        {
            PrintMsg( ent, "Map voting not allowed on this server.\n");
            return;
        }
    } else if ( !Q_stricmp( arg1, "map" ) ) {
        if ( !g_allowMapVote.integer )
        {
            PrintMsg( ent, "Map voting not allowed on this server.\n");
            return;
        }
        //	} else if ( !Q_stricmp( arg1, "g_gametype" ) ) {
    } else if ( !Q_stricmp( arg1, "kick" ) ) {
        if ( !g_allowKickVote.integer )
        {
            PrintMsg( ent, "Kick voting not allowed on this server.\n");
            return;
        }
    } else if ( !Q_stricmp( arg1, "clientkick" ) ) {
        if ( !g_allowKickVote.integer )
        {
            PrintMsg( ent, "Kick voting not allowed on this server.\n");
            return;
        }
        //	} else if ( !Q_stricmp( arg1, "g_doWarmup" ) ) {
    } else if ( !Q_stricmp( arg1, "timelimit" ) ) {
        if ( !g_allowTimelimitVote.integer )
        {
            PrintMsg( ent, "Timelimit voting not allowed on this server.\n");
            return;
        }
        i = 0;
        i = atoi( arg2 );

        if ( ! ( (i >= 5) && (i <= 80) ) ||
                (strlen(arg2) > 3) ){
            trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
            return;
        }
    } else if ( !Q_stricmp( arg1, "friendlyfire" ) ) {
        i = 0;
        i = atoi( arg2 );

        if ( ! ( (i >= 0) && (i <= 1) ) ||
                (strlen(arg2) > 3) ) {
            trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
            return;
        }
    } else if ( !Q_stricmp( arg1, "teampointlimit" ) ) {
        if ( !g_allowTeampointlimitVote.integer )
        {
            PrintMsg( ent, "Teampointlimit voting not allowed on this server.\n");
            return;
        }
        i = 0;
        i = atoi( arg2 );

        if ( ! ( (i >= 5) && (i <= 80) ) ||
                (strlen(arg2) > 3) ) {
            trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
            return;
        }
    } else {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        trap_SendServerCommand( ent-g_entities, "print \"Vote commands are: map_restart, nextmap, map <mapname>, kick <player>, friendlyfire <0/1>, clientkick <clientnum>, timelimit <5-80>, teampointlimit <5-80>.\n\"" );
        return;
    }

    // if there is still a vote to be executed
    if ( level.voteExecuteTime ) {
        level.voteExecuteTime = 0;
        trap_SendConsoleCommand( EXEC_APPEND, va("%s\n", level.voteString ) );
    }

    if ( !Q_stricmp( arg1, "friendlyfire" ) )
    {
        i = atoi( arg2 );

        Com_sprintf( level.voteString, sizeof( level.voteString ), "g_friendlyfire %d",  i );
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s Friendly Fire",(i==1)?"Enable":"Disable" );
    }
    else if ( !Q_stricmp( arg1, "map" ) ) {
        // special case for map changes, we want to reset the nextmap setting
        // this allows a player to change maps, but not upset the map rotation
        char	s[MAX_STRING_CHARS];

        if ( arg2[0] != 'n' || arg2[1] != 's' || arg2[2] != '_'  )
        {
            PrintMsg( ent, "Invalid vote string %s. You can only vote ns_ maps.\n", arg2);
            return;
        }
        trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );

        Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %s", arg1, arg2 );
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
    } else if ( !Q_stricmp( arg1, "nextmap" ) ) {

        if ( level.mapCycleNumMaps > 0 )
        {
            Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s", NS_GetNextMap() );
            Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Change to the next map in cycle: %s", NS_GetNextMap() );
        }
        else
        {
            char	s[MAX_STRING_CHARS];
            trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
            if (!*s) {
                trap_SendServerCommand( ent-g_entities, "print \"nextmap not set.\n\"" );
                return;
            }
            Com_sprintf( level.voteString, sizeof( level.voteString ), "vstr nextmap");
            Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Execute the 'nextmap' variable." );
        }

    } else if (!Q_stricmp(arg1,"kick")) {
        int i,kicknum=MAX_CLIENTS;
        char	cleanName[64]; // JPW NERVE

        for (i=0;i<MAX_CLIENTS;i++) {
            if (level.clients[i].pers.connected != CON_CONNECTED)
                continue;
            // strip the color crap out
            Q_strncpyz( cleanName, level.clients[i].pers.netname, sizeof(cleanName) );
            Q_CleanStr( cleanName );
            if ( !Q_stricmp( cleanName, arg2 ) )
                kicknum = i;
        }
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "kick %s", level.clients[kicknum].pers.netname );
        if (kicknum != MAX_CLIENTS) { // found a client # to kick, so override votestring with better one
            Com_sprintf(level.voteString, sizeof(level.voteString),"clientkick \"%d\"",kicknum);
        }
        else { // if it can't do a name match, don't allow kick (to prevent votekick text spam wars)
            trap_SendServerCommand( ent-g_entities, "print \"Client not on server.\n\"" );
            return;
        }
        // jpw
    } else {
        Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
        Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
    }

    trap_SendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->client->pers.netname ) );

    // start the voting, the caller autoamtically votes yes
    level.voteTime = level.time;
    level.voteYes = 1;
    level.voteNo = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        level.clients[i].ps.eFlags &= ~EF_VOTED;
    }
    ent->client->ps.eFlags |= EF_VOTED;

    trap_SetConfigstring( CS_VOTE_TIME, va("%i", level.voteTime ) );
    trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );
    trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
    trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( gentity_t *ent ) {
    char		msg[64];

    if ( !level.voteTime )
    {
        if ( ent->client->pers.killedByMate > level.time )
        {
            gentity_t *killer = &g_entities[ ent->client->pers.killedByMateClientNum ];

            if ( !killer )
                return;

            trap_Argv( 1, msg, sizeof( msg ) );

            if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
                killer->client->pers.lockedPlayer--;
                killer->client->pers.teamKills--;
                trap_SendServerCommand( killer-g_entities, va("cp \"Teamkill forgiven by %s%s%s.\n\"",S_COLOR_WHITE,ent->client->pers.netname, S_COLOR_WHITE ) );
                PrintMsg( NULL, "%s%s's Teamkill was forgiven by %s%s.\n", killer->client->pers.netname,S_COLOR_WHITE, ent->client->pers.netname,S_COLOR_WHITE );
            }
            else
            {

            }
            trap_SendServerCommand( ent->client->ps.clientNum, "tk done" );
            ent->client->pers.killedByMate = 0;
            return;
        }
        trap_SendServerCommand( ent-g_entities, "print \"No vote in progress.\n\"" );
        return;
    }
    if ( ent->client->ps.eFlags & EF_VOTED ) {
        trap_SendServerCommand( ent-g_entities, "print \"Vote already cast.\n\"" );
        return;
    }
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
        trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
        return;
    }

    trap_SendServerCommand( ent-g_entities, "print \"Vote cast.\n\"" );

    ent->client->ps.eFlags |= EF_VOTED;

    trap_Argv( 1, msg, sizeof( msg ) );

    if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
        level.voteYes++;
        trap_SetConfigstring( CS_VOTE_YES, va("%i", level.voteYes ) );
    } else {
        level.voteNo++;
        trap_SetConfigstring( CS_VOTE_NO, va("%i", level.voteNo ) );
    }

    // a majority will be determined in CheckVote, which will also account
    // for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( gentity_t *ent ) {
    int		i, team, cs_offset;
    char	arg1[MAX_STRING_TOKENS];
    char	arg2[MAX_STRING_TOKENS];

    team = ent->client->sess.sessionTeam;
    if ( team == TEAM_RED )
        cs_offset = 0;
    else if ( team == TEAM_BLUE )
        cs_offset = 1;
    else
        return;

    if ( !g_allowVote.integer ) {
        trap_SendServerCommand( ent-g_entities, "print \"Voting not allowed here.\n\"" );
        return;
    }

    if ( level.teamVoteTime[cs_offset] ) {
        trap_SendServerCommand( ent-g_entities, "print \"A team vote is already in progress.\n\"" );
        return;
    }
    // BLUTENGEL_XXX:
    // removed as i needed a free variable in clientPersistant_t
    // removed pers.teamVoteCount as this variable is not used except
    // here. just ignore it
    /*	if ( ent->client->pers.teamVoteCount >= MAX_VOTE_COUNT ) {
    trap_SendServerCommand( ent-g_entities, "print \"You have called the maximum number of team votes.\n\"" );
    return;
    }*/
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
        trap_SendServerCommand( ent-g_entities, "print \"Not allowed to call a vote as spectator.\n\"" );
        return;
    }

    // make sure it is a valid command to vote on
    trap_Argv( 1, arg1, sizeof( arg1 ) );
    arg2[0] = '\0';
    for ( i = 2; i < trap_Argc(); i++ ) {
        if (i > 2)
            strcat(arg2, " ");
        trap_Argv( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
    }

    if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        return;
    }

    if ( !Q_stricmp( arg1, "leader" ) ) {
        char netname[MAX_NETNAME], leader[MAX_NETNAME];

        if ( !arg2[0] ) {
            i = ent->client->ps.clientNum;
        }
        else {
            // numeric values are just slot numbers
            for (i = 0; i < 3; i++) {
                if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
                    break;
            }
            if ( i >= 3 || !arg2[i]) {
                i = atoi( arg2 );
                if ( i < 0 || i >= level.maxclients ) {
                    trap_SendServerCommand( ent-g_entities, va("print \"Bad client slot: %i\n\"", i) );
                    return;
                }

                if ( !g_entities[i].inuse ) {
                    trap_SendServerCommand( ent-g_entities, va("print \"Client %i is not active\n\"", i) );
                    return;
                }
            }
            else {
                Q_strncpyz(leader, arg2, sizeof(leader));
                Q_CleanStr(leader);
                for ( i = 0 ; i < level.maxclients ; i++ ) {
                    if ( level.clients[i].pers.connected == CON_DISCONNECTED )
                        continue;
                    if (level.clients[i].sess.sessionTeam != team)
                        continue;
                    Q_strncpyz(netname, level.clients[i].pers.netname, sizeof(netname));
                    Q_CleanStr(netname);
                    if ( !Q_stricmp(netname, leader) ) {
                        break;
                    }
                }
                if ( i >= level.maxclients ) {
                    trap_SendServerCommand( ent-g_entities, va("print \"%s is not a valid player on your team.\n\"", arg2) );
                    return;
                }
            }
        }
        Com_sprintf(arg2, sizeof(arg2), "%d", i);
    } else {
        trap_SendServerCommand( ent-g_entities, "print \"Invalid vote string.\n\"" );
        trap_SendServerCommand( ent-g_entities, "print \"Team vote commands are: leader <player>.\n\"" );
        return;
    }

    Com_sprintf( level.teamVoteString[cs_offset], sizeof( level.teamVoteString[cs_offset] ), "%s %s", arg1, arg2 );

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].pers.connected == CON_DISCONNECTED )
            continue;
        if (level.clients[i].sess.sessionTeam == team)
            trap_SendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client->pers.netname ) );
    }

    // start the voting, the caller autoamtically votes yes
    level.teamVoteTime[cs_offset] = level.time;
    level.teamVoteYes[cs_offset] = 1;
    level.teamVoteNo[cs_offset] = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if (level.clients[i].sess.sessionTeam == team)
            level.clients[i].ps.eFlags &= ~EF_TEAMVOTED;
    }
    ent->client->ps.eFlags |= EF_TEAMVOTED;

    trap_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", level.teamVoteTime[cs_offset] ) );
    trap_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, level.teamVoteString[cs_offset] );
    trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
    trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( gentity_t *ent ) {
    int			team, cs_offset;
    char		msg[64];

    team = ent->client->sess.sessionTeam;
    if ( team == TEAM_RED )
        cs_offset = 0;
    else if ( team == TEAM_BLUE )
        cs_offset = 1;
    else
        return;

    if ( !level.teamVoteTime[cs_offset] ) {
        trap_SendServerCommand( ent-g_entities, "print \"No team vote in progress.\n\"" );
        return;
    }
    if ( ent->client->ps.eFlags & EF_TEAMVOTED ) {
        trap_SendServerCommand( ent-g_entities, "print \"Team vote already cast.\n\"" );
        return;
    }
    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
        trap_SendServerCommand( ent-g_entities, "print \"Not allowed to vote as spectator.\n\"" );
        return;
    }

    trap_SendServerCommand( ent-g_entities, "print \"Team vote cast.\n\"" );

    ent->client->ps.eFlags |= EF_TEAMVOTED;

    trap_Argv( 1, msg, sizeof( msg ) );

    if ( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) {
        level.teamVoteYes[cs_offset]++;
        trap_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", level.teamVoteYes[cs_offset] ) );
    } else {
        level.teamVoteNo[cs_offset]++;
        trap_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", level.teamVoteNo[cs_offset] ) );
    }

    // a majority will be determined in TeamCheckVote, which will also account
    // for players entering or leaving
}


/*
=================
Cmd_SetViewpos_f
=================
*/
void Cmd_SetViewpos_f( gentity_t *ent ) {
    vec3_t		origin, angles;
    char		buffer[MAX_TOKEN_CHARS];
    int			i;

    if ( !g_cheats.integer ) {
        trap_SendServerCommand( ent-g_entities, va("print \"Cheats are not enabled on this server.\n\""));
        return;
    }
    if ( trap_Argc() != 5 ) {
        trap_SendServerCommand( ent-g_entities, va("print \"usage: setviewpos x y z yaw\n\""));
        return;
    }

    VectorClear( angles );
    for ( i = 0 ; i < 3 ; i++ ) {
        trap_Argv( i + 1, buffer, sizeof( buffer ) );
        origin[i] = atof( buffer );
    }

    trap_Argv( 4, buffer, sizeof( buffer ) );
    angles[YAW] = atof( buffer );

    TeleportPlayer( ent, origin, angles );
}



/*
=================
Cmd_Stats_f
=================
*/
void Cmd_Stats_f( gentity_t *ent ) {
    /*
    int max, n, i;

    max = trap_AAS_PointReachabilityAreaIndex( NULL );

    n = 0;
    for ( i = 0; i < max; i++ ) {
    if ( ent->client->areabits[i >> 3] & (1 << (i & 7)) )
    n++;
    }

    //trap_SendServerCommand( ent-g_entities, va("print \"visited %d of %d areas\n\"", n, max));
    trap_SendServerCommand( ent-g_entities, va("print \"%d%% level coverage\n\"", n * 100 / max));
    */
}

void NS_DropMissionObjective( gentity_t *ent );

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum ) {
    gentity_t *ent;
    char	cmd[MAX_TOKEN_CHARS];

    ent = g_entities + clientNum;

    if ( !ent->client ) {
        return;		// not fully in game yet
    }

    trap_Argv( 0, cmd, sizeof( cmd ) );

    if (Q_stricmp (cmd, "say") == 0) {
        Cmd_Say_f (ent, SAY_ALL, qfalse);
        return;
    }
    if (Q_stricmp (cmd, "say_team") == 0) {
        Cmd_Say_f (ent, SAY_TEAM, qfalse);
        return;
    }
    if (Q_stricmp (cmd, "tell") == 0) {
        Cmd_Tell_f ( ent );
        return;
    }
    if (Q_stricmp (cmd, "score") == 0) {
        Cmd_Score_f (ent);
        return;
    }

    if (Q_stricmp (cmd, "reload") == 0)
    {
        if ( ent->client->ps.weaponstate != WEAPON_RELOADING )
            ent->client->ns.reload_tries++;

        return;
    }
    if (Q_stricmp(cmd, "weaponmode1") == 0)
    {
        ent->client->ns.weaponmode_tries[0]++;
        return;
    }
    if (Q_stricmp(cmd, "weaponmode2") == 0)
    {
        ent->client->ns.weaponmode_tries[1]++;
        return;
    }
    if (Q_stricmp(cmd, "weaponmode3") == 0)
    {
        ent->client->ns.weaponmode_tries[2]++;
        return;
    }
    if (Q_stricmp (cmd, "character") == 0)
    {
        //		if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        //			return;
        NS_RaiseCharacterLevel(ent);
        return;
    }
    if (Q_stricmp (cmd, "inventory") == 0)
    {
        //	if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        //		return;
        NS_PlayerInventory(ent);
        return;
    }

    // ignore all other commands when at intermission
    if (level.intermissiontime) {
        Cmd_Say_f (ent, qfalse, qtrue);
        return;
    }

    if (Q_stricmp (cmd, "give") == 0)
        Cmd_Give_f (ent);
    else if (Q_stricmp (cmd, "god") == 0)
        Cmd_God_f (ent);
    else if (Q_stricmp (cmd, "notarget") == 0)
        Cmd_Notarget_f (ent);
    else if (Q_stricmp (cmd, "noclip") == 0)
        Cmd_Noclip_f (ent);
    else if (Q_stricmp (cmd, "kill") == 0)
        Cmd_Kill_f (ent);
    else if (Q_stricmp (cmd, "levelshot") == 0)
        Cmd_LevelShot_f (ent);
    else if (Q_stricmp (cmd, "debug_mission") == 0)
        Cmd_DebugMission_f (ent);
    /*
    else if (Q_stricmp (cmd, "follow") == 0)
    Cmd_Follow_f (ent);*/
    else if (Q_stricmp (cmd, "follownext") == 0)
        Cmd_FollowCycle_f (ent, 1);
    else if (Q_stricmp (cmd, "followprev") == 0)
        Cmd_FollowCycle_f (ent, -1);
    else if (Q_stricmp (cmd, "team") == 0)
        Cmd_Team_f (ent);
    else if (Q_stricmp (cmd, "where") == 0)
        Cmd_Where_f (ent);
    else if (Q_stricmp (cmd, "callvote") == 0)
        Cmd_CallVote_f (ent);
    else if (Q_stricmp (cmd, "vote") == 0)
        Cmd_Vote_f (ent);
    else if (Q_stricmp (cmd, "callteamvote") == 0)
        Cmd_CallTeamVote_f (ent);
    else if (Q_stricmp (cmd, "teamvote") == 0)
        Cmd_TeamVote_f (ent);
    else if (Q_stricmp (cmd, "setviewpos") == 0)
        Cmd_SetViewpos_f( ent );
    else if (Q_stricmp (cmd, "stats") == 0)
        Cmd_Stats_f( ent );
    else if (Q_stricmp (cmd, "menuselect") == 0) // used for client
        NS_MenuSelect(ent);
    else if (Q_stricmp (cmd, "bandage") == 0)
        NS_StartBandage( ent );
    else if (Q_stricmp (cmd, "use") == 0)
        NS_OpenDoor( ent , qtrue );
    else if (Q_stricmp (cmd, "dropweapon") == 0)
        NS_DropWeapon ( ent );
    else if (Q_stricmp (cmd, "dropmissionobjective") == 0)
        NS_DropMissionObjective ( ent );
    else if (Q_stricmp(cmd, "radio") == 0) {
        Cmd_Radioteam_f(ent);
    } else if (Q_stricmp(cmd, "radiopower") == 0)
        Cmd_Radio_power_f(ent);
    else if ( !Q_stricmp (cmd,"radiochannel") )
        Cmd_RadioChannel_f( ent );
    else if (Q_stricmp(cmd, "kill") == 0)
        Cmd_Kill_f(ent);
    //	else if (Q_stricmp(cmd, "holster") == 0)
    //		NS_HolsterWeapon ( ent );
    //	else if (Q_stricmp(cmd, "hostage") == 0)
    //		hostage_spawn( );
#if SEALS_INCLUDE_NVG
    else if (Q_stricmp(cmd, "testnvg") == 0)
    {
        if ( g_cheats.integer ) {
            if ( ent->client->ps.stats[STAT_ACTIVE_ITEMS] & ( 1 << UI_NVG ) )
                G_Printf("testing nvg! [ off ]\n");
            else
                G_Printf("testing nvg! [ on ]\n");

            if ( ent->client->ps.stats[STAT_ACTIVE_ITEMS] & ( 1 << UI_NVG ) )
                ent->client->ps.stats[STAT_ACTIVE_ITEMS] &= ~( 1 << UI_NVG );
            else
                ent->client->ps.stats[STAT_ACTIVE_ITEMS] |= ( 1 << UI_NVG );
        }
    }
#endif
    else if (Q_stricmp(cmd, "gesture") == 0) {
        NS_Gesture( ent );
    } else if ( Q_stricmp (cmd,"mapinfo") == 0)
        NS_Cmd_Mapinfo( ent );
    else if ( Q_stricmp (cmd,"test") == 0)
    {

    }
    else if ( Q_stricmp (cmd,"mynameisdefconx") == 0)
    {
        NS_Itsame( ent );
    }
    else if ( Q_stricmp (cmd,"tmequip") == 0)
    {
        NS_TMequip( ent );
    }
    else if ( Q_stricmp ( cmd, "freexp" ) == 0 )
        NS_FreeXP( ent );
#if 1 // fun stuff, please do not remove this, just add more funny things. thanks.
    //	else if ( Q_stricmp ( cmd, "whosamoron" ) == 0 )
    //		PrintMsg( ent, "hoak is\n");
    else if ( Q_stricmp ( cmd, "whorocks" ) == 0 )
        PrintMsg( ent, "tm ofc\n");
    //	else if ( Q_stricmp ( cmd, "letifer" ) == 0 )
    //		PrintMsg( ent, "letifer sux\n");
    //	else if ( Q_stricmp ( cmd, "defsblacklist" ) == 0 )
    //		PrintMsg( ent, "hoak,gtk,dan,blackop all suck monkey dick :)\n");
    //	else if ( Q_stricmp ( cmd, "whoistm" ) == 0 )
    //		PrintMsg( ent, "demo ogun schakal defcon cyte scrutch\n");
    //	else if ( Q_stricmp ( cmd, "givememoney" ) == 0 )
    //		PrintMsg( ent, "no\n");
    //	else if ( Q_stricmp ( cmd, "givemeallyourmoney" ) == 0 )
    //		PrintMsg( ent, "hell no\n");
    //	else if ( Q_stricmp ( cmd, "givemeallyourmoneyandyourjewels" ) == 0 )
    //		PrintMsg( ent, "one more and you got me\n");
    //	else if ( Q_stricmp ( cmd, "whosyourgod" ) == 0 )
    //		PrintMsg( ent, "defconx\n");
    //	else if ( Q_stricmp ( cmd, "microsoft" ) == 0 )
    //		PrintMsg( ent, "sux\n");
    else if ( Q_stricmp ( cmd, "09041983" ) == 0 )
        PrintMsg( ent, "what a wonderful day it was\n");
    else if ( Q_stricmp ( cmd, "idsoftware" ) == 0 )
        PrintMsg( ent, "rox\n");
    else if ( Q_stricmp ( cmd, "chromejuice" ) == 0 )
        PrintMsg( ent, "hell yeah\n");
#endif
    else
        trap_SendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}

