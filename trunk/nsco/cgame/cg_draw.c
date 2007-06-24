// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

#ifdef MISSIONPACK
#include "../ui/ui_shared.h"

// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;
#else
int drawTeamOverlayModificationCount = -1;
#endif

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;
int nextChBlinik = 0;
qboolean chBlink = qfalse;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];


#ifdef MISSIONPACK

int CG_Text_Width(const char *text, float scale, int limit) {
    int count,len;
    float out;
    glyphInfo_t *glyph;
    float useScale;
    const char *s = text;
    fontInfo_t *font = &cgDC.Assets.textFont;
    if (scale <= cg_smallFont.value) {
        font = &cgDC.Assets.smallFont;
    } else if (scale > cg_bigFont.value) {
        font = &cgDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;
    out = 0;
    if (text) {
        len = strlen(text);
        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len) {
            if ( Q_IsColorString(s) ) {
                s += 2;
                continue;
            } else {
                glyph = &font->glyphs[*s];
                out += glyph->xSkip;
                s++;
                count++;
            }
        }
    }
    return out * useScale;
}

int CG_Text_Height(const char *text, float scale, int limit) {
    int len, count;
    float max;
    glyphInfo_t *glyph;
    float useScale;
    const char *s = text;
    fontInfo_t *font = &cgDC.Assets.textFont;
    if (scale <= cg_smallFont.value) {
        font = &cgDC.Assets.smallFont;
    } else if (scale > cg_bigFont.value) {
        font = &cgDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;
    max = 0;
    if (text) {
        len = strlen(text);
        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len) {
            if ( Q_IsColorString(s) ) {
                s += 2;
                continue;
            } else {
                glyph = &font->glyphs[*s];
                if (max < glyph->height) {
                    max = glyph->height;
                }
                s++;
                count++;
            }
        }
    }
    return max * useScale;
}

void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader) {
    float w, h;
    w = width * scale;
    h = height * scale;
    CG_AdjustFrom640( &x, &y, &w, &h );
    trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style)
{
    int len, count;
    vec4_t newColor;
    glyphInfo_t *glyph;
    float useScale;
    fontInfo_t *font = &cgDC.Assets.textFont;

    if (scale <= cg_smallFont.value) {
        font = &cgDC.Assets.smallFont;
    } else if (scale > cg_bigFont.value) {
        font = &cgDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;

    if (text) {
        const char *s = text;
        trap_R_SetColor( color );
        memcpy(&newColor[0], &color[0], sizeof(vec4_t));

        len = strlen(text);

        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;

        while (s && *s && count < len)
        {
            glyph = &font->glyphs[*s];
            //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
            //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
            if ( Q_IsColorString( s ) )
            {
                memcpy( newColor, g_color_table[ ColorIndex( *(s+1) ) ], sizeof( newColor ) );
                newColor[3] = color[3];
                trap_R_SetColor( newColor );
                s += 2;
                continue;
            }
            else
            {
                float yadj = useScale * glyph->top;

                if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE || style == ITEM_TEXTSTYLE_OUTLINESHADOWED)
                {
                    int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

                    colorBlack[3] = newColor[3];
                    trap_R_SetColor( colorBlack );

                    CG_Text_PaintChar(x + ofs, y - yadj + ofs,
                                      glyph->imageWidth,
                                      glyph->imageHeight,
                                      useScale,
                                      glyph->s,
                                      glyph->t,
                                      glyph->s2,
                                      glyph->t2,
                                      glyph->glyph);
                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }

                if (style == ITEM_TEXTSTYLE_OUTLINED || style == ITEM_TEXTSTYLE_OUTLINESHADOWED )
                {

                    int ofs = 1;
                    colorBlack[3] = newColor[3];
                    if ( colorBlack[3] <= 0.0f)
                        colorBlack[3] = 0.0f;

                    trap_R_SetColor( colorBlack );



                    CG_Text_PaintChar(x + ofs, y - yadj + ofs,
                                      glyph->imageWidth,
                                      glyph->imageHeight,
                                      useScale,
                                      glyph->s,
                                      glyph->t,
                                      glyph->s2,
                                      glyph->t2,
                                      glyph->glyph);


                    CG_Text_PaintChar(x - ofs, y - yadj + ofs,
                                      glyph->imageWidth,
                                      glyph->imageHeight,
                                      useScale,
                                      glyph->s,
                                      glyph->t,
                                      glyph->s2,
                                      glyph->t2,
                                      glyph->glyph);
                    CG_Text_PaintChar(x + ofs, y - yadj - ofs,
                                      glyph->imageWidth,
                                      glyph->imageHeight,
                                      useScale,
                                      glyph->s,
                                      glyph->t,
                                      glyph->s2,
                                      glyph->t2,
                                      glyph->glyph);
                    CG_Text_PaintChar(x - ofs, y - yadj - ofs,
                                      glyph->imageWidth,
                                      glyph->imageHeight,
                                      useScale,
                                      glyph->s,
                                      glyph->t,
                                      glyph->s2,
                                      glyph->t2,
                                      glyph->glyph);
                    CG_Text_PaintChar(x - ofs, y - yadj,
                                      glyph->imageWidth,
                                      glyph->imageHeight,
                                      useScale,
                                      glyph->s,
                                      glyph->t,
                                      glyph->s2,
                                      glyph->t2,
                                      glyph->glyph);
                    CG_Text_PaintChar(x + ofs, y - yadj,
                                      glyph->imageWidth,
                                      glyph->imageHeight,
                                      useScale,
                                      glyph->s,
                                      glyph->t,
                                      glyph->s2,
                                      glyph->t2,
                                      glyph->glyph);

                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }

                CG_Text_PaintChar(x, y - yadj,
                                  glyph->imageWidth,
                                  glyph->imageHeight,
                                  useScale,
                                  glyph->s,
                                  glyph->t,
                                  glyph->s2,
                                  glyph->t2,
                                  glyph->glyph);
                // CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
                x += (glyph->xSkip * useScale) + adjust;
                s++;
                count++;
            }
        }
        trap_R_SetColor( NULL );
    }
}

//
// special NS:CO version

void CG_Text_PaintChar2(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader) {
    float w = width, h = height;

    x = x * ( (float)cgs.glconfig.vidWidth/1024.0f );
    y = y * ( (float)cgs.glconfig.vidHeight/768.0f );
    w = width * ( (float)cgs.glconfig.vidWidth/1024.0f ) * scale;
    h = height * ( (float)cgs.glconfig.vidHeight/768.0f ) * scale;

    trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void CG_Text_Paint2(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style)
{
    int len, count;
    vec4_t newColor;
    glyphInfo_t *glyph;
    float useScale;
    fontInfo_t *font = &cgDC.Assets.textFont;

    if (scale <= cg_smallFont.value) {
        font = &cgDC.Assets.smallFont;
    } else if (scale > cg_bigFont.value) {
        font = &cgDC.Assets.bigFont;
    }
    useScale = scale * font->glyphScale;

    if (text) {
        const char *s = text;
        trap_R_SetColor( color );
        memcpy(&newColor[0], &color[0], sizeof(vec4_t));

        len = strlen(text);

        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;

        while (s && *s && count < len)
        {
            glyph = &font->glyphs[*s];
            //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
            //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
            if ( Q_IsColorString( s ) )
            {
                memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
                newColor[3] = color[3];
                trap_R_SetColor( newColor );
                s += 2;
                continue;
            }
            else
            {
                float yadj = useScale * glyph->top;

                if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE || style == ITEM_TEXTSTYLE_OUTLINESHADOWED)
                {
                    int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

                    colorBlack[3] = newColor[3];
                    trap_R_SetColor( colorBlack );

                    CG_Text_PaintChar2(x + ofs, y - yadj + ofs,
                                       glyph->imageWidth,
                                       glyph->imageHeight,
                                       useScale,
                                       glyph->s,
                                       glyph->t,
                                       glyph->s2,
                                       glyph->t2,
                                       glyph->glyph);
                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }

                if (style == ITEM_TEXTSTYLE_OUTLINED || style == ITEM_TEXTSTYLE_OUTLINESHADOWED )
                {

                    int ofs = 1;
                    colorBlack[3] = newColor[3];
                    if ( colorBlack[3] <= 0.0f)
                        colorBlack[3] = 0.0f;

                    trap_R_SetColor( colorBlack );



                    CG_Text_PaintChar2(x + ofs, y - yadj + ofs,
                                       glyph->imageWidth,
                                       glyph->imageHeight,
                                       useScale,
                                       glyph->s,
                                       glyph->t,
                                       glyph->s2,
                                       glyph->t2,
                                       glyph->glyph);


                    CG_Text_PaintChar2(x - ofs, y - yadj + ofs,
                                       glyph->imageWidth,
                                       glyph->imageHeight,
                                       useScale,
                                       glyph->s,
                                       glyph->t,
                                       glyph->s2,
                                       glyph->t2,
                                       glyph->glyph);
                    CG_Text_PaintChar2(x + ofs, y - yadj - ofs,
                                       glyph->imageWidth,
                                       glyph->imageHeight,
                                       useScale,
                                       glyph->s,
                                       glyph->t,
                                       glyph->s2,
                                       glyph->t2,
                                       glyph->glyph);
                    CG_Text_PaintChar2(x - ofs, y - yadj - ofs,
                                       glyph->imageWidth,
                                       glyph->imageHeight,
                                       useScale,
                                       glyph->s,
                                       glyph->t,
                                       glyph->s2,
                                       glyph->t2,
                                       glyph->glyph);
                    CG_Text_PaintChar2(x - ofs, y - yadj,
                                       glyph->imageWidth,
                                       glyph->imageHeight,
                                       useScale,
                                       glyph->s,
                                       glyph->t,
                                       glyph->s2,
                                       glyph->t2,
                                       glyph->glyph);
                    CG_Text_PaintChar2(x + ofs, y - yadj,
                                       glyph->imageWidth,
                                       glyph->imageHeight,
                                       useScale,
                                       glyph->s,
                                       glyph->t,
                                       glyph->s2,
                                       glyph->t2,
                                       glyph->glyph);

                    colorBlack[3] = 1.0;
                    trap_R_SetColor( newColor );
                }

                CG_Text_PaintChar2(x, y - yadj,
                                   glyph->imageWidth,
                                   glyph->imageHeight,
                                   useScale,
                                   glyph->s,
                                   glyph->t,
                                   glyph->s2,
                                   glyph->t2,
                                   glyph->glyph);
                // CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
                x += (glyph->xSkip * useScale) + adjust;
                s++;
                count++;
            }
        }
        trap_R_SetColor( NULL );
    }
}

#endif

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
    refdef_t		refdef;
    refEntity_t		ent;

    if ( !cg_draw3dIcons.integer || !cg_drawIcons.integer ) {
        return;
    }

    CG_AdjustFrom640( &x, &y, &w, &h );

    memset( &refdef, 0, sizeof( refdef ) );

    memset( &ent, 0, sizeof( ent ) );

    AnglesToAxis( angles, ent.axis );
    VectorCopy( origin, ent.origin );
    ent.hModel = model;
    ent.customSkin = skin;
    ent.renderfx = RF_NOSHADOW;		// no stencil shadows

    ent.nonNormalizedAxes = qtrue;

    refdef.rdflags = RDF_NOWORLDMODEL;

    AxisClear( refdef.viewaxis );

    refdef.fov_x = 30;
    refdef.fov_y = 30;

    refdef.x = x;
    refdef.y = y;
    refdef.width = w;
    refdef.height = h;

    refdef.time = cg.time;

    trap_R_ClearScene();
    trap_R_AddRefEntityToScene( &ent );
    trap_R_RenderScene( &refdef );
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles ) {
    clipHandle_t	cm;
    clientInfo_t	*ci;
    float			len;
    vec3_t			origin;
    vec3_t			mins, maxs;

    ci = &cgs.clientinfo[ clientNum ];

    if ( cg_draw3dIcons.integer ) {
        cm = ci->headModel;
        if ( !cm ) {
            return;
        }

        // offset the origin y and z to center the head
        trap_R_ModelBounds( cm, mins, maxs );

        origin[2] = -0.5 * ( mins[2] + maxs[2] );
        origin[1] = 0.5 * ( mins[1] + maxs[1] );

        // calculate distance so the head nearly fills the box
        // assume heads are taller than wide
        len = 0.7 * ( maxs[2] - mins[2] );
        origin[0] = len / 0.268;	// len / tan( fov/2 )

        // allow per-model tweaking
        VectorAdd( origin, ci->headOffset, origin );

        CG_Draw3DModel( x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles );
    }/* else if ( cg_drawIcons.integer ) {
     CG_DrawPic( x, y, w, h, ci->modelIcon );
     }*/

    // if they are deferred, draw a cross out
    if ( ci->deferred ) {
        CG_DrawPic( x, y, w, h, cgs.media.deferShader );
    }
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team )
{
    vec4_t		hcolor;

    hcolor[3] = alpha;
    if ( team == TEAM_RED ) {
        hcolor[0] = 1;
        hcolor[1] = 0;
        hcolor[2] = 0;
    } else if ( team == TEAM_BLUE ) {
        hcolor[0] = 0;
        hcolor[1] = 0;
        hcolor[2] = 1;
    } else {
        hcolor[0] = 0.2f;
        hcolor[1] = 0.8f;
        hcolor[2] = 0.2f;
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
    trap_R_SetColor( NULL );
}


/*
===========================================================================================

UPPER RIGHT CORNER

===========================================================================================
*/
/*
================
CG_DrawLocation

tell the player if he is in an assault/bomb field

================
*/
static float CG_DrawLocation( float y ) {

    //	const char *string;

    if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
        return y;
    }

    if ( cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_RED && cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_BLUE ) {
        return y;
    }

    return y;
}


/*
==================
CG_DrawSnapshot
==================
*/
static int CG_DrawSnapshot( int y ) {
    char		*s;
    int			w;

    s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime,
            cg.latestSnapshotNum, cgs.serverCommandSequence );
    w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

    CG_DrawBigString( 635 - w, y + 2, s, 1.0F);

    return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static int CG_DrawFPS( int y ) {
    char		*s;
    int			w;
    static int	previousTimes[FPS_FRAMES];
    static int	index;
    int		i, total;
    int		fps;
    float	scale = 0.15f;
    static	int	previous;
    int		t, frameTime;

    if ( !cg_drawFPS.integer ) {
        return y;
    }

    y += CG_Text_Height( "100Fps", scale, 0 ) + 2;

    // don't use serverTime, because that will be drifting to
    // correct for internet lag changes, timescales, timedemos, etc
    t = trap_Milliseconds();
    frameTime = t - previous;
    previous = t;

    previousTimes[index % FPS_FRAMES] = frameTime;
    index++;
    if ( index > FPS_FRAMES ) {
        // average multiple frames together to smooth changes out a bit
        total = 0;
        for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
            total += previousTimes[i];
        }
        if ( !total ) {
            total = 1;
        }
        fps = 1000 * FPS_FRAMES / total;

        s = va( "%iFPS", fps );
        w = CG_Text_Width( s ,scale,0 );

        CG_Text_Paint( 640 - w, y, scale, colorWhite,s, 0,0,ITEM_TEXTSTYLE_OUTLINED );
    }
    return y + 24;
}
/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight( void ) {
    int	y = 0;

    if ( cg_drawFPS.integer ) {
        CG_DrawFPS( y );
    }
    if ( cg_drawSnapshot.integer ) {
        CG_DrawSnapshot( y );
    }
}

/*
===========================================================================================

LOWER RIGHT CORNER

===========================================================================================
*/

/*
=================
CG_DrawMapOverview
=================
*/
#define MAP_SIZE_WIDTH		512
#define MAP_SIZE_HEIGHT		512

static void CG_DrawPlayer( int x, int y, int size, qboolean team )
{
    vec4_t		hcolor;

    hcolor[3] = 1.0f;
    if ( team == TEAM_RED ) {
        hcolor[0] = 0.0f;
        hcolor[1] = 0.0f;
        hcolor[2] = 1.0f;
    } else if ( team == TEAM_BLUE ) {
        hcolor[0] = 1.0f;
        hcolor[1] = 0.0f;
        hcolor[2] = 0.0f;
    } else {
        hcolor[0] = 0.0f;
        hcolor[1] = 1.0f;
        hcolor[2] = 0.0f;
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic( x, y, size, size, cgs.media.teamStatusBar );
    trap_R_SetColor( NULL );

}

/*
=================
CG_DrawSniperRifle
=================
*/
static void CG_DrawSniperRifle(void) {

    int weaponstate = cg.snap->ps.weaponstate;
    int team = 0;

    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
        return;
    }

    // chasing a bullet.
    if ( cg.cameraActive )
        return;
    if ( cg.renderingThirdPerson )
        return;

    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
        team = 1;

    // check to see if we're in zoom mode
    if (
        ! (
            (cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM2X ) ) ||
            (cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_ZOOM4X ) )
        )
    )
        return;


    // if not in ready
    if ( weaponstate != WEAPON_FIRING && weaponstate !=	WEAPON_FIRING2 && weaponstate != WEAPON_FIRING3 && weaponstate != WEAPON_READY )
        return;

    trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLACK)] );

    switch ( cg.snap->ps.weapon ) {
    case WP_M4:
        CG_DrawPic( 0, 0, 640, 480, cgs.media.sniperScopeShader[0] );
        break;
    case WP_AK47:
        CG_DrawPic( 0, 0, 640, 480, cgs.media.sniperScopeShader[1] );
        break;
    case WP_M14:
        CG_DrawPic( 0, 0, 640, 480, cgs.media.sniperScopeShader[2] );
        break;
    case WP_PSG1:
        CG_DrawPic( 0, 0, 640, 480, cgs.media.sniperScopeShader[3] );
        break;
    case WP_MACMILLAN:
        CG_DrawPic( 0, 0, 640, 480, cgs.media.sniperScopeShader[4] );
        break;
    case WP_SL8SD:
        CG_DrawPic( 0, 0, 640, 480, cgs.media.sniperScopeShader[5] );
        break;
    default:
        CG_DrawPic( 0, 0, 640, 480, cgs.media.sniperScopeShader[2] );
        break;
    }
    trap_R_SetColor( NULL );
}

/*
=================
CG_DrawThermalGoggles
=================
*/
static int CG_DrawThermalGoggles(void) {
    float	hcolor[4];

    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
        return 0;
    }

    // if not enabled
    if ( ! ( cg.snap->ps.stats[STAT_ACTIVE_ITEMS] & ( 1 << UI_NVG ) ) )
        return 0;

    hcolor[0] = 1;
    hcolor[1] = 0;
    hcolor[2] = 0;

    hcolor[3] = 0.7f;

    CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);

    trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLACK)] );
    CG_DrawPic(0,0,640,480, cgs.media.thermalGogglesShader );
    trap_R_SetColor( NULL );

    return 1;
}

/*
=================
CG_DrawDamageFeedback
=================
*/
static void CG_DrawDamageFeedback ( void ) {
    int			t;
    int			maxTime;
    //	refEntity_t		ent;
    // ns
    float	hcolor[4];
    // ns end

    if (  ( CG_PointContents( cg.refdef.vieworg, -1 ) &  CONTENTS_WATER )){
        hcolor[0] = 0.2f;
        hcolor[1] = 0.2f;
        hcolor[2] = 0.5f;
        hcolor[3] = 0.6f;
        CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);

        cg.WaterTime = cg.snap->serverTime;

    }
    else if (cg.WaterTime > 0)
    {
        maxTime = 500;
        t = cg.time - cg.WaterTime;

        hcolor[0] = 0.2f;
        hcolor[1] = 0.2f;
        hcolor[2] = 0.5f;
        hcolor[3] = 0.6 - ((float)t / 10000);

        CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);

        if ( hcolor[3] <= 0.0 )
            cg.WaterTime = 0;
    }

    if ( !cg.damageValue )
        return;

    // ragePro systems can't fade blends, so don't obscure the screen
    if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
        return;
    }

    maxTime = 500;
    t = cg.time - cg.damageTime;


    if ( t <= 0 || t >= maxTime ) {

        return;
    }

    else {

        hcolor[0] = 1;
        hcolor[1] = 0;
        hcolor[2] = 0;

        hcolor[3] = 1.0 - ((float)t / maxTime);
    }

    //	CG_Printf("hcolor 3 (blood debug) : %f \n", hcolor[3] );

    CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);
}

/*
=================
CG_DrawFlashed
=================
*/
static void CG_DrawFlashed ( void ) {
    int			t;
    float	hcolor[4];

    // render the blindspot
    if ( cg.flashedVisionTime > 0 ) {

        // only calculate a new blindspot position
        if ( !cg.flashedVision_x || !cg.flashedVision_y ) {
            cg.flashedVision_x = random()*400;
            cg.flashedVision_y = random()*300;
        }

        // t \in [SEALS_FLASHBANGTIME*SEALS_BLINDSPOTFACTOR ... -SEALS_FLASHBANGTIME*SEALS_BLINDSPOTFACTOR]
        t = cg.flashedVisionTime - cg.time;

        hcolor[0] = 1.0;
        hcolor[1] = 1.0;
        hcolor[2] = 1.0;

        // clever part
        if ( t > 0 ) {
            hcolor[3] = 1.0;
        } else {
            hcolor[3] = 1.0 + ((float)t / (float)(SEALS_FLASHBANGTIME*SEALS_BLINDSPOTFACTOR));
            if (hcolor[3] <= 0.0) hcolor[3] = 0.0;
        }

        trap_R_SetColor(hcolor);
        CG_DrawPic( cg.flashedVision_x,cg.flashedVision_y, 320, 320,cgs.media.flashedSpot );
        trap_R_SetColor(NULL);

        if ( hcolor[3] <= 0.0  ) {
            cg.flashedVision_x = cg.flashedVision_y = 0;
            cg.flashedVisionTime = 0;
        }
    }

    if ( cg.FlashTime <= 0 )
        return;

    // t \in [SEALS_FLASHBANGTIME .. -SEALS_FLASHBANGFADETIME]
    t = cg.FlashTime - cg.time;

    hcolor[0] = 1.0;
    hcolor[1] = 1.0;
    hcolor[2] = 1.0;

    // clever part
    if ( t > 0) {
        hcolor[3] = 1.0;
    } else {
        hcolor[3] = 1.0 + ((float)t / (float)SEALS_FLASHBANGFADETIME);
        if (hcolor[3] <= 0.0) hcolor[3] = 0.0;
    }


    CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);

    if ( hcolor[3] <= 0.0 )
        cg.FlashTime = 0;
}

/*
==================
CG_DrawDeathblend

used to draw a small fade in effect while respawning
==================
*/
static void CG_DrawDeathblend( void )
{
    vec4_t hcolor;
    float  t;

    if ( cg.DeathBlendTime <= 0 )
        return;


    t = cg.DeathBlendTime - cg.time;

    hcolor[0] = 0.0;
    hcolor[1] = 0.0;
    hcolor[2] = 0.0;
    hcolor[3] = ( (float)t / 1000 );

    if ( hcolor[3] > 1.0f) {
        hcolor[3] = 1.0;
    }

    if ( hcolor[3] <= 0.0 || t <= 0 )
    {
        cg.DeathBlendTime = 0;
        return;
    }

    CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);

}
/*
==================
CG_DrawSmokeblend

draws a blend-out effect from the smoke grenades
==================
*/
static void CG_DrawSmokeblend( void )
{
    vec4_t hcolor;

    if ( cg.smokeBlendAlpha <= 0.0f )
        return;

    switch (cgs.camoType) {
    case CAMO_URBAN:
        hcolor[0] = SEALS_SMOKENADE_R_URBAN;
        hcolor[1] = SEALS_SMOKENADE_G_URBAN;
        hcolor[2] = SEALS_SMOKENADE_B_URBAN;
        break;
    case CAMO_ARCTIC:
        hcolor[0] = SEALS_SMOKENADE_R_ARCTIC;
        hcolor[1] = SEALS_SMOKENADE_G_ARCTIC;
        hcolor[2] = SEALS_SMOKENADE_B_ARCTIC;
        break;
    case CAMO_DESERT:
        hcolor[0] = SEALS_SMOKENADE_R_DESERT;
        hcolor[1] = SEALS_SMOKENADE_G_DESERT;
        hcolor[2] = SEALS_SMOKENADE_B_DESERT;
        break;
    case CAMO_JUNGLE:
    default:
        hcolor[0] = SEALS_SMOKENADE_R_JUNGLE;
        hcolor[1] = SEALS_SMOKENADE_G_JUNGLE;
        hcolor[2] = SEALS_SMOKENADE_B_JUNGLE;
        break;
    }
    hcolor[3] = cg.smokeBlendAlpha;

    CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);
}

/*
=================
CG_DrawBlend
=================
*/
#define BLEND_START_FADE_TIME	100
#define BLEND_FADE_TIME			200

static void CG_DrawBlend ( void ) {
    float	hcolor[4];

    if ( cg.savedblendAlpha > 0.0f && cg.blendAlpha == 0.0f )
    {
        float a;

        if ( cg.blendFadeTime <= 0 )
            cg.blendFadeTime = cg.time + BLEND_START_FADE_TIME;

        if ( cg.time > cg.blendFadeTime )
        {
            a = cg.time - cg.blendFadeTime;

            hcolor[0] = 1.0f;
            hcolor[1] = 1.0f;
            hcolor[2] = 1.0f;
            hcolor[3] = cg.savedblendAlpha - ( a / BLEND_FADE_TIME );

            if ( hcolor[3] > 1.0f)
                hcolor[3] = 1.0f;
            if ( hcolor[3] < 0.0f)
            {
                hcolor[3] = 0.0f;
                cg.savedblendAlpha = 0.0f;
                cg.blendFadeTime = 0;
            }

            CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);
        }
        else
        {

            hcolor[0] = 1.0f;
            hcolor[1] = 1.0f;
            hcolor[2] = 1.0f;
            hcolor[3] = cg.savedblendAlpha;

            if ( hcolor[3] > 1.0f)
                hcolor[3] = 1.0;

            CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);
        }
    }
    else if ( cg.blendAlpha <= 0.0f )
        return;
    else
    {
        hcolor[0] = 1.0f;
        hcolor[1] = 1.0f;
        hcolor[2] = 1.0f;
        hcolor[3] = cg.blendAlpha;

        if ( hcolor[3] > 1.0f) {
            hcolor[3] = 1.0;
        }

        CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);
    }

    if ( cg.blendAlpha > 0.0f )
        cg.savedblendAlpha = cg.blendAlpha;
    cg.blendAlpha = 0.0f;
}

/*
=================
CG_DrawC4
=================
*/
static void CG_DrawC4( void ) {
    int y,x;
    int width,height;
    //	int i = 0;
    float color[4];

    if ( cg.snap->ps.pm_flags & PMF_BOMBCASE )
        return;
    if ( cg.snap->ps.weapon != WP_C4 || cg.snap->ps.weaponstate != WEAPON_FIRING)
        return;

    // only draw if we don't have the anims visible
    if ( cg_drawGun.integer )
        return;

    if ( cg.snap->ps.weaponTime < 1 )
        return;

    // set color
    color[0] = 0.0;
    color[1] = 1;
    color[2] = 0.0;
    // set alpha
    color[3] = 0.7f;

    x = 155;
    y = 201;
    width = 342;
    height = 44;

    CG_FillRect( x , y , width, height, color );

    color[0] = color[1] = color[2] = 0;

    x = 160;
    y = 206;
    width = 330;
    height = 34;

    CG_FillRect( x , y , width, height, color );

    // set color
    color[0] = 0.0;
    color[1] = 1;
    color[2] = 0.0;
    // set alpha
    //	color[3] = 1;

    //	CG_Printf("weapontime: %i\n", cg.snap->ps.weaponTime );

    x = 170;
    y = 211;
    width = 1 + (int)( 310 -  ( cg.snap->ps.weaponTime / 13.73 ) );
    height = 23;

    CG_FillRect( x , y , width, height, color );

}
/*
==================
CG_DrawMissionInformation

[ ] [ ] [ ]

[       ]
[       ]
[       ]
640 - 80 / 3
==================
*/
int CG_ButtonPushed( int button );
extern char SealBriefing[ 128 ][ 128 ]; // max chars per line
extern int	sealBriefingLines;
extern char TangoBriefing[ 128 ][ 128 ]; // max chars per line
extern int	tangoBriefingLines;
static void CG_MissionInformation( void ) {
    int x;
    int y;
    int	value;
    vec4_t color = { 1.0f,1.0f,1.0f,0.75f };
    float	fontsize = .23f;
    qhandle_t	backPic = trap_R_RegisterShaderNoMip( "ui/assets/missioninfo_bg1" );
    if ( !cg.viewMissionInfo )
        return;

    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ||
            cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
        backPic = trap_R_RegisterShaderNoMip( "ui/assets/missioninfo_bg2" );

    if ( CG_ButtonPushed( BUTTON_USE ) )
    {
        cg.viewMissionInfo = 0;
        return;
    }

    CG_DrawPic( 0,0,640,480, backPic );

    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED )
    {
        x = 320 - CG_Text_Width("Press "S_COLOR_RED"Use"S_COLOR_WHITE" To Close", 0.4f, 99 )/2;
        CG_Text_Paint( x,480-10-CG_Text_Height("Press Spaced",0.4f,0 ),0.4f, colorWhite, "Press "S_COLOR_RED"Use"S_COLOR_WHITE" To Close", 0,0,ITEM_TEXTSTYLE_OUTLINED );

        x = 64;
        y = 120;
        if ( sealBriefingLines > 0 ) {

            for ( value = 0; value <= sealBriefingLines*2; value ++ )
            {
                CG_Text_Paint( x,y, fontsize, color, SealBriefing[value], 0,0,0);//ITEM_TEXTSTYLE_SHADOWED);
                y += CG_Text_Height( SealBriefing[value], fontsize, 0 ) + 2;
            }
        }

        if ( cgs.infoPicLeft )
            CG_DrawPic( 416,40, 160, 120,cgs.infoPicLeft );
        if ( cgs.infoPicMiddle )
            CG_DrawPic( 416,172, 160, 120,cgs.infoPicMiddle );
        if ( cgs.infoPicRight )
            CG_DrawPic( 416,304, 160, 120,cgs.infoPicRight );
    }
    else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
    {
        x = 320 - CG_Text_Width("Press "S_COLOR_RED"Use"S_COLOR_WHITE" To Close", 0.4f, 99 )/2;
        CG_Text_Paint( x,480-10-CG_Text_Height("Press Spaced",0.4f,0 ),0.4f, colorWhite, "Press "S_COLOR_RED"Use"S_COLOR_WHITE" To Close", 0,0,ITEM_TEXTSTYLE_OUTLINED );

        x = 64;
        y = 120;
        if ( tangoBriefingLines > 0 ) {

            for ( value = 0; value <= tangoBriefingLines*2; value ++ )
            {
                CG_Text_Paint( x,y, fontsize, color, TangoBriefing[value], 0,0,0);//ITEM_TEXTSTYLE_SHADOWED);
                y += CG_Text_Height( TangoBriefing[value], fontsize, 0 ) + 2 ;
            }
        }

        if ( cgs.infoPicLeft )
            CG_DrawPic( 416,40, 160, 120,cgs.infoPicLeft );
        if ( cgs.infoPicMiddle )
            CG_DrawPic( 416,172, 160, 120,cgs.infoPicMiddle );
        if ( cgs.infoPicRight )
            CG_DrawPic( 416,304, 160, 120,cgs.infoPicRight );
    }
    else
    {
        x = 320 - CG_Text_Width("Press "S_COLOR_RED"Use"S_COLOR_WHITE" To Close", 0.4f, 99 )/2;

        CG_Text_Paint( x,480-10-CG_Text_Height("Press Spaced",0.4f,0 ),0.4f, colorWhite, "Press "S_COLOR_RED"Use"S_COLOR_WHITE" To Close", 0,0,ITEM_TEXTSTYLE_OUTLINED );

        x = 48;
        y = 204;
        if ( sealBriefingLines > 0 ) {

            for ( value = 0; value <= sealBriefingLines*2; value ++ )
            {
                CG_Text_Paint( x,y, fontsize, color, SealBriefing[value], 0,0,0);//ITEM_TEXTSTYLE_SHADOWED);
                y += CG_Text_Height( SealBriefing[value], fontsize, 0 ) + 2;
            }
        }

        x = 316;
        y = 204;
        if ( tangoBriefingLines > 0 ) {

            for ( value = 0; value <= tangoBriefingLines*2; value ++ )
            {
                CG_Text_Paint( x,y, fontsize, color, TangoBriefing[value], 0,0,0);//ITEM_TEXTSTYLE_SHADOWED);
                y += CG_Text_Height( TangoBriefing[value], fontsize, 0 ) + 2 ;
            }
        }


        if ( cgs.infoPicLeft )
            CG_DrawPic( 48,40, 160, 120,cgs.infoPicLeft );
        if ( cgs.infoPicMiddle )
            CG_DrawPic( 240,40, 160, 120,cgs.infoPicMiddle );
        if ( cgs.infoPicRight )
            CG_DrawPic( 432,40, 160, 120,cgs.infoPicRight );
    }

}

//===========================================================================================


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct {
    int		frameSamples[LAG_SAMPLES];
    int		frameCount;
    int		snapshotFlags[LAG_SAMPLES];
    int		snapshotSamples[LAG_SAMPLES];
    int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
    int			offset;

    offset = cg.time - cg.latestSnapshotTime;
    lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
    lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
    // dropped packet
    if ( !snap ) {
        lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
        lagometer.snapshotCount++;
        return;
    }

    // add this snapshot's info
    lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
    lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
    lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
    float		x, y;
    int			cmdNum;
    usercmd_t	cmd;
    const char		*s;
    int			w;

    // draw the phone jack if we are completely past our buffers
    cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
    trap_GetUserCmd( cmdNum, &cmd );
    if ( cmd.serverTime <= cg.snap->ps.commandTime
            || cmd.serverTime > cg.time ) {	// special check for map_restart
        return;
    }

    // also add text in center of screen
    s = "Connection Interrupted";
    w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
    CG_DrawBigString( 320 - w/2, 100, s, 1.0F);

    // blink the icon
    if ( ( cg.time >> 9 ) & 1 ) {
        return;
    }

    x = 640 - 48;
    y = 480 - 48;

    CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga" ) );
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( void ) {
    int		a, x, y, i;
    float	v;
    float	ax, ay, aw, ah, mid, range;
    int		color;
    float	vscale;

    if ( !cg_lagometer.integer || cgs.localServer ) {
        CG_DrawDisconnect();
        return;
    }

    //
    // draw the graph
    //
#ifdef MISSIONPACK
    x = 640 - 48;
    y = 480 - 144;
#else
    x = 640 - 48;
    y = 480 - 48;
#endif

    trap_R_SetColor( NULL );
    CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

    ax = x;
    ay = y;
    aw = 48;
    ah = 48;
    CG_AdjustFrom640( &ax, &ay, &aw, &ah );

    color = -1;
    range = ah / 3;
    mid = ay + range;

    vscale = range / MAX_LAGOMETER_RANGE;

    // draw the frame interpoalte / extrapolate graph
    for ( a = 0 ; a < aw ; a++ ) {
        i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
        v = lagometer.frameSamples[i];
        v *= vscale;
        if ( v > 0 ) {
            if ( color != 1 ) {
                color = 1;
                trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
            }
            if ( v > range ) {
                v = range;
            }
            trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
        } else if ( v < 0 ) {
            if ( color != 2 ) {
                color = 2;
                trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
            }
            v = -v;
            if ( v > range ) {
                v = range;
            }
            trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
        }
    }

    // draw the snapshot latency / drop graph
    range = ah / 2;
    vscale = range / MAX_LAGOMETER_PING;

    for ( a = 0 ; a < aw ; a++ ) {
        i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
        v = lagometer.snapshotSamples[i];
        if ( v > 0 ) {
            if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
                if ( color != 5 ) {
                    color = 5;	// YELLOW for rate delay
                    trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
                }
            } else {
                if ( color != 3 ) {
                    color = 3;
                    trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
                }
            }
            v = v * vscale;
            if ( v > range ) {
                v = range;
            }
            trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
        } else if ( v < 0 ) {
            if ( color != 4 ) {
                color = 4;		// RED for dropped snapshots
                trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
            }
            trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
        }
    }

    trap_R_SetColor( NULL );

    if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
        CG_DrawBigString( ax, ay, "snc", 1.0 );
    }

    CG_DrawDisconnect();
}



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth ) {
    char	*s;

    Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

    cg.centerPrintTime = cg.time;
    cg.centerPrintY = y;
    cg.centerPrintCharWidth = 16;

    // count the number of lines for centering
    cg.centerPrintLines = 1;
    s = cg.centerPrint;
    while( *s ) {
        if (*s == '\n')
            cg.centerPrintLines++;
        s++;
    }
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
    char	*start;
    int		l;
    int		x, y, w , h;
    float	*color;

    if ( !cg.centerPrintTime ) {
        return;
    }

    color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
    if ( !color ) {
        return;
    }

    trap_R_SetColor( color );

    start = cg.centerPrint;

    y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

    while ( 1 ) {
        char linebuffer[1024];

        for ( l = 0; l < 50; l++ ) {
            if ( !start[l] || start[l] == '\n' ) {
                break;
            }
            linebuffer[l] = start[l];
        }
        linebuffer[l] = 0;

#ifdef MISSIONPACK
        w = CG_Text_Width(linebuffer, 0.5, 0);
        h = CG_Text_Height(linebuffer, 0.5, 0);
        x = (SCREEN_WIDTH - w) / 2;
        CG_Text_Paint(x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_OUTLINED);
        y += h + 6;
#else
        w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

        x = ( SCREEN_WIDTH - w ) / 2;


        CG_DrawStringExt2( x, y, linebuffer, color, qfalse, qtrue,
                           cg.centerPrintCharWidth, cg.centerPrintCharWidth, 0 );

        y += cg.centerPrintCharWidth;
#endif
        while ( *start && ( *start != '\n' ) ) {
            start++;
        }
        if ( !*start ) {
            break;
        }
        start++;
    }

    trap_R_SetColor( NULL );
}



/*
===============================================================================

NEWBEE HELP FUNCTIONS

===============================================================================
*/

extern	vmCvar_t	cg_newbeeHeight;
extern	vmCvar_t	cg_newbeeTime;

/*
==============
CG_NewbieMessage

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_NewbieMessage( const char *str, int y, float charHeight ) {
    char	*s;

    if ( cg_newbeeTime.integer <= 0 )
        return;

    Q_strncpyz( cg.ns_newbiehelp.string, str, sizeof(cg.ns_newbiehelp.string) );

    cg.ns_newbiehelp.stringTime = cg.time;
    cg.ns_newbiehelp.stringY = y;
    cg.ns_newbiehelp.stringCharWidth = 16;

    if ( charHeight > 0 )
        cg.ns_newbiehelp.stringCharHeight = charHeight;
    else
        cg.ns_newbiehelp.stringCharHeight = 1.0f;

    trap_S_StartLocalSound( cgs.media.newbeeMsgSound, CHAN_AUTO );

    // count the number of lines for centering
    cg.ns_newbiehelp.stringLines = 1;
    s = cg.ns_newbiehelp.string;
    while( *s ) {
        if (  *s == '\n'  )
            cg.ns_newbiehelp.stringLines++;
        s++;
    }
}


/*
===================
CG_DrawNewbieMessage
===================
*/
static void CG_DrawNewbieMessage( void ) {
    char	*start;
    int		l;
    int		x, y, w , h;
    float	*color;

    if ( !cg.ns_newbiehelp.stringTime ) {
        return;
    }

    color = CG_FadeColor( cg.ns_newbiehelp.stringTime, 1000 * cg_newbeeTime.value );
    if ( !color ) {
        return;
    }

    trap_R_SetColor( color );

    start = cg.ns_newbiehelp.string;

    y = cg.ns_newbiehelp.stringY - cg.ns_newbiehelp.stringLines * CG_Text_Height("A",cg.ns_newbiehelp.stringCharHeight,0) / 2;

    while ( 1 ) {
        char linebuffer[1024];

        for ( l = 0; l < 255; l++ ) {
            if ( !start[l] || start[l] == '\n'  ) {
                break;
            }
            linebuffer[l] = start[l];
        }
        linebuffer[l] = 0;

        w = CG_Text_Width(linebuffer, cg.ns_newbiehelp.stringCharHeight, 0);
        h = CG_Text_Height(linebuffer, cg.ns_newbiehelp.stringCharHeight, 0);
        x = (SCREEN_WIDTH - w) / 2;
        CG_Text_Paint(x, y + h, cg.ns_newbiehelp.stringCharHeight, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_OUTLINED);
        y += h + 6;

        while	( *start && (*start != '\n') )
        {
            start++;
        }
        if ( !*start ) {
            break;
        }
        start++;
    }

    trap_R_SetColor( NULL );
}

/*
================================================================================

CROSSHAIR

================================================================================
*/


#define CROSSHAIR_HOR_WIDTH	 3
#define CROSSHAIR_HOR_HEIGHT 1

#define CROSSHAIR_VER_WIDTH (CROSSHAIR_HOR_HEIGHT)
#define CROSSHAIR_VER_HEIGHT (CROSSHAIR_HOR_WIDTH)

extern	vmCvar_t	cg_crosshair_r;
extern	vmCvar_t	cg_crosshair_g;
extern	vmCvar_t	cg_crosshair_b;

// |---|
// |---|
// const float *color
/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect2( float x, float y, float width, float height, const float *color ) {
    trap_R_SetColor( color );

    trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader );

    trap_R_SetColor( NULL );
}

extern vmCvar_t	cg_crosshairWidth;

static void CG_DrawStaticCrosshair( int x, int y, int width, int height, const float *color)
{
    int chw, chh, cvw, cvh;

    // zooming? no crosshair...
    if ( BG_IsZooming( cg.snap->ps.stats[STAT_WEAPONMODE] ) )
        return;
    if ( !cg_drawCrosshair.integer )
        return;
    if ( cg.renderingThirdPerson )
        return;
    if ( cg.snap->ps.weapon == WP_PSG1 ||
            cg.snap->ps.weapon == WP_SL8SD ||
            cg.snap->ps.weapon == WP_MACMILLAN )
        return;
    if ( cg.snap->ps.weapon == WP_C4 )
        return;
    if ( cg.snap->ps.weapon == WP_NONE)
        return;
    if ( cg.snap->ps.weapon == WP_KHURKURI ||
            cg.snap->ps.weapon == WP_SEALKNIFE )
        return;
    if ( ( cg.snap->ps.weapon == WP_GRENADE ||
            cg.snap->ps.weapon == WP_FLASHBANG ||
            cg.snap->ps.weapon == WP_SMOKE ) &&
            SEALS_NO_GRENADE_CROSSHAIR )
        return;
    if (  ( ( cg.snap->ps.weapon == WP_AK47 ) ||
            ( cg.snap->ps.weapon == WP_M4 ) ) &&
            ( cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) ) &&
            ( cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) ) )
        return;


    chw = CROSSHAIR_HOR_WIDTH;
    chh = cg_crosshairWidth.integer;
    cvw = cg_crosshairWidth.integer;
    cvh = CROSSHAIR_VER_HEIGHT;

    /*CG_FillRect2( x , ( y + height/2 ) - chh/2 , chw,chh*cg_crosshairWidth.integer,color);
    CG_FillRect2( x + width -  chw, ( y + height/2 ) - chh/2 , chw,chh*cg_crosshairWidth.integer,color);

    CG_FillRect2( x + width/2 - cvw/2, y, cvw*cg_crosshairWidth.integer,cvh,color);
    CG_FillRect2( x + width/2 - cvw/2, y + height - cvh, cvw*cg_crosshairWidth.integer,cvh,color);*/

    CG_FillRect2( x , ( y + height/2 ) - chh/2 , chw,chh,color);
    CG_FillRect2( x + width -  chw, ( y + height/2 ) - chh/2 , chw,chh,color);

    CG_FillRect2( x + width/2 - cvw/2, y, cvw,cvh,color);
    CG_FillRect2( x + width/2 - cvw/2, y + height - cvh, cvw,cvh,color);
}

/*
=================
CG_DrawCrosshair
=================
*/

#define CROSSHAIR_FADE_TIME	2000
#define CROSSHAIR_FADE_MINTIME 500

static int CG_GetCrosshairFadeTime(qboolean fadein) {
    int i;
    int accuracy = char_accuracy.integer;

    if ( accuracy <= 0 )
        accuracy = 1;

    if (fadein)
        i = CROSSHAIR_FADE_TIME - ( accuracy * (CROSSHAIR_FADE_TIME / 10) );
    else
        i = ( accuracy * (CROSSHAIR_FADE_TIME / 10) );

    return i + CROSSHAIR_FADE_MINTIME;
}


static void CG_DrawCrosshair(void) {
    float		w, h;
    float		f;
    float		color[4];
    float		x, y;
    int			crosshairMod;
    int			crosshairFadeTime ;
    qboolean	running = qtrue;
    float   xyzspeed;

    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
        return;

    if ( cg.snap->ps.pm_type == PM_SPECTATOR )
        return;

    xyzspeed = sqrt( cg.snap->ps.velocity[0] * cg.snap->ps.velocity[0]
                     +  cg.snap->ps.velocity[1] * cg.snap->ps.velocity[1]
                     +  cg.snap->ps.velocity[2] * cg.snap->ps.velocity[2] );

    if ( CG_ButtonPushed( BUTTON_WALKING )
            || cg.snap->ps.pm_flags & PMF_DUCKED
            || cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_LACTIVE )
            || xyzspeed < 40.0f
       )
        running = qfalse;

    // calculate the time needed to fade the crosshair
    if ( running ) {
        // fade out the crosshair

        // check if the crosshair was fading in
        if ( (cg.crosshairFadeIn == qtrue) ) {
            cg.crosshairFadeIn = qfalse;
            cg.crosshairTime = cg.time + CG_GetCrosshairFadeTime(cg.crosshairFadeIn);
            if ( (cg.crosshairTime < cg.time) ) cg.crosshairTime -= (cg.crosshairTime - cg.time);

        }

    } else {
        // fade in the crosshair

        // check if the crosshair was fading out
        if ( (cg.crosshairFadeIn == qfalse) ) {
            cg.crosshairFadeIn = qtrue;
            cg.crosshairTime = cg.time + CG_GetCrosshairFadeTime(cg.crosshairFadeIn);
            if ( (cg.crosshairTime < cg.time) ) cg.crosshairTime -= (cg.crosshairTime - cg.time);

        }

    }

    crosshairFadeTime = CG_GetCrosshairFadeTime (cg.crosshairFadeIn);

    color[0]=cg_crosshair_r.value;
    color[1]=cg_crosshair_g.value;
    color[2]=cg_crosshair_b.value;

    // calculate color
    if (chBlink) {
        color[0]=1.0-color[0];
        color[1]=1.0-color[1];
        color[2]=1.0-color[2];
    }

    // crosshair gets black-effect when out of rounds
    if ( ( cg.snap->ps.stats[STAT_ROUNDS] <= cg_lowAmmoWarning.integer ) ) {
        if (
            ( (cg.time - nextChBlinik) >= 0 ) &&
            ! ( cg.snap->ps.weapon == WP_C4 ) &&
            ! ( cg.snap->ps.weapon == WP_KHURKURI ) &&
            ! ( cg.snap->ps.weapon == WP_SEALKNIFE ) &&
            ! ( cg.snap->ps.weapon == WP_GRENADE ) &&
            ! ( cg.snap->ps.weapon == WP_FLASHBANG ) &&
            ! ( cg.snap->ps.weapon == WP_NONE ) &&
            ! ( cg.snap->ps.weapon == WP_SMOKE ) ) {
            nextChBlinik = cg.time + (
                               (cg.snap->ps.stats[STAT_ROUNDS] < 0 ? 0 : cg.snap->ps.stats[STAT_ROUNDS])
                               +1)*100;
            chBlink = !chBlink;
        }
    } else {
        chBlink = qfalse;
    }


    // calculate alpha value
    if (!cg.crosshairFadeIn) {

        if (cg.crosshairTime > cg.time) {
            color[3] = (float)((float)cg.crosshairTime - (float)cg.time) / (float)crosshairFadeTime;
            if (color[3] <= 0.05) color[3] = 0.0;
        } else color[3] = 0.0;

    } else {

        if (cg.crosshairTime > cg.time) {
            color[3] = 1.0 - (float)((float)cg.crosshairTime - (float)cg.time) / (float)crosshairFadeTime ;
            if (color[3] >= 0.95) color[3] = 1.0;
        } else color[3] = 1.0;

    } // if (running... else...

    // fading crosshair
    if ( cg_crosshairFade.integer == 1 ) {

        if ( CG_ButtonPushed( BUTTON_SPRINT) ) {
            if (color[3] < 0.10) color[3] = 0.10f;
        } else {
            if (color[3] < 0.30) color[3] = 0.30f;
        }

    } else if ( cg_crosshairFade.integer == 2 ) {

        if (color[3] < 0.0) color[3] = 0.0;

    }	else {

        if ( CG_ButtonPushed( BUTTON_SPRINT) ) {
        if (color[3] <= 0.25) color[3] = 0.25f;
        } else {
            if (color[3] < 0.75) color[3] = 0.75f;
        }
    }

    w = h = cg_crosshairSize.value;

    // pulse the size of the crosshair when picking up items
    f = cg.time - cg.itemPickupBlendTime;
    if ( f > 0 && f < ITEM_BLOB_TIME ) {
        f /= ITEM_BLOB_TIME;
        w *= ( 1 + f );
        h *= ( 1 + f );
    }

    x = cg.refdef.x + 0.5 * ( cg.refdef.width  - w);
    y = cg.refdef.y + 0.5 * ( cg.refdef.height  - h);

    // if using a crosshair, don't make the crosshair too big
    if ( cg.snap->ps.stats[STAT_WEAPONMODE] & ( 1 << WM_LACTIVE ) )
        crosshairMod = cg.crosshairMod/2 + xyzspeed / (12 + cg.snap->ps.persistant[PERS_ACCURACY]) ;
    else
        crosshairMod = cg.crosshairMod/2 + xyzspeed / (6 + (cg.snap->ps.persistant[PERS_ACCURACY]/2));

    x -= crosshairMod;
    y -= crosshairMod;

    CG_DrawStaticCrosshair (x, y,
                            w + crosshairMod * 2, h + crosshairMod * 2,
                            color);

}



/*
===========================================================================

RADAR

===========================================================================
*/
extern	vmCvar_t	cg_drawRadar;
extern	vmCvar_t	cg_radarX;
extern	vmCvar_t	cg_radarY;

#define RADAR_BACK_WIDTH	80
#define RADAR_BACK_HEIGHT	80

#define RADAR_BLIP_WIDTH	8
#define RADAR_BLIP_HEIGHT	8

#define RADAR_OBJECT_WIDTH	16
#define RADAR_OBJECT_HEIGHT	16

#define	SCANNER_UNIT                   32
#define	SCANNER_RANGE                  35 //35

#define SCANNER_WIDTH					40

static void CG_CalcUpdateRadarPositions( void )
{
    float	len;
    int		hd;
    vec3_t	v;
    int		i;
    vec3_t	blipOrg;

    // update players
    for ( i = 0; i < cg.radarNumEntities; i++ )
    {
        VectorCopy( cg.radarEntities[i].origin, blipOrg );

        // calc player to enemy vector
        VectorSubtract (cg.predictedPlayerState.origin, blipOrg, v);

        // save height differential
        hd = v[2] / SCANNER_UNIT;

        // remove height component
        v[2] = 0;

        // calc length of distance from top down view (no z)
        len = VectorLength (v) / SCANNER_UNIT;

        if ( len > SCANNER_RANGE )
            len = SCANNER_RANGE;

        // in range ?
        if ( len <= SCANNER_RANGE )
        {
            vec3_t	dp;
            vec3_t	normal = {0,0,-1};

            // normal vector to enemy
            VectorNormalize(v);

            // rotate round player view angle (yaw)
            RotatePointAroundVector( dp, normal, v, cg.refdefViewAngles[1]);

            // scale to fit scanner range (80 = pixel range of scanner)
            VectorScale(dp,len*SCANNER_WIDTH/SCANNER_RANGE,dp);

            // update x/y and height
            cg.radarEntities[i].x = dp[1];
            cg.radarEntities[i].y = dp[0];
            cg.radarEntities[i].height = hd;
        }
    }

    // update various clientside handled entities
    // update players
    for ( i = 0; i < cg.radarNumObjects; i++ )
    {
        VectorCopy( cg.radarObjects[i].origin, blipOrg );

        // calc player to enemy vector
        VectorSubtract (cg.predictedPlayerState.origin, blipOrg, v);

        // save height differential
        hd = v[2] / SCANNER_UNIT;

        // remove height component
        v[2] = 0;

        // calc length of distance from top down view (no z)
        len = VectorLength (v) / SCANNER_UNIT;

        if ( len > SCANNER_RANGE )
            len = SCANNER_RANGE;

        // in range ?
        if ( len <= SCANNER_RANGE )
        {
            vec3_t	dp;
            vec3_t	normal = {0,0,-1};

            // normal vector to enemy
            VectorNormalize(v);

            // rotate round player view angle (yaw)
            RotatePointAroundVector( dp, normal, v, cg.refdefViewAngles[1]);

            // scale to fit scanner range (80 = pixel range of scanner)
            VectorScale(dp,len*SCANNER_WIDTH/SCANNER_RANGE,dp);

            // update x/y and height
            cg.radarObjects[i].x = dp[1];
            cg.radarObjects[i].y = dp[0];
            cg.radarObjects[i].height = hd;
        }
    }
}
static void CG_DrawRadar( void ) {
    int i;
    qhandle_t	radarBackShader = trap_R_RegisterShader("gfx/radar/radar_back"),
                                radarFriendShader = trap_R_RegisterShader("gfx/radar/radar_friendly"),
                                                    radarEnemyShader = trap_R_RegisterShader("gfx/radar/radar_enemy"),
                                                                       radarBombShader = trap_R_RegisterShader("gfx/radar/radar_bomb"),
                                                                                         radarUpShader = trap_R_RegisterShader("gfx/radar/radar_up"),
                                                                                                         radarDownShader = trap_R_RegisterShader("gfx/radar/radar_dn"),
                                                                                                                           radarFriendMissionShader = trap_R_RegisterShader("gfx/radar/radar_friendly_m"),
                                                                                                                                                      radarBandageShader = trap_R_RegisterShader("gfx/radar/radar_friendly_b"),
                                                                                                                                                                           radarFriendRadio = trap_R_RegisterShader("gfx/radar/radar_friendly_r"),
                                                                                                                                                                                              hShader;

    if ( !cg_drawRadar.integer )
        return;
    if ( cg.snap->ps.pm_flags & PMF_FOLLOW )
        return;

    CG_DrawPic( cg_radarX.integer - RADAR_BACK_WIDTH/2,
                cg_radarY.integer - RADAR_BACK_HEIGHT/2,
                RADAR_BACK_WIDTH,
                RADAR_BACK_HEIGHT, radarBackShader );

    CG_CalcUpdateRadarPositions( );

    if ( cgs.gametype == GT_LTS ) // radar entities nur rendern wenn missionsmodus aktiv ist
        for ( i = 0; i < cg.radarNumObjects; i++ )
        {
            qhandle_t shader;

            shader = trap_R_RegisterShader( va( "gfx/radar/radar_%c",cg.radarObjects[i].type ) );

            CG_DrawPic( cg_radarX.integer + cg.radarObjects[i].x-RADAR_OBJECT_WIDTH/2,
                        cg_radarY.integer + cg.radarObjects[i].y-RADAR_OBJECT_HEIGHT/2,
                        RADAR_OBJECT_WIDTH,RADAR_OBJECT_HEIGHT, shader );

            if ( cg.radarObjects[i].height < 0 )
                CG_DrawPic( cg_radarX.integer + cg.radarObjects[i].x+RADAR_OBJECT_WIDTH/4,
                            cg_radarY.integer + cg.radarObjects[i].y-RADAR_OBJECT_HEIGHT/2,
                            RADAR_OBJECT_WIDTH,RADAR_OBJECT_HEIGHT, radarUpShader );
            else if ( cg.radarObjects[i].height > 0 )
                CG_DrawPic( cg_radarX.integer + cg.radarObjects[i].x+RADAR_OBJECT_WIDTH/4,
                            cg_radarY.integer + cg.radarObjects[i].y-RADAR_OBJECT_HEIGHT/2,
                            RADAR_OBJECT_WIDTH,RADAR_OBJECT_HEIGHT, radarDownShader );
        }
    for ( i = 0; i < cg.radarNumEntities; i++ )
    {
        char action = cg.radarEntities[i].type;

        if ( action == 'F' )
            hShader = radarFriendShader;
        else if ( action == 'A' && sin( cg.time / 25  ) >= 0.0f  )
            hShader = radarEnemyShader; // firing, too
        else if ( action == 'B' )
            hShader = radarBandageShader;
        else if ( action == 'M' )
            hShader = radarFriendMissionShader;
        else if ( action == 'R' )
            hShader = radarFriendRadio;
        else
            hShader = radarFriendShader;

        CG_DrawPic( cg_radarX.integer + cg.radarEntities[i].x-RADAR_BLIP_WIDTH/2,
                    cg_radarY.integer + cg.radarEntities[i].y-RADAR_BLIP_HEIGHT/2,
                    RADAR_BLIP_WIDTH,RADAR_BLIP_HEIGHT, hShader );

        if ( cg.radarEntities[i].height < 0 )
            CG_DrawPic( cg_radarX.integer + cg.radarEntities[i].x+RADAR_BLIP_WIDTH/4,
                        cg_radarY.integer + cg.radarEntities[i].y-RADAR_BLIP_HEIGHT/2,
                        RADAR_BLIP_WIDTH,RADAR_BLIP_HEIGHT, radarUpShader );
        else if ( cg.radarEntities[i].height > 0 )
            CG_DrawPic( cg_radarX.integer + cg.radarEntities[i].x+RADAR_BLIP_WIDTH/4,
                        cg_radarY.integer + cg.radarEntities[i].y-RADAR_BLIP_HEIGHT/2,
                        RADAR_BLIP_WIDTH,RADAR_BLIP_HEIGHT, radarDownShader );

    }
}

void CG_EditRadar_AddObject( char type, vec3_t org )
{
    CG_Printf("Added Radar Object: %c at %s\n", type, vtos(org) );

    cg.radarObjects[cg.radarNumObjects].type = type;
    VectorCopy(org, cg.radarObjects[cg.radarNumObjects].origin );

    cg.radarNumObjects++;
}
void CG_WriteRadarInfoToBRF_f( void );

void CG_EditRadar_HandleKey( int key )
{
    int mousex=cgs.cursorX,mousey=cgs.cursorY;
    int y,y2;
    int x,x2;

    x = 4;
    x2 = 300;

    y = 258;
    y2 = y + CG_Text_Height( "A", 0.20f, 0 ) + 2;

    if ( mousex > x && mousex < x2 &&
            mousey > y && mousey < y2 )
    {
        //place V
        CG_EditRadar_AddObject('V', cg.refdef.vieworg );
    }

    y = y2;
    y2 = y + CG_Text_Height( "A", 0.20f, 0 ) + 2;

    if ( mousex > x && mousex < x2 &&
            mousey > y && mousey < y2 )
    {
        //place E
        CG_EditRadar_AddObject('E', cg.refdef.vieworg );
    }

    y = y2;
    y2 = y + CG_Text_Height( "A", 0.20f, 0 ) + 2;

    if ( mousex > x && mousex < x2 &&
            mousey > y && mousey < y2 )
    {
        //place B
        CG_EditRadar_AddObject('B', cg.refdef.vieworg );
    }

    y = y2;
    y2 = y + CG_Text_Height( "A", 0.20f, 0 ) + 2;
    if ( mousex > x && mousex < x2 &&
            mousey > y && mousey < y2 )
    {
        //place A
        CG_EditRadar_AddObject('A', cg.refdef.vieworg );
    }

    y = y2;
    y2 = y + CG_Text_Height( "A", 0.20f, 0 ) + 2;
    if ( mousex > x && mousex < x2 &&
            mousey > y && mousey < y2 )
    {
        //place A
        CG_EditRadar_AddObject('X', cg.refdef.vieworg );
    }

    y = y2 + 20;
    y2 = y + CG_Text_Height( "A", 0.20f, 0 ) + 2;
    if ( mousex > x && mousex < x2 &&
            mousey > y && mousey < y2 )
    {
        //place A
        CG_WriteRadarInfoToBRF_f();
    }
}
qboolean CG_Radar_MouseIn( int x, int x2, int y, int y2 ) {

    if ( cgs.cursorX > x && cgs.cursorX < x2 &&
            cgs.cursorY > y && cgs.cursorY < y2 )
        return qtrue;

    return qfalse;
}
static void CG_DrawEditRadar( void ) {
    int y = 254;
    char *s;

    if ( cgs.eventHandling != CGAME_EVENT_EDITRADAR )
        return;

    s = "Radar Entity Editor. Press Escape to exit.";
    CG_Text_Paint( 4, y, 0.20f, colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );
    y += CG_Text_Height( s, 0.20f, 0 ) + 2;

    s = "Place VIP Rescue Entity";
    CG_Text_Paint( 4, y, 0.20f, CG_Radar_MouseIn( 4, 300, y, y+CG_Text_Height( s, 0.20f, 0 ) + 2 )?colorRed:colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );
    y += CG_Text_Height( s, 0.20f, 0 ) + 2;

    s = "Place Extraction Point";
    CG_Text_Paint( 4, y, 0.20f, CG_Radar_MouseIn( 4, 300, y, y+CG_Text_Height( s, 0.20f, 0 ) + 2 )?colorRed:colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );
    y += CG_Text_Height( s, 0.20f, 0 ) + 2;

    s = "Place Briefcase";
    CG_Text_Paint( 4, y, 0.20f, CG_Radar_MouseIn( 4, 300, y, y+CG_Text_Height( s, 0.20f, 0 ) + 2 )?colorRed:colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );
    y += CG_Text_Height( s, 0.20f, 0 ) + 2;

    s = "Place Assaultfield";
    CG_Text_Paint( 4, y, 0.20f, CG_Radar_MouseIn( 4, 300, y, y+CG_Text_Height( s, 0.20f, 0 ) + 2 )?colorRed:colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );
    y += CG_Text_Height( s, 0.20f, 0 ) + 2;

    s = "Place BombSpot";
    CG_Text_Paint( 4, y, 0.20f, CG_Radar_MouseIn( 4, 300, y, y+CG_Text_Height( s, 0.20f, 0 ) + 2 )?colorRed:colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );
    y += CG_Text_Height( s, 0.20f, 0 ) + 2;

    y += 20;

    s = "Write File";
    CG_Text_Paint( 4, y, 0.20f, CG_Radar_MouseIn( 4, 300, y, y+CG_Text_Height( s, 0.20f, 0 ) + 2 )?colorRed:colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );
    y += CG_Text_Height( s, 0.20f, 0 ) + 2;

}



//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {
    char *string;
    int w;

    if ( cg.viewMissionInfo )
        return;


    string = "SPECTATOR";
    w = CG_Text_Width(string, 0.3f, 0);
    CG_Text_Paint( 320 - w / 2, 480-48, 0.3f, colorWhite, string, 0, 0, ITEM_TEXTSTYLE_OUTLINED);

    string = "press <USE> key to toggle Cam-mode";
    w = CG_Text_Width(string, 0.3f, 0);
    CG_Text_Paint( 320 - w / 2, 480-16, 0.3f, colorWhite, string, 0, 0, ITEM_TEXTSTYLE_OUTLINED);
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void) {
    char	*s;
    int		sec;

    if ( !cgs.voteTime ) {
        return;
    }

    // play a talk beep whenever it is modified
    if ( cgs.voteModified ) {
        cgs.voteModified = qfalse;

        if ( cg_chatBeep.integer )
            trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
    }

    sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;

    if ( sec < 0 ) {
        sec = 0;
    }

    if (
        cgs.voteString[0] == 'F' &&
        cgs.voteString[1] == 'o' &&
        cgs.voteString[2] == 'r' &&
        cgs.voteString[3] == 'g' &&
        cgs.voteString[4] == 'i' &&
        cgs.voteString[5] == 'v' &&
        cgs.voteString[6] == 'e'
    )
    {
        s = va("%s (%is left) Press F1 to forgive or F2 to refuse", cgs.voteString, sec );

        if ( sec == 0 )
        {
            cgs.voteTime = 0;
            cgs.voteModified = qfalse;
        }
    }
    else
        s = va("Vote (%is left):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
    CG_Text_Paint( 4, 58, 0.20f, colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );

    s = "or press ESC then click Vote";
    CG_Text_Paint( 4, 58 + CG_Text_Height( s, 0.20f, 0 ) + 2, 0.20f, colorWhite, s, 0,0, ITEM_TEXTSTYLE_OUTLINED );

}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void) {
    char	*s;
    int		sec, cs_offset;

    if ( cgs.clientinfo->team == TEAM_RED )
        cs_offset = 0;
    else if ( cgs.clientinfo->team == TEAM_BLUE )
        cs_offset = 1;
    else
        return;

    if ( !cgs.teamVoteTime[cs_offset] ) {
        return;
    }

    // play a talk beep whenever it is modified
    if ( cgs.teamVoteModified[cs_offset] ) {
        cgs.teamVoteModified[cs_offset] = qfalse;
        trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
    }

    sec = ( VOTE_TIME - ( cg.time - cgs.teamVoteTime[cs_offset] ) ) / 1000;
    if ( sec < 0 ) {
        sec = 0;
    }
    s = va("Teamvote (%is left):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
           cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
    CG_DrawSmallString( 0, 90, s, 1.0F );
}


static qboolean CG_DrawScoreboard() {
    static qboolean firstTime = qtrue;
    float fade, *fadeColor;

    if (menuScoreboard) {
        menuScoreboard->window.flags &= ~WINDOW_FORCED;
    }
    if (cg_paused.integer) {
        cg.deferredPlayerLoading = 0;
        firstTime = qtrue;
        return qfalse;
    }

    // don't draw scoreboard during death while warmup up
    if ( cg.warmup && !cg.showScores ) {
        return qfalse;
    }

    if ( cg.showScores || cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
        fade = 1.0;
        fadeColor = colorWhite;
    } else {
        fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
        if ( !fadeColor ) {
            // next time scoreboard comes up, don't print killer
            cg.deferredPlayerLoading = 0;
            cg.killerName[0] = 0;
            firstTime = qtrue;
            return qfalse;
        }
        fade = *fadeColor;
    }


    if (menuScoreboard == NULL) {
        if ( cgs.gametype >= GT_TEAM ) {
            menuScoreboard = Menus_FindByName("teamscore_menu");
        } else {
            menuScoreboard = Menus_FindByName("score_menu");
        }
    }

    if (menuScoreboard) {
        if (firstTime) {
            CG_SetScoreSelection(menuScoreboard);
            firstTime = qfalse;
        }
        Menu_Paint(menuScoreboard, qtrue);
    }

    // load any models that have been deferred
    if ( ++cg.deferredPlayerLoading > 10 ) {
        CG_LoadDeferredPlayers();
    }

    return qtrue;
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
    //	int key;
#ifdef MISSIONPACK
    //if (cg_singlePlayer.integer) {
    //	CG_DrawCenterString();
    //	return;
    //}
#else
    if ( cgs.gametype == GT_SINGLE_PLAYER ) {
        CG_DrawCenterString();
        return;
    }
#endif
    cg.scoreFadeTime = cg.time;
    cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
char *xp_to_rank( int xp, int team );

static qboolean CG_DrawFollow( void ) {
    int			w;
    vec4_t		color;
    const char	 *s;
    int			chaseclient = cg.snap->ps.clientNum;

    if ( cg.showScores )
        return qfalse;

    if ( !( cg.snap->ps.pm_flags & PMF_FOLLOW ) ) {
        if ( cg.snap->ps.pm_type == PM_SPECTATOR ||
                cg.snap->ps.pm_type == PM_NOCLIP )
        {

            if ( cg.viewMissionInfo )
                return qfalse;

            w = CG_Text_Width("Spectator", 0.55f, 0);

            CG_Text_Paint( 320 - w / 2, 48, 0.55f, colorWhite, S_COLOR_BLUE"Spectator", 0, 0, ITEM_TEXTSTYLE_OUTLINED);

        }
        return qfalse;
    }
    color[0] = 1;
    color[1] = 1;
    color[2] = 1;
    color[3] = 1;

    s = "Chasing Soldier:";
    w = CG_Text_Width(s, 0.3f, 0);

    CG_Text_Paint( 320 - w / 2, 24, 0.4f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_OUTLINED);


    cg.cameraFollowNumber = cg.snap->ps.weapon;

    if ( cgs.gametype >= GT_TEAM )
        s = va("%s. %s",
               xp_to_rank( cgs.clientinfo[  chaseclient ].score, cgs.clientinfo[ chaseclient ].team),
               cgs.clientinfo[ chaseclient ].name);
    else
        s = va("%s",cgs.clientinfo[ chaseclient ].name);

    w = CG_Text_Width(s, 0.4f, 0);

    CG_Text_Paint( 320 - w / 2, 48, 0.4f, colorLtBlue, s, 0, 0, ITEM_TEXTSTYLE_OUTLINED);

    return qtrue;
}


#if 0
/*
=================
CG_DrawProxWarning
=================
*/
static void CG_DrawProxWarning( void ) {
    char s [32];
    int			w;
    static int proxTime;
    static int proxCounter;
    static int proxTick;

    if( !(cg.snap->ps.eFlags & EF_TICKING ) ) {
        proxTime = 0;
        return;
    }

    if (proxTime == 0) {
        proxTime = cg.time + 5000;
        proxCounter = 5;
        proxTick = 0;
    }

    if (cg.time > proxTime) {
        proxTick = proxCounter--;
        proxTime = cg.time + 1000;
    }

    if (proxTick != 0) {
        Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
    } else {
        Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED");
    }

    w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
    CG_DrawBigStringColor( 320 - w / 2, 64 + BIGCHAR_HEIGHT, s, g_color_table[ColorIndex(COLOR_RED)] );
}
#endif


/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
    int			w;
    int			sec;
    float scale;
    int			cw;
    const char	*s;

    sec = cg.warmup;
    if ( !sec ) {
        return;
    }

    if ( sec < 0 ) {

        if ( cg.viewMissionInfo )
            return;

        s = "Waiting for players";
        w = CG_Text_Width(s, 0.3f, 0);

        CG_Text_Paint( 320 - w / 2, 24, 0.3f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_OUTLINED);

        cg.warmupCount = 0;
        return;
    }
    /*

    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) // seals
    {
    int value;
    extern	char SealBriefing[ 128 ][ MAX_CHARS_PER_LINE ];
    extern	int	sealBriefingLines;

    if ( sealBriefingLines > 0 )
    {
    int y;

    y = 480 - ( (sealBriefingLines) + 4) * 16;

    for ( value = 0; value < sealBriefingLines; value ++ )
    {
    w = CG_Text_Width(SealBriefing[value], 0.3f, 0);

    CG_Text_Paint( 320 - w / 2, y, 0.3f, colorWhite, SealBriefing[value], 0, 0, ITEM_TEXTSTYLE_OUTLINED);

    y += 16;
    }
    }
    }
    else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) // seals
    {
    int value;
    extern	char TangoBriefing[ 128 ][ MAX_CHARS_PER_LINE ];
    extern	int	tangoBriefingLines;

    if ( tangoBriefingLines > 0 )
    {
    int y;

    y = 480 - ( (tangoBriefingLines) + 4) * 16;

    for ( value = 0; value < tangoBriefingLines; value ++ )
    {
    w = CG_Text_Width(TangoBriefing[value], 0.3f, 0);

    CG_Text_Paint( 320 - w / 2, y, 0.3f, colorWhite, TangoBriefing[value], 0, 0, ITEM_TEXTSTYLE_OUTLINED);
    y += 16;
    }
    }
    }
    */

    sec = ( sec - cg.time ) / 1000;
    if ( sec < 0 ) {
        cg.warmup = 0;
        sec = 0;
    }
    s = va( "Starts in: "S_COLOR_RED"%i"S_COLOR_WHITE, sec + 1 );
    if ( sec != cg.warmupCount ) {
        cg.warmupCount = sec;
        if ( sec < 10 )
        {

            // fixme: precache sounds
            char soundname[128];

            Com_sprintf( soundname, sizeof(soundname), "sound/commentary/%i.wav", sec+1 );

            trap_S_StartLocalSound( trap_S_RegisterSound( soundname, qfalse ) , CHAN_ANNOUNCER );
        }
    }
    scale = 0.45f;
    switch ( cg.warmupCount ) {
    case 0:
        cw = 28;
        scale = 0.54f;
        break;
    case 1:
        cw = 24;
        scale = 0.51f;
        break;
    case 2:
        cw = 20;
        scale = 0.48f;
        break;
    default:
        cw = 16;
        scale = 0.45f;
        break;
    }


    w = CG_Text_Width(s, scale, 0);

    CG_Text_Paint( 320 - w / 2, 70, scale, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_OUTLINED);

}

//==================================================================================
#ifdef MISSIONPACK
/*
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus() {
    if (cg.voiceTime) {
        int t = cg.time - cg.voiceTime;
        if ( t > 2500 ) {
            trap_Cvar_Set("cl_conXOffset", "0");
            cg.voiceTime = 0;
        }
    }
}
#endif

/*
=================
CG_DrawHDR
=================
*/
void CG_DrawHDR() {
    if ( !r_hdr.integer )
        return;

    CG_DrawPic( 0, 0, 640, 480, cgs.media.hdrShader );
}
/*
=================
CG_DrawMotionBlur
=================
*/
void CG_DrawMotionBlur() {
    if ( !r_motionblur.integer )
        return;

    CG_DrawPic( 0, 0, 640, 480, cgs.media.motionblurShader );
}
/*
=================
CG_DrawBlur
=================
*/
void CG_DrawBlur() {
    if ( !r_blur.integer )
        return;

    CG_DrawPic( 0, 0, 640, 480, cgs.media.blurShader );
}

extern vmCvar_t	cg_hudStyle;
void CG_DrawPic2( float x, float y, float width, float height, qhandle_t hShader );

void CG_DrawMouse(int x,int y, int height, int width )
{
    qhandle_t	hShader;

    hShader = cgDC.Assets.cursor;
    //CG_Printf("drawmouse: %i %i", x , y );

    // after we've setted the color draw the pic
    CG_DrawPic( x, y, width, height, hShader );

    if ( cgs.eventHandling == CGAME_EVENT_EDITHUD )
    {
        vec4_t whiteTrans = { 1.0f,1.0f,1.0f,0.35f };
        int	width = 130;
        int height = 220;
        int x2 = cgs.cursorX * ( 1024.0f/640 );
        int y2 = cgs.cursorY * ( 768.0f/480 );

        if ( cg_hudStyle.integer == 2 )
        {
            width = 200;
            height = 96;
        }
        trap_R_SetColor( whiteTrans );
        CG_DrawPic2( x2, y2, width, height, cgs.media.whiteShader );
        trap_R_SetColor( NULL );
    }
    else if ( cgs.eventHandling == CGAME_EVENT_EDITRADARPOS )
    {
        vec4_t whiteTrans = { 1.0f,1.0f,1.0f,0.35f };

        CG_FillRect( cgs.cursorX-40, cgs.cursorY-40, 80, 80, whiteTrans );
    }

}

void CG_QCmd_HandleMenu( void );
void CG_DrawScrutchHud( void );
void CG_DrawSpectatorHud( void );
extern	vmCvar_t	cg_drawScriptedUI;

/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D( void )
{

    // if we are taking a levelshot for the menu, don't draw anything
    if ( cg.levelShot ) {
        return;
    }

    CG_DrawDeathblend( );
    CG_DrawSmokeblend( );

    // headdamage always on
    if( cg.snap->ps.pm_type == PM_NORMAL && (cg.snap->ps.stats[STAT_HEAD_DAMAGE] > 10 || cg.snap->ps.stats[STAT_HEALTH] < 50) )
    {
        vec4_t		hcolor;

        // set color based on health
        hcolor[0] = 0.0f;
        hcolor[1] = 0.0f;
        hcolor[2] = 0.0f;
        hcolor[3] = (25 + 100 - ( cg.snap->ps.stats[STAT_HEAD_DAMAGE] + 100 - cg.snap->ps.stats[STAT_HEALTH] )) * 0.001 * sin(cg.time / 100.0) + 0.6 - (cg.snap->ps.stats[STAT_HEALTH] * 0.5 / 100) ;

        if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 )
            hcolor[3] = 0.9f;
        if(hcolor[3] > 1 )
            hcolor[3] = 1;
        else if(hcolor[3] < 0)
            hcolor[3] = 0;

        trap_R_SetColor( hcolor );
        CG_FillRect( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT, hcolor);
        trap_R_SetColor( NULL );
    }

    CG_DrawHDR();
	CG_DrawMotionBlur();
	CG_DrawBlur();

    if ( cg_draw2D.integer == 0 ) {
        return;
    }

    if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
        CG_DrawIntermission();
        return;
    }

    if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 &&
            cgs.gametype == GT_TEAM )
    {
        char *string;
        int w;
        float deathTime = ( ( (float)cg.deathTime + (float)((cgs.teamRespawn > 0)?(cgs.teamRespawn):(2500))*1000 ) - (float)cg.time ) / 1000.0f;

        if ( cgs.squadAssault )
            deathTime = ( (float)cgs.levelRoundStartTime  - (float)cg.time ) / 1000.0f;

        if ( deathTime > 0.0f )
            string = va( "Respawn in: "S_COLOR_RED"%.1fs", deathTime );
        else
            string = va( "Press "S_COLOR_RED"USE"S_COLOR_WHITE" or "S_COLOR_RED"FIRE"S_COLOR_WHITE" to Respawn" );

        w =	CG_Text_Width( string, 0.5f, 0 );
        CG_Text_Paint( 320 - w / 2, 480-48, 0.5f, colorWhite, string, 0, 0, ITEM_TEXTSTYLE_OUTLINED);
    }

    if (
        ( cgs.eventHandling == CGAME_EVENT_EDITRADARPOS ||
          cgs.eventHandling == CGAME_EVENT_EDITHUD ) &&
        ( cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP )
    )
    {
        CG_DrawRadar();

        CG_DrawScrutchHud();
    }
    else if ( ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP ) ) {
        if ( !cg.showScores && cg.viewMissionInfo == qfalse  )
        {
            Menu_PaintAll();
            CG_DrawTimedMenus();
            CG_DrawSpectatorHud();
        }
    } else {
        if ( cg.snap->ps.stats[STAT_HEALTH] > 0 )
        {

            if ( !CG_DrawThermalGoggles() ) // ns       // only draw sniperrifle crosshair
                CG_DrawSniperRifle();      // ns        // if not in thermalgoggle mode

            CG_DrawFlashed();
            CG_DrawBlend();
            CG_DrawDamageFeedback(); // screenblend
        }
        // don't draw any status if dead or the scoreboard is being explicitly shown
        if ( !cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0  && cg.viewMissionInfo == qfalse )
        {
            CG_DrawCrosshair();
            CG_DrawWeaponSelect();
            CG_DrawC4();
            CG_DrawRadar();

            // draw scripted hud/ui
            if ( cg_drawScriptedUI.integer )
            {
                Menu_PaintAll();
                CG_DrawTimedMenus();
            }
            // draw hud
            CG_DrawScrutchHud();
        }

    }

    //	CG_DrawChat();
    CG_DrawEditRadar();
    CG_DrawVote();
    CG_DrawTeamVote();

    if ( cg.viewCmd )
        CG_QCmd_HandleMenu( );

    CG_DrawLagometer();

    if (!cg_paused.integer) {
        CG_DrawUpperRight();
    }
    if ( !CG_DrawFollow() ) {
        CG_DrawWarmup();
    }

    // don't draw center string if scoreboard is up
    cg.scoreBoardShowing = CG_DrawScoreboard();
    if ( !cg.scoreBoardShowing) {
        CG_DrawCenterString();
    }
    CG_DrawNewbieMessage();
    CG_MissionInformation();
    //	CG_DrawMenu();

    if (trap_Key_GetCatcher() & KEYCATCH_CGAME  )
        CG_DrawMouse( cgs.cursorX, cgs.cursorY, 32,32 );
}


static void CG_DrawTourneyScoreboard() {
#ifdef MISSIONPACK
#else
    CG_DrawOldTourneyScoreboard();
#endif
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
    float		separation;
    vec3_t		baseOrg;

    // optionally draw the info screen instead
    if ( !cg.snap ) {
        CG_DrawInformation();
        return;
    }

    // optionally draw the tournement scoreboard instead
    if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
            ( cg.snap->ps.pm_flags & PMF_SCOREBOARD ) ) {
        CG_DrawTourneyScoreboard();
        return;
    }

    switch ( stereoView ) {
    case STEREO_CENTER:
        separation = 0;
        break;
    case STEREO_LEFT:
        separation = -cg_stereoSeparation.value / 2;
        break;
    case STEREO_RIGHT:
        separation = cg_stereoSeparation.value / 2;
        break;
    default:
        separation = 0;
        CG_Error( "CG_DrawActive: Undefined stereoView" );
    }


    // clear around the rendered view if sized down
    CG_TileClear();

    // offset vieworg appropriately if we're doing stereo separation
    VectorCopy( cg.refdef.vieworg, baseOrg );
    if ( separation != 0 ) {
        VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
    }

#ifdef SAME_WEAPONPIPE
    if ( BG_IsMelee( cg.snap->ps.weapon ) || cg_nopredict.integer )
        CG_WeaponAnimation( &cg.snap->ps );
    else
        CG_WeaponAnimation( &cg.predictedPlayerState );
#endif
    // draw 3D view
    trap_R_RenderScene( &cg.refdef );

    // restore original viewpoint if running stereo
    if ( separation != 0 ) {
        VectorCopy( baseOrg, cg.refdef.vieworg );
    }
#ifndef SAME_WEAPONPIPE
    if ( BG_IsMelee( cg.snap->ps.weapon ) || cg_nopredict.integer )
        CG_WeaponAnimation( &cg.snap->ps );
    else
        CG_WeaponAnimation( &cg.predictedPlayerState );

    CG_AddLocalEntities( qtrue );

    if (!test_x.integer )
        trap_R_RenderScene( &cg.weaponrefdef );
#endif
    // draw status bar and other floating elements
    CG_Draw2D();

    //	void ClientScript_Update( void )
    ClientScript_Update();
}

