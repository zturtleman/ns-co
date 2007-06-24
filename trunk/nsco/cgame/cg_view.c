// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"
#include "../../ui/menudef.h"


void CG_ForceCvar( const char *cvar, int value );

/*
=============================================================================

MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
    vec3_t		angles;

    memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
    if ( trap_Argc() < 2 ) {
        return;
    }

    Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
    cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );

    if ( trap_Argc() == 3 ) {
        cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
        cg.testModelEntity.frame = 1;
        cg.testModelEntity.oldframe = 0;
    }
    if (! cg.testModelEntity.hModel ) {
        CG_Printf( "Can't register model\n" );
        return;
    }

    VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

    angles[PITCH] = 0;
    angles[YAW] = 180 + cg.refdefViewAngles[1];
    angles[ROLL] = 0;

    AnglesToAxis( angles, cg.testModelEntity.axis );
    cg.testGun = qfalse;
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
    CG_TestModel_f();
    cg.testGun = qtrue;
    cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f (void) {
    cg.testModelEntity.frame++;
    CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
    cg.testModelEntity.frame--;
    if ( cg.testModelEntity.frame < 0 ) {
        cg.testModelEntity.frame = 0;
    }
    CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
    cg.testModelEntity.skinNum++;
    CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
    cg.testModelEntity.skinNum--;
    if ( cg.testModelEntity.skinNum < 0 ) {
        cg.testModelEntity.skinNum = 0;
    }
    CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

static void CG_AddTestModel (void) {
    int		i;

    // re-register the model, because the level may have changed
    cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );
    if (! cg.testModelEntity.hModel ) {
        CG_Printf ("Can't register model\n");
        return;
    }

    // if testing a gun, set the origin reletive to the view origin
    if ( cg.testGun ) {
        VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
        VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
        VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
        VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

        // allow the position to be adjusted
        for (i=0 ; i<3 ; i++) {
            cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
            cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
            cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
        }
    }

    trap_R_AddRefEntityToScene( &cg.testModelEntity );
}



//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {
    int		size;

    // the intermission should allways be full screen
    if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
        size = 100;
    } else {
        // bound normal viewsize
        if (cg_viewsize.integer < 30) {
            trap_Cvar_Set ("cg_viewsize","30");
            size = 30;

        } else if (cg_viewsize.integer > 100) {
            trap_Cvar_Set ("cg_viewsize","100");
            size = 100;
        } else {
            size = cg_viewsize.integer;
        }

    }


    cg.refdef.width = cgs.glconfig.vidWidth*size/100;
    cg.refdef.width &= ~1;

    cg.refdef.height = cgs.glconfig.vidHeight*size/100;
    cg.refdef.height &= ~1;

    cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
    cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512

#define MIN_CAMERA_ZOOM		50
#define MAX_CAMERA_ZOOM		200
#define STEP_CAMERA_ZOOM	2

int CG_FollowCycle( int dir );
qboolean CG_ButtonPushed( int button );
qboolean CG_LastButtonPushed( int button );

static void CG_OffsetThirdPersonView( void ) {
    vec3_t		forward, right, up;
    vec3_t		view;
    vec3_t		focusAngles;
    trace_t		trace;
    static vec3_t	mins = { -3, -3, -3 };
    static vec3_t	maxs = { 3, 3, 3 };
    vec3_t		focusPoint;
    float		focusDist;
    float		forwardScale, sideScale;

    if ( cg.cameraActive && !cg.cameraSpectator )
    {
        VectorCopy( cg.cameraOrigin , cg.refdef.vieworg );

        // turn off camera
        if ( cg.cameraRemainTime < cg.time )
        {
            cg.cameraActive = qfalse;
            cg.cameraFollowNumber = 0;
            // so we won't run into bugs
            VectorClear( cg.cameraOrigin );
        }
    }
    else
    {
        //		if ( cg.snap->ps.pm_flags & PMF_FOLLOW )
        //			VectorCopy( cg.cameraOrigin , cg.refdef.vieworg );
    }


    VectorCopy( cg.predictedPlayerState.viewangles, focusAngles );

    /*
    focusAngles[PITCH] = 25;
    focusAngles[YAW] = focusAngles[ROLL] = 0;
    */

    // if dead, look at killer
    /*	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
    focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
    cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
    }
    */
    if ( focusAngles[PITCH] > 45 ) {
        focusAngles[PITCH] = 45;		// don't go too far overhead
    }
    //	if ( !(cg.predictedPlayerState.pm_flags & PMF_FOLLOW) )
    //	cg.refdef.vieworg[2] += 20;

    AngleVectors( focusAngles, forward, NULL, NULL );

    VectorMA( cg.refdef.vieworg, FOCUS_DISTANCE, forward, focusPoint );
    cg.refdef.vieworg[2] += 20;

    VectorCopy( cg.refdef.vieworg, view );

    //	view[2] += 8;

    //	cg.refdefViewAngles[PITCH] *= 0.5;

    AngleVectors( cg.predictedPlayerState.viewangles, forward, right, up );

    if ( cg.snap->ps.pm_flags & PMF_FOLLOW )
    {
        // camera handling:
        usercmd_t	cmd;
        int			cmdNum;

        cmdNum = trap_GetCurrentCmdNumber();
        trap_GetUserCmd( cmdNum, &cmd );

        if ( cmd.forwardmove < 0 )
        {
            cg.cameraZoom += STEP_CAMERA_ZOOM;

        }
        else if ( cmd.forwardmove > 0 )
        {
            cg.cameraZoom -= STEP_CAMERA_ZOOM;

        }

        if ( cg.cameraZoom < MIN_CAMERA_ZOOM )
            cg.cameraZoom = MIN_CAMERA_ZOOM;
        if ( cg.cameraZoom > MAX_CAMERA_ZOOM )
            cg.cameraZoom = MAX_CAMERA_ZOOM;

        if ( cmd.rightmove < 0 )
        {
            cg.cameraAngle += STEP_CAMERA_ZOOM;

            if ( cg.cameraAngle > 360 )
                cg.cameraAngle -= 360;
        }
        else if ( cmd.rightmove > 0 )
        {
            cg.cameraAngle -= STEP_CAMERA_ZOOM;
        }

        if ( cg.cameraAngle < -360 )
            cg.cameraAngle += 360;

        forwardScale = cos( cg.cameraAngle / 180 * M_PI );
        sideScale = sin( cg.cameraAngle / 180 * M_PI );
        VectorMA( view, -cg.cameraZoom * forwardScale, forward, view );
        VectorMA( view, -cg.cameraZoom * sideScale, right, view );
    }
    else if ( cg.deathCam )
    {
        if ( cg.deathZoom < 80 )
            cg.deathZoom += (cg.time - cg.oldTime)/12;

        cg.deathRotation += (cg.time - cg.oldTime)/16;

        forwardScale = cos( (float)cg.deathRotation / 180 * M_PI );
        sideScale = sin( (float)cg.deathRotation / 180 * M_PI );
        VectorMA( view, (float)-cg.deathZoom * forwardScale, forward, view );

        VectorCopy( cg.refdef.vieworg, focusPoint );

        view[2] += (float)cg.deathZoom / 4;
        focusPoint[2] -= 30;

        VectorMA( view, (float)-cg.deathZoom * sideScale, right, view );
    }
    else
    {
        forwardScale = cos( cg_thirdPersonAngle.value / 180 * M_PI );
        sideScale = sin( cg_thirdPersonAngle.value / 180 * M_PI );
        VectorMA( view, -cg_thirdPersonRange.value * forwardScale, forward, view );
        VectorMA( view, -cg_thirdPersonRange.value * sideScale, right, view );
    }

    // trace a ray from the origin to the viewpoint to make sure the view isn't
    // in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

    if (!cg_cameraMode.integer) {
        CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

        if ( trace.fraction != 1.0 ) {
            VectorCopy( trace.endpos, view );
            view[2] += (1.0 - trace.fraction) * 32;
            // try another trace to this position, because a tunnel may have the ceiling
            // close enogh that this is poking out

            CG_Trace( &trace, cg.refdef.vieworg, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
            VectorCopy( trace.endpos, view );
        }
    }


    VectorCopy( view, cg.refdef.vieworg );

    // select pitch to look at focus point from vieword
    VectorSubtract( focusPoint, cg.refdef.vieworg, focusPoint );
    focusDist = sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
    if ( focusDist < 1 ) {
        focusDist = 1;	// should never happen
    }
    cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2( focusPoint[2], focusDist );

    if ( cg.snap->ps.pm_flags & PMF_FOLLOW )
        cg.refdefViewAngles[YAW] -= cg.cameraAngle;
    else if ( cg.deathCam )
        cg.refdefViewAngles[YAW] -= cg.deathRotation;
    else
        cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void ) {
    int		timeDelta;

    // smooth out stair climbing
    timeDelta = cg.time - cg.stepTime;
    if ( timeDelta < STEP_TIME ) {
        cg.refdef.vieworg[2] -= cg.stepChange
                                * (STEP_TIME - timeDelta) / STEP_TIME;
    }
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
extern vmCvar_t	cg_bobyaw;
extern vmCvar_t	cg_runyaw;

static void CG_OffsetFirstPersonView( void ) {
    float			*origin;
    float			*angles;
    float			bob;
    float			ratio;
    float			delta;
    float			speed;
    float			f;
    vec3_t			predictedVelocity;
    int				timeDelta;

    if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
        return;
    }

    origin = cg.refdef.vieworg;
    angles = cg.refdefViewAngles;

    // if dead, fix the angle and don't add any kick
    /*	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
    angles[ROLL] = 40;
    angles[PITCH] = -15;
    angles[YAW] = cg.snap->ps.stats[STAT_DEAD_YAW];
    origin[2] += cg.predictedPlayerState.viewheight;
    return;
    }*/

    // add angles based on weapon kick
    VectorAdd (angles, cg.kick_angles, angles);


    // add pitch based on fall kick
#if 0
    ratio = ( cg.time - cg.landTime) / FALL_TIME;
    if (ratio < 0)
        ratio = 0;
    angles[PITCH] += ratio * cg.fall_value;
#endif

    // add angles based on velocity
    VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

    delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
    angles[PITCH] += delta * cg_runpitch.value;

    delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
    angles[ROLL] -= delta * cg_runroll.value;

    delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
    angles[YAW] -= delta * cg_runyaw.value;

    // make sure the bob is visible even at low speeds
    speed = cg.xyspeed > 180 ? cg.xyspeed : 180;

    delta = cg.bobfracsin * cg_bobpitch.value * speed;
    if (cg.predictedPlayerState.pm_flags & PMF_DUCKED ||
            BG_IsZooming( cg.predictedPlayerState.stats[STAT_WEAPONMODE] ) )
        delta *= 2;		// crouching
    angles[PITCH] += delta;

    delta = cg.bobfracsin * cg_bobroll.value * speed; // 0.0035

    // sprinting
    if ( CG_ButtonPushed( BUTTON_SPRINT ) && cg.predictedPlayerState.stats[STAT_STAMINA] > 0 &&
            cg.predictedPlayerState.pm_type == PM_NORMAL &&
            !(cg.predictedPlayerState.pm_flags & PMF_DUCKED) )
        delta = cg.bobfracsin * cg_bobroll.value * speed; // 0.01
    else if (cg.predictedPlayerState.pm_flags & PMF_DUCKED ||
             BG_IsZooming( cg.predictedPlayerState.stats[STAT_WEAPONMODE] ) )
        delta *= 2;		// crouching accentuates roll

    if (cg.bobcycle & 1)
        delta = -delta;
    angles[ROLL] += delta;


    // yaw

    delta = cg.bobfracsin * cg_bobyaw.value * speed; // 0.0035

    // sprinting
    if ( CG_ButtonPushed( BUTTON_SPRINT ) && cg.predictedPlayerState.stats[STAT_STAMINA] > 0 &&
            cg.predictedPlayerState.pm_type == PM_NORMAL &&
            !(cg.predictedPlayerState.pm_flags & PMF_DUCKED) )
        delta = cg.bobfracsin * cg_bobyaw.value * speed; // 0.01
    else if (cg.predictedPlayerState.pm_flags & PMF_DUCKED ||
             BG_IsZooming( cg.predictedPlayerState.stats[STAT_WEAPONMODE] ) )
        delta *= 2;		// crouching accentuates roll

    if (cg.bobcycle & 1)
        delta = -delta;
    angles[YAW] += delta;

    //===================================

    // add view height
    origin[2] += cg.predictedPlayerState.viewheight;

    // smooth out duck height changes
    timeDelta = cg.time - cg.duckTime;
    if ( timeDelta < DUCK_TIME) {
        cg.refdef.vieworg[2] -= cg.duckChange
                                * (DUCK_TIME - timeDelta) / DUCK_TIME;
    }

    // add bob height
    bob = cg.bobfracsin * speed * cg_bobup.value;
    if (bob > 6) {
        bob = 6;
    }

    origin[2] += bob;


    // add fall height
    delta = cg.time - cg.landTime;
    if ( delta < LAND_DEFLECT_TIME ) {
        f = delta / LAND_DEFLECT_TIME;
        cg.refdef.vieworg[2] += cg.landChange * f;
    } else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
        delta -= LAND_DEFLECT_TIME;
        f = 1.0 - ( delta / LAND_RETURN_TIME );
        cg.refdef.vieworg[2] += cg.landChange * f;
    }

    // add step offset
    CG_StepOffset();

    // add kick offset

    VectorAdd (origin, cg.kick_origin, origin);

    // pivot the eye based on a neck length
    {
        extern vmCvar_t cg_viewHeight;

        cg.refdef.vieworg[2] += cg_viewHeight.value;
    }
    VectorCopy( angles, cg.weaponAngles );
    // add angles based on damage kick
    if ( cg.damageTime ) {
        int returntime = ( cg.damageDuration / 5 ) * 4;
        int time = cg.damageDuration;
        int deflecttime = ( cg.damageDuration / 5 );

        ratio = cg.time - cg.damageTime;

        if ( ratio < deflecttime ) {
            ratio /= deflecttime;
            angles[PITCH] += ratio * cg.v_dmg_pitch;
            angles[ROLL] += ratio * cg.v_dmg_roll;
        } else {
            ratio = 1.0 - ( ratio - deflecttime ) / returntime;
            if ( ratio > 0 ) {
                angles[PITCH] += ratio * cg.v_dmg_pitch;
                angles[ROLL] += ratio * cg.v_dmg_roll;
            }
        }
    }

#if 0
    {
#define	NECK_LENGTH		8
        vec3_t			forward, up;

        cg.refdef.vieworg[2] -= NECK_LENGTH;
        AngleVectors( cg.refdefViewAngles, forward, NULL, up );
        VectorMA( cg.refdef.vieworg, 3, forward, cg.refdef.vieworg );
        VectorMA( cg.refdef.vieworg, NECK_LENGTH, up, cg.refdef.vieworg );
    }
#endif
}

//======================================================================


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov( void ) {
    float	x;
    float	phase;
    float	v;
    int		contents;
    float	fov_x, fov_y;
    float	zoomFov;
    float	f;
    int		inwater;
    int		weaponstate = cg.predictedPlayerState.weaponstate;

    if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION || cg.cameraActive ) {
        // if in intermission, use a fixed value
        fov_x = 75;
    } else {
        // user selectable
        if ( cgs.dmflags & DF_FIXED_FOV ) {
            // dmflag to prevent wide fov for all clients
            fov_x = 75;
        } else {
            fov_x = cg_fov.value;
            if ( fov_x < 1 ) {
                fov_x = 1;
            } else if ( fov_x > 160 ) {
                fov_x = 160;
            }
        }

        // account for zooms
        zoomFov = 75/8;
        if ( zoomFov < 1 ) {
            zoomFov = 1;
        } else if ( zoomFov > 160 ) {
            zoomFov = 160;
        }

        // only change fov when not in 3rd person
        if ( !(cg.renderingThirdPerson) ) {

            if ( ( cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM2X ) ) )
                zoomFov = 75/2;

            if ( (
                        (cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM2X ) ) ||
                        (cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM4X ) ) ) &&
                    (weaponstate == WEAPON_FIRING || weaponstate == WEAPON_FIRING2 || weaponstate == WEAPON_FIRING3 || weaponstate == WEAPON_READY) )
            {
                f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
                if ( f > 1.0 ) {
                    fov_x = zoomFov;
                } else {
                    fov_x = fov_x + f * ( zoomFov - fov_x );
                }
            } else {
                f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
                if ( f > 1.0 ) {
                    fov_x = fov_x;
                } else {
                    fov_x = zoomFov + f * ( fov_x - zoomFov );
                }
            }
        } // if ( !cg.render...
    }

    x = cg.refdef.width / tan( fov_x / 360 * M_PI );
    fov_y = atan2( cg.refdef.height, x );
    fov_y = fov_y * 360 / M_PI;

    //only render headdamage when not in 3rdperson
    if ( !cg.renderingThirdPerson )
    {
        if(cg.snap->ps.stats[STAT_HEAD_DAMAGE] > 15 && cg.snap->ps.stats[STAT_HEALTH] > 0)
        {
            fov_x += 0.5 * cg.snap->ps.stats[STAT_HEAD_DAMAGE] * sin(cg.time / (500.0 - 2 * cg.snap->ps.stats[STAT_HEAD_DAMAGE]));
            if(fov_x < 1)
                fov_x = 1;
            fov_y += 0.5 * cg.snap->ps.stats[STAT_HEAD_DAMAGE] * cos(cg.time / (500.0 - 2 * cg.snap->ps.stats[STAT_HEAD_DAMAGE]));
            if(fov_y < 1)
                fov_y = 1;
        }
    }

    // warp if underwater
    contents = CG_PointContents( cg.refdef.vieworg, -1 );
    if ( contents & CONTENTS_WATER ){
        phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
        v = WAVE_AMPLITUDE * sin( phase );
        fov_x += v;
        fov_y -= v;
        inwater = qtrue;
    }
    else {
        inwater = qfalse;
    }

    // concussion effect
    if ( cg.ConcussionTime > cg.time ) {
        fov_x *=  1.0 + 0.3 *
                  (cg.ConcussionTime - cg.time) / (SEALS_FLASHBANGTIME*SEALS_CONCUSSIONFACTOR) *
                  sin( 3 * 2.0 * M_PI * (cg.ConcussionTime - cg.time) / (SEALS_FLASHBANGTIME*SEALS_CONCUSSIONFACTOR) );
        fov_y *= 1.0 + 0.3 *
                 (cg.ConcussionTime - cg.time) / (SEALS_FLASHBANGTIME) *
                 cos( 3 * 2.0 * M_PI * (cg.ConcussionTime - cg.time) / (SEALS_FLASHBANGTIME*SEALS_CONCUSSIONFACTOR) );
    }

    // set it
    cg.refdef.fov_x = fov_x;
    cg.refdef.fov_y = fov_y;

    //	if ( fov_x < 90 ) {
    //		cg.zoomSensitivity = 1;
    //	} else {
    cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
    //	}

    return inwater;
}

/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
    playerState_t	*ps;

    memset( &cg.refdef, 0, sizeof( cg.refdef ) );

    // strings for in game rendering
    // Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
    // Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

    // calculate size of 3D view
    CG_CalcVrect();

    ps = &cg.predictedPlayerState;

    // intermission view
    if ( ps->pm_type == PM_INTERMISSION ) {
        VectorCopy( ps->origin, cg.refdef.vieworg );
        VectorCopy( ps->viewangles, cg.refdefViewAngles );
        AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
        return CG_CalcFov();
    }

    cg.bobcycle = ( ps->bobCycle & 128 ) >> 7;
    cg.bobfracsin = fabs( sin( ( ps->bobCycle & 127 ) / 127.0 * M_PI ) );
    cg.xyspeed = sqrt( ps->velocity[0] * ps->velocity[0] +
                       ps->velocity[1] * ps->velocity[1] );


    VectorCopy( ps->origin, cg.refdef.vieworg );
    VectorCopy( ps->viewangles, cg.refdefViewAngles );

    if (cg_cameraOrbit.integer) {
        if (cg.time > cg.nextOrbitTime) {
            cg.nextOrbitTime = cg.time + cg_cameraOrbitDelay.integer;
            cg_thirdPersonAngle.value += cg_cameraOrbit.value;
        }
    }

    if(ps->stats[STAT_HEAD_DAMAGE] > 15 && ps->stats[STAT_HEALTH] > 0)
    {
        cg.refdefViewAngles[ROLL] = ps->stats[STAT_HEAD_DAMAGE] * sin(cg.time / (1000.0 - ps->stats[STAT_HEAD_DAMAGE] * 5));
    }

    // concussion effect
    if ( cg.ConcussionTime > cg.time ) {
        cg.refdefViewAngles[ROLL] = 16 * (cg.ConcussionTime - cg.time) / (SEALS_FLASHBANGTIME*SEALS_CONCUSSIONFACTOR) *
                                    sin( 3 * 2.0 * M_PI * (cg.ConcussionTime - cg.time) / (SEALS_FLASHBANGTIME*SEALS_CONCUSSIONFACTOR) );
    }

    // add error decay
    if ( cg_errorDecay.value > 0 ) {
        int		t;
        float	f;

        t = cg.time - cg.predictedErrorTime;
        f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
        if ( f > 0 && f < 1 ) {
            VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
        } else {
            cg.predictedErrorTime = 0;
        }
    }

    if ( cg.renderingThirdPerson ) {
        // back away from character
        CG_OffsetThirdPersonView();
    }
    else if ( cgs.gametype == GT_TEAM && cg.snap->ps.stats[STAT_HEALTH] <= 0 )
    {
        cg.renderingThirdPerson = qtrue;
        cg.deathCam = qtrue;
        CG_OffsetThirdPersonView();
    }
    else if ( cg.cameraSpectator /* || cg.snap->ps.pm_flags & PMF_FOLLOW */ )
    {
        cg.renderingThirdPerson = qtrue;
        cg.cameraSpectator = qtrue;
        CG_OffsetThirdPersonView();
    }
    else if ( cg.cameraActive /* || ( cg.snap->ps.pm_flags & PMF_FOLLOW ) */ ) {
        cg.cameraActive = qtrue;
        cg.cameraOrigin[2] += DEFAULT_VIEWHEIGHT;

        // back away from character
        CG_OffsetThirdPersonView();
    }
    else {
        // offset for local bobbing and kicks
        CG_OffsetFirstPersonView();
    }

    // position eye reletive to origin
    AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

    if ( cg.hyperspace ) {
        cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
    }

    // field of view
    return CG_CalcFov();
}

/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx ) {
    if ( !sfx )
        return;
    cg.soundBuffer[cg.soundBufferIn] = sfx;
    cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
    if (cg.soundBufferIn == cg.soundBufferOut) {
        cg.soundBufferOut++;
    }
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void ) {
    if ( cg.soundTime < cg.time ) {
        if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut]) {
            trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
            cg.soundBuffer[cg.soundBufferOut] = 0;
            cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
            cg.soundTime = cg.time + 1000;
        }
    }
}

/*
===============
CG_WeaponUpdate

Update client weapon with a new weapon.
===============
*/
static void CG_WeaponDrop( int weapon )
{
    cg.weaponSelectTime = cg.time;
    cg.weaponSelect = weapon;
}

//=========================================================================

/*
===============
CG_UpdateCharacterCvars

Updates cvars for UI with information from the server
===============
*/

int CG_GetTotalXPforLevel( int level )
{
    int total=0;
    int i;

    for (i=0;i<=level;i++)
    {
        if ( i > 1 )
            total += i;
    }
    return total;
}
int CG_GetTotalXPforAllLevels(  )
{
    int value = 0;

    value += CG_GetTotalXPforLevel( cg.snap->ps.persistant[PERS_STRENGTH] );
    value += CG_GetTotalXPforLevel( cg.snap->ps.persistant[PERS_TECHNICAL] );
    value += CG_GetTotalXPforLevel( cg.snap->ps.persistant[PERS_STAMINA] );
    value += CG_GetTotalXPforLevel( cg.snap->ps.persistant[PERS_SPEED] );
    value += CG_GetTotalXPforLevel( cg.snap->ps.persistant[PERS_STEALTH] );
    value += CG_GetTotalXPforLevel( cg.snap->ps.persistant[PERS_ACCURACY] );

    return value;
}
static void CG_UpdateCharacterCvars( void )
{
    CG_ForceCvar("char_strength", cg.snap->ps.persistant[PERS_STRENGTH] );
    CG_ForceCvar("char_technical", cg.snap->ps.persistant[PERS_TECHNICAL] );
    CG_ForceCvar("char_stamina", cg.snap->ps.persistant[PERS_STAMINA] );
    CG_ForceCvar("char_speed", cg.snap->ps.persistant[PERS_SPEED] );
    CG_ForceCvar("char_stealth", cg.snap->ps.persistant[PERS_STEALTH] );
    CG_ForceCvar("char_accuracy", cg.snap->ps.persistant[PERS_ACCURACY] );
    CG_ForceCvar("char_xp_total", cg.snap->ps.persistant[PERS_XP] );
    CG_ForceCvar("char_xp", cg.snap->ps.persistant[PERS_XP] - CG_GetTotalXPforAllLevels() );

    // for the hud icons.
    CG_ForceCvar("ui_gotbriefcase", ( cg.snap->ps.powerups[PW_BRIEFCASE] ) );
    CG_ForceCvar("ui_isvip", ( cg.snap->ps.eFlags & EF_VIP ) );
    CG_ForceCvar("ui_gotbomb", BG_GotWeapon ( WP_C4, cg.snap->ps.stats ) );
    CG_ForceCvar("mi_blowup", cgs.mi_bombSpot );
    CG_ForceCvar("mi_viprescue", cgs.mi_vipRescue);
    CG_ForceCvar("mi_viptime", ( cgs.mi_vipTime ) );
    CG_ForceCvar("mi_assaultfield", ( cgs.mi_assaultFields ) );
    CG_ForceCvar("ui_bombstate", ( cg.snap->ps.pm_flags & PMF_BOMBRANGE ) );

    if ( cg.snap->ps.pm_flags & PMF_BLOCKED )
        CG_ForceCvar("ui_assaultstate",2 );
    else if ( cg.snap->ps.pm_flags & PMF_ASSAULTRANGE )
        CG_ForceCvar("ui_assaultstate",1 );
    else
        CG_ForceCvar("ui_assaultstate",0 );

    if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW ) )
    {
        int team = cgs.clientinfo[cg.clientNum].team;

        if ( cgs.gametype >= GT_TEAM ) {
            if ( team == TEAM_RED )
                CG_ForceCvar("ui_team",1 );
            else if ( team == TEAM_BLUE )
                CG_ForceCvar("ui_team",2 );
            else
                CG_ForceCvar("ui_team",0 );
        }
        else {
            if ( team == TEAM_FREE )
                CG_ForceCvar("ui_team", 1 );
            else
                CG_ForceCvar("ui_team", 0 );
        }
    }
}

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/

#define KICKBACK_TIME	250
int CG_GoChase ( );
void CG_QCmd_HandleMenu( void );

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
    int		inwater;

    cg.time = serverTime;
    cg.demoPlayback = demoPlayback;

    if ( cg.smokeBlendAlpha > 0.0f ) // fadeout if smokegrenade stop smoking
        cg.smokeBlendAlpha -= 0.1f;

    // update cvars
    CG_UpdateCvars(); 

    // if we are only updating the screen as a loading
    // pacifier, don't even try to read snapshots
    if ( cg.infoScreenText[0] != 0 ) {
        CG_DrawInformation();
        return;
    }

    // any looped sounds will be respecified as entities
    // are added to the render list
    trap_S_ClearLoopingSounds(qfalse);

    // clear all the render lists
    trap_R_ClearScene();

    // set up cg.snap and possibly cg.nextSnap
    CG_ProcessSnapshots();

    // if we haven't received any snapshots yet, all
    // we can draw is the information screen
    if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
        CG_DrawInformation();
        return;
    }

    // this counter will be bumped for every valid scene we generate
    cg.clientFrame++;

    // update cg.predictedPlayerState
    CG_PredictPlayerState();

    // decide on third person view
    cg.renderingThirdPerson = cg.thirdpersonCamera + cg_thirdPerson.integer;

    // build cg.refdef
    inwater = CG_CalcViewValues();

    // if clientweapon state changed update it
    if ( cg.weaponSelect != cg.snap->ps.weapon && cg.time > cg.weaponSelectTime )
        CG_WeaponDrop( cg.snap->ps.weapon );

    // let the client system know what our weapon and zoom settings are
    trap_SetUserCmdValue( cg.weaponSelect, cg.zoomSensitivity );

    if ( cg.snap->ps.powerups[PW_BRIEFCASE] && BG_IsPrimary( cg.snap->ps.weapon ) )
    {
        int i;
        int wp = WP_NONE;

        for ( i = WP_SMOKE - 1; i > WP_NONE; i-- )
        {
            //if ( i == WP_NUTSHELL ) continue;
            if ( i == WP_C4 ) // no c4
                continue;
            if ( BG_IsPrimary( i ) )
                continue;
            if ( !BG_GotWeapon( i , cg.snap->ps.stats ) )
                continue;

            wp = i;
            break;
        }

        cg.weaponSelect = wp;
        cg.weaponSelectTime = cg.time + 1000;
    }
    if (cg.activeInventory)
    {
        usercmd_t	cmd;
        int			cmdNum;

        cmdNum = trap_GetCurrentCmdNumber();
        trap_GetUserCmd( cmdNum, &cmd );

        if (cg.InventoryTime == 0 && cg_enableTimeSelect.value > 0.0 )
            cg.InventoryTime = cg.time + cg_enableTimeSelect.value*1000;

        if ( ( cg.InventoryTime < cg.time && cg_enableTimeSelect.value > 0.0 ) || ( cmd.buttons & BUTTON_USE ) ) {
            cg.weaponSelectTime = cg.time + 1000;
            CG_InvenSelect ( );
            cg.InventoryTime = 0;
        }
    }
    else if (cg.InventoryTime != 0)
        cg.InventoryTime = 0;

    CG_UpdateCharacterCvars( );

    if (cg.snap->ps.stats[STAT_HEALTH] <= 0)
    {
        cg.WaterTime = 0;
        cg.FlashTime = 0;
        cg.gunSmokeTime = 0;
        cg.DeafTime = 0;

        cg.predictedPlayerEntity.pe.weapAnim = WANIM_IDLE;
        cg.predictedPlayerEntity.pe.nextweapAnim = WANIM_IDLE;
    }

    // build the render lists
    if ( !cg.hyperspace ) {
        CG_AddPacketEntities();			// adter calcViewValues, so predicted player state is correct
        CG_AddMarks();
        CG_AddLocalEntities( qfalse );
        CG_AddAtmosphericEffects();	//add enviroment effects like rain/snow/
    }

#if 0
    if ( cg.predictedPlayerState.eFlags & EF_IRONSIGHT )
    {
        if ( cg.ns_ironsightState == IS_NONE )
        {
            cg.ns_ironsightState = IS_PUTUP;

            if ( BG_IsRifle( cg.snap->ps.weapon ) )
                cg.ns_ironsightTimer = cg.time + IS_RIFLE_TIME;
            else
                cg.ns_ironsightTimer = cg.time + IS_TIME;

            cg.ns_ironsightDeactivate = qfalse;
        }
    }
    else
    {
        if ( cg.ns_ironsightState != IS_NONE && !cg.ns_ironsightDeactivate )
        {
            cg.ns_ironsightDeactivate = qtrue;
        }
    }
#endif

    if ( cg.crosshairMod && cg.crosshairModTime < cg.time )
    {
        cg.crosshairModTime = cg.time + ( KICKBACK_TIME / 4 );
        cg.crosshairMod--;
    }

    if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW ) && cg.cameraSpectator )
        cg.cameraSpectator = qfalse;

    // add buffered sounds
    CG_PlayBufferedSounds();

    // handle newbee msgs (only if activated)
    if ( cg_newbeeTime.value > 0.0f )
        CG_HandleHelp();

    // finish up the rest of the refdef
    if ( cg.testModelEntity.hModel ) {
        CG_AddTestModel();
    }
    cg.refdef.time = cg.time;
    memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

    if ( cg.predictedPlayerState.pm_flags & PMF_FOLLOW )
    {
        if ( CG_ButtonPushed( BUTTON_WEAPON2 ) && !cg.button3pushed )
        {
            if ( cg.thirdpersonCamera )
                cg.thirdpersonCamera = qfalse;
            else
                cg.thirdpersonCamera = qtrue;

            cg.button3pushed = qtrue;
        }
        else if ( !CG_ButtonPushed( BUTTON_WEAPON2 ) )
            cg.button3pushed = qfalse;
    }
    else
        cg.thirdpersonCamera = qfalse;

    // update audio positions
    trap_S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

    // make sure the lagometerSample and frame timing isn't done twice when in stereo
    if ( stereoView != STEREO_RIGHT ) {
        cg.frametime = cg.time - cg.oldTime;
        if ( cg.frametime < 0 ) {
            cg.frametime = 0;
        }
        cg.oldTime = cg.time;
        CG_AddLagometerFrameInfo();
    }
    if (cg_timescale.value != cg_timescaleFadeEnd.value) {
        if (cg_timescale.value < cg_timescaleFadeEnd.value) {
            cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
            if (cg_timescale.value > cg_timescaleFadeEnd.value)
                cg_timescale.value = cg_timescaleFadeEnd.value;
        }
        else {
            cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
            if (cg_timescale.value < cg_timescaleFadeEnd.value)
                cg_timescale.value = cg_timescaleFadeEnd.value;
        }
        if (cg_timescaleFadeSpeed.value) {
            trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
        }
    }
    // actually issue the rendering calls
    CG_DrawActive( stereoView );

    if ( cg_stats.integer ) {
        CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
    }
}

