// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_ents.c -- present snapshot entities, happens every single frame

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

#include "..\game\variables.h"


/*
======================
CG_PositionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
                             qhandle_t parentModel, char *tagName ) {
    int				i;
    orientation_t	lerped;

    // lerp the tag
    trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
                    1.0 - parent->backlerp, tagName );

    // FIXME: allow origin offsets along tag?
    VectorCopy( parent->origin, entity->origin );
    for ( i = 0 ; i < 3 ; i++ ) {
        VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
    }

    // had to cast away the const to avoid compiler problems...
    MatrixMultiply( lerped.axis, ((refEntity_t *)parent)->axis, entity->axis );
    entity->backlerp = parent->backlerp;
}


/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
                                    qhandle_t parentModel, char *tagName ) {
    int				i;
    orientation_t	lerped;
    vec3_t			tempAxis[3];

    // AxisClear( entity->axis );
    // lerp the tag
    trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
                    1.0 - parent->backlerp, tagName );

    // FIXME: allow origin offsets along tag?
    VectorCopy( parent->origin, entity->origin );
    for ( i = 0 ; i < 3 ; i++ ) {
        VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
    }

    // had to cast away the const to avoid compiler problems...
    MatrixMultiply( entity->axis, lerped.axis, tempAxis );
    MatrixMultiply( tempAxis, ((refEntity_t *)parent)->axis, entity->axis );
}

/*
======================
CG_GetOriginFromTag

changes "out" to the given tag origin
======================
*/
void CG_GetOriginFromTag( const refEntity_t *parent,
                          qhandle_t parentModel, char *tagName, vec3_t out ) {
    int				i;
    orientation_t	lerped;

    // lerp the tag
    trap_R_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
                    1.0 - parent->backlerp, tagName );

    VectorCopy( parent->origin, out );

    for ( i = 0 ; i < 3 ; i++ ) {
        VectorMA( out , lerped.origin[i], parent->axis[i], out );
    }
}


/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent ) {
    if ( cent->currentState.solid == SOLID_BMODEL ) {
        vec3_t	origin;
        float	*v;

        v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
        VectorAdd( cent->lerpOrigin, v, origin );
        trap_S_UpdateEntityPosition( cent->currentState.number, origin );
    } else {
        trap_S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
    }
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {

    // update sound origins
    CG_SetEntitySoundPosition( cent );

    // add loop sound
    if ( cent->currentState.loopSound ) {
        if (cent->currentState.eType != ET_SPEAKER) {
            trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
                                    cgs.gameSounds[ cent->currentState.loopSound ] );
        } else {
            trap_S_AddRealLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin,
                                        cgs.gameSounds[ cent->currentState.loopSound ] );
        }
    }


    // constant light glow
    if ( cent->currentState.constantLight ) {
        int		cl;
        int		i, r, g, b;

        cl = cent->currentState.constantLight;
        r = cl & 255;
        g = ( cl >> 8 ) & 255;
        b = ( cl >> 16 ) & 255;
        i = ( ( cl >> 24 ) & 255 ) * 4;
        trap_R_AddLightToScene( cent->lerpOrigin, i, r, g, b );
    }

}


/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) {
    refEntity_t			ent;
    entityState_t		*s1;

    s1 = &cent->currentState;

    // if set to invisible, skip
    if (!s1->modelindex) {
        return;
    }

    memset (&ent, 0, sizeof(ent));

    // set frame

    ent.frame = s1->frame;
    ent.oldframe = ent.frame;
    ent.backlerp = 0;

    VectorCopy( cent->lerpOrigin, ent.origin);
    VectorCopy( cent->lerpOrigin, ent.oldorigin);

    ent.hModel = cgs.gameModels[s1->modelindex];

    // player model
    if (s1->number == cg.snap->ps.clientNum) {
        ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
    }

    // convert angles to axis
    AnglesToAxis( cent->lerpAngles, ent.axis );

    // add to refresh list
    trap_R_AddRefEntityToScene (&ent);
}

/*
==================
CG_ParticleHost

Launches particles

"random"		randomizes amount of particles it spawns (full numbers)
"randomdir"		randomizes velocity of each particle ( x/y NOT Z(up) ) (full numbers)
"velocity"		velocity (full numbers)
"particles"		amount of particles randomized by 'random' key (full numbers)
"size"			size modifier (only for smoke) (full numbers)
"dlight"		add a dynamic light (0=none)-(255=biggest dlight) (full numbers)
"wait"			fire particles every X milliseconds. if you set a value you cannot "trigger" it. (full numbers)

==================
*/

#define PARTFLAGS_DIRT			1
#define	PARTFLAGS_GLASS			2
#define PARTFLAGS_SAND			4
#define PARTFLAGS_SPARKS		8
#define PARTFLAGS_METALSPARKS	16
#define PARTFLAGS_SMOKE			32
#define PARTFLAGS_STARTOFF		64

void CG_Spark( vec3_t org, vec3_t dir, float width );
void CG_MetalSpark( vec3_t org, vec3_t dir, float width );
void CG_SurfaceEffect( vec3_t origin, vec3_t dir, int surface, int weapon, float radius );

static void CG_ParticleHost( centity_t *cent ) {
    int random = 0, randomdir = 0, velocity = 0, particles = 0, size = 0, dlight = 0, wait = 0;
    int i, spawnflags;
    vec3_t	angles;

    // miscTime? err, time to launch particles?
    if ( cent->miscTime > cg.time )
    {
        //	CG_Printf("cent->miscTime > cg.time\n");
        return;
    }

    random		=	cent->currentState.powerups;
    randomdir	=	cent->currentState.time2;
    velocity	=	cent->currentState.torsoAnim;
    particles	=	cent->currentState.generic1;
    size		=	cent->currentState.legsAnim;
    dlight		=	cent->currentState.otherEntityNum2;
    wait		=	cent->currentState.time;
    spawnflags	=	cent->currentState.eventParm;

#if 0 // debug
    CG_Printf( "spawned particle %s: random %i,randomdir %i,velocity %i,particles %i,size %i,dlight %i, wait %i spawnflags %i\n",
               vtos ( cent->lerpOrigin), random,randomdir,velocity,particles,size,dlight,wait,spawnflags );
#endif

    // randomize amount of particle it will spawn
    particles -= random()*random;

    if ( particles < 1 )
        particles = 1;

    VectorNormalize( cent->currentState.angles );

    VectorScale(cent->currentState.angles, velocity - 10 + random()*10, cent->currentState.angles);

    // spawn particles
    for ( i = 0; i < particles; i ++ )
    {
        VectorCopy( cent->currentState.angles, angles );

        // randomize XY
        angles[0] += -(randomdir/2) + random()*randomdir;
        angles[1] += -(randomdir/2) + random()*randomdir;

        // spawn effect
        if ( spawnflags & PARTFLAGS_DIRT )
            CG_SurfaceEffect( cent->lerpOrigin, angles, BHOLE_DIRT, WP_M4, size );
        else if ( spawnflags & PARTFLAGS_GLASS )
            CG_SurfaceEffect( cent->lerpOrigin, angles, BHOLE_GLASS, WP_M4, size );
        else if ( spawnflags & PARTFLAGS_SAND )
            CG_SurfaceEffect( cent->lerpOrigin, angles, BHOLE_SAND, WP_M4, size );
        else if ( spawnflags & PARTFLAGS_SPARKS )
            CG_Spark( cent->lerpOrigin, angles, 1.0f );
        else if ( spawnflags & PARTFLAGS_METALSPARKS )
            CG_MetalSpark( cent->lerpOrigin, angles, 0.5f );
        else if ( spawnflags & PARTFLAGS_SMOKE )
            CG_SmokePuff( cent->lerpOrigin, angles, 0.5 + size + random(), 1, 1, 1, 1, 1000 + random()*500, cg.time, 0,LE_MOVE_SCALE_FADE, cgs.media.smokePuffShader );
        else
            CG_Spark( cent->lerpOrigin, angles, 1.0f );
    }

    // add dynamic light
    if ( dlight )
        trap_R_AddLightToScene( cent->lerpOrigin, (float)dlight,1.0f,1.0f,1.0f );

    // set time next spawn will happen
    cent->miscTime = cg.time + wait;
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker( centity_t *cent ) {
    if ( ! cent->currentState.clientNum ) {	// FIXME: use something other than clientNum...
        return;				// not auto triggering
    }

    if ( cg.time < cent->miscTime ) {
        return;
    }

    if ((cg.DeafTime < cg.time)) trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm] );

    //	ent->s.frame = ent->wait * 10;
    //	ent->s.clientNum = ent->random * 10;
    cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * crandom();
}

void CG_AddWeaponWithPowerups( refEntity_t gun, weaponInfo_t	*weapon,int i_equipment, int weaponmode );
/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) {
    refEntity_t			ent,ent2;
    entityState_t		*es;
    gitem_t				*item;

    es = &cent->currentState;
    if ( es->modelindex >= bg_numItems ) {
        CG_Error( "Bad item index %i on entity", es->modelindex );
    }

    // if set to invisible, skip
    if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) ) {
        return;
    }

    item = &bg_itemlist[ es->modelindex ];

    //
    // if simpleitems are enable render as sprite and then return
    //
    if ( cg_simpleItems.integer ) //&& item->giType != IT_TEAM )
    {
        memset( &ent, 0, sizeof( ent ) );

        ent.reType = RT_SPRITE;
        VectorCopy( cent->lerpOrigin, ent.origin );
        ent.radius = 14;
        ent.customShader = cg_items[es->modelindex].icon;
        ent.shaderRGBA[0] = 255;
        ent.shaderRGBA[1] = 255;
        ent.shaderRGBA[2] = 255;
        ent.shaderRGBA[3] = 240;
        trap_R_AddRefEntityToScene(&ent);
        return;
    }

    // briefcase return points are handled as items
    if ( !strcmp( item->classname, "team_briefcase_return" ) )
    {
        memset( &ent, 0, sizeof( ent ) );

        ent.reType = RT_SPRITE;
        VectorCopy( cent->lerpOrigin, ent.origin );
        ent.radius = 26;
        ent.origin[2] += 16; // move up
        ent.customShader = trap_R_RegisterShader("gfx/misc/baggage_return.tga");
        ent.shaderRGBA[0] = 255;
        ent.shaderRGBA[1] = 255;
        ent.shaderRGBA[2] = 255;
        ent.shaderRGBA[3] = 240;

        trap_R_AddRefEntityToScene(&ent);
        return;
    }	// C4 is hovering above ground
    else if ( item->giTag == WP_C4 )
    {
        memset( &ent2, 0, sizeof( ent2 ) );

        /*	ent2.reType = RT_SPRITE;
        VectorCopy( cent->lerpOrigin, ent2.origin );
        ent2.origin[2] += 1.5;
        ent2.radius = 8;
        ent2.customShader = cgs.media.flare;
        ent2.shaderRGBA[0] = 255;
        ent2.shaderRGBA[1] = 255;
        ent2.shaderRGBA[2] = 255;
        ent2.shaderRGBA[3] = 175;

        trap_R_AddRefEntityToScene(&ent2);  */

        // prepare angles for bomb model
        VectorCopy( cg.autoAngles, cent->lerpAngles );

        cent->lerpAngles[PITCH] += 40;
        cent->lerpAngles[ROLL] -= 180;
        cent->lerpOrigin[2] += 10;
    }

    memset( &ent, 0, sizeof( ent ) );

    // the weapons have their origin where they attatch to player
    // models, so we need to offset them or they will rotate
    // eccentricly
    if (item->giType == IT_WEAPON ) {
        ent.hModel = cg_weapons[item->giTag].viewweaponModel;
        cent->lerpOrigin[2] -= 3; // bring to ground
    }
    else if ( item->giTag == PW_BRIEFCASE )
    {
        cent->lerpOrigin[2] -= 10;
        ent.hModel = cgs.media.briefcaseModel;
    }
    else
        ent.hModel = cg_items[es->modelindex].models[0];

    // no model ?
    if ( !ent.hModel )
        return;

    AnglesToAxis( cent->lerpAngles, ent.axis );

    VectorCopy( cent->lerpOrigin, ent.origin);
    VectorCopy( cent->lerpOrigin, ent.oldorigin);

    ent.nonNormalizedAxes = qfalse;

    // add to refresh list
    if ( item->giType == IT_WEAPON ) {

        weaponInfo_t	*weapon;

        weapon = &cg_weapons[item->giTag];

        //trap_R_AddRefEntityToScene(&ent);
        if ( BG_WeaponMods( item->giTag ) != 0 )
            CG_AddWeaponWithPowerups( ent, weapon, cent->currentState.powerups, cent->currentState.eFlags );
        else
            trap_R_AddRefEntityToScene(&ent);
    }
    else
        trap_R_AddRefEntityToScene(&ent);
}

//============================================================================



#define SUB_FLARES		11
#define DIST_START_FADE 2000
#define DIST_END_FADE	3000
#define TIME_FADEOUT	200
/*
===============
CG_Flare
===============
*/
static void CG_Flare( centity_t *cent, vec3_t lerpOrigin, float size, float r, float g, float b, qboolean showCorona ) {
    refEntity_t			ent;
    trace_t				trace;
    float				distance;
    vec3_t				angles;
    int					i;
    vec3_t				forward;
    vec3_t				middle;
    float				dot;
    float				radius = 1;
    float				a = 1.0f;
    int					weaponstate = cg.snap->ps.weaponstate;

    AngleVectors( cg.refdefViewAngles, forward,NULL,NULL );

    VectorSubtract( cg.refdef.vieworg, lerpOrigin , angles );

    VectorNormalize( angles );
    dot = DotProduct (angles, forward);

    // not in same point contents
    if (
        CG_PointContents( cg.refdef.vieworg , -1 ) & CONTENTS_WATER
    )
    {
        //		CG_Printf("Flare not in same contents.\n");
        return;
    }

    // if the flare isn't in the screen of the player don't render
    if ( dot < -1 )
    {
        cent->miscTime = 0;
        //		CG_Printf("Flare out of screen.\n");
        return;
    }
    else if ( dot > -0.60 )
    {
        cent->miscTime = 0;
        //		CG_Printf("Flare out of screen.\n");
        return;
    }

    // trace a line from flare origin to viewpoint to see if it's visible
    CG_Trace( &trace, cg.refdef.vieworg, NULL, NULL, lerpOrigin, cg.predictedPlayerState.clientNum, MASK_SOLID );

    // if the flare is blocked, forget it
    if (trace.fraction < 1 )
    {
        if ( showCorona && cent )
        {
            int t = cent->miscTime - cg.time;

            if ( t > 0 )
            {
                // get alpha
                a = (float)(t / (float)TIME_FADEOUT);

                // create the base flare entity
                memset (&ent, 0, sizeof(ent));

                VectorCopy( lerpOrigin, ent.origin);
                VectorCopy( lerpOrigin, ent.oldorigin);

                distance = Distance( cg.refdef.vieworg, lerpOrigin);

                if ( !BG_IsZooming(cg.snap->ps.stats[STAT_WEAPONMODE] ) ||
                        cg.renderingThirdPerson ||
                        ( weaponstate != WEAPON_FIRING && weaponstate !=	WEAPON_FIRING2 && weaponstate != WEAPON_FIRING3 && weaponstate != WEAPON_READY )
                   )
                    ent.radius = 1 + ( size * distance/175 );
                else
                {
                    float newdist;

                    newdist = distance;
                    newdist -= 30;
                    if (newdist < 0.0f )
                        newdist = 0.0f;

                    ent.radius = ( size * newdist/85 );
                }


                ent.reType = RT_SPRITE;
                ent.renderfx = RF_DEPTHHACK;
                ent.rotation = distance;
                ent.customShader =  cgs.media.flare;


                if ( distance > DIST_START_FADE )
                {

                    if ( distance <= 0 )
                        distance = 0;

                    if ( distance > DIST_END_FADE )
                        distance = DIST_END_FADE;

                    a = a - ( (distance-DIST_START_FADE) / 1000.0f);

                    // it already faded out
                    if ( a <= 0.0f )
                        a = 0.0f;
                }
                // alpha fade | custom color
                // --- //
                //
                {

                    r /= 100.0f;
                    g /= 100.0f;
                    b /= 100.0f;

                    ent.shaderRGBA[0] = 0xff * r;
                    ent.shaderRGBA[1] = 0xff * g;
                    ent.shaderRGBA[2] = 0xff * b;
                    ent.shaderRGBA[3] = 0xff * a;
                }

                //				CG_Printf("rendering flare invisible w/ alpha %f and %i\n", a , t);
                trap_R_AddRefEntityToScene( &ent );
            }
        }
        //		CG_Printf("Flare not inline with player (blocked).\n");
        return;
    }

    if ( cent && showCorona ) // set fadeout time
    {
        cent->miscTime = cg.time + TIME_FADEOUT;
        //	CG_Printf("flare visible setted flaretime to %i\n", TIME_FADEOUT );
    }

    distance = Distance( cg.refdef.vieworg, lerpOrigin);

    VectorMA( cg.refdef.vieworg,  distance/2 , forward, middle );
    VectorSubtract(middle, lerpOrigin , angles );
    VectorNormalize( angles );

    //	CG_Printf("Distance: %f Dot: %f\n", distance, dot);


    // create the base flare entity
    memset (&ent, 0, sizeof(ent));

    VectorCopy( lerpOrigin, ent.origin);
    VectorCopy( lerpOrigin, ent.oldorigin);

    if ( !BG_IsZooming( cg.snap->ps.stats[STAT_WEAPONMODE] ) ||
            cg.renderingThirdPerson )
    {
        float newdist;

        newdist = distance;
        newdist -= 30;
        if (newdist < 0.0f )
            newdist = 0.0f;

        ent.radius = ( size * newdist/175 );
    }
    else
        ent.radius = 1 + ( size * distance/85 );

    if ( distance > DIST_START_FADE )
    {

        if ( distance <= 0 )
            distance = 0;

        if ( distance > DIST_END_FADE )
            distance = DIST_END_FADE;

        a = 1.0f - ( (distance-DIST_START_FADE) / 1000.0f);

    }
    //	CG_Printf("a %f dist %f \n", a, distance);

    ent.reType = RT_SPRITE;

    ent.renderfx = RF_DEPTHHACK;
    ent.rotation = distance;
    ent.customShader =  cgs.media.flare;

    // alpha fade | custom color
    r /= 100.0f;
    g /= 100.0f;
    b /= 100.0f;

    ent.shaderRGBA[0] = 0xff * r;
    ent.shaderRGBA[1] = 0xff * g;
    ent.shaderRGBA[2] = 0xff * b;
    ent.shaderRGBA[3] = 0xff * a;

    //
    // show corona?
    //
    if ( showCorona )
        trap_R_AddRefEntityToScene( &ent );

    //
    // only render circlers / flash if we're scoping
    //
    if ( !BG_IsZooming(cg.snap->ps.stats[STAT_WEAPONMODE] ) ||
            cg.renderingThirdPerson ||
            ( weaponstate != WEAPON_FIRING && weaponstate !=	WEAPON_FIRING2 && weaponstate != WEAPON_FIRING3 && weaponstate != WEAPON_READY )
       )
        return;

    //
    // blend code [ when looking directly into a flare with a scope the screen gets blended white ]
    //
    if ( dot >= -1.0f && dot <= -0.999f )
    {
        float alpha;

        alpha = -dot;
        alpha -= 0.999f;
        alpha *= 1000;

        if ( alpha > cg.blendAlpha )
        {
            cg.blendAlpha = alpha;
        }
    }

    //
    // check if the flare is inside our scope. if not , or close the boarders then close the circles
    //
    if ( dot < -0.925 && dot > -0.975 )
    {
        radius = -dot;
        radius -= 0.925f;
        radius *= 20;

        if ( radius <= 0 )
            return;

    }
    else if ( dot > -0.925 )
        return;

    //	CG_Printf("flare dot(%f) distance(%f)\n",  dot, distance);

    for ( i = 1; i < SUB_FLARES + 1; i ++ )
    {
        if ( i == 1 )
        {
            ent.radius = 2.5;
            ent.customShader = cgs.media.flare_flare_green;
            ent.shaderRGBA[3] = 100;
        }
        else if ( i == 2 )
        {
            ent.radius = 5;
            ent.customShader = cgs.media.flare_circle_green;
            ent.shaderRGBA[3] = 60;
        }
        else if ( i == 3 )
        {
            ent.radius = 3.7f;
            ent.customShader = cgs.media.flare_circle_green;
            ent.shaderRGBA[3] = 80;
        }
        else if ( i == 4 )
        {
            ent.radius = 3;
            ent.customShader = cgs.media.flare_circle_orange;
            ent.shaderRGBA[3] = 120;
        }
        else if ( i == 5 )
        {
            ent.radius = 2;
            ent.customShader = cgs.media.flare_flare_turkis;
            ent.shaderRGBA[3] = 230;
        }
        else if ( i == 6 )
        {
            ent.radius = 4;
            ent.customShader = cgs.media.flare_flare_turkis;
            ent.shaderRGBA[3] = 100;
        }
        else if ( i == 7 )
        {
            ent.radius = 6;
            ent.customShader = cgs.media.flare_circle_orange;
            ent.shaderRGBA[3] = 140;
        }
        else if ( i == 7 )
        {
            ent.radius = 8;
            ent.customShader = cgs.media.flare_circle_fadein;
            ent.shaderRGBA[3] = 200;
        }
        else if ( i == 8 )
        {
            ent.radius = 4;
            ent.customShader = cgs.media.flare_flare_turkis;
            ent.shaderRGBA[3] = 140;
        }
        else if ( i == 9 )
        {
            ent.radius = 12;
            ent.customShader = cgs.media.flare_circle_blue;
            ent.shaderRGBA[3] = 200;
        }
        else if ( i == 10 )
        {
            ent.radius = 8;
            ent.customShader = cgs.media.flare_circle_fadein;
            ent.shaderRGBA[3] = 180;
        }
        else if ( i == 11 )
        {
            ent.radius = 16;
            ent.customShader = cgs.media.flare_circle_rainbow;
            ent.shaderRGBA[3] = 240;
        }

        ent.shaderRGBA[3] *= radius;
        ent.radius *= radius;

        VectorMA( trace.endpos,  (distance/SUB_FLARES + 1)*i-30 , angles, ent.origin );

        trap_R_AddRefEntityToScene( &ent );
    }

    return;
}

void _CG_Flare( centity_t *cent, vec3_t lerpOrigin, float size, float r, float g, float b, qboolean showCorona )
{
    CG_Flare(cent,lerpOrigin,size,r,g,b,showCorona );
}
/*
==========================
CG_WaterBulletTrail
==========================
*/
void CG_RealBloodTrail( vec3_t start, vec3_t end, float spacing );
static void CG_WaterBulletTrail( centity_t *ent ) {
    int		step;
    vec3_t	origin, lastPos;
    int		t;
    int		startTime, contents;
    int		lastContents;
    entityState_t	*es;
    vec3_t	up;

    up[0] = 0;
    up[1] = 0;
    up[2] = 0;

    step = 35;

    es = &ent->currentState;
    startTime = ent->trailTime;
    t = step * ( (startTime + step) / step );

    BG_EvaluateTrajectory( &es->pos, cg.time, origin );
    contents = CG_PointContents( origin, -1 );

    // if object (e.g. grenade) is stationary, don't toss up smoke
    if ( es->pos.trType == TR_STATIONARY ) {
        ent->trailTime = cg.time;
        return;
    }

    BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
    lastContents = CG_PointContents( lastPos, -1 );

    ent->trailTime = cg.time;

    if ( contents & CONTENTS_WATER  ) {
        if ( contents & lastContents & CONTENTS_WATER ) {
            CG_BubbleTrail( lastPos, origin, 8 );
        }
        return;
    }

    /*
    for ( ; t <= ent->trailTime ; t += step ) {
    BG_EvaluateTrajectory( &es->pos, t, lastPos );


    smoke = CG_SmokePuff( lastPos, up,
    4,
    1, 1, 1, 0.33f,
    1000,
    cg.time,
    0, 0,
    cgs.media.smokePuffShader );
    // use the optimized local entity add
    smoke->leType = LE_SCALE_FADE;
    }*/
    // add trails
    if ( ent->currentState.powerups & ( 1 << 1 ) ) {
        /// blood trail
        CG_RealBloodTrail( lastPos, origin, 15 );
    }
    if ( ent->currentState.powerups & ( 1 << 2 ) ) {
        // water trail
        // blood trail
    }
    if ( ent->currentState.powerups & ( 1 << 3 ) ) {
        // misc trail.
    }
}
/*

/*
===============
CG_Bullet
===============
*/
static void CG_BulletEnt( centity_t *cent ) {
    entityState_t		*s1;
    vec3_t	oldOrigin;
    //	int	col;
    vec4_t		rgba;
    vec3_t	curOrigin;
    refEntity_t			ent;

    s1 = &cent->currentState;

    if ( s1->weapon > WP_NUM_WEAPONS ) {
        s1->weapon = 0;
    }


    // calculate the axis
    VectorCopy( s1->angles, cent->lerpAngles);

    // 'prediction'
    BG_EvaluateTrajectory( &cent->currentState.pos, cg.time,  curOrigin );
    //	CG_Printf("%s %s\n", vtos(cent->lerpOrigin ), vtos(curOrigin ) );
    VectorCopy( curOrigin, cent->lerpOrigin );

    CG_WaterBulletTrail( cent );

    // add dynamic light

    //	BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

    //	trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, cgs.media.tracerSound  );


    BG_EvaluateTrajectory( &cent->currentState.pos, cg.time - cg_bulletTracerLength.value, oldOrigin );

    rgba[0] = rgba[1] = rgba[2] = 1;
    rgba[3] = 0.75f;

    // create the render entity

    CG_Tracer( oldOrigin , cent->lerpOrigin, cg_bulletTracerWidth.value, cgs.media.metalsparkShader , rgba );

    if ( s1->otherEntityNum == 1 )
    {
        //		CG_Printf("#: %i oe: %i oe2: %i\n",s1->number,s1->otherEntityNum, s1->otherEntityNum2 );

        if ( !cg.cameraActive && !cg.cameraFollowNumber  && cent->currentState.otherEntityNum2-1 == cg.snap->ps.clientNum )
        {
            cg.cameraFollowNumber = s1->number;
            cg.cameraActive = qtrue;

            //	CG_Printf("Changed camera state to active and following %i \n", cg.cameraFollowNumber );
        }
        if (  cg.cameraActive && cg.cameraFollowNumber == s1->number && cent->currentState.otherEntityNum2-1 == cg.snap->ps.clientNum )
        {
            vec3_t oldOrigin2;
            vec3_t origin;
            trace_t trace;

            BG_EvaluateTrajectory( &s1->pos, cg.time, origin );

            if ( cg.cameraOrigin[0] == 0.0f && cg.cameraOrigin[1] == 0.0f && cg.cameraOrigin[2] == 0.0f )
            {
                //		CG_Printf("setted initial state.");
                VectorCopy( origin, cg.cameraOrigin );
            }
            VectorCopy( cg.cameraOrigin, oldOrigin2  );

            CG_Trace( &trace, oldOrigin2, NULL,NULL,origin, s1->number, MASK_SHOT );

            if ( trace.fraction == 1.0f )
            {
                VectorCopy( origin, cg.cameraOrigin  );
                cg.cameraRemainTime = cg.time + 1000; // remain 1.5s at the endpoint
                //	CG_Printf("Updated position.\n");
            }
        }
    }
    // create the render entity
    memset (&ent, 0, sizeof(ent));
    VectorCopy( cent->lerpOrigin, ent.origin);
    VectorCopy( cent->lerpOrigin, ent.oldorigin);

    if ( BG_IsShotgun(cent->currentState.weapon)  ) {
        ent.reType = RT_SPRITE;
        ent.radius = s1->frame / 3;
        ent.rotation = 0;
        ent.customShader = cgs.media.flare;
        trap_R_AddRefEntityToScene( &ent );
        return;
    }

    // flicker between two skins
    ent.hModel = cgs.gameModels[ s1->modelindex ];
    ent.renderfx = RF_NOSHADOW;


    // convert direction of travel into axis
    if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
        ent.axis[0][2] = 1;
    }

    // spin as it moves

    if ( s1->pos.trType != TR_STATIONARY ) {
        RotateAroundDirection( ent.axis, cg.time / 4 );
    } else {

        /*
        #ifdef MISSIONPACK
        if ( s1->weapon == WP_PROX_LAUNCHER ) {
        AnglesToAxis( cent->lerpAngles, ent.axis );
        }
        else
        #endif*/
        {
            RotateAroundDirection( ent.axis, s1->time );
        }
    }

    // add to refresh list, possibly with quad glow
    CG_AddRefEntityWithPowerups( &ent, s1, TEAM_FREE );
}

/*
===============
CG_Missile
===============
*/
static void CG_Missile( centity_t *cent ) {
    refEntity_t			ent;
    entityState_t		*s1;
    const weaponInfo_t		*weapon;
    //	int	col;

    s1 = &cent->currentState;

    if ( s1->weapon > WP_NUM_WEAPONS ) {
        s1->weapon = 0;
    }
    // if it's a 40mm grenade, use m4 / ak47 as weapon
    if ( ( s1->frame == WP_AK47 || s1->frame == WP_M4 ) && s1->weapon == WP_GRENADE )
        weapon = &cg_weapons[s1->frame];
    else
        weapon = &cg_weapons[s1->weapon];

    // calculate the axis
    VectorCopy( s1->angles, cent->lerpAngles);

    // add trails
    if ( s1->generic1 != MF_SMOKE ) 
        CG_40mmGrenadeTrail( cent, weapon ); 

    if ( s1->generic1 == MF_SMOKE )
    {
        vec3_t up, orig;
        localEntity_t *smoke;
        int seed = cent->currentState.frame & SEALS_SMOKEMASK_RNDNUM;
        int radius = SEALS_SMOKEPUFF_RADIUS + Q_random(&seed)*8;
        int dummy;

        dummy = (cent->currentState.frame & SEALS_SMOKEMASK_RIGHT) >> SEALS_SMOKEMASK_SRIGHT;
        dummy += (cent->currentState.frame & SEALS_SMOKEMASK_LEFT) >> SEALS_SMOKEMASK_SLEFT;
        dummy += (cent->currentState.frame & SEALS_SMOKEMASK_FORWARD) >> SEALS_SMOKEMASK_SFORWARD;
        dummy += (cent->currentState.frame & SEALS_SMOKEMASK_BACKWARD) >> SEALS_SMOKEMASK_SBACKWARD;
        dummy += (cent->currentState.frame & SEALS_SMOKEMASK_UP) >> SEALS_SMOKEMASK_SUP;
        radius *= 1.0 + (float)dummy/15.0;

        if ( CG_PointContents( cent->lerpOrigin, -1 ) &  CONTENTS_WATER   ) {

            VectorCopy( cent->lerpOrigin, up );

            up[0] += -10+Q_random(&seed)*20;
            up[1] += -10+Q_random(&seed)*20;
            up[2] += 5+Q_random(&seed)*5;

            CG_BubbleTrail( cent->lerpOrigin , up, 4 );
        }
        else {
            float distance;
            // smoke blend
            distance = Distance( cent->lerpOrigin, cg.refdef.vieworg );

            if ( distance < ( SEALS_SMOKEBLEND_RANGE * (1.0 + dummy / 15.0 ) ) ) {

                // check the directions for the additional blend effect
                if ( (32 + ((cent->currentState.frame & SEALS_SMOKEMASK_RIGHT) >> SEALS_SMOKEMASK_SRIGHT) *
                        (int)(SEALS_SMOKEBLEND_RANGE / SEALS_SMOKEMASK_VALUE)) >
                        (cg.refdef.vieworg[1] - cent->lerpOrigin[1])  )
                    cg.smokeBlendAlpha = 1.0f - distance/( SEALS_SMOKEBLEND_RANGE * (1.0 + dummy/15.0));
                else if ( (32 + ((cent->currentState.frame & SEALS_SMOKEMASK_LEFT) >> SEALS_SMOKEMASK_SLEFT) *
                           (int)(SEALS_SMOKEBLEND_RANGE / SEALS_SMOKEMASK_VALUE)) >
                          ( - cg.refdef.vieworg[1] + cent->lerpOrigin[1])  )
                    cg.smokeBlendAlpha = 1.0f - distance/( SEALS_SMOKEBLEND_RANGE * (1.0 + dummy/15.0));
                else if ( (32 + ((cent->currentState.frame & SEALS_SMOKEMASK_FORWARD) >> SEALS_SMOKEMASK_SFORWARD) *
                           (int)(SEALS_SMOKEBLEND_RANGE / SEALS_SMOKEMASK_VALUE)) >
                          ( cg.refdef.vieworg[0] - cent->lerpOrigin[0])  )
                    cg.smokeBlendAlpha = 1.0f - distance/( SEALS_SMOKEBLEND_RANGE * (1.0 + dummy/15.0));
                else if ( (32 + ((cent->currentState.frame & SEALS_SMOKEMASK_BACKWARD) >> SEALS_SMOKEMASK_SBACKWARD) *
                           (int)(SEALS_SMOKEBLEND_RANGE / SEALS_SMOKEMASK_VALUE)) >
                          ( - cg.refdef.vieworg[0] + cent->lerpOrigin[0])  )
                    cg.smokeBlendAlpha = 1.0f - distance/( SEALS_SMOKEBLEND_RANGE * (1.0 + dummy/15.0));
                else if ( (32 + ((cent->currentState.frame & SEALS_SMOKEMASK_UP) >> SEALS_SMOKEMASK_SUP) *
                           (int)(SEALS_SMOKEBLEND_RANGE / SEALS_SMOKEMASK_VALUE)) >
                          ( cg.refdef.vieworg[2] - cent->lerpOrigin[2])  )
                    cg.smokeBlendAlpha = 1.0f - distance/( SEALS_SMOKEBLEND_RANGE * (1.0 + dummy/15.0));

                if ( cg.smokeBlendAlpha > 0.8f )
                    cg.smokeBlendAlpha = 0.8f;
            }

            // main smokes
            if ( cent->trailTime < cg.time ) {

                // get the base speed for the smoke
                up[0] = -6.0 + Q_random(&seed)*12;
                up[1] = -6.0 + Q_random(&seed)*12;
                up[2] =  4.0 + Q_random(&seed)*6;

                // modify the speed according to open area stuff
                if (up[0] > 0.0)
                    up[0] += Q_random(&seed)*14.0* ((cent->currentState.frame & SEALS_SMOKEMASK_RIGHT) >> SEALS_SMOKEMASK_SRIGHT);
                if (up[0] < 0.0)
                    up[0] -= Q_random(&seed)*14.0* ((cent->currentState.frame & SEALS_SMOKEMASK_LEFT) >> SEALS_SMOKEMASK_SLEFT);

                if (up[1] > 0.0)
                    up[1] += Q_random(&seed)*14.0* ((cent->currentState.frame & SEALS_SMOKEMASK_FORWARD) >> SEALS_SMOKEMASK_SFORWARD);
                if (up[1] < 0.0)
                    up[1] -= Q_random(&seed)*14.0* ((cent->currentState.frame & SEALS_SMOKEMASK_BACKWARD) >> SEALS_SMOKEMASK_SBACKWARD);

                if ( up[2] > 0.0 )
                    up[2] += Q_random(&seed)*12.0* ((cent->currentState.frame & SEALS_SMOKEMASK_UP) >> SEALS_SMOKEMASK_SUP);

                VectorCopy(cent->lerpOrigin, orig);
                orig[2] += 4.0;

                switch (cgs.camoType) {
                case CAMO_URBAN:
                    smoke = CG_SmokePuff( orig,
                                          up,
                                          radius,
                                          SEALS_SMOKENADE_R_URBAN,
                                          SEALS_SMOKENADE_G_URBAN,
                                          SEALS_SMOKENADE_B_URBAN,
                                          1.0,
                                          SEALS_SMOKEPUFF_TIME,
                                          cg.time,
                                          0,
                                          LEF_PUFF_DONT_FADE,
                                          cgs.media.smokePuffShader );
                    break;
                case CAMO_ARCTIC:
                    smoke = CG_SmokePuff( orig,
                                          up,
                                          radius,
                                          SEALS_SMOKENADE_R_ARCTIC,
                                          SEALS_SMOKENADE_G_ARCTIC,
                                          SEALS_SMOKENADE_B_ARCTIC,
                                          1.0,
                                          SEALS_SMOKEPUFF_TIME,
                                          cg.time,
                                          0,
                                          LEF_PUFF_DONT_FADE,
                                          cgs.media.smokePuffShader );
                    break;
                case CAMO_DESERT:
                    smoke = CG_SmokePuff( orig,
                                          up,
                                          radius,
                                          SEALS_SMOKENADE_R_DESERT,
                                          SEALS_SMOKENADE_G_DESERT,
                                          SEALS_SMOKENADE_B_DESERT,
                                          1.0,
                                          SEALS_SMOKEPUFF_TIME,
                                          cg.time,
                                          0,
                                          LEF_PUFF_DONT_FADE,
                                          cgs.media.smokePuffShader );
                    break;
                case CAMO_JUNGLE:
                default:
                    smoke = CG_SmokePuff( orig,
                                          up,
                                          radius,
                                          SEALS_SMOKENADE_R_JUNGLE,
                                          SEALS_SMOKENADE_G_JUNGLE,
                                          SEALS_SMOKENADE_B_JUNGLE,
                                          1.0,
                                          SEALS_SMOKEPUFF_TIME,
                                          cg.time,
                                          0,
                                          LEF_PUFF_DONT_FADE,
                                          cgs.media.smokePuffShader );
                    break;
                }

                //smoke->leFlags = LEF_PUFF_DONT_FADE;
                //smoke->endTime = cg.time + 1000;

                cent->trailTime = cg.time + SEALS_SMOKENADETIME;
            }
        }
    }

    // add dynamic light
    if ( weapon->missileDlight ) {
        trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight,
                               weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
    }

    // add missile sound
    if ( weapon->missileSound ) {
        vec3_t	velocity;

        BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

        trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound );
    }


    // create the render entity
    memset (&ent, 0, sizeof(ent));
    VectorCopy( cent->lerpOrigin, ent.origin);
    VectorCopy( cent->lerpOrigin, ent.oldorigin);

    if ( cent->currentState.weapon == WP_MP5 ) {
        ent.reType = RT_SPRITE;
        ent.radius = 16;
        ent.rotation = 0;
        ent.customShader = trap_R_RegisterShader( "sprites/plasma1" );
        trap_R_AddRefEntityToScene( &ent );
        return;
    }

    // flicker between two skins
    ent.skinNum = cg.clientFrame & 1;
    ent.hModel = weapon->missileModel;
    ent.renderfx = RF_NOSHADOW;


    // convert direction of travel into axis
    if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
        ent.axis[0][2] = 1;
    }

    // spin as it moves
    if ( s1->pos.trType != TR_STATIONARY ) {
        RotateAroundDirection( ent.axis, cg.time / 4 );
    } else {
        /*
        #ifdef MISSIONPACK
        if ( s1->weapon == WP_PROX_LAUNCHER ) {
        AnglesToAxis( cent->lerpAngles, ent.axis );
        }
        else
        #endif*/
        {
            RotateAroundDirection( ent.axis, s1->time );
        }
    }

    // add to refresh list, possibly with quad glow
    CG_AddRefEntityWithPowerups( &ent, s1, TEAM_FREE );
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) {
    refEntity_t			ent;
    entityState_t		*s1;

    s1 = &cent->currentState;

    // create the render entity
    memset (&ent, 0, sizeof(ent));
    VectorCopy( cent->lerpOrigin, ent.origin);
    VectorCopy( cent->lerpOrigin, ent.oldorigin);
    AnglesToAxis( cent->lerpAngles, ent.axis );

    ent.renderfx = RF_NOSHADOW;

    // flicker between two skins (FIXME?)
    ent.skinNum = ( cg.time >> 6 ) & 1;

    // get the model, either as a bmodel or a modelindex
    if ( s1->solid == SOLID_BMODEL ) {
        ent.hModel = cgs.inlineDrawModel[s1->modelindex];
    } else {
        ent.hModel = cgs.gameModels[s1->modelindex];
    }

    // add to refresh list
    trap_R_AddRefEntityToScene(&ent);

    // add the secondary model
    if ( s1->modelindex2 ) {
        ent.skinNum = 0;
        ent.hModel = cgs.gameModels[s1->modelindex2];
        trap_R_AddRefEntityToScene(&ent);
    }

}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent ) {
    refEntity_t			ent;
    entityState_t		*s1;

    s1 = &cent->currentState;

    // create the render entity
    memset (&ent, 0, sizeof(ent));
    VectorCopy( s1->pos.trBase, ent.origin );
    VectorCopy( s1->origin2, ent.oldorigin );
    AxisClear( ent.axis );
    ent.reType = RT_BEAM;

    ent.renderfx = RF_NOSHADOW;

    // add to refresh list
    trap_R_AddRefEntityToScene(&ent);
}


/*
===============
CG_Portal
===============
*/
static void CG_Portal( centity_t *cent ) {
    refEntity_t			ent;
    entityState_t		*s1;

    s1 = &cent->currentState;

    // create the render entity
    memset (&ent, 0, sizeof(ent));
    VectorCopy( cent->lerpOrigin, ent.origin );
    VectorCopy( s1->origin2, ent.oldorigin );
    ByteToDir( s1->eventParm, ent.axis[0] );
    PerpendicularVector( ent.axis[1], ent.axis[0] );

    // negating this tends to get the directions like they want
    // we really should have a camera roll value
    VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

    CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
    ent.reType = RT_PORTALSURFACE;
    ent.oldframe = s1->powerups;
    ent.frame = s1->frame;		// rotation speed
    ent.skinNum = s1->clientNum/256.0 * 360; // roll offset

    // add to refresh list
    trap_R_AddRefEntityToScene(&ent);
}


/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out ) {
    centity_t	*cent;
    vec3_t	oldOrigin, origin, deltaOrigin;
    vec3_t	oldAngles, angles, deltaAngles;

    if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
        VectorCopy( in, out );
        return;
    }

    cent = &cg_entities[ moverNum ];
    if ( cent->currentState.eType != ET_MOVER &&
            cent->currentState.eType != ET_DOOR ) {
        VectorCopy( in, out );
        return;
    }

    BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
    BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

    BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
    BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

    VectorSubtract( origin, oldOrigin, deltaOrigin );
    VectorSubtract( angles, oldAngles, deltaAngles );

    VectorAdd( in, deltaOrigin, out );

    // FIXME: origin change when on a rotating object
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
    vec3_t		current, next;
    float		f;

    // it would be an internal error to find an entity that interpolates without
    // a snapshot ahead of the current one
    if ( cg.nextSnap == NULL ) {
        CG_Error( "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
    }

    if ( cent->currentState.eType == ET_MOVER ||
            cent->currentState.eType == ET_DOOR )
        return;

    f = cg.frameInterpolation;

    // this will linearize a sine or parabolic curve, but it is important
    // to not extrapolate player positions if more recent data is available
    BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
    BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

    cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
    cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
    cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

    BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
    BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

    cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
    cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
    cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

}

/*
===============
CG_CalcEntityLerpPositions

===============
*/
static void CG_CalcEntityLerpPositions( centity_t *cent ) {

    // if this player does not want to see extrapolated players
    if ( !cg_smoothClients.integer ) {
        // make sure the clients use TR_INTERPOLATE
        if ( cent->currentState.number < MAX_CLIENTS ) {
            cent->currentState.pos.trType = TR_INTERPOLATE;
            cent->nextState.pos.trType = TR_INTERPOLATE;
        }
    }

    if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
        CG_InterpolateEntityPosition( cent );
        return;
    }

    // first see if we can interpolate between two snaps for
    // linear extrapolated clients
    if ( cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP &&
            cent->currentState.number < MAX_CLIENTS) {
        CG_InterpolateEntityPosition( cent );
        return;
    }

    // just use the current frame and evaluate as best we can
    BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
    BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

    // adjust for riding a mover if it wasn't rolled into the predicted
    // player state
    if ( cent != &cg.predictedPlayerEntity ) {
        CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum,
                                   cg.snap->serverTime, cg.time, cent->lerpOrigin );
    }
}

/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) {
    // event-only entities will have been dealt with already
    if ( cent->currentState.eType >= ET_EVENTS ) {
        return;
    }

    // calculate the current origin
    CG_CalcEntityLerpPositions( cent );

    // add automatic effects
    CG_EntityEffects( cent );

    switch ( cent->currentState.eType ) {
    default:
        CG_Error( "Bad entity type: %i\n", cent->currentState.eType );
        break;
    case ET_INVISIBLE:
    case ET_PUSH_TRIGGER:
    case ET_TELEPORT_TRIGGER:
        break;
        /*	case ET_ACTOR:
        CG_Flare( cent, cent->lerpOrigin, 25, 0.3f,0.3f,1, qtrue );
        break;*/
    case ET_GENERAL:
        CG_General( cent );
        break;
    case ET_PLAYER:
        CG_Player( cent );
        break;
    case ET_ITEM:
        CG_Item( cent );
        break;
    case ET_FLARE:
        CG_Flare( cent, cent->lerpOrigin, cent->currentState.frame, cent->currentState.generic1, cent->currentState.otherEntityNum, cent->currentState.otherEntityNum2, cent->currentState.eventParm );
        break;
    case ET_MISSILE:
        CG_Missile( cent );
        break;
#ifdef PARTICLEHOST
    case ET_PARTICLEHOST:
        //	CG_Printf("particlehost:\n");
        CG_ParticleHost( cent );
        break;
#endif
    case ET_BULLET:
        CG_BulletEnt( cent );
        break;
    case ET_DOOR:
    case ET_FUNCEXPLOSIVE:
    case ET_ELEVBUT0:
    case ET_ELEVBUT1:
    case ET_ELEVBUT2:
    case ET_ELEVBUT3:
    case ET_ELEVBUT4:
    case ET_ELEVBUT5:
    case ET_ELEVBUT6:
    case ET_ELEVBUT7:
    case ET_MOVER:
        CG_Mover( cent );
        break;
    case ET_BEAM:
        CG_Beam( cent );
        break;
    case ET_PORTAL:
        CG_Portal( cent );
        break;
    case ET_SPEAKER:
        CG_Speaker( cent );
        break;
    case ET_GRAPPLE:
        break;
    }
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
    int					num;
    centity_t			*cent;
    playerState_t		*ps;

    // set cg.frameInterpolation
    if ( cg.nextSnap ) {
        int		delta;

        delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
        if ( delta == 0 ) {
            cg.frameInterpolation = 0;
        } else {
            cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
        }
    } else {
        cg.frameInterpolation = 0;	// actually, it should never be used, because
        // no entities should be marked as interpolating
    }

    // the auto-rotating items will all have the same axis
    cg.autoAngles[0] = 0;
    cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
    cg.autoAngles[2] = 90;

    cg.autoAnglesFast[0] = 0;
    cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
    cg.autoAnglesFast[2] = 0;

    AnglesToAxis( cg.autoAngles, cg.autoAxis );
    AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

    // generate and add the entity from the playerstate
    ps = &cg.predictedPlayerState;
    BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
    CG_AddCEntity( &cg.predictedPlayerEntity );

    // lerp the non-predicted value for lightning gun origins
    CG_CalcEntityLerpPositions( &cg_entities[ cg.snap->ps.clientNum ] );

    // add each entity sent over by the server
    for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
        cent = &cg_entities[ cg.snap->entities[ num ].number ];
        CG_AddCEntity( cent );
    }
}

