// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
#if 0
    // adjust for wide screens
    if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
        *x += 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * 640 / 480 ) );
    }
#endif
    // scale for screen sizes

    *x *= cgs.screenXScale;
    *y *= cgs.screenYScale;
    *w *= cgs.screenXScale;
    *h *= cgs.screenYScale;
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect( float x, float y, float width, float height, const float *color ) {
    trap_R_SetColor( color );

    CG_AdjustFrom640( &x, &y, &width, &height );
    trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader );

    trap_R_SetColor( NULL );
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size) {
    CG_AdjustFrom640( &x, &y, &w, &h );
    size *= cgs.screenXScale;
    trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
    trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size) {
    CG_AdjustFrom640( &x, &y, &w, &h );
    size *= cgs.screenYScale;
    trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
    trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color ) {
    trap_R_SetColor( color );

    CG_DrawTopBottom(x, y, width, height, size);
    CG_DrawSides(x, y, width, height, size);

    trap_R_SetColor( NULL );
}



/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader ) {
    CG_AdjustFrom640( &x, &y, &width, &height );
    trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}


/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar( int x, int y, int width, int height, int ch ) {
    int row, col;
    float frow, fcol;
    float size;
    float	ax, ay, aw, ah;

    ch &= 255;

    if ( ch == ' ' ) {
        return;
    }

    ax = x;
    ay = y;
    aw = width;
    ah = height;
    CG_AdjustFrom640( &ax, &ay, &aw, &ah );

    row = ch>>4;
    col = ch&15;

    frow = row*0.0625;
    fcol = col*0.0625;
    size = 0.0625;

    trap_R_DrawStretchPic( ax, ay, aw, ah,
                           fcol, frow,
                           fcol + size, frow + size,
                           cgs.media.charsetShader );
}


/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt( int x, int y, const char *string, const float *setColor,
                       qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars ) {
    vec4_t		color;
    const char	*s;
    int			xx;
    int			cnt;

    if (maxChars <= 0)
        maxChars = 32767; // do them all!

    // draw the drop shadow
    if (shadow) {
        color[0] = color[1] = color[2] = 0;
        color[3] = setColor[3];
        trap_R_SetColor( color );
        s = string;
        xx = x;
        cnt = 0;
        while ( *s && cnt < maxChars) {
            if ( Q_IsColorString( s ) ) {
                s += 2;
                continue;
            }
            CG_DrawChar( xx + 2, y + 2, charWidth, charHeight, *s );
            cnt++;
            xx += charWidth;
            s++;
        }
    }

    // draw the colored text
    s = string;
    xx = x;
    cnt = 0;
    trap_R_SetColor( setColor );
    while ( *s && cnt < maxChars) {
        if ( Q_IsColorString( s ) ) {
            if ( !forceColor ) {
                memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
                color[3] = setColor[3];
                trap_R_SetColor( color );
            }
            s += 2;
            continue;
        }
        CG_DrawChar( xx, y, charWidth, charHeight, *s );
        xx += charWidth;
        cnt++;
        s++;
    }
    trap_R_SetColor( NULL );
}
/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt2( int x, int y, const char *string, const float *setColor,
                        qboolean forceColor, qboolean outline, int charWidth, int charHeight, int maxChars ) {
    vec4_t		color;
    const char	*s;
    int			xx;
    int			cnt;

    if (maxChars <= 0)
        maxChars = 32767; // do them all!

    // draw the drop shadow
    if (outline) {
        color[0] = color[1] = color[2] = 0;
        color[3] = setColor[3]-0.25;
        if (color[3] <= 0)
            color[3] = 0;
        trap_R_SetColor( color );
        s = string;
        xx = x;
        cnt = 0;
        while ( *s && cnt < maxChars) {
            if ( Q_IsColorString( s ) ) {
                s += 2;
                continue;
            }
            CG_DrawChar( xx - 1, y + 1, charWidth, charHeight, *s );
            CG_DrawChar( xx + 1, y + 1, charWidth, charHeight, *s );
            CG_DrawChar( xx + 1, y - 1, charWidth, charHeight, *s );
            CG_DrawChar( xx - 1, y - 1, charWidth, charHeight, *s );
            cnt++;
            xx += charWidth;
            s++;
        }
    }

    // draw the colored text
    s = string;
    xx = x;
    cnt = 0;
    trap_R_SetColor( setColor );
    while ( *s && cnt < maxChars) {
        if ( Q_IsColorString( s ) ) {
            if ( !forceColor ) {
                memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
                color[3] = setColor[3];
                trap_R_SetColor( color );
            }
            s += 2;
            continue;
        }
        CG_DrawChar( xx, y, charWidth, charHeight, *s );
        xx += charWidth;
        cnt++;
        s++;
    }
    trap_R_SetColor( NULL );
}



void CG_DrawBigString( int x, int y, const char *s, float alpha ) {
    float	color[4];

    color[0] = color[1] = color[2] = 1.0;
    color[3] = alpha;

    CG_DrawStringExt( x, y, s, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
}

void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color ) {
    CG_DrawStringExt( x, y, s, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
}
void CG_DrawStringOutline( int x, int y, const char *s, vec4_t color ) {

    int		size = 16;

    CG_DrawStringExt2( x, y, s, color, qtrue, qtrue, size, size, 0 );
}

void CG_DrawSmallString( int x, int y, const char *s, float alpha ) {
    float	color[4];

    color[0] = color[1] = color[2] = 1.0;
    color[3] = alpha;
    CG_DrawStringExt( x, y, s, color, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
}

void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color ) {
    CG_DrawStringExt( x, y, s, color, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
}
void CG_DrawTinyStringColor( int x, int y, const char *s, vec4_t color ) {
    CG_DrawStringExt( x, y, s, color, qtrue, qfalse, 6, SMALLCHAR_HEIGHT, 0 );
}


/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen( const char *str ) {
    const char *s = str;
    int count = 0;

    while ( *s ) {
        if ( Q_IsColorString( s ) ) {
            s += 2;
        } else {
            count++;
            s++;
        }
    }

    return count;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox( int x, int y, int w, int h, qhandle_t hShader ) {
    float	s1, t1, s2, t2;

    s1 = x/64.0;
    t1 = y/64.0;
    s2 = (x+w)/64.0;
    t2 = (y+h)/64.0;
    trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}



/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear( void ) {
    int		top, bottom, left, right;
    int		w, h;

    w = cgs.glconfig.vidWidth;
    h = cgs.glconfig.vidHeight;

    if ( cg.refdef.x == 0 && cg.refdef.y == 0 &&
            cg.refdef.width == w && cg.refdef.height == h ) {
        return;		// full screen rendering
    }

    top = cg.refdef.y;
    bottom = top + cg.refdef.height-1;
    left = cg.refdef.x;
    right = left + cg.refdef.width-1;

    // clear above view screen
    CG_TileClearBox( 0, 0, w, top, cgs.media.backTileShader );

    // clear below view screen
    CG_TileClearBox( 0, bottom, w, h - bottom, cgs.media.backTileShader );

    // clear left of view screen
    CG_TileClearBox( 0, top, left, bottom - top + 1, cgs.media.backTileShader );

    // clear right of view screen
    CG_TileClearBox( right, top, w - right, bottom - top + 1, cgs.media.backTileShader );
}



/*
================
CG_FadeColor
================
*/
float *CG_FadeColor( int startMsec, int totalMsec ) {
    static vec4_t		color;
    int			t;

    if ( startMsec == 0 ) {
        return NULL;
    }

    t = cg.time - startMsec;

    if ( t >= totalMsec ) {
        return NULL;
    }

    // fade out
    if ( totalMsec - t < FADE_TIME ) {
        color[3] = ( totalMsec - t ) * 1.0/FADE_TIME;
    } else {
        color[3] = 1.0;
    }
    color[0] = color[1] = color[2] = 1;

    return color;
}


/*
================
CG_TeamColor
================
*/
float *CG_TeamColor( int team ) {
    static vec4_t	red = {1, 0.2f, 0.2f, 1};
    static vec4_t	blue = {0.2f, 0.2f, 1, 1};
    static vec4_t	other = {1, 1, 1, 1};
    static vec4_t	spectator = {0.7f, 0.7f, 0.7f, 1};

    switch ( team ) {
    case TEAM_RED:
        return red;
    case TEAM_BLUE:
        return blue;
    case TEAM_SPECTATOR:
        return spectator;
    default:
        return other;
    }
}



/*
=================
CG_GetColorForHealth
=================
*/
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor ) {
    int		count;
    int		max;

    // calculate the total points of damage that can
    // be sustained at the current health / armor level
    if ( health <= 0 ) {
        VectorClear( hcolor );	// black
        hcolor[3] = 1;
        return;
    }
    count = armor;
    max = health;
    if ( max < count ) {
        count = max;
    }
    health += count;

    // set the color based on health
    hcolor[0] = 1.0;
    hcolor[3] = 1.0;
    if ( health >= 100 ) {
        hcolor[2] = 1.0;
    } else if ( health < 66 ) {
        hcolor[2] = 0;
    } else {
        hcolor[2] = ( health - 66 ) / 33.0;
    }

    if ( health > 60 ) {
        hcolor[1] = 1.0;
    } else if ( health < 30 ) {
        hcolor[1] = 0;
    } else {
        hcolor[1] = ( health - 30 ) / 30.0;
    }
}

/*
=================
CG_ColorForHealth
=================
*/
void CG_ColorForHealth( vec4_t hcolor ) {

    CG_GetColorForHealth( cg.snap->ps.stats[STAT_HEALTH],
                          0, hcolor );
}



/*
=================
UI_DrawProportionalString2
=================
*/
static int	propMap[128][3] = {
                                 {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
                                 {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

                                 {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
                                 {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

                                 {0, 0, PROP_SPACE_WIDTH},		// SPACE
                                 { 21, 137,   6},			// !
                                 { 64, 205,  11},			// "
                                 { 53, 137,  16},			// #
                                 { 70, 137,  16},			// $
                                 { 87, 137,  26},			// %
                                 {125, 137,  20},			// &
                                 { 21, 103,   5},			// `
                                 {159, 137,   7},			// (
                                 {167, 137,   7},			// )
                                 {146, 137,  12},			// *
                                 { 41, 171,  15},			// +
                                 { 42, 205,   8},			// ,
                                 { 27, 103,  10},			// -
                                 { 51, 205,   6},			// .
                                 { 88, 171,  13},			// /

                                 { 41,  69,  16}, // 0
                                 { 58,  69,   9}, // 1
                                 { 68,  69,  16}, // 2
                                 { 85,  69,  16}, // 3
                                 {102,  69,  16}, // 4
                                 {119,  69,  16}, // 5
                                 {136,  69,  16}, // 6
                                 {153,  69,  16}, // 7
                                 {170,  69,  16}, // 8
                                 {187,  69,  16}, // 9
                                 { 35, 205,   6}, // :
                                 { 27, 205,   7}, // ;
                                 {123, 171,  17}, // <
                                 { 38, 103,  17}, // =
                                 {141, 171,  17}, // >
                                 { 76, 205,  12}, // ?

                                 { 28, 137,  24}, // @
                                 {  1,   1,  22}, // A
                                 { 24,   1,  16}, // B
                                 { 41,   1,  20}, // C
                                 { 62,   1,  20}, // D
                                 { 83,   1,  14}, // E
                                 { 98,   1,  14}, // F
                                 {113,   1,  24}, // G
                                 {138,   1,  18}, // H
                                 {158,   1,   5}, // I
                                 {164,   1,  12}, // J
                                 {177,   1,  18}, // K
                                 {196,   1,  13}, // L
                                 {210,   1,  27}, // M

                                 {  1,  35,  21}, // N
                                 { 23,  35,  25}, // O
                                 { 49,  35,  15}, // P
                                 { 65,  35,  26}, // Q
                                 { 92,  35,  17}, // R
                                 {110,  35,  16}, // S
                                 {127,  35,  17}, // T
                                 {145,  35,  20}, // U
                                 {166,  35,  21}, // V
                                 {188,  35,  30}, // W
                                 {219,  35,  19}, // X
                                 {  1,  69,  20}, // Y
                                 { 22,  69,  18}, // Z

                                 { 57, 171,   7}, // [
                                 { 74, 171,  13},
                                 { 65, 171,   7}, // ]
                                 {114, 137,  10}, // ^
                                 //{  1,   1,  22}, // SPACER TO MAKE LOWER CASE LETTERS WORK! which char is missing?
                                 { 21, 171,  19}, // _

                                 { 58, 205,   5}, // '
                                 {  1,   1,  22}, // A
                                 { 24,   1,  16}, // B
                                 { 41,   1,  20}, // C
                                 { 62,   1,  20}, // D
                                 { 83,   1,  14}, // E
                                 { 98,   1,  14}, // F
                                 {113,   1,  24}, // G
                                 {138,   1,  18}, // H
                                 {158,   1,   5}, // I
                                 {164,   1,  12}, // J
                                 {177,   1,  18}, // K
                                 {196,   1,  13}, // L
                                 {210,   1,  27}, // M
                                 {  1,  35,  21}, // N
                                 { 23,  35,  25}, // O

                                 { 49,  35,  15}, // P
                                 { 65,  35,  26}, // Q
                                 { 92,  35,  17}, // R
                                 {110,  35,  16}, // S
                                 {127,  35,  17}, // T
                                 {145,  35,  20}, // U
                                 {166,  35,  21}, // V
                                 {188,  35,  30}, // W
                                 {219,  35,  19}, // X
                                 {  1,  69,  20}, // Y
                                 { 22,  69,  18}, // Z
                                 {102, 171,  10}, // {
                                 { 21, 205,   5}, // |
                                 {113, 171,   9}, // }
                                 { 56, 103,  17}, // ~
                                 {  0,   0,  -1}  // DEL
                             };


static int propMapB[26][3] = {
                                 {11, 12, 33},
                                 {49, 12, 31},
                                 {85, 12, 31},
                                 {120, 12, 30},
                                 {156, 12, 21},
                                 {183, 12, 21},
                                 {207, 12, 32},

                                 {13, 55, 30},
                                 {49, 55, 13},
                                 {66, 55, 29},
                                 {101, 55, 31},
                                 {135, 55, 21},
                                 {158, 55, 40},
                                 {204, 55, 32},

                                 {12, 97, 31},
                                 {48, 97, 31},
                                 {82, 97, 30},
                                 {118, 97, 30},
                                 {153, 97, 30},
                                 {185, 97, 25},
                                 {213, 97, 30},

                                 {11, 139, 32},
                                 {42, 139, 51},
                                 {93, 139, 32},
                                 {126, 139, 31},
                                 {158, 139, 25},
                             };

#define PROPB_GAP_WIDTH		4
#define PROPB_SPACE_WIDTH	12
#define PROPB_HEIGHT		36

/*
=================
UI_DrawBannerString
=================
*/
static void UI_DrawBannerString2( int x, int y, const char* str, vec4_t color )
{
    const char* s;
    char	ch;
    float	ax;
    float	ay;
    float	aw;
    float	ah;
    float	frow;
    float	fcol;
    float	fwidth;
    float	fheight;

    // draw the colored text
    trap_R_SetColor( color );

    ax = x * cgs.screenXScale + cgs.screenXBias;
    ay = y * cgs.screenXScale;

    s = str;
    while ( *s )
    {
        ch = *s & 127;
        if ( ch == ' ' ) {
            ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH)* cgs.screenXScale;
        }
        else if ( ch >= 'A' && ch <= 'Z' ) {
            ch -= 'A';
            fcol = (float)propMapB[ch][0] / 256.0f;
            frow = (float)propMapB[ch][1] / 256.0f;
            fwidth = (float)propMapB[ch][2] / 256.0f;
            fheight = (float)PROPB_HEIGHT / 256.0f;
            aw = (float)propMapB[ch][2] * cgs.screenXScale;
            ah = (float)PROPB_HEIGHT * cgs.screenXScale;
            trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, cgs.media.charsetPropB );
            ax += (aw + (float)PROPB_GAP_WIDTH * cgs.screenXScale);
        }
        s++;
    }

    trap_R_SetColor( NULL );
}

void UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color ) {
    const char *	s;
    int				ch;
    int				width;
    vec4_t			drawcolor;

    // find the width of the drawn text
    s = str;
    width = 0;
    while ( *s ) {
        ch = *s;
        if ( ch == ' ' ) {
            width += PROPB_SPACE_WIDTH;
        }
        else if ( ch >= 'A' && ch <= 'Z' ) {
            width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
        }
        s++;
    }
    width -= PROPB_GAP_WIDTH;

    switch( style & UI_FORMATMASK ) {
    case UI_CENTER:
        x -= width / 2;
        break;

    case UI_RIGHT:
        x -= width;
        break;

    case UI_LEFT:
    default:
        break;
    }

    if ( style & UI_DROPSHADOW ) {
        drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
        drawcolor[3] = color[3];
        UI_DrawBannerString2( x+2, y+2, str, drawcolor );
    }

    UI_DrawBannerString2( x, y, str, color );
}


int UI_ProportionalStringWidth( const char* str ) {
    const char *	s;
    int				ch;
    int				charWidth;
    int				width;

    s = str;
    width = 0;
    while ( *s ) {
        ch = *s & 127;
        charWidth = propMap[ch][2];
        if ( charWidth != -1 ) {
            width += charWidth;
            width += PROP_GAP_WIDTH;
        }
        s++;
    }

    width -= PROP_GAP_WIDTH;
    return width;
}

static void UI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset )
{
    const char* s;
    char	ch;
    float	ax;
    float	ay;
    float	aw;
    float	ah;
    float	frow;
    float	fcol;
    float	fwidth;
    float	fheight;

    // draw the colored text
    trap_R_SetColor( color );

    ax = x * cgs.screenXScale + cgs.screenXBias;
    ay = y * cgs.screenXScale;

    s = str;
    while ( *s )
    {
        ch = *s & 127;
        if ( ch == ' ' ) {
            aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale * sizeScale;
        } else if ( propMap[ch][2] != -1 ) {
            fcol = (float)propMap[ch][0] / 256.0f;
            frow = (float)propMap[ch][1] / 256.0f;
            fwidth = (float)propMap[ch][2] / 256.0f;
            fheight = (float)PROP_HEIGHT / 256.0f;
            aw = (float)propMap[ch][2] * cgs.screenXScale * sizeScale;
            ah = (float)PROP_HEIGHT * cgs.screenXScale * sizeScale;
            trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
        } else {
            aw = 0;
        }

        ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * sizeScale);
        s++;
    }

    trap_R_SetColor( NULL );
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale( int style ) {
    if(  style & UI_SMALLFONT ) {
        return 0.75;
    }

    return 1.00;
}


/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) {
    vec4_t	drawcolor;
    int		width;
    float	sizeScale;

    sizeScale = UI_ProportionalSizeScale( style );

    switch( style & UI_FORMATMASK ) {
    case UI_CENTER:
        width = UI_ProportionalStringWidth( str ) * sizeScale;
        x -= width / 2;
        break;

    case UI_RIGHT:
        width = UI_ProportionalStringWidth( str ) * sizeScale;
        x -= width;
        break;

    case UI_LEFT:
    default:
        break;
    }

    if ( style & UI_DROPSHADOW ) {
        drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
        drawcolor[3] = color[3];
        UI_DrawProportionalString2( x+2, y+2, str, drawcolor, sizeScale, cgs.media.charsetProp );
    }

    if ( style & UI_INVERSE ) {
        drawcolor[0] = color[0] * 0.8;
        drawcolor[1] = color[1] * 0.8;
        drawcolor[2] = color[2] * 0.8;
        drawcolor[3] = color[3];
        UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, cgs.media.charsetProp );
        return;
    }

    if ( style & UI_PULSE ) {
        drawcolor[0] = color[0] * 0.8;
        drawcolor[1] = color[1] * 0.8;
        drawcolor[2] = color[2] * 0.8;
        drawcolor[3] = color[3];
        UI_DrawProportionalString2( x, y, str, color, sizeScale, cgs.media.charsetProp );

        drawcolor[0] = color[0];
        drawcolor[1] = color[1];
        drawcolor[2] = color[2];
        drawcolor[3] = 0.5 + 0.5 * sin( cg.time / PULSE_DIVISOR );
        UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow );
        return;
    }

    UI_DrawProportionalString2( x, y, str, color, sizeScale, cgs.media.charsetProp );
}

