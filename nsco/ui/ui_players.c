// Copyright (C) 1999-2000 Id Software, Inc.
//
// ui_players.c

#include "ui_local.h"


#define UI_TIMER_GESTURE		2300
#define UI_TIMER_JUMP			1000
#define UI_TIMER_LAND			130
#define UI_TIMER_WEAPON_SWITCH	300
#define UI_TIMER_ATTACK			500
#define	UI_TIMER_MUZZLE_FLASH	20
#define	UI_TIMER_WEAPON_DELAY	250

#define JUMP_HEIGHT				56

#define SWINGSPEED				0.3f

#define SPIN_SPEED				0.9f
#define COAST_TIME				1000


static int			dp_realtime;
static float		jumpHeight;
sfxHandle_t weaponChangeSound;


/*
===============
UI_PlayerInfo_SetWeapon
===============
*/
static void UI_PlayerInfo_SetWeapon( playerInfo_t *pi, weapon_t weaponNum ) {
    gitem_t *	item;
    char		path[MAX_QPATH];

    pi->currentWeapon = weaponNum;
tryagain:
    pi->realWeapon = weaponNum;
    pi->weaponModel = 0;
    pi->barrelModel = 0;
    pi->flashModel = 0;

    if ( weaponNum == WP_NONE ) {
        return;
    }

    for ( item = bg_itemlist + 1; item->classname ; item++ ) {
        if ( item->giType != IT_WEAPON ) {
            continue;
        }
        if ( item->giTag == weaponNum ) {
            break;
        }
    }

    if ( item->classname ) {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_vweap.md3" );
        pi->flashModel = trap_R_RegisterModel( path );
        pi->weaponModel = trap_R_RegisterModel( path );
    }

    if( pi->weaponModel == 0 ) {
        if( weaponNum == WP_MK23 ) {
            weaponNum = WP_NONE;
            goto tryagain;
        }
        weaponNum = WP_MK23;
        goto tryagain;
    }

    strcpy( path, item->world_model[0] );
    COM_StripExtension( path, path );
    strcat( path, "_flash.md3" );
    pi->flashModel = trap_R_RegisterModel( path );

    MAKERGB( pi->flashDlightColor, 0.6f, 0.6f, 1 );
}


/*
===============
UI_ForceLegsAnim
===============
*/
static void UI_ForceLegsAnim( playerInfo_t *pi, int anim ) {
    pi->legsAnim = ( ( pi->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

    if ( anim == LEGS_JUMP ) {
        pi->legsAnimationTimer = UI_TIMER_JUMP;
    }
}


/*
===============
UI_SetLegsAnim
===============
*/
static void UI_SetLegsAnim( playerInfo_t *pi, int anim ) {
    if ( pi->pendingLegsAnim ) {
        anim = pi->pendingLegsAnim;
        pi->pendingLegsAnim = 0;
    }
    UI_ForceLegsAnim( pi, anim );
}


/*
===============
UI_ForceTorsoAnim
===============
*/
static void UI_ForceTorsoAnim( playerInfo_t *pi, int anim ) {
    pi->torsoAnim = ( ( pi->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

    if ( anim == TORSO_GESTURE1 ) {
        pi->torsoAnimationTimer = UI_TIMER_GESTURE;
    }

    if ( anim == TORSO_ATTACK_RIFLE || anim == TORSO_ATTACK_PISTOL || anim == TORSO_ATTACK_MELEE ) {
        pi->torsoAnimationTimer = UI_TIMER_ATTACK;
    }
}


/*
===============
UI_SetTorsoAnim
===============
*/
static void UI_SetTorsoAnim( playerInfo_t *pi, int anim ) {
    if ( pi->pendingTorsoAnim ) {
        anim = pi->pendingTorsoAnim;
        pi->pendingTorsoAnim = 0;
    }

    UI_ForceTorsoAnim( pi, anim );
}


/*
===============
UI_TorsoSequencing
===============
*/
static void UI_TorsoSequencing( playerInfo_t *pi ) {
    int		currentAnim;

    currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

    if ( pi->weapon != pi->currentWeapon ) {
        if ( currentAnim != TORSO_DROP_RIFLE && currentAnim != TORSO_DROP_PISTOL ) {

            pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
            if ( BG_IsPrimary( pi->currentWeapon ) )
                UI_ForceTorsoAnim( pi, TORSO_DROP_RIFLE );
            else
                UI_ForceTorsoAnim( pi, TORSO_DROP_PISTOL );
        }
    }

    if ( pi->torsoAnimationTimer > 0 ) {
        return;
    }

    if( currentAnim == TORSO_GESTURE1 ) {
        UI_SetTorsoAnim( pi, TORSO_STAND_RIFLE );
        return;
    }

    if( currentAnim == TORSO_ATTACK_RIFLE || currentAnim == TORSO_ATTACK_MELEE || currentAnim == TORSO_ATTACK_PISTOL ) {
        UI_SetTorsoAnim( pi, TORSO_STAND_RIFLE );
        return;
    }

    if ( currentAnim == TORSO_DROP_RIFLE ) {
        UI_PlayerInfo_SetWeapon( pi, pi->weapon );
        pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
        UI_ForceTorsoAnim( pi, TORSO_RAISE_RIFLE );
        return;
    }
    if ( currentAnim == TORSO_DROP_PISTOL ) {
        UI_PlayerInfo_SetWeapon( pi, pi->weapon );
        pi->torsoAnimationTimer = UI_TIMER_WEAPON_SWITCH;
        UI_ForceTorsoAnim( pi, TORSO_RAISE_PISTOL );
        return;
    }
    if ( currentAnim == TORSO_RAISE_RIFLE ) {
        UI_SetTorsoAnim( pi, TORSO_STAND_RIFLE );
        return;
    }

    if ( currentAnim == TORSO_RAISE_PISTOL ) {
        UI_SetTorsoAnim( pi, TORSO_STAND_PISTOL );
        return;
    }
}


/*
===============
UI_LegsSequencing
===============
*/
static void UI_LegsSequencing( playerInfo_t *pi ) {
    int		currentAnim;

    currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;

    if ( pi->legsAnimationTimer > 0 ) {
        if ( currentAnim == LEGS_JUMP ) {
            jumpHeight = JUMP_HEIGHT * sin( M_PI * ( UI_TIMER_JUMP - pi->legsAnimationTimer ) / UI_TIMER_JUMP );
        }
        return;
    }

    if ( currentAnim == LEGS_JUMP ) {
        UI_ForceLegsAnim( pi, LEGS_LAND );
        pi->legsAnimationTimer = UI_TIMER_LAND;
        jumpHeight = 0;
        return;
    }

    if ( currentAnim == LEGS_LAND ) {
        UI_SetLegsAnim( pi, LEGS_IDLE );
        return;
    }
}


/*
======================
UI_PositionEntityOnTag
======================
*/
static void UI_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
                                    clipHandle_t parentModel, char *tagName ) {
    int				i;
    orientation_t	lerped;

    // lerp the tag
    trap_CM_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
                     1.0 - parent->backlerp, tagName );

    // FIXME: allow origin offsets along tag?
    VectorCopy( parent->origin, entity->origin );
    for ( i = 0 ; i < 3 ; i++ ) {
        VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
    }

    // cast away const because of compiler problems
    MatrixMultiply( lerped.axis, ((refEntity_t*)parent)->axis, entity->axis );
    entity->backlerp = parent->backlerp;
}


/*
======================
UI_PositionRotatedEntityOnTag
======================
*/
static void UI_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
        clipHandle_t parentModel, char *tagName ) {
    int				i;
    orientation_t	lerped;
    vec3_t			tempAxis[3];

    // lerp the tag
    trap_CM_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
                     1.0 - parent->backlerp, tagName );

    // FIXME: allow origin offsets along tag?
    VectorCopy( parent->origin, entity->origin );
    for ( i = 0 ; i < 3 ; i++ ) {
        VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
    }

    // cast away const because of compiler problems
    MatrixMultiply( entity->axis, ((refEntity_t *)parent)->axis, tempAxis );
    MatrixMultiply( lerped.axis, tempAxis, entity->axis );
}


/*
===============
UI_SetLerpFrameAnimation
===============
*/
static void UI_SetLerpFrameAnimation( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
    animation_t	*anim;

    lf->animationNumber = newAnimation;
    newAnimation &= ~ANIM_TOGGLEBIT;

    if ( newAnimation < 0 || newAnimation >= MAX_ANIMATIONS ) {
        trap_Error( va("Bad animation number: %i", newAnimation) );
    }

    anim = &ci->animations[ newAnimation ];

    lf->animation = anim;
    lf->animationTime = lf->frameTime + anim->initialLerp;
}


/*
===============
UI_RunLerpFrame
===============
*/
static void UI_RunLerpFrame( playerInfo_t *ci, lerpFrame_t *lf, int newAnimation ) {
    int			f;
    animation_t	*anim;

    // see if the animation sequence is switching
    if ( newAnimation != lf->animationNumber || !lf->animation ) {
        UI_SetLerpFrameAnimation( ci, lf, newAnimation );
    }

    // if we have passed the current frame, move it to
    // oldFrame and calculate a new frame
    if ( dp_realtime >= lf->frameTime ) {
        lf->oldFrame = lf->frame;
        lf->oldFrameTime = lf->frameTime;

        // get the next frame based on the animation
        anim = lf->animation;
        if ( dp_realtime < lf->animationTime ) {
            lf->frameTime = lf->animationTime;		// initial lerp
        } else {
            lf->frameTime = lf->oldFrameTime + anim->frameLerp;
        }
        f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
        if ( f >= anim->numFrames ) {
            f -= anim->numFrames;
            if ( anim->loopFrames ) {
                f %= anim->loopFrames;
                f += anim->numFrames - anim->loopFrames;
            } else {
                f = anim->numFrames - 1;
                // the animation is stuck at the end, so it
                // can immediately transition to another sequence
                lf->frameTime = dp_realtime;
            }
        }
        lf->frame = anim->firstFrame + f;
        if ( dp_realtime > lf->frameTime ) {
            lf->frameTime = dp_realtime;
        }
    }

    if ( lf->frameTime > dp_realtime + 200 ) {
        lf->frameTime = dp_realtime;
    }

    if ( lf->oldFrameTime > dp_realtime ) {
        lf->oldFrameTime = dp_realtime;
    }
    // calculate current lerp value
    if ( lf->frameTime == lf->oldFrameTime ) {
        lf->backlerp = 0;
    } else {
        lf->backlerp = 1.0 - (float)( dp_realtime - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
    }
}


/*
===============
UI_PlayerAnimation
===============
*/
static void UI_PlayerAnimation( playerInfo_t *pi, int *legsOld, int *legs, float *legsBackLerp,
                                int *torsoOld, int *torso, float *torsoBackLerp ) {

    // legs animation
    pi->legsAnimationTimer -= uiInfo.uiDC.frameTime;
    if ( pi->legsAnimationTimer < 0 ) {
        pi->legsAnimationTimer = 0;
    }

    UI_LegsSequencing( pi );

    if ( pi->legs.yawing && ( pi->legsAnim & ~ANIM_TOGGLEBIT ) == LEGS_IDLE ) {
        UI_RunLerpFrame( pi, &pi->legs, LEGS_TURN );
    } else {
        UI_RunLerpFrame( pi, &pi->legs, pi->legsAnim );
    }
    *legsOld = pi->legs.oldFrame;
    *legs = pi->legs.frame;
    *legsBackLerp = pi->legs.backlerp;

    // torso animation
    pi->torsoAnimationTimer -= uiInfo.uiDC.frameTime;
    if ( pi->torsoAnimationTimer < 0 ) {
        pi->torsoAnimationTimer = 0;
    }

    UI_TorsoSequencing( pi );

    UI_RunLerpFrame( pi, &pi->torso, pi->torsoAnim );
    *torsoOld = pi->torso.oldFrame;
    *torso = pi->torso.frame;
    *torsoBackLerp = pi->torso.backlerp;
}


/*
==================
UI_SwingAngles
==================
*/
static void UI_SwingAngles( float destination, float swingTolerance, float clampTolerance,
                            float speed, float *angle, qboolean *swinging ) {
    float	swing;
    float	move;
    float	scale;

    if ( !*swinging ) {
        // see if a swing should be started
        swing = AngleSubtract( *angle, destination );
        if ( swing > swingTolerance || swing < -swingTolerance ) {
            *swinging = qtrue;
        }
    }

    if ( !*swinging ) {
        return;
    }

    // modify the speed depending on the delta
    // so it doesn't seem so linear
    swing = AngleSubtract( destination, *angle );
    scale = fabs( swing );
    if ( scale < swingTolerance * 0.5 ) {
        scale = 0.5;
    } else if ( scale < swingTolerance ) {
        scale = 1.0;
    } else {
        scale = 2.0;
    }

    // swing towards the destination angle
    if ( swing >= 0 ) {
        move = uiInfo.uiDC.frameTime * scale * speed;
        if ( move >= swing ) {
            move = swing;
            *swinging = qfalse;
        }
        *angle = AngleMod( *angle + move );
    } else if ( swing < 0 ) {
        move = uiInfo.uiDC.frameTime * scale * -speed;
        if ( move <= swing ) {
            move = swing;
            *swinging = qfalse;
        }
        *angle = AngleMod( *angle + move );
    }

    // clamp to no more than tolerance
    swing = AngleSubtract( destination, *angle );
    if ( swing > clampTolerance ) {
        *angle = AngleMod( destination - (clampTolerance - 1) );
    } else if ( swing < -clampTolerance ) {
        *angle = AngleMod( destination + (clampTolerance - 1) );
    }
}


/*
======================
UI_MovedirAdjustment
======================
*/
static float UI_MovedirAdjustment( playerInfo_t *pi ) {
    vec3_t		relativeAngles;
    vec3_t		moveVector;

    VectorSubtract( pi->viewAngles, pi->moveAngles, relativeAngles );
    AngleVectors( relativeAngles, moveVector, NULL, NULL );
    if ( Q_fabs( moveVector[0] ) < 0.01 ) {
        moveVector[0] = 0.0;
    }
    if ( Q_fabs( moveVector[1] ) < 0.01 ) {
        moveVector[1] = 0.0;
    }

    if ( moveVector[1] == 0 && moveVector[0] > 0 ) {
        return 0;
    }
    if ( moveVector[1] < 0 && moveVector[0] > 0 ) {
        return 22;
    }
    if ( moveVector[1] < 0 && moveVector[0] == 0 ) {
        return 45;
    }
    if ( moveVector[1] < 0 && moveVector[0] < 0 ) {
        return -22;
    }
    if ( moveVector[1] == 0 && moveVector[0] < 0 ) {
        return 0;
    }
    if ( moveVector[1] > 0 && moveVector[0] < 0 ) {
        return 22;
    }
    if ( moveVector[1] > 0 && moveVector[0] == 0 ) {
        return  -45;
    }

    return -22;
}


/*
===============
UI_PlayerAngles
===============
*/
static void UI_PlayerAngles( playerInfo_t *pi, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
    vec3_t		legsAngles, torsoAngles, headAngles;
    float		dest;
    float		adjust;

    VectorCopy( pi->viewAngles, headAngles );
    headAngles[YAW] = AngleMod( headAngles[YAW] );
    VectorClear( legsAngles );
    VectorClear( torsoAngles );

    // --------- yaw -------------

    // allow yaw to drift a bit
    if ( ( pi->legsAnim & ~ANIM_TOGGLEBIT ) != LEGS_IDLE
            || ( pi->torsoAnim & ~ANIM_TOGGLEBIT ) != TORSO_STAND_RIFLE  ) {
        // if not standing still, always point all in the same direction
        pi->torso.yawing = qtrue;	// always center
        pi->torso.pitching = qtrue;	// always center
        pi->legs.yawing = qtrue;	// always center
    }

    // adjust legs for movement dir
    adjust = UI_MovedirAdjustment( pi );
    legsAngles[YAW] = headAngles[YAW] + adjust;
    torsoAngles[YAW] = headAngles[YAW] + 0.25 * adjust;


    // torso
    UI_SwingAngles( torsoAngles[YAW], 25, 90, SWINGSPEED, &pi->torso.yawAngle, &pi->torso.yawing );
    UI_SwingAngles( legsAngles[YAW], 40, 90, SWINGSPEED, &pi->legs.yawAngle, &pi->legs.yawing );

    torsoAngles[YAW] = pi->torso.yawAngle;
    legsAngles[YAW] = pi->legs.yawAngle;

    // --------- pitch -------------

    // only show a fraction of the pitch angle in the torso
    if ( headAngles[PITCH] > 180 ) {
        dest = (-360 + headAngles[PITCH]) * 0.75;
    } else {
        dest = headAngles[PITCH] * 0.75;
    }
    UI_SwingAngles( dest, 15, 30, 0.1f, &pi->torso.pitchAngle, &pi->torso.pitching );
    torsoAngles[PITCH] = pi->torso.pitchAngle;

    // pull the angles back out of the hierarchial chain
    AnglesSubtract( headAngles, torsoAngles, headAngles );
    AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
    AnglesToAxis( legsAngles, legs );
    AnglesToAxis( torsoAngles, torso );
    AnglesToAxis( headAngles, head );
}

/*
===============
UI_HeadAngle
===============
*/
static void UI_HeadAngle( playerInfo_t *pi, vec3_t head[3] ) {
    vec3_t		headAngles;
    float		dest;

    VectorCopy( pi->viewAngles, headAngles );
    headAngles[YAW] = AngleMod( headAngles[YAW] );

    // --------- pitch -------------
    // only show a fraction of the pitch angle in the torso
    if ( headAngles[PITCH] > 180 ) {
        dest = (-360 + headAngles[PITCH]) * 0.75;
    } else {
        dest = headAngles[PITCH] * 0.75;
    }

    AnglesToAxis( headAngles, head );
}



/*
===============
UI_PlayerFloatSprite
===============
*/
static void UI_PlayerFloatSprite( playerInfo_t *pi, vec3_t origin, qhandle_t shader ) {
    refEntity_t		ent;

    memset( &ent, 0, sizeof( ent ) );
    VectorCopy( origin, ent.origin );
    ent.origin[2] += 48;
    ent.reType = RT_SPRITE;
    ent.customShader = shader;
    ent.radius = 10;
    ent.renderfx = 0;
    trap_R_AddRefEntityToScene( &ent );
}


/*
===============
UI_DrawPlayer
===============
*/
void UI_DrawHead( float x, float y, float w, float h, playerInfo_t *pi, int time ) {
    refdef_t		refdef;
    refEntity_t		head ;
    vec3_t			origin;
    int				renderfx;
    vec3_t			mins = {-16, -16, -24};
    vec3_t			maxs = {16, 16, 32};
    float			len;
    float			xx;

    if ( !pi->headModel ) {
        Com_Printf("couldn't find headmodel 1.\n");
        return;
    }

    // this allows the ui to cache the player model on the main menu
    if (w == 0 || h == 0) {
        return;
    }

    dp_realtime = time;

    UI_AdjustFrom640( &x, &y, &w, &h );

    UI_HeadAngle(pi,head.axis );

    memset( &refdef, 0, sizeof( refdef ) );
    memset( &head, 0, sizeof(head) );

    refdef.rdflags = RDF_NOWORLDMODEL;

    AxisClear( refdef.viewaxis );

    refdef.x = x;
    refdef.y = y;
    refdef.width = w;
    refdef.height = h;

    refdef.fov_x = (int)((float)refdef.width / 640.0f * 90.0f);
    xx = refdef.width / tan( refdef.fov_x / 360 * M_PI );
    refdef.fov_y = atan2( refdef.height, xx );
    refdef.fov_y *= ( 360 / M_PI );

    // calculate distance so the player nearly fills the box
    len = 0.7 * ( maxs[2] - mins[2] );
    origin[0] = len / tan( DEG2RAD(refdef.fov_x) * 0.5 );
    origin[1] = 0.5 * ( mins[1] + maxs[1] );
    origin[2] = -0.5 * ( mins[2] + maxs[2] );

    refdef.time = dp_realtime;

    trap_R_ClearScene();

    // get the rotation information
    renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;

    //
    // add the legs
    //
    head.hModel = pi->headModel;
    head.customSkin = pi->headSkin;

    VectorCopy( origin, head.origin );

    VectorCopy( origin, head.lightingOrigin );
    head.renderfx = renderfx;
    VectorCopy (head.origin, head.oldorigin);

    trap_R_AddRefEntityToScene( &head );

    if (!head.hModel) {
        Com_Printf("couldn't find headmodel 2.\n");
        return;
    }

    //
    // add the chat icon
    //
    if ( pi->chat ) {
        UI_PlayerFloatSprite( pi, origin, trap_R_RegisterShaderNoMip( "sprites/balloon3" ) );
    }

    //
    // add an accent light
    //
    origin[0] -= 100;	// + = behind, - = in front
    origin[1] += 100;	// + = left, - = right
    origin[2] += 100;	// + = above, - = below
    trap_R_AddLightToScene( origin, 500, 1.0, 1.0, 1.0 );

    origin[0] -= 100;
    origin[1] -= 100;
    origin[2] -= 100;
    trap_R_AddLightToScene( origin, 500, 1.0, 0.0, 0.0 );
    //Com_Printf("rendered.\n");
    trap_R_RenderScene( &refdef );
}
/*
==========================
88
==========================
*/
qboolean UI_RegisterClientStyleModels( playerInfo_t *pi, const char *mouthName, const char *eyesName, const char *headName ) {
    char		filename[MAX_QPATH];

    // load cmodels before models so filecache works
    Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/m_%s.md3", mouthName );
    pi->equipmentMouth = trap_R_RegisterModel( filename );

    Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/h_%s.md3", headName );
    pi->equipmentHead = trap_R_RegisterModel( filename );

    Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/e_%s.md3", eyesName );
    pi->equipmentEyes = trap_R_RegisterModel( filename );

    return qtrue;
}
extern	vmCvar_t	ui_s_model;
extern	vmCvar_t	ui_s_skin;
/*- *** -*/
extern	vmCvar_t	ui_t_model;
extern	vmCvar_t	ui_t_skin;
extern	vmCvar_t	cg_gunSmokeTime;

/*
===============
UI_DrawPlayerHead
===============
*/
extern vmCvar_t ui_test;

void UI_DrawPlayerHead( float x, float y, float w, float h, playerInfo_t *pi, int time, int team ) {
    refdef_t		refdef;
    refEntity_t		head, torso;
    vec3_t			origin;
    int				renderfx;
    vec3_t			mins = {-4, -4, -4};
    vec3_t			maxs = {4, 4, 4};
    float			len;
    float			xx;

    if (  !pi->headModel  ) {
        return;
    }

    // this allows the ui to cache the player model on the main menu
    if (w == 0 || h == 0) {
        return;
    }

    dp_realtime = time;

    UI_AdjustFrom640( &x, &y, &w, &h );

    y -= jumpHeight;

    memset( &refdef, 0, sizeof( refdef ) );

    memset( &head, 0, sizeof(head) );
    memset( &torso, 0, sizeof(torso) );

    refdef.rdflags = RDF_NOWORLDMODEL;

    AxisClear( refdef.viewaxis );

    refdef.x = x;
    refdef.y = y;
    refdef.width = w;
    refdef.height = h;

    refdef.fov_x = (int)((float)refdef.width / 640.0f * 20.0f );
    xx = refdef.width / tan( refdef.fov_x / 360 * M_PI );
    refdef.fov_y = atan2( refdef.height, xx );
    refdef.fov_y *= ( 360 / (float)M_PI );

    // calculate distance so the player nearly fills the box
    len = 0.7 * ( maxs[2] - mins[2] );
    origin[0] = len / tan( DEG2RAD(refdef.fov_x) * 0.5 );
    origin[1] = 0.5 * ( mins[1] + maxs[1] );
    origin[2] = -0.5 * ( mins[2] + maxs[2] );

//    origin[2] -= 2.75;

    refdef.time = dp_realtime;

    trap_R_ClearScene();

    pi->viewAngles[YAW] = 170 +  10 * sin( time / 1000.0 );

    if ( team == TEAM_RED )
        pi->viewAngles[YAW] += 5;

    // get the rotation information
    UI_HeadAngle( pi, head.axis );

    renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;

    memcpy( &torso, &head, sizeof( head ) );

    torso.hModel = pi->torsoModel;
    torso.customSkin = pi->torsoSkin;

    VectorCopy( origin, torso.origin );
    VectorCopy( origin, torso.lightingOrigin );
 
	if ( team == TEAM_RED ) {
        torso.origin[2] -= 17;  
		torso.origin[0] += 25;
		torso.origin[1] += 0.4f;
		torso.frame = 75 ;
	}
	else if ( team == TEAM_BLUE ) {
        torso.origin[2] -= 20.5; 
		torso.origin[0] += 25 ;
	}  

    torso.renderfx = renderfx;

    VectorCopy (torso.origin, torso.oldorigin);

    trap_R_AddRefEntityToScene( &torso );


    if (!torso.hModel) {
        return;
    }


    //
    // add the head
    //
    head.hModel = pi->headModel ;
    head.customSkin = pi->headSkin ;

    VectorCopy( origin, head.origin );

    VectorCopy( origin, head.lightingOrigin );
    head.renderfx = renderfx;
    VectorCopy (head.origin, head.oldorigin);

    UI_PositionEntityOnTag( &head, &torso, pi->torsoModel, "tag_head");

    trap_R_AddRefEntityToScene( &head );

    if (!head.hModel) {
        return;
    }

    if (pi->equipmentEyes)
    {
        head.hModel = pi->equipmentEyes;//CG_GetEquipmentModel(ci->playerEquipment, ci->team );
        head.customSkin = 0;
        trap_R_AddRefEntityToScene( &head );
    }
    if (pi->equipmentHead)
    {
        head.hModel = pi->equipmentHead;//CG_GetEquipmentModel(ci->playerEquipment, ci->team );
        head.customSkin = 0;
        trap_R_AddRefEntityToScene( &head );
    }
    if (pi->equipmentMouth)
    {
        head.hModel = pi->equipmentMouth;//CG_GetEquipmentModel(ci->playerEquipment, ci->team );
        head.customSkin = 0;
        trap_R_AddRefEntityToScene( &head );
    }
	

    trap_R_RenderScene( &refdef );
}


/*
==========================
UI_RegisterClientSkin
==========================
*/
static qboolean	UI_RegisterClientSkin( playerInfo_t *pi, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName , const char *teamName) {
    char		filename[MAX_QPATH];

    Com_sprintf( filename, sizeof( filename ), "models/players/heads/head_%s.skin", skinName );
    pi->headSkin = trap_R_RegisterSkin( filename );

    if ( !pi->headSkin ) {
        return qfalse;
    }

    Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso%s.skin", modelName, "_urban"  );
    pi->torsoSkin = trap_R_RegisterSkin( filename );

    if ( !pi->torsoSkin ) {
        return qfalse;
    }


    return qtrue;
}


/*
======================
UI_ParseAnimationFile
======================

static qboolean UI_ParseAnimationFile( const char *filename, animation_t *animations ) {
char		*text_p, *prev;
int			len;
int			i;
char		*token;
float		fps;
int			skip;
char		text[20000];
fileHandle_t	f;

memset( animations, 0, sizeof( animation_t ) * MAX_ANIMATIONS );

// load the file
len = trap_FS_FOpenFile( filename, &f, FS_READ );
if ( len <= 0 ) {
return qfalse;
}
if ( len >= ( sizeof( text ) - 1 ) ) {
Com_Printf( "File %s too long\n", filename );
return qfalse;
}
trap_FS_Read( text, len, f );
text[len] = 0;
trap_FS_FCloseFile( f );

COM_Compress(text);

// parse the text
text_p = text;
skip = 0;	// quite the compiler warning

// read optional parameters
while ( 1 ) {
prev = text_p;	// so we can unget
token = COM_Parse( &text_p );
if ( !token ) {
break;
}
if ( !Q_stricmp( token, "footsteps" ) ) {
token = COM_Parse( &text_p );
if ( !token ) {
break;
}
continue;
} else if ( !Q_stricmp( token, "headoffset" ) ) {
for ( i = 0 ; i < 3 ; i++ ) {
token = COM_Parse( &text_p );
if ( !token ) {
break;
}
}
continue;
} else if ( !Q_stricmp( token, "sex" ) ) {
token = COM_Parse( &text_p );
if ( !token ) {
break;
}
continue;
}

// if it is a number, start parsing animations
if ( token[0] >= '0' && token[0] <= '9' ) {
text_p = prev;	// unget the token
break;
}

Com_Printf( "unknown token '%s' is %s\n", token, filename );
}

// read information for each frame
for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

token = COM_Parse( &text_p );
if ( !token ) {
break;
}
animations[i].firstFrame = atoi( token );
// leg only frames are adjusted to not count the upper body only frames
if ( i == LEGS_WALKCR ) {
skip = animations[LEGS_IDLECR].firstFrame - animations[TORSO_GESTURE1].firstFrame;
}
if ( i >= LEGS_WALKCR ) {
animations[i].firstFrame -= skip;
}

token = COM_Parse( &text_p );
if ( !token ) {
break;
}
animations[i].numFrames = atoi( token );

token = COM_Parse( &text_p );
if ( !token ) {
break;
}
animations[i].loopFrames = atoi( token );

token = COM_Parse( &text_p );
if ( !token ) {
break;
}
fps = atof( token );
if ( fps == 0 ) {
fps = 1;
}
animations[i].frameLerp = 1000 / fps;
animations[i].initialLerp = 1000 / fps;
}

if ( i != MAX_ANIMATIONS ) {
Com_Printf( "Error parsing animation file: %s", filename );
return qfalse;
}

animations[TORSO_CLIMB_IDLE].firstFrame = animations[TORSO_CLIMB].firstFrame+1;
animations[TORSO_CLIMB_IDLE].numFrames = 1;
animations[TORSO_CLIMB_IDLE].loopFrames = 1;
animations[TORSO_CLIMB_IDLE].frameLerp = animations[TORSO_CLIMB].frameLerp;
animations[TORSO_CLIMB_IDLE].initialLerp = animations[TORSO_CLIMB].initialLerp;


memcpy(&animations[TORSO_RAISE_RIFLE], &animations[TORSO_DROP_RIFLE], sizeof(animation_t));
animations[TORSO_RAISE_RIFLE].reversed = qtrue;

// limb backwards animation
memcpy(&animations[LEGS_BACKLIMB], &animations[LEGS_LIMP], sizeof(animation_t));
animations[LEGS_BACKLIMB].reversed = qtrue;


// crouch backwards animation
memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));
animations[LEGS_BACKCR].reversed = qtrue;

return qtrue;
}
*/


/*
==========================
UI_RegisterClientModelname
==========================
*/
qboolean UI_RegisterClientModelname( playerInfo_t *pi, const char *modelSkinName, const char *headName, const char *teamName ) {
    char		modelName[MAX_QPATH];
    char		headModelName[MAX_QPATH];
    char		skinName[MAX_QPATH];
    char		filename[MAX_QPATH];
    char		*slash, *headslash;

    pi->torsoModel = 0;
    pi->headModel = 0;

    if ( !modelSkinName[0] ) {
        return qfalse;
    }

    Q_strncpyz( modelName, modelSkinName, sizeof( modelName ) );
    Q_strncpyz( headModelName, headName, sizeof( headModelName ) );

    slash = strchr( modelName, '/' );
    if ( !slash ) {
        // modelName did not include a head name
        Q_strncpyz( skinName, "jamal", sizeof( skinName ) );
    } else {
        Q_strncpyz( skinName, slash + 1, sizeof( skinName ) );
        // truncate modelName
        *slash = 0;
    }

    headslash = strchr( headModelName, '/' );
    if ( headslash ) {
        *headslash = 0;
    }

    // load default head model.
    Com_sprintf( filename, sizeof( filename ), "models/players/heads/head.md3" );
    pi->headModel = trap_R_RegisterModel( filename );

    if (!pi->headModel) {
        Com_Printf( "Failed to load model file %s\n", filename );
        return qfalse;
    }

    // load torso
    Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso.md3", modelName );
    pi->torsoModel = trap_R_RegisterModel( filename );

    if (!pi->torsoModel) {
        Com_Printf( "Failed to load model file %s\n", filename );
        return qfalse;
    }


    // if any skins failed to load, fall back to default
    if ( !UI_RegisterClientSkin( pi, modelName, skinName, headModelName, skinName, teamName) ) {
        if ( !UI_RegisterClientSkin( pi, "t_medium", "bruce", headName, "bruce", teamName ) ) {
            Com_Printf( "Failed to load skin file: %s : %s\n", modelName, skinName );
            return qfalse;
        }
    }

    return qtrue;
}


/*
===============
UI_PlayerInfo_SetModel
===============
*/
void UI_PlayerInfo_SetModel( playerInfo_t *pi, const char *model, const char *headmodel, char *teamName ) {
    memset( pi, 0, sizeof(*pi) );
    UI_RegisterClientModelname( pi, model, headmodel, teamName );
    pi->weapon = WP_MK23;
    pi->currentWeapon = pi->weapon;
    pi->lastWeapon = pi->weapon;
    pi->pendingWeapon = -1;
    pi->weaponTimer = 0;
    pi->chat = qfalse;
    pi->newModel = qtrue;
    UI_PlayerInfo_SetWeapon( pi, pi->weapon );
}


/*
===============
UI_PlayerInfo_SetInfo
===============
*/
void UI_PlayerInfo_SetInfo( playerInfo_t *pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNumber, qboolean chat ) {
    int			currentAnim;
    weapon_t	weaponNum;

    pi->chat = chat;

    // view angles
    VectorCopy( viewAngles, pi->viewAngles );

    // move angles
    VectorCopy( moveAngles, pi->moveAngles );

    if ( pi->newModel ) {
        pi->newModel = qfalse;

        jumpHeight = 0;
        pi->pendingLegsAnim = 0;
        UI_ForceLegsAnim( pi, legsAnim );
        pi->legs.yawAngle = viewAngles[YAW];
        pi->legs.yawing = qfalse;

        pi->pendingTorsoAnim = 0;
        UI_ForceTorsoAnim( pi, torsoAnim );
        pi->torso.yawAngle = viewAngles[YAW];
        pi->torso.yawing = qfalse;

        if ( weaponNumber != -1 ) {
            pi->weapon = weaponNumber;
            pi->currentWeapon = weaponNumber;
            pi->lastWeapon = weaponNumber;
            pi->pendingWeapon = -1;
            pi->weaponTimer = 0;
            UI_PlayerInfo_SetWeapon( pi, pi->weapon );
        }

        return;
    }

    // weapon
    if ( weaponNumber == -1 ) {
        pi->pendingWeapon = -1;
        pi->weaponTimer = 0;
    }
    else if ( weaponNumber != WP_NONE ) {
        pi->pendingWeapon = weaponNumber;
        pi->weaponTimer = dp_realtime + UI_TIMER_WEAPON_DELAY;
    }
    weaponNum = pi->lastWeapon;
    pi->weapon = weaponNum;


    // leg animation
    currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;
    if ( legsAnim != LEGS_JUMP && ( currentAnim == LEGS_JUMP || currentAnim == LEGS_LAND ) ) {
        pi->pendingLegsAnim = legsAnim;
    }
    else if ( legsAnim != currentAnim ) {
        jumpHeight = 0;
        pi->pendingLegsAnim = 0;
        UI_ForceLegsAnim( pi, legsAnim );
    }

    // torso animation
    if ( torsoAnim == TORSO_STAND_RIFLE || torsoAnim == TORSO_STAND_PISTOL || torsoAnim == TORSO_STAND_ITEM ) {
        if ( BG_IsMelee( weaponNum ) ) {
            torsoAnim = TORSO_STAND_ITEM;
        }
        else if ( BG_IsPistol( weaponNum ) || BG_IsSMG( weaponNum)  ) {
            torsoAnim = TORSO_STAND_PISTOL;
        }
        else if ( BG_IsRifle( weaponNum ) ) {
            torsoAnim = TORSO_STAND_RIFLE;
        }
        else{
            torsoAnim = TORSO_STAND_ITEM;
        }
    }

    if ( torsoAnim == TORSO_ATTACK_RIFLE || torsoAnim == TORSO_ATTACK_PISTOL || torsoAnim == TORSO_ATTACK_MELEE ) {
        if ( BG_IsMelee( weaponNum ) ) {
            torsoAnim = TORSO_ATTACK_MELEE;
        }
        else if ( BG_IsPistol( weaponNum ) || BG_IsSMG( weaponNum)  ) {
            torsoAnim = TORSO_ATTACK_PISTOL;
        }
        else if ( BG_IsRifle( weaponNum ) ) {
            torsoAnim = TORSO_ATTACK_RIFLE;
        }
        else{
            torsoAnim = TORSO_ATTACK_MELEE;
        }		pi->muzzleFlashTime = dp_realtime + UI_TIMER_MUZZLE_FLASH;
        //FIXME play firing sound here
    }

    currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

    if ( weaponNum != pi->currentWeapon || currentAnim == TORSO_RAISE_RIFLE || currentAnim == TORSO_RAISE_PISTOL || currentAnim == TORSO_DROP_PISTOL || currentAnim == TORSO_DROP_RIFLE ) {
        pi->pendingTorsoAnim = torsoAnim;
    }
    else if ( ( currentAnim == TORSO_GESTURE1 || currentAnim == TORSO_ATTACK_RIFLE || currentAnim == TORSO_ATTACK_PISTOL || currentAnim == TORSO_ATTACK_MELEE ) && ( torsoAnim != currentAnim ) ) {
        pi->pendingTorsoAnim = torsoAnim;
    }
    else if ( torsoAnim != currentAnim ) {
        pi->pendingTorsoAnim = 0;
        UI_ForceTorsoAnim( pi, torsoAnim );
    }
}
