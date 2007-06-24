// Copyright (C) 1999-2000 Id Software, Inc.
//

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

// removed as defconx said 07.01.2004
// #include "camclient.h"

void NS_SendPlayersStatusToAllPlayers( int clientNum, int status );
void NS_SendStatusMessageToTeam ( gentity_t *affected, int status, int team );
// g_client.c -- client functions that don't happen every frame

// static vec3_t	playerMins = {-15, -15, -24};
// static vec3_t	playerMaxs = { 15, 15, 32};

static vec3_t	playerMins = {-10, -10, MINS_Z};
static vec3_t	playerMaxs = {10, 10, MAXS_Z};

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( gentity_t *ent ) {
    int		i;

    G_SpawnInt( "nobots", "0", &i);
    if ( i ) {
        ent->flags |= FL_NO_BOTS;
    }
    G_SpawnInt( "nohumans", "0", &i );
    if ( i ) {
        ent->flags |= FL_NO_HUMANS;
    }
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start(gentity_t *ent) {
    ent->classname = "info_player_deathmatch";
    SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( gentity_t *ent ) {

}



/*
=======================================================================

SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
qboolean SpotWouldTelefrag( gentity_t *spot ) {
    int			i, num;
    int			touch[MAX_GENTITIES];
    gentity_t	*hit;
    vec3_t		mins, maxs;

    VectorAdd( spot->s.origin, playerMins, mins );
    VectorAdd( spot->s.origin, playerMaxs, maxs );
    num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

    for (i=0 ; i<num ; i++) {
        hit = &g_entities[touch[i]];
        //if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
        if ( hit->client) {
            return qtrue;
        }

    }

    return qfalse;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectNearestDeathmatchSpawnPoint( vec3_t from ) {
    gentity_t	*spot;
    vec3_t		delta;
    float		dist, nearestDist;
    gentity_t	*nearestSpot;

    nearestDist = 999999;
    nearestSpot = NULL;
    spot = NULL;

    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {

        VectorSubtract( spot->s.origin, from, delta );
        dist = VectorLength( delta );
        if ( dist < nearestDist ) {
            nearestDist = dist;
            nearestSpot = spot;
        }
    }

    return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
gentity_t *SelectRandomDeathmatchSpawnPoint( void ) {
    gentity_t	*spot;
    int			count;
    int			selection;
    gentity_t	*spots[MAX_SPAWN_POINTS];

    count = 0;
    spot = NULL;

    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
        if ( SpotWouldTelefrag( spot ) ) {
            continue;
        }
        spots[ count ] = spot;
        count++;
    }

    if ( !count ) {	// no spots that won't telefrag
        return G_Find( NULL, FOFS(classname), "info_player_deathmatch");
    }

    selection = rand() % count;
    return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectRandomFurthestSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
    gentity_t	*spot;
    vec3_t		delta;
    float		dist;
    float		list_dist[64];
    gentity_t	*list_spot[64];
    int			numSpots, rnd, i, j;

    numSpots = 0;
    spot = NULL;

    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
        if ( SpotWouldTelefrag( spot ) ) {
            continue;
        }
        VectorSubtract( spot->s.origin, avoidPoint, delta );
        dist = VectorLength( delta );
        for (i = 0; i < numSpots; i++) {
            if ( dist > list_dist[i] ) {
                if ( numSpots >= 64 )
                    numSpots = 64-1;
                for (j = numSpots; j > i; j--) {
                    list_dist[j] = list_dist[j-1];
                    list_spot[j] = list_spot[j-1];
                }
                list_dist[i] = dist;
                list_spot[i] = spot;
                numSpots++;
                if (numSpots > 64)
                    numSpots = 64;
                break;
            }
        }
        if (i >= numSpots && numSpots < 64) {
            list_dist[numSpots] = dist;
            list_spot[numSpots] = spot;
            numSpots++;
        }
    }
    if (!numSpots) {
        spot = G_Find( NULL, FOFS(classname), "info_player_deathmatch");
        if (!spot)
            G_Error( "Couldn't find a spawn point" );
        VectorCopy (spot->s.origin, origin);
        origin[2] += 9;
        VectorCopy (spot->s.angles, angles);
        return spot;
    }

    // select a random spot from the spawn points furthest away
    rnd = random() * (numSpots / 2);

    VectorCopy (list_spot[rnd]->s.origin, origin);
    origin[2] += 9;
    VectorCopy (list_spot[rnd]->s.angles, angles);

    return list_spot[rnd];
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
gentity_t *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles ) {
    return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles ); 
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles ) {
    gentity_t	*spot;

    spot = NULL;
    while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL) {
        if ( spot->spawnflags & 1 ) {
            break;
        }
    }

    if ( !spot || SpotWouldTelefrag( spot ) ) {
        return SelectSpawnPoint( vec3_origin, origin, angles );
    }

    VectorCopy (spot->s.origin, origin);
    origin[2] += 9;
    VectorCopy (spot->s.angles, angles);

    return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) {
    FindIntermissionPoint();

    VectorCopy( level.intermission_origin, origin );
    VectorCopy( level.intermission_angle, angles );

    return NULL;
}

/*
=======================================================================

BODYQUE

=======================================================================
*/

/*
===============
InitBodyQue
===============
*/
void InitBodyQue (void) {
    int		i;
    gentity_t	*ent;

    level.bodyQueIndex = 0;
    for (i=0; i<BODY_QUEUE_SIZE ; i++) {
        ent = G_Spawn();
        ent->classname = "bodyque";
        ent->neverFree = qtrue;
        level.bodyQue[i] = ent;
    }
}

void UnlinkBodyfromQueforClient( int clientNum )
{
    int i;

    for (i=0; i<BODY_QUEUE_SIZE ; i++) {
        if (  level.bodyQue[i]->r.ownerNum == clientNum  && level.bodyQue[i]->physicsObject )
        {
            // free it
            trap_UnlinkEntity( level.bodyQue[i] );
            level.bodyQue[i]->physicsObject = qfalse;
        }
    }
}
/*
=============
BodySink

After sitting around for five seconds, fall into the ground and dissapear
=============
*/
void BodySink( gentity_t *ent ) {


    if ( level.time - ent->timestamp > 30 * ONE_SECOND ) {
        // the body ques are never actually freed, they are just unlinked
        trap_UnlinkEntity( ent );
        ent->physicsObject = qfalse;
        return;
    }
    ent->nextthink = level.time + 100;
    ent->s.pos.trBase[2] -= 1;
}

/*
=============
BodyBloodPool

Sends tempentity to clients
=============
*/
void BodyBloodPool( gentity_t *ent ) {

    G_AddEvent( ent, EV_BLOODPOOL, 0 );

    if (g_gametype.integer == GT_LTS )
    {
        ent->nextthink = level.time + 600 * ONE_SECOND; // a max of 10 minutes
        ent->timestamp = level.time;
    }
    else
    {
        ent->nextthink = level.time + 30 * ONE_SECOND; // stay 10seconds on ground
        ent->timestamp = level.time;
    }


    ent->think = BodySink;

}

void BodyTouch( gentity_t *ent, gentity_t *other, trace_t *trace )
{
    // when somebody stepped on us, he actually see who he killed and he'll tell the over radio (if it's one of ours)
    if ( !other || !other->client )
        return;

    if ( ent->ns_team == other->client->sess.sessionTeam && ent->bot_chattime == 0 )
    {
        NS_SendStatusMessageToTeam( ent, MS_DEAD, ent->ns_team );
        // BLUTENGEL 20040207 removed, need bot_chattime for
        // bomb placement
        ent->bot_chattime = 1;
    }
    else if ( ent->ns_team == OtherTeam( other->client->sess.sessionTeam ) && ent->count == 0 )
    {
        NS_SendStatusMessageToTeam( ent, MS_DEAD, OtherTeam(ent->ns_team) );
        ent->count = 1;
    }
}

/*
=============
CopyToBodyQue

A player is respawning, so make an entity that looks
just like the existing corpse to leave behind.
=============
*/
void CopyToBodyQue( gentity_t *ent ) {
    int	i;
    gentity_t		*body;
    int			contents;

    trap_UnlinkEntity (ent);

    //	G_Printf("waiting %i dead %i bbox_head %i\n", ent->client->sess.waiting, (ent->client->ps.pm_type == PM_DEAD ),( !Q_stricmp( ent->classname, "player_bbox_head") ) );
    if (
        ( !ent->client->sess.waiting || ent->client->ps.pm_type != PM_DEAD ) &&
        g_gametype.integer == GT_LTS
    )
        return;

    // if client is in a nodrop area, don't leave the body
    contents = trap_PointContents( ent->s.origin, -1 );

    if ( contents & CONTENTS_NODROP ) {
        return;
    }

    // grab a body que and cycle to the next one
    body = level.bodyQue[ level.bodyQueIndex ];
    level.bodyQueIndex = (level.bodyQueIndex + 1) % BODY_QUEUE_SIZE;

    ent->client->ps.eFlags ^= EF_TELEPORT_BIT;
    trap_UnlinkEntity (body);

    body->s = ent->s;
    body->s.eFlags = EF_DEAD;		// clear EF_TALK, etc

    //	body->s.powerups = 0;	// clear powerups
    body->s.loopSound = 0;	// clear lava burning
    body->s.weapon = WP_NONE;
    body->s.number = body - g_entities;
    body->timestamp = level.time;
    body->physicsObject = qtrue;
    body->physicsBounce = 0;		// don't bounce
    if ( body->s.groundEntityNum == ENTITYNUM_NONE ) {
        body->s.pos.trType = TR_GRAVITY;
        body->s.pos.trTime = level.time;
        VectorCopy( ent->client->ps.velocity, body->s.pos.trDelta );
    } else {
        body->s.pos.trType = TR_STATIONARY;
    }
    body->s.event = 0;

    for (i=WP_NUM_WEAPONS-1;i>0;i--)
    {
        if (BG_GotWeapon( i , ent->client->ps.stats ) )
            if ( BG_IsPrimary( i ) || BG_IsSecondary( i ) )
                body->s.frame |= 1 << i;
    }

    // change the animation to the last-frame only, so the sequence
    // doesn't repeat anew for the body
    switch ( body->s.legsAnim & ~ANIM_TOGGLEBIT ) {
    case BOTH_DEATH_LEGS:
    case BOTH_DEAD_LEGS:
        body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_LEGS;
        break;
    case BOTH_DEATH_BACK:
    case BOTH_DEAD_BACK:
        body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_BACK;
        break;
    case BOTH_DEATH_NECK:
    case BOTH_DEAD_NECK:
        body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_NECK;
        break;
    case BOTH_DEATH_BLEED:
    case BOTH_DEAD_BLEED:
    default:
        body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_BLEED;
        break;
    case BOTH_DEATH_STOMACH:
    case BOTH_DEAD_STOMACH:
        body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_STOMACH;
        break;
    case BOTH_DEATH_CHEST:
    case BOTH_DEAD_CHEST:
        body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_CHEST;
        break;
    case BOTH_DEATH_FACE:
    case BOTH_DEAD_FACE:
        body->s.torsoAnim = body->s.legsAnim = BOTH_DEAD_FACE;
        break;
    }

    body->r.svFlags = ent->r.svFlags;

    VectorCopy (ent->r.mins, body->r.mins);
    VectorCopy (ent->r.maxs, body->r.maxs);

    VectorCopy (ent->r.absmin, body->r.absmin);
    VectorCopy (ent->r.absmax, body->r.absmax);

    body->clipmask = CONTENTS_SOLID | CONTENTS_PLAYERCLIP;
    body->r.contents = CONTENTS_CORPSE;
    body->r.ownerNum = ent->s.number;



    body->nextthink = level.time + 250;
    body->think = BodyBloodPool;

    //	body->die = body_die;

    // don't take more damage if already gibbed
    body->takedamage = qtrue;

    if (g_gametype.integer == GT_LTS )
    {
        body->touch = BodyTouch;
        body->ns_team = ent->client->sess.sessionTeam;
    }

    VectorCopy ( body->s.pos.trBase, body->r.currentOrigin );
    VectorCopy ( ent->client->ps.viewangles, body->r.currentAngles );
    VectorCopy ( ent->client->ps.viewangles, body->s.apos.trBase ) ;
    // VectorCopy ( ent-> body->s.apos.trBase );
    trap_LinkEntity (body);
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( gentity_t *ent, vec3_t angle ) {
    int			i;

    // set the delta angle
    for (i=0 ; i<3 ; i++) {
        int		cmdAngle;

        cmdAngle = ANGLE2SHORT(angle[i]);
        ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
    }
    VectorCopy( angle, ent->s.angles );
    VectorCopy (ent->s.angles, ent->client->ps.viewangles);
}

/*
================
respawn
================
*/
void respawn( gentity_t *ent ) {
    //	gentity_t	*tent;


    CopyToBodyQue(ent);
    ClientSpawn(ent);

    /* defcon-X: do not....
    // add a teleportation effect
    tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
    tent->s.clientNum = ent->s.clientNum;
    */
}

/*
================
TeamCount

Returns number of players on a team
================
*/
team_t TeamCount( int ignoreClientNum, int team ) {
    int		i;
    int		count = 0;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( i == ignoreClientNum ) {
            continue;
        }
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        // Navy Seals ++
        if ( g_gametype.integer == GT_LTS && level.clients[i].pers.connected != CON_CONNECTED )
            continue;

        if ( level.clients[i].pers.nsPC.playerclass <= CLASS_NONE)
            continue;
        // Navy Seals --
        if ( level.clients[i].sess.sessionTeam == team ) {
            count++;
        }
    }

    return count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) {
    int		i;

    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( level.clients[i].sess.sessionTeam == team ) {
            if ( level.clients[i].sess.teamLeader )
                return i;
        }
    }

    return -1;
}


/*
================
PickTeam

================
*/
team_t PickTeam( int ignoreClientNum ) {
    int		counts[TEAM_NUM_TEAMS];

    counts[TEAM_BLUE] = TeamCount( ignoreClientNum, TEAM_BLUE );
    counts[TEAM_RED] = TeamCount( ignoreClientNum, TEAM_RED );

    if ( counts[TEAM_BLUE] > counts[TEAM_RED] ) {
        return TEAM_RED;
    }
    if ( counts[TEAM_RED] > counts[TEAM_BLUE] ) {
        return TEAM_BLUE;
    }
    // equal team count, so join the team with the lowest score
    if ( level.teamScores[TEAM_BLUE] > level.teamScores[TEAM_RED] ) {
        return TEAM_RED;
    }
    return TEAM_BLUE;
}

/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
    int		len, colorlessLen;
    char	ch;
    char  oldch;
    char	*p;
    int		spaces;

    //save room for trailing null byte
    outSize--;

    len = 0;
    colorlessLen = 0;
    p = out;
    *p = 0;
    spaces = 0;
    oldch = ' ';

    while( 1 ) {
        ch = *in++;
        if( !ch ) {
            break;
        }

        // BLUTENGEL
        //
        // do not allow special characters (could be produced by hex-editing
        // the config file!)
        if ( (ch < '!') || (ch > '~') ) continue;

        // don't allow leading spaces
        //
        // BLUTENGEL
        // what about colors at the beginning, after that
        // spaces are allowed???? weird, removing that
        // if( !*p && ch == ' ' ) {
        if (!colorlessLen && ch == ' ') {
            continue;
        }

        // check colors
        if( ch == Q_COLOR_ESCAPE ) {
            // solo trailing carat is not a color prefix
            if( !*in ) {
                break;
            }

            // don't allow black in a name, period
            //
            // BLUTENGEL:
            // ignore ^^
            if( (*in == '^') ) {
                in++;
                continue;
            }

            // make sure room in dest for both chars
            if( len > outSize - 2 ) {
                break;
            }

            *out++ = ch;
            *out++ = *in++;
            len += 2;
            continue;
        }

        // don't allow too many consecutive spaces
        //
        // BLUTENGEL
        // removed repeated characters, not only spaces
        if( ch == oldch ) {
            spaces++;
            if( spaces > 3 ) {
                continue;
            }
        }
        else {
            spaces = 0;
        }

        if( len > outSize - 1 ) {
            break;
        }

        *out++ = ch;
        oldch = ch;
        colorlessLen++;
        len++;
    }
    *out = 0;

    // don't allow empty names
    if( *p == 0 || colorlessLen == 0 ) {
        Q_strncpyz(p, "^1U^0ber^1N^0oob", outSize );
    }
}

qboolean NS_ActiveRound( void );

/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call trap_SetUserinfo
if desired.
============
*/

void ClientUserinfoChanged( int clientNum ) {
    gentity_t *ent;
    int		team,i;//, health;
    char	*s;
    char	e_head[MAX_QPATH],e_eyes[MAX_QPATH],e_mouth[MAX_QPATH];
    char	custom[MAX_QPATH];
    char	oldname[MAX_STRING_CHARS];
    gclient_t	*client;
    char	c1[MAX_INFO_STRING];
    char	userinfo[MAX_INFO_STRING];

    char	modelName[MAX_QPATH];
    char	headName[MAX_QPATH];

    ent = g_entities + clientNum;
    client = ent->client;

    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

    // check for malformed or illegal info strings
    if ( !Info_Validate(userinfo) ) {
        strcpy (userinfo, "\\name\\badinfo");
    }

    // check for local client
    s = Info_ValueForKey( userinfo, "ip" );
    if ( !strcmp( s, "localhost" ) ) {
        client->pers.localClient = qtrue;
    }

    // Is anti-lag turned on?
    s = Info_ValueForKey ( userinfo, "cg_antiLag" );
    client->pers.antiLag = atoi( s )?qtrue:qfalse;

    // Is auto reload active
    s = Info_ValueForKey ( userinfo, "cg_autoReload" );
    client->pers.autoReload = atoi( s )?qtrue:qfalse;

    // Is autobandage active?
    s = Info_ValueForKey ( userinfo, "cg_useBandage" );
    client->pers.useBandage = atoi( s )?qtrue:qfalse;

    // Is autobandage active?
    i = atoi ( Info_ValueForKey ( userinfo, "cg_radarUpdate" ) );

    if ( i < 500 )
        i = 500;

    client->pers.radarUpdateTime = i;

    // get the quitmessage
    s = Info_ValueForKey ( userinfo, "quitmsg" );
    Q_strncpyz( client->pers.quitMsg, s, sizeof( client->pers.quitMsg ) );


    // set name
    if ( !NS_ActiveRound( ) || client->pers.connected == CON_CONNECTING )
    {
        Q_strncpyz ( oldname, client->pers.netname, sizeof( oldname ) );
        s = Info_ValueForKey (userinfo, "name");

        // BLUTENGEL:
        //
        // really sick, the input string s will be manipulated
        ClientCleanName( s, client->pers.netname, sizeof(client->pers.netname) );

        if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
            if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
                Q_strncpyz( client->pers.netname, "scoreboard", sizeof(client->pers.netname) );
            }
        }

        if ( client->pers.connected == CON_CONNECTED ) {
            if ( strcmp( oldname, client->pers.netname ) ) {
                trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname,
                                               client->pers.netname) );
            }
        }
    }
   
    team = client->sess.sessionTeam; 

    Q_strncpyz( custom, "0",sizeof(custom ) );

    {	// use the headskin we want
        char		*slash;

        Q_strncpyz( modelName, Info_ValueForKey (userinfo, "model"), sizeof( modelName ) );

        slash = strchr( modelName, '/' );
        if ( !slash ) {
            // headskin didn not include a skin name 
			Q_strncpyz( headName, "curtis", sizeof( headName ) ); 
        } else {
            Q_strncpyz( headName, slash + 1, sizeof( headName ) );
            // truncate modelName
            *slash = 0;
        }

    } 
	
    // character
    s = Info_ValueForKey ( userinfo, "character" );
    {
        if ( strlen(s) < 7 ) {
            s = "C111111";
        }

        // only recalc the character if it's changed
        if ( Q_stricmp( client->pers.character, s ) ) // it's not the same
        {
            Q_strncpyz( client->pers.character, s, sizeof( client->pers.character ) );
            NS_RecalcCharacter( ent );
        }
    }

    Q_strncpyz( e_eyes, Info_ValueForKey (userinfo, "e_eyes"), sizeof( e_eyes ) );
    Q_strncpyz( e_mouth, Info_ValueForKey (userinfo, "e_mouth"), sizeof( e_mouth ) );
    Q_strncpyz( e_head, Info_ValueForKey (userinfo, "e_head"), sizeof( e_head ) );

    // check and confirm (or change) the users looks!
    NS_ValidatePlayerLooks( client->sess.sessionTeam,
                            e_head, e_eyes, e_mouth, modelName, headName );

    // for the player to the VIP model if he's the VIP
    // do this after validating the looks, so he won't change it back to seal
    if ( client->ns.is_vip )
    {
        Q_strncpyz( modelName, "vip_male", sizeof(modelName) );
        Q_strncpyz( headName, "vip", sizeof(headName) );
    }

    // apply the "hardcoded" looks
    if ( client->pers.nsPC.is_defconx_cap )
        Com_sprintf( e_head, sizeof(e_head), "cap_tm_defcon" );
    else if ( client->pers.nsPC.is_defconx_hat )
        Com_sprintf( e_head, sizeof(e_head), "sealhat_tm_defcon" );
    else if ( client->pers.nsPC.is_democritus )
    {
        if ( client->pers.nsPC.is_democritus == 2 )
            Com_sprintf( e_mouth, sizeof(e_mouth), "dcs_mask2" );
        else
            Com_sprintf( e_mouth, sizeof(e_mouth), "dcs_mask" );
    }
    else if ( client->pers.nsPC.is_hoak )
    {
        if ( client->pers.nsPC.is_hoak == 2 )
            Com_sprintf( e_eyes, sizeof(e_eyes), "nvg_hoak" );
        else
            Com_sprintf( e_eyes, sizeof(e_eyes), "mask_hoak" );
    }
    else if ( client->pers.nsPC.is_ogun )
    {
        if ( client->pers.nsPC.is_ogun == 2 )
            Com_sprintf( e_eyes, sizeof(e_eyes), "glasses_ogun2" );
        else
            Com_sprintf( e_eyes, sizeof(e_eyes), "glasses_ogun" );
    }
    // Navy Seals --

    // teamInfo
    s = Info_ValueForKey( userinfo, "teamoverlay" );
    if ( ! *s || atoi( s ) != 0 ) {
        client->pers.teamInfo = qtrue;
    } else {
        client->pers.teamInfo = qfalse;
    }

    // colors
    strcpy(c1, Info_ValueForKey( userinfo, "color" ));

    // send over a subset of the userinfo keys so other clients can
    // print scoreboards, display models, and play custom sounds
    if ( ent->r.svFlags & SVF_BOT ) {
        s = va("n\\%s\\t\\%i\\model\\%s/%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\e_eyes\\%s\\e_head\\%s\\e_mouth\\%s",
               client->pers.netname, client->sess.sessionTeam, modelName, headName, c1,
               client->pers.maxHealth, client->sess.wins, client->sess.losses,
               Info_ValueForKey( userinfo, "skill" ), e_eyes,e_head,e_mouth );
    } else {
        s = va("n\\%s\\t\\%i\\model\\%s/%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\e_eyes\\%s\\e_head\\%s\\e_mouth\\%s",
               client->pers.netname, client->sess.sessionTeam, modelName, headName, c1,
               client->pers.maxHealth, client->sess.wins, client->sess.losses , e_eyes,e_head,e_mouth );
    }

    trap_SetConfigstring( CS_PLAYERS+clientNum, s );

    G_LogPrintf( "ClientUserinfoChanged: [%i] \"%s\"\n", clientNum, s );
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
    char		*value;
    //	char		*areabits;
    gclient_t	*client;
    char		userinfo[MAX_INFO_STRING];
    gentity_t	*ent;

    ent = &g_entities[ clientNum ];

    trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

    // check to see if they are on the banned IP list
    value = Info_ValueForKey (userinfo, "ip");
    if ( G_FilterPacket( value ) ) {
        return "You are banned from this server.";
    }

    // check for a password
    value = Info_ValueForKey (userinfo, "password");
    if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
            strcmp( g_password.string, value) != 0) {
        return "Invalid password";
    }

    // they can connect
    ent->client = level.clients + clientNum;
    client = ent->client;

    //	areabits = client->areabits;

    memset( client, 0, sizeof(*client) );

    client->pers.connected = CON_CONNECTING;

    // read or initialize the session data
    if ( firstTime || level.newSession ) {
        G_InitSessionData( client, userinfo );
    }
    G_ReadSessionData( client );

    // get and distribute relevent paramters
    G_LogPrintf( "ClientConnect: %i\n", clientNum );
    ClientUserinfoChanged( clientNum );

    // don't do the "xxx connected" messages if they were caried over from previous level
    if ( firstTime ) {
        trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname) );
    }

    if ( g_gametype.integer >= GT_TEAM &&
            client->sess.sessionTeam != TEAM_SPECTATOR ) {
        BroadcastTeamChange( client, -1 );
    }

    // count current clients and rank for scoreboard
    CalculateRanks();

    if ( client->sess.sessionTeam == TEAM_SPECTATOR )
        client->sess.waiting = qtrue;

    // for statistics
    //	client->areabits = areabits;
    //	if ( !client->areabits )
    //		client->areabits = G_Alloc( (trap_AAS_PointReachabilityAreaIndex( NULL ) + 7) / 8 );

    return NULL;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns
============
*/
void ClientBegin( int clientNum ) {
    gentity_t	*ent;
    gclient_t	*client;
    gentity_t	*tent;
    int			flags;

    ent = g_entities + clientNum;

    client = level.clients + clientNum;

    if ( ent->r.linked ) {
        trap_UnlinkEntity( ent );
    }
    G_InitGentity( ent );
    ent->touch = 0;
    ent->pain = 0;
    ent->client = client;

    client->pers.connected = CON_CONNECTED;
    client->pers.enterTime = level.time;
    client->pers.teamState.state = TEAM_BEGIN;
    client->pers.lmt = level.time; // remember this as the last message sent to client

    // save eflags around this, because changing teams will
    // cause this to happen with a valid entity, and we
    // want to make sure the teleport bit is set right
    // so the viewpoint doesn't interpolate through the
    // world to the new position
    flags = client->ps.eFlags;
    memset( &client->ps, 0, sizeof( client->ps ) );
    client->ps.eFlags = flags;

    // force to spectator
    if ( client->sess.sessionTeam == TEAM_SPECTATOR ||
            g_gametype.integer == GT_LTS )
        client->sess.waiting = qtrue;

    NS_ClearInventory( ent );

    if ( client->sess.sessionTeam != TEAM_SPECTATOR )
        SetClass( ent, CLASS_CUSTOM );

    ent->health = 0;

    // locate ent at a spawn point
    ClientSpawn( ent );

    if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
        // send event
        tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_IN );
        tent->s.clientNum = ent->s.clientNum;

        trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
    }

    // Navy Seals --
    G_LogPrintf( "ClientBegin: %i\n", clientNum );

    // count current clients and rank for scoreboard
    CalculateRanks();

    if ( g_gametype.integer == GT_TEAM )
    {
        gitem_t *pri,*sec;
        //PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: weapons changed for client %i\n", clientNum);
        // primary weapon
        if ( ent->client->sess.sessionTeam == TEAM_BLUE )
            ent->client->pers.nsInven.primaryweapon = WP_MAC10;
        else
            ent->client->pers.nsInven.primaryweapon = WP_MP5;

        // secondary
        if (ent->client->sess.sessionTeam == TEAM_BLUE)
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

        NS_NavySeals_ClientInit( ent, qtrue );
    }
}

/*
===========
ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
void ClientSpawn(gentity_t *ent) {
    int		index;
    vec3_t	spawn_origin, spawn_angles;
    gclient_t	*client;
    int		i;
    clientPersistant_t	saved;
    clientSession_t		savedSess;
    int		persistant[MAX_PERSISTANT];
    gentity_t	*spawnPoint;
    int		flags;
    int		savedPing;
    //	char	*savedAreaBits;
    int		accuracy_hits, accuracy_shots;
    int		savedEvents[MAX_PS_EVENTS];
    int		eventSequence;
    char	userinfo[MAX_INFO_STRING];
    int		rewards,kills,knifekills;


    index = ent - g_entities;
    client = ent->client;

    // select spawnpoint
    spawnPoint = SelectSpawnPoint (
                     client->ps.origin,
                     spawn_origin, spawn_angles);

    // find a spawn point
    // do it before setting health back up, so farthest
    // ranging doesn't count this client

    // if we just died, just spawn directly where we died so we can track the following action

    if ( client->sess.waiting && ent->r.contents == CONTENTS_CORPSE && ( client->sess.sessionTeam == TEAM_RED || client->sess.sessionTeam == TEAM_BLUE ) )
    {
        // spawn right in place of death
        VectorCopy( client->ps.origin, spawn_origin );
        spawn_origin[2] += 15;
    }
    else if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
        spawnPoint = SelectSpectatorSpawnPoint (
                         spawn_origin, spawn_angles);
    } else if (g_gametype.integer >= GT_TEAM ) {
        // all base oriented team games use the team spawn points
        spawnPoint = SelectTeamSpawnPoint (
                         client->sess.sessionTeam,
                         client->pers.teamState.state,
                         spawn_origin, spawn_angles);
    } else {
        do {
            // the first spawn should be at a good looking spot
            if ( !client->pers.initialSpawn && client->pers.localClient ) {
                client->pers.initialSpawn = qtrue;
                spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles );
            } else {
                // don't spawn near existing origin if possible
                spawnPoint = SelectSpawnPoint (
                                 client->ps.origin,
                                 spawn_origin, spawn_angles);
            }

            // Tim needs to prevent bots from spawning at the initial point
            // on q3dm0...
            if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
                continue;	// try again
            }
            // just to be symetric, we have a nohumans option...
            if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
                continue;	// try again
            }

            break;

        } while ( 1 );
    }
    client->pers.teamState.state = TEAM_ACTIVE;

    // always clear the kamikaze flag
    //	ent->s.eFlags &= ~EF_KAMIKAZE;

    // toggle the teleport bit so the client knows to not lerp
    // and never clear the voted flag
    flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
    flags ^= EF_TELEPORT_BIT;

    // clear everything but the persistant data
    saved = client->pers;
    savedSess = client->sess;
    savedPing = client->ps.ping;
    //	savedAreaBits = client->areabits;
    accuracy_hits = client->accuracy_hits;
    accuracy_shots = client->accuracy_shots;
    for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
        persistant[i] = client->ps.persistant[i];
        //if (i == PERS_TEAM) PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: Saved Team %i\n", persistant[i]);
    }
    // also save the predictable events otherwise we might get double or dropped events
    for (i = 0; i < MAX_PS_EVENTS; i++) {
        savedEvents[i] = client->ps.events[i];
    }
    eventSequence = client->ps.eventSequence;

    //memcpy( &rewards, &client->ns.rewards, sizeof(int) );
    //memcpy( &kills, &client->ns.num_killed, sizeof(int) );
    //memcpy( &knifekills, &client->knife_kills, sizeof(int) );
    rewards = client->ns.rewards;
    kills = client->ns.num_killed;
    knifekills = client->knife_kills;

    memset (client, 0, sizeof(*client));

    client->pers = saved;
    client->sess = savedSess;
    client->ps.ping = savedPing;
    //client->areabits = savedAreaBits;
    client->accuracy_hits = accuracy_hits;
    client->accuracy_shots = accuracy_shots;
    client->lastkilled_client = -1;

    client->ns.rewards = rewards;
    client->knife_kills = knifekills;
    client->ns.num_killed = kills;

    for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
        client->ps.persistant[i] = persistant[i];
        //if (i == PERS_TEAM) PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: Restored Team %i\n", client->ps.persistant[i]);
    }
    for (i = 0; i < MAX_PS_EVENTS; i++) {
        client->ps.events[i] = savedEvents[i];
    }
    client->ps.eventSequence = eventSequence;
    // increment the spawncount so the client will detect the respawn
    client->ps.persistant[PERS_SPAWN_COUNT]++;
    //PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: game/g_client.c ClientSpawn %i %i  (%i:%i:%i:%i:%i:%i)\n",
    //client->ps.persistant[PERS_TEAM],
    //client->sess.sessionTeam,
    //client->ps.persistant[PERS_ACCURACY],
    //client->ps.persistant[PERS_STRENGTH],
    //client->ps.persistant[PERS_SPEED],
    //client->ps.persistant[PERS_STAMINA],
    //client->ps.persistant[PERS_STEALTH],
    //client->ps.persistant[PERS_TECHNICAL]);
    client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;

    client->airOutTime = level.time + 12000;

    trap_GetUserinfo( index, userinfo, sizeof(userinfo) );

    // set max health based on stamina
    client->pers.maxHealth = 100 + ( client->pers.nsPC.stamina * 3 );

    // clear entity values
    client->ps.eFlags = flags;

    ent->s.groundEntityNum = ENTITYNUM_NONE;
    ent->client = &level.clients[index];
    ent->takedamage = qtrue;
    ent->inuse = qtrue;
    ent->classname = "player";
    ent->r.contents = CONTENTS_BODY;
    ent->clipmask = MASK_PLAYERSOLID;
    ent->die = player_die;
    ent->waterlevel = 0;
    ent->watertype = 0;
    ent->flags = 0;


    VectorCopy (playerMins, ent->r.mins);
    VectorCopy (playerMaxs, ent->r.maxs);

    client->ps.clientNum = index;

    // Navy Seals ++

    // if we're a late-time joiner, lock our weapons
    if ( g_gametype.integer == GT_LTS && GameState == STATE_OPEN )
    {
        client->ps.eFlags |= EF_WEAPONS_LOCKED;
        NS_SendPlayersStatusToAllPlayers( client->ps.clientNum , MS_HEALTH5 );
    }
    else if ( g_gametype.integer == GT_TEAM )
        NS_SendPlayersStatusToAllPlayers(  client->ps.clientNum, MS_HEALTH5 );
    else
        NS_SendPlayersStatusToAllPlayers( client->ps.clientNum, MS_DEAD );

    // Navy Seals --

    // set max health
    ent->health = client->ps.stats[STAT_HEALTH] = client->pers.maxHealth;

    G_SetOrigin( ent, spawn_origin );
    VectorCopy( spawn_origin, client->ps.origin );

    // the respawned flag will be cleared after the attack and jump keys come up
    client->ps.pm_flags |= PMF_RESPAWNED;

    trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
    SetClientViewAngle( ent, spawn_angles );

    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR || client->sess.waiting ) {
        ent->r.svFlags |= SVF_NOCLIENT;
    } else {
        G_KillBox( ent );
        ent->r.svFlags &= ~ SVF_NOCLIENT;
        trap_LinkEntity (ent);
        // Navy Seals ++
        // force the base weapon up
        NS_NavySeals_ClientInit(ent, qtrue);
        // Navy Seals --
    }
    // Navy Seals ++

    // Navy Seals --
    // don't allow full run speed for a bit
    client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
    client->ps.pm_time = 100;

    client->respawnTime = level.time;
    client->inactivityTime = level.time + g_inactivity.integer * 1000;
    client->latched_buttons = 0;

    if (!client->sess.waiting)
    {
        if ( level.intermissiontime )
        {
            MoveClientToIntermission( ent );
        }
        else
        {
            // fire the targets of the spawn point
            if (spawnPoint)
                G_UseTargets( spawnPoint, ent );

            // select the highest weapon number available, after any
            // spawn given items have fired
            if ( g_gametype.integer < GT_TEAM )
            {
                client->ps.weapon = 1;
                for ( i = WP_SMOKE - 1 ; i > 0 ; i-- )
                {
                    if ( BG_GotWeapon(i, client->ps.stats ) )
                    {
                        client->ps.weapon = i;
                        break;
                    }
                }
            }
        }
    }

    // run a client frame to drop exactly to the floor,
    // initialize animations and other things
    client->ps.commandTime = level.time - 100;
    ent->client->pers.cmd.serverTime = level.time;
    ClientThink( ent-g_entities );

    // positively link the client, even if the command times are weird
    if ( ent->client->sess.sessionTeam != TEAM_SPECTATOR && !ent->client->sess.waiting ) {
        BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
        VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
        trap_LinkEntity( ent );
    }

    // run the presend to set anything else
    ClientEndFrame( ent );

    // clear entity state values
    BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
}


/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call trap_DropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) {
    gentity_t	*ent;
    gentity_t	*tent;
    int			i;

    // cleanup if we are kicking a bot that
    // hasn't spawned yet
    ent = g_entities + clientNum;
    if ( !ent->client ) {
        return;
    }


    if ( strlen( ent->client->pers.quitMsg ) > 0 )
        PrintMsg( NULL, "%s "S_COLOR_WHITE"quit (%s"S_COLOR_WHITE")\n", ent->client->pers.netname,ent->client->pers.quitMsg );
    else
        PrintMsg( NULL, "%s "S_COLOR_WHITE"quit\n", ent->client->pers.netname );

    // send effect if they were completely connected
    if ( !level.intermissiontime ) // only do this when we're not on the intermission
        if (
            ent->client->pers.connected == CON_CONNECTED &&
            ent->client->sess.sessionTeam != TEAM_SPECTATOR &&
            ! (ent->client->ps.pm_flags & PMF_FOLLOW )
        ) {
            tent = G_TempEntity( ent->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
            tent->s.clientNum = ent->s.clientNum;

            // They don't get to take powerups with them!
            // Especially important for stuff like CTF flags
            TossClientItems( ent );
        }

    // stop any following clients
    for ( i = 0 ; i < level.maxclients ; i++ ) {
        if ( level.clients[i].sess.spectatorState == SPECTATOR_FOLLOW
                && level.clients[i].sess.spectatorClient == clientNum ) {
            StopFollowing( &g_entities[i] );
        }
        if ( level.clients[i].sess.waiting && level.clients[i].sess.spectatorClient == clientNum )
            StopFollowing( &g_entities[i] );
    }

    G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

    trap_UnlinkEntity (ent);
    ent->s.modelindex = 0;
    ent->inuse = qfalse;
    ent->classname = "disconnected";
    ent->client->pers.connected = CON_DISCONNECTED;
    //PrintMsg(NULL, S_COLOR_RED"BLUTENGEL DEBUG MESSAGE: g_client.c ClientDisconnect changed PERS_TEAM to TEAM_FREE\n");
    ent->client->ps.persistant[PERS_TEAM] = TEAM_FREE;
    ent->client->sess.sessionTeam = TEAM_FREE;

    trap_SetConfigstring( CS_PLAYERS + clientNum, "");

    CalculateRanks();
}


