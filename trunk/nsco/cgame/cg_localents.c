// Copyright (C) 1999-2000 Id Software, Inc.
//

// cg_localents.c -- every frame, generate renderer commands for locally
// processed entities, like smoke puffs, gibs, shells, etc.

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

#define	MAX_LOCAL_ENTITIES	512
localEntity_t	cg_localEntities[MAX_LOCAL_ENTITIES];
localEntity_t	cg_activeLocalEntities;		// double linked list
localEntity_t	*cg_freeLocalEntities;		// single linked list

/*
===================
CG_InitLocalEntities

This is called at startup and for tournement restarts
===================
*/
void	CG_InitLocalEntities( void ) {
    int		i;

    memset( cg_localEntities, 0, sizeof( cg_localEntities ) );
    cg_activeLocalEntities.next = &cg_activeLocalEntities;
    cg_activeLocalEntities.prev = &cg_activeLocalEntities;
    cg_freeLocalEntities = cg_localEntities;
    for ( i = 0 ; i < MAX_LOCAL_ENTITIES - 1 ; i++ ) {

        cg_localEntities[i].next = &cg_localEntities[i+1];
    }
}


/*
==================
CG_FreeLocalEntity
==================
*/
void CG_FreeLocalEntity( localEntity_t *le ) {
    if ( !le->prev ) {
        CG_Error( "CG_FreeLocalEntity: not active" );
    }

    // remove from the doubly linked active list
    le->prev->next = le->next;
    le->next->prev = le->prev;

    // the free list is only singly linked
    le->next = cg_freeLocalEntities;
    cg_freeLocalEntities = le;
}

/*
===================
CG_AllocLocalEntity

Will allways succeed, even if it requires freeing an old active entity
===================
*/
localEntity_t	*CG_AllocLocalEntity( void ) {
    localEntity_t	*le;

    if ( !cg_freeLocalEntities ) {
        // no free entities, so free the one at the end of the chain
        // remove the oldest active entity
        CG_FreeLocalEntity( cg_activeLocalEntities.prev );
    }

    le = cg_freeLocalEntities;
    cg_freeLocalEntities = cg_freeLocalEntities->next;

    memset( le, 0, sizeof( *le ) );

    // link into the active list
    le->next = cg_activeLocalEntities.next;
    le->prev = &cg_activeLocalEntities;

    // make sure it gets rendered correctly
    le->leFlags &= ~LEF_3RDPERSON;

    cg_activeLocalEntities.next->prev = le;
    cg_activeLocalEntities.next = le;
    return le;
}


/*
====================================================================================

FRAGMENT PROCESSING

A fragment localentity interacts with the environment in some way (hitting walls),
or generates more localentities along a trail.

====================================================================================
*/
/*
===================
CG_AddRefEntity
===================
*/
void CG_AddRefEntity( localEntity_t *le ) {
    if (le->endTime < cg.time) {
        CG_FreeLocalEntity( le );
        return;
    }
    trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
================
CG_BloodTrail

Leave expanding blood puffs behind gibs
================
*/
void CG_BloodTrail( localEntity_t *le ) {
    int		t;
    int		b = (int)random()*5;
    float	size;
    int		t2;
    int		step;
    vec3_t	newOrigin;
    localEntity_t	*blood;

    step = 50;
    t = step * ( (cg.time - cg.frametime + step ) / step );
    t2 = step * ( cg.time / step );

    while ( b>4)
        b=(int)random()*5;

    size = 1 + random()*2;

    for ( ; t <= t2; t += step ) {
        BG_EvaluateTrajectory( &le->pos, t, newOrigin );
        // Navy Seals ++
        blood = CG_SmokePuff( newOrigin, vec3_origin,
                              size,		// radius
                              1, 1, 1, 0.7f,	// color
                              2000,		// trailTime
                              t,		// startTime
                              0,		// fadeInTime
                              0,		// flags
                              cgs.media.bloodparticleShaders[b] );

        // Navy Seals --
        // use the optimized version
        blood->leType = LE_FALL_SCALE_FADE;
        // drop a total of 40 units over its lifetime
        blood->pos.trDelta[2] = 40;
    }
}

void CG_DirectImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
                          float orientation, float red, float green, float blue, float alpha,
                          qboolean alphaFade, float radius, qboolean temporary, int entityNum );
/*
================
CG_FragmentBounceMark
================
*/
void CG_FragmentBounceMark( localEntity_t *le, trace_t *trace ) {
    float			radius;
    int i;
    qhandle_t shader;
    int max = 14;

    if ( le->leMarkType == LEMT_BLOOD ||
            le->leMarkType == LEMT_BLEEDER )
        radius = 0.75 + random()*2.5 + random()*2.5  + random();


    i = random()*max;

    if (i>max)
        i=max;

    if (i<0)
        i=0;

    if ( le->radius <= 5 )
    {
        max = 4;

        i = random()*max;

        if (i>max)
            i=max;

        shader = cgs.media.ns_bloodStainSmall[i];
    }
    else
        shader = cgs.media.ns_bloodStain[i];



    if ( le->leMarkType == LEMT_BLOOD ) {
        if ( trace->entityNum != ENTITYNUM_NONE )
            CG_DirectImpactMark( shader, trace->endpos, trace->plane.normal, random()*360,
                                 1,1,1,0.6 + random()/3, qfalse,  radius, qfalse,trace->entityNum );
        else
            CG_DirectImpactMark( shader, trace->endpos, trace->plane.normal, random()*360,
                                 1,1,1,0.6 + random()/3, qfalse,  radius, qfalse, -1 );
    }
    else if ( le->leMarkType == LEMT_BLEEDER ) {


        CG_ImpactMark( shader, trace->endpos, trace->plane.normal, random()*360,
                       1,1,1,0.6 + random()/3, qtrue, radius , qfalse );
    }

    // don't allow a fragment to make multiple marks, or they
    // pile up while settling
    le->leMarkType = LEMT_NONE;
}

/*
================
CG_FragmentBounceSound
================
*/
void CG_FragmentBounceSound( localEntity_t *le, trace_t *trace ) {
    if ( le->leBounceSoundType == LEBS_BLOOD ) {
        // half the gibs will make splat sounds
        /*
        if ( rand() & 1 ) {
        int r = rand()&3;
        sfxHandle_t	s;

        if ( r < 2 ) {
        s = cgs.media.gibBounce1Sound;
        } else if ( r == 2 ) {
        s = cgs.media.gibBounce2Sound;
        } else {
        s = cgs.media.gibBounce3Sound;
        }
        trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );
        } */
    } else if ( le->leBounceSoundType == LEBS_BRASS ) {
        int r = rand()&3;
        sfxHandle_t	s;

        if ( r < 2 ) {
            s = cgs.media.sfxShellHitWall[0];
        } else if ( r == 2 ) {
            s = cgs.media.sfxShellHitWall[1];
        } else {
            s = cgs.media.sfxShellHitWall[2];
        }
        if ((cg.DeafTime < cg.time)) trap_S_StartSound( trace->endpos, ENTITYNUM_WORLD, CHAN_AUTO, s );

    }

    // don't allow a fragment to make multiple bounce sounds,
    // or it gets too noisy as they settle
    le->leBounceSoundType = LEBS_NONE;
}


/*
================
CG_ReflectVelocity
================
*/
void CG_ReflectVelocity( localEntity_t *le, trace_t *trace ) {
    vec3_t	velocity;
    float	dot;
    int		hitTime;

    // reflect the velocity on the trace plane
    hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
    BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );
    dot = DotProduct( velocity, trace->plane.normal );
    VectorMA( velocity, -2*dot, trace->plane.normal, le->pos.trDelta );

    VectorScale( le->pos.trDelta, le->bounceFactor, le->pos.trDelta );

    VectorCopy( trace->endpos, le->pos.trBase );
    le->pos.trTime = cg.time;


    // check for stop, making sure that even on low FPS systems it doesn't bobble
    if ( trace->allsolid ||
            ( trace->plane.normal[2] > 0 &&
              ( le->pos.trDelta[2] < 40 || le->pos.trDelta[2] < -cg.frametime * le->pos.trDelta[2] ) ) ) {
        le->pos.trType = TR_STATIONARY;
    } else {

    }
}

/*
================
CG_AddFragment
================
*/
void CG_AddFragment( localEntity_t *le ) {
    vec3_t	newOrigin;
    trace_t	trace;

    // so it looks like things get spawned correctly :)
    if ( ( cg.time > ( le->startTime + 300 ) ) &&
            le->leFlags & LEF_3RDPERSON
       )
        le->leFlags &= ~LEF_3RDPERSON;


    if ( le->pos.trType == TR_STATIONARY ) {
        // sink into the ground if near the removal time
        int		t;
        float	oldZ;

        t = le->endTime - cg.time;
        if ( t < SINK_TIME ) {
            // we must use an explicit lighting origin, otherwise the
            // lighting would be lost as soon as the origin went
            // into the ground
            VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
            le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
            oldZ = le->refEntity.origin[2];
            le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
            trap_R_AddRefEntityToScene( &le->refEntity );
            le->refEntity.origin[2] = oldZ;
        } else {
            trap_R_AddRefEntityToScene( &le->refEntity );
        }

        return;
    }


    // calculate new position
    BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

    // trace a line from previous position to new position
    CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, CONTENTS_SOLID );
    if ( trace.fraction == 1.0 ) {
        // still in free fall
        VectorCopy( newOrigin, le->refEntity.origin );

        if ( le->leFlags & LEF_TUMBLE ) {
            vec3_t angles;

            BG_EvaluateTrajectory( &le->angles, cg.time, angles );
            AnglesToAxis( angles, le->refEntity.axis );
        }

        trap_R_AddRefEntityToScene( &le->refEntity );

        // add a blood trail
        if ( le->leBounceSoundType == LEBS_BLOOD )
            CG_BloodTrail( le );
        else if ( le->leBounceSoundType == LEBS_BLEEDER)
            CG_BleederTrail ( le ); // smaller!
        else if ( le->leBounceSoundType == LEBS_SPARK )
            CG_SparkTrail( le );

        return;
    }

    // if it is in a nodrop zone, remove it
    // this keeps gibs from waiting at the bottom of pits of death
    // and floating levels
    if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
        CG_FreeLocalEntity( le );
        return;
    }

    // leave a mark
    CG_FragmentBounceMark( le, &trace );

    // do a bouncy sound
    CG_FragmentBounceSound( le, &trace );

    // reflect the velocity on the trace plane
    CG_ReflectVelocity( le, &trace );

    trap_R_AddRefEntityToScene( &le->refEntity );
}

// Navy Seals ++

/*
==================
CG_AddParticle
==================
*/
vec3_t pMins = { -0.5,-0.5,-0.5 };
vec3_t pMaxs = {  0.5, 0.5, 0.5 };
static void CG_AddParticle ( localEntity_t *le ) {
    refEntity_t	*re;
    float		c;
    trace_t	trace;
    vec3_t	oldOrigin;

    re = &le->refEntity;

    if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
        // fade / grow time
        c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
    }
    else {
        // fade / grow time
        c = ( le->endTime - cg.time ) * le->lifeRate;
    }

    re->shaderRGBA[0] = 255 * le->color[0];
    re->shaderRGBA[1] = 255 * le->color[1];
    re->shaderRGBA[2] = 255 * le->color[2];
    re->shaderRGBA[3] = 255 * c;// * c;// * le->color[3];

    if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
        re->radius = le->radius * ( (/*1.0 -*/ c ) );
    }
    else if ( re->radius <= 0 || !re->radius )
        re->radius = le->radius;

    // early start ( might be stuck in a wall - no coldet)
    if ( cg.time < le->startTime + 50 )
    {
        BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );
    } else {
        // new position this frame
        BG_EvaluateTrajectory( &le->pos, cg.time, oldOrigin );
        // trace a line from previous position to new position
        CG_Trace(&trace, le->refEntity.origin, NULL, NULL, oldOrigin , -1, MASK_SOLID );
        // if touched ground
        if ( trace.fraction == 1 )
        {
            VectorCopy( oldOrigin, re->origin );
        }
        else
        {
            if ( le->leMarkType != LEMT_NONE )
            {
                CG_FragmentBounceMark( le, &trace );
            }

            CG_ReflectVelocity( le, &trace );
        }
    }

    trap_R_AddRefEntityToScene( &le->refEntity );
}

/*
==================
CG_AddShrapnel
==================
*/
static void CG_AddShrapnel( localEntity_t *le ) {
    vec3_t		org, org2;
    trace_t		tr;
    vec4_t		rgba;
    float	c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );

    if ( c > 1 ) {
        c = 1.0;	// can happen during connection problems
    }

    // fade / grow time
    rgba[0] = 1;
    rgba[1] = 1;
    rgba[2] = 1;
    rgba[3] = ( le->endTime - cg.time ) * le->lifeRate;

    // this is similar to the laserbeam in q2
    BG_EvaluateTrajectory( &le->pos, cg.time, org );

    // so they don't get stuck i a wall
    if ( cg.time - le->startTime < 50 && le->leMarkType != LEMT_BLOOD )
        return;

    if ( le->leFlags == cgs.media.metalsparkShader )
        BG_EvaluateTrajectory( &le->pos, cg.time - 50/*( ( cg.time - cg.oldTime )*1.5f )*/ , org2 );
    else
        BG_EvaluateTrajectory( &le->pos, cg.time - 30/*( ( cg.time - cg.oldTime )*1.25f )*/, org2 );


    CG_Trace(&tr, org, vec3_origin, vec3_origin, org2, -1, MASK_SOLID);

    // if we're in a sky remove us
    /*
    if ( tr.surfaceFlags & SURF_SKY )
    {
    CG_FreeLocalEntity( le );
    return;
    }*/

    // if we hit something reflect our velocity on plane
    if ( ( tr.fraction < 1 ) && le->bounceFactor > 0)
    {
        if ( le->leMarkType == LEMT_BLOOD )
        {
            CG_FragmentBounceMark( le, &tr );

            CG_FreeLocalEntity( le );
            return;
        }
        else
        {
            CG_ReflectVelocity( le, &tr );
        }
    }

    if (le->leFlags == cgs.media.metalsparkShader && le->radius < 0 )
    {
        refEntity_t			ent;
        float rad = -le->radius;


        float	r,g,b;

        memset( &ent, 0, sizeof( ent ) );
        ent.reType = RT_SPRITE;
        VectorCopy(tr.endpos, ent.origin );

        // scale out
        ent.radius = rad * 5 * c;

        ent.customShader = cgs.media.smallFlare;//le->leFlags;

        g = 255 * ( 1.0f - c );
        r = g;
        b = 50 + 400 * ( c/2 );

        if ( g < 0 )
            g = 0;
        if ( b < 50 )
            b = 50;

        // fade out
        ent.shaderRGBA[0] = r;// * c;
        ent.shaderRGBA[1] = g;
        ent.shaderRGBA[2] = b;
        ent.shaderRGBA[3] = 255 ;//* c;

        trap_R_AddRefEntityToScene(&ent);

        return;
    }

    if ( le->leFlags == cgs.media.waterBubbleShader )
    {
        refEntity_t			ent;

        memset( &ent, 0, sizeof( ent ) );
        ent.reType = RT_SPRITE;
        VectorCopy(tr.endpos, ent.origin );

        // scale out
        ent.radius = le->radius * 5 * c;

        ent.customShader = le->leFlags;//le->leFlags;

        // fade out
        ent.shaderRGBA[0] = 0 * c;
        ent.shaderRGBA[1] = 0 * c;
        ent.shaderRGBA[2] = 255 * c;
        ent.shaderRGBA[3] = 300 * c;

        trap_R_AddRefEntityToScene(&ent);
    }
    else if (le->leMarkType == LEMT_BLOOD ) {
        refEntity_t			ent;

        memset( &ent, 0, sizeof( ent ) );
        ent.reType = RT_SPRITE;
        VectorCopy(org, ent.origin );

        ent.radius = le->radius * c;

        ent.customShader = le->leFlags;

        trap_R_AddRefEntityToScene(&ent);
    }
    else
        CG_Tracer(org, tr.endpos, le->radius, le->leFlags  , rgba); // flags is shader



}


/*
==================
CG_AddTracer
==================
*/
#define TRACER_LENGTH	100

static void CG_AddTracer( localEntity_t *le ) {
    vec4_t		rgba;
    float		c = (float)( le->endTime - cg.time ) * le->lifeRate;

    // fade out
    rgba[0] = rgba[1] = rgba[2] = 1;
    rgba[3] = c;

    CG_Tracer( le->pos.trBase, le->angles.trBase, cg_tracerWidth.value, le->leFlags, rgba ); // flags is shader
}

// Navy Seals --
/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
void CG_AddFadeRGB( localEntity_t *le ) {
    refEntity_t *re;
    float c;

    re = &le->refEntity;

    c = ( le->endTime - cg.time ) * le->lifeRate;
    c *= 0xff;

    re->shaderRGBA[0] = le->color[0] * c;
    re->shaderRGBA[1] = le->color[1] * c;
    re->shaderRGBA[2] = le->color[2] * c;
    re->shaderRGBA[3] = le->color[3] * c;

    trap_R_AddRefEntityToScene( re );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
static void CG_AddMoveScaleFade( localEntity_t *le ) {
    refEntity_t	*re;
    float		c;
    //	vec3_t		delta;
    //	float		len;

    re = &le->refEntity;

    if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
        // fade / grow time
        c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
    }
    else {
        // fade / grow time
        c = ( le->endTime - cg.time ) * le->lifeRate;
    }

    if ( !( le->leFlags & LEF_PUFF_DONT_FADE ) ) {
        re->shaderRGBA[3] = 0xff * c * le->color[3];
    }
    else
    {
        re->shaderRGBA[3] = 0xff * le->color[3];
    }

    if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
        re->radius = le->radius * ( 1.0 - c ) ;
    }

    BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

    // if the view would be "inside" the sprite, kill the sprite
    // so it doesn't add too much overdraw
    /*	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
    len = VectorLength( delta );
    if ( len < le->radius ) {
    CG_FreeLocalEntity( le );
    return;
    }*/

    trap_R_AddRefEntityToScene( re );
}


/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
    refEntity_t	*re;
    float		c;
    vec3_t		delta;
    float		len;

    re = &le->refEntity;

    // fade / grow time
    c = ( le->endTime - cg.time ) * le->lifeRate;

    re->shaderRGBA[3] = 0xff * c * le->color[3];
    re->radius = le->radius * ( 1.0 - c ) + 8;

    // if the view would be "inside" the sprite, kill the sprite
    // so it doesn't add too much overdraw
    VectorSubtract( re->origin, cg.refdef.vieworg, delta );
    len = VectorLength( delta );
    if ( len < le->radius ) {
        CG_FreeLocalEntity( le );
        return;
    }

    trap_R_AddRefEntityToScene( re );
}


/*
=================
CG_AddFallScaleFade

This is just an optimized CG_AddMoveScaleFade
For blood mists that drift down, fade out, and are
removed if the view passes through them.
There are often 100+ of these, so it needs to be simple.
=================
*/
static void CG_AddFallScaleFade( localEntity_t *le ) {
    refEntity_t	*re;
    float		c;
    vec3_t		delta;
    float		len;

    re = &le->refEntity;

    // fade time
    c = ( le->endTime - cg.time ) * le->lifeRate;

    re->shaderRGBA[3] = 0xff * c * le->color[3];

    re->origin[2] = le->pos.trBase[2] - ( 1.0 - c ) * le->pos.trDelta[2];

    re->radius = le->radius * ( 1.0 - c ) + 16;

    // if the view would be "inside" the sprite, kill the sprite
    // so it doesn't add too much overdraw
    VectorSubtract( re->origin, cg.refdef.vieworg, delta );
    len = VectorLength( delta );
    if ( len < le->radius ) {
        CG_FreeLocalEntity( le );
        return;
    }

    trap_R_AddRefEntityToScene( re );
}



/*
================
CG_AddExplosion
================
*/
/*
================
CG_AddExplosion
================
*/
static void CG_AddExplosion( localEntity_t *ex ) {
    refEntity_t	*ent;
    int r;
    float radius;
    ent = &ex->refEntity;

    if(ent->radius)
        radius = ent->radius / 8.0;/// 8.0;
    else
        radius = 8.0;

    r = ent->renderfx & RF_EXPANDING;
    ent->renderfx &= ~RF_EXPANDING;

    // add the entity
    if(r)
    {
        int i, j;
        float dt = ((float) (cg.time - ex->startTime)) / ((float) ( ex->endTime - ex->startTime ));
        vec3_t r;
        r[0] = ent->oldorigin[0] * dt;
        r[1] = ent->oldorigin[1] * dt;
        r[2] = ent->oldorigin[2] * dt;
        //AnglesToAxis(r, ent->axis);
        //VectorScale(cg.refdef.viewaxis[0], -1, ent->axis[0]);
        VectorSubtract(cg.refdef.vieworg, ent->origin, ent->axis[0]);
        VectorNormalize(ent->axis[0]);
        VectorScale(cg.refdef.viewaxis[1], -1, ent->axis[1]);
        VectorNormalize(ent->axis[1]);
        ProjectPointOnPlane(ent->axis[1], ent->axis[1], ent->axis[0]);
        CrossProduct(ent->axis[0], ent->axis[1], ent->axis[2]);//VectorScale(cg.refdef.viewaxis[2], -1, ent->axis[2]);
        for(i = 0; i < 3; i++)
            for(j = 0; j < 3; j++)
                ent->axis[i][j] *= radius * sqrt(dt);
        ent->shaderRGBA[0] = 255 - (255 * dt);
        ent->shaderRGBA[1] = 255 - (255 * dt);
        ent->shaderRGBA[2] = 255 - (255 * dt);
        ent->shaderRGBA[3] = 255 - (255 * dt);
        //VectorMA(ex->pos.trBase, -8, ent->axis[2], ent->origin);
    }

    trap_R_AddRefEntityToScene(ent);
    ent->renderfx |= r;
    // add the dlight
    if ( ex->light ) {
        float		light;

        light = (float)( cg.time - ex->startTime ) / ( ex->endTime - ex->startTime );
        if ( light < 0.5 ) {
            light = 1.0;
        } else {
            light = 1.0 - ( light - 0.5 ) * 2;
        }
        light = ex->light * light;
        trap_R_AddLightToScene(ent->origin, light, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2] );
    }
}


/*
================
CG_AddSpriteExplosion
================
*/
static void CG_AddSpriteExplosion( localEntity_t *le ) {
    refEntity_t	re;
    float c;

    re = le->refEntity;

    c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
    if ( c > 1 ) {
        c = 1.0;	// can happen during connection problems
    }

    re.shaderRGBA[0] = 0xff;
    re.shaderRGBA[1] = 0xff;
    re.shaderRGBA[2] = 0xff;
    re.shaderRGBA[3] = 0xff * c * 0.33;

    re.reType = RT_SPRITE;
    re.radius = 42 * ( 1.0 - c ) + 30;

    trap_R_AddRefEntityToScene( &re );

    // add the dlight
    if ( le->light ) {
        float		light;

        light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
        if ( light < 0.5 ) {
            light = 1.0;
        } else {
            light = 1.0 - ( light - 0.5 ) * 2;
        }
        light = le->light * light;
        trap_R_AddLightToScene(re.origin, light, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
    }
}


/*
===================
CG_RemoveAllFragments

removes all fragments in game
===================
*/
void CG_RemoveAllFragments( void )
{
    localEntity_t	*le, *next;

    // walk the list backwards, so any new local entities generated
    // (trails, marks, etc) will be revmoed this frame
    le = cg_activeLocalEntities.prev;
    for ( ; le != &cg_activeLocalEntities ; le = next ) {
        // grab next now, so if the local entity is freed we
        // still have it
        next = le->prev;

        // no fragment? then leave
        if ( le->leType != LE_FRAGMENT )
            continue;

        CG_FreeLocalEntity( le );
    }
}
//==============================================================================

/*
===================
CG_AddLocalEntities

qboolean thirdperson ? renders thirdperson items only (with different fov)
===================
*/
void CG_AddLocalEntities( qboolean thirdperson ) {
    localEntity_t	*le, *next;

    // walk the list backwards, so any new local entities generated
    // (trails, marks, etc) will be present this frame
    le = cg_activeLocalEntities.prev;
    for ( ; le != &cg_activeLocalEntities ; le = next ) {
        // grab next now, so if the local entity is freed we
        // still have it
        next = le->prev;

        if ( cg.time >= le->endTime ) {
            CG_FreeLocalEntity( le );
            continue;
        }

#ifndef SAME_WEAPONPIPE
        // because these got the leFlags reservd for other things
        if ( le->leType != LE_SHRAPNEL )
        {
            if ( !thirdperson && (le->leFlags & LEF_3RDPERSON) )
                continue;

            if ( thirdperson && !(le->leFlags & LEF_3RDPERSON) )
                continue;
        }
        else
        {
            if ( thirdperson )
                continue;
        }
#endif

        switch ( le->leType ) {
        default:
            CG_Error( "Bad leType: %i", le->leType );
            break;

        case LE_MARK:
            break;
        case LE_PARTICLE:
            CG_AddParticle( le );
            break;
            // Navy Seals ++
        case LE_TRACER:
            CG_AddTracer( le );
            break;
        case LE_SHRAPNEL:
            CG_AddShrapnel( le );
            break;
            // Navy Seals --
        case LE_SPRITE_EXPLOSION:
            CG_AddSpriteExplosion( le );
            break;

        case LE_EXPLOSION:
            CG_AddExplosion( le );
            break;

        case LE_FRAGMENT:			// gibs and brass
            CG_AddFragment( le );
            break;

        case LE_MOVE_SCALE_FADE:		// water bubbles
            CG_AddMoveScaleFade( le );
            break;

        case LE_FADE_RGB:				// teleporters, railtrails
            CG_AddFadeRGB( le );
            break;

        case LE_FALL_SCALE_FADE: // gib blood trails
            CG_AddFallScaleFade( le );
            break;

        case LE_SCALE_FADE:		// rocket trails
            CG_AddScaleFade( le );
            break;

        case LE_SCOREPLUM:
            break;

        case LE_SHOWREFENTITY:
            CG_AddRefEntity( le );
            break;

        }
    }
}




