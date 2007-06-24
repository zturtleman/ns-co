// Copyright (C) 2001-2002 defcon-x/team-mirage
//
// g_antilag.c -- handles server side anti-lag
//
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

void LerpVector ( vec3_t from, vec3_t to, float lerp, vec3_t out )
{
    out[0] = from[0] + (to[0]-from[0]) * lerp;
    out[1] = from[1] + (to[1]-from[1]) * lerp;
    out[2] = from[2] + (to[2]-from[2]) * lerp;
}

/*
================
G_UpdateClientAntiLag
================
*/
void G_UpdateClientAntiLag ( gentity_t* ent )
{
    int			head;
    int			newtime;

    head = ent->client->antilagHead;

    // If on a new frame snap the head up to the end of the last frame and
    // add a new head
    if ( ent->client->antilag[head].leveltime < level.time )
    {
        ent->client->antilag[head].time = level.previousTime;

        // Move to the next position
        if ( (++ent->client->antilagHead) > MAX_ANTILAG )
        {
            ent->client->antilagHead = 0;
        }

        head = ent->client->antilagHead;
    }

    // Bots only move once per frame
    if ( ent->r.svFlags & SVF_BOT )
    {
        newtime = level.time;
    }
    else
    {
        // calculate the actual server time
        newtime = level.previousTime + trap_Milliseconds() - level.frameStartTime;

        if ( newtime > level.time )
        {
            newtime = level.time;
        }
        else if ( newtime <= level.previousTime )
        {
            newtime = level.previousTime + 1;
        }
    }

    // Copy the clients current state into the antilag cache
    ent->client->antilag[head].leveltime = level.time;
    ent->client->antilag[head].time = newtime;

    // save inforamtion
    VectorCopy ( ent->r.currentOrigin, ent->client->antilag[head].rOrigin );
    VectorCopy ( ent->r.currentAngles, ent->client->antilag[head].rAngles );
    VectorCopy ( ent->r.mins, ent->client->antilag[head].mins );
    VectorCopy ( ent->r.maxs, ent->client->antilag[head].maxs );

    ent->client->antilag[head].pm_flags  = ent->client->ps.pm_flags;
    //	ent->client->antilag[head].leanTime  = ent->client->ps.leanTime;
}

/*
================
G_UndoClientAntiLag
================
*/
void G_UndoClientAntiLag ( gentity_t* ent )
{
    // If the client isnt already in the past then
    // dont bother doing anything
    if ( ent->client->antilagUndo.leveltime != level.time  )
        return;

    // Move the client back into reality by moving over the undo information
    VectorCopy ( ent->client->antilagUndo.rOrigin, ent->r.currentOrigin );
    VectorCopy ( ent->client->antilagUndo.rAngles, ent->r.currentAngles );
    VectorCopy ( ent->client->antilagUndo.mins, ent->r.mins );
    VectorCopy ( ent->client->antilagUndo.maxs, ent->r.maxs );

    ent->client->ps.pm_flags = ent->client->antilagUndo.pm_flags;

    // Mark the undo information so it cant be used again
    ent->client->antilagUndo.leveltime = 0;
}

/*
================
G_ApplyClientAntiLag
================
*/
void G_ApplyClientAntiLag ( gentity_t* ent, int time )
{
    float	lerp;
    int		from;
    int		to;
    int   steps = 0;

    // Find the two pieces of history information that sandwitch the
    // time we are looking for
    from = ent->client->antilagHead;
    to   = ent->client->antilagHead;
    do {
        if ( (ent->client->antilag[from].time) <= time )
            break;

        to = from;
        from--;
        steps++;   // count the number of steps we do in antilag

        if ( from < 0 )
            from = MAX_ANTILAG - 1;
    }
    while ( from != ent->client->antilagHead );

    // If the from is equal to the to then there wasnt even
    // one piece of information worth using so just use the current time frame
    if ( from == to )
    {
        return;
    }

    // Save the undo information if its not already saved
    if ( ent->client->antilagUndo.leveltime != level.time )
    {
        // Save the undo information
        ent->client->antilagUndo.leveltime = level.time;

        VectorCopy ( ent->r.currentOrigin, ent->client->antilagUndo.rOrigin );
        VectorCopy ( ent->r.currentAngles, ent->client->antilagUndo.rAngles );
        VectorCopy ( ent->r.mins, ent->client->antilagUndo.mins );
        VectorCopy ( ent->r.maxs, ent->client->antilagUndo.maxs );

        ent->client->antilagUndo.pm_flags  = ent->client->ps.pm_flags;
        //		ent->client->antilagUndo.leanTime  = ent->client->ps.leanTime;
    }

    // If the best history found was the last in the list then
    // dont lerp, just use the last one
    if ( from == ent->client->antilagHead )
    {
        VectorCopy ( ent->client->antilag[to].rOrigin, ent->r.currentOrigin );
        VectorCopy ( ent->client->antilag[to].rAngles, ent->r.currentAngles );
        VectorCopy ( ent->client->antilag[to].mins, ent->r.maxs );
        VectorCopy ( ent->client->antilag[to].maxs, ent->r.mins );

        ent->client->ps.pm_flags = ent->client->antilag[to].pm_flags;
        //		ent->client->ps.leanTime = ent->client->antilag[to].leanTime;
    }
    else
    {
        // Calculate the lerp value to use for the vectors
        lerp = (float)(time - ent->client->antilag[from].time) / (float)(ent->client->antilag[to].time - ent->client->antilag[from].time);

        // Lerp all the vectors between the before and after history information
        LerpVector ( ent->client->antilag[from].rOrigin, ent->client->antilag[to].rOrigin, lerp, ent->r.currentOrigin );
        LerpVector ( ent->client->antilag[from].rAngles, ent->client->antilag[to].rAngles, lerp, ent->r.currentAngles );
        LerpVector ( ent->client->antilag[from].maxs, ent->client->antilag[to].maxs, lerp, ent->r.maxs );
        LerpVector ( ent->client->antilag[from].mins, ent->client->antilag[to].mins, lerp, ent->r.mins );

        // 		ent->client->ps.leanTime = ent->client->antilag[from].leanTime + (ent->client->antilag[from].leanTime-ent->client->antilag[to].leanTime) * lerp;

        ent->client->ps.pm_flags = ent->client->antilag[to].pm_flags;
    }
}

/*
================
G_UndoAntiLag
================
*/
void G_UndoAntiLag ( void )
{
    int i;

    // Undo all history
    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* other = &g_entities[level.sortedClients[i]];

        if ( other->client->pers.connected != CON_CONNECTED )
        {
            continue;
        }

        // Skip clients that are spectating
        if ( other->client->sess.waiting ||
                other->client->ps.pm_type == PM_NOCLIP ||
                other->client->ps.pm_type == PM_SPECTATOR ||
                other->health <= 0 )
        {
            continue;
        }
#if 0
        if ( other->r.svFlags & SVF_DOUBLED_BBOX )
        {
            // Put the hitbox back the way it was
            other->r.maxs[0] /= 2;
            other->r.maxs[1] /= 2;
            other->r.mins[0] /= 2;
            other->r.mins[1] /= 2;

            other->r.svFlags &= (~SVF_DOUBLED_BBOX);
        }
#endif

        G_UndoClientAntiLag ( other );

        // update the head bbox
        NS_ModifyClientBBox( other );
        // Relink the entity into the world
        trap_LinkEntity ( other );
    }
}

/*
================
G_ApplyAntiLag
================
*/
#if 0
void G_ApplyAntiLag ( gentity_t* ref, qboolean enlargeHitBox )
#else
void G_ApplyAntiLag ( gentity_t* ref )
#endif
{
    int i;
    int reftime;

    // Figure out the reference time based on the reference clients server time
    reftime = ref->client->pers.cmd.serverTime;
    if ( reftime > level.time )
        reftime = level.time;

    // Move all the clients back into the reference clients time frame.
    for ( i = 0; i < level.numConnectedClients; i ++ )
    {
        gentity_t* other = &g_entities[level.sortedClients[i]];

        if ( other->client->pers.connected != CON_CONNECTED )
            continue;

        // Skip the reference client
        if ( other == ref )
            continue;

        // Skip entities not in use
        if ( !other->inuse )
            continue;

        // Skip clients that are spectating
        if ( other->client->sess.waiting ||
                other->client->ps.pm_type == PM_NOCLIP ||
                other->client->ps.pm_type == PM_SPECTATOR ||
                other->health <= 0 )
        {
            continue;
        }

        // Dont bring them back in time unless requested
        if ( !(ref->r.svFlags & SVF_BOT) & ref->client->pers.antiLag )
        {
            // Apply the antilag to this player
            G_ApplyClientAntiLag ( other, reftime );
        }

#if 0
        if ( enlargeHitBox )
        {
            // Adjust the hit box to account for hands and such
            // that are sticking out of the normal bounding box
            other->r.maxs[0] *= 2;
            other->r.maxs[1] *= 2;
            other->r.mins[0] *= 2;
            other->r.mins[1] *= 2;
            other->r.svFlags |= SVF_DOUBLED_BBOX;
        }
#endif

        // move his headbbox according to the coordinates.
        NS_ModifyClientBBox( other );
        // Relink the entity into the world
        trap_LinkEntity ( other );
    }
}
