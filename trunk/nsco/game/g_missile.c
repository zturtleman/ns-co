// Copyright (C) 1999-2000 Id Software, Inc.
//

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

#define	MISSILE_PRESTEP_TIME	1

void Lead_Impact ( gentity_t *lead, trace_t *trace );
void Touch_GlassTrigger (gentity_t *ent, gentity_t *other, trace_t *trace);
void assault_link_all( qboolean unlink );


/*
============
G_TouchMissileTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchMissileTriggers( gentity_t *ent ) {
    int			i, num;
    int			touch[MAX_GENTITIES];
    gentity_t	*hit;
    trace_t		trace;
    vec3_t		mins, maxs;
    static vec3_t	range = { 2, 2, 2 };



    VectorSubtract( ent->r.currentOrigin, range, mins );
    VectorAdd( ent->r.currentOrigin, range, maxs );

    num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );



    for ( i=0 ; i<num ; i++ ) {
        hit = &g_entities[touch[i]];

        if ( !hit->touch   ) {
            continue;
        }
        if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
            continue;
        }

        // use seperate code for determining if an item is picked up
        // so you don't have to actually contact its bounding box
        if ( !trap_EntityContact( mins, maxs, hit ) ) {
            continue;
        }

        memset( &trace, 0, sizeof(trace) );

        hit->touch (hit, ent, &trace);
    }
}

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
    vec3_t	velocity;
    float	dot;
    int		hitTime;

    // reflect the velocity on the trace plane
    hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
    BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
    dot = DotProduct( velocity, trace->plane.normal );
    VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

    ent->s.pos.trDelta[2] *= 0.5f;

    if ( trace->surfaceFlags & SURF_DIRTSTEPS || trace->surfaceFlags & SURF_SANDSTEPS || trace->surfaceFlags & SURF_SOFTSTEPS )
        ent->s.pos.trDelta[2] *= 0.2f;
    else if ( trace->surfaceFlags & SURF_WOODSTEPS || trace->surfaceFlags & SURF_GLASS )
        ent->s.pos.trDelta[2] *= 0.5f;
    else if ( trace->surfaceFlags & SURF_METALSTEPS )
        ent->s.pos.trDelta[2] *= 0.8f;
    else if ( trace->surfaceFlags & SURF_SNOWSTEPS )
        ent->s.pos.trDelta[2] *= 0.1f;
    else
        ent->s.pos.trDelta[2] *= 0.65f;

    ent->s.pos.trDelta[1] *= 0.8f;
    ent->s.pos.trDelta[0] *= 0.8f;

    if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
        int mindelta = 40;

        if ( ent->fly_sound_debounce_time ) {
            VectorScale( ent->s.pos.trDelta, 0.80, ent->s.pos.trDelta );
            //		G_Printf("delta: %f normal:%2f\n", VectorLength( ent->s.pos.trDelta ), trace->plane.normal[2] );
        }
        else
            VectorScale( ent->s.pos.trDelta, 0.4, ent->s.pos.trDelta );
        // check for stop
        if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < mindelta ) {
            G_SetOrigin( ent, trace->endpos );
            return;
        }
    }

    VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
    VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
    ent->s.pos.trTime = level.time;
}


#define         FLASH_RADIUS                    200
#define         BLIND_FLASH                     50      // Time of blindness in FRAMES

qboolean pointinback( gentity_t *self, vec3_t point );

qboolean G_FlashBangExplode ( vec3_t origin, gentity_t *self, gentity_t *attacker, int blindtime, float radius, gentity_t *ignore ) {
    int			dist;
    int			i ;
    qboolean	hitClient = qfalse;
    gclient_t	*client;

    // unlink all assault fields
    assault_link_all( qtrue );

    if ( radius < 1.0 ) {
        radius = 1.0;
    }

    // org is in ground try to raise the flash org to center.
    origin[2] += 10.0f;

    // get a point that's not in any wall
    while ( trap_PointContents( origin, self->s.number ) & CONTENTS_SOLID )
    {
        //		G_Printf("grenadepoint stuck. changing: %f\n", origin[2] );
        origin[2] -= 2.0f;

        if ( origin[2] < self->r.currentOrigin[2] ) // don't get stuck in a loop
        {
            origin[2] = self->r.currentOrigin[2]; // don't put the flashbang into the floor
            break;
        }
    }

    // give any nearby players a flash event
    for ( i = 0 ; i < level.maxclients ; i++ )
    {
        vec3_t		delta;
        float		len;
        float		temp;
        vec3_t		forward;
        trace_t		tr;
        qboolean	behindPlayer = qfalse;
        vec3_t		org;

        client = &level.clients[i];

        if ( client == ignore->client ) {
            continue;
        }
        if ( client->pers.connected == CON_DISCONNECTED ) {
            continue;
        }
        if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
            continue;
        }
        if ( client->sess.waiting ) {
            continue;
        }
        if (
            ( ( level.time - client->respawnTime ) < RESPAWN_INVUNERABILITY_TIME ) &&
            g_gametype.integer == GT_TEAM
        )
            continue;

        // if too far away
        VectorSubtract( origin, client->ps.origin, delta );
        len = VectorNormalize( delta );
        if ( len > radius ) {
            continue;
        }

        // figure out if we're facing the bang
        AngleVectors( client->ps.viewangles, forward, NULL, NULL );

        if ( DotProduct( delta, forward ) < 0.4 ) {
            behindPlayer = qtrue;
        }

        // if not line of sight, no sound
        VectorCopy( client->ps.origin, org );
        org[2] += client->ps.viewheight;

        trap_Trace( &tr, org, NULL, NULL, origin, client->ps.clientNum, ( CONTENTS_SOLID | CONTENTS_BODY ) );

        if ( tr.fraction != 1.0 ) {
            continue;
        }

        hitClient = qtrue;

        //  blindtime * ( radius - len ) / radius
        temp = (radius-len)/radius;
        dist = blindtime * ( temp );


        // now let him flash
        {
            gentity_t *tent = G_TempEntity( origin, EV_FLASHBANG );

            tent->s.otherEntityNum = client->ps.clientNum;
            tent->s.frame = dist;
            tent->s.eventParm = behindPlayer;

            // make suire clients recieve this packet
            tent->r.singleClient = client->ps.clientNum;
            tent->r.svFlags |= SVF_SINGLECLIENT;
        }
    }

    assault_link_all( qfalse ); // relink all assaultfields
    return hitClient;
}

void G_ExplodeSmokenade( gentity_t *ent ) {

    // clear the lower 16 bits (there will be the angle stored)
    ent->s.frame &= SEALS_SMOKEMASK_FLAGS;

    ent->s.generic1 = MF_SMOKE;
    ent->r.svFlags |= SVF_BROADCAST;

    if (ent->count <= 0) {

        // after SEALS_SMOKENADETIME the smokegrenade vanishes
        ent->nextthink = level.time + SEALS_SMOKENADETIME;
        ent->think = G_FreeEntity;

    } else {

        // check the area every 10 steps
        if ((ent->count % 10) == 0) {
            trace_t tr;
            vec3_t start, end, dir;

            VectorCopy(ent->r.currentOrigin, start);
            start[2]+=16.0;

            // unlink all assault fields
            assault_link_all( qtrue );

            // get a point that's not in any wall
            while ( trap_PointContents( start, ent->s.number ) & CONTENTS_SOLID ) {
                start[2] -= 2.0f;

                if ( start[2] < ent->r.currentOrigin[2] ) {
                    start[2] = ent->r.currentOrigin[2]; // don't put the flashbang into the floor
                    break;
                }
            }

            // first clear the old the old directions
            ent->s.frame &= ~SEALS_SMOKEMASK_FLAGS;

            // check UP
            dir[0] = 0.0;
            dir[1] = 0.0;
            dir[2] = 1.0;
            VectorMA( start, SEALS_SMOKENADE_DISTANCE, dir, end);

            trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SOLID);
            if (tr.fraction > 1.0) tr.fraction = 1.0;
            ent->s.frame |= ((int)(SEALS_SMOKEMASK_VALUE*tr.fraction)) << SEALS_SMOKEMASK_SUP;

            // check LEFT
            dir[0] = -1.0;
            dir[1] = 0.0;
            dir[2] = 0.0;
            VectorMA( start, SEALS_SMOKENADE_DISTANCE, dir, end);

            trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SOLID);
            if (tr.fraction > 1.0) tr.fraction = 1.0;
            ent->s.frame |= ((int)(SEALS_SMOKEMASK_VALUE*tr.fraction)) << SEALS_SMOKEMASK_SLEFT;

            // check RIGHT
            dir[0] = 1.0;
            dir[1] = 0.0;
            dir[2] = 0.0;
            VectorMA( start, SEALS_SMOKENADE_DISTANCE, dir, end);

            trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SOLID);
            if (tr.fraction > 1.0) tr.fraction = 1.0;
            ent->s.frame |= ((int)(SEALS_SMOKEMASK_VALUE*tr.fraction)) << SEALS_SMOKEMASK_SRIGHT;

            // check FORWARD
            dir[0] = 0.0;
            dir[1] = 1.0;
            dir[2] = 0.0;
            VectorMA( start, SEALS_SMOKENADE_DISTANCE, dir, end);

            trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SOLID);
            if (tr.fraction > 1.0) tr.fraction = 1.0;
            ent->s.frame |= ((int)(SEALS_SMOKEMASK_VALUE*tr.fraction)) << SEALS_SMOKEMASK_SFORWARD;

            // check BACKWARD
            dir[0] = 0.0;
            dir[1] = -1.0;
            dir[2] = 0.0;
            VectorMA( start, SEALS_SMOKENADE_DISTANCE, dir, end);

            trap_Trace( &tr, start, NULL, NULL, end, ent->s.number, MASK_SOLID);
            if (tr.fraction > 1.0) tr.fraction = 1.0;
            ent->s.frame |= ((int)(SEALS_SMOKEMASK_VALUE*tr.fraction)) << SEALS_SMOKEMASK_SBACKWARD;

            // relink all assault fields
            assault_link_all( qfalse );

        }

        ent->count--;
        ent->nextthink = level.time + SEALS_SMOKENADETIME;
        ent->think = G_ExplodeMissile;
    }

    // the same random seed for all players
    ent->s.frame &= ~SEALS_SMOKEMASK_RNDNUM;
    ent->s.frame |= ((int)(64*random())) & SEALS_SMOKEMASK_RNDNUM ;

    trap_LinkEntity(ent);
}

/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
    vec3_t		dir;
    vec3_t		origin;

    ent->takedamage = qfalse;

    if ( !Q_stricmp( ent->classname , "smokegrenade" ) ||
            ent->s.weapon == WP_SMOKE ) {

        if (ent->s.generic1 != MF_SMOKE) {
            G_Sound( ent, CHAN_AUTO, G_SoundIndex("sound/misc/40mm_explode.wav") );
            ent->s.loopSound = G_SoundIndex( "sound/misc/40mm_gasloop.wav" );
        }

        ent->nextthink = level.time + 100;
        ent->think = G_ExplodeSmokenade;

        ent->s.generic1 = MF_SMOKE;
        ent->r.svFlags |= SVF_BROADCAST;

        /* 	*/
        return;
    }

    BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
    SnapVector( origin );
    G_SetOrigin( ent, origin );

    // we don't have a valid direction, so just point straight up
    dir[0] = dir[1] = 0;
    dir[2] = 1;

    ent->s.eType = ET_GENERAL;
    G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ) );

    ent->r.svFlags = SVF_BROADCAST;	// send to everyone
    ent->freeAfterEvent = qtrue;


    if ( ent->s.weapon == WP_FLASHBANG )
    {
        G_FlashBangExplode( ent->r.currentOrigin, ent, ent->parent, SEALS_FLASHBANGTIME , ent->splashRadius, ent );
        trap_LinkEntity( ent );
        return;
    }

    // splash damage
    if ( ent->splashDamage ) {
        if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent, ent->splashMethodOfDeath ) ) {
            g_entities[ent->r.ownerNum].client->accuracy_hits++;
        }
    }

    trap_LinkEntity( ent );
}

void G_FlashBangDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {

    if (inflictor == self)
        return;

    self->takedamage = qfalse;
    self->think = G_ExplodeMissile;
    self->nextthink = level.time + 150;
}

void G_SmokeNadeDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {

    if (inflictor == self)
        return;

    self->takedamage = qfalse;
    self->think = G_ExplodeMissile;
    self->nextthink = level.time + 150;
}

void G_GrenadeDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {

    if (inflictor == self)
        return;

    self->takedamage = qfalse;
    self->think = G_ExplodeMissile;
    self->nextthink = level.time + 250;
}

void G_40MMGrenadeDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod ) {

    if (inflictor == self)
        return;

    self->takedamage = qfalse;
    self->think = G_ExplodeMissile;
    self->nextthink = level.time + 250;
}


/*
================
G_MissileImpact
================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
    gentity_t		*other;
    qboolean		hitClient = qfalse;

    other = &g_entities[trace->entityNum];

    // check for bounce
    // Navy Seals ++

    if ( !Q_stricmp( ent->classname, "reallead") )
    {
        Lead_Impact( ent, trace );
        return;
    }

    if (!Q_stricmp( ent->classname, "40mmgrenade") ) {
        if ( (level.time > (ent->timestamp + SEALS_40MMGREN_ARMEDTIME)) && ent->count == 0) {
            ent->count = 1;
            ent->nextthink = level.time + 10;
            ent->think = G_ExplodeMissile;
            ent->r.svFlags |= SVF_BROADCAST;
        } else if (ent->count == 0) {
            ent->count = 1;
            ent->nextthink = level.time + 15000;
            ent->think = G_FreeEntity;
            ent->r.svFlags |= SVF_BROADCAST;
        } else {
            ent->r.svFlags |= SVF_BROADCAST;
        }
    }

    if ( ( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) )  {

        G_BounceMissile( ent, trace );

        // no sound, just idle
        if ( ent->s.pos.trType == TR_STATIONARY )
            return;

        if ( ent->fly_sound_debounce_time ) {
            // so it won't produce a bounce event too often
            if ( VectorLength( ent->s.pos.trDelta ) < 450 )
                return;
        }
        G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
        return;
    }

    // impact damage
    if (other->takedamage) {
        // FIXME: wrong damage direction?
        if ( ent->damage ) {
            vec3_t	velocity;

            if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
                g_entities[ent->r.ownerNum].client->accuracy_hits++;
                hitClient = qtrue;
            }
            BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
            if ( VectorLength( velocity ) == 0 ) {
                velocity[2] = 1;	// stepped on a grenade
            }
            G_Damage ( other, ent, &g_entities[ent->r.ownerNum], velocity,
                       ent->s.origin, ent->damage,
                       DAMAGE_NO_BLEEDING, ent->methodOfDeath);
        }
    }

    /*	if ( !Q_stricmp( ent->classname , "40mmgrenade" ) )
    {
    G_BounceMissile( ent, trace );

    // no sound, just idle
    if ( ent->s.pos.trType == TR_STATIONARY )
    return;

    G_AddEvent( ent, EV_GRENADE_BOUNCE, 0 );
    return;
    }*/

    // is it cheaper in bandwidth to just remove this ent and create a new
    // one, rather than changing the missile into the explosion?

    if ( other->takedamage && other->client ) {
        G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ) );
        ent->s.otherEntityNum = other->s.number;
    } else if( trace->surfaceFlags & SURF_METALSTEPS ) {
        G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ) );
    } else {
        G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ) );
    }

    // change over to a normal entity right at the point of impact
    ent->s.eType = ET_GENERAL;

    SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

    G_SetOrigin( ent, trace->endpos );

    // splash damage (doesn't apply to person directly hit)
    if ( ent->splashDamage ) {
        if( G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius,ent, ent->splashMethodOfDeath ) ) {
            if( !hitClient ) {
                g_entities[ent->r.ownerNum].client->accuracy_hits++;
            }
        }
    }

    trap_LinkEntity( ent );
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent ) {
    vec3_t		origin; 
    trace_t		tr;
    int			passent;
    qboolean	inwater = qfalse;

    // get current position
    BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

    if (trap_PointContents(origin,-1) & MASK_WATER)
        inwater = qtrue;
 
    // if this missile bounced off an invulnerability sphere
    if ( ent->target_ent ) {
        passent = ent->target_ent->s.number;
    }
    else {
        // ignore interactions with the missile owner
        passent = ent->r.ownerNum;
    }
 
    // trace a line from the previous position to the current position
    trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

    if (tr.startsolid || tr.allsolid) {
        G_Printf("Entity: %s\n", g_entities[tr.entityNum].classname);
        // make sure the tr.entityNum is set to the entity we're stuck in
        trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );

        tr.fraction = 0;
    }
    else {
        VectorCopy( tr.endpos, ent->r.currentOrigin );
    }

    trap_LinkEntity( ent );

    if ( !Q_stricmp( ent->classname, "reallead") )
        if ( ent->r.ownerNum != ENTITYNUM_NONE )
            if ( Distance( g_entities[ent->r.ownerNum].r.currentOrigin , ent->r.currentOrigin ) > 50 )
                ent->r.ownerNum = ENTITYNUM_NONE;

    if ( tr.fraction != 1 ) {
        if (
            ( BG_IsGrenade( ent->s.weapon )
              || !Q_stricmp( ent->classname, "40mmgrenade")
            ) &&
            !Q_stricmp( g_entities[tr.entityNum].classname, "func_explosive_glass")
        )
        {
            Touch_GlassTrigger( &g_entities[tr.entityNum], ent, &tr );
            return;
        }

        // never explode or bounce on sky
        // BLUTENGEL_XXX: fixed it a strange way
        // now nades can bounce from invisible walls over the skybox
        // dunno if thats exactly what i want, but it's definitly better
        // than before -> needs to be tested
        // only other solution: nades vanish if hitting skybox
        if ( tr.surfaceFlags & SURF_NOIMPACT  &&
                !BG_IsGrenade( ent->s.weapon ) ) {
            G_FreeEntity( ent );
            return;
        } else if ( !(tr.surfaceFlags & SURF_SKY) || !BG_IsGrenade( ent->s.weapon) ){
            G_MissileImpact( ent, &tr );
            if ( ent->s.eType != ET_MISSILE ) {
                return;		// exploded
            }
        }
    }

    // check think function after bouncing
    G_RunThink( ent );
}


//=============================================================================


/*
=================
fire_grenade
=================
*/
gentity_t *fire_grenade (gentity_t *self, vec3_t start, vec3_t dir, int firestrength) {
    gentity_t	*bolt;

    if ( firestrength < 0 )
        firestrength *= -1;

    VectorNormalize (dir);

    bolt = G_Spawn();
    bolt->classname = "grenade";
    bolt->nextthink = level.time + 3000;

    // 3sec default priming
    if ( self->client->ns.weaponmode[self->client->ps.weapon] & ( 1 << WM_SINGLE) )
        bolt->nextthink = level.time + 4000; // 3sec default priming
    else if ( self->client->ns.weaponmode[self->client->ps.weapon] & ( 1 << WM_WEAPONMODE2 ) )
        bolt->nextthink = level.time + 5000;

    bolt->think = G_ExplodeMissile;
    bolt->s.eType = ET_MISSILE;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    bolt->s.weapon = WP_GRENADE ;
    bolt->s.eFlags = EF_BOUNCE_HALF;
    bolt->r.ownerNum = self->s.number;
    bolt->timestamp = level.time;
    bolt->parent = self;
    bolt->damage = SEALS_GRENADEDAMAGE;
    bolt->splashDamage = SEALS_GRENADESPLASHDAMAGE;
    bolt->splashRadius = SEALS_GRENADERADIUS;
    bolt->methodOfDeath = MOD_GRENADE;
    bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
    bolt->clipmask = MASK_SHOT;
    bolt->target_ent = NULL;

    // shootable grenades
    bolt->health = 4;
    bolt->takedamage = qtrue;
    bolt->r.contents = CONTENTS_BODY;
    bolt->die = G_GrenadeDie;
    VectorSet(bolt->r.mins, -2, -2, 0);
    VectorCopy(bolt->r.mins, bolt->r.absmin);
    VectorSet(bolt->r.maxs, 2, 2, 2);
    VectorCopy(bolt->r.maxs, bolt->r.absmax);
    // /shootable grenades


    if ( self->client && self->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADEROLL ) )
    {
        start[2] -= 15;
        bolt->fly_sound_debounce_time = qtrue;//rolling
        VectorScale( dir, SEALS_BASEGRENRANGE_ROLL +
                     ((float)(self->client->pers.nsPC.strength)/(10.0f) *
                      SEALS_BASEGRENRANGE_ROLL *
                      SEALS_BASEGRENRANGE_ADDSTRENGTH )
                     + firestrength , bolt->s.pos.trDelta );
    } else {
        VectorScale( dir, SEALS_BASEGRENRANGE_THROW +
                     ((float)(self->client->pers.nsPC.strength)/(10.0f) *
                      SEALS_BASEGRENRANGE_THROW *
                      SEALS_BASEGRENRANGE_ADDSTRENGTH ) +
                     firestrength , bolt->s.pos.trDelta );
    }

    bolt->s.pos.trType = TR_GRAVITY;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
    VectorCopy( start, bolt->s.pos.trBase );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

    VectorCopy (start, bolt->r.currentOrigin);

    return bolt;
}
/*
=================
fire_40mmgrenade
=================
*/
gentity_t *fire_40mmgrenade (gentity_t *self, vec3_t start, vec3_t dir, int speed) {
    gentity_t	*bolt;

    VectorNormalize (dir);

    bolt = G_Spawn();
    bolt->classname = "40mmgrenade";
    bolt->think = G_ExplodeMissile;
    bolt->nextthink = level.time + 5000;	  // disappear after 3 seconds

    bolt->s.eType = ET_MISSILE;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    bolt->s.weapon = WP_GRENADE;
    bolt->s.eFlags = EF_BOUNCE_HALF;
    bolt->s.frame = self->s.weapon;
    bolt->r.ownerNum = self->s.number;
    bolt->parent = self;
    bolt->takedamage = qfalse;
    bolt->timestamp = level.time;
    bolt->damage = SEALS_40MMGRENDAMAGE;
    bolt->splashDamage = SEALS_40MMGRENSPLASHDAMAGE;
    bolt->splashRadius = SEALS_40MMGRENRADIUS;
    bolt->methodOfDeath = MOD_GRENADE;
    bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
    bolt->clipmask = MASK_SHOT;
    bolt->target_ent = NULL;
    bolt->count = 0;

    // shootable 40mmgrenade
    bolt->health = 4;
    bolt->takedamage = qtrue;
    bolt->r.contents = CONTENTS_BODY;
    bolt->die = G_40MMGrenadeDie;
    VectorSet(bolt->r.mins, -1.5, -1.5, 0);
    VectorCopy(bolt->r.mins, bolt->r.absmin);
    VectorSet(bolt->r.maxs, 1.5, 1.5, 1.5);
    VectorCopy(bolt->r.maxs, bolt->r.absmax);
    // /shootable 40mmgrenade

    VectorScale( dir, speed, bolt->s.pos.trDelta );

    bolt->s.pos.trType = TR_MOREGRAVITY;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame

    VectorCopy( start, bolt->s.pos.trBase );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

    VectorCopy (start, bolt->r.currentOrigin);

    VectorCopy( start, bolt->pos1 );



    return bolt;
}
/*
=================
fire_smoke
=================
*/
gentity_t *fire_smoke (gentity_t *self, vec3_t start, vec3_t dir, int firestrength) {
    gentity_t	*bolt;

    if ( firestrength < 0 )
        firestrength *= -1;

    VectorNormalize (dir);

    bolt = G_Spawn();
    bolt->classname = "smokegrenade";
    bolt->nextthink = level.time + 3000;

    // 3sec default priming
    if ( self->client->ns.weaponmode[self->client->ps.weapon] & ( 1 << WM_SINGLE) )
        bolt->nextthink = level.time + 2000; // 3sec default priming
    else if ( self->client->ns.weaponmode[self->client->ps.weapon] & ( 1 << WM_WEAPONMODE2 ) )
        bolt->nextthink = level.time + 1000;

    bolt->think = G_ExplodeMissile;
    bolt->s.eType = ET_MISSILE;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    bolt->s.weapon = WP_SMOKE ;
    bolt->s.eFlags = EF_BOUNCE_HALF;
    bolt->r.ownerNum = self->s.number;
    bolt->timestamp = level.time;
    bolt->parent = self;
    bolt->methodOfDeath = MOD_GRENADE;
    bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
    bolt->clipmask = MASK_SHOT;
    bolt->count = SEALS_SMOKEPUFF_NUMBER;
    bolt->target_ent = NULL;

    // shootable smokegrenade
    bolt->health = 4;
    bolt->takedamage = qtrue;
    bolt->r.contents = CONTENTS_BODY;
    bolt->die = G_SmokeNadeDie;
    VectorSet(bolt->r.mins, -2, -2, 0);
    VectorCopy(bolt->r.mins, bolt->r.absmin);
    VectorSet(bolt->r.maxs, 2, 2, 2);
    VectorCopy(bolt->r.maxs, bolt->r.absmax);
    // /shootable smokegrenade

    if ( self->client && self->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADEROLL ) )
    {
        start[2] -= 15;
        bolt->fly_sound_debounce_time = qtrue;//rolling
        VectorScale( dir, SEALS_BASEGRENRANGE_ROLL +
                     ((float)(self->client->pers.nsPC.strength)/(10.0f) *
                      SEALS_BASEGRENRANGE_ROLL *
                      SEALS_BASEGRENRANGE_ADDSTRENGTH ) +
                     firestrength , bolt->s.pos.trDelta );
    } else {
        VectorScale( dir, SEALS_BASEGRENRANGE_THROW +
                     ((float)(self->client->pers.nsPC.strength)/(10.0f) *
                      SEALS_BASEGRENRANGE_THROW *
                      SEALS_BASEGRENRANGE_ADDSTRENGTH ) +
                     firestrength , bolt->s.pos.trDelta );
    }

    bolt->s.pos.trType = TR_GRAVITY;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
    VectorCopy( start, bolt->s.pos.trBase );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

    VectorCopy (start, bolt->r.currentOrigin);

    return bolt;
}
/*
=================
fire_flashbang
=================
*/
gentity_t *fire_flashbang (gentity_t *self, vec3_t start, vec3_t dir, int firestrength) {
    gentity_t	*bolt;

    if ( firestrength < 0 )
        firestrength *= -1 ;

    //	G_Printf("firestrength: %i\n", firestrength );

    VectorNormalize (dir);

    bolt = G_Spawn();
    bolt->classname = "flashbang";

    // timer settings
    bolt->nextthink = level.time + 3000;

    // 3sec default priming
    if ( self->client->ns.weaponmode[self->client->ps.weapon] & ( 1 << WM_SINGLE) )
        bolt->nextthink = level.time + 2000; // 3sec default priming
    else if ( self->client->ns.weaponmode[self->client->ps.weapon] & ( 1 << WM_WEAPONMODE2 ) )
        bolt->nextthink = level.time + 1000;

    bolt->think = G_ExplodeMissile;
    bolt->s.eType = ET_MISSILE;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    bolt->s.weapon = WP_FLASHBANG;
    bolt->s.eFlags = EF_BOUNCE_HALF;
    bolt->timestamp = level.time;
    bolt->r.ownerNum = self->s.number;
    bolt->parent = self;
    bolt->damage = 5;
    bolt->splashDamage = 25;
    bolt->splashRadius = SEALS_FLASHBANGRADIUS;
    bolt->methodOfDeath = MOD_GRENADE;
    bolt->splashMethodOfDeath = MOD_GRENADE_SPLASH;
    bolt->clipmask = MASK_SHOT;
    bolt->target_ent = NULL;

    // shootable flashbang
    bolt->health = 4;
    bolt->takedamage = qtrue;
    bolt->r.contents = CONTENTS_BODY;
    bolt->die = G_FlashBangDie;
    VectorSet(bolt->r.mins, -2, -2, 0);
    VectorCopy(bolt->r.mins, bolt->r.absmin);
    VectorSet(bolt->r.maxs, 2, 2, 2);
    VectorCopy(bolt->r.maxs, bolt->r.absmax);
    // /shootable flashbang

    if ( self->client && self->client->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADEROLL ) ) 	{
        start[2] -= 15;
        bolt->fly_sound_debounce_time = qtrue;//rolling
        VectorScale( dir, SEALS_BASEGRENRANGE_ROLL +
                     ((float)(self->client->pers.nsPC.strength)/(10.0f) *
                      SEALS_BASEGRENRANGE_ROLL * SEALS_BASEGRENRANGE_ADDSTRENGTH ) +
                     firestrength, bolt->s.pos.trDelta );
    } else {
        VectorScale( dir, SEALS_BASEGRENRANGE_THROW +
                     ((float)(self->client->pers.nsPC.strength)/(10.0f) *
                      SEALS_BASEGRENRANGE_THROW *
                      SEALS_BASEGRENRANGE_ADDSTRENGTH ) +
                     firestrength , bolt->s.pos.trDelta );
    }

    bolt->s.pos.trType = TR_GRAVITY;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
    VectorCopy( start, bolt->s.pos.trBase );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth

    VectorCopy (start, bolt->r.currentOrigin);

    return bolt;
}

//=============================================================================

/*
=================
fire_rocket
=================
*/
gentity_t *fire_rocket (gentity_t *self, vec3_t start, vec3_t dir) {
    gentity_t	*bolt;

    VectorNormalize (dir);

    bolt = G_Spawn();
    bolt->classname = "rocket";
    bolt->nextthink = level.time + 15000;
    bolt->think = G_ExplodeMissile;
    bolt->s.eType = ET_MISSILE;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    //	bolt->s.weapon = WP_ROCKET_LAUNCHER;
    bolt->r.ownerNum = self->s.number;
    bolt->parent = self;
    bolt->damage = 100;
    bolt->splashDamage = 100;
    bolt->splashRadius = 120;
    bolt->methodOfDeath = MOD_ROCKET;
    bolt->splashMethodOfDeath = MOD_ROCKET_SPLASH;
    bolt->clipmask = MASK_SHOT;
    bolt->target_ent = NULL;

    bolt->s.pos.trType = TR_LINEAR;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
    VectorCopy( start, bolt->s.pos.trBase );
    VectorScale( dir, 900, bolt->s.pos.trDelta );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
    VectorCopy (start, bolt->r.currentOrigin);

    return bolt;
}

/*
==========
ball
==========
*/
gentity_t *fire_ball (gentity_t *self, vec3_t start, vec3_t dir) {
    gentity_t	*bolt;

    VectorNormalize (dir);

    bolt = G_Spawn();
    bolt->classname = "ball";
    bolt->s.eType = ET_MISSILE;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    bolt->s.weapon = WP_MP5;
    bolt->clipmask = MASK_ALL;
    bolt->target_ent = NULL;
    bolt->s.eFlags = EF_BOUNCE_HALF;

    bolt->r.mins[0] = -4;
    bolt->r.mins[1] = -4;
    bolt->r.mins[2] = -4;

    bolt->r.maxs[0] = 4;
    bolt->r.maxs[1] = 8;
    bolt->r.maxs[2] = 4;

    bolt->s.pos.trType = TR_GRAVITY;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame

    VectorCopy( start, bolt->s.pos.trBase );
    VectorScale( dir, 300, bolt->s.pos.trDelta );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
    VectorCopy (start, bolt->r.currentOrigin);

    // save start position
    VectorCopy( start, bolt->pos2 );

    return bolt;
}

/////////           //
//    //           //
//////  e a l      //    e a d
//  //            /////
gentity_t *fire_reallead (gentity_t *self, vec3_t start,vec3_t dir, int weapon, int caliber, int damage, float muzzlevelocity);
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout );


//#define DEBUG_LEAD

vec3_t lMins = { -0.5, -0.5, -0.5 };
vec3_t lMaxs = { 0.5, 0.5, 0.5 };
void Lead_Impact ( gentity_t *lead, trace_t *trace )
{
    gentity_t		*other, *tent,*tent2;
    qboolean		hitClient = qfalse;
    trace_t			tr;
    vec3_t			start;
    vec3_t			dir;
    qboolean		funcexplosive = qfalse;
    int				dmg = 10;
    int				weapon = lead->s.weapon;

    other = &g_entities[trace->entityNum];

    memset( &tent, 0, sizeof(tent ) );
    memset( &tent2, 0, sizeof(tent2 ) );

    //	G_Printf("weapon: %i ( %i %i )\n", weapon, WP_GRENADE, WP_FLASHBANG );


    if ( lead->fly_sound_debounce_time > 10 ) // if it's been here atleast 10 times
    {
#ifdef DEBUG_LEAD
        G_Printf("Destroying lead %i because maximum hits reached\n", lead->s.number );
#endif
        G_FreeEntity(lead);
        return;
    }
    lead->fly_sound_debounce_time++;

    if ( lead->r.ownerNum != other->s.number && lead->splashDamage == other->s.number )
    {
        G_FreeEntity( lead );
        return;
    }
    // if we've been moving silently
    if ( lead->r.svFlags & SVF_NOCLIENT ) // set us to visible so that the effects are visible
        lead->r.svFlags &= ~SVF_NOCLIENT;

    // stop here if we're a corspe, we can't recieve no damage
    if ( other->r.contents == CONTENTS_CORPSE && !(!Q_stricmp( other->classname, "player_bbox_head") ) )
    {
        tent = G_TempEntity( trace->endpos, EV_BULLET_HIT_FLESH );
        tent->s.eventParm = other->s.number;
        tent->s.torsoAnim = DirToByte( lead->movedir );
        tent->s.legsAnim = dmg;

        G_FreeEntity( lead );
        return;
    }

    // if we hitted a  we can get hitted by our own missile
    lead->r.ownerNum = ENTITYNUM_NONE;


    if ( other && !Q_stricmp(other->classname , "reallead") )
    {
#ifdef DEBUG_LEAD
        G_Printf("Destroying lead %i\n", lead->s.number );
#endif
        G_FreeEntity(lead);
        return;
    }
    if ( lead->timestamp < level.time && lead->s.powerups & ( 1 << 1 ) )
    {
#ifdef DEBUG_LEAD
        G_Printf("Removed trail for lead %i", lead->s.number );
#endif
        lead->s.powerups &=~(1<<1);
    }


    // impact damage
    if (other->takedamage /* && other->r.contents != CONTENTS_CORPSE*/ )
    {
        if ( lead->damage ) {

            if( LogAccuracyHit( other, &g_entities[lead->s.otherEntityNum2-1] ) ) {
                g_entities[lead->s.otherEntityNum2-1].client->accuracy_hits++;
                hitClient = qtrue;
            }

#ifdef DEBUG_LEAD
            G_Printf("Lead %i inflicted %i damage on %i\n", lead->s.number, lead->damage, other->s.number );
#endif
            dmg = G_Damage (other, lead, &g_entities[lead->s.otherEntityNum2-1], lead->movedir,
                            trace->endpos, lead->damage,	0, lead->methodOfDeath);
        }

        if ( is_func_explosive( other ) )
        {
            tent = G_TempEntity( trace->endpos, EV_BULLET_HIT_WALL );
            tent->s.eventParm = DirToByte( trace->plane.normal );
            tent->s.weapon = weapon;
#ifdef DEBUG_LEAD
            G_Printf("Lead %i hitted func_explosive %i\n", lead->s.number, trace->entityNum );
#endif
            if ( !Q_stricmp( other->classname,"func_explosive_glass" ) && other->ns_flags > 0 )
                tent->s.generic1 = BHOLE_GLASS;
            else
                tent->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );

            if ( other->ns_flags > 0 && !other->fly_sound_debounce_time )
                tent->s.otherEntityNum2 = trace->entityNum;
            else
                tent->s.frame = 3; // leave no mark... if we're dead

            if ( !Q_stricmp( other->classname,"func_explosive_glass" ) && other->ns_flags > 0 )
            {
                int lastglass = 0;
                int	checks = 10; // run atleast 10 checks

                vec3_t temp;

                VectorCopy(trace->endpos , temp );
                VectorMA (temp, 300, lead->movedir, temp);   //a bullet will go through 3000qu glass

                if ( trap_PointContents( temp, lead->s.number ) & CONTENTS_SOLID )
                {
                    // don't go further we're stuck in a wall
                    G_FreeEntity( lead );
                    return;
                }
                else
                {
                    lead->wait++;

                    while ( checks )
                    {
                        // get exit position...
                        trap_Trace (&tr, temp, NULL, NULL, trace->endpos, lastglass, MASK_SOLID );

                        // get the point where the bullet leaves the wall
                        if ( tr.entityNum == trace->entityNum )
                            break;

                        lastglass = tr.entityNum;
                        VectorCopy(tr.endpos, temp);
                        checks--;
                    }
                    tent2 = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
                    tent2->s.eventParm = DirToByte( tr.plane.normal );
                    tent2->s.generic1 = BHOLE_GLASS;
                    tent2->s.weapon = weapon;

                    if ( other->ns_flags > 0 && !other->fly_sound_debounce_time )
                        tent2->s.otherEntityNum2 = tr.entityNum;
                    else
                        tent2->s.frame = 3; // leave no mark... if we're dead
                }
            }

            if ( tent2 )
                SnapVector( tent2->s.origin );

            // if we're on a funcexplosive we don't have to leave marks.
            funcexplosive = qtrue;
        }
        else {
            int HitLocation = LOC_NULL;

            if ( other->client )
                HitLocation = NS_CheckLocationDamage( other, trace->endpos, WP_MK23 );

#ifdef DEBUG_LEAD
            G_Printf("Lead %i hitted player %i on %i\n", lead->s.number, other->s.number, HitLocation );
#endif
            // covered by a vest?
            if ( ( HitLocation == LOC_BACK || HitLocation == LOC_STOMACH || HitLocation == LOC_CHEST ) && other->client && other->client->ps.powerups[PW_VEST] && !BG_IsRifle( weapon ) )
            {
                tent = G_TempEntity( trace->endpos, EV_BULLET_HIT_WALL );
                tent->s.eventParm = DirToByte( trace->plane.normal );
                tent->s.otherEntityNum = lead->s.otherEntityNum2-1;
                tent->s.weapon = weapon;
                tent->s.frame = 1; // kevlar vest
            }		       // or even a helmet , call sparks then
            else if ( ( HitLocation == LOC_HEAD || HitLocation == LOC_FACE ) && other->client && other->client->ps.powerups[PW_HELMET] )
            {
                tent = G_TempEntity( trace->endpos, EV_BULLET_HIT_WALL );
                tent->s.eventParm = DirToByte( trace->plane.normal );
                tent->s.otherEntityNum = lead->s.otherEntityNum2-1;
                tent->s.weapon = weapon;
                tent->s.frame = 2; // helmet hit
            }		       // call blood
            else if ( other->takedamage && other->client )
            {
                tent = G_TempEntity( trace->endpos, EV_BULLET_HIT_FLESH );
                tent->s.eventParm = other->s.number;
                tent->s.torsoAnim = DirToByte( lead->movedir );
                tent->s.legsAnim = dmg;

                // only if we're NOT going through a head
                if ( ( HitLocation != LOC_HEAD && HitLocation != LOC_FACE ) )
                {
                    lead->s.powerups |= ( 1 << 1 );
                    lead->timestamp = level.time + 100;
                }
            }
            else // play default concrete...
            {
                tent = G_TempEntity( trace->endpos, EV_BULLET_HIT_WALL );
                tent->s.eventParm = DirToByte( trace->plane.normal );
                tent->s.otherEntityNum = lead->s.otherEntityNum2-1;
                tent->s.weapon = weapon;
                tent->s.frame = 0;
            }

            lead->wait--;
            // ignore htis entity so we can't hit someone twice with one bullet.
            lead->r.ownerNum = trace->entityNum;

            if (tent)
                SnapVector( tent->s.origin );

            // 9mm & pistols don't go through ppls
            if ( lead->count == AM_LIGHT_PISTOL || lead->count == AM_MEDIUM_PISTOL || lead->count == AM_SMG )
            {
                G_FreeEntity( lead );
#ifdef DEBUG_LEAD
                G_Printf("Lead %i free after killing player.\n", lead->s.number );
#endif
            }
            return;
        }
    }

    if ( other &&
            ( !Q_stricmp( other->classname, "func_door" ) || !Q_stricmp( other->classname, "func_door_rotate" ) )  &&
            ( BG_IsShotgun( lead->s.weapon ) || BG_IsPistol( lead->s.weapon ) )
       )
    {
#ifdef DEBUG_LEAD
        G_Printf("Lead %i changed to FX\n", lead->s.number );
#endif
        G_AddEvent( lead, EV_BULLET_HIT_WALL, DirToByte( trace->plane.normal ) );
        lead->freeAfterEvent = qtrue;
        lead->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );
        VectorCopy( trace->endpos , lead->s.pos.trBase );

        // change over to a normal entity right at the point of impact
        lead->s.eType = ET_GENERAL;
        lead->s.weapon = weapon;
        lead->s.otherEntityNum2 = 0;
        lead->s.frame = 0;

        SnapVector( lead->s.origin );
        // splash damage (doesn't apply to person directly hit)
        trap_LinkEntity( lead );
        return;
    }

    // only 9mm bounces off the wall, and not every projectile
    if ( trace->surfaceFlags & SURF_METALSTEPS &&
            ( lead->count == AM_SMG ||
              lead->count == AM_LIGHT_PISTOL ||
              lead->count == AM_MEDIUM_PISTOL  )
            && !BG_IsShotgun( weapon )
       )
    {
        vec3_t temp;
        vec3_t end;

        qboolean bounce = ( random() < 0.75 );

        if ( bounce  )
        {
            gentity_t *new_lead;

            VectorCopy( trace->endpos, temp);
            G_BounceProjectile( lead->s.pos.trBase, temp,  trace->plane.normal, end );
            VectorCopy( temp, start );
            VectorSubtract( end, temp, dir );
            VectorNormalize(dir);

            new_lead = fire_reallead( &g_entities[ lead->s.otherEntityNum2-1 ], start, dir, weapon, lead->count, lead->damage - 1, lead->moverState );

            // move through original owner
            new_lead->r.ownerNum = ENTITYNUM_NONE;
            // inflict damage on parent
            new_lead->splashDamage = ENTITYNUM_NONE;

#ifdef DEBUG_LEAD
            G_Printf("Lead %i new bouncer spawned %i.\n", lead->s.number, new_lead->s.number );
#endif
            trap_LinkEntity( new_lead );
        }

        // transform current entity into hit wall and spawn a new bullet
        if ( funcexplosive )
        {
#ifdef DEBUG_LEAD
            G_Printf("Lead %i freed after bounce on func explosive.\n", lead->s.number );
#endif
            G_FreeEntity( lead );
        }
        else {
#ifdef DEBUG_LEAD
            G_Printf("Lead %i changed to FX\n", lead->s.number );
#endif
            G_AddEvent( lead, EV_BULLET_HIT_WALL, DirToByte( trace->plane.normal ) );
            lead->s.otherEntityNum = lead->s.otherEntityNum2-1;
            lead->freeAfterEvent = qtrue;
            lead->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );
            VectorCopy( trace->endpos , lead->s.pos.trBase );

            // change over to a normal entity right at the point of impact
            lead->s.eType = ET_GENERAL;
            lead->s.weapon = weapon;
            lead->s.otherEntityNum2 = 0;
            lead->s.frame = 0;

            if ( is_func_explosive( other )  && !other->fly_sound_debounce_time )
            {
                lead->s.otherEntityNum2 = trace->entityNum;
            }

            SnapVector( lead->s.origin );
            // splash damage (doesn't apply to person directly hit)
            trap_LinkEntity( lead );
        }
        // after rebounced... break, this won't go through metal then...
        return;
    }

    // check if the bullet could crash through the wall
    {
        float breakValue = BG_LeadGetBreakValueForSurface( trace );// ;

        switch (weapon)
        {
        case WP_MACMILLAN:
            breakValue *= 2.75; // uber tuff :>
        case WP_M14:
        case WP_PSG1:
#ifdef SL8SD
        case WP_SL8SD:
#endif
            breakValue *= 2; // really tuff
            break;
        default:
            break;
        }

        // get the point where the bullet leaves the wall
        VectorMA (trace->endpos, breakValue, lead->movedir, start);

        // stuck in a wall?
        if ( trap_PointContents( start, -1 ) & CONTENTS_SOLID || BG_IsShotgun(weapon) ||  lead->count == AM_SHOTGUNMAG )
        {
            if ( funcexplosive )
            {	// already handled this brush
#ifdef DEBUG_LEAD
                G_Printf("Lead %i freed because mark already spawned on func_explosive \n", lead->s.number );
#endif
                G_FreeEntity( lead );
                return;
            }
            else if ( is_func_explosive( other )  && !other->fly_sound_debounce_time )
            {
#ifdef DEBUG_LEAD
                G_Printf("Lead %i spawned mark on func_explosive\n", lead->s.number );
#endif
                G_AddEvent( lead, EV_BULLET_HIT_WALL, DirToByte( trace->plane.normal ) );
                lead->s.otherEntityNum = lead->s.otherEntityNum2-1;
                lead->freeAfterEvent = qtrue;
                lead->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );
                lead->s.otherEntityNum2 = trace->entityNum;
                lead->s.frame = 0;
                lead->s.weapon = weapon;
            }
            else
            {
#ifdef DEBUG_LEAD
                G_Printf("Lead %i spawned mark\n", lead->s.number );
#endif
                G_AddEvent( lead, EV_BULLET_HIT_WALL, DirToByte( trace->plane.normal ) );
                lead->s.otherEntityNum = lead->s.otherEntityNum2-1;
                lead->freeAfterEvent = qtrue;
                lead->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );
                lead->s.frame = 0;
                lead->s.otherEntityNum2 = 0;
                lead->s.weapon = weapon;
            }

            lead->s.modelindex = 0;

            // change over to a normal entity right at the point of impact
            lead->s.eType = ET_GENERAL;

            G_SetOrigin( lead, trace->endpos );

            SnapVector( lead->s.origin );
            SnapVector( lead->r.currentOrigin );

            // splash damage (doesn't apply to person directly hit)
            trap_LinkEntity( lead );
            return;
        }

        // some materials are also breakable for smaller calibers
        if (  trace->surfaceFlags & SURF_WOODSTEPS
                || trace->surfaceFlags & SURF_DIRTSTEPS
                || trace->surfaceFlags & SURF_SNOWSTEPS
                || trace->surfaceFlags & SURF_SANDSTEPS
                || trace->surfaceFlags & SURF_GLASS
                || trace->surfaceFlags & SURF_SOFTSTEPS )
        {
#ifdef DEBUG_LEAD
            G_Printf("Lead %i broke %i times through soft material\n", lead->s.number, lead->wait+1 );
#endif
            lead->wait++;
        }


        // get exit position...
        trap_Trace( &tr, start, lMins, lMaxs, trace->endpos, lead->s.number, MASK_SOLID );

        // we actually found a plane that might be a window
        if(tr.entityNum < ENTITYNUM_MAX_NORMAL && tr.entityNum != trace->entityNum ) // and our entrace may not be a func_explosive
        {
            // do a trace back again, but this time ignore the entity.
            trap_Trace ( &tr, start, lMins, lMaxs, trace->endpos, tr.entityNum, MASK_SOLID );

            // get the point where the bullet leaves the wall
            VectorCopy (tr.endpos, start);
        }
        else
            VectorCopy (tr.endpos, start);

        if ( VectorCompare( start, lead->s.pos.trBase ) )
        {
#ifdef DEBUG_LEAD
            G_Printf("Lead %i couldn't break through surface\n", lead->s.number );
#endif
            tent2 = G_TempEntity( trace->endpos, EV_BULLET_HIT_WALL );
            tent2->s.eventParm = DirToByte( trace->plane.normal );
            tent2->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );
            tent2->freeAfterEvent = qtrue;

            G_FreeEntity( lead );
            return;
        }

        VectorCopy ( start, lead->s.pos.trBase );
        VectorCopy ( start, lead->r.currentOrigin );


        if ( !funcexplosive )
        {
#ifdef DEBUG_LEAD
            G_Printf("Lead %i spawned FX \n", lead->s.number );
#endif

            tent2 = G_TempEntity( trace->endpos, EV_BULLET_HIT_WALL );
            tent2->s.eventParm = DirToByte( trace->plane.normal );
            tent2->s.generic1 = NS_BulletHoleTypeForSurface( trace->surfaceFlags );
            tent2->freeAfterEvent = qtrue;
            tent2->s.weapon = weapon;

            if ( lead->wait > 1 ) {
                tent = G_TempEntity( tr.endpos, EV_BULLET_HIT_WALL );
                tent->s.eventParm = DirToByte( tr.plane.normal );
                tent->s.weapon = weapon;
                tent->s.generic1 = NS_BulletHoleTypeForSurface( tr.surfaceFlags );
                tent->freeAfterEvent = qtrue;
            }

            // we didn't hit a funcexplosive that can take damage. but it might be
            // a func_explosive
            if ( is_func_explosive( &g_entities[trace->entityNum]) && !g_entities[trace->entityNum].fly_sound_debounce_time )
            {
                if ( tent )
                    tent->s.otherEntityNum2 = trace->entityNum;
                if ( tent2 )
                    tent2->s.otherEntityNum2 = trace->entityNum;
            }
        }

        //  save net bandwidth
        if ( tent )
            SnapVector( tent->s.origin );
        if ( tent2 )
            SnapVector( tent2->s.origin );

        lead->wait--;

        // how often the bullet may crash through a wall...
        if ( lead->wait <= 0 )
        {
#ifdef DEBUG_LEAD
            G_Printf("Lead %i freed because too often hitted a wall\n", lead->s.number );
#endif
            G_FreeEntity( lead );
            return;
        }
    }
    trap_LinkEntity( lead );
}

/*
=================
fire_bullet
=================
*/
gentity_t *fire_reallead (gentity_t *self, vec3_t start,vec3_t dir, int weapon, int caliber, int damage, float muzzlevelocity) {
    gentity_t	*bolt;
    vec3_t		end;
    trace_t		tr;
    float		dist;

    VectorMA( start, 8920, dir, end );
    trap_Trace( &tr, start, NULL,NULL,end, self->s.number, MASK_SOLID );

    dist = Distance( start, tr.endpos );

    bolt = G_Spawn();
#ifdef DEBUG_LEAD
    G_Printf("firing bullet: %i %i %i %f %i %f range: %f\n", damage, weapon,caliber,muzzlevelocity, bolt->s.otherEntityNum , tr.fraction,  Distance(start, tr.endpos ) );
#endif

    VectorNormalize (dir);
    bolt->classname = "reallead";
    bolt->nextthink = level.time + 4500; // free after 41/2 seconds so camera won't chase it into nirvana
    bolt->think = G_FreeEntity;
    bolt->s.eType = ET_BULLET;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
    bolt->s.weapon = weapon;
    bolt->r.ownerNum = self->s.number;
    bolt->parent = self;
    bolt->damage = damage;
    bolt->methodOfDeath = MOD_LEAD;
    bolt->clipmask = MASK_SHOT;
    bolt->target_ent = NULL;
    bolt->count = caliber;

    bolt->fly_sound_debounce_time = 0;
    if ( BG_IsRifle( weapon ) )
        bolt->wait = 4;
    else
        bolt->wait = 1;

    if ( g_silentBullets.integer )
        bolt->r.svFlags |= SVF_NOCLIENT;

    bolt->timestamp = level.time + 150;
    bolt->splashDamage = self->s.number; // to prevent commited suicide bug
    bolt->s.otherEntityNum2 = self->s.number+1; // the person who fired it. +1 so we don't get to 0
    bolt->s.modelindex = G_ModelIndex( "models/misc/bullets/bullet_pistol.md3" );
    bolt->moverState = muzzlevelocity;
    bolt->s.pos.trType = TR_LINEAR;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
    VectorCopy( start, bolt->s.pos.trBase );
    VectorScale( dir, muzzlevelocity, bolt->s.pos.trDelta );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
    VectorCopy (start, bolt->r.currentOrigin);

    VectorCopy ( dir,  bolt->movedir );

    // if zooming and a great distance we CAN go into camera mode

    /*

    if ( dist >= 1000 && BG_IsZooming( self->client->ns.weaponmode[self->s.weapon] ) && self->client->ps.weapon == WP_MACMILLAN )
    bolt->s.otherEntityNum = 1; // follow this bullet
    else
    bolt->s.otherEntityNum = 0;

    */

    trap_LinkEntity( bolt );

    VectorAdd( bolt->s.pos.trDelta, self->client->ps.velocity, bolt->s.pos.trDelta );	// "real" physics

    return bolt;
}


/*
=================
fire_realshotgun
=================
*/
gentity_t *fire_realshotgun (gentity_t *self, vec3_t start,vec3_t dir, int weapon, int caliber, int damage, float muzzlevelocity) {
    gentity_t	*bolt;
    vec3_t		end;
    trace_t		tr;
    float		dist;

    VectorMA( start, 8920, dir, end );
    trap_Trace( &tr, start, NULL,NULL,end, self->s.number, MASK_SOLID );

    dist = Distance( start, tr.endpos );

    bolt = G_Spawn();

    //	G_Printf("firing bullet: %i %i %i %f %i %f range: %f\n", damage, weapon,caliber,muzzlevelocity, bolt->s.otherEntityNum , tr.fraction,  Distance(start, tr.endpos ) );

    VectorNormalize (dir);
    bolt->classname = "reallead";
    bolt->nextthink = level.time + 2500; // free after 21/2 seconds so camera won't chase it into nirvana
    bolt->think = G_FreeEntity;
    bolt->s.eType = ET_BULLET;
    bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;

    //	if ( g_silentBullets.integer )
    bolt->r.svFlags |= SVF_NOCLIENT;

    bolt->s.weapon = weapon;
    bolt->r.ownerNum = self->s.number;
    bolt->parent = self;
    bolt->damage = damage;
    bolt->methodOfDeath = MOD_LEAD;
    bolt->clipmask = MASK_SHOT;
    bolt->target_ent = NULL;
    bolt->count = caliber;

    bolt->wait = 0;

    bolt->s.frame = damage;
    bolt->timestamp = level.time + 150;
    bolt->s.otherEntityNum2 = self->s.number+1; // the person who fired it. +1 so we don't get to 0
    bolt->s.modelindex = G_ModelIndex( "models/misc/bullets/bullet_pistol.md3" );
    bolt->moverState = muzzlevelocity;
    bolt->s.pos.trType = TR_GRAVITY;
    bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
    VectorCopy( start, bolt->s.pos.trBase );
    VectorScale( dir, muzzlevelocity, bolt->s.pos.trDelta );
    SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
    VectorCopy (start, bolt->r.currentOrigin);

    VectorCopy ( dir,  bolt->movedir );

    // if zooming and a great distance we CAN go into camera mode
    if ( dist >= 1000 && BG_IsZooming( self->client->ns.weaponmode[self->s.weapon] ) && self->client->ps.weapon == WP_MACMILLAN )
        bolt->s.otherEntityNum = 1; // follow this bullet
    else
        bolt->s.otherEntityNum = 0;

    trap_LinkEntity( bolt );

    VectorAdd( bolt->s.pos.trDelta, self->client->ps.velocity, bolt->s.pos.trDelta );	// "real" physics

    return bolt;
}
