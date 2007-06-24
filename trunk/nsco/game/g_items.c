// Copyright (C) 1999-2000 Id Software, Inc.
//

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

/*

Items are any object that a player can touch to gain some effect.

Pickup will return the number of seconds until they should respawn.

all items should pop when dropped in lava or slime

Respawnable items don't actually go away when picked up, they are
just made invisible and untouchable.  This allows them to ride
movers and respawn apropriately.

*/


#define	RESPAWN_ARMOR		25
#define	RESPAWN_HEALTH		35
#define	RESPAWN_AMMO		40
#define	RESPAWN_HOLDABLE	60
#define	RESPAWN_MEGAHEALTH	35//120
#define	RESPAWN_POWERUP		120


//======================================================================

int Pickup_Powerup( gentity_t *ent, gentity_t *other ) {
    int			quantity;
    int			i;
    gclient_t	*client;

    if ( !other->client->ps.powerups[ent->item->giTag] ) {
        // round timing to seconds to make multiple powerup timers
        // count in sync
        other->client->ps.powerups[ent->item->giTag] =
            level.time - ( level.time % 1000 );
    }

    if ( ent->count ) {
        quantity = ent->count;
    } else {
        quantity = ent->item->quantity;
    }

    other->client->ps.powerups[ent->item->giTag] += quantity * 1000;

    // give any nearby players a "denied" anti-reward
    for ( i = 0 ; i < level.maxclients ; i++ ) {
        vec3_t		delta;
        float		len;
        vec3_t		forward;
        trace_t		tr;

        client = &level.clients[i];
        if ( client == other->client ) {
            continue;
        }
        if ( client->pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
            continue;
        }

        // if same team in team game, no sound
        // cannot use OnSameTeam as it expects to g_entities, not clients
        if ( g_gametype.integer >= GT_TEAM && other->client->sess.sessionTeam == client->sess.sessionTeam  ) {
            continue;
        }

        // if too far away, no sound
        VectorSubtract( ent->s.pos.trBase, client->ps.origin, delta );
        len = VectorNormalize( delta );
        if ( len > 192 ) {
            continue;
        }

        // if not facing, no sound
        AngleVectors( client->ps.viewangles, forward, NULL, NULL );
        if ( DotProduct( delta, forward ) < 0.4 ) {
            continue;
        }

        // if not line of sight, no sound
        trap_Trace( &tr, client->ps.origin, NULL, NULL, ent->s.pos.trBase, ENTITYNUM_NONE, CONTENTS_SOLID );
        if ( tr.fraction != 1.0 ) {
            continue;
        }

        // anti-reward
        client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_DENIEDREWARD;
    }
    return RESPAWN_POWERUP;
}

//======================================================================




//======================================================================

// Navy Seals ++
void AddAmmo (gentity_t *ent, int ammotype, int count)
{
    int max = 3;

    switch( ammotype ) {
    default:
        max = MAX_DEFAULT;
        break;
    }

    ent->client->ps.ammo[ammotype] += count;
    if ( ent->client->ps.ammo[ammotype] > max ) {
        ent->client->ps.ammo[ammotype] = max;
    }
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
    int		quantity;

    if ( ent->count ) {
        quantity = ent->count;
    } else {
        quantity = ent->item->quantity;
    }

    AddAmmo (other, ent->item->giTag , quantity);

    return RESPAWN_AMMO;
}
// Navy Seals --

//======================================================================

// Navy Seals ++
int Pickup_Weapon (gentity_t *ent, gentity_t *other) {

    if ( ent->ns_team != other->client->sess.sessionTeam
            && ent->ns_team != TEAM_FREE )
    {
        // not same team.
        PrintMsg( other, S_COLOR_RED "This item is not for your team.\n");
        return 0;
    }

    if ( !PI_CheckWeapon ( other->client->sess.sessionTeam, ent->item->giTag,
                           other->client->pers.nsPC.accuracy, other->client->pers.nsPC.strength,
                           other->client->pers.nsPC.stamina, other->client->pers.nsPC.stealth , qtrue ) )
    {
        PrintMsg( other, S_COLOR_RED "You cannot use this weapon.\n");
        return 0;
    }

    BG_PackWeapon( ent->item->giTag, other->client->ps.stats );

    if ( ent->item->giAmmoTag == AM_FLASHBANGS) // give grenades
    {
        if ( other->client->ps.ammo[AM_FLASHBANGS] < 2 )
            other->client->ps.ammo[AM_FLASHBANGS]++;

        return g_weaponRespawn.integer;
    }
    else if ( ent->item->giAmmoTag == AM_GRENADES) // give grenades
    {
        if ( other->client->ps.ammo[AM_GRENADES] < 2 )
            other->client->ps.ammo[AM_GRENADES]++;

        return g_weaponRespawn.integer;
    }
    else if ( ent->item->giAmmoTag == AM_SMOKE ) // give grenades
    {
        if ( other->client->ps.ammo[AM_SMOKE] < 2 )
            other->client->ps.ammo[AM_SMOKE]++;

        return g_weaponRespawn.integer;
    }


    if (ent->flags & FL_DROPPED_ITEM)
    {
        other->client->ns.rounds[ent->item->giTag] = ent->count;
        // has got additional equipment?
        if ( ent->s.generic1 )
        {
            other->client->ns.weaponmode[ent->item->giTag] = ent->s.generic1;
            ent->s.generic1 = 0;
        }
    }
    else {
        // full out clips
        other->client->ns.rounds[ent->item->giTag] = BG_GetMaxRoundForWeapon( ent->item->giTag );
    }


    // if we're in training / singplayer / dm mode
    if (g_gametype.integer < GT_TEAM )
    {
        return g_weaponRespawn.integer;
    }
    /*
    if ( BG_IsPrimary( ent->item->giTag ) )
    {
    int num_clips = 0;

    // get old primary
    gitem_t *it_wp;

    if ( other->client->pers.nsInven.primaryweapon != WP_NONE )
    {
    it_wp = BG_FindItemForWeapon( other->client->pers.nsInven.primaryweapon );

    if ( it_wp )
    {
    num_clips = other->client->pers.nsInven.ammo[ it_wp->giAmmoTag ];
    other->client->pers.nsInven.ammo[ it_wp->giAmmoTag ] = 0;
    }
    }

    NS_SetPrimary( other, ent->item->giTag );

    // get new ammo
    num_clips = other->client->pers.nsInven.ammo[ ent->item->giAmmoTag ];

    if ( ent->item->giTag == WP_M249 )
    other->client->pers.nsInven.ammo[ ent->item->giAmmoTag ] = 1;
    else
    other->client->pers.nsInven.ammo[ ent->item->giAmmoTag ] = 6;
    }
    else if ( BG_IsSecondary( ent->item->giTag ) )
    {
    int num_clips = 0;

    // get old primary
    gitem_t *it_wp;

    if ( other->client->pers.nsInven.secondaryweapon != WP_NONE   )
    {
    it_wp = BG_FindItemForWeapon( other->client->pers.nsInven.secondaryweapon );

    if ( it_wp )
    {
    num_clips = other->client->pers.nsInven.ammo[ it_wp->giAmmoTag ];
    other->client->pers.nsInven.ammo[ it_wp->giAmmoTag ] = 0;
    }
    }

    NS_SetSecondary( other, ent->item->giTag );

    // get new ammo
    num_clips = other->client->pers.nsInven.ammo[ ent->item->giAmmoTag ];
    other->client->pers.nsInven.ammo[ ent->item->giAmmoTag ] = 4;
    }
    */
    return g_weaponRespawn.integer;
}

// Navy Seals --

//======================================================================

int Checked_BotRoam( gentity_t *ent, gentity_t *other)
{
    if ( !NS_IsBot(other) )
        return 0;

    return 350;
}
//======================================================================

// Navy Seals ++
int Pickup_Armor (gentity_t *ent, gentity_t *other) {

    // already got this item.
    if ( other->client->ps.powerups[ent->item->giTag])
        return 0;

    // add item
    other->client->ps.powerups[ent->item->giTag] = ARMOR_LIGHT;

    return 50;
}
// Navy Seals --

//======================================================================

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {
    // randomly select from teamed entities
    if (ent->team) {
        gentity_t	*master;
        int	count;
        int choice;

        if ( !ent->teammaster ) {
            G_Error( "RespawnItem: bad teammaster");
        }
        master = ent->teammaster;

        for (count = 0, ent = master; ent; ent = ent->teamchain, count++)
            ;

        choice = rand() % count;

        for (count = 0, ent = master; count < choice; ent = ent->teamchain, count++)
            ;
    }

    ent->r.contents = CONTENTS_TRIGGER;
    ent->s.eFlags &= ~EF_NODRAW;
    ent->r.svFlags &= ~SVF_NOCLIENT;
    trap_LinkEntity (ent);

    if ( ent->item->giType == IT_POWERUP ) {
        // play powerup spawn sound to all clients
        gentity_t	*te;

        // if the powerup respawn sound should Not be global
        if (ent->speed) {
            te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
        }
        else {
            te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
        }
        te->s.eventParm = G_SoundIndex( "sound/items/poweruprespawn.wav" );
        te->r.svFlags |= SVF_BROADCAST;
    }

    if ( ent->item->giType == IT_HOLDABLE && ent->item->giTag == HI_KAMIKAZE ) {
        // play powerup spawn sound to all clients
        gentity_t	*te;

        // if the powerup respawn sound should Not be global
        if (ent->speed) {
            te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
        }
        else {
            te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
        }
        te->s.eventParm = G_SoundIndex( "sound/items/kamikazerespawn.wav" );
        te->r.svFlags |= SVF_BROADCAST;
    }
    if ( ent->item->giType == IT_BOTROAM )
    {
        // we previously removed it, now set it again... :-p
        ent->s.eFlags |= EF_NODRAW;
        ent->nextthink = 0;

        trap_LinkEntity( ent );
        return;
    }

    // play the normal respawn sound only to nearby clients
    G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );

    ent->nextthink = 0;
}

qboolean BotCanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps ) {
    gitem_t	*item;

    if ( ent->modelindex < 1 || ent->modelindex >= bg_numItems ) {
        Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: index out of range" );
    }
    // Navy Seals ++
    item = &bg_itemlist[ent->modelindex];
    // if vip return

    if ( ps->eFlags & EF_VIP ) {
        if ( BG_IsPistol( item->giTag ) )
            return qtrue;
        return qfalse;
    }

    // Navy Seals --

    switch( item->giType ) {
        // Navy Seals ++
    case IT_WEAPON:
        {
            if ( BG_IsPrimary ( item->giTag ) && BG_GotPrimary( ps ) )
                return qfalse;
            if ( BG_IsSecondary ( item->giTag  ) && BG_GotSecondary( ps ) )
                return qfalse;

            if ( item->giTag == WP_FLASHBANG ) {
                if ( ps->ammo[AM_FLASHBANGS] < 2 )
                    return qtrue;
                else
                    return qfalse;
            }
            else if ( item->giTag == WP_GRENADE ) {
                if ( ps->ammo[AM_GRENADES] < 2 )
                    return qtrue;
                else
                    return qfalse;
            } else if ( item->giTag == WP_SMOKE ) {
                if ( ps->ammo[AM_SMOKE] < 2 )
                    return qtrue;
                else
                    return qfalse;
            }

            // already got a c4 - max is one
            if ( item->giTag == WP_C4 && ( BG_GotWeapon( WP_C4, (int*)ps->stats ) ) )
                return qfalse;

            return qtrue;
        }
        // Navy Seals --

    case IT_AMMO:
        // Navy Seals ++
        {
            int max = 3;

            switch( item->giTag ) {
            default:
                max = MAX_DEFAULT;
                break;
            }

            if ( ps->ammo[ item->giTag ] >= max ) {
                return qfalse;	// can't hold any more
            }

            return qtrue;
        }
        // Navy Seals --

    case IT_ARMOR:
        // Navy Seals ++
        {
            if ( ps->powerups[ item->giTag ] )
                return qfalse;

            return qtrue;
        }// Navy Seals --

    case IT_BOTROAM:
        return qtrue;

    case IT_TEAM: // team items, such as flags
        if (item->giTag == PW_BRIEFCASE_RETURN) {
            if ( ps->powerups[PW_BRIEFCASE] && ps->persistant[PERS_TEAM] == TEAM_RED)
                return qtrue;

            return qfalse;
        }

        if (item->giTag == PW_BRIEFCASE) {
            if ( ps->persistant[PERS_TEAM] == TEAM_BLUE || ps->powerups[PW_BRIEFCASE] )
                return qfalse;
            else
                return qtrue;
        }
        return qfalse;

    default:
    case IT_HOLDABLE:
        return qfalse;
    case IT_BAD:
        Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD" );
    }

    return qfalse;
}


/*
===============
Touch_Item

+defcon-X: reactivated it.
defcon-X: just keep this for bots!
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace ) {
    int			respawn;

    if (!other->client)
        return;
    if (other->health < 1)
        return;		// dead people can't pickup
    if ( NS_IsBot( other ) ) {
        // the same pickup rules are used for client side and server side
        if ( !BotCanItemBeGrabbed( g_gametype.integer, &ent->s, &other->client->ps ) ) {
            return;
        }
    }
    else if ( ent->item->giTag == PW_BRIEFCASE_RETURN && ent->item->giType == IT_TEAM )
    {
        if ( other->client->ps.powerups[PW_BRIEFCASE] <= 0 )
            return;
    }
    else if ( !(other->client->buttons & BUTTON_USE) || (other->client->oldbuttons & BUTTON_USE)  )
        return;
    if ( !BG_CanItemBeGrabbed( g_gametype.integer, &ent->s, &other->client->ps ) && !NS_IsBot(other) ) {
        return;
    }

    // call the item-specific pickup function
    switch( ent->item->giType ) {
    case IT_WEAPON:
        respawn = Pickup_Weapon(ent, other);
        break;
    case IT_AMMO:
        respawn = Pickup_Ammo(ent, other);
        break;
    case IT_ARMOR:
        respawn = Pickup_Armor(ent, other);
        break;
    case IT_TEAM:
        respawn = Pickup_Team(ent, other);
        break;
    case IT_BOTROAM:
        respawn = Checked_BotRoam(ent,other);
        break;
    default:
        return;
    }

    if ( respawn == 0 ) {
        return;
    }

    // play the normal pickup sound
    if ( (ent->item->giTag != PW_BRIEFCASE_RETURN) && (ent->item->giTag != PW_BRIEFCASE ) ) {
        if ( ent->item->giType != IT_BOTROAM )
            G_AddEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
    }

    // fire item targets
    G_UseTargets (ent, other);

    // wait of -1 will not respawn
    if ( ent->wait == -1 ) {
        ent->r.svFlags |= SVF_NOCLIENT;
        ent->s.eFlags |= EF_NODRAW;
        ent->r.contents = 0;
        ent->unlinkAfterEvent = qtrue;
        return;
    }

    // non zero wait overrides respawn time
    if ( ent->wait ) {
        respawn = ent->wait;
    }

    // random can be used to vary the respawn time
    if ( ent->random ) {
        respawn += crandom() * ent->random;
        if ( respawn < 1 ) {
            respawn = 1;
        }
    }

    // dropped items will not respawn
    if ( ent->flags & FL_DROPPED_ITEM ) {
        ent->freeAfterEvent = qtrue;
    }

    // picked up items still stay around, they just don't
    // draw anything.  This allows respawnable items
    // to be placed on movers.
    ent->r.svFlags |= SVF_NOCLIENT;
    ent->s.eFlags |= EF_NODRAW;
    ent->r.contents = 0;

    // ZOID
    // A negative respawn times means to never respawn this item (but don't
    // delete it).  This is used by items that are respawned by third party
    // events such as ctf flags
    if ( respawn <= 0 ) {
        ent->nextthink = 0;
        ent->think = 0;
    } else {
        ent->nextthink = level.time + respawn * 1000;
        ent->think = RespawnItem;
    }
    trap_LinkEntity( ent );
}

/*
===============
Touch_Item

defcon-X: just keep this for bots!
===============
*/
void Pick_Item (gentity_t *ent, gentity_t *other, trace_t *trace ) {
    int			respawn;

    if (!other->client)
        return;
    if (other->health < 1)
        return;		// dead people can't pickup
    if ( !BG_CanItemBeGrabbed( g_gametype.integer, &ent->s, &other->client->ps ) ) {
        return;
    }

    // call the item-specific pickup function
    switch( ent->item->giType ) {
    case IT_WEAPON:
        respawn = Pickup_Weapon(ent, other);
        break;
    case IT_AMMO:
        respawn = Pickup_Ammo(ent, other);
        break;
    case IT_ARMOR:
        respawn = Pickup_Armor(ent, other);
        break;
    case IT_TEAM:
        respawn = Pickup_Team(ent, other);
        break;
    default:
        return;
    }

    if ( !respawn ) {
        return;
    }

    // play the normal pickup sound
    if ( (ent->item->giTag != PW_BRIEFCASE_RETURN) && (ent->item->giTag != PW_BRIEFCASE ) ) {
        if ( ent->item->giType != IT_BOTROAM )
            G_AddEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
    }
    // Navy Seals ++

    // fire item targets
    G_UseTargets (ent, other);

    // wait of -1 will not respawn
    if ( ent->wait == -1 ) {
        ent->r.svFlags |= SVF_NOCLIENT;
        ent->s.eFlags |= EF_NODRAW;
        ent->r.contents = 0;
        ent->unlinkAfterEvent = qtrue;
        return;
    }

    // non zero wait overrides respawn time
    if ( ent->wait ) {
        respawn = ent->wait;
    }

    // random can be used to vary the respawn time
    if ( ent->random ) {
        respawn += crandom() * ent->random;
        if ( respawn < 1 ) {
            respawn = 1;
        }
    }

    // dropped items will not respawn
    if ( ent->flags & FL_DROPPED_ITEM ) {
        ent->freeAfterEvent = qtrue;
    }

    // picked up items still stay around, they just don't
    // draw anything.  This allows respawnable items
    // to be placed on movers.
    ent->r.svFlags |= SVF_NOCLIENT;
    ent->s.eFlags |= EF_NODRAW;
    ent->r.contents = 0;

    // ZOID
    // A negative respawn times means to never respawn this item (but don't
    // delete it).  This is used by items that are respawned by third party
    // events such as ctf flags
    if ( respawn <= 0 ) {
        ent->nextthink = 0;
        ent->think = 0;
    } else {
        ent->nextthink = level.time + respawn * 1000;
        ent->think = RespawnItem;
    }
    trap_LinkEntity( ent );
}

//======================================================================

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity ) {
    gentity_t	*dropped;

    dropped = G_Spawn();

    dropped->s.eType = ET_ITEM;
    dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
    dropped->s.modelindex2 = 1;			// This is non-zero is it's a dropped item

    dropped->classname = item->classname;
    dropped->item = item;
    VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
    VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
    dropped->r.contents = CONTENTS_TRIGGER;

    dropped->touch = Touch_Item;

    G_SetOrigin( dropped, origin );
    dropped->s.pos.trType = TR_GRAVITY;
    dropped->s.pos.trTime = level.time;
    VectorCopy( velocity, dropped->s.pos.trDelta );

    dropped->s.eFlags |= EF_BOUNCE_HALF;
    // Navy Seals ++
    if (g_gametype.integer >= GT_TEAM && item->giType == IT_TEAM) { // Special case for CTF flags
        Team_CheckDroppedItem( dropped );
    } else if ( g_gametype.integer != GT_LTS ) { // auto-remove after 10 seconds
        dropped->think = G_FreeEntity;
        dropped->nextthink = level.time + 10000;
    }
    // Navy Seals --
    dropped->flags = FL_DROPPED_ITEM;

    trap_LinkEntity (dropped);
    return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle ) {
    vec3_t	velocity;
    vec3_t	angles;

    VectorCopy( ent->s.apos.trBase, angles );
    angles[YAW] += angle;
    angles[PITCH] = 0;	// always forward

    AngleVectors( angles, velocity, NULL, NULL );
    VectorScale( velocity, 150, velocity );
    velocity[2] += 200 + crandom() * 50;

    return LaunchItem( item, ent->s.pos.trBase, velocity );
}


/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
    RespawnItem( ent );
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) {
    trace_t		tr;
    vec3_t		dest;

    VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
    VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );

    ent->s.eType = ET_ITEM;
    ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
    ent->s.modelindex2 = 0;					// zero indicates this isn't a dropped item

    ent->r.contents = CONTENTS_TRIGGER;
    ent->touch = Touch_Item;
    // useing an item causes it to respawn
    ent->use = Use_Item;

    if ( ent->spawnflags & 1 ) {
        // suspended
        G_SetOrigin( ent, ent->s.origin );
    } else {
        // drop to floor
        VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
        trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
        if ( tr.startsolid ) {
            G_Printf ("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
            G_FreeEntity( ent );
            return;
        }

        // allow to ride movers
        ent->s.groundEntityNum = tr.entityNum;

        G_SetOrigin( ent, tr.endpos );
    }

    // team slaves and targeted items aren't present at start
    if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ) {
        ent->s.eFlags |= EF_NODRAW;
        ent->r.contents = 0;
        return;
    }
    if ( !Q_stricmp(ent->item->classname , "item_botroam" ) )
    {
        ent->s.eFlags |= EF_NODRAW;
        ent->r.svFlags |= SVF_NOCLIENT;
        // G_LogPrintf("Spawned item_botroam at %s\n", vtos( ent->s.origin ) );
    }

    // powerups don't spawn in for a while
    if ( ent->item->giType == IT_POWERUP ) {
        float	respawn;

        respawn = 45 + crandom() * 15;
        ent->s.eFlags |= EF_NODRAW;
        ent->r.contents = 0;
        ent->nextthink = level.time + respawn * 1000;
        ent->think = RespawnItem;
        return;
    }


    trap_LinkEntity (ent);
}


qboolean	itemRegistered[MAX_ITEMS];

/*
==================
G_CheckTeamItems
==================
*/
void G_CheckTeamItems( void ) {

    // Set up team stuff
    Team_InitGame();

    /*	if( g_gametype.integer == GT_CTF ) {
    gitem_t	*item;

    // check for the two flags
    item = BG_FindItem( "Red Flag" );
    if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
    G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map" );
    }
    item = BG_FindItem( "Blue Flag" );
    if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
    G_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map" );
    }
    }*/

}

#define REGISTER_ALL_ITEMS	1
/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {

    int i = 0;
    memset( itemRegistered, 0, sizeof( itemRegistered ) );


    // Navy Seals ++
    if ( g_cheats.integer == 0 ) {
        for ( i = 1 ; i < WP_NUM_WEAPONS-1;i++ ) {
            // if ( i == WP_NUTSHELL ) continue;
            if ( i == WP_NONE )
                continue;

            RegisterItem( BG_FindItemForWeapon( i ) );
        }
    }
    // Navy Seals --

}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) {
    if ( !item ) {
        G_Error( "RegisterItem: NULL" );
    }
    itemRegistered[ item - bg_itemlist ] = qtrue;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) {
    char	string[MAX_ITEMS+1];
    int		i;
    int		count;

    count = 0;
    for ( i = 0 ; i < bg_numItems ; i++ ) {
        if ( itemRegistered[i] ) {
            count++;
            string[i] = '1';
        } else {
            string[i] = '0';
        }
    }
    string[ bg_numItems ] = 0;

    G_Printf( "%i items registered\n", count );
    trap_SetConfigstring(CS_ITEMS, string);
}

/*
============
G_ItemDisabled
============
*/
int G_ItemDisabled( gitem_t *item ) {

    char name[128];

    Com_sprintf(name, sizeof(name), "disable_%s", item->classname);
    return trap_Cvar_VariableIntegerValue( name );
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) {
    G_SpawnFloat( "random", "0", &ent->random );
    G_SpawnFloat( "wait", "0", &ent->wait );

    RegisterItem( item );
    if ( G_ItemDisabled(item) )
        return;

    ent->item = item;
    // some movers spawn on the second frame, so delay item
    // spawns until the third frame so they can ride trains
    ent->nextthink = level.time + FRAMETIME * 2;
    ent->think = FinishSpawningItem;

    ent->physicsBounce = 0.50;		// items are bouncy

    if ( item->giType == IT_POWERUP ) {
        G_SoundIndex( "sound/items/poweruprespawn.wav" );
        G_SpawnFloat( "noglobalsound", "0", &ent->speed);
    } 
}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
    vec3_t	velocity;
    float	dot;
    int		hitTime;

    // reflect the velocity on the trace plane
    hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
    BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
    dot = DotProduct( velocity, trace->plane.normal );
    VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

    // cut the velocity to keep from bouncing forever
    VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

    // check for stop
    if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
        trace->endpos[2] += 1.0;	// make sure it is off ground
        SnapVector( trace->endpos );
        G_SetOrigin( ent, trace->endpos );
        ent->s.groundEntityNum = trace->entityNum;
        return;
    }

    VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
    VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
    ent->s.pos.trTime = level.time;
}


/*
================
G_RunItem

================
*/
void G_RunItem( gentity_t *ent ) {
    vec3_t		origin;
    trace_t		tr;
    int			contents;
    int			mask;

    // if groundentity has been set to -1, it may have been pushed off an edge
    if ( ent->s.groundEntityNum == -1 ) {
        if ( ent->s.pos.trType != TR_GRAVITY ) {
            ent->s.pos.trType = TR_GRAVITY;
            ent->s.pos.trTime = level.time;
        }
    }

    if ( ent->s.pos.trType == TR_STATIONARY ) {
        // check think function
        G_RunThink( ent );
        return;
    }

    // get current position
    BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

    // trace a line from the previous position to the current position
    if ( ent->clipmask ) {
        mask = ent->clipmask;
    } else {
        mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
    }
    trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin,
                ent->r.ownerNum, mask );

    VectorCopy( tr.endpos, ent->r.currentOrigin );

    if ( tr.startsolid ) {
        tr.fraction = 0;
    }

    trap_LinkEntity( ent );	// FIXME: avoid this for stationary?

    // check think function
    G_RunThink( ent );

    if ( tr.fraction == 1 ) {
        return;
    }

    // if it is in a nodrop volume, remove it
    contents = trap_PointContents( ent->r.currentOrigin, -1 );
    if ( contents & CONTENTS_NODROP ) {
        if (ent->item && ent->item->giType == IT_TEAM) {
            Team_FreeEntity(ent);
        } else {
            G_FreeEntity( ent );
        }
        return;
    }

    G_BounceItem( ent, &tr );
}

