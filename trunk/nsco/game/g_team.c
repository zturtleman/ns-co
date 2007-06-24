// Copyright (C) 1999-2000 Id Software, Inc.
//
// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

void Team_InitGame( void ) {

    switch( g_gametype.integer ) {
    default:
        break;
    }
}

int OtherTeam(int team) {
    if (team==TEAM_RED)
        return TEAM_BLUE;
    else if (team==TEAM_BLUE)
        return TEAM_RED;
    return team;
}

const char *TeamName(int team)  {

    if (team==TEAM_RED)
        return S_COLOR_BLUE"Seals"S_COLOR_WHITE;
    else if (team==TEAM_BLUE)
        return S_COLOR_RED"Tangos"S_COLOR_WHITE;
    else if (team==TEAM_SPECTATOR)
        return "SPECTATOR";
    return "FREE";
}

const char *OtherTeamName(int team) {

    if (team==TEAM_BLUE)
        return S_COLOR_BLUE"Seals"S_COLOR_WHITE;
    else if (team==TEAM_RED)
        return S_COLOR_RED"Tangos"S_COLOR_WHITE;
    else if (team==TEAM_SPECTATOR)
        return "SPECTATOR";
    return "FREE";
}

const char *TeamColorString(int team) {
    if (team==TEAM_RED)
        return S_COLOR_BLUE;
    else if (team==TEAM_BLUE)
        return S_COLOR_RED;
    else if (team==TEAM_SPECTATOR)
        return S_COLOR_YELLOW;
    return S_COLOR_WHITE;
}

// NULL for everyone
void QDECL PrintMsg( gentity_t *ent, const char *fmt, ... ) {
    char		msg[1024];
    va_list		argptr;
    char		*p;

    va_start (argptr,fmt);
    if (vsprintf (msg, fmt, argptr) > sizeof(msg)) {
        G_Error ( "PrintMsg overrun" );
    }
    va_end (argptr);

    // double quotes are bad
    while ((p = strchr(msg, '"')) != NULL)
        *p = '\'';

    trap_SendServerCommand ( ( (ent == NULL) ? -1 : ent-g_entities ), va("print \"%s\"", msg ));
}

/*
==============
AddTeamScore

used for gametype > GT_TEAM
for gametype GT_TEAM the level.teamScores is updated in AddScore in g_combat.c
==============
*/
void AddTeamScore(vec3_t origin, int team, int score) {
    gentity_t	*te;

    te = G_TempEntity(origin, EV_GLOBAL_TEAM_SOUND );
    te->r.svFlags |= SVF_BROADCAST;

    if ( team == TEAM_RED ) {/*
            if ( level.teamScores[ TEAM_RED ] + score == level.teamScores[ TEAM_BLUE ] ) {
            //teams are tied sound
            te->s.eventParm = GTS_TEAMS_ARE_TIED;
            }
            else*/
        if ( level.teamScores[ TEAM_RED ] <= level.teamScores[ TEAM_BLUE ] &&
                level.teamScores[ TEAM_RED ] + score > level.teamScores[ TEAM_BLUE ]) {
            // red took the lead sound
            te->s.eventParm = GTS_REDTEAM_TOOK_LEAD;
        }
        else {
            // red scored sound
            te->s.eventParm = GTS_REDTEAM_SCORED;
        }
    }
    else {
        /*
        if ( level.teamScores[ TEAM_BLUE ] + score == level.teamScores[ TEAM_RED ] ) {
        //teams are tied sound
        te->s.eventParm = GTS_TEAMS_ARE_TIED;
        }
        else*/ if ( level.teamScores[ TEAM_BLUE ] <= level.teamScores[ TEAM_RED ] &&
                    level.teamScores[ TEAM_BLUE ] + score > level.teamScores[ TEAM_RED ]) {
            // blue took the lead sound
            te->s.eventParm = GTS_BLUETEAM_TOOK_LEAD;
        }
        else  {
            // blue scored sound
            te->s.eventParm = GTS_BLUETEAM_SCORED;
        }
    }
    level.teamScores[ team ] += score;
}

/*
==============
OnSameTeam
==============
*/
qboolean OnSameTeam( gentity_t *ent1, gentity_t *ent2 ) {
    if ( !ent1->client || !ent2->client ) {
        return qfalse;
    }

    if ( g_gametype.integer < GT_TEAM ) {
        return qfalse;
    }

    if ( ent1->client->sess.sessionTeam == ent2->client->sess.sessionTeam ) {
        return qtrue;
    }


    return qfalse;
}

void Pickup_Briefcase(gentity_t *ent, gentity_t *other);
void Return_Briefcase(gentity_t *ent, gentity_t *other);

void Team_FreeEntity(gentity_t *ent)
{
    // this shouldn't happen!
    if (ent->item->giTag == PW_BRIEFCASE)
        Reset_Briefcase();
}

void Team_CheckDroppedItem( gentity_t *dropped )
{
    //	if ( dropped->item->giTag == PW_BRIEFCASE )
    //		PrintMsg(NULL, "The Briefcase has been dropped.\n");
}

int Pickup_Team( gentity_t *ent, gentity_t *other ) {
    // figure out what team this flag is
    if ( !Q_stricmp(ent->classname, "team_briefcase") )
    {
        Pickup_Briefcase( ent, other );
        return -1; // never respawn
    }
    else if ( !Q_stricmp(ent->classname, "team_briefcase_return") && other->client->ps.powerups[PW_BRIEFCASE] )
    {
        Return_Briefcase( ent, other );
        return 0;
    }

    return 0;
}

/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/
gentity_t *Team_GetLocation(gentity_t *ent)
{
    gentity_t		*eloc, *best;
    float			bestlen, len;
    vec3_t			origin;

    best = NULL;
    bestlen = 3*8192.0*8192.0;

    VectorCopy( ent->r.currentOrigin, origin );

    for (eloc = level.locationHead; eloc; eloc = eloc->nextTrain) {
        len = ( origin[0] - eloc->r.currentOrigin[0] ) * ( origin[0] - eloc->r.currentOrigin[0] )
              + ( origin[1] - eloc->r.currentOrigin[1] ) * ( origin[1] - eloc->r.currentOrigin[1] )
              + ( origin[2] - eloc->r.currentOrigin[2] ) * ( origin[2] - eloc->r.currentOrigin[2] );

        if ( len > bestlen ) {
            continue;
        }

        if ( !trap_InPVS( origin, eloc->r.currentOrigin ) ) {
            continue;
        }

        bestlen = len;
        best = eloc;
    }

    return best;
}


/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/
qboolean Team_GetLocationMsg(gentity_t *ent, char *loc, int loclen)
{
    gentity_t *best;

    best = Team_GetLocation( ent );

    if ( ent->client->sess.waiting )
        return qfalse;

    if (!best)
        return qfalse;

    if (best->count) {
        if (best->count < 0)
            best->count = 0;
        if (best->count > 7)
            best->count = 7;
        Com_sprintf(loc, loclen, "%c%c%s" S_COLOR_WHITE, Q_COLOR_ESCAPE, best->count + '0', best->message );
    } else
        Com_sprintf(loc, loclen, "%s", best->message);

    return qtrue;
}


/*---------------------------------------------------------------------------*/




/*---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*/

/*QUAKED team_CTF_redplayer (1 0 0) (-16 -16 -16) (16 16 32)
Only in CTF games.  Red players spawn here at game start.
*/
void SP_team_CTF_redplayer( gentity_t *ent ) {
}


/*QUAKED team_CTF_blueplayer (0 0 1) (-16 -16 -16) (16 16 32)
Only in CTF games.  Blue players spawn here at game start.
*/
void SP_team_CTF_blueplayer( gentity_t *ent ) {
}


/*QUAKED team_CTF_redspawn (1 0 0) (-16 -16 -24) (16 16 32)
potential spawning position for red team in CTF games.
Targets will be fired when someone spawns in on them.
*/
void SP_team_CTF_redspawn(gentity_t *ent) {
}

/*QUAKED team_CTF_bluespawn (0 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for blue team in CTF games.
Targets will be fired when someone spawns in on them.
*/
void SP_team_CTF_bluespawn(gentity_t *ent) {
}

