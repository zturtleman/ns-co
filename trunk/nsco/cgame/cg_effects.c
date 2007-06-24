// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

static qhandle_t fogshader = -1;

void CG_VolumetricFog( vec3_t source, vec3_t dest ) {
    vec3_t		forward, right;
    polyVert_t	verts[4];
    vec3_t		line;
    float		len, begin, end;
    vec3_t		start, finish;
    float width = 2.0;
    vec4_t rgba;

    if (fogshader < 0) fogshader = trap_R_RegisterShader("nsq3_sfx/volumetricfog");

    rgba[0] = 1.0;
    rgba[1] = 1.0;
    rgba[2] = 1.0;
    rgba[3] = 1.0;

    VectorSubtract( dest, source, forward );
    len = VectorNormalize( forward );

    begin = 1;
    end = len;

    VectorMA( source, begin, forward, start );
    VectorMA( source, end, forward, finish );
    line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
    line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

    VectorScale( cg.refdef.viewaxis[1], line[1], right );
    VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
    VectorNormalize( right );

    VectorMA( finish, width, right, verts[0].xyz );
    verts[0].st[0] = 0;
    verts[0].st[1] = 1;
    verts[0].modulate[0] = 255 * rgba[0];
    verts[0].modulate[1] = 255 * rgba[1];
    verts[0].modulate[2] = 255 * rgba[2];
    verts[0].modulate[3] = 255 * rgba[3];

    VectorMA( finish, -width, right, verts[1].xyz );
    verts[1].st[0] = 1;
    verts[1].st[1] = 1;
    verts[1].modulate[0] = 255 * rgba[0];
    verts[1].modulate[1] = 255 * rgba[1];
    verts[1].modulate[2] = 255 * rgba[2];
    verts[1].modulate[3] = 255 * rgba[3];

    VectorMA( start, -width, right, verts[2].xyz );
    verts[2].st[0] = 1;
    verts[2].st[1] = 0;
    verts[2].modulate[0] = 255 * rgba[0];
    verts[2].modulate[1] = 255 * rgba[1];
    verts[2].modulate[2] = 255 * rgba[2];
    verts[2].modulate[3] = 255 * rgba[3];

    VectorMA( start, width, right, verts[3].xyz );
    verts[3].st[0] = 0;
    verts[3].st[1] = 0;
    verts[3].modulate[0] = 255 * rgba[0];
    verts[3].modulate[1] = 255 * rgba[1];
    verts[3].modulate[2] = 255 * rgba[2];
    verts[3].modulate[3] = 255 * rgba[3];

    trap_R_AddPolyToScene( fogshader, 4, verts );

}

/*
==================
CG_RealBloodTrail

Bullets shot underwater
==================
*/
void CG_RealBloodTrail( vec3_t start, vec3_t end, float spacing ) {
    vec3_t		move;
    vec3_t		vec;
    float		len;
    int			i;
    int			type = 5;

    VectorCopy (start, move);
    VectorSubtract (end, start, vec);
    len = VectorNormalize (vec);

    while(type > 4)
        type = (int)random()*5;

    // advance a random amount first
    i = rand() % (int)spacing;
    VectorMA( move, i, vec, move );

    VectorScale (vec, spacing, vec);

    for ( ; i < len; i += spacing ) {
        localEntity_t	*le;
        refEntity_t		*re;

        le = CG_AllocLocalEntity();
        le->leFlags = LEF_PUFF_DONT_SCALE;
        le->leType = LE_MOVE_SCALE_FADE;//LE_MOVE_SCALE_FADE;
        le->startTime = cg.time;
        le->endTime = cg.time + 2000 + random() * 250;
        le->lifeRate = 1.0 / ( le->endTime - le->startTime );

        re = &le->refEntity;
        re->shaderTime = cg.time / 1000.0f;

        re->reType = RT_SPRITE;
        re->rotation = rand() % 360;
        re->radius = 1 + random()*2;
        re->customShader = cgs.media.bloodparticleShaders[type];
        re->shaderRGBA[0] = 0xff;
        re->shaderRGBA[1] = 0xff;
        re->shaderRGBA[2] = 0xff;
        re->shaderRGBA[3] = 0xff;

        le->color[3] = 1.0;

        le->pos.trType = TR_LINEAR;
        le->pos.trTime = cg.time;
        VectorCopy( move, le->pos.trBase );

        //	le->pos.trDelta[0] = crandom()*10;
        //	le->pos.trDelta[1] = crandom()*10;
        le->pos.trDelta[2] = ( 10 + crandom()*10  ) *-1;

        VectorAdd (move, vec, move);
    }
}

/*
==================
CG_Bubble
Bullets shot underwater
==================
*/
void CG_Bubble ( vec3_t start, vec3_t velocity, int time, float radius, float alpha ) {
    localEntity_t	*le;
    refEntity_t		*re;

    // advance a random amount first

    le = CG_AllocLocalEntity();
    le->leFlags = LEF_PUFF_DONT_SCALE;
    le->leType = LE_MOVE_SCALE_FADE;
    le->startTime = cg.time;
    le->endTime = cg.time + time + random() * 250;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );

    re = &le->refEntity;
    re->shaderTime = cg.time / 1000.0f;

    re->reType = RT_SPRITE;
    re->rotation = 0;
    re->radius = radius;
    re->customShader = cgs.media.waterBubbleShader;
    re->shaderRGBA[0] = 0xff;
    re->shaderRGBA[1] = 0xff;
    re->shaderRGBA[2] = 0xff;
    re->shaderRGBA[3] = 0xff;

    le->color[3] = alpha;

    le->pos.trType = TR_LINEAR;
    le->pos.trTime = cg.time;
    VectorCopy( start, le->pos.trBase );
    VectorScale( velocity, 150 + test_h.value,  le->pos.trDelta );
}

/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing ) {
    vec3_t		move;
    vec3_t		vec;
    float		len;
    int			i;

    VectorCopy (start, move);
    VectorSubtract (end, start, vec);
    len = VectorNormalize (vec);

    // advance a random amount first
    i = rand() % (int)spacing;
    VectorMA( move, i, vec, move );

    VectorScale (vec, spacing, vec);

    for ( ; i < len; i += spacing ) {
        localEntity_t	*le;
        refEntity_t		*re;

        le = CG_AllocLocalEntity();
        le->leFlags = LEF_PUFF_DONT_SCALE;
        le->leType = LE_MOVE_SCALE_FADE;
        le->startTime = cg.time;
        le->endTime = cg.time + 1000 + random() * 250;
        le->lifeRate = 1.0 / ( le->endTime - le->startTime );

        re = &le->refEntity;
        re->shaderTime = cg.time / 1000.0f;

        re->reType = RT_SPRITE;
        re->rotation = 0;
        re->radius = 2;
        re->customShader = cgs.media.waterBubbleShader;
        re->shaderRGBA[0] = 0xff;
        re->shaderRGBA[1] = 0xff;
        re->shaderRGBA[2] = 0xff;
        re->shaderRGBA[3] = 0xff;

        le->color[3] = 1.0;

        le->pos.trType = TR_LINEAR;
        le->pos.trTime = cg.time;
        VectorCopy( move, le->pos.trBase );
        le->pos.trDelta[0] = crandom()*5;
        le->pos.trDelta[1] = crandom()*5;
        le->pos.trDelta[2] = crandom()*5 + 6;

        VectorAdd (move, vec, move);
    }
}

/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t *CG_SmokePuff( const vec3_t p, const vec3_t vel,
                             float radius,
                             float r, float g, float b, float a,
                             float duration,
                             int startTime,
                             int fadeInTime,
                             int leFlags,
                             qhandle_t hShader ) {
    static int	seed = 0x92;
    localEntity_t	*le;
    refEntity_t		*re;
    //	int fadeInTime = startTime + duration / 2;

    le = CG_AllocLocalEntity();
    le->leFlags = leFlags;
    le->radius = radius;

    re = &le->refEntity;
    re->rotation = Q_random( &seed ) * 360;
    re->radius = radius;
    re->shaderTime = startTime / 1000.0f;

    le->leType = LE_MOVE_SCALE_FADE;
    le->startTime = startTime;
    le->fadeInTime = fadeInTime;
    le->endTime = startTime + duration;
    if ( fadeInTime > startTime ) {
        le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
    }
    else {
        le->lifeRate = 1.0 / ( le->endTime - le->startTime );
    }
    le->color[0] = r;
    le->color[1] = g;
    le->color[2] = b;
    le->color[3] = a;


    le->pos.trType = TR_LINEAR;
    le->pos.trTime = startTime;
    VectorCopy( vel, le->pos.trDelta );
    VectorCopy( p, le->pos.trBase );

    VectorCopy( p, re->origin );
    re->customShader = hShader;

    // light smoke
    CG_LightParticles( re->origin, le->color, 0.4f );

    // rage pro can't alpha fade, so use a different shader
    if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
        re->customShader = cgs.media.smokePuffRageProShader;
        re->shaderRGBA[0] = 0xff;
        re->shaderRGBA[1] = 0xff;
        re->shaderRGBA[2] = 0xff;
        re->shaderRGBA[3] = 0xff;
    } else {
        re->shaderRGBA[0] = le->color[0] * 0xff;
        re->shaderRGBA[1] = le->color[1] * 0xff;
        re->shaderRGBA[2] = le->color[2] * 0xff;
        re->shaderRGBA[3] = 0xff;
    }

    re->reType = RT_SPRITE;
    re->radius = le->radius;

    return le;
}

/*
==================
CG_SpawnEffect

Player teleporting in or out
==================
*/
localEntity_t *CG_SpawnParticle( vec3_t org, vec3_t dir, float speed, float bouncefactor, float radius, float r,float g,float b,float a, qboolean size );

void CG_SpawnEffect( vec3_t org ) {
    int i = 0;
    int max = 64;
    vec3_t origin;
    vec3_t dir = { 0, 0, 2 };

    for ( i = 0; i < 64; i++ )
    {
        localEntity_t *ent;

        VectorCopy( org, origin );

        origin[0] += -15 + random()*30;
        origin[1] += -15 + random()*30;
        origin[2] += -30 + random()*30;

        ent = CG_SpawnParticle( origin, dir, 200, 0.3f, 1, 0.3f,0.3f,1.0f - random()/2,1.0f, qfalse );

        ent->endTime = cg.time + 500;
    }

}


/*
==================
CG_ScorePlum
==================
*/
void CG_ScorePlum( int client, vec3_t org, int score ) {
    localEntity_t	*le;
    refEntity_t		*re;
    vec3_t			angles;
    static vec3_t lastPos;

    // only visualize for the client that scored
    if (client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0) {
        return;
    }

    le = CG_AllocLocalEntity();
    le->leFlags = 0;
    le->leType = LE_SCOREPLUM;
    le->startTime = cg.time;
    le->endTime = cg.time + 4000;
    le->lifeRate = 1.0 / ( le->endTime - le->startTime );


    le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
    le->radius = score;

    VectorCopy( org, le->pos.trBase );
    if (org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20) {
        le->pos.trBase[2] -= 20;
    }

    //CG_Printf( "Plum origin %i %i %i -- %i\n", (int)org[0], (int)org[1], (int)org[2], (int)Distance(org, lastPos));
    VectorCopy(org, lastPos);


    re = &le->refEntity;

    re->reType = RT_SPRITE;
    re->radius = 16;

    VectorClear(angles);
    AnglesToAxis( angles, re->axis );
}


/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
                                 qhandle_t hModel, qhandle_t shader,
                                 int msec, qboolean isSprite ) {
    float			ang;
    localEntity_t	*ex;
    int				offset;
    vec3_t			tmpVec, newOrigin;

    if ( msec <= 0 ) {
        CG_Error( "CG_MakeExplosion: msec = %i", msec );
    }

    // skew the time a bit so they aren't all in sync
    offset = rand() & 63;

    ex = CG_AllocLocalEntity();
    if ( isSprite ) {
        ex->leType = LE_SPRITE_EXPLOSION;

        // randomly rotate sprite orientation
        ex->refEntity.rotation = rand() % 360;
        VectorScale( dir, 16, tmpVec );
        VectorAdd( tmpVec, origin, newOrigin );
    } else {
        ex->leType = LE_EXPLOSION;
        VectorCopy( origin, newOrigin );

        // set axis with random rotate
        if ( !dir ) {
            AxisClear( ex->refEntity.axis );
        } else {
            ang = rand() % 360;
            VectorCopy( dir, ex->refEntity.axis[0] );
            RotateAroundDirection( ex->refEntity.axis, ang );
        }
    }
    ex->startTime = cg.time - offset;
    ex->endTime = ex->startTime + msec;

    // bias the time so all shader effects start correctly
    ex->refEntity.shaderTime = ex->startTime / 1000.0f;

    ex->refEntity.hModel = hModel;
    ex->refEntity.customShader = shader;

    // set origin
    VectorCopy( newOrigin, ex->refEntity.origin );
    VectorCopy( newOrigin, ex->refEntity.oldorigin );
    VectorCopy( newOrigin, ex->pos.trBase );

    ex->color[0] = ex->color[1] = ex->color[2] = 1.0;

    return ex;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed( vec3_t origin, int entityNum ) {
    localEntity_t	*ex;

    if ( !cg_blood.integer ) {
        return;
    }

    ex = CG_AllocLocalEntity();
    ex->leType = LE_EXPLOSION;

    ex->startTime = cg.time;
    ex->endTime = ex->startTime + 500;

    VectorCopy ( origin, ex->refEntity.origin);
    ex->refEntity.reType = RT_SPRITE;
    ex->refEntity.rotation = rand() % 360;
    ex->refEntity.radius = 4;

    ex->refEntity.customShader = cgs.media.bloodExplosionShader;

    // don't show player's own blood in view
    if ( entityNum == cg.snap->ps.clientNum ) {
        ex->refEntity.renderfx |= RF_THIRD_PERSON;
    }
}



/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchGib( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    le->leType = LE_FRAGMENT;
    le->startTime = cg.time;
    le->endTime = le->startTime + 5000 + random() * 3000;

    VectorCopy( origin, re->origin );
    AxisCopy( axisDefault, re->axis );
    re->hModel = hModel;

    le->pos.trType = TR_GRAVITY;
    VectorCopy( origin, le->pos.trBase );
    VectorCopy( velocity, le->pos.trDelta );
    le->pos.trTime = cg.time;

    le->bounceFactor = 0.6f;

    le->leBounceSoundType = LEBS_BLOOD;
    le->leMarkType = LEMT_BLOOD;
}

// Navy Seals ++
/*
==================
CG_LaunchRocketSpark
==================
*/
void CG_LaunchRocketSpark( vec3_t origin, vec3_t velocity  ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    le->leType = LE_FRAGMENT;
    le->startTime = cg.time;
    le->endTime = le->startTime + 4000;

    VectorCopy( origin, re->origin );
    AxisCopy( axisDefault, re->axis );
    re->hModel = cgs.media.nullModel;
    //	re->reType = RT_SPRITE;

    le->pos.trType = TR_GRAVITY;
    VectorCopy( origin, le->pos.trBase );
    VectorCopy( velocity, le->pos.trDelta );
    le->pos.trTime = cg.time;

    le->bounceFactor = 0.5;

    //	re->radius = 16;
    //	le->radius = 1;

    le->leBounceSoundType = LEBS_SPARK;
    //	le->leMarkType = LEMT_BLOOD;
}
// Navy Seals --

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayer( vec3_t playerOrigin ) {
#if 0
    vec3_t	origin, velocity;

    if ( !cg_blood.integer ) {
        return;
    }

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    if ( rand() & 1 ) {
        CG_LaunchGib( origin, velocity, cgs.media.gibSkull );
    } else {
        CG_LaunchGib( origin, velocity, cgs.media.gibBrain );
    }

    // allow gibs to be turned off for speed
    if ( !cg_gibs.integer ) {
        return;
    }

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibAbdomen );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibArm );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibChest );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibFist );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibFoot );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibForearm );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibIntestine );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibLeg );

    VectorCopy( playerOrigin, origin );
    velocity[0] = crandom()*GIB_VELOCITY;
    velocity[1] = crandom()*GIB_VELOCITY;
    velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
    CG_LaunchGib( origin, velocity, cgs.media.gibLeg );
#endif
}

/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchExplode( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
    localEntity_t	*le;
    refEntity_t		*re;

    le = CG_AllocLocalEntity();
    re = &le->refEntity;

    le->leType = LE_FRAGMENT;
    le->startTime = cg.time;
    le->endTime = le->startTime + 10000 + random() * 6000;

    VectorCopy( origin, re->origin );
    AxisCopy( axisDefault, re->axis );
    re->hModel = hModel;

    le->pos.trType = TR_GRAVITY;
    VectorCopy( origin, le->pos.trBase );
    VectorCopy( velocity, le->pos.trDelta );
    le->pos.trTime = cg.time;

    le->bounceFactor = 0.1f;

    le->leBounceSoundType = LEBS_BRASS;
    le->leMarkType = LEMT_NONE;
}


/*
Explosion Sparks

randomly shoots out "rocket sparks"
*/
void CG_ExplosionSparks( vec3_t playerOrigin )
{
    vec3_t	origin, velocity;
    //	int		i = 0;

    if ( !cg_grenadeSparks.value )
        return;

    VectorCopy( playerOrigin, origin );

    origin[0] += random()*20;
    origin[1] += random()*20;
    origin[2] += 20 + crandom()*30;

    velocity[0] = 150 + crandom()*150;
    velocity[1] = 250 + crandom()*150;
    velocity[2] = 50 + random()*150;
    CG_LaunchRocketSpark( origin, velocity  );

    VectorCopy( playerOrigin, origin );

    velocity[0] = 150 + crandom()*150;
    velocity[1] = 150 + crandom()*150;
    velocity[2] = 50 + random()*150;
    CG_LaunchRocketSpark( origin, velocity  );

    velocity[0] = 150 + crandom()*150;
    velocity[1] = 250 + crandom()*150;
    velocity[2] = 50 + random()*150;
    CG_LaunchRocketSpark( origin, velocity  );
}

