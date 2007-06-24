// Copyright (C) 1999-2000 Id Software, Inc.
//

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

void G_UpdateClientAntiLag ( gentity_t *ent );

// remove as defcaonX said 07.01.2004
// #include "camclient.h"

/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( gentity_t *player ) {
    gclient_t	*client;
    float	count;
    vec3_t	angles;

    client = player->client;
    if ( client->ps.pm_type == PM_DEAD ) {
        return;
    }

    // total points of damage shot at the player this frame
    count = client->damage_blood + client->damage_armor;
    if ( count == 0 ) {
        return;		// didn't take any damage
    }

    if ( count > 255 ) {
        count = 255;
    }

    // send the information to the client

    // world damage (falling, slime, etc) uses a special code
    // to make the blend blob centered instead of positional
    if ( client->damage_fromWorld ) {
        client->ps.damagePitch = 255;
        client->ps.damageYaw = 255;

        client->damage_fromWorld = qfalse;
    } else {
        vectoangles( client->damage_from, angles );
        client->ps.damagePitch = angles[PITCH]/360.0 * 256;
        client->ps.damageYaw = angles[YAW]/360.0 * 256;
    }

    // play an apropriate pain sound
    if ( (level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) ) {
        player->pain_debounce_time = level.time + 700;
        // Navy Seals ++
        //		G_AddEvent( player, EV_PAIN, player->health );
        // Navy Seals --
        client->ps.damageEvent++;
    }


    client->ps.damageCount = count;

    //
    // clear totals
    //
    client->damage_blood = 0;
    client->damage_armor = 0;
    client->damage_knockback = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( gentity_t *ent ) {
    qboolean	envirosuit = qfalse; // never got enviro
    int			waterlevel;

    if ( ent->client->noclip ) {
        ent->client->airOutTime = level.time + 12000;	// don't need air
        return;
    }

    waterlevel = ent->waterlevel;
#if 0
    envirosuit = ent->client->ps.powerups[PW_BATTLESUIT] > level.time;
#endif
    //
    // check for drowning
    //
    if ( waterlevel == 3 ) {
        // envirosuit give air
        if ( envirosuit ) {
            ent->client->airOutTime = level.time + 10000;
        }

        // if out of air, start drowning
        if ( ent->client->airOutTime < level.time) {
            // drown!
            ent->client->airOutTime += 1000;
            if ( ent->health > 0 ) {
                // take more damage the longer underwater
                ent->damage += 2;
                if (ent->damage > 15)
                    ent->damage = 15;

                // play a gurp sound instead of a normal pain sound
                if (ent->health <= ent->damage) {
                    G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/actors/player/drown.wav"));
                } else if (rand()&1) {
                    G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp1.wav"));
                } else {
                    G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp2.wav"));
                }

                // don't play a normal pain sound
                ent->pain_debounce_time = level.time + 200;

                G_Damage (ent, NULL, NULL, NULL, NULL, ent->damage, DAMAGE_NO_ARMOR, MOD_WATER);
            }
        }
    } else {
        ent->client->airOutTime = level.time + 12000;
        ent->damage = 2;
    }
}



/*
===============
G_SetClientSound
===============
*/
void G_SetClientSound( gentity_t *ent ) {
    if ( ent->client->sess.waiting ||
            ent->health <= 0 || ent->client->ps.stats[STAT_HEALTH] <= 0 ||
            ent->client->ps.pm_type != PM_NORMAL )
    {
        ent->client->ps.loopSound = 0;
        return;
    }

    if ( ent->health < 10 )
    {
        int	xyzspeed = sqrt( ent->client->ps.velocity[0] * ent->client->ps.velocity[0]
                             +  ent->client->ps.velocity[1] * ent->client->ps.velocity[1]
                             +  ent->client->ps.velocity[2] * ent->client->ps.velocity[2] );

        if ( xyzspeed > 10 )
        {
            // FIXME female breath sound
            if ( ent->client->bleed_num >= 3 || ent->health < 25 )
                ent->client->ps.loopSound = level.breathsnd_injured;
            else
                ent->client->ps.loopSound = level.breathsnd_male;
        }
    }
    else{
        ent->client->ps.loopSound = 0;
    }
}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( gentity_t *ent, pmove_t *pm ) {
    int		i, j;
    trace_t	trace;
    gentity_t	*other;

    memset( &trace, 0, sizeof( trace ) );
    for (i=0 ; i<pm->numtouch ; i++) {
        for (j=0 ; j<i ; j++) {
            if (pm->touchents[j] == pm->touchents[i] ) {
                break;
            }
        }
        if (j != i) {
            continue;	// duplicated
        }
        other = &g_entities[ pm->touchents[i] ];

        if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
            ent->touch( ent, other, &trace );
        }

        if ( !other->touch ) {
            continue;
        }

        other->touch( other, ent, &trace );
    }

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( gentity_t *ent ) {
    int			i, num;
    int			touch[MAX_GENTITIES];
    gentity_t	*hit;
    trace_t		trace;
    vec3_t		mins, maxs;
    static vec3_t	range = { 40, 40, 52 };

    if ( !ent->client ) {
        return;
    }

    // dead clients don't activate triggers!
    if ( ent->client->ps.stats[STAT_HEALTH] <= 0 ) {
        return;
    }

    VectorSubtract( ent->client->ps.origin, range, mins );
    VectorAdd( ent->client->ps.origin, range, maxs );

    num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

    // can't use ent->absmin, because that has a one unit pad
    VectorAdd( ent->client->ps.origin, ent->r.mins, mins );
    VectorAdd( ent->client->ps.origin, ent->r.maxs, maxs );

    for ( i=0 ; i<num ; i++ ) {
        hit = &g_entities[touch[i]];

        if ( !hit->touch && !ent->touch ) {
            continue;
        }
        if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
            continue;
        }

        // ignore most entities if a spectator
        // or as an camera
        if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR || ent->client->ps.pm_type == PM_SPECTATOR || ent->client->ps.pm_type == PM_NOCLIP ) {
            if ( hit->s.eType != ET_TELEPORT_TRIGGER &&
                    // this is ugly but adding a new ET_? type will
                    // most likely cause network incompatibilities
                    hit->touch != Touch_DoorTrigger) {
                continue;
            }
        }

        // use seperate code for determining if an item is picked up
        // so you don't have to actually contact its bounding box
        if ( hit->s.eType == ET_ITEM || !Q_stricmp( hit->classname, "c4_placed") ) {
            if ( !BG_PlayerTouchesItem( &ent->client->ps, &hit->s, level.time ) ) {
                continue;
            }
        } else {
            if ( !trap_EntityContact( mins, maxs, hit ) ) {
                continue;
            }
        }

        memset( &trace, 0, sizeof(trace) );

        if ( hit->touch ) {
            hit->touch (hit, ent, &trace);
        }

        if ( ( ent->r.svFlags & SVF_BOT ) && ( ent->touch ) ) {
            ent->touch( ent, hit, &trace );
        }
    }

    // if we didn't touch a jump pad this pmove frame
    if ( ent->client->ps.jumppad_frame != ent->client->ps.pmove_framecount ) {
        ent->client->ps.jumppad_frame = 0;
        ent->client->ps.jumppad_ent = 0;
    }
}

void ChangeCameraState ( int *i , qboolean on)
{
    if ( on )
        *i = 1;
    else
        *i = 0;
}

/*
=================
SpectatorThink
=================
*/
void SpectatorThink( gentity_t *ent, usercmd_t *ucmd ) {
    pmove_t	pm;
    gclient_t	*client;


    client = ent->client;

    // always full health as spectator
    if ( ent->health < 100 )
        ent->health = 100;

    if ( !( client->ps.pm_flags & PMF_FOLLOW ) && client->sess.spectatorState != SPECTATOR_FOLLOW )
    {
        client->ps.pm_type = PM_NOCLIP;
        client->ps.speed = 250;	// faster than normal

        ent->r.contents = 0;

        // set up for pmove
        memset (&pm, 0, sizeof(pm));
        pm.ps = &client->ps;
        pm.cmd = *ucmd;
        pm.tracemask = 0;
        // spectators can fly through bodies
        pm.trace = trap_Trace;
        pm.pointcontents = trap_PointContents;

        // perform a pmove
        Pmove (&pm);
        // save results of pmove
        VectorCopy( client->ps.origin, ent->s.origin );

        //	G_TouchTriggers( ent );
        trap_UnlinkEntity( ent );
    }
    else
    {

        //	client->ps.pm_type = PM_NORMAL;
        //	client->ps.speed = 0;	// faster than normal

        ent->r.contents = 0;

        // set up for pmove
        /*		memset (&pm, 0, sizeof(pm));
        pm.ps = &client->ps;
        pm.cmd = *ucmd;
        pm.tracemask = 0;
        // spectators can fly through bodies
        pm.trace = trap_Trace;
        pm.pointcontents = trap_PointContents;

        // perform a pmove
        Pmove (&pm);
        // save results of pmove
        VectorCopy( client->ps.origin, ent->s.origin );
        */
        trap_UnlinkEntity( ent );

        //	G_TouchTriggers( ent );

    }

    client->oldbuttons = client->buttons;
    client->buttons = ucmd->buttons;

    // this fixes the changing
    if ( NS_IsBot( ent ) )
        return;

    // only allow camera switch when game is running or over
    if ( GameState != STATE_LOCKED &&
            GameState != STATE_OVER )
        return;

    if ( g_teamlockcamera.integer && !( client->ps.pm_flags & PMF_FOLLOW ) &&
            GameState == STATE_LOCKED ) // only force camera when round begun
    {
        Cmd_FollowCycle_f( ent, 1 );
        ent->spec_updatetime = level.time;
    }
    // attack button cycles through spectators
    if ( client->sess.spectatorState == SPECTATOR_FOLLOW )
    {

        if ( ( client->buttons & BUTTON_ATTACK ) && ! ( client->oldbuttons & BUTTON_ATTACK ) ) {
            Cmd_FollowCycle_f( ent, 1 );
            ent->spec_updatetime = level.time;
        }
        else if ( ( client->buttons & BUTTON_WEAPON1 ) && ! ( client->oldbuttons & BUTTON_WEAPON1 ) ) {
            Cmd_FollowCycle_f( ent, -1 );
            ent->spec_updatetime = level.time;
        }

    }

    if ( ( client->buttons & BUTTON_USE ) && ! ( client->oldbuttons & BUTTON_USE ) ) {

        if ( client->sess.spectatorState != SPECTATOR_FOLLOW )
        {
            Cmd_FollowCycle_f( ent, 1 );
            ent->spec_updatetime = level.time;
        }
        else
        {
            PrintMsg( ent, "Camera Disabled.\n");
            StopFollowing( ent );
        }


    }

    /*	if ( ( client->buttons & BUTTON_USE ) && ! ( client->oldbuttons & BUTTON_USE ) ) {
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
    {
    ent->client->ps.pm_flags &= ~PMF_FOLLOW;
    PrintMsg( ent, "Camera Disabled.\n");
    ChangeCameraState ( &client->ps.generic1 , qfalse );
    }
    else
    {
    ent->client->ps.pm_flags |= PMF_FOLLOW;
    PrintMsg( ent, "Camera Enabled.\n");
    Cmd_FollowCycle_f( ent, 1 );
    ent->spec_updatetime = level.time;
    }
    }*//*
    /*
    if ( ( client->buttons & BUTTON_USE ) && ! ( client->oldbuttons & BUTTON_USE ) )
    {
    if ( client->sess.spectatorState != SPECTATOR_FOLLOW )
    client->sess.spectatorState = SPECTATOR_FOLLOW;
    else
    client->sess.spectatorState = SPECTATOR_FREE;
    }*/
}


/*
=================
ClientInactivityTimer

Returns qfalse if the client is dropped
=================
*/
qboolean ClientInactivityTimer( gclient_t *client ) {
    if ( ! g_inactivity.integer ) {
        // give everyone some time, so if the operator sets g_inactivity during
        // gameplay, everyone isn't kicked
        client->inactivityTime = level.time + 60 * ONE_SECOND;
        client->inactivityWarning = qfalse;
    } else if ( client->pers.cmd.forwardmove ||
                client->pers.cmd.rightmove ||
                client->pers.cmd.upmove ||
                (client->pers.cmd.buttons & BUTTON_ATTACK) ) {
        client->inactivityTime = level.time + g_inactivity.integer * ONE_SECOND;
        client->inactivityWarning = qfalse;
    } else if ( !client->pers.localClient ) {
        if ( level.time > client->inactivityTime ) {
            trap_DropClient( client - level.clients, "Dropped due to inactivity" );
            return qfalse;
        }
        if ( level.time > client->inactivityTime - 10000 && !client->inactivityWarning ) {
            client->inactivityWarning = qtrue;
            trap_SendServerCommand( client - level.clients, "cp \"Ten seconds until inactivity drop!\n\"" );
        }
    }
    return qtrue;
}

/*
==================
ClientTimerActions

Actions that happen 10times a second
==================
*/
void ClientTimerActions( gentity_t *ent, int msec ) {
    gclient_t *client;

    client = ent->client;
    client->timeResidual += msec;

    while ( client->timeResidual >= 100 ) {
        client->timeResidual -= 100;

        // only regenerate stamina when we're standing still
        if (
            (client->ps.stats[STAT_STAMINA] < 300) && ( client->ps.weaponstate != WEAPON_BANDAGING_START ) && ( client->ps.weaponstate != WEAPON_BANDAGING_END )
        )

        {
            int add;
            float speed = sqrt( ent->client->ps.velocity[0] * ent->client->ps.velocity[0] +  ent->client->ps.velocity[1] * ent->client->ps.velocity[1] + ent->client->ps.velocity[2] * ent->client->ps.velocity[2]);


            if ( (  speed < 5 || ( client->pers.nsPC.stamina > 5 && client->buttons & BUTTON_WALKING ) ) )
            {
                int stalvl = client->pers.nsPC.stamina;

                if ( stalvl > 8 )
                    stalvl = 8;

                add = 4 + ( stalvl ) - (client->bleed_num);

                if ( client->ps.powerups[PW_VEST] || client->ps.powerups[PW_HELMET] )
                    add--;

                if ( client->buttons & BUTTON_WALKING && speed > 5 )
                {
                    if ( add > 1 )
                            add = add / 3;

                    if ( client->ps.stats[STAT_STAMINA] < 200 )
                    {
                        client->ps.stats[STAT_STAMINA] += add;
                    }
                }
                else
                {
                    if ( client->ps.stats[STAT_STAMINA] > 300 )
                        client->ps.stats[STAT_STAMINA] = 300;

                    client->ps.stats[STAT_STAMINA] += add;
                }

                if (client->ps.stats[STAT_STAMINA] < 0)
                    client->ps.stats[STAT_STAMINA] = 0;
            }

        }
    }

}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( gclient_t *client ) {
    client->ps.eFlags &= ~EF_TALK;
    client->ps.eFlags &= ~EF_FIRING;

    // the level will exit when everyone wants to or after timeouts

    // swap and latch button actions
    client->oldbuttons = client->buttons;
    client->buttons = client->pers.cmd.buttons;
    if ( client->buttons & ( BUTTON_ATTACK  ) & ( client->oldbuttons ^ client->buttons ) ) {
        // this used to be an ^1 but once a player says ready, it should stick
        client->readyToExit = 1;
    }
}

void ClientHandleBombEvent( gentity_t *ent, int event )
{
    int wire = 0;
    int i = 0;

    if ( !ent->client->ns.bomb_world )
        return;

    if ( event == EV_CLIPWIRE_1 )
        wire = 1;
    else if ( event == EV_CLIPWIRE_2 )
        wire = 2;
    else if ( event == EV_CLIPWIRE_3 )
        wire = 3;
    else if ( event == EV_CLIPWIRE_4 )
        wire = 4;
    else if ( event == EV_CLIPWIRE_5 )
        wire = 5;
    else if ( event == EV_CLIPWIRE_6 )
        wire = 6;
    else if ( event == EV_CLIPWIRE_7 )
        wire = 7;
    else if ( event == EV_CLIPWIRE_8 )
        wire = 8;

    if ( ent->client->ns.bomb_world->health != wire )
    {
        if ( random() < 0.5 ) // bomb blow up!
        {
            // remove flag
            ent->client->ps.pm_flags &= ~PMF_BOMBCASE;

            bomb_explode( ent->client->ns.bomb_world );
            ent->client->ns.bomb_world = 0;

            for ( i = WP_SMOKE - 1 ; i > 0 ; i-- ) {
                if ( BG_GotWeapon( i, ent->client->ps.stats ) ) {
                    ent->s.weapon = ent->client->ps.weapon = i;
                    break;
                }
            }

            // remove weapon
            BG_RemoveWeapon( WP_C4, ent->client->ps.stats );
        }
        else
        {
            // tell the client that 'wire' was a wrong wire...
            trap_SendServerCommand( ent-g_entities, va("bombwire %i %i", wire, -1 ) );
        }
        return;
    }

    // bomb defused!
    bomb_defused( ent->client->ns.bomb_world, ent );

    // remove flag
    ent->client->ps.pm_flags &= ~PMF_BOMBCASE;
    ent->client->ns.bomb_world->count = 0;
    ent->client->ns.bomb_world = 0;

    for ( i = WP_SMOKE - 1 ; i > 0 ; i-- ) {
        if ( BG_GotWeapon( i, ent->client->ps.stats ) ) {
            ent->s.weapon = ent->client->ps.weapon = i;
            break;
        }
    }

    // remove weapon
    BG_RemoveWeapon( WP_C4, ent->client->ps.stats );
}

/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( gentity_t *ent, int oldEventSequence ) {
    int		event;
    gclient_t *client;
    int		damage;
    vec3_t	dir;
#ifdef MISSINOPACK
    vec3_t	origin, angles;
    gitem_t *item;
    gentity_t *drop;
    int		j;
#endif
    int		i;
    //	qboolean	fired;

    client = ent->client;

    if ( oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS ) {
        oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
    }
    for ( i = oldEventSequence ; i < client->ps.eventSequence ; i++ ) {
        event = client->ps.events[ i & (MAX_PS_EVENTS-1) ];

        switch ( event ) {
        case EV_FALL_LIGHT:
        case EV_FALL_MEDIUM:
        case EV_FALL_FAR:
        case EV_FALL_DEATH:
            if ( ent->s.eType != ET_PLAYER ) {
                break;		// not in the player model
            }
            if ( g_dmflags.integer & DF_NO_FALLING ) {
                break;
            }
            // Navy Seals ++
            if ( event == EV_FALL_DEATH ) {
                damage = 999; // immediate death.
            }
            else if ( event == EV_FALL_FAR ) {
                damage = 90;  // original 75

                // set wait time
                client->ps.pm_time = 2500;
                client->ps.pm_flags |= PMF_TIME_WATERJUMP;
            } else if ( event == EV_FALL_MEDIUM ) {
                damage = 65;  // original 50

                // set wait time
                client->ps.pm_time = 1500;
                client->ps.pm_flags |= PMF_TIME_WATERJUMP;
            } else if ( event == EV_FALL_SHORT ) {
                damage = 35;  // original 25
            } else {
                damage = 20;  // original 10
            }

            // Navy Seals --
            VectorSet (dir, 0, 0, 1);
            ent->pain_debounce_time = level.time + 200;	// no normal pain sound

            if (g_debugDamage.integer == 1) {
                G_Printf("Damage: %i\n", damage);
            }
            G_Damage (ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
            break;

        case EV_BREAKLOCK:
            break;
        case EV_FIRE_WEAPON:
        case EV_FIRE_WEAPON_OTHER:
            FireWeapon( ent );
            break;
            // Navy Seals ++
        case EV_C4DEPLOY:
            Weapon_C4( ent ); // deploy C4
            break;
            // Navy Seals --
        case EV_CLIPWIRE_1:
        case EV_CLIPWIRE_2:
        case EV_CLIPWIRE_3:
        case EV_CLIPWIRE_4:
        case EV_CLIPWIRE_5:
        case EV_CLIPWIRE_6:
        case EV_CLIPWIRE_7:
        case EV_CLIPWIRE_8:
            ClientHandleBombEvent( ent, event );
            break;
            // Navy Seals ++
        case EV_RELOAD:
            {
                if ( ent->client->ps.weapon == WP_870 || ent->client->ps.weapon == WP_M590 ) {
                    if ( ent->client->ps.ammo[ AM_SHOTGUN ] )
                    {
                        ent->client->ns.rounds[ent->client->ps.weapon]++;
                        // remove one clip
                        ent->client->ps.ammo[ AM_SHOTGUN ]--;

                        ent->client->ps.torsoAnim = ( ( ent->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | TORSO_RELOAD_RIFLE;
                        ent->client->ps.torsoTimer = 850;
                    }
                    else {
                        ent->client->ps.weaponstate = WEAPON_RELOADING_STOP;
                        ent->client->ps.weaponTime = 1000;
                        G_AddEvent(ent, EV_RELOAD_EMPTY, 0 );
                    }
                }
            }
            break;
        case EV_BANDAGING:
            NS_Bandaging( ent );
            break;
        case EV_CHANGE_WEAPON:
            {
                // if we got an active lasersight , deactivate it
                if ( ent->client->ns.lasersight )
                {
                    G_FreeEntity( ent->client->ns.lasersight );
                    ent->client->ns.lasersight = NULL;
                }
                break;

            }
            // Navy Seals --
        default:
            break;
        }
    }

}
int getBBox( gentity_t *ent )
{
    int i;

    for (i=0;i<MAX_CLIENTS;i++)
    {
        if ( level.head_bbox[i]->client == ent->client );
        return i;
    }
    return 0;


}
#if 0
/*
==============
StuckInOtherClient
==============
*/
static int StuckInOtherClient(gentity_t *ent) {
    int i;
    gentity_t	*ent2;

    ent2 = &g_entities[0];
    for ( i = 0; i < MAX_CLIENTS; i++, ent2++ ) {
        if ( ent2 == ent ) {
            continue;
        }
        if ( !ent2->inuse ) {
            continue;
        }
        if ( !ent2->client ) {
            continue;
        }
        if ( ent2->health <= 0 ) {
            continue;
        }
        //
        if (ent2->r.absmin[0] > ent->r.absmax[0])
            continue;
        if (ent2->r.absmin[1] > ent->r.absmax[1])
            continue;
        if (ent2->r.absmin[2] > ent->r.absmax[2])
            continue;
        if (ent2->r.absmax[0] < ent->r.absmin[0])
            continue;
        if (ent2->r.absmax[1] < ent->r.absmin[1])
            continue;
        if (ent2->r.absmax[2] < ent->r.absmin[2])
            continue;
        return qtrue;
    }
    return qfalse;
}
#endif
void BotTestSolid(vec3_t origin);
int OriginWouldTelefrag( vec3_t origin, int ignoreClientnum );

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( gentity_t *ent ) {
    gclient_t	*client;
    pmove_t		pm;
    int			oldEventSequence;
    int			msec;
    usercmd_t	*ucmd;
    int			speed;

    client = ent->client;

    // don't think if the client is not yet connected (and thus not yet spawned in)
    if (client->pers.connected != CON_CONNECTED) {
        return;
    }
    // mark the time, so the connection sprite can be removed
    ucmd = &ent->client->pers.cmd;

    // sanity check the command time to prevent speedup cheating
    if ( ucmd->serverTime > level.time + 200 ) {
        ucmd->serverTime = level.time + 200;
        //		G_Printf("serverTime <<<<<\n" );
    }
    if ( ucmd->serverTime < level.time - 1000 ) {
        ucmd->serverTime = level.time - 1000;
        //		G_Printf("serverTime >>>>>\n" );
    }

    msec = ucmd->serverTime - client->ps.commandTime;
    // following others may result in bad times, but we still want
    // to check for follow toggles
    if ( msec < 1 && client->sess.spectatorState != SPECTATOR_FOLLOW ) {
        return;
    }
    if ( msec > 200 ) {
        msec = 200;
    }

    if ( pmove_msec.integer < 8 ) {
        trap_Cvar_Set("pmove_msec", "8");
    }
    else if (pmove_msec.integer > 33) {
        trap_Cvar_Set("pmove_msec", "33");
    }

    if ( pmove_fixed.integer || client->pers.pmoveFixed ) {
        ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
        //if (ucmd->serverTime - client->ps.commandTime <= 0)
        //	return;
    }

    //
    // check for exiting intermission
    //
    if ( level.intermissiontime ) {
        ClientIntermissionThink( client );
        return;
    }

    // spectators don't do much
    // removing as defconx said 07.01.2003
    // if ( client->sess.sessionTeam == TEAM_SPECTATOR || ent->client->iMode > CAM_OFF ) {
    if ( client->sess.sessionTeam == TEAM_SPECTATOR ) {
        if ( client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
            return;
        }
        SpectatorThink( ent, ucmd );
        return;
    }

    if ( ent->client->sess.waiting )
    {
        SpectatorThink( ent, ucmd );
        return;
    }

    // Navy Seals --

    // check for inactivity timer, but never drop the local client of a non-dedicated server
    if ( !ClientInactivityTimer( client ) ) {
        return;
    }
    if ( ent->client->sess.waiting == qfalse )
        NS_ModifyClientBBox( ent );
#ifdef AWARDS
    // clear the rewards if time
    if ( level.time > client->rewardTime ) {
        client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
    }
#endif
    if ( client->noclip  /* Navy Seals ++ */// Navy Seals --
       ) {
        client->ps.pm_type = PM_NOCLIP;
    } else if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
        client->ps.pm_type = PM_DEAD;
    } else if ( client->sess.waiting ) {
        client->ps.pm_type = PM_NOCLIP;
        client->noclip = qtrue;
    } else {
        client->ps.pm_type = PM_NORMAL;

        if ( level.warmupTime > level.time )
            client->ps.pm_type = PM_FREEZE;
    }

    NS_SetClientCrosshairState( ent );

    client->ps.gravity = g_gravity.value;
    // Navy Seals ++
    // set speed (based on weigth)
    client->ns.weigth = NS_CalcWeight(ent);
    speed = NS_CalcSpeed(ent);

    NS_CorrectWeaponAim(ent);

    // Navy Seals --

    // BLUTENGEL:
    // kicks out zoom when jumping / falling / sprinting / climbimg ladder
    if ( BG_IsZooming( client->ps.stats[STAT_WEAPONMODE] ) &&
            ( ( ucmd->upmove > 0 && ( client->ps.velocity[2] > 0 || client->ps.velocity[2] < -10 ) ) ||
              ( client->buttons & BUTTON_SPRINT ) || (client->ps.pm_flags & PMF_CLIMB) ) ) {
        if ( client->ns.weaponmode[client->ps.weapon] & ( 1 << WM_ZOOM4X ) )
            client->ns.weaponmode[client->ps.weapon] &=~ ( 1 << WM_ZOOM4X );
        else if ( client->ns.weaponmode[client->ps.weapon] & ( 1 << WM_ZOOM2X ) )
            client->ns.weaponmode[client->ps.weapon] &=~ ( 1 << WM_ZOOM2X );
        G_LocalSound(ent, CHAN_WEAPON, G_SoundIndex("sound/weapons/zoom.wav") );
    }
    // simulate humping...
    if (/*level.framenum % 6 <= 2 && */client->ps.stats[STAT_LEG_DAMAGE] > 0 && client->ps.stats[STAT_HEALTH] > 0 && ent->waterlevel <= 2)
    {
        float limp = ( client->ps.stats[STAT_LEG_DAMAGE] );

        limp /= 175.0f;

        if ( limp > 0.6f )
            limp = 0.6f;

        limp = 1.0f - limp;

        client->ps.speed = speed * limp;
    }
    else
        client->ps.speed = speed;

    if ( ( client->ps.stats[STAT_ARM_DAMAGE] > 0 || client->ps.stats[STAT_HEAD_DAMAGE] > 0 ) && client->ps.stats[STAT_HEALTH] > 0 ) {
        float yaw,pitch;

        pitch = ( random() * ( client->ps.stats[STAT_ARM_DAMAGE]*15 + client->ps.stats[STAT_HEAD_DAMAGE]*7.5f ) / 2 );
        yaw = ( random() * ( client->ps.stats[STAT_ARM_DAMAGE]*15 + client->ps.stats[STAT_HEAD_DAMAGE]*7.5f ) / 2 );

        if (random() < 0.5)
            ent->client->ps.delta_angles[PITCH] += ANGLE2SHORT( pitch / (225));
        else
            ent->client->ps.delta_angles[PITCH] -= ANGLE2SHORT( yaw / (225));

        if (random() > 0.5)
            ent->client->ps.delta_angles[YAW] += ANGLE2SHORT( pitch / (225));
        else
            ent->client->ps.delta_angles[YAW] -= ANGLE2SHORT( yaw  / (225));
    }
    if ( client->ps.pm_flags & PMF_BOMBCASE )
        bomb_checkremovewire( ent );

    // set up for pmove
    oldEventSequence = client->ps.eventSequence;

    memset (&pm, 0, sizeof(pm));

    // check for the hit-scan gauntlet, don't let the action
    // go through as an attack unless it actually hits something
    if ( ( BG_IsMelee( client->ps.weapon ) ) && !( ucmd->buttons & BUTTON_TALK ) &&
            ( ucmd->buttons & BUTTON_ATTACK ) && client->ps.weaponTime <= 330 && client->ps.weaponTime > 0 ) {
        pm.gauntletHit = CheckMeleeAttack( ent, qfalse );
    } // stab mode
    else if ( ( client->ps.weapon == WP_M4 || client->ps.weapon == WP_AK47 ) && ent->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_BAYONET ) && !( ucmd->buttons & BUTTON_TALK ) &&
              client->ps.weaponstate == WEAPON_MELEE && client->ps.weaponTime <= 300 && client->ps.weaponTime > 0
            ) {
        pm.gauntletHit = CheckMeleeAttack( ent , qtrue );
    }
    if ( !(client->oldbuttons & BUTTON_ATTACK) && (ucmd->buttons & BUTTON_ATTACK ) &&
            client->ns.rounds[ client->ps.weapon ] <= 0 &&
            ( BG_IsPrimary( client->ps.weapon ) || BG_IsSecondary( client->ps.weapon ) ) &&
            ( client->ps.weaponstate == WEAPON_READY || client->ps.weaponstate == WEAPON_LASTRND ) &&
            client->ps.weaponTime <= 0
       )
        ent->client->ns.reload_tries++;

    if (	client->ps.weaponTime < 750 &&
            ( client->ps.weaponstate == WEAPON_RELOADING || client->ps.weaponstate == WEAPON_RELOADING_EMPTY ) &&
            client->ns.rounds[ client->ps.weapon ] != BG_GetMaxRoundForWeapon( client->ps.weapon ) &&
            client->ps.weapon == WP_M249
       )
        client->ns.rounds[ client->ps.weapon ] = BG_GetMaxRoundForWeapon( client->ps.weapon );

    pm.ps = &client->ps;
    pm.cmd = *ucmd;
    if ( pm.ps->pm_type == PM_DEAD ) {
        pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
    } else {
        pm.tracemask = MASK_PLAYERSOLID;
    }

    //
    // unstuck code. if multiple players spawn in one point.
    // then all get thrown into a random direction.
    //
    if ( OriginWouldTelefrag( client->ps.origin , client->ps.clientNum ) != -1 && client->unstuck == 1 )
    {
        pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
        client->unstuck = 1;

        client->ps.velocity[0] += -150 + random()*300;
        client->ps.velocity[1] += -150 + random()*300;
        client->ps.velocity[2] += 5 + random()*10;
    }
    else if ( OriginWouldTelefrag( client->ps.origin , client->ps.clientNum ) == -1 && client->unstuck == 1 )
        client->unstuck = 0;

    pm.trace = trap_Trace;
    pm.pointcontents = trap_PointContents;
    pm.debugLevel = g_debugMove.integer;
    pm.noFootsteps = ( g_dmflags.integer & DF_NO_FOOTSTEPS ) > 0;

    pm.pmove_fixed = pmove_fixed.integer | client->pers.pmoveFixed;
    pm.pmove_msec = pmove_msec.integer;

    VectorCopy( client->ps.origin, client->oldOrigin );

    Pmove (&pm);

    //if ( ent->client->sess.waiting == qfalse )
    //	NS_ModifyClientBBox( ent );

    // save results of pmove
    if ( ent->client->ps.eventSequence != oldEventSequence ) {
        ent->eventTime = level.time;
    }
    if (g_smoothClients.integer) {
        BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
    }
    else {
        BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
    }
    if ( !( ent->client->ps.eFlags & EF_FIRING ) ) {
        client->fireHeld = qfalse;		// for grapple
    }

    // use the snapped origin for linking so it matches client predicted versions
    VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

    VectorCopy (pm.mins, ent->r.mins);
    VectorCopy (pm.maxs, ent->r.maxs);

    ent->waterlevel = pm.waterlevel;
    ent->watertype = pm.watertype;

    // execute client events
    ClientEvents( ent, oldEventSequence );

    // link entity now, after any personal teleporters have been used
    trap_LinkEntity (ent);
    if ( !ent->client->noclip ) {
        G_TouchTriggers( ent );
    }

    // NOTE: now copy the exact origin over otherwise clients can be snapped into solid
    VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );

    // Update the clients anti-lag history
    G_UpdateClientAntiLag( ent );


    // touch other objects
    ClientImpacts( ent, &pm );

    // save results of triggers and client events
    if (ent->client->ps.eventSequence != oldEventSequence) {
        ent->eventTime = level.time;
    }

    // swap and latch button actions
    client->oldbuttons = client->buttons;
    client->buttons = ucmd->buttons;
    client->latched_buttons |= client->buttons & ~client->oldbuttons;

    if ( client->ps.weaponTime <= 0 && ( client->buttons & BUTTON_USE && !(client->oldbuttons & BUTTON_USE) ) )
        NS_OpenDoor(ent, qfalse);

    // reloading; i do this now that way, because the server sometimes drops client commands. very annoying.
    if ( (client->ns.reload_tries || ( client->buttons & BUTTON_RELOAD && !(client->oldbuttons & BUTTON_RELOAD) ) )  )
    {
        if ( ( client->buttons & BUTTON_RELOAD && !(client->oldbuttons & BUTTON_RELOAD) ) )
            Cmd_Reload_f( ent );

        if ( client->pers.autoReload )
        {
            if ( client->ns.reload_tries > 0 )
                Cmd_Reload_f( ent );
        }
    }

    if ( client->ns.weaponmode_tries[0] || ( client->buttons & BUTTON_WEAPON1 && !(client->oldbuttons & BUTTON_WEAPON1) ) )
        NS_WeaponMode( ent, 0 );
    if ( client->ns.weaponmode_tries[1] || ( client->buttons & BUTTON_WEAPON2 && !(client->oldbuttons & BUTTON_WEAPON2) ) )
        NS_WeaponMode( ent, 1 );
    if ( client->ns.weaponmode_tries[2] || ( client->buttons & BUTTON_WEAPON3 && !(client->oldbuttons & BUTTON_WEAPON3) ) )
        NS_WeaponMode( ent, 2 );

    // check for respawning
    if ( client->ps.stats[STAT_HEALTH] <= 0 )
    {
        // wait for the attack button to be pressed
        if ( level.time > client->respawnTime )
        {
            // Navy Seals ++
            if( g_gametype.integer == GT_LTS )
            {
                if ( client->sess.sessionTeam != TEAM_SPECTATOR )
                {
                    // dx: not leaving bodys suxx ass
                    if ( g_gametype.integer == GT_LTS )
                        ent->client->sess.waiting = qtrue;

                    respawn( ent );
                    return;
                }
            }
            else
            {
                // forcerespawn is to prevent users from waiting out powerups
                if ( g_forcerespawn.integer > 0 &&
                        ( level.time - client->respawnTime ) > g_forcerespawn.integer * 1000 )
                {
                    if ( g_gametype.integer == GT_LTS )
                        ent->client->sess.waiting = qtrue;

                    respawn( ent );
                    return;
                }
            }
            // Navy Seals --
            // pressing attack or use is the normal respawn method
            if ( ucmd->buttons & ( BUTTON_ATTACK  ) ||
                    ucmd->buttons & ( BUTTON_USE  ) )
            {
                if ( g_gametype.integer == GT_LTS )
                    ent->client->sess.waiting = qtrue;

                respawn( ent );
            }
        }
        return;
    }

    // perform once-a-second actions
    ClientTimerActions( ent, msec );
}

/*
==================
ClientThink

A new command has arrived from the client
==================
*/

void ClientThink( int clientNum ) {
    gentity_t *ent;

    ent = g_entities + clientNum;
    trap_GetUsercmd( clientNum, &ent->client->pers.cmd );

    // mark the time we got info, so we can display the
    // phone jack if they don't get any for a while
    ent->client->lastCmdTime = level.time;

    if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
        ClientThink_real( ent );
    }
}


void G_RunClient( gentity_t *ent ) {
    if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) {
        return;
    }

    ent->client->pers.cmd.serverTime = level.time;
    ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( gentity_t *ent ) {
    gclient_t	*cl;

    // make sure headbbox is gone
    if ( level.head_bbox[ ent->client->ps.clientNum ]->inuse )
        trap_UnlinkEntity( level.head_bbox[ ent->client->ps.clientNum ] );

    // if we are doing a chase cam or a remote view, grab the latest info
    if ( ent->client->sess.spectatorState == SPECTATOR_FOLLOW ) {
        int		clientNum , flags;
        int		kills = ent->client->ps.persistant[PERS_SCORE];
        int		ping	= ent->client->ps.ping;
        clientNum = ent->client->sess.spectatorClient;

        // team follow1 and team follow2 go to whatever clients are playing
        if ( clientNum == -1 ) {
            clientNum = level.follow1;
        } else if ( clientNum == -2 ) {
            clientNum = level.follow2;
        }
        if ( clientNum >= 0 ) {
            cl = &level.clients[ clientNum ];
            if ( cl->pers.connected == CON_CONNECTED
                    && cl->sess.sessionTeam != TEAM_SPECTATOR
                    && !cl->sess.waiting
                    && cl->ps.pm_type != PM_FREEZE ) {

                flags = (cl->ps.eFlags & ~(EF_VOTED | EF_TEAMVOTED )) | (ent->client->ps.eFlags & (EF_VOTED | EF_TEAMVOTED));
                ent->client->ps = cl->ps;
                ent->client->ps.pm_flags |= PMF_FOLLOW;
                ent->r.svFlags |= SVF_NOCLIENT;
                ent->client->ps.eFlags = flags;
                ent->client->ps.persistant[PERS_SCORE] = kills;
                ent->client->ps.ping = ping;
            } else {
                // drop them to free spectators unless they are dedicated camera followers
                if ( ent->client->sess.spectatorClient >= 0 ) {
                    StopFollowing( ent ); // just stop following
                }
            }
        }
    }

    // update character if anything changed - so knows about any changes
    ent->client->ps.persistant[PERS_STRENGTH] = ent->client->pers.nsPC.strength;
    ent->client->ps.persistant[PERS_TECHNICAL] = ent->client->pers.nsPC.technical;
    ent->client->ps.persistant[PERS_STAMINA] = ent->client->pers.nsPC.stamina;
    ent->client->ps.persistant[PERS_SPEED] = ent->client->pers.nsPC.speed;
    ent->client->ps.persistant[PERS_STEALTH] = ent->client->pers.nsPC.stealth;
    ent->client->ps.persistant[PERS_ACCURACY] = ent->client->pers.nsPC.accuracy;
    ent->client->ps.persistant[PERS_XP] = ent->client->pers.nsPC.entire_xp;

    if ( ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD ) {
        ent->client->ps.pm_flags |= PMF_SCOREBOARD;
    } else {
        ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
    }
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( gentity_t *ent ) {
    //	int			i;
    clientPersistant_t	*pers;

    if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ||
            ent->client->sess.spectatorState == SPECTATOR_FOLLOW ||
            ent->client->sess.waiting ) {
        SpectatorClientEndFrame( ent );
        return;
    }

    pers = &ent->client->pers;

    // save network bandwidth
#if 0
    if ( !g_synchronousClients->integer && ent->client->ps.pm_type == PM_NORMAL ) {
        // FIXME: this must change eventually for non-sync demo recording
        VectorClear( ent->client->ps.viewangles );
    }
#endif

    //
    // If the end of unit layout is displayed, don't give
    // the player any normal movement attributes
    //
    if ( level.intermissiontime ) {
        return;
    }

    // burn from lava, etc
    P_WorldEffects ( ent );

    // apply all the damage taken this frame
    P_DamageFeedback ( ent );

    NS_CauseBleeding( ent );

    NS_CheckRemoveTeamKill( ent );

    if ( ent->client->pers.radarUpdateTime > 0 )
        NS_CalculateRadar( ent );
    // radio
    RadioThink(ent);

    // add the EF_CONNECTION flag if we haven't gotten commands recently
    if ( level.time - ent->client->lastCmdTime > 1000 ) {
        ent->s.eFlags |= EF_CONNECTION;
    } else {
        ent->s.eFlags &= ~EF_CONNECTION;
    }

    if (
        ( ( level.time - ent->client->respawnTime ) < RESPAWN_INVUNERABILITY_TIME ) &&
        ent->health == ent->client->pers.maxHealth &&
        g_gametype.integer == GT_TEAM )
        ent->client->ps.eFlags |= EF_REDGLOW;
    else
        ent->client->ps.eFlags &= ~EF_REDGLOW;
    /*
    if ( ent->client->ns.is_vip )
    ent->client->ps.eFlags |= EF_VIP;
    else
    ent->client->ps.eFlags &= ~EF_VIP; */

    ent->client->ps.persistant[PERS_XP] = ent->client->pers.nsPC.entire_xp;
    ent->client->ps.stats[STAT_WEAPONMODE] = ent->client->ns.weaponmode[ent->s.weapon];
    ent->client->ps.stats[STAT_HEALTH] = ent->health;	// FIXME: get rid of ent->health...

    if (ent->client->ns.weaponmode[ent->s.weapon] & ( 1 << WM_GRENADELAUNCHER ) &&  ent->client->ns.weaponmode[ent->s.weapon] & ( 1 << WM_WEAPONMODE2 ) )
        ent->client->ps.stats[STAT_ROUNDS] = ent->client->ns.rounds[ent->s.weapon*3]; // ns
    else
        ent->client->ps.stats[STAT_ROUNDS] = ent->client->ns.rounds[ent->s.weapon]; // ns

    NS_AdjustClientVWeap( ent->client->ns.weaponmode[ent->s.weapon] , ent->client->ps.powerups );

    //bot
    if ( NS_IsBot(ent) )
    {
        if (ent->client->ps.stats[STAT_ROUNDS] <= 0)
            Cmd_Reload_f(ent);

        if (ent->client->bleed_num)
        {
            NS_StartBandage( ent );
        }
    }

    // Navy Seals --
    G_SetClientSound (ent);

    // set the latest infor
    if (g_smoothClients.integer) {
        BG_PlayerStateToEntityStateExtraPolate( &ent->client->ps, &ent->s, ent->client->ps.commandTime, qtrue );
    }
    else {
        BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );
    }

    // set the bit for the reachability area the client is currently in
    //	i = trap_AAS_PointReachabilityAreaIndex( ent->client->ps.origin );
    //	ent->client->areabits[i >> 3] |= 1 << (i & 7);
}


