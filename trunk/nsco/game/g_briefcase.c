// Copyright (C) 2000 Team Mirage
////
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

void Pickup_Briefcase( gentity_t *ent, gentity_t *other ) {
    other->client->ps.powerups[PW_BRIEFCASE] = 1;

    G_LogPrintf( "OBJECTIVE: [%i] \"%s\" picked up a briefcase\n", other->client->ps.clientNum, other->client->pers.netname );

    // play sound
    /*
    G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

    if ( other->client->sess.sessionTeam == TEAM_RED ) // only play sound if the otherteam
    {
    gentity_t *te;

    te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND );

    te->s.eventParm = GTS_BRIEFCASE_TAKEN;
    te->r.svFlags |= SVF_BROADCAST;

    PrintMsg(NULL,"%s got the Tango Briefcase!\n",other->client->pers.netname);
    G_LogPrintf(S_COLOR_RED"%s"S_COLOR_WHITE" got the tango briefcase.\n", other->client->pers.netname);
    } */
}

qboolean NS_BriefCaseExist( void )
{
    gentity_t *ent;
    int	count = 0;

    ent = NULL;
    while ((ent = G_Find (ent, FOFS(classname), "team_briefcase")) != NULL) {
        count++;
    }

    return count;
}

void Reset_Briefcase( void )
{
    gentity_t *ent, *rent = NULL;


    ent = NULL;
    while ((ent = G_Find (ent, FOFS(classname), "team_briefcase")) != NULL) {
        if (ent->flags & FL_DROPPED_ITEM)
            G_FreeEntity(ent);
        else {
            rent = ent;
            RespawnItem(ent);
        }
    }

    // should be done over radio
    //	Team_SetFlagStatus( team, FLAG_ATBASE );

}
void Return_Briefcase( gentity_t *ent, gentity_t *other )
{
    gentity_t	*te;

    if (!other->client->ps.powerups[PW_BRIEFCASE])
        return;

    if (other->client->sess.sessionTeam != TEAM_RED )
        return;

    other->client->ps.powerups[PW_BRIEFCASE] = 0;

    PrintMsg(NULL,"%s"S_COLOR_WHITE" captured the tango briefcase!\n",other->client->pers.netname);

    G_LogPrintf( "OBJECTIVE: [%i] \"%s\" captured a briefcase\n", other->client->ps.clientNum, other->client->pers.netname  );

    // if you've already caputred it
    if ( other->client->ns.rewards & REWARD_BRIEFCASE_CAPTURE )
        other->client->ns.rewards |= REWARD_BRIEFCASE_2ND;
    else
        other->client->ns.rewards |= REWARD_BRIEFCASE_CAPTURE;
    //	CalculateRanks();

    // play sound
    te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND );

    te->s.eventParm = GTS_RED_BRIEFCASE_RETURN;
    te->r.svFlags |= SVF_BROADCAST;

    level.done_objectives[TEAM_RED]++;

}

