// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_marks.c -- wall marks

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

/*
===================================================================

MARK POLYS

===================================================================
*/


markPoly_t	cg_activeMarkPolys;			// double linked list
markPoly_t	*cg_freeMarkPolys;			// single linked list
markPoly_t	cg_markPolys[MAX_MARK_POLYS];
static		int	markTotal;
extern	vmCvar_t	cg_lightmarks;
int CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts );

/*
===================
CG_InitMarkPolys

This is called at startup and for tournement restarts
===================
*/
void	CG_InitMarkPolys( void ) {
    int		i;

    memset( cg_markPolys, 0, sizeof(cg_markPolys) );

    cg_activeMarkPolys.nextMark = &cg_activeMarkPolys;
    cg_activeMarkPolys.prevMark = &cg_activeMarkPolys;
    cg_freeMarkPolys = cg_markPolys;
    for ( i = 0 ; i < MAX_MARK_POLYS - 1 ; i++ ) {
        cg_markPolys[i].nextMark = &cg_markPolys[i+1];
    }
}


/*
==================
CG_FreeMarkPoly
==================
*/
void CG_FreeMarkPoly( markPoly_t *le ) {
    if ( !le->prevMark ) {
        CG_Error( "CG_FreeLocalEntity: not active" );
    }

    // remove from the doubly linked active list
    le->prevMark->nextMark = le->nextMark;
    if ( le->nextMark ) {
        le->nextMark->prevMark = le->prevMark;
    }
    // the free list is only singly linked
    le->nextMark = cg_freeMarkPolys;
    cg_freeMarkPolys = le;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markPoly_t	*CG_AllocMark( void ) {
    markPoly_t	*le;
    int time;

    if ( !cg_freeMarkPolys ) {
        // no free entities, so free the one at the end of the chain
        // remove the oldest active entity
        time = cg_activeMarkPolys.prevMark->time;
        while (cg_activeMarkPolys.prevMark && time == cg_activeMarkPolys.prevMark->time) {
            CG_FreeMarkPoly( cg_activeMarkPolys.prevMark );
        }
    }

    le = cg_freeMarkPolys;
    cg_freeMarkPolys = cg_freeMarkPolys->nextMark;

    memset( le, 0, sizeof( *le ) );

    // link into the active list
    le->nextMark = cg_activeMarkPolys.nextMark;
    le->prevMark = &cg_activeMarkPolys;
    cg_activeMarkPolys.nextMark->prevMark = le;
    cg_activeMarkPolys.nextMark = le;
    return le;
}



/*
=================
CG_ImpactMark

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================
*/
#define	MAX_MARK_FRAGMENTS	128
#define	MAX_MARK_POINTS		384

void CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
                    float orientation, float red, float green, float blue, float alpha,
                    qboolean alphaFade, float radius, qboolean temporary ) {
    vec3_t			axis[3];
    float			texCoordScale;
    vec3_t			originalPoints[4];
    byte			colors[4];
    int				i, j;
    int				numFragments;
    markFragment_t	markFragments[MAX_MARK_FRAGMENTS], *mf;
    vec3_t			markPoints[MAX_MARK_POINTS];
    vec3_t			projection;

    if ( !cg_addMarks.integer ) {
        return;
    }

    if ( radius <= 0 ) {
        CG_Error( "CG_ImpactMark called with <= 0 radius" );
    }

    //if ( markTotal >= MAX_MARK_POLYS ) {
    //	return;
    //}

    // create the texture axis
    VectorNormalize2( dir, axis[0] );
    PerpendicularVector( axis[1], axis[0] );
    RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
    CrossProduct( axis[0], axis[2], axis[1] );

    texCoordScale = 0.5 * 1.0 / radius;

    // create the full polygon
    for ( i = 0 ; i < 3 ; i++ ) {
        originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
        originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
        originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
        originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
    }

    // get the fragments
    if ( markShader == cgs.media.wakeMarkShader )
        VectorScale( dir, -1, projection );
    else
        VectorScale( dir, -20, projection );

    numFragments = trap_CM_MarkFragments( 4, (void *)originalPoints,
                                          projection, MAX_MARK_POINTS, markPoints[0],
                                          MAX_MARK_FRAGMENTS, markFragments );

    colors[0] = red * 255;
    colors[1] = green * 255;
    colors[2] = blue * 255;
    colors[3] = alpha * 255;

    for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ ) {
        polyVert_t	*v;
        polyVert_t	verts[MAX_VERTS_ON_POLY];
        markPoly_t	*mark;

        // we have an upper limit on the complexity of polygons
        // that we store persistantly
        if ( mf->numPoints > MAX_VERTS_ON_POLY ) {
            mf->numPoints = MAX_VERTS_ON_POLY;
        }
        for ( j = 0, v = verts ; j < mf->numPoints ; j++, v++ ) {
            vec3_t		delta;

            VectorCopy( markPoints[mf->firstPoint + j], v->xyz );

            VectorSubtract( v->xyz, origin, delta );
            v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
            v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
            *(int *)v->modulate = *(int *)colors;
        }

        // if it is a temporary (shadow) mark, add it immediately and forget about it
        if ( temporary  ) {

            if ( cg_lightmarks.integer )
            {
                vec3_t normal;

                VectorCopy( dir, normal );

                CG_LightVerts( normal,  mf->numPoints , verts);
            }

            trap_R_AddPolyToScene( markShader, mf->numPoints, verts );
            continue;
        }

        // otherwise save it persistantly
        mark = CG_AllocMark();
        mark->time = cg.time;
        mark->alphaFade = alphaFade;
        mark->markShader = markShader;
        mark->poly.numVerts = mf->numPoints;
        mark->color[0] = red;
        mark->color[1] = green;
        mark->color[2] = blue;
        mark->color[3] = alpha;

        if ( mark->markShader == cgs.media.ns_bloodPool || mark->markShader == cgs.media.wakeMarkShader ) {
            VectorCopy( origin, mark->org );
            VectorCopy( dir, mark->plane );

            if ( mark->markShader == cgs.media.wakeMarkShader )
                mark->poly.numVerts = radius*10;
        }


        if ( cg_lightmarks.integer && mark->markShader  != cgs.media.wakeMarkShader  )
        {
            vec3_t normal;

            VectorCopy( dir, normal );

            CG_LightVerts( normal,  mf->numPoints , mark->verts);
        }
        //	if ( cg_lightmarks.integer && mark->markShader  != cgs.media.wakeMarkShader  )
        //		CG_LightVerts( dir, mark->poly.numVerts , &mark->poly.verts );

        memcpy( mark->verts, verts, mf->numPoints * sizeof( verts[0] ) );
        markTotal++;
    }
}

/*
=================
CG_DirectImpactMark

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================
*/
void CG_DirectImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir,
                          float orientation, float red, float green, float blue, float alpha,
                          qboolean alphaFade, float radius, qboolean temporary, int entityNum ) {

    vec3_t  	  	  	axis[3];
    float  	  	  	texCoordScale;
    vec3_t  	  	  	originalPoints[4];
    byte  	  	  	colors[4];
    int  	  	  	  	i;
    polyVert_t  	  	*v;
    polyVert_t  	  	verts[4];
    markPoly_t	*mark;

    if ( !cg_addMarks.integer ) {
        return;
    }

    if ( radius <= 0 ) {
        CG_Printf( "CG_DirectImpactMark called with <= 0 radius\n" );
        radius = 4;
    }

    if ( cg.noMarkEntities[entityNum] ) // this entity seems blocked
    {
        //	CG_Printf("entity seems blocked. won't create a mark");
        return;
    }
    // create the texture axis
    VectorNormalize2( dir, axis[0] );
    PerpendicularVector( axis[1], axis[0] );
    RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
    CrossProduct( axis[0], axis[2], axis[1] );

    texCoordScale = 0.5 * 1.0 / radius;

    // create the full polygon
    for ( i = 0 ; i < 3 ; i++ ) {
        originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
        originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
        originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
        originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
    }

    colors[0] = red * 255;
    colors[1] = green * 255;
    colors[2] = blue * 255;
    colors[3] = alpha * 255;

    for ( i = 0, v = verts ; i < 4 ; i++, v++ ) {
        vec3_t  	  	delta;

        VectorCopy( originalPoints[i], v->xyz );

        VectorSubtract( v->xyz, origin, delta );
        v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
        v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
        *(int *)v->modulate = *(int *)colors;
    }

    // if it is a temporary (shadow) mark, add it immediately and forget about it
    if ( temporary ) {
        trap_R_AddPolyToScene( markShader, 4, verts );
        return;
    }

    // otherwise save it persistantly
    mark = CG_AllocMark();
    mark->time = cg.time;
    mark->alphaFade = alphaFade;
    mark->markShader = markShader;
    mark->poly.numVerts = 4;
    mark->color[0] = red;
    mark->color[1] = green;
    mark->color[2] = blue;
    mark->color[3] = alpha;

    mark->org[0] = (float)entityNum;

    mark->org[1] = mark->org[2] = 0.0f;

    if ( mark->markShader == cgs.media.ns_bloodPool || mark->markShader == cgs.media.wakeMarkShader )
    {
        VectorCopy( origin, mark->org );
        VectorCopy( dir, mark->plane );

        if ( mark->markShader == cgs.media.wakeMarkShader )
            mark->poly.numVerts = radius*10;
    }

    memcpy( mark->verts, verts, 4 * sizeof( verts[0] ) );
    markTotal++;

    if ( cg_lightmarks.integer && mark->markShader  != cgs.media.wakeMarkShader  )
    {
        vec3_t normal;

        VectorCopy( dir, normal );

        CG_LightVerts( normal,  4 , mark->verts);
    }
}




/*
===============
CG_AddMarks
===============
*/
#define	MARK_TOTAL_TIME		60000 * 6
#define	MARK_FADE_TIME		1000

#define WAKEMARK_TOTAL_TIME	400
#define WAKEMARK_FADE_TIME	100

// sizes up to 7seconds
#define MARK_BLOODPOOL_SIZE_TIME 8000

void CG_AddMarks( void ) {
    int			j;
    markPoly_t	*mp, *next;
    int			t;
    int			fade;
    qboolean	removeAllMarks = qfalse;

    if ( !cg_addMarks.integer ) {
        return;
    }

    if ( cg.roundStarted )
        removeAllMarks = qtrue;

    mp = cg_activeMarkPolys.nextMark;
    for ( ; mp != &cg_activeMarkPolys ; mp = next ) {
        // grab next now, so if the local entity is freed we
        // still have it
        next = mp->nextMark;

        if (removeAllMarks)
        {
            CG_FreeMarkPoly(mp);
            continue;
        }

        if ( mp->markShader == cgs.media.wakeMarkShader && cg.time > mp->time + WAKEMARK_TOTAL_TIME ) {
            CG_FreeMarkPoly( mp );
            continue;
        }   // see if it is time to completely remove it
        else if ( cg.time > mp->time + (cg_addMarks.integer * 1000) ) {
            CG_FreeMarkPoly( mp );
            continue;
        }


        if ( mp->markShader == cgs.media.wakeMarkShader )
        {
            float t = mp->time + WAKEMARK_TOTAL_TIME - cg.time;
            float radius;
            float fade = t / WAKEMARK_TOTAL_TIME;

            mp->poly.numVerts = mp->poly.numVerts * 1.1;

            radius = ( float ) ( ( float ) (mp->poly.numVerts)  / 20.0f );

            if (radius <= 0 )
                radius = 1;

            if ( fade <= 0 )
                fade = 0;

            CG_ImpactMark( mp->markShader, mp->org, mp->plane, 20,fade,fade,fade,fade,qtrue, radius, qtrue );
            continue;
        }
        else if ( mp->markShader == cgs.media.ns_bloodPool )
        {
            t = mp->time + (cg_addMarks.integer * 1000) - cg.time;

            if ( t < MARK_FADE_TIME ) {
                fade = 255 * t / MARK_FADE_TIME;

                if ( mp->alphaFade ) {
                    for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
                        mp->verts[j].modulate[3] = fade;
                    }
                } else {
                    for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
                        mp->verts[j].modulate[0] = mp->color[0] * fade;
                        mp->verts[j].modulate[1] = mp->color[1] * fade;
                        mp->verts[j].modulate[2] = mp->color[2] * fade;
                    }
                }
            }
        }
        else
        {
            t = mp->time + (cg_addMarks.integer * 1000) - cg.time;
            if ( t < MARK_FADE_TIME ) {
                fade = 255 * t / MARK_FADE_TIME;

                if ( mp->alphaFade ) {
                    for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
                        mp->verts[j].modulate[3] = fade;
                    }
                } else {
                    for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
                        mp->verts[j].modulate[0] = mp->color[0] * fade;
                        mp->verts[j].modulate[1] = mp->color[1] * fade;
                        mp->verts[j].modulate[2] = mp->color[2] * fade;
                    }
                }
            }
        }

        if ( mp->markShader == cgs.media.ns_bloodPool )
        {
            float t = mp->time + (cg_addMarks.integer * 1000) - cg.time;
            float radius;

            if ( t > ( (cg_addMarks.integer * 1000) - MARK_BLOODPOOL_SIZE_TIME ) )
                mp->poly.numVerts++;

            radius = ( float ) ( ( float ) (mp->poly.numVerts)  / 20.0f );

            if (radius <= 0 )
                radius = 1;

            CG_ImpactMark( mp->markShader, mp->org, mp->plane, 20,1,1,1,0.6f,qfalse, radius, qtrue );

            continue;
        }

        trap_R_AddPolyToScene( mp->markShader, mp->poly.numVerts, mp->verts );
    }

    if ( removeAllMarks )
        cg.roundStarted = qfalse;
}

void CG_DeleteDirectMark( int entityNum )
{
    markPoly_t	*mp, *next;

    // don't create marks for this entity in the future
    cg.noMarkEntities[ entityNum ] = qtrue;

    mp = cg_activeMarkPolys.nextMark;

    for ( ; mp != &cg_activeMarkPolys ; mp = next ) {
        // grab next now, so if the local entity is freed we
        // still have it
        next = mp->nextMark;

        // see if it is an mark that should be removed
        if ( (int)mp->org[0] == entityNum ) {
            CG_FreeMarkPoly( mp );
            continue;
        }
    }
}

