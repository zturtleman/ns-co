// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_pmove.c -- both games player movement code

// takes a playerstate and a usercmd as input and returns a modifed playerstate
// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t		*pm;
pml_t		pml;

// movement parameters
float	pm_stopspeed = 100.0f;
float	pm_scopeScale = 0.45f;
float	pm_duckScale = 0.40f;
float	pm_swimScale = 0.60f;
float	pm_wadeScale = 0.70f;

float	pm_sprintScale = 1.75f;

float	pm_accelerate = 10.0f;
float	pm_walkaccelerate = 25.0f;

float	pm_airaccelerate = 1.0f;
float	pm_wateraccelerate = 4.0f;
float	pm_flyaccelerate = 8.0f;

float	pm_friction = 6.0f;
float	pm_waterfriction = 1.0f;
float	pm_flightfriction = 3.0f;
float	pm_spectatorfriction = 1.0f;

int		c_pmove = 0;

// Navy Seals ++
float	pm_ladderScale = 0.50;
float	pm_ladderaccelerate = 3000;
float	pm_ladderfriction = 3000;

int		NS_OnLadder();

vec3_t pm_previous_origin[MAX_CLIENTS];
int    pm_lastsprint[MAX_CLIENTS];

// Navy Seals --

int BG_GetMaxRoundForWeapon( int weapon );

/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent( int newEvent ) {
    BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
    int		i;

    if ( entityNum == ENTITYNUM_WORLD ) {
        return;
    }
    if ( pm->numtouch == MAXTOUCH ) {
        return;
    }

    // see if it is already added
    for ( i = 0 ; i < pm->numtouch ; i++ ) {
        if ( pm->touchents[ i ] == entityNum ) {
            return;
        }
    }

    // add it
    pm->touchents[pm->numtouch] = entityNum;
    pm->numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim( int anim ) {
    if ( pm->ps->pm_type >= PM_DEAD ) {
        return;
    }
    pm->ps->torsoAnim = ( ( pm->ps->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
                        | anim;
}
static void PM_StartLegsAnim( int anim ) {
    if ( pm->ps->pm_type >= PM_DEAD ) {
        return;
    }
    if ( pm->ps->legsTimer > 0 ) {
        return;		// a high priority animation is running
    }
    pm->ps->legsAnim = ( ( pm->ps->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT )
                       | anim;
}

static void PM_ContinueLegsAnim( int anim ) {
    if ( ( pm->ps->legsAnim & ~ANIM_TOGGLEBIT ) == anim ) {
        return;
    }
    if ( pm->ps->legsTimer > 0 ) {
        return;		// a high priority animation is running
    }
    PM_StartLegsAnim( anim );
}

static void PM_ContinueTorsoAnim( int anim ) {
    if ( ( pm->ps->torsoAnim & ~ANIM_TOGGLEBIT ) == anim ) {
        return;
    }
    if ( pm->ps->torsoTimer > 0 ) {
        return;		// a high priority animation is running
    }
    PM_StartTorsoAnim( anim );
}

static void PM_ForceLegsAnim( int anim ) {
    pm->ps->legsTimer = 0;
    PM_StartLegsAnim( anim );
}


/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
    float	backoff;
    float	change;
    int		i;

    backoff = DotProduct (in, normal);

    if ( backoff < 0 ) {
        backoff *= overbounce;
    } else {
        backoff /= overbounce;
    }

    for ( i=0 ; i<3 ; i++ ) {
        change = normal[i]*backoff;
        out[i] = in[i] - change;
    }
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
    vec3_t	vec;
    float	*vel;
    float	speed, newspeed, control;
    float	drop;

    vel = pm->ps->velocity;

    VectorCopy( vel, vec );
    if ( pml.walking ) {
        vec[2] = 0;	// ignore slope movement
    }

    speed = VectorLength(vec);
    if (speed < 1) {
        vel[0] = 0;
        vel[1] = 0;		// allow sinking underwater
        // FIXME: still have z friction underwater?
        return;
    }

    drop = 0;

    // apply ground friction
    if ( pm->waterlevel <= 1 ) {
        if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
            // if getting knocked back, no friction
            if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
                control = speed < pm_stopspeed ? pm_stopspeed : speed;
                drop += control*pm_friction*pml.frametime;
            }
        }
    }

    // apply water friction even if just wading
    if ( pm->waterlevel ) {
        drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
    }

    // apply flying friction
    if ( pm->ps->pm_type == PM_SPECTATOR  ) {
        drop += speed*pm_spectatorfriction*pml.frametime;
    }
    // Navy Seals ++
    if ( NS_OnLadder() != 0 ) // ladder?
    {
        drop += speed*pm_ladderfriction*pml.frametime;
    }
    // Navy Seals --
    // scale the velocity
    newspeed = speed - drop;
    if (newspeed < 0) {
        newspeed = 0;
    }
    newspeed /= speed;

    vel[0] = vel[0] * newspeed;
    vel[1] = vel[1] * newspeed;
    vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
#if 0
    // q2 style
    int			i;
    float		addspeed, accelspeed, currentspeed;

    currentspeed = DotProduct (pm->ps->velocity, wishdir);
    addspeed = wishspeed - currentspeed;
    if (addspeed <= 0) {
        return;
    }
    accelspeed = accel*pml.frametime*wishspeed;
    if (accelspeed > addspeed) {
        accelspeed = addspeed;
    }

    for (i=0 ; i<3 ; i++) {
        pm->ps->velocity[i] += accelspeed*wishdir[i];
    }
#else
    // proper way (avoids strafe jump maxspeed bug), but feels bad
    vec3_t		wishVelocity;
    vec3_t		pushDir;
    float		pushLen;
    float		canPush;

    VectorScale( wishdir, wishspeed, wishVelocity );
    VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
    pushLen = VectorNormalize( pushDir );

    canPush = accel*pml.frametime*wishspeed;
    if (canPush > pushLen) {
        canPush = pushLen;
    }

    VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
#endif
}



/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
    int		max;
    float	total;
    float	scale;

    max = abs( cmd->forwardmove );
    if ( abs( cmd->rightmove ) > max ) {
        max = abs( cmd->rightmove );
    }
    if ( abs( cmd->upmove ) > max ) {
        max = abs( cmd->upmove );
    }
    if ( !max ) {
        return 0;
    }

    total = sqrt( cmd->forwardmove * cmd->forwardmove
                  + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
    scale = (float)pm->ps->speed * max / ( 127.0 * total );

    return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir( void ) {
    if ( pm->cmd.forwardmove || pm->cmd.rightmove ) {
        if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0 ) {
            pm->ps->movementDir = 0;
        } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0 ) {
            pm->ps->movementDir = 1;
        } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0 ) {
            pm->ps->movementDir = 2;
        } else if ( pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0 ) {
            pm->ps->movementDir = 3;
        } else if ( pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0 ) {
            pm->ps->movementDir = 4;
        } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0 ) {
            pm->ps->movementDir = 5;
        } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0 ) {
            pm->ps->movementDir = 6;
        } else if ( pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0 ) {
            pm->ps->movementDir = 7;
        }
    } else {
        // if they aren't actively going directly sideways,
        // change the animation to the diagonal so they
        // don't stop too crooked
        if ( pm->ps->movementDir == 2 ) {
            pm->ps->movementDir = 1;
        } else if ( pm->ps->movementDir == 6 ) {
            pm->ps->movementDir = 7;
        }
    }
}


/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump( void ) {
    if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
        return qfalse;		// don't allow jump until all buttons are up
    }
    if ( pm->cmd.upmove < 10 ) {
        // not holding jump
        return qfalse;
    }

    if ( pm->ps->pm_flags & PMF_TIME_LAND &&
            pm->ps->pm_time > 0 )
        return qfalse;

    // Navy Seals ++
    // don't allow jump if we got major leg damage
    if ( pm->ps->stats[STAT_LEG_DAMAGE] > 40 )
        return qfalse;
    // need some stamina.. .against bunnyhopping
    if (pm->ps->stats[STAT_STAMINA] < 25 )
        return qfalse;

    // Navy Seals --
    // must wait for jump to be released
    if ( pm->ps->pm_flags & PMF_JUMP_HELD ) {
        // clear upmove so cmdscale doesn't lower running speed
        pm->cmd.upmove = 0;
        return qfalse;
    }

    pml.groundPlane = qfalse;		// jumping away
    pml.walking = qfalse;
    pm->ps->pm_flags |= PMF_JUMP_HELD;

    pm->ps->groundEntityNum = ENTITYNUM_NONE;

    // Navy Seals ++
    if ( pm->ps->stats[STAT_LEG_DAMAGE] > 5) // leg damage lowers jumping ability
        pm->ps->velocity[2] = JUMP_VELOCITY - ( pm->ps->stats[STAT_LEG_DAMAGE] * 2 );
    else
        pm->ps->velocity[2] = JUMP_VELOCITY;
    // Navy Seals --

    // Navy Seals ++

    // drain stamina when jumping
    if (pm->ps->stats[STAT_STAMINA] > 0 )
        pm->ps->stats[STAT_STAMINA] -= 25;

    if (pm->ps->stats[STAT_STAMINA] < 0 )
        pm->ps->stats[STAT_STAMINA] = 0;

    // Navy Seals --

    // Navy Seals ++ : no jump sound
    // PM_AddEvent( EV_JUMP );

    /*	if ( pm->cmd.forwardmove >= 0 ) {*/
    PM_ForceLegsAnim( LEGS_JUMP );
    pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
    /*	}/* else {
    PM_ForceLegsAnim( LEGS_JUMPB );
    pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
    }*/

    return qtrue;
}

/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump( void ) {
    vec3_t	spot;
    int		cont;
    vec3_t	flatforward;

    if (pm->ps->pm_time) {
        return qfalse;
    }

    // check for water jump
    if ( pm->waterlevel != 2 ) {
        return qfalse;
    }

    flatforward[0] = pml.forward[0];
    flatforward[1] = pml.forward[1];
    flatforward[2] = 0;
    VectorNormalize (flatforward);

    VectorMA (pm->ps->origin, 30, flatforward, spot);
    spot[2] += 4;
    cont = pm->pointcontents (spot, pm->ps->clientNum );
    if ( !(cont & CONTENTS_SOLID) ) {
        return qfalse;
    }

    spot[2] += 12;
    cont = pm->pointcontents (spot, pm->ps->clientNum );
    if ( cont ) {
        return qfalse;
    }

    // jump out of water
    VectorScale (pml.forward, 200, pm->ps->velocity);
    pm->ps->velocity[2] = 450;

    pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
    pm->ps->pm_time = 2000;

    return qtrue;
}

//============================================================================

// Navy Seals ++

/*
=============
NS_OnLadder (...)
date = 8-2-99
author = dX

return value:		-2 = trying to climb up/down ladder (finish/start)
-1 = climbing
0 = no
=============
*/
int NS_OnLadder( void )
{
    vec3_t forward,spot;
    vec3_t spot2;
    trace_t trace;
    vec3_t	traceMins;
    vec3_t	traceMaxs;

    if ( pm->cmd.upmove > 0 )
        return 0;
    if ( pm->ps->weaponTime > 0 )
        return 0;
    traceMins[0] = -16;
    traceMins[1] = -16;
    traceMins[2] = -16;

    traceMaxs[0] = 16;
    traceMaxs[1] = 16;
    traceMaxs[2] = 16;

    // check for ladder
    forward[0] = pml.forward[0];
    forward[1] = pml.forward[1];
    forward[2] = 0;

    VectorNormalize (forward);
    // trace 1 unit
    VectorMA (pm->ps->origin, 2 ,forward, spot);

    pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, spot,
               pm->ps->clientNum, MASK_PLAYERSOLID);

    if ( pm->ps->pm_type == PM_DEAD )
        return 0;

    if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
        return 0;

    if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER)) {
        // standing on ground(succefully climbed down) and moving back, no time to climb
        if ( pml.groundPlane && pm->cmd.forwardmove <= -15 )
            return 0;
        // else still on ladder
        return 1;
    }

    VectorClear( spot);

    VectorMA (pm->ps->origin, -5, forward, spot );

    spot[2] -= 25; // adjust height

    VectorMA( spot, 10, forward, spot2 );

    pm->trace ( &trace, spot,traceMins,traceMaxs, spot2, pm->ps->clientNum, MASK_PLAYERSOLID );

    if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER)  )
    {
        if ( pml.groundPlane )
            return -2; // standing on plane, allow forward/backward movement
        else
        {
            // no, only upwards/downwards climbing
            return -1;
        }
    }
    return 0;
}

/*
===================
PM_LadderMove(...)
date = 7-2-99
author = dX

basically the same as water move, some smaller changes here and there
needs some more improvements...

===================
*/
static void PM_LadderMove( void ) {
    int i;
    vec3_t wishvel;
    float wishspeed;
    vec3_t wishdir;
    float scale;
    float vel;

    PM_Friction ();

    // jump out of water
#if 0
    if ( pm->cmd.upmove > 0 )
    {
        VectorScale (pml.forward, -100, pm->ps->velocity);

        pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
        pm->ps->pm_time = 3000;

        PM_ForceLegsAnim( LEGS_JUMP );
        return;
    }
#endif

    // get user input
    scale = PM_CmdScale( &pm->cmd );

    VectorClear(wishvel);

    for (i=0 ; i<3 ; i++) {
        if ( NS_OnLadder() == -1 )
        {
            if ( i < 2 )
            {
                if ( pm->cmd.forwardmove > 0 )
                {
                    wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove +
                                 scale * pml.right[i]*pm->cmd.rightmove;
                }
                else
                    wishvel[i] = 0;
            }
        }
        // not midair, but still walking towards
        else if ( NS_OnLadder() == -2 )  {
            wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove +
                         scale * pml.right[i]*pm->cmd.rightmove;
        }
        else {
            wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove +
                         scale * pml.right[i]*pm->cmd.rightmove;
            /*
            if ( pm->cmd.forwardmove > 0 )
            wishvel[i] = scale * pml.right[i]*pm->cmd.rightmove + pml.forward[i]*pm->cmd.forwardmove;
            else if ( pm->cmd.forwardmove < 0 )
            wishvel[i] = scale * pml.right[i]*pm->cmd.rightmove + pml.forward[i]*(pm->cmd.forwardmove*-1);
            //	else*/
            //	wishvel[i] = scale * pml.right[i]*pm->cmd.rightmove;
        }

    }
    wishvel[2] = scale * pm->cmd.forwardmove + scale  ;

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    if ( wishspeed > pm->ps->speed * pm_ladderScale ) {
        wishspeed = pm->ps->speed * pm_ladderScale;
    }

    PM_Accelerate (wishdir, wishspeed, pm_ladderaccelerate);

    if ( pml.groundPlane && DotProduct( pm->ps->velocity,
                                        pml.groundTrace.plane.normal ) < 0 ) {
        vel = VectorLength(pm->ps->velocity);
        PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
                         pm->ps->velocity, OVERCLIP );

        VectorNormalize(pm->ps->velocity);
        VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
    }

    PM_SlideMove( qfalse );




}

// Navy Seals --

/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove( void ) {
    // waterjump has no control, but falls

    PM_StepSlideMove( qtrue );

    pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
    //	if (pm->ps->velocity[2] < 0) {
    // cancel as soon as we are falling down again
    //		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
    //		pm->ps->pm_time = 0;
    //	}
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove( void ) {
    int		i;
    vec3_t	wishvel;
    float	wishspeed;
    vec3_t	wishdir;
    float	scale;
    float	vel;

    if ( PM_CheckWaterJump() ) {
        PM_WaterJumpMove();
        return;
    }
#if 1
    // jump = head for surface
    if ( pm->cmd.upmove >= 10 ) {
        if (pm->ps->velocity[2] > -300) {
            if ( pm->watertype == CONTENTS_WATER ) {
                pm->ps->velocity[2] = 100;
            } else {
                pm->ps->velocity[2] = 50;
            }
        }
    }
#endif
    PM_Friction ();

    scale = PM_CmdScale( &pm->cmd );
    //
    // user intentions
    //
    if ( !scale ) {
        wishvel[0] = 0;
        wishvel[1] = 0;
        wishvel[2] = -60;		// sink towards bottom
    } else {
        for (i=0 ; i<3 ; i++)
            wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;

        wishvel[2] += scale * pm->cmd.upmove;
    }

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    if ( wishspeed > pm->ps->speed * pm_swimScale ) {
        wishspeed = pm->ps->speed * pm_swimScale;
    }

    PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

    // make sure we can go up slopes easily under water
    if ( pml.groundPlane && DotProduct( pm->ps->velocity, pml.groundTrace.plane.normal ) < 0 ) {
        vel = VectorLength(pm->ps->velocity);
        // slide along the ground plane
        PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
                         pm->ps->velocity, OVERCLIP );

        VectorNormalize(pm->ps->velocity);
        VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
    }

    PM_SlideMove( qfalse );
}

#if 0
/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
    int		i;
    vec3_t	wishvel;
    float	wishspeed;
    vec3_t	wishdir;
    float	scale;

    // normal slowdown
    PM_Friction ();

    scale = PM_CmdScale( &pm->cmd );
    //
    // user intentions
    //
    if ( !scale ) {
        wishvel[0] = 0;
        wishvel[1] = 0;
        wishvel[2] = 0;
    } else {
        for (i=0 ; i<3 ; i++) {
            wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
        }

        wishvel[2] += scale * pm->cmd.upmove;
    }

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

    PM_StepSlideMove( qfalse );
}
#endif

/*
===================
PM_AirMove

===================
*/
static void PM_AirMove( void ) {
    int			i;
    vec3_t		wishvel;
    float		fmove, smove;
    vec3_t		wishdir;
    float		wishspeed;
    float		scale;
    usercmd_t	cmd;

    PM_Friction();

    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.rightmove;

    cmd = pm->cmd;
    scale = PM_CmdScale( &cmd );

    // set the movementDir so clients can rotate the legs for strafing
    PM_SetMovementDir();

    // project moves down to flat plane
    pml.forward[2] = 0;
    pml.right[2] = 0;
    VectorNormalize (pml.forward);
    VectorNormalize (pml.right);

    for ( i = 0 ; i < 2 ; i++ ) {
        wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
    }
    wishvel[2] = 0;

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);
    wishspeed *= scale;

    // not on ground, so little effect on velocity
    PM_Accelerate (wishdir, wishspeed, pm_airaccelerate );

    // we may have a ground plane that is very steep, even
    // though we don't have a groundentity
    // slide along the steep plane
    if ( pml.groundPlane ) {
        PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
                         pm->ps->velocity, OVERCLIP );
    }

#if 0
    //ZOID:  If we are on the grapple, try stair-stepping
    //this allows a player to use the grapple to pull himself
    //over a ledge
    //	if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
    //		PM_StepSlideMove ( qtrue );
    //	else
    PM_SlideMove ( qtrue );
#endif

    PM_StepSlideMove ( qtrue );
}

/*
===================
PM_GrappleMove

===================
static void PM_GrappleMove( void ) {
vec3_t vel, v;
float vlen;

VectorScale(pml.forward, -16, v);
VectorAdd(pm->ps->grapplePoint, v, v);
VectorSubtract(v, pm->ps->origin, vel);
vlen = VectorLength(vel);
VectorNormalize( vel );

if (vlen <= 100)
VectorScale(vel, 10 * vlen, vel);
else
VectorScale(vel, 800, vel);

VectorCopy(vel, pm->ps->velocity);

pml.groundPlane = qfalse;
}
*/

static float PM_GetSprintScale( void )
{
    int speed = pm->ps->persistant[PERS_SPEED];
    float	base = pm_sprintScale;

    base -= (float)( (float)speed / 10.0f ) * 0.50;

    return base;
}
/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove( void ) {
    int			i;
    vec3_t		wishvel;
    float		fmove, smove;
    vec3_t		wishdir;
    float		wishspeed;
    float		scale;
    usercmd_t	cmd;
    float		accelerate;
    float		vel;
    vec3_t  dist;

    if ( pm->waterlevel > 2 && DotProduct( pml.forward, pml.groundTrace.plane.normal ) > 0 ) {
        // begin swimming
        PM_WaterMove();
        return;
    }


    if ( PM_CheckJump () ) {
        // jumped away
        if ( pm->waterlevel > 1 ) {
            PM_WaterMove();
        } else {
            PM_AirMove();
        }
        return;
    }

    PM_Friction ();

    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.rightmove;

    cmd = pm->cmd;
    scale = PM_CmdScale( &cmd );

    // set the movementDir so clients can rotate the legs for strafing
    PM_SetMovementDir();

    // project moves down to flat plane
    pml.forward[2] = 0;
    pml.right[2] = 0;

    // project the forward and right directions onto the ground plane
    PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP );
    PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP );
    //
    VectorNormalize (pml.forward);
    VectorNormalize (pml.right);

    for ( i = 0 ; i < 3 ; i++ ) {
        wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
    }
    // when going up or down slopes the wish velocity should Not be zero
    //	wishvel[2] = 0;

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);
    wishspeed *= scale;

    {
        float	pm_backScale = 0.60f;
        float	pm_sideScale = 0.75f;

        if ( fmove < 0 ) {
            if ( wishspeed > pm->ps->speed * pm_backScale )
                wishspeed = pm->ps->speed * pm_backScale;
        }
        if ( smove != 0 ) {
            if ( wishspeed > pm->ps->speed * pm_sideScale )
                wishspeed = pm->ps->speed * pm_sideScale;
        }

    }

    // adjust speed for sprinting
    if ( pm->cmd.buttons & BUTTON_SPRINT ) {
        if ( pm->ps->stats[STAT_STAMINA] ) {
            int	xyzspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
                                 +  pm->ps->velocity[1] * pm->ps->velocity[1]
                                 +  pm->ps->velocity[2] * pm->ps->velocity[2] );

            if ( ( pm->cmd.forwardmove > 0 /* && pm->cmd.rightmove == 0 */ ) &&
                    xyzspeed > 50 &&
                    !(pm->cmd.buttons & BUTTON_WALKING) &&
                    /*	pm->ps->stats[STAT_LEG_DAMAGE] < 25  && */
                    pm->ps->groundEntityNum != ENTITYNUM_NONE
               )
            {
                int fixme; // !insert formula
                int mod = 0;

                switch ( pm->ps->persistant[PERS_STAMINA] ) {
                case 0:
                case 1:
                    mod = 1;
                    break;
                case 2:
                    mod = 1;
                    break;
                case 3:
                case 4:
                    mod = 2;
                    break;
                case 5:
                case 6:
                    mod = 2;
                    break;
                case 7:
                case 8:
                    mod = 3;
                    break;
                case 9:
                case 10:
                    mod = 3;
                    break;
                default:
                    mod = 1;
                    break;
                }
                fixme = 5 - mod;

                //			Com_Printf("sta lvl: %i mod: %i powr: %i scale: %f\n", pm->ps->persistant[PERS_STAMINA],
                //				mod, pm->ps->stats[STAT_STAMINA], PM_GetSprintScale() );

                if ( ( pm->ps->stats[PW_VEST] || pm->ps->stats[PW_HELMET] ) && pm->ps->persistant[PERS_STAMINA] < 5)
                    fixme++;
                /*	if ( pm->ps->stats[PW_VEST] && pm->ps->persistant[PERS_STAMINA] < 5 )
                fixme++;
                */
                if (fixme <= 0)
                    fixme = 1;


                // whee, Stamina fix
                VectorSubtract(pm->ps->origin, pm_previous_origin[pm->ps->clientNum], dist);
                if (VectorLength(dist) > 12) {

                    VectorCopy(pm->ps->origin, pm_previous_origin[pm->ps->clientNum]);
                    if (pm_previous_origin[pm->ps->clientNum] == 0) {
                        pm_lastsprint[pm->ps->clientNum] = 1;
                        pm->ps->stats[STAT_STAMINA] -= fixme;
                    } else {
                        if ( (VectorLength(dist) / 12) > 4)
                            pm->ps->stats[STAT_STAMINA] -= fixme;
                        else
                            pm->ps->stats[STAT_STAMINA] -= fixme * VectorLength(dist) / 12;
                    }

                }

                if ( pm->ps->stats[STAT_STAMINA] < 0 )
                    pm->ps->stats[STAT_STAMINA] = 0;

                wishspeed = pm->ps->speed * PM_GetSprintScale();
            }
        }
    } else {
        VectorCopy(pm->ps->origin, pm_previous_origin[pm->ps->clientNum]);
        pm_lastsprint[pm->ps->clientNum] = 0;
    }

    // clamp the speed lower if ducking
    if ( pm->ps->pm_flags & PMF_DUCKED ) {
        if ( wishspeed > pm->ps->speed * pm_duckScale ) {
            wishspeed = pm->ps->speed * pm_duckScale;
        }
    }

    // clamp the speed lower if wading or walking on the bottom
    if ( pm->waterlevel ) {
        float	waterScale;

        waterScale = pm->waterlevel / 3.0;
        waterScale = 1.0 - ( 1.0 - pm_swimScale ) * waterScale;
        if ( wishspeed > pm->ps->speed * waterScale ) {
            wishspeed = pm->ps->speed * waterScale;
        }
    }

    // clamp the speed lower when using a scope
    if (BG_IsZooming(pm->ps->stats[STAT_WEAPONMODE]) ){
        // moving like crouching
        if (wishspeed  > pm->ps->speed * pm_duckScale ) {
            wishspeed = pm->ps->speed * pm_duckScale;
        }
    }

    // when a player gets hit, they temporarily lose
    // full control, which allows them to be moved a bit
    if ( pml.groundTrace.surfaceFlags & SURF_SLICK ) {
        accelerate = pm_airaccelerate;
    }	else if (  pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
        accelerate = 0;
    }	else if ( pm->cmd.buttons & BUTTON_WALKING || pm->cmd.upmove < 0 ) {
        accelerate = pm_walkaccelerate;
    } else if ( BG_IsZooming(pm->ps->stats[STAT_WEAPONMODE]) ) {
        accelerate = pm_walkaccelerate;
    } else {
        accelerate = pm_accelerate;
    }

    PM_Accelerate (wishdir, wishspeed, accelerate);

    //Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
    //Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

    if ( ( pml.groundTrace.surfaceFlags & SURF_SLICK ) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK ) {
        pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
    } else {
        // don't reset the z velocity for slopes
        //		pm->ps->velocity[2] = 0;
    }

    vel = VectorLength(pm->ps->velocity);

    // slide along the ground plane
    PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
                     pm->ps->velocity, OVERCLIP );

    // don't decrease velocity when going up or down a slope
    VectorNormalize(pm->ps->velocity);
    VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

    // don't do anything if standing still
    if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
        return;
    }

    PM_StepSlideMove( qfalse );

    //Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));

}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove( void ) {
    float	forward;

    if ( !pml.walking ) {
        return;
    }

    // extra friction

    forward = VectorLength (pm->ps->velocity);
    forward -= 20;
    if ( forward <= 0 ) {
        VectorClear (pm->ps->velocity);
    } else {
        VectorNormalize (pm->ps->velocity);
        VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
    }
}

#if 1
/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
    float	speed, drop, friction, control, newspeed;
    int			i;
    vec3_t		wishvel;
    float		fmove, smove;
    vec3_t		wishdir;
    float		wishspeed;
    float		scale;

    pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

    // friction

    speed = VectorLength (pm->ps->velocity);
    if (speed < 1)
    {
        VectorCopy (vec3_origin, pm->ps->velocity);
    }
    else
    {
        drop = 0;

        friction = pm_friction*1.5;	// extra friction
        control = speed < pm_stopspeed ? pm_stopspeed : speed;
        drop += control*friction*pml.frametime;

        // scale the velocity
        newspeed = speed - drop;
        if (newspeed < 0)
            newspeed = 0;
        newspeed /= speed;

        VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
    }

    // accelerate
    scale = PM_CmdScale( &pm->cmd );

    fmove = pm->cmd.forwardmove;
    smove = pm->cmd.rightmove;

    for (i=0 ; i<3 ; i++)
        wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
    wishvel[2] += pm->cmd.upmove;

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);
    wishspeed *= scale;

    PM_Accelerate( wishdir, wishspeed, pm_accelerate );

    // move
    VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}
#endif
//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface( void ) {
    if ( pml.groundTrace.surfaceFlags & SURF_NOSTEPS ) {
        return 0;
    }
    else if ( pml.groundTrace.surfaceFlags & SURF_METALSTEPS ) {
        return EV_FOOTSTEP_METAL;
    }
    else if ( pml.groundTrace.surfaceFlags & SURF_DIRTSTEPS || pml.groundTrace.surfaceFlags & SURF_SANDSTEPS ) {
        return EV_FOOTSTEP_DIRT;
    }
    else if ( pml.groundTrace.surfaceFlags & SURF_SNOWSTEPS ) {
        return EV_FOOTSTEP_SNOW;
    }
    else if ( pml.groundTrace.surfaceFlags & SURF_WOODSTEPS ) {
        return EV_FOOTSTEP_WOOD;
    } 
    // Navy Seals --
    return EV_FOOTSTEP;
}


/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
#define DELTA_METER	32 // 32 deltas are equal 1 meter falling
static void PM_CrashLand( void ) {
    float		dist, vel;
  
    pm->ps->legsTimer = TIMER_LAND;

    // calculate the exact velocity on landing
    dist = pm->ps->origin[2] - pml.previous_origin[2];
    if (dist < 0) dist = -dist;
    vel = sqrt ( pml.previous_velocity[0] * pml.previous_velocity[0] +
                 pml.previous_velocity[1] * pml.previous_velocity[1] +
                 pml.previous_velocity[2] * pml.previous_velocity[2] );
    if (vel < 0) vel = -vel;


    // never take falling damage if completely underwater
    if ( pm->waterlevel == 3 ) {
        return;
    }

    // reduce falling damage if there is standing water
    if ( pm->waterlevel == 2 ) {
        vel *= 0.7f;
    }
    if ( pm->waterlevel == 1 ) {
        vel *= 0.8f;
    } 

    PM_ForceLegsAnim( LEGS_LAND );

    if ( pml.groundTrace.surfaceFlags & SURF_DIRTSTEPS || pml.groundTrace.surfaceFlags & SURF_SANDSTEPS )
        vel *= 0.9f;
    else if ( pml.groundTrace.surfaceFlags & SURF_SNOWSTEPS || pml.groundTrace.surfaceFlags & SURF_SOFTSTEPS )
        vel *= 0.95f;

    if ( pm->debugLevel ) {
        Com_Printf("velocity %f\n", vel);
    }

    // SURF_NODAMAGE is used for bounce pads where you don't ever
    // want to take damage or play a crunch sound
    if ( !(pml.groundTrace.surfaceFlags & SURF_NODAMAGE) ) {

        if ( vel >  700.0 ) {

            PM_AddEvent( EV_FALL_DEATH );

        } else if ( vel > 660.0 ) {

            PM_AddEvent( EV_FALL_FAR );
            PM_AddEvent( PM_FootstepForSurface() );

        } else if ( vel > 580.0  ) {

            PM_AddEvent( EV_FALL_MEDIUM );
            PM_AddEvent( PM_FootstepForSurface() );

        } else if ( vel > 500.0 ) {

            PM_AddEvent( EV_FALL_LIGHT );
            PM_AddEvent( PM_FootstepForSurface() );

        } else if ( vel > 400.0 ) {

            PM_AddEvent( EV_FALL_SHORT );
            PM_AddEvent( PM_FootstepForSurface() );

        } else if ( vel > 300.0 ) {

            PM_AddEvent( PM_FootstepForSurface() );

        }
    }
    // start footstep cycle over
    pm->ps->bobCycle = 0;
}

/*
=============
PM_CheckStuck
=============
*/
/*
void PM_CheckStuck(void) {
trace_t trace;

pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
if (trace.allsolid) {
//int shit = qtrue;
}
}
*/

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid( trace_t *trace ) {
    int			i, j, k;
    vec3_t		point;

    if ( pm->debugLevel ) {
        Com_Printf("%i:allsolid\n", c_pmove);
    }

    // jitter around
    for (i = -1; i <= 1; i++) {
        for (j = -1; j <= 1; j++) {
            for (k = -1; k <= 1; k++) {
                VectorCopy(pm->ps->origin, point);
                point[0] += (float) i;
                point[1] += (float) j;
                point[2] += (float) k;
                pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
                if ( !trace->allsolid ) {
                    point[0] = pm->ps->origin[0];
                    point[1] = pm->ps->origin[1];
                    point[2] = pm->ps->origin[2] - 0.25;

                    pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
                    pml.groundTrace = *trace;
                    return qtrue;
                }
            }
        }
    }

    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;

    return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed( void ) {
    trace_t		trace;
    vec3_t		point;

    if ( pm->ps->groundEntityNum != ENTITYNUM_NONE ) {
        // we just transitioned into freefall
        if ( pm->debugLevel ) {
            Com_Printf("%i:lift\n", c_pmove);
        }

        // if they aren't in a jumping animation and the ground is a ways away, force into it
        // if we didn't do the trace, the player would be backflipping down staircases
        VectorCopy( pm->ps->origin, point );
        point[2] -= 64;

        pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
        if ( trace.fraction == 1.0 ) {
            /*	if ( pm->cmd.forwardmove >= 0 ) {*/
            PM_ForceLegsAnim( LEGS_JUMP );
            pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
            /*	} else {
            PM_ForceLegsAnim( LEGS_JUMPB );
            pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
            }*/
        }
    }

    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace( void ) {
    vec3_t		point;
    trace_t		trace;

    point[0] = pm->ps->origin[0];
    point[1] = pm->ps->origin[1];
    point[2] = pm->ps->origin[2] - 0.25;

    pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
    pml.groundTrace = trace;

    // do something corrective if the trace starts in a solid...
    if ( trace.allsolid ) {
        if ( !PM_CorrectAllSolid(&trace) )
            return;
    }

    // if the trace didn't hit anything, we are in free fall
    if ( trace.fraction == 1.0 ) {
        PM_GroundTraceMissed();
        pml.groundPlane = qfalse;
        pml.walking = qfalse;
        return;
    }

    // check if getting thrown off the ground
    /*	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
    if ( pm->debugLevel ) {
    Com_Printf("%i:kickoff\n", c_pmove);
    }
    // go into jump animation
    /////////*		if ( pm->cmd.forwardmove >= 0 ) {*/
    /*		PM_ForceLegsAnim( LEGS_JUMP );
    pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;*/
    /*	} else {
    PM_ForceLegsAnim( LEGS_JUMPB );
    pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
    }*/
    /*
    pm->ps->groundEntityNum = ENTITYNUM_NONE;
    pml.groundPlane = qfalse;
    pml.walking = qfalse;
    return;
    }
    */
    // slopes that are too steep will not be considered onground
    if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
        if ( pm->debugLevel ) {
            Com_Printf("%i:steep\n", c_pmove);
        }
        // FIXME: if they can't slide down the slope, let them
        // walk (sharp crevices)
        pm->ps->groundEntityNum = ENTITYNUM_NONE;
        pml.groundPlane = qtrue;
        pml.walking = qfalse;
        return;
    }

    pml.groundPlane = qtrue;
    pml.walking = qtrue;

    // hitting solid ground will end a waterjump
    if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
    {
        pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
        pm->ps->pm_time = 0;
    }

    if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {
        // just hit the ground
        if ( pm->debugLevel ) {
            Com_Printf("%i:Land\n", c_pmove);
        }

        PM_CrashLand();

        // don't do landing time if we were just going down a slope
        if ( pml.previous_velocity[2] < -200 ) {
            // don't allow another jump for a little while
            pm->ps->pm_flags |= PMF_TIME_LAND;
            pm->ps->pm_time = 100;
        }
    }

    pm->ps->groundEntityNum = trace.entityNum;

    // don't reset the z velocity for slopes
    //	pm->ps->velocity[2] = 0;

    PM_AddTouchEnt( trace.entityNum );
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
    vec3_t		point;
    int			cont;
    int			sample1;
    int			sample2;

    //
    // get waterlevel, accounting for ducking
    //
    pm->waterlevel = 0;
    pm->watertype = 0;

    point[0] = pm->ps->origin[0];
    point[1] = pm->ps->origin[1];
    point[2] = pm->ps->origin[2] + MINS_Z + 1;
    cont = pm->pointcontents( point, pm->ps->clientNum );

    if ( cont & MASK_WATER ) {
        sample2 = pm->ps->viewheight - MINS_Z;
        sample1 = sample2 / 2;

        pm->watertype = cont;
        // feet in water
        pm->waterlevel = 1;
        point[2] = pm->ps->origin[2] + MINS_Z + sample1;
        cont = pm->pointcontents (point, pm->ps->clientNum );
        if ( cont & MASK_WATER ) {
            // wading
            pm->waterlevel = 2;
            point[2] = pm->ps->origin[2] + MINS_Z + sample2;
            cont = pm->pointcontents (point, pm->ps->clientNum );
            // face in water
            if ( cont & MASK_WATER ){
                pm->waterlevel = 3;
            }
        }
    }

}

/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/

static void PM_CheckDuck (void)
{
    trace_t	trace;

    pm->mins[0] = -10;
    pm->mins[1] = -10;

    pm->maxs[0] = 10;
    pm->maxs[1] = 10;

    pm->mins[2] = MINS_Z;

    if (pm->ps->pm_type == PM_DEAD)
    {
        pm->maxs[2] = -12;
        pm->ps->viewheight = DEAD_VIEWHEIGHT;
        return;
    }

    if (pm->cmd.upmove < 0)
    {	// duck
        pm->ps->pm_flags |= PMF_DUCKED;
    }
    else
    {	// stand up if possible
        if (pm->ps->pm_flags & PMF_DUCKED)
        {
            // try to stand up
            pm->maxs[2] = MAXS_Z;
            pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask );
            if (!trace.allsolid)
                pm->ps->pm_flags &= ~PMF_DUCKED;
        }
    }

    if (pm->ps->pm_flags & PMF_DUCKED)
    {
        pm->maxs[2] = 18; // original 18
        pm->ps->viewheight = CROUCH_VIEWHEIGHT;
    }
    else
    {
        pm->maxs[2] = MAXS_Z;
        pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
    }
}



//===================================================================


/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps( void ) {
    float		bobmove;
    int			old;
    qboolean	footstep;
    int			xyzspeed;
    int			onladder = NS_OnLadder();

    //
    // calculate speed and cycle to be used for
    // all cyclic walking effects
    //
    pm->xyspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
                        +  pm->ps->velocity[1] * pm->ps->velocity[1] );
    xyzspeed = sqrt( pm->ps->velocity[0] * pm->ps->velocity[0]
                     +  pm->ps->velocity[1] * pm->ps->velocity[1]
                     +  pm->ps->velocity[2] * pm->ps->velocity[2] );

    // Navy Seals ++
    //	CG_Printf("ladder: %i\n", onladder );

    if ( onladder == -1 || onladder == 1 ) // on ladder
    {
        // moving up or down)
        if ( xyzspeed ) {
            PM_ContinueTorsoAnim ( TORSO_CLIMB );
            PM_ContinueLegsAnim  ( LEGS_CLIMB );
            footstep = qtrue;
        }
        else {
            PM_ContinueTorsoAnim( TORSO_CLIMB_IDLE );
            PM_ContinueLegsAnim( LEGS_CLIMB_IDLE );
            pm->ps->bobCycle = 0;	// start at beginning of cycle again
            footstep = qfalse;
        }

        bobmove = 0.45f;	// walking bobs slow
        old = pm->ps->bobCycle;
        pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

        // Navy Seals ++
        if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) {
            // on ground will only play sounds if running
            if ( footstep && !pm->noFootsteps && ( pm->ps->stats[STAT_STEALTH] < 7  ||  pm->ps->eFlags & EF_VIP  || pm->cmd.buttons & BUTTON_SPRINT )  ) {
                PM_AddEvent( EV_FOOTSTEP_METAL );
            }
        }
        return;
    }
    // Navy Seals --

    if ( pm->ps->groundEntityNum == ENTITYNUM_NONE ) {


        // airborne leaves position in cycle intact, but doesn't advance
        if ( pm->waterlevel > 1 ) {
            PM_ContinueLegsAnim( LEGS_SWIM );
        }
        return;
    }

    // if not trying to move
    if ( !pm->cmd.forwardmove && !pm->cmd.rightmove ) {
        if (  pm->xyspeed < 5 ) {
            pm->ps->bobCycle = 0;	// start at beginning of cycle again
            if ( pm->ps->pm_flags & PMF_DUCKED ) {
                if (
                    BG_IsZooming( pm->ps->stats[STAT_WEAPONMODE] )
                    &&
                    pm->ps->weaponstate == WEAPON_READY
                )
                {
                    PM_ContinueLegsAnim( LEGS_IDLECR_RIFLE_SCOPED );
                }
                else
                    PM_ContinueLegsAnim( LEGS_IDLECR );
            } else {

                // soulcroushers tip: stop playermodels from bobbing.
                if (
                    BG_IsZooming( pm->ps->stats[STAT_WEAPONMODE] )
                    &&
                    pm->ps->weaponstate == WEAPON_READY
                )
                {
                    PM_ContinueLegsAnim( LEGS_IDLE_RIFLE_SCOPED );
                }
                else
                {
                    PM_ContinueLegsAnim( LEGS_IDLE );
                }
            }
        }
        return;
    }


    footstep = qfalse;

    if ( (pm->ps->pm_flags & PMF_DUCKED) ) {
        bobmove = 0.3f;	// ducked characters bob much slower
        if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
            PM_ContinueLegsAnim( LEGS_BACKCR ); // need back cr
        }
        else {
            PM_ContinueLegsAnim( LEGS_WALKCR );
        }
        // ducked characters never play footsteps
        /*
        } else 	if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
        if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
        bobmove = 0.4;	// faster speeds bob faster
        footstep = qtrue;
        } else {
        bobmove = 0.3;
        }
        PM_ContinueLegsAnim( LEGS_BACK );
        */
    } else if ( (pm->ps->stats[STAT_WEAPONMODE] & (1 << WM_ZOOM2X) ) ||
                (pm->ps->stats[STAT_WEAPONMODE] & (1 << WM_ZOOM2X) ) ) {

        bobmove = 0.3f;	// walking bobs slow

        if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
            PM_ContinueLegsAnim( LEGS_BACK  );
        }
        else {
            PM_ContinueLegsAnim( LEGS_WALK );
        }
    } else {
        if ( pm->ps->stats[STAT_LEG_DAMAGE] > 20 ) {
            bobmove = 0.2f;
            if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN )
                PM_ContinueLegsAnim( LEGS_BACKLIMB );
            else
                PM_ContinueLegsAnim( LEGS_LIMP );
            footstep = qfalse;
        }
        else
        {
            if ( !( pm->cmd.buttons & BUTTON_WALKING ) ) {
                bobmove = 0.4f;	// faster speeds bob faster
                if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
                    PM_ContinueLegsAnim( LEGS_BACK );
                }
                else {
                    PM_ContinueLegsAnim( LEGS_RUN );
                }
                footstep = qtrue;
            } else {
                bobmove = 0.3f;	// walking bobs slow

                if ( pm->ps->pm_flags & PMF_BACKWARDS_RUN ) {
                    PM_ContinueLegsAnim( LEGS_BACK  );
                }
                else {

                    PM_ContinueLegsAnim( LEGS_WALK );
                }
            }
        }
    }

    // check for footstep / splash sounds
    old = pm->ps->bobCycle;
    pm->ps->bobCycle = (int)( old + bobmove * pml.msec ) & 255;

    // Navy Seals ++
    if ( ( ( old + 64 ) ^ ( pm->ps->bobCycle + 64 ) ) & 128 ) {
        if ( pm->waterlevel == 0 ) {
            // on ground will only play sounds if running
            if ( footstep && !pm->noFootsteps && ( pm->ps->stats[STAT_STEALTH] < 6  ||  pm->ps->eFlags & EF_VIP  || pm->cmd.buttons & BUTTON_SPRINT ) && PM_FootstepForSurface() == EV_FOOTSTEP ) {
                PM_AddEvent( EV_FOOTSTEP );
            }
            else if ( footstep && !pm->noFootsteps && ( pm->ps->stats[STAT_STEALTH] < 8  ||  pm->ps->eFlags & EF_VIP  || pm->cmd.buttons & BUTTON_SPRINT )&& PM_FootstepForSurface() > EV_FOOTSTEP ) {
                PM_AddEvent( PM_FootstepForSurface() );
            }
        } else if ( pm->waterlevel == 1 && ( pm->ps->stats[STAT_STEALTH] < 9  ||  pm->ps->eFlags & EF_VIP  || pm->cmd.buttons & BUTTON_SPRINT ) ){
            // splashing
            PM_AddEvent( EV_FOOTSPLASH );
        } else if ( pm->waterlevel == 2 && ( pm->ps->stats[STAT_STEALTH] < 9  ||  pm->ps->eFlags & EF_VIP || pm->cmd.buttons & BUTTON_SPRINT ) ){
            // wading / swimming at surface
            PM_AddEvent( EV_SWIM );
        } else if ( pm->waterlevel == 3 ) {
            // no sound when completely underwater

        }
    }
    // Navy Seals --

}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {		// FIXME?
    //
    // if just entered a water volume, play a sound
    //
    // Navy Seals ++
    if ( pm->ps->stats[STAT_STEALTH] < 4 ||   pm->ps->eFlags & EF_VIP || pm->cmd.buttons & BUTTON_SPRINT )
        // Navy Seals --
        if (!pml.previous_waterlevel && pm->waterlevel) {
            PM_AddEvent( EV_WATER_TOUCH );
        }

    //
    // if just completely exited a water volume, play a sound
    //
    // Navy Seals ++
    if ( pm->ps->stats[STAT_STEALTH] < 4 ||   pm->ps->eFlags & EF_VIP || pm->cmd.buttons & BUTTON_SPRINT )
        // Navy Seals --
        if (pml.previous_waterlevel && !pm->waterlevel) {
            PM_AddEvent( EV_WATER_LEAVE );
        }

    //
    // check for head just going under water
    //
    // Navy Seals ++
    if ( pm->ps->stats[STAT_STEALTH] < 3  || pm->ps->eFlags & EF_VIP || pm->cmd.buttons & BUTTON_SPRINT )
        // Navy Seals --
        if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
            PM_AddEvent( EV_WATER_UNDER );
        }

    //
    // check for head just coming out of water
    //
    // Navy Seals ++
    if ( pm->ps->stats[STAT_STEALTH] < 3  || pm->ps->eFlags & EF_VIP || pm->cmd.buttons & BUTTON_SPRINT )
        // Navy Seals --
        if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
            PM_AddEvent( EV_WATER_CLEAR );
        }
}

// Navy Seals ++
/*
===============
PM_GetWeaponTimeOnRaise
===============
*/
static int PM_GetWeaponTimeOnRaise( void ) {

    switch (pm->ps->weapon) {
    case WP_KHURKURI:
    case WP_SEALKNIFE:
        return 480;
    case WP_MAC10:
        return 600;
    case WP_MP5:
        return 640;
    case WP_M249:
        return 800;
    case WP_M4:
        return 880;
    case WP_AK47:
        return 400;
    case WP_MK23:
        return 600;
    case WP_DEAGLE:
    case WP_SW629:
    case WP_SW40T:
        return 600;
    case WP_PSG1:
        return 840;
    case WP_MACMILLAN:
        return 750;
    case WP_PDW:
    case WP_M14:
        return 400;
    case WP_870:
        return 600;
    case WP_M590:
        return 600;
    case WP_P9S:
    case WP_SPAS15:
        return 600;
    case WP_SMOKE:
    case WP_GRENADE:
    case WP_FLASHBANG:
    case WP_GLOCK:
        return 400;
#ifdef SL8SD
    case WP_SL8SD:
        return 800;
#endif
    default:
        return 100;
    }
}

/*
===============
PM_GetWeaponTimeOnLower
===============
*/
static int PM_GetWeaponTimeOnLower( void ) {

    switch (pm->ps->weapon) {
    case WP_AK47:
        return 400;
    case WP_M249:
        return 400;
    case WP_M14:
    case WP_KHURKURI:
    case WP_SEALKNIFE:
        return 400;
    case WP_MP5:
        return 400;
    case WP_MK23:
    case WP_MAC10:
    case WP_M4:
    case WP_P9S:
    case WP_SW40T:
    case WP_GLOCK:
    case WP_DEAGLE:
    case WP_SW629:
        return 400;
    case WP_MACMILLAN:
        return 500;
    case WP_PSG1:
        return 600;
    case WP_PDW:
        return 400;
    case WP_870:
        return 600;
    case WP_M590:
        return 600;
    case WP_SPAS15:
        return 400;
    case WP_SMOKE:
    case WP_GRENADE:
    case WP_FLASHBANG:
        return 360;
#ifdef SL8SD
    case WP_SL8SD:
        return 400;
#endif
    default:
        return 100;
    }
}
// Navy Seals --
/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange( int weapon ) {
    float weaponTime = PM_GetWeaponTimeOnLower() * BG_GetSpeedMod( pm->ps->persistant[PERS_TECHNICAL] );

    if ( weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS ) {
        return;
    }

    if ( !( BG_GotWeapon( weapon, pm->ps->stats ) ) ) {
        return;
    }

    if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
        return;
    }

    //	Com_Printf("weaponTime: %3f\n", weaponTime );

    pm->ps->weaponstate = WEAPON_DROPPING;
    pm->ps->weaponTime = weaponTime;

    //	Com_Printf("weapontime WEAPON_DROPPING: %i\n", (int)weaponTime );

    pm->ps->torsoTimer = pm->ps->weaponTime;

    if (BG_IsPrimary( pm->ps->weapon ) )
        PM_StartTorsoAnim( TORSO_DROP_RIFLE );
    else
        PM_StartTorsoAnim( TORSO_DROP_PISTOL );

}


/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange( void ) {
    // Navy Seals ++
    int		weapon;
    float weaponTime = 1.0f;

    weapon = pm->cmd.weapon;
    if ( weapon < WP_NONE || weapon >= WP_NUM_WEAPONS ) {
        weapon = WP_NONE;
    }

    if ( ! ( BG_GotWeapon( weapon, pm->ps->stats ) ) ) {
        weapon = WP_NONE;
    }

    if (pm->ps->weaponstate != WEAPON_BANDAGING) {
        pm->ps->weapon = weapon;
        // only send sound if we're actually changing
        if ( pm->ps->weaponstate != WEAPON_BANDAGING_END )
            PM_AddEvent( EV_CHANGE_WEAPON ); // get weapon cock sound
    }

    weaponTime = PM_GetWeaponTimeOnRaise() * BG_GetSpeedMod( pm->ps->persistant[PERS_TECHNICAL] );

    //	Com_Printf("weaponTime: %3f\n", weaponTime );

    pm->ps->weaponstate = WEAPON_RAISING;
    pm->ps->weaponTime = (int)weaponTime;

    //	Com_Printf("weapontime WEAPON_RAISING: %i\n", (int)weaponTime );

    pm->ps->torsoTimer = pm->ps->weaponTime;

    if ( BG_IsPrimary( weapon ) )
        PM_StartTorsoAnim( TORSO_RAISE_RIFLE );
    else
        PM_StartTorsoAnim( TORSO_RAISE_PISTOL );
    // Navy Seals --
}


/*
==============
PM_TorsoAnimation

==============
*/
static void PM_TorsoAnimation( void ) {
    // Navy Seals ++
    if ( NS_OnLadder() == 1 || NS_OnLadder() == -1 )
        return;

    if ( pm->ps->weaponstate == WEAPON_READY ) {
        if ( pm->ps->powerups[PW_BRIEFCASE] )
            PM_ContinueTorsoAnim( TORSO_STAND_SUITCASE );
        else if (
            ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM2X ) ) ||
            ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM4X ) ) &&
            ( BG_IsRifle( pm->ps->weapon ) || pm->ps->weapon == WP_M4 )
        )
            PM_ContinueTorsoAnim( TORSO_STAND_RIFLE_SCOPED );
        else if
        (
            ( pm->ps->weapon == WP_M4 || pm->ps->weapon == WP_AK47 ||
              pm->ps->weapon == WP_MP5 || pm->ps->weapon == WP_M14 ) &&
            !(pm->cmd.buttons & BUTTON_SPRINT && pm->ps->stats[STAT_STAMINA] > 0)
        )
            PM_ContinueTorsoAnim( TORSO_STAND_RIFLE_SCOPED );
        else if ( BG_IsPistol( pm->ps->weapon ) || pm->ps->weapon == WP_PDW || pm->ps->weapon == WP_MAC10 )
            PM_ContinueTorsoAnim( TORSO_STAND_PISTOL );
        else if ( BG_IsRifle( pm->ps->weapon ) )
            PM_ContinueTorsoAnim( TORSO_STAND_RIFLE );
        else if ( BG_IsMelee( pm->ps->weapon ) || BG_IsGrenade( pm->ps->weapon ) || pm->ps->weapon == WP_C4 || pm->ps->weapon == WP_NONE )
            PM_ContinueTorsoAnim( TORSO_STAND_ITEM );
        else
            PM_ContinueTorsoAnim( TORSO_STAND_RIFLE );
        return;
    }
    // Navy Seals --
}

// Navy Seals ++
/*
============
PM_GetWeaponTime

the firerates of weapons
============
*/
static int PM_GetWeaponTime( int weapon ) {
    int addTime;

    switch( weapon ) {
    case WP_PDW:
        addTime = SEALS_FRATE_PDW;
        break;
    case WP_MP5:
        addTime = SEALS_FRATE_MP5;
        break;
    case WP_MAC10:
        addTime = SEALS_FRATE_MAC10;
        break;
    case WP_P9S:
        addTime = SEALS_FRATE_P9S;
        break;
    case WP_GLOCK:
        addTime = SEALS_FRATE_GLOCK;
        break;
    case WP_MK23:
        addTime = SEALS_FRATE_MK23;
        break;
    case WP_M590:
        addTime = SEALS_FRATE_M590;
        break;
    case WP_870:
        addTime = SEALS_FRATE_870;
        break;
    case WP_SPAS15:
        addTime = SEALS_FRATE_SPAS15;
        break;
    case WP_M14:
        addTime = SEALS_FRATE_M14;
        break;
    case WP_MACMILLAN:
        addTime = SEALS_FRATE_MACMILLAN;
        break;
    case WP_SL8SD:
        addTime = SEALS_FRATE_SL8SD;
        break;
    case WP_PSG1:
        addTime = SEALS_FRATE_SL8SD;
        break;
    case WP_M4:
        addTime = SEALS_FRATE_M4;
        break;
    case WP_AK47:
        addTime = SEALS_FRATE_AK47;
        break;
    case WP_M249:
        addTime = SEALS_FRATE_M249;
        break;
    case WP_SW629:
        addTime = SEALS_FRATE_SW629;
        break;
    case WP_DEAGLE:
        addTime = SEALS_FRATE_DEAGLE;
        break;
    case WP_SW40T:
        addTime = SEALS_FRATE_SW40T;
        break;
    case WP_SEALKNIFE:
        addTime = SEALS_FRATE_SEALKNIFE;
        break;
    case WP_KHURKURI:
        addTime = SEALS_FRATE_KHURKURI;
        break;
    default:
        addTime = SEALS_FRATE_DEFAULT;
    }

    return addTime;
}
// Navy Seals --
/*
==============
PM_BombCase

Handles bombcase 1st person weapon
==============
*/
static void PM_Weapon_Bombcase( void )
{
    int event = 0;

    if ( !(pm->ps->pm_flags & PMF_BOMBCASE) )
        return;

    if ( pm->ps->weaponTime > 0 )
        pm->ps->weaponTime -= pml.msec;

    PM_ContinueTorsoAnim( TORSO_STAND_ITEM );
    PM_ContinueLegsAnim( LEGS_IDLE );

    if ( pm->ps->weaponstate == WEAPON_LASTRND
            || pm->ps->weaponstate == WEAPON_BANDAGING_START
            || pm->ps->weaponstate == WEAPON_BANDAGING_END
            || pm->ps->weaponstate == WEAPON_BANDAGING
            || pm->ps->weaponstate == WEAPON_MELEE
            || pm->ps->weaponstate == WEAPON_THROW
            || pm->ps->weaponstate == WEAPON_FIRING21
            || pm->ps->weaponstate == WEAPON_FIRING22 && pm->ps->weaponTime <= 5)
    {
        if ( pm->ps->weaponstate == WEAPON_LASTRND )
            event = EV_CLIPWIRE_1;
        else if ( pm->ps->weaponstate == WEAPON_BANDAGING_START )
            event = EV_CLIPWIRE_2;
        else if ( pm->ps->weaponstate == WEAPON_BANDAGING_END )
            event = EV_CLIPWIRE_3;
        else if ( pm->ps->weaponstate == WEAPON_BANDAGING )
            event = EV_CLIPWIRE_4;
        else if ( pm->ps->weaponstate == WEAPON_MELEE )
            event = EV_CLIPWIRE_5;
        else if ( pm->ps->weaponstate == WEAPON_THROW )
            event = EV_CLIPWIRE_6;
        else if ( pm->ps->weaponstate == WEAPON_FIRING21 )
            event = EV_CLIPWIRE_7;
        else if ( pm->ps->weaponstate == WEAPON_FIRING22 )
            event = EV_CLIPWIRE_8;
        else
            return;

        PM_AddEvent(event);
    }

    if ( pm->ps->weaponTime > 0 )
        return;

    //	Com_Printf("ws: %i  -- ", pm->ps->weaponstate );

    if ( pm->ps->weaponstate == WEAPON_RAISING ) {
        //	Com_Printf("finished raising.\n");
        pm->ps->weaponstate = WEAPON_FIRING;
        return;
    }

    // pressing use...
    if ( pm->cmd.buttons & BUTTON_USE && !(pm->ps->pm_flags & PMF_SHOT_LOCKED) )
    {
        //		Com_Printf("pushed use.\n");
        pm->ps->pm_flags |= PMF_SHOT_LOCKED;
        /*
        WEAPON_FIRING - on wire 1
        WEAPON_FIRING2 - on wire 2
        WEAPON_FIRING3 - on wire 3
        WEAPON_FIREEMPTY - on wire 4
        WEAPON_RELOADING - on wire 5
        WEAPON_RELOADING_CYCLE - on wire 6
        WEAPON_RELOADING_STOP - on wire 7
        WEAPON_RELOADING_EMPTY - on wire 8
        */
        if ( pm->ps->weaponstate == WEAPON_FIRING )
        {
            pm->ps->weaponstate = WEAPON_FIRING2;
        }
        else if ( pm->ps->weaponstate == WEAPON_FIRING2 )
        {
            pm->ps->weaponstate = WEAPON_FIRING3;
        }
        else if ( pm->ps->weaponstate == WEAPON_FIRING3 )
        {
            pm->ps->weaponstate = WEAPON_FIREEMPTY;
        }
        else if ( pm->ps->weaponstate == WEAPON_FIREEMPTY )
        {
            pm->ps->weaponstate = WEAPON_RELOADING;
        }
        else if ( pm->ps->weaponstate == WEAPON_RELOADING )
        {
            pm->ps->weaponstate = WEAPON_RELOADING_CYCLE;
        }
        else if ( pm->ps->weaponstate == WEAPON_RELOADING_CYCLE )
        {
            pm->ps->weaponstate = WEAPON_RELOADING_STOP;
        }
        else if ( pm->ps->weaponstate == WEAPON_RELOADING_STOP )
        {
            pm->ps->weaponstate = WEAPON_RELOADING_EMPTY;
        }
        else if ( pm->ps->weaponstate == WEAPON_RELOADING_EMPTY )
        {
            pm->ps->weaponstate = WEAPON_FIRING;
        }
        else
            pm->ps->weaponstate = WEAPON_FIRING; // if we came here from some unknown wire go back to first!

        //		Com_Printf("Changed to wire #%i.\n", pm->ps->weaponstate );
        pm->ps->weaponTime = 200;
        return;
    }
    if (!( pm->cmd.buttons & BUTTON_ATTACK ) ) {
        pm->ps->weaponTime = 0;
        pm->ps->pm_flags &= ~PMF_SHOT_LOCKED;
        return;
    }

    if ( pm->ps->pm_flags & PMF_SHOT_LOCKED )
        return;


    //	Com_Printf("Clipping wire #%i.\n", pm->ps->weaponstate );

    pm->ps->pm_flags |= PMF_SHOT_LOCKED;

    if ( pm->ps->weaponstate == WEAPON_FIRING )
    {
        pm->ps->weaponstate = WEAPON_LASTRND;
    }
    else if ( pm->ps->weaponstate == WEAPON_FIRING2 )
    {
        pm->ps->weaponstate = WEAPON_BANDAGING_START;
    }
    else if ( pm->ps->weaponstate == WEAPON_FIRING3 )
    {
        pm->ps->weaponstate = WEAPON_BANDAGING_END;
    }
    else if ( pm->ps->weaponstate == WEAPON_FIREEMPTY )
    {
        pm->ps->weaponstate = WEAPON_BANDAGING;
    }
    else if ( pm->ps->weaponstate == WEAPON_RELOADING )
    {
        pm->ps->weaponstate = WEAPON_MELEE;
    }
    else if ( pm->ps->weaponstate == WEAPON_RELOADING_CYCLE )
    {
        pm->ps->weaponstate = WEAPON_THROW;
    }
    else if ( pm->ps->weaponstate == WEAPON_RELOADING_STOP )
    {
        pm->ps->weaponstate = WEAPON_FIRING21;
    }
    else if ( pm->ps->weaponstate == WEAPON_RELOADING_EMPTY )
    {
        pm->ps->weaponstate = WEAPON_FIRING22;
    }

    pm->ps->weaponTime = 1000;

    return;
}

/*
==============
PM_Seed

Update weapon seed for firing
==============
*/
static void PM_Seed( void )
{
    int seed;

    // Update the seed
    seed = pm->ps->stats[STAT_SEED];
    Q_rand ( &seed );
    seed = seed & 0xFFFF;
    pm->ps->stats[STAT_SEED] = seed;
}


/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void ) {
    // Navy Seals ++ : everything has been rewritten
    // don't allow attack until all buttons are up
    if ( pm->ps->pm_flags & PMF_RESPAWNED ) {
        return;
    }

    // ignore if spectator
    if ( pm->ps->pm_type == PM_SPECTATOR || pm->ps->pm_type == PM_NOCLIP ) {
        return;
    }


    // check for dead player
    if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
        pm->ps->weapon = WP_NONE;
        return;
    }

    // handle bombcase
    if ( (pm->ps->pm_flags & PMF_BOMBCASE) && pm->ps->weapon == WP_C4 )
    {
        PM_Weapon_Bombcase();
        return;
    }
#if 0
    // check for item using
    if ( pm->cmd.buttons & BUTTON_USE_HOLDABLE ) {
        if ( ! ( pm->ps->pm_flags & PMF_USE_ITEM_HELD ) ) {
            if ( bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT
                    && pm->ps->stats[STAT_HEALTH] >= pm->ps->stats[STAT_MAX_HEALTH] ) {
                // don't use medkit if at max health
            } else {
                pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
                PM_AddEvent( EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag );
                pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
            }
            return;
        }
    } else {
        pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
    }
#endif


    // make weapon function
    if ( pm->ps->weaponTime > 0 )
    {
        pm->ps->weaponTime -= pml.msec;



        if
        (
            !( pm->ps->eFlags & EF_WEAPONS_LOCKED)
            &&
            (
                pm->ps->weaponstate == WEAPON_FIRING ||
                pm->ps->weaponstate == WEAPON_FIRING2 ||
                pm->ps->weaponstate == WEAPON_FIRING3
            )
        )
        {
            // special rule for pistols to allow extrem fast trigger pushing.
            if ( BG_IsMelee( pm->ps->weapon ) )
            {
                if ( (pm->cmd.buttons & BUTTON_ATTACK ) && pm->ps->weaponTime < PM_GetWeaponTime(pm->ps->weapon)-50 && !(pm->ps->pm_flags & PMF_SHOT_LOCKED) )
                {
                    if ( pm->ps->weaponstate == WEAPON_FIRING )
                        pm->ps->weaponstate = WEAPON_FIRING2;
                    else if ( pm->ps->weaponstate == WEAPON_FIRING2 )
                        pm->ps->weaponstate = WEAPON_FIRING;
                    else
                        pm->ps->weaponstate = WEAPON_FIRING2;

                    if (pm->ps->stats[STAT_ROUNDS] == 1)
                        pm->ps->weaponstate = WEAPON_LASTRND; // go to lastrnd

                    PM_AddEvent( EV_FIRE_WEAPON );
                    pm->ps->weaponTime = PM_GetWeaponTime( pm->ps->weapon );
                    pm->ps->pm_flags |= PMF_SHOT_LOCKED;
                    PM_StartTorsoAnim( TORSO_ATTACK_MELEE );

                    return;
                }
                if ( pm->ps->pm_flags & PMF_SHOT_LOCKED && !(pm->cmd.buttons & 1 ) && pm->ps->weaponTime < PM_GetWeaponTime(pm->ps->weapon)-50 )
                {
                    pm->ps->pm_flags &=~ PMF_SHOT_LOCKED;
                }
            }
            else if ( BG_IsPistol( pm->ps->weapon ) )
            {
                if ( (pm->cmd.buttons & BUTTON_ATTACK ) && pm->ps->weaponTime < PM_GetWeaponTime(pm->ps->weapon)-50 &&  pm->ps->stats[STAT_ROUNDS] > 0 && !(pm->ps->pm_flags & PMF_SHOT_LOCKED) )
                {
                    if ( pm->ps->weaponstate == WEAPON_FIRING )
                        pm->ps->weaponstate = WEAPON_FIRING2;
                    else if ( pm->ps->weaponstate == WEAPON_FIRING2 )
                        pm->ps->weaponstate = WEAPON_FIRING;
                    else
                        pm->ps->weaponstate = WEAPON_FIRING2;

                    if (pm->ps->stats[STAT_ROUNDS] == 1)
                        pm->ps->weaponstate = WEAPON_LASTRND; // go to lastrnd

                    PM_Seed( );
                    PM_AddEvent( EV_FIRE_WEAPON );
                    pm->ps->weaponTime = PM_GetWeaponTime( pm->ps->weapon );
                    pm->ps->pm_flags |= PMF_SHOT_LOCKED;
                    PM_StartTorsoAnim( TORSO_ATTACK_PISTOL );
                    return;
                }
                if ( pm->ps->pm_flags & PMF_SHOT_LOCKED && !(pm->cmd.buttons & 1 ) && pm->ps->weaponTime < PM_GetWeaponTime(pm->ps->weapon)-50 )
                {
                    pm->ps->pm_flags &=~ PMF_SHOT_LOCKED;
                }
            }
        }
    }

    // special c4 stuff
    if ( pm->ps->weapon == WP_C4 && pm->ps->weaponstate == WEAPON_FIRING )
    {
        if ( ( pm->cmd.buttons & BUTTON_ATTACK ) && ( pm->ps->pm_flags & PMF_BOMBRANGE))
        {
            if ( pm->ps->weaponTime <= 0 )
            {
                pm->ps->weaponTime = 0;
                BG_RemoveWeapon( WP_C4, pm->ps->stats );
                pm->ps->weapon = WP_NONE;
                pm->ps->weaponTime = 100;

                PM_AddEvent( EV_C4DEPLOY );
                return;
            }
            pm->ps->weaponTime -= pml.msec;
            return;
        }
        else
        {
            pm->ps->weaponTime = 0;
            pm->ps->weaponstate = WEAPON_READY;
        }
    }

    // generic grenade throwing
    if (  pm->ps->weapon == WP_GRENADE || pm->ps->weapon == WP_FLASHBANG || pm->ps->weapon == WP_SMOKE )
    {
        int ammotype = AM_GRENADES;

        if ( pm->ps->weapon == WP_FLASHBANG )
            ammotype = AM_FLASHBANGS;
        else if ( pm->ps->weapon == WP_SMOKE )
            ammotype = AM_SMOKE;

        if ( pm->ps->weaponstate == WEAPON_FIRING2 || pm->ps->weaponstate == WEAPON_FIRING3)
        {
            if ( pm->ps->weaponTime <= 0 ) // finished throwing
            {
                if ( pm->ps->ammo[ammotype] <= 0) {
                    BG_RemoveWeapon( pm->ps->weapon, pm->ps->stats );
                    pm->ps->weapon = WP_NONE;
                }
                else {
                    float weaponTime = PM_GetWeaponTimeOnRaise() * BG_GetSpeedMod( pm->ps->persistant[PERS_TECHNICAL] );

                    //					Com_Printf("weaponTime: %3f\n", weaponTime );

                    pm->ps->weaponTime = (int)weaponTime;
                    pm->ps->weaponstate = WEAPON_RAISING;
                }

                return;
            }
        }
        if ( pm->ps->weaponstate == WEAPON_FIRING ) // released button
        {
            if ( !(pm->cmd.buttons & BUTTON_ATTACK ) && pm->ps->weaponTime <= 0 )
            {
                PM_AddEvent( EV_FIRE_WEAPON ); // send event
                pm->ps->ammo[ammotype]--;	   // remove a grenade
                if ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADEROLL ) )
                {
                    pm->ps->weaponstate = WEAPON_FIRING3;
                    pm->ps->weaponTime = 560; // set weapontime
                }
                else {
                    pm->ps->weaponstate = WEAPON_FIRING2;
                    pm->ps->weaponTime = 360; // set weapontime
                }
                PM_StartTorsoAnim( TORSO_THROW );
            }
            else if ( pm->cmd.buttons & BUTTON_ATTACK )
            {
                if (pm->ps->weaponTime <= -500 )
                    return;

                if ( pm->ps->weaponstate == WEAPON_FIRING3 || pm->ps->weaponstate == WEAPON_FIRING2 )
                    pm->ps->weaponTime--;

            }
            return;
        }

    }


    //
    // Bandaging
    //

    if ( pm->ps->weaponstate == WEAPON_BANDAGING_START ) {
        if (  pm->ps->weaponTime <= 0 ) {
            pm->ps->weaponstate = WEAPON_BANDAGING;
            return;
        }
        else
            return;
    }
    if ( pm->ps->weaponstate == WEAPON_BANDAGING) {
        if ( pm->cmd.weapon != pm->ps->weapon )
            pm->ps->weapon = pm->cmd.weapon;

        if ( pm->cmd.buttons & BUTTON_ATTACK )
        {
            pm->ps->weaponstate = WEAPON_BANDAGING_END;
            return;
        }
        if (pm->ps->weaponTime <= 0) {
            PM_AddEvent( EV_BANDAGING );
            return;
        }
        else
            return;
    }
    if ( pm->ps->weaponstate == WEAPON_BANDAGING_END ) {
        PM_FinishWeaponChange( );
    }

    if ( pm->ps->weaponstate == WEAPON_RELOADING_STOP )
    {
        if ( pm->ps->weaponTime > 0 )
            return;

        if ( pm->ps->weapon == WP_870 || pm->ps->weapon == WP_M590 )
        {
            pm->ps->weaponstate = WEAPON_READY;
            return;
        }
    }
    // change weapon to null if we've finished holstering
    if ( pm->ps->weaponstate == WEAPON_HOLSTERING ) {
        pm->ps->weapon = WP_NONE;
        if ( pm->cmd.buttons & BUTTON_ATTACK ) // if we hold the button key
        {					 // un holster weapon
            PM_FinishWeaponChange( );
        }
        //G_Printf("holstering\n");
        return;
    }

    if ( pm->ps->weaponstate == WEAPON_RELOADING_CYCLE )
    {
        if ( pm->ps->weaponTime > 0 )
            return;

        if ( pm->ps->weapon == WP_870 || pm->ps->weapon == WP_M590 )
        {
            int maxround = 7;

            maxround = BG_GetMaxRoundForWeapon(pm->ps->weapon);

            if ( pm->ps->stats[STAT_ROUNDS] >= maxround || pm->cmd.buttons & BUTTON_ATTACK || pm->ps->ammo[ AM_SHOTGUN ] <= 0 )
            {
                pm->ps->weaponstate = WEAPON_RELOADING_STOP;
                pm->ps->weaponTime = 1000;
                PM_AddEvent( EV_RELOAD_EMPTY );
                return;
            }
            else if ( pm->ps->stats[STAT_ROUNDS] < maxround && pm->ps->ammo[ AM_SHOTGUN ] > 0)
            {
                pm->ps->weaponstate = WEAPON_RELOADING_CYCLE;
                pm->ps->weaponTime = 600;
                PM_AddEvent( EV_RELOAD );
                return;
            }
        }
    }

    if ( pm->ps->weapon == WP_MACMILLAN && pm->ps->weaponstate == WEAPON_FIRING && pm->ps->weaponTime <= 0)
    {
        //	if (pm->ps->stats[STAT_ROUNDS] > 0) {
        pm->ps->weaponstate = WEAPON_RELOADING_CYCLE;
        pm->ps->weaponTime = 1800;
        PM_AddEvent( EV_RELOAD_EMPTY ); //  play bolt action sound
        return;
        //	}
    }
    if ( pm->ps->weaponstate == WEAPON_RELOADING || pm->ps->weaponstate == WEAPON_RELOADING_EMPTY) {
        if (pm->ps->weapon == WP_MACMILLAN && pm->ps->weaponstate == WEAPON_RELOADING_EMPTY)
        {
            if (pm->ps->weaponTime <= 0 )
            {
                pm->ps->weaponstate = WEAPON_RELOADING_CYCLE;
                pm->ps->weaponTime = 1800;
                PM_AddEvent( EV_RELOAD_EMPTY );
                return;
            }
        }
        if ( ( pm->ps->weapon == WP_870 || pm->ps->weapon == WP_M590 ) && pm->ps->weaponTime <= 0 ) {

            pm->ps->weaponstate = WEAPON_RELOADING_CYCLE;
            pm->ps->weaponTime = 600;
            PM_AddEvent( EV_RELOAD );
        }
        if (pm->ps->weaponTime <= 0)
            pm->ps->weaponstate = WEAPON_READY;
        else
            return;
    }

    if ( pm->ps->weaponstate == WEAPON_DROPPING && pm->ps->weaponTime > 0 ) {
        //	G_Printf("dropping: %i\n", pm->ps->weaponTime );
        return;
    }

    if ( pm->ps->weaponTime > 0 ) {
        return;
    }

    // check for weapon change
    // can't change if weapon is firing, but can change
    // again if lowering or raising
    if ( pm->ps->weaponstate == WEAPON_READY ) {
        if ( pm->ps->powerups[PW_BRIEFCASE] && BG_IsPrimary( pm->cmd.weapon ) && pm->cmd.weapon != WP_C4 )
            ;
        else if ( pm->ps->weapon != pm->cmd.weapon && pm->ps->weaponstate != WEAPON_HOLSTERING) {
            PM_BeginWeaponChange( pm->cmd.weapon );
            return;
        }
    }


    // change weapon if time
    if ( pm->ps->weaponstate == WEAPON_DROPPING ) {
        PM_FinishWeaponChange();
        return;
    }

    if ( pm->ps->weaponstate == WEAPON_RAISING ) {
        pm->ps->weaponstate = WEAPON_READY;

        if ( pm->ps->powerups[PW_BRIEFCASE] )
            PM_StartTorsoAnim( TORSO_STAND_SUITCASE );
        else if (
            ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM2X ) ) ||
            ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM4X ) ) &&
            ( ( BG_IsRifle( pm->ps->weapon ) || pm->ps->weapon == WP_M4 ))
        )
            PM_StartTorsoAnim( TORSO_STAND_RIFLE_SCOPED );
        else if ( BG_IsRifle( pm->ps->weapon )  )
            PM_StartTorsoAnim( TORSO_STAND_RIFLE );
        else if ( BG_IsPistol( pm->ps->weapon ) || pm->ps->weapon == WP_PDW || pm->ps->weapon == WP_MAC10)
            PM_StartTorsoAnim( TORSO_STAND_PISTOL );
        else if ( BG_IsMelee( pm->ps->weapon ) )
            PM_StartTorsoAnim( TORSO_STAND_ITEM );
        else // default
            PM_StartTorsoAnim( TORSO_STAND_ITEM );

        return;
    }

    //
    // bayonet attack
    //  -- instant stab --
    if ( /* pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) && */
        pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_BAYONET ) &&
        ( pm->ps->weapon == WP_M4 || pm->ps->weapon == WP_AK47 ) &&
        !( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) ) &&
        ( pm->cmd.buttons & BUTTON_WEAPON3 )
    )
    {
        if ( pm->ps->pm_flags & PMF_SHOT_LOCKED )
        {
            pm->ps->weaponstate = WEAPON_READY;
            pm->ps->weaponTime = 0;
            return;
        }

        pm->ps->weaponstate = WEAPON_MELEE;
        pm->ps->weaponTime = 400;

        PM_StartTorsoAnim( TORSO_ATTACK_MELEE );

        pm->ps->pm_flags |= PMF_SHOT_LOCKED;
        return;
    }

    // check for fire
    if (!(pm->cmd.buttons & 1) ) {

        pm->ps->weaponTime = 0;

        pm->ps->weaponstate = WEAPON_READY;

        // remove flag
        if ( pm->ps->pm_flags & PMF_SHOT_LOCKED )
            pm->ps->pm_flags &= ~PMF_SHOT_LOCKED;
        return;
    }

    // in water no weapons
    if ( pm->waterlevel > 2 && !BG_IsMelee( pm->ps->weapon ) && !BG_IsGrenade( pm->ps->weapon ) )
    {
        pm->ps->weaponTime = 0;
        pm->ps->weaponstate = WEAPON_READY;
        return;
    }

    // single shot mode
    if ( pm->ps->pm_flags & PMF_SHOT_LOCKED ) {
        pm->ps->weaponstate = WEAPON_READY;
        pm->ps->weaponTime = 0;
        return;
    }


    // ss-mode end
    if ( pm->ps->weapon <= WP_NONE )
        return;

    if ( pm->ps->weapon == WP_C4 && !( pm->ps->pm_flags & PMF_BOMBRANGE) )
        return;

    if ( pm->ps->eFlags & EF_WEAPONS_LOCKED)
        return;


    // start the animation even if out of ammo
    if ( BG_IsMelee( pm->ps->weapon ) ) {
        // we hit anything !!!
        int anim;
        float rnd;

        PM_Seed( );

        rnd = Q_crandom( &pm->ps->stats[STAT_SEED] );

        if (rnd <= 0.33)
        {
            anim = TORSO_ATTACK_MELEE;//TORSO_MEELE_ATTACK1;
            pm->ps->weaponstate = WEAPON_FIRING;
        }
        else if (rnd <= 0.66)
        {
            anim = TORSO_ATTACK_MELEE;//TORSO_MEELE_ATTACK2;
            pm->ps->weaponstate = WEAPON_FIRING2;
        }
        else {
            anim = TORSO_ATTACK_MELEE;//TORSO_MEELE_ATTACK3;
            pm->ps->weaponstate = WEAPON_FIRING3;
        }
        pm->ps->weaponTime = 380;
        PM_StartTorsoAnim( anim );
        PM_AddEvent( EV_FIRE_WEAPON );
        pm->ps->pm_flags |= PMF_SHOT_LOCKED;
        return;
    }

    // Set correct animation
    if ( pm->ps->powerups[PW_BRIEFCASE] )
        PM_StartTorsoAnim( TORSO_ATTACK_SUITCASE );
    else if ( ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM2X ) ) ||
              ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM4X ) ) &&
              ( BG_IsRifle( pm->ps->weapon ) )
            )
        PM_StartTorsoAnim( TORSO_ATTACK_RIFLE_SCOPED );
    else if (
        ( pm->ps->weapon == WP_M4 || pm->ps->weapon == WP_AK47 ||
          pm->ps->weapon == WP_MP5 || pm->ps->weapon == WP_M14 ) &&
        !(pm->cmd.buttons & BUTTON_SPRINT && pm->ps->stats[STAT_STAMINA] > 0 )
    )
        PM_StartTorsoAnim( TORSO_ATTACK_RIFLE_SCOPED );
    else if ( BG_IsPistol( pm->ps->weapon ) || pm->ps->weapon == WP_PDW || pm->ps->weapon == WP_MAC10 )
        PM_StartTorsoAnim( TORSO_ATTACK_PISTOL );
    else if ( BG_IsRifle( pm->ps->weapon ) )
        PM_StartTorsoAnim( TORSO_ATTACK_RIFLE );
    else if ( pm->ps->weapon == WP_C4 ) {
        PM_StartTorsoAnim( TORSO_USE );
        pm->ps->weaponTime = 4120;
        pm->ps->weaponstate = WEAPON_FIRING;
        return;
    } else if ( pm->ps->weapon == WP_GRENADE || pm->ps->weapon == WP_FLASHBANG
                || pm->ps->weapon == WP_SMOKE ) {
        int at = AM_GRENADES;

        if ( pm->ps->weapon == WP_FLASHBANG )
            at = AM_FLASHBANGS;
        else if ( pm->ps->weapon == WP_SMOKE )
            at = AM_SMOKE;

        if ( pm->ps->ammo[at] <= 0 )
            return;

        pm->ps->weaponTime = 480;
        pm->ps->weaponstate = WEAPON_FIRING; // go to pin pull
        return;
    }
    else
        PM_StartTorsoAnim( TORSO_ATTACK_RIFLE );

    // check for out of ammo
    if ( pm->ps->stats[STAT_ROUNDS] <= 0) {
        PM_AddEvent( EV_NOAMMO );
        pm->ps->weaponTime = 150;
        // jump to idle , since we haven't got a fire empty
        pm->ps->weaponstate = WEAPON_READY;
        return;
    } 	// fire weapon normally

    // last round case :
    if ( pm->ps->stats[STAT_ROUNDS] == 1 && ( pm->ps->weapon == WP_P9S || pm->ps->weapon == WP_SW40T || pm->ps->weapon == WP_MK23 || pm->ps->weapon == WP_GLOCK || pm->ps->weapon == WP_MP5 || pm->ps->weapon == WP_DEAGLE ) )
        pm->ps->weaponstate = WEAPON_LASTRND; // go to lastrnd
    else
        pm->ps->weaponstate = WEAPON_FIRING; // fire normally

    pm->ps->weaponTime = PM_GetWeaponTime(pm->ps->weapon );

    // fire grenadelauncher time
    if ( BG_IsInGLMode( pm->ps->stats[STAT_WEAPONMODE] ) )
    {
        pm->ps->weaponstate = WEAPON_FIRING;
        pm->ps->weaponTime = 250;

        PM_AddEvent( EV_FIRE_WEAPON_OTHER );
        return;
    }

    PM_Seed( );
    PM_AddEvent( EV_FIRE_WEAPON );

    //	Com_Printf("animation set\n");
    if ( pm->ps->weapon == WP_MAC10 ||
            pm->ps->weapon == WP_M249 || pm->ps->weapon == WP_M14 || pm->ps->weapon == WP_M4 || pm->ps->weapon == WP_PDW ||
#ifdef SL8SD
            pm->ps->weapon == WP_SL8SD ||
#endif
            pm->ps->weapon == WP_PSG1 || pm->ps->weapon == WP_SPAS15 || BG_IsPistol( pm->ps->weapon ) )
    {
        float rnd = Q_crandom( &pm->ps->stats[STAT_SEED] );

        if (rnd <= 0.33)
            pm->ps->weaponstate = WEAPON_FIRING;
        else if ( (rnd <= 0.66 || BG_IsPistol( pm->ps->weapon ) ) && ( pm->ps->weapon != WP_PSG1 && pm->ps->weapon != WP_SL8SD ) )
            pm->ps->weaponstate = WEAPON_FIRING2;
        else
            pm->ps->weaponstate = WEAPON_FIRING3;
    }

    // now lock our weapon to semimode.
    //
    // MUST BE SAME AS IN G_SEALS.C ; NS_BackupWeaponAim
    if ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SINGLE ) || BG_IsSemiAutomatic( pm->ps->weapon ) )
        pm->ps->pm_flags |= PMF_SHOT_LOCKED;
    // Navy Seals --
}
#if 0
/*
================
PM_Animate
================
*/

static void PM_Animate( void ) {
    // Navy Seals ++
    /*
    if ( pm->cmd.buttons & BUTTON_GESTURE ) {
    if ( pm->ps->torsoTimer == 0 ) {
    PM_StartTorsoAnim( TORSO_GESTURE );
    pm->ps->torsoTimer = TIMER_GESTURE;
    PM_AddEvent( EV_TAUNT );
    }
    #ifdef MISSIONPACK
    } else if ( pm->cmd.buttons & BUTTON_GETFLAG ) {
    if ( pm->ps->torsoTimer == 0 ) {
    PM_StartTorsoAnim( TORSO_GETFLAG );
    pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
    }
    } else if ( pm->cmd.buttons & BUTTON_GUARDBASE ) {
    if ( pm->ps->torsoTimer == 0 ) {
    PM_StartTorsoAnim( TORSO_GUARDBASE );
    pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
    }
    } else if ( pm->cmd.buttons & BUTTON_PATROL ) {
    if ( pm->ps->torsoTimer == 0 ) {
    PM_StartTorsoAnim( TORSO_PATROL );
    pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
    }
    } else if ( pm->cmd.buttons & BUTTON_FOLLOWME ) {
    if ( pm->ps->torsoTimer == 0 ) {
    PM_StartTorsoAnim( TORSO_FOLLOWME );
    pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
    }
    } else if ( pm->cmd.buttons & BUTTON_AFFIRMATIVE ) {
    if ( pm->ps->torsoTimer == 0 ) {
    PM_StartTorsoAnim( TORSO_AFFIRMATIVE);
    pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
    }
    } else if ( pm->cmd.buttons & BUTTON_NEGATIVE ) {
    if ( pm->ps->torsoTimer == 0 ) {
    PM_StartTorsoAnim( TORSO_NEGATIVE );
    pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
    }
    #endif
    }*/
    // Navy Seals --
}

#endif

/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
    // drop misc timing counter
    if ( pm->ps->pm_time ) {
        if ( pml.msec >= pm->ps->pm_time ) {
            pm->ps->pm_flags &= ~PMF_ALL_TIMES;
            pm->ps->pm_time = 0;
        } else {
            pm->ps->pm_time -= pml.msec;
        }
    }

    // drop animation counter
    if ( pm->ps->legsTimer > 0 ) {
        pm->ps->legsTimer -= pml.msec;
        if ( pm->ps->legsTimer < 0 ) {
            pm->ps->legsTimer = 0;
        }
    }

    if ( pm->ps->torsoTimer > 0 ) {
        pm->ps->torsoTimer -= pml.msec;
        if ( pm->ps->torsoTimer < 0 ) {
            pm->ps->torsoTimer = 0;
        }
    }
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd ) {
    short		temp;
    int		i;

    if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
        return;		// no view changes at all
    }
    if (ps->pm_type == PM_SPECTATOR  && ( ps->eFlags & EF_TELEPORT_BIT)  ) {
        return;
    }
    if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 ) {
        return;		// no view changes at all
    }

    // circularly clamp the angles with deltas
    for (i=0 ; i<3 ; i++) {
        temp = cmd->angles[i] + ps->delta_angles[i];
        if ( i == PITCH ) {
            // don't let the player look up or down more than 90 degrees
            if ( temp > 16000 ) {
                ps->delta_angles[i] = 16000 - cmd->angles[i];
                temp = 16000;
            } else if ( temp < -16000 ) {
                ps->delta_angles[i] = -16000 - cmd->angles[i];
                temp = -16000;
            }
        }
        ps->viewangles[i] = SHORT2ANGLE(temp);
    }

}
#if 0
/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
    int		i;
    vec3_t	wishvel;
    float	wishspeed;
    vec3_t	wishdir;
    float	scale;

    // normal slowdown
    PM_Friction ();

    scale = PM_CmdScale( &pm->cmd );
    //
    // user intentions
    //
    if ( !scale ) {
        wishvel[0] = 0;
        wishvel[1] = 0;
        wishvel[2] = 0;
    } else {
        for (i=0 ; i<3 ; i++) {
            wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
        }

        wishvel[2] += scale * pm->cmd.upmove;
    }

    VectorCopy (wishvel, wishdir);
    wishspeed = VectorNormalize(wishdir);

    PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

    PM_StepSlideMove( qfalse );
}
#endif

/*
================
PmoveSingle

================
*/
void trap_SnapVector( float *v );



void PmoveSingle (pmove_t *pmove) {

    pm = pmove;

    // this counter lets us debug movement problems with a journal
    // by setting a conditional breakpoint fot the previous frame
    c_pmove++;

    // clear results
    pm->numtouch = 0;
    pm->watertype = 0;
    pm->waterlevel = 0;

    if ( pm->ps->stats[STAT_HEALTH] <= 0 ) {
        pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
    }

    // make sure walking button is clear if they are running, to avoid
    // proxy no-footsteps cheats
    if ( abs( pm->cmd.forwardmove ) > 64 || abs( pm->cmd.rightmove ) > 64 ) {
        pm->cmd.buttons &= ~BUTTON_WALKING;
    }

    // set the talk balloon flag
    if ( pm->cmd.buttons & BUTTON_TALK ) {
        pm->ps->eFlags |= EF_TALK;
    } else {
        pm->ps->eFlags &= ~EF_TALK;
    }



    // set the firing flag for continuous beam weapons
    if ( !(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION
            && ( pm->cmd.buttons & BUTTON_ATTACK )   ) {
        pm->ps->eFlags |= EF_FIRING;
    } else {
        pm->ps->eFlags &= ~EF_FIRING;
    }


    if ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) && ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_FLASHLIGHT ) ) &&
            (pm->ps->weaponstate == WEAPON_READY || pm->ps->weaponstate == WEAPON_FIRING || pm->ps->weaponstate == WEAPON_FIRING2 || pm->ps->weaponstate == WEAPON_FIRING3 ) )
        pm->ps->eFlags |= EF_WEAPONMODE3;
    else
        pm->ps->eFlags &=~ EF_WEAPONMODE3;

    if ( pm->cmd.buttons & BUTTON_IRONSIGHT &&
            !(pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 )) &&
            !(pm->ps->weaponstate == WEAPON_DROPPING) &&
            !(pm->ps->weaponstate == WEAPON_RAISING) &&
            !(pm->ps->weaponstate == WEAPON_RELOADING || pm->ps->weaponstate == WEAPON_RELOADING_EMPTY)
       )
        pm->ps->eFlags |= EF_IRONSIGHT;
    else
        pm->ps->eFlags &= ~EF_IRONSIGHT;

    // Navy Seals ++ : Silenced weapon?
    if ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_SILENCER ) )
        pm->ps->eFlags |= EF_SILENCED;
    else
        pm->ps->eFlags &= ~EF_SILENCED;

    if ( pm->ps->stats[STAT_WEAPONMODE] & ( 1 << WM_LACTIVE )  && (pm->ps->weaponstate == WEAPON_READY || pm->ps->weaponstate == WEAPON_FIRING ) )
        pm->ps->eFlags |= EF_LASERSIGHT;
    else
        pm->ps->eFlags &= ~EF_LASERSIGHT;


    // clear the respawned flag if attack and use are cleared
    if ( pm->ps->stats[STAT_HEALTH] > 0 &&
            !( pm->cmd.buttons & (BUTTON_ATTACK  ) ) ) {
        pm->ps->pm_flags &= ~PMF_RESPAWNED;
    }

    // if talk button is down, dissallow all other input
    // this is to prevent any possible intercept proxy from
    // adding fake talk balloons
    if ( pmove->cmd.buttons & BUTTON_TALK ) {
        // keep the talk button set tho for when the cmd.serverTime > 66 msec
        // and the same cmd is used multiple times in Pmove
        pmove->cmd.buttons = BUTTON_TALK;
        pmove->cmd.forwardmove = 0;
        pmove->cmd.rightmove = 0;
        pmove->cmd.upmove = 0;
    }

    // clear all pmove local vars
    memset (&pml, 0, sizeof(pml));

    // determine the time
    pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
    if ( pml.msec < 1 ) {
        pml.msec = 1;
    } else if ( pml.msec > 200 ) {
        pml.msec = 200;
    }
    pm->ps->commandTime = pmove->cmd.serverTime;


    // save old org for crashlanding
    if (pml.groundTrace.fraction < 1.0) {
        VectorCopy (pm->ps->origin, pml.previous_origin);
    }

    // save old velocity for crashlanding
    VectorCopy (pm->ps->velocity, pml.previous_velocity);

    pml.frametime = pml.msec * 0.001;

    // update the viewangles
    PM_UpdateViewAngles( pm->ps, &pm->cmd );

    AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

    if ( pm->cmd.upmove < 10 ) {
        // not holding jump
        pm->ps->pm_flags &= ~PMF_JUMP_HELD;
    }

    // decide if backpedaling animations should be used
    if ( pm->cmd.forwardmove < 0 ) {
        pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
    } else if ( pm->cmd.forwardmove > 0 || ( pm->cmd.forwardmove == 0 && pm->cmd.rightmove ) ) {
        pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
    }


    if ( pm->ps->pm_type == PM_SPECTATOR ) {
        //		G_Printf("handling spectator.\n");
        //		if ( pm->ps->eFlags & EF_TELEPORT_BIT )
        //			return;

        PM_CheckDuck ();
        PM_NoclipMove();//
        // 		PM_FlyMove ();
        PM_DropTimers ();
        return;
    }

    if ( pm->ps->pm_type == PM_NOCLIP ) {
        PM_NoclipMove();
        PM_CheckDuck ();
        PM_NoclipMove();//
        // 		PM_FlyMove ();
        PM_DropTimers ();
        return;
    }

    // defusing

    if ( pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION) {
        return;		// no movement at all
    }

    // set watertype, and waterlevel
    PM_SetWaterLevel();
    pml.previous_waterlevel = pmove->waterlevel;

    // set mins, maxs, and viewheight
    PM_CheckDuck ();

    // set groundentity
    PM_GroundTrace();

    if ( pm->ps->pm_type == PM_DEAD ) {
        pm->cmd.upmove = pm->cmd.rightmove = pm->cmd.forwardmove = 0;
        PM_DeadMove ();
        //		return;
    }

    PM_DropTimers();

    if ( NS_OnLadder() == 0 )
        pm->ps->pm_flags &= ~PMF_CLIMB;

    if ( pm->ps->pm_flags & PMF_BOMBCASE || pm->ps->pm_type == PM_FREEZE ) {
        // no movement
        pm->cmd.forwardmove = 0;
        pm->cmd.rightmove = 0;
        pm->cmd.upmove = 0;

        if ( pm->ps->pm_type == PM_FREEZE )
        {
            pm->ps->torsoAnim = TORSO_STAND_RIFLE_SCOPED;
            pm->ps->legsAnim = LEGS_IDLE;
        }
    }/* else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
     {		PM_GrappleMove();
     // We can wiggle a bit
     PM_AirMove();
     }*/  else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
        PM_WaterJumpMove();
    } else if ( NS_OnLadder () != 0) {
        PM_LadderMove();
        pm->ps->pm_flags |= PMF_CLIMB;
    } else if ( pm->waterlevel > 1 ) {
        // swimming
        PM_WaterMove();
    } else if ( pml.walking ) {
        // walking on ground
        PM_WalkMove();
    } else {
        // airborne
        PM_AirMove();
    }

    //	PM_Animate();

    // set groundentity, watertype, and waterlevel
    PM_GroundTrace();
    PM_SetWaterLevel();

    // weapons
    if ( NS_OnLadder () == 0 )
        PM_Weapon();

    // torso animation
    PM_TorsoAnimation();

    // footstep events / legs animations
    PM_Footsteps();

    // entering / leaving water splashes
    PM_WaterEvents();

    // snap some parts of playerstate to save network bandwidth
    trap_SnapVector( pm->ps->velocity );
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
    int			finalTime;

    finalTime = pmove->cmd.serverTime;

    if ( finalTime < pmove->ps->commandTime ) {
        return;	// should not happen
    }

    if ( finalTime > pmove->ps->commandTime + 1000 ) {
        pmove->ps->commandTime = finalTime - 1000;
    }

    pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

    // chop the move up if it is too long, to prevent framerate
    // dependent behavior
    while ( pmove->ps->commandTime != finalTime ) {
        int		msec;

        msec = finalTime - pmove->ps->commandTime;

        if ( pmove->pmove_fixed ) {
            if ( msec > pmove->pmove_msec ) {
                msec = pmove->pmove_msec;
            }
        }
        else {
            if ( msec > 66 ) {
                msec = 66;
            }
        }
        pmove->cmd.serverTime = pmove->ps->commandTime + msec;
        PmoveSingle( pmove );


        if ( pmove->ps->pm_flags & PMF_JUMP_HELD &&
                pmove->ps->stats[STAT_STAMINA] >= 25 ) {
            pmove->cmd.upmove = 20;
        }
    }

    //PM_CheckStuck();

}

