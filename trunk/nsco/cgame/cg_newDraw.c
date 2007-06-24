// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"
#include "../ui/ui_shared.h"

extern displayContextDef_t cgDC;

void CG_ForceCvar( const char *cvar, int value );

// set in CG_ParseTeamInfo

//static int sortedTeamPlayers[TEAM_MAXOVERLAY];
//static int numSortedTeamPlayers;
int drawTeamOverlayModificationCount = -1;

//static char systemChat[256];
//static char teamChat1[256];
//static char teamChat2[256];

void CG_InitTeamChat() {
    memset(teamChat1, 0, sizeof(teamChat1));
    memset(teamChat2, 0, sizeof(teamChat2));
    memset(systemChat, 0, sizeof(systemChat));
}

void CG_SetPrintString(int type, const char *p) {
    if (type == SYSTEM_PRINT) {
        strcpy(systemChat, p);
    } else {
        strcpy(teamChat2, teamChat1);
        strcpy(teamChat1, p);
    }
}
static void CG_DrawPlayerArmorIcon( rectDef_t *rect, qboolean draw2D ) {
    centity_t	*cent;
    playerState_t	*ps;
    vec3_t		angles;
    vec3_t		origin;

    if ( cg_drawStatus.integer == 0 ) {
        return;
    }

    cent = &cg_entities[cg.snap->ps.clientNum];
    ps = &cg.snap->ps;

    if ( draw2D || !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
        //	CG_DrawPic( rect->x, rect->y + rect->h/2 + 1, rect->w, rect->h, cgs.media.armorIcon );
    } else if (cg_draw3dIcons.integer) {
        VectorClear( angles );
        origin[0] = 90;
        origin[1] = 0;
        origin[2] = -10;
        angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;

        // CG_Draw3DModel( rect->x, rect->y, rect->w, rect->h, cgs.media.armorModel, 0, origin, angles );
    }

}

static void CG_DrawPlayerArmorValue(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    char	num[16];
    int value;
    centity_t	*cent;
    playerState_t	*ps;

    cent = &cg_entities[cg.snap->ps.clientNum];
    ps = &cg.snap->ps;

    value = cg.snap->ps.stats[STAT_ROUNDS];

    if (shader) {
        trap_R_SetColor( color );
        CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
        trap_R_SetColor( NULL );
    } else {
        // BLUTENGEL:
        // is this the occurence of the ammo displayed wrong? probably
        // yes!
        Com_sprintf (num, sizeof(num), "%i", (value > 0 ? value : 0));
        value = CG_Text_Width(num, scale, 0);
        CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
    }
}

static float healthColors[4][4] = {
                                      //		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
                                      { 1, 0.69f, 0, 1.0f } ,		// normal
                                      { 1.0f, 0.2f, 0.2f, 1.0f },	// low health
                                      {0.5f, 0.5f, 0.5f, 1},		// weapon firing
                                      { 1, 1, 1, 1 }
                                  };			// health > 100


static void CG_DrawPlayerAmmoValue(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    char	num[16];
    int value;
    centity_t	*cent;
    playerState_t	*ps;

    cent = &cg_entities[cg.snap->ps.clientNum];
    ps = &cg.snap->ps;

    if ( cent->currentState.weapon ) {
        gitem_t *it;

        it = BG_FindItemForWeapon( cent->currentState.weapon );

        if ( !it )
            return;

        value = ps->ammo[it->giAmmoTag];

        if ( value > -1 ) {
            if (shader) {
                trap_R_SetColor( color );
                CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
                trap_R_SetColor( NULL );
            } else {
                Com_sprintf (num, sizeof(num), "%i", value);
                value = CG_Text_Width(num, scale, 0);
                CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
            }
        }
    }

}



static void CG_DrawPlayerHead(rectDef_t *rect, qboolean draw2D) {
    vec3_t		angles;
    float		size, stretch;
    float		frac;
    float		x = rect->x;

    VectorClear( angles );

    if ( cg.damageTime && cg.time - cg.damageTime < cg.damageDuration  ) {
        frac = (float)(cg.time - cg.damageTime ) / cg.damageDuration;
        size = rect->w * 1.25 * ( 1.5 - frac * 0.5 );

        stretch = size - rect->w * 1.25;
        // kick in the direction of damage
        x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

        cg.headStartYaw = 180 + cg.damageX * 45;

        cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
        cg.headEndPitch = 5 * cos( crandom()*M_PI );

        cg.headStartTime = cg.time;
        cg.headEndTime = cg.time + 100 + random() * 2000;
    } else {
        if ( cg.time >= cg.headEndTime ) {
            // select a new head angle
            cg.headStartYaw = cg.headEndYaw;
            cg.headStartPitch = cg.headEndPitch;
            cg.headStartTime = cg.headEndTime;
            cg.headEndTime = cg.time + 100 + random() * 2000;

            cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
            cg.headEndPitch = 5 * cos( crandom()*M_PI );
        }

        size = rect->w * 1.25;
    }

    // if the server was frozen for a while we may have a bad head start time
    if ( cg.headStartTime > cg.time ) {
        cg.headStartTime = cg.time;
    }

    frac = ( cg.time - cg.headStartTime ) / (float)( cg.headEndTime - cg.headStartTime );
    frac = frac * frac * ( 3 - 2 * frac );
    angles[YAW] = cg.headStartYaw + ( cg.headEndYaw - cg.headStartYaw ) * frac;
    angles[PITCH] = cg.headStartPitch + ( cg.headEndPitch - cg.headStartPitch ) * frac;

    CG_DrawHead( x, rect->y, rect->w, rect->h, cg.snap->ps.clientNum, angles );
}


#ifdef IS_REDUNDANT
qhandle_t CG_StatusHandle(int task) {
    qhandle_t h = cgs.media.bloodHitShader;


    return h;
}
static void CG_DrawPlayerStatus( rectDef_t *rect ) {
    clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
    if (ci) {
        qhandle_t h = CG_StatusHandle(ci->teamTask);
        CG_DrawPic( rect->x, rect->y, rect->w, rect->h, h);
    }
}
#endif



static void CG_DrawPlayerLocation( rectDef_t *rect, float scale, vec4_t color, int textStyle  ) {
    clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];
    if (ci) {
        const char *p = CG_ConfigString(CS_LOCATIONS + ci->location);
        if (!p || !*p) {
            p = "unknown";
        }
        CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, p, 0, 0, textStyle);
    }
}


static void CG_DrawPlayerScore( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
    char num[16];
    int value = cg.snap->ps.persistant[PERS_SCORE];

    if (shader) {
        trap_R_SetColor( color );
        CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
        trap_R_SetColor( NULL );
    } else {
        Com_sprintf (num, sizeof(num), "%i", value);
        value = CG_Text_Width(num, scale, 0);
        CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
    }
}
 
 
static void CG_DrawPlayerHealth(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
    playerState_t	*ps;
    int value;
    char	num[16];

    ps = &cg.snap->ps;

    value = ps->stats[STAT_HEALTH];

    if (shader) {
        trap_R_SetColor( color );
        CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
        trap_R_SetColor( NULL );
    } else {
        Com_sprintf (num, sizeof(num), "%i", value);
        value = CG_Text_Width(num, scale, 0);
        CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
    }
}


static void CG_DrawRedScore(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
    int value;
    char num[16];
    if ( cgs.scores1 == SCORE_NOT_PRESENT ) {
        Com_sprintf (num, sizeof(num), "-");
    }
    else {
        Com_sprintf (num, sizeof(num), "%i", cgs.scores1);
    }
    value = CG_Text_Width(num, scale, 0);
    CG_Text_Paint(rect->x + rect->w - value, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
}

static void CG_DrawBlueScore(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
    int value;
    char num[16];

    if ( cgs.scores2 == SCORE_NOT_PRESENT ) {
        Com_sprintf (num, sizeof(num), "-");
    }
    else {
        Com_sprintf (num, sizeof(num), "%i", cgs.scores2);
    }
    value = CG_Text_Width(num, scale, 0);
    CG_Text_Paint(rect->x + rect->w - value, rect->y + rect->h, scale, color, num, 0, 0, textStyle);
}









static void CG_DrawTeamColor(rectDef_t *rect, vec4_t color) {
    CG_DrawTeamBackground(rect->x, rect->y, rect->w, rect->h, color[3], cg.snap->ps.persistant[PERS_TEAM]);
}

static void CG_DrawAreaPowerUp(rectDef_t *rect, int align, float special, float scale, vec4_t color) {
    char num[16];
    int		sorted[MAX_POWERUPS];
    int		sortedTime[MAX_POWERUPS];
    int		i, j, k;
    int		active;
    playerState_t	*ps;
    int		t;
    gitem_t	*item;
    float	f;
    rectDef_t r2;
    float *inc;
    r2.x = rect->x;
    r2.y = rect->y;
    r2.w = rect->w;
    r2.h = rect->h;

    inc = (align == HUD_VERTICAL) ? &r2.y : &r2.x;

    ps = &cg.snap->ps;

    if ( ps->stats[STAT_HEALTH] <= 0 ) {
        return;
    }

    // sort the list by time remaining
    active = 0;
    for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
        if ( !ps->powerups[ i ] ) {
            continue;
        }
        t = ps->powerups[ i ] - cg.time;
        // ZOID--don't draw if the power up has unlimited time (999 seconds)
        // This is true of the CTF flags
        if ( t <= 0 || t >= 999000) {
            continue;
        }

        // insert into the list
        for ( j = 0 ; j < active ; j++ ) {
            if ( sortedTime[j] >= t ) {
                for ( k = active - 1 ; k >= j ; k-- ) {
                    sorted[k+1] = sorted[k];
                    sortedTime[k+1] = sortedTime[k];
                }
                break;
            }
        }
        sorted[j] = i;
        sortedTime[j] = t;
        active++;
    }

    // draw the icons and timers
    for ( i = 0 ; i < active ; i++ ) {
        item = BG_FindItemForPowerup( sorted[i] );

        if (item) {
            t = ps->powerups[ sorted[i] ];
            if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
                trap_R_SetColor( NULL );
            } else {
                vec4_t	modulate;

                f = (float)( t - cg.time ) / POWERUP_BLINK_TIME;
                f -= (int)f;
                modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
                trap_R_SetColor( modulate );
            }

            CG_DrawPic( r2.x, r2.y, r2.w * .75, r2.h, trap_R_RegisterShader( item->icon ) );

            Com_sprintf (num, sizeof(num), "%i", sortedTime[i] / 1000);
            CG_Text_Paint(r2.x + (r2.w * .75) + 3 , r2.y + r2.h, scale, color, num, 0, 0, 0);
            *inc += r2.w + special;
        }

    }
    trap_R_SetColor( NULL );

}

float CG_GetValue(int ownerDraw) {
    centity_t	*cent;
    playerState_t	*ps;

    cent = &cg_entities[cg.snap->ps.clientNum];
    ps = &cg.snap->ps;

    switch (ownerDraw)
    {
    case CG_PLAYER_ROUNDS_VALUE:
        return ps->stats[STAT_ROUNDS];
        break;
    case CG_PLAYER_AMMO_VALUE:
        if ( cent->currentState.weapon ) {
            gitem_t *it;

            it = BG_FindItemForWeapon( cent->currentState.weapon );

            if (!it)
                return 0;

            return ps->ammo[it->giAmmoTag];
        }
        break;
    case CG_PLAYER_SCORE:
        return cg.snap->ps.persistant[PERS_SCORE];
        break;
    case CG_PLAYER_HEALTH:
        return ps->stats[STAT_HEALTH];
        break;
    case CG_RED_SCORE:
        return cgs.scores1;
        break;
    case CG_BLUE_SCORE:
        return cgs.scores2;
        break;
    default:
        break;
    }
    return -1;
}


// THINKABOUTME: should these be exclusive or inclusive..
//
qboolean CG_OwnerDrawVisible(int flags) {


    // only show if we're on the waiting line
    if (flags & CG_SHOW_SPECTATOR ) {
        return ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP );
    }
    if (flags & CG_SHOW_NO_SPECTATOR ) {
        if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg.snap->ps.pm_type == PM_SPECTATOR || cg.snap->ps.pm_type == PM_NOCLIP )
            return qfalse;

        return qtrue;
    }

    if (flags & (CG_SHOW_BLUE_TEAM_HAS_REDFLAG | CG_SHOW_RED_TEAM_HAS_BLUEFLAG)) {
        if (flags & CG_SHOW_BLUE_TEAM_HAS_REDFLAG && (cgs.redflag == FLAG_TAKEN || cgs.flagStatus == FLAG_TAKEN_RED)) {
            return qtrue;
        } else if (flags & CG_SHOW_RED_TEAM_HAS_BLUEFLAG && (cgs.blueflag == FLAG_TAKEN || cgs.flagStatus == FLAG_TAKEN_BLUE)) {
            return qtrue;
        }
        return qfalse;
    }

    if (flags & CG_SHOW_ANYTEAMGAME) {
        if( cgs.gametype >= GT_TEAM) {
            return qtrue;
        }
    }

    if (flags & CG_SHOW_ANYNONTEAMGAME) {
        if( cgs.gametype < GT_TEAM) {
            return qtrue;
        }
    }

    if (flags & CG_SHOW_HEALTHCRITICAL) {
        if (cg.snap->ps.stats[STAT_HEALTH] < 25) {
            return qtrue;
        }
    }

    if (flags & CG_SHOW_HEALTHOK) {
        if (cg.snap->ps.stats[STAT_HEALTH] > 25) {
            return qtrue;
        }
    }

    if (flags & CG_SHOW_DURINGINCOMINGVOICE) {
    }

    if ( flags & CG_SHOW_IF_PLAYER_HAS_FLAG) {
        if ( cg.snap->ps.powerups[PW_BRIEFCASE]  ) {
            return qtrue;
        }
    }
    return qfalse;
}




static void CG_DrawAreaSystemChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, systemChat, 0, 0, 0);
}

static void CG_DrawAreaTeamChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color,teamChat1, 0, 0, 0);
}

static void CG_DrawAreaChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, teamChat2, 0, 0, 0);
}

const char *CG_GetKillerText() {
    const char *s = "";
    if ( cg.killerName[0] ) {
        s = va("Fragged by %s", cg.killerName );
    }
    return s;
}


static void CG_DrawKiller(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
    // fragged by ... line
    if ( cg.killerName[0] ) {
        int x = rect->x + rect->w / 2;
        CG_Text_Paint(x - CG_Text_Width(CG_GetKillerText(), scale, 0) / 2, rect->y + rect->h, scale, color, CG_GetKillerText(), 0, 0, textStyle);
    }

}


static void CG_DrawCapFragLimit(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    int limit = cgs.teampointlimit;
    CG_Text_Paint(rect->x, rect->y, scale, color, va("%2i", limit),0, 0, textStyle);
}

static void CG_Draw1stPlace(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    if (cgs.scores1 != SCORE_NOT_PRESENT) {
        CG_Text_Paint(rect->x, rect->y, scale, color, va("%2i", cgs.scores1),0, 0, textStyle);
    }
}

static void CG_Draw2ndPlace(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    if (cgs.scores2 != SCORE_NOT_PRESENT) {
        CG_Text_Paint(rect->x, rect->y, scale, color, va("%2i", cgs.scores2),0, 0, textStyle);
    }
}

const char *CG_GetGameStatusText() {
    const char *s = "";
    if ( cgs.gametype < GT_TEAM) {
        if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
            s = va("%s place with %i",CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),cg.snap->ps.persistant[PERS_SCORE] );
        }
    } else {
        if ( cg.teamScores[0] == cg.teamScores[1] ) {
            s = va("Teams are tied at %i", cg.teamScores[0] );
        } else if ( cg.teamScores[0] >= cg.teamScores[1] ) {
            s = va("Seals lead Tangos, %i to %i", cg.teamScores[0], cg.teamScores[1] );
        } else {
            s = va("Tangos lead Seals, %i to %i", cg.teamScores[1], cg.teamScores[0] );
        }
    }
    return s;
}

static void CG_DrawGameStatus(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GetGameStatusText(), 0, 0, textStyle);
}

const char *CG_GameTypeString() {
    if ( cgs.gametype == GT_FFA ) {
        return "Free For All";
    } else if ( cgs.gametype == GT_TEAM ) {
        return "Free Teamplay";
    }/* else if ( cgs.gametype == GT_TRAINING ) {
     return "Training";
     }*/ else if ( cgs.gametype == GT_LTS) {
        return "Mission Objective";
    }
    return "";
}
static void CG_DrawGameType(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle ) {
    CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GameTypeString(), 0, 0, textStyle);
}

static void CG_Text_Paint_Limit(float *maxX, float x, float y, float scale, vec4_t color, const char* text, float adjust, int limit) {
    int len, count;
    vec4_t newColor;
    glyphInfo_t *glyph;
    if (text) {
        const char *s = text;
        float max = *maxX;
        float useScale;
        fontInfo_t *font = &cgDC.Assets.textFont;
        if (scale <= cg_smallFont.value) {
            font = &cgDC.Assets.smallFont;
        } else if (scale > cg_bigFont.value) {
            font = &cgDC.Assets.bigFont;
        }
        useScale = scale * font->glyphScale;
        trap_R_SetColor( color );
        len = strlen(text);
        if (limit > 0 && len > limit) {
            len = limit;
        }
        count = 0;
        while (s && *s && count < len) {
            glyph = &font->glyphs[*s];
            if ( Q_IsColorString( s ) ) {
                memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
                newColor[3] = color[3];
                trap_R_SetColor( newColor );
                s += 2;
                continue;
            } else {
                float yadj = useScale * glyph->top;
                if (CG_Text_Width(s, useScale, 1) + x > max) {
                    *maxX = 0;
                    break;
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
                x += (glyph->xSkip * useScale) + adjust;
                *maxX = x;
                count++;
                s++;
            }
        }
        trap_R_SetColor( NULL );
    }

}



#define PIC_WIDTH 12

void CG_DrawNewTeamInfo(rectDef_t *rect, float text_x, float text_y, float scale, vec4_t color, qhandle_t shader) {
    int xx;
    float y;
    int i, j, len, count;
    const char *p;
    vec4_t		hcolor;
    float pwidth, lwidth, maxx, leftOver;
    clientInfo_t *ci;
    gitem_t	*item;
    qhandle_t h;

    // max player name width
    pwidth = 0;
    count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
    for (i = 0; i < count; i++) {
        ci = cgs.clientinfo + sortedTeamPlayers[i];
        if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
            len = CG_Text_Width( ci->name, scale, 0);
            if (len > pwidth)
                pwidth = len;
        }
    }

    // max location name width
    lwidth = 0;
    for (i = 1; i < MAX_LOCATIONS; i++) {
        p = CG_ConfigString(CS_LOCATIONS + i);
        if (p && *p) {
            len = CG_Text_Width(p, scale, 0);
            if (len > lwidth)
                lwidth = len;
        }
    }

    y = rect->y;

    for (i = 0; i < count; i++) {
        ci = cgs.clientinfo + sortedTeamPlayers[i];
        if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

            xx = rect->x + 1;
            for (j = 0; j <= PW_NUM_POWERUPS; j++) {
                if (ci->powerups & (1 << j)) {

                    item = BG_FindItemForPowerup( j );

                    if (item) {
                        CG_DrawPic( xx, y, PIC_WIDTH, PIC_WIDTH, trap_R_RegisterShader( item->icon ) );
                        xx += PIC_WIDTH;
                    }
                }
            }

            // FIXME: max of 3 powerups shown properly
            xx = rect->x + (PIC_WIDTH * 3) + 2;

            CG_GetColorForHealth( ci->health, ci->armor, hcolor );
            trap_R_SetColor(hcolor);
            //			CG_DrawPic( xx, y + 1, PIC_WIDTH - 2, PIC_WIDTH - 2, cgs.media.heartShader );

            //Com_sprintf (st, sizeof(st), "%3i %3i", ci->health,	ci->armor);
            //CG_Text_Paint(xx, y + text_y, scale, hcolor, st, 0, 0);

            // draw weapon icon
            xx += PIC_WIDTH + 1;

            // weapon used is not that useful, use the space for task
#if 0
            if ( cg_weapons[ci->curWeapon].weaponIcon ) {
                CG_DrawPic( xx, y, PIC_WIDTH, PIC_WIDTH, cg_weapons[ci->curWeapon].weaponIcon );
            } else {
                CG_DrawPic( xx, y, PIC_WIDTH, PIC_WIDTH, cgs.media.deferShader );
            }
#endif

            trap_R_SetColor(NULL);
            if (cgs.orderPending) {
                // blink the icon
                if ( cg.time > cgs.orderTime - 2500 && (cg.time >> 9 ) & 1 ) {
                    h = 0;
                }
#ifdef IS_REDUNDANT
                else {
                    h = CG_StatusHandle(cgs.currentOrder);
                }
#endif
            }

#ifdef IS_REDUNDANT
            else {
                h = CG_StatusHandle(ci->teamTask);
            }
#endif
            if (h) {
                CG_DrawPic( xx, y, PIC_WIDTH, PIC_WIDTH, h);
            }

            xx += PIC_WIDTH + 1;

            leftOver = rect->w - xx;
            maxx = xx + leftOver / 3;



            CG_Text_Paint_Limit(&maxx, xx, y + text_y, scale, color, ci->name, 0, 0);

            p = CG_ConfigString(CS_LOCATIONS + ci->location);
            if (!p || !*p) {
                p = "unknown";
            }

            xx += leftOver / 3 + 2;
            maxx = rect->w - 4;

            CG_Text_Paint_Limit(&maxx, xx, y + text_y, scale, color, p, 0, 0);
            y += text_y + 2;
            if ( y + text_y + 2 > rect->y + rect->h ) {
                break;
            }

        }
    }
}


void CG_DrawTeamSpectators(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader) {
    if (cg.spectatorLen)
    {
        float maxX;

        if (cg.spectatorWidth == -1) {
            cg.spectatorWidth = 0;
            cg.spectatorPaintX = rect->x + 1;
            cg.spectatorPaintX2 = -1;
        }

        if (cg.spectatorOffset > cg.spectatorLen) {
            cg.spectatorOffset = 0;
            cg.spectatorPaintX = rect->x + 1;
            cg.spectatorPaintX2 = -1;
        }

        if (cg.time > cg.spectatorTime) {
            cg.spectatorTime = cg.time + 10;
            if (cg.spectatorPaintX <= rect->x + 2) {
                if (cg.spectatorOffset < cg.spectatorLen) {
                    cg.spectatorPaintX += CG_Text_Width(&cg.spectatorList[cg.spectatorOffset], scale, 1) - 1;
                    cg.spectatorOffset++;
                } else {
                    cg.spectatorOffset = 0;
                    if (cg.spectatorPaintX2 >= 0) {
                        cg.spectatorPaintX = cg.spectatorPaintX2;
                    } else {
                        cg.spectatorPaintX = rect->x + rect->w - 2;
                    }
                    cg.spectatorPaintX2 = -1;
                }
            } else {
                cg.spectatorPaintX--;
                if (cg.spectatorPaintX2 >= 0) {
                    cg.spectatorPaintX2--;
                }
            }
        }

        maxX = rect->x + rect->w - 2;
        CG_Text_Paint_Limit(&maxX, cg.spectatorPaintX, rect->y + rect->h - 3, scale, color, &cg.spectatorList[cg.spectatorOffset], 0, 0);
        if (cg.spectatorPaintX2 >= 0) {
            float maxX2 = rect->x + rect->w - 2;
            CG_Text_Paint_Limit(&maxX2, cg.spectatorPaintX2, rect->y + rect->h - 3, scale, color, cg.spectatorList, 0, cg.spectatorOffset);
        }
        if (cg.spectatorOffset && maxX > 0) {
            // if we have an offset ( we are skipping the first part of the string ) and we fit the string
            if (cg.spectatorPaintX2 == -1) {
                cg.spectatorPaintX2 = rect->x + rect->w - 2;
            }
        } else {
            cg.spectatorPaintX2 = -1;
        }

    }
}

#define STAMINA_MAX	300

static void CG_DrawStaminaBarVert( rectDef_t *rect , vec4_t color, qhandle_t shader) {

    int height;
    int stamina = cg.snap->ps.stats[STAT_STAMINA];

    height = ( rect->h / STAMINA_MAX ) * stamina;


    trap_R_SetColor(color);
    CG_DrawPic( rect->x, rect->y, rect->w, height, shader );
    trap_R_SetColor(NULL);

}

static void CG_DrawStaminaBarHor( rectDef_t *rect , vec4_t color, qhandle_t shader) {

    int width;
    int stamina = cg.snap->ps.stats[STAT_STAMINA];

    width = ( rect->w / STAMINA_MAX ) * stamina;


    trap_R_SetColor(color);
    CG_DrawPic( rect->x, rect->y, width, rect->h, shader );
    trap_R_SetColor(NULL);

}

int BG_GetMaxRoundForWeapon( int weapon );

static void CG_DrawAmmoBarHor( rectDef_t *rect , vec4_t color, qhandle_t shader) {

    int width;
    int maxrnd = BG_GetMaxRoundForWeapon( cg.snap->ps.weapon );
    int rnds = cg.snap->ps.stats[STAT_ROUNDS];

    if ( BG_IsGrenade( cg.snap->ps.weapon ) || BG_IsMelee( cg.snap->ps.weapon ) )
        return;

    width = ( rect->w / maxrnd ) * rnds;


    trap_R_SetColor(color);
    CG_DrawPic( rect->x, rect->y, width, rect->h, shader );
    trap_R_SetColor(NULL);


}

static void CG_DrawAmmoBarVert( rectDef_t *rect , vec4_t color, qhandle_t shader) {

    int height;
    int maxrnd = BG_GetMaxRoundForWeapon( cg.snap->ps.weapon );
    int rnds = cg.snap->ps.stats[STAT_ROUNDS];

    if ( BG_IsGrenade( cg.snap->ps.weapon ) || BG_IsMelee( cg.snap->ps.weapon ) )
        return;

    height = ( rect->h / maxrnd ) * rnds;


    trap_R_SetColor(color);
    CG_DrawPic( rect->x, rect->y, rect->w, height, shader );
    trap_R_SetColor(NULL);


}

static void CG_DrawClipGfx( rectDef_t *rect ) {

    int weapon = cg.snap->ps.weapon;
    int value;
    playerState_t	*ps;

    ps = &cg.snap->ps;

    if ( BG_IsGrenade( cg.snap->ps.weapon ) || BG_IsMelee( cg.snap->ps.weapon ) )
        return;

    //
    // rounds
    //
    if (  weapon > WP_FLASHBANG &&
            !( ( weapon == WP_M4 ||  weapon == WP_AK47) && (ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) ) && ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) ) ) &&
            !( ( weapon == WP_M4 ||  weapon == WP_AK47) && (ps->stats[STAT_WEAPONMODE] & ( 1 << WM_BAYONET ) ) && ( ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ) ) ) )
    {

        int i,x1 ;
        int times = 0;

        value = ps->stats[STAT_ROUNDS]; // dX

        if ( cg.predictedPlayerState.weaponstate == WEAPON_FIRING
                && cg.predictedPlayerState.weaponTime > 50 )
            trap_R_SetColor( colorBlue );
        else if ( cg.predictedPlayerState.weaponstate == WEAPON_RELOADING
                  && cg.predictedPlayerState.weaponTime > 100 )
            trap_R_SetColor( colorGreen );
        else {
            if (value <= 5)
                trap_R_SetColor( colorRed );
            else
                trap_R_SetColor( colorWhite );
        }

        while (value > 15)
        {
            times++;
            value -= 15;
        }

        x1 = rect->x;

        for (i=0;i<15;i++)
        {
            if ( i < value ) {
                if ( cg.predictedPlayerState.weaponstate == WEAPON_FIRING
                        && cg.predictedPlayerState.weaponTime > 50 )
                    trap_R_SetColor( colorBlue );
            }
            else
                trap_R_SetColor( colorBlack );

            if ( BG_IsRifle( ps->weapon ) )
                CG_DrawPic( x1, rect->y , 16, rect->h, cgs.media.ammoMag_bullet[1] );
            else if ( BG_IsShotgun( ps->weapon ) )
                CG_DrawPic( x1, rect->y , 16, rect->h , cgs.media.ammoMag_bullet[2] );
            else
                CG_DrawPic( x1, rect->y , 16, rect->h, cgs.media.ammoMag_bullet[0] );

            x1 += 16;

            if ( i==14)
                break;
        }

    }
}

/*
==============
CG_DrawDigits

Draws Digits for Hud
==============
*/
static void CG_DrawDigits2 (int x, int y, int width,int value) {
    char	num[16], *ptr;
    int		l;
    int		frame;

    if ( width < 1 ) {
        return;
    }

    // draw number string
    if ( width > 5 ) {
        width = 5;
    }

    switch ( width ) {
    case 1:
        value = value > 9 ? 9 : value;
        value = value < 0 ? 0 : value;
        Com_sprintf (num, sizeof(num), "%i", value);
        break;
    case 2:
        value = value > 99 ? 99 : value;
        value = value < -9 ? -9 : value;

        if ( value < 10 )
            Com_sprintf (num, sizeof(num), "0%i", value);
        else
            Com_sprintf (num, sizeof(num), "%i", value);
        break;
    case 3:
        value = value > 999 ? 999 : value;
        value = value < -99 ? -99 : value;
        if ( value < 10 )
            Com_sprintf (num, sizeof(num), "00%i", value);
        else if ( value < 100 )
            Com_sprintf (num, sizeof(num), "0%i", value);
        else
            Com_sprintf (num, sizeof(num), "%i", value);
        break;
    case 4:
        value = value > 9999 ? 9999 : value;
        value = value < -999 ? -999 : value;
        if ( value < 10 )
            Com_sprintf (num, sizeof(num), "000%i", value);
        else if ( value < 100 )
            Com_sprintf (num, sizeof(num), "00%i", value);
        else if ( value < 1000 )
            Com_sprintf (num, sizeof(num), "0%i", value);
        else
            Com_sprintf (num, sizeof(num), "%i", value);
        break;
    }

    l = strlen(num);
    if (l > width)
        l = width;
    x += 2 + 24*(width - l);

    ptr = num;
    while (*ptr && l)
    {
        if (*ptr == '-')
            frame = STAT_MINUS;
        else
            frame = *ptr -'0';

        CG_DrawPic( x,y, 24, 32, cgs.media.digitalNumberShaders[frame] );
        x += 24;
        ptr++;
        l--;
    }
}

static void CG_DrawClipValue( rectDef_t *rect )
{
    int value;
    int times = 0;

    value = cg.snap->ps.stats[STAT_ROUNDS]; // dX

    if ( BG_IsGrenade( cg.snap->ps.weapon ) || BG_IsMelee( cg.snap->ps.weapon ) )
        return;

    while (value > 15)
    {
        times++;
        value -= 15;
    }

    trap_R_SetColor(colorWhite);

    CG_DrawPic(  rect->x, rect->y, 16, rect->h, cgs.media.slashIcon );
    CG_DrawDigits2 (  rect->x + 16 , rect->y , 2 , times);

    trap_R_SetColor( NULL );

}

static void CG_DrawDamageLoc ( rectDef_t *rect, int special ) {
    vec4_t		hcolor;
    playerState_t	*ps;
    float pulse = sin( cg.time / 25  );

    ps = &cg.snap->ps;

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH] - ps->stats[STAT_CHEST_DAMAGE] , 0, hcolor );
    if ( cg.flashDmgLocTime[0] > cg.time && ( cg.flashDmgLocTime[0] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2];
            */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_chestIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_STOMACH_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[1] > cg.time && ( cg.flashDmgLocTime[1] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_stomachIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 1 0 0 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_ARM_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[2] > cg.time && ( cg.flashDmgLocTime[2] - cg.time ) > 0  )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_leftArmIcon);
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_rightArmIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 100 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_LEG_DAMAGE] ,0, hcolor );
    if ( cg.time < cg.flashDmgLocTime[3] && ( cg.flashDmgLocTime[3] - cg.time ) > 0 )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
        /*
        hcolor[0] *= sin( cg.time / PULSE_DIVISOR );
        hcolor[1] *= sin( cg.time / PULSE_DIVISOR );
        hcolor[2] *= sin( cg.time / PULSE_DIVISOR );
        */
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_leftLegIcon);
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_rightLegIcon);
    trap_R_SetColor( NULL );

    CG_GetColorForHealth( /* 100 - */ ps->stats[STAT_HEALTH]  - ps->stats[STAT_HEAD_DAMAGE] ,0, hcolor );
    if ( cg.flashDmgLocTime[4] > cg.time && ( cg.flashDmgLocTime[4] - cg.time ) > 0 )
    {
        if ( pulse < 0.0 )
        {
            /*
            hcolor[0] = colorWhite[0];
            hcolor[1] = colorWhite[1];
            hcolor[2] = colorWhite[2]; */
        }
        else
        {
            hcolor[0] = colorBlack[0];
            hcolor[1] = colorBlack[1];
            hcolor[2] = colorBlack[2];
        }
    }
    trap_R_SetColor( hcolor );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_headIcon);
    trap_R_SetColor( NULL );

    trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLACK)] );
    CG_DrawPic( rect->x, rect->y, rect->w, rect->h ,cgs.media.loc_bodyLines);
    trap_R_SetColor( NULL );
}
static void CG_DrawWeaponIcon( rectDef_t *rect, int special ) {
    qhandle_t	icon = 0;
    qboolean	weapon = qtrue;
    qboolean	item = qfalse;
    int			wNum = WP_NONE;

    if ( special )
    {
        wNum = special;
    }

    if (!weapon&&!item)
        return;

    if ( weapon )
    {

        icon = cg_weapons[wNum].weaponIcon;
    }

    if ( icon ) {
        CG_DrawPic( rect->x, rect->y, rect->w, rect->h, icon );
    }
}

static void CG_DrawClipIcon( rectDef_t *rect ) {
    playerState_t	*ps;
    int	weapon = cg.snap->ps.weapon;

    ps = &cg.snap->ps;


    if ( weapon > WP_SEALKNIFE) {
        int		clipGfx = 0;

        // background layer for clips + ammo
        //	CG_DrawTeamBackground( 88, 447, 640, 40, 0.33f, cg.snap->ps.persistant[PERS_TEAM] );

        if ( BG_IsShotgun( weapon ) )
            clipGfx = 1;
        else if ( weapon == WP_GRENADE )
            clipGfx = 2;
        else if ( weapon == WP_FLASHBANG || weapon == WP_SMOKE )
            clipGfx = 3;
        else if (ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) && ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ))
            clipGfx = 4;

        // draw the bullet pic
        trap_R_SetColor(colorWhite);
        CG_DrawPic( rect->x, rect->y , rect->w, rect->h, cgs.media.clipIcon[clipGfx] );
        trap_R_SetColor( NULL );
    }

}

static void CG_DrawClipDigitValue( rectDef_t *rect ) {
    playerState_t	*ps;
    int	value = -1;
    int weapon = cg.snap->ps.weapon ;

    ps = &cg.snap->ps;


    if ( weapon > WP_SEALKNIFE) {
        gitem_t *item;

        item = BG_FindItemForWeapon( weapon );

        if (ps->stats[STAT_WEAPONMODE] & ( 1 << WM_GRENADELAUNCHER ) && ps->stats[STAT_WEAPONMODE] & ( 1 << WM_WEAPONMODE2 ))
            value = ps->ammo[ AM_40MMGRENADES ]; // dX
        else
            value = ps->ammo[ item->giAmmoTag ]; // dX

        if ( value > -1 ) {

            if ( value <= 1 && ( (cg.time >> 8) & 1 && value <= 1))
                trap_R_SetColor(colorRed);
            else
                trap_R_SetColor(colorWhite);

            //CG_DrawDigits2 (  rect->x + 16 , rect->y , 2, rect->h - (rect->h/3), rect->h, times);
            CG_DrawDigits2 ( rect->x , rect->y , 2 , value);

            trap_R_SetColor( NULL );
        }
    }
}


void CG_NewbieMessage( const char *str, int y, float charHeight );
/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity( void ) {
    trace_t		trace;
    vec3_t		start, end;
    int			content;

    VectorCopy( cg.refdef.vieworg, start );
    VectorMA( start, 512, cg.refdef.viewaxis[0], end );

    CG_Trace( &trace, start, vec3_origin, vec3_origin, end,
              cg.snap->ps.clientNum, MASK_ALL );
    if ( trace.entityNum >= MAX_CLIENTS &&
            cg_entities[trace.entityNum].currentState.eType != ET_DOOR &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT0 &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT1 &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT2 &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT3 &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT4 &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT5 &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT6 &&
            cg_entities[trace.entityNum].currentState.eType != ET_ELEVBUT7 ) {
        return;
    }
    if ( cg_entities[trace.entityNum].currentState.eType == ET_DOOR )
    {
        if ( Distance( trace.endpos, cg.refdef.vieworg ) > 128 )
            return;
    } else if ( cg_entities[trace.entityNum].currentState.eType >= ET_ELEVBUT0 &&
                cg_entities[trace.entityNum].currentState.eType <= ET_ELEVBUT7 ) {
        if ( Distance( trace.endpos, cg.refdef.vieworg ) > 32 )
            return;
    }


    // if the player is in fog, don't show it
    content = trap_CM_PointContents( trace.endpos, 0 );
    if ( content & CONTENTS_FOG ) {
        return;
    }

    if ( cg.FlashTime - cg.time  >= 250 )
        return;

    // if the player is invisible, don't show it
#if 0
    if ( cg_entities[ trace.entityNum ].currentState.powerups & ( 1 << PW_INVIS ) ) {
        return;
    }
#endif


    // update the fade timer
    cg.crosshairClientNum = trace.entityNum;
    cg.crosshairClientTime = cg.time;
}

static void CG_DrawCrosshairEntity(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    float		*newcolor;
    vec4_t		vec_color;
    char		*name;
    int			x;

    if ( !cg_drawCrosshair.integer ) {
        return;
    }
    if ( !cg_drawCrosshairNames.integer ) {
        return;
    }
    if ( cg.renderingThirdPerson ) {
        return;
    }

    // scan the known entities to see if the crosshair is sighted on one
    CG_ScanForCrosshairEntity();

    // draw the name of the player being looked at
    if ( cg.crosshairClientNum >= MAX_CLIENTS )
        newcolor = CG_FadeColor( cg.crosshairClientTime, 250 );
    else
        newcolor = CG_FadeColor( cg.crosshairClientTime, 500 );
    if ( !newcolor ) {
        trap_R_SetColor( NULL );
        return;
    }

    //	newcolor[3] *= 0.5;

    vec_color[0] = 0.3f;
    vec_color[1] = 1;
    vec_color[2] = 0.3f;
    vec_color[3] = newcolor[3];

    if ( cg.snap->ps.pm_type == PM_SPECTATOR )
    {
        vec_color[0] = 1;
        vec_color[1] = 1;
        vec_color[2] = 1;

        name = va("%s", cgs.clientinfo[ cg.crosshairClientNum ].name );
    }
    else if ( cgs.gametype >= GT_TEAM )
    {
        if ( cg.crosshairClientNum >= MAX_CLIENTS &&
                cg_entities[cg.crosshairClientNum].currentState.eType == ET_DOOR )
        {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Open Door");

            if ( !cg.ns_newbiehelp.w_doorSpotted )
            {
                cg.ns_newbiehelp.w_doorSpotted = 1;
                CG_NewbieMessage(S_COLOR_GREEN "You're infront of a door,\npress and hold your USE key to open it.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
            }
        }
        else if ( cgs.clientinfo[ cg.crosshairClientNum ].team == cg.snap->ps.persistant[PERS_TEAM] )
        {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;
            name = va(S_COLOR_GREEN"Mate:"S_COLOR_WHITE" %s", cgs.clientinfo[ cg.crosshairClientNum ].name );

            if ( !cg.ns_newbiehelp.w_friendSpotted  )
            {
                cg.ns_newbiehelp.w_friendSpotted = 1;
                CG_NewbieMessage(S_COLOR_GREEN "You've spotted a team-mate.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
            }
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT0 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 0");
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT1 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 1");
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT2 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 2");
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT3 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 3");
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT4 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 4");
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT5 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 5");
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT6 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 6");
        }
        else if ( cg.crosshairClientNum >= MAX_CLIENTS && cg_entities[ cg.crosshairClientNum ].currentState.eType == ET_ELEVBUT7 ) {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;

            name = va(S_COLOR_YELLOW"Use:"S_COLOR_WHITE" Level 7");
        }
        else
        {
            vec_color[0] = 1;
            vec_color[1] = 1;
            vec_color[2] = 1;


#if SEALS_DRAW_NOT_ENEMY_NAME
            return;
#endif
            name = va(S_COLOR_RED"Enemy:"S_COLOR_WHITE" %s", cgs.clientinfo[ cg.crosshairClientNum ].name );

            if ( !cg.ns_newbiehelp.w_enemySpotted )
            {
                cg.ns_newbiehelp.w_enemySpotted = 1;
                CG_NewbieMessage(S_COLOR_GREEN "You've spotted an enemy.", SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
            }
        }

    }
    else
        return;

    x = rect->x + rect->w/2 - CG_Text_Width( name,scale,0 )/2;

    CG_Text_Paint( x, rect->y, scale, vec_color, name, 0, 0, textStyle);
}

static void CG_DrawWeaponStatus(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    playerState_t	*ps;
    char *mode = "";
    int	weaponmode ;
    int	weapon;

    ps = &cg.snap->ps;
    weaponmode = ps->stats[STAT_WEAPONMODE];
    weapon = ps->weapon;


    if ( ps->weaponstate == WEAPON_BANDAGING_START ||
            ps->weaponstate == WEAPON_BANDAGING_END ||
            ps->weaponstate == WEAPON_BANDAGING )
        mode = "Bandaging";
    else if ( (weapon == WP_M4 || weapon == WP_AK47) && (weaponmode & ( 1 << WM_GRENADELAUNCHER ) ) && ( weaponmode & ( 1 << WM_WEAPONMODE2 ) ) )
    {
        mode = "GrenadeLauncher";
    }
    else if ( (weapon == WP_M4 || weapon == WP_AK47) && (weaponmode & ( 1 << WM_BAYONET ) ) && ( weaponmode & ( 1 << WM_WEAPONMODE2 ) ) )
    {
        mode = "Stab Mode";
    } /*
      else if ( (weapon == WP_PSG1 || weapon == WP_MACMILLAN ) && (( weaponmode & ( 1 << WM_ZOOM4X) )||( weaponmode & ( 1 << WM_ZOOM2X) )) ) // got 4x zoom
      {
      // if i want to switch to
      if ( weaponmode & ( 1 << WM_ZOOM4X) )
      mode = "4x Zoom";
      else if ( weaponmode & (1 << WM_ZOOM2X) )
      mode = "2x Zoom";
      }
      else if ( weaponmode & (1 << WM_SCOPE ) && ( weaponmode & (1 << WM_ZOOM2X) ) ) // scope add-on only2x
      {
      mode = "2x Zoom";
      }*/
    /*
    else if ( weaponmode & (1 << WM_LASER ) && ( weaponmode & (1 << WM_LACTIVE) ) ) // can't have scope + lasersight
    {
    mode = "Laser On";
    }		*/
    else if ( BG_IsGrenade( weapon ) )
    {
        int sec = 3;

        if ( weapon == WP_FLASHBANG )
        {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 2;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 1;
        }
        else if (weapon == WP_SMOKE ) {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 2;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 1;
        } else {
            if ( weaponmode & ( 1 << WM_SINGLE) )
                sec = 4;
            else if ( weaponmode & ( 1 << WM_WEAPONMODE2 ) )
                sec = 5;
        }

        if ( weaponmode & ( 1 << WM_GRENADEROLL ) )
            mode = va( "%is Roll", sec );
        else
            mode = va( "%is Throw", sec );
    }/*
     else if ( weapon == WP_PDW && ( weaponmode & ( 1 << WM_WEAPONMODE2 ) ) )
     {
     mode = "Recoilcatcher";
     } */
    else if ( weapon == WP_M4 || weapon == WP_M249 || weapon == WP_M14 || weapon == WP_MAC10 || weapon == WP_AK47 || weapon == WP_MP5 || weapon == WP_PDW ||
              weapon == WP_SPAS15 )
    {
        // if i want to switch to
        if ( weaponmode & ( 1 << WM_SINGLE) )
            mode = "Single Shot";
        else
            mode = "Full Auto";
    }
    else if ( !BG_IsMelee(weapon) )
        mode = "Single Shot";


    if (shader) {
        trap_R_SetColor( color );
        CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
        trap_R_SetColor( NULL );
    } else {
        int value = CG_Text_Width(mode, scale, 0);
        CG_Text_Paint(rect->x + (rect->w - value) / 2, rect->y + rect->h, scale, color, mode, 0, 0, textStyle);
    }
}

const char *CG_GetStringForTimer( int timer )
{
    switch (timer) {
    case CG_TIMER_WORLD:
        return "ui_roundtimerworld";
    case CG_TIMER_VIP:
        return "ui_viptimer";
    case CG_TIMER_ASSAULT1:
        return "ui_assaulttimer1";
    case CG_TIMER_ASSAULT2:
        return "ui_assaulttimer2";
    case CG_TIMER_ASSAULT3:
        return "ui_assaulttimer3";
    case CG_TIMER_ASSAULT4:
        return "ui_assaulttimer4";
    case CG_TIMER_BOMB:
        return "ui_bombtimer";
    default:
        return "ui_roundtimerworld";
    }
}
const char *CG_GetBlockCvarForTimer( int timer )
{
    switch (timer) {
    case CG_TIMER_ASSAULT1:
        return "ui_assaultblocked1";
    case CG_TIMER_ASSAULT2:
        return "ui_assaultblocked2";
    case CG_TIMER_ASSAULT3:
        return "ui_assaultblocked3";
    case CG_TIMER_ASSAULT4:
        return "ui_assaultblocked4";
    default:
        return "wrong";
    }
}


static void CG_DrawTimerDigit( rectDef_t *rect, int timer ) {
    int			mins, seconds ;
    int			msec;
    int			starttime = cgs.levelStartTime;
    qboolean	assaultfield_taken = qfalse;
    //	qhandle_t	icon;

    switch (timer) {
    case CG_TIMER_WORLD:
        starttime = cgs.levelRoundStartTime;
        //	icon = cgs.media.clockIcon;
        break;
    case CG_TIMER_VIP:
        starttime = cgs.levelVipStartTime;
        //	icon = cgs.media.vipIcon;
        break;
    case CG_TIMER_ASSAULT1:
        starttime = cgs.levelAssaultStartTime[0];
        assaultfield_taken = cgs.assaultFieldsCaptured[0];
        //	icon = cgs.media.assaultIcon;
        break;
    case CG_TIMER_ASSAULT2:
        starttime = cgs.levelAssaultStartTime[1];
        assaultfield_taken = cgs.assaultFieldsCaptured[1];
        //	icon = cgs.media.assaultIcon;
        break;
    case CG_TIMER_ASSAULT3:
        starttime = cgs.levelAssaultStartTime[2];
        assaultfield_taken = cgs.assaultFieldsCaptured[2];
        //	icon = cgs.media.assaultIcon;
        break;
    case CG_TIMER_ASSAULT4:
        starttime = cgs.levelAssaultStartTime[3];
        assaultfield_taken = cgs.assaultFieldsCaptured[3];
        //	icon = cgs.media.assaultIcon;
        break;
    case CG_TIMER_BOMB:
        starttime = cgs.levelBombStartTime;
        //	icon = cgs.media.bombIcon;
        break;
    default:
        starttime = cgs.levelStartTime;
        //	icon = cgs.media.clockIcon;
        break;
    }

    // this field has been deactivated.
    if ( assaultfield_taken )
    {
        char buf[128];

        trap_Cvar_VariableStringBuffer( CG_GetBlockCvarForTimer( timer), buf, sizeof(buf ) );

        //		CG_Printf("timer %s is %s\n", CG_GetBlockCvarForTimer(timer),buf );
        if ( atoi(buf) != 1 )
            trap_Cvar_Set( CG_GetBlockCvarForTimer( timer ) , "1" );

        trap_Cvar_VariableStringBuffer( CG_GetStringForTimer( timer), buf, sizeof(buf ) );

        //		CG_Printf("timer %s is %s\n", CG_GetStringForTimer(timer),buf );
        if ( atoi(buf) != 1 )
            trap_Cvar_Set( CG_GetStringForTimer( timer ) , "2" );

        return;
    }

    msec = ( starttime + 1000 ) - cg.time;

    // show server start time - if we're not in LTS
    if ( timer == CG_TIMER_WORLD && cgs.gametype < GT_LTS )
        msec = cg.time - cgs.levelStartTime;

    seconds = msec / 1000;
    mins = seconds / 60;
    seconds -= mins * 60;

    if ( seconds < 0 )
        seconds = 0;
    if ( mins < 0 )
        mins = 0;



    // timer is empty... render?
    if ( !seconds && !mins && timer != CG_TIMER_WORLD )
    {
        char buf[128];

        trap_Cvar_VariableStringBuffer( CG_GetStringForTimer( timer), buf, sizeof(buf ) );

        if ( atoi(buf) != 0 )
            trap_Cvar_Set( CG_GetStringForTimer( timer ) , "0" );

        return;
    }
    else
    { // activate timer. set the corresponding cvar to 1 so the ui can draw it
        char buf[128];

        trap_Cvar_VariableStringBuffer( CG_GetStringForTimer( timer), buf, sizeof(buf ) );

        if ( atoi(buf) != 1 )
            trap_Cvar_Set( CG_GetStringForTimer( timer ) , "1" );
    }
    CG_DrawDigits2( rect->x, rect->y, 1, mins );
    CG_DrawPic( rect->x + 22, rect->y, 16,32 , cgs.media.colonIcon );
    CG_DrawDigits2( rect->x + 40 , rect->y, 2, seconds );
    //	CG_DrawPic( rect->x + 88, rect->y, 32,32, icon );

}

//
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle) {
    rectDef_t rect;


    if ( cg_drawStatus.integer == 0 ) {
        return;
    }

    //if (ownerDrawFlags != 0 && !CG_OwnerDrawVisible(ownerDrawFlags)) {
    //	return;
    //}

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;


    if (ownerDraw == CG_PRIMARY || ownerDraw == CG_SECONDARY )
    {
        char		var[MAX_TOKEN_CHARS];

        if (ownerDraw == CG_PRIMARY)
            trap_Cvar_VariableStringBuffer("inven_primary", var , sizeof( var ) );
        else
            trap_Cvar_VariableStringBuffer("inven_secondary", var , sizeof( var ) );

        // set the "special to our current weapon"
        special = atoi(var);
    }

    switch (ownerDraw) {
    case CG_TIMER_WORLD:
    case CG_TIMER_VIP:
    case CG_TIMER_ASSAULT1:
    case CG_TIMER_ASSAULT2:
    case CG_TIMER_ASSAULT3:
    case CG_TIMER_ASSAULT4:
    case CG_TIMER_BOMB:
        CG_DrawTimerDigit( &rect, ownerDraw);
        break;
    case CG_PLAYER_WEAPONSTATUS:
        CG_DrawWeaponStatus( &rect, scale, color, shader, textStyle );
        break;
    case CG_PLAYER_ENEMY:
        CG_DrawCrosshairEntity( &rect, scale, color, shader, textStyle );
        break;
    case CG_PLAYER_CLIPS_ICON:
        CG_DrawClipIcon( &rect );
        break;
    case CG_PLAYER_CLIPS_VALUE:
        CG_DrawClipDigitValue( &rect );
        break;
    case CG_PLAYER_STAMINABAR_VERT:
        CG_DrawStaminaBarVert(&rect, color, shader );
        break;
    case CG_PLAYER_STAMINABAR_HOR:
        CG_DrawStaminaBarHor( &rect, color, shader );
        break;
    case CG_PLAYER_AMMOBAR_VERT:
        CG_DrawAmmoBarVert( &rect, color, shader );
        break;
    case CG_PLAYER_AMMOBAR_HOR:
        CG_DrawAmmoBarHor(&rect, color, shader );
        break;
    case CG_PLAYER_DAMAGELOC:
        CG_DrawDamageLoc( &rect, special );
        break;
    case CG_PLAYER_CLIPVALUE:
        CG_DrawClipValue( &rect );
        break;
    case CG_PLAYER_CLIPGFX:
        CG_DrawClipGfx( &rect );
        break;
    case CG_PRIMARY:
    case CG_SECONDARY:
    case CG_PICKUP:
        CG_DrawWeaponIcon( &rect, special );
        break;

    case CG_PLAYER_ROUNDS_ICON:
        CG_DrawPlayerArmorIcon(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
        break;
    case CG_PLAYER_ARMOR_ICON2D:
        CG_DrawPlayerArmorIcon(&rect, qtrue);
        break;
    case CG_PLAYER_ROUNDS_VALUE:
        CG_DrawPlayerArmorValue(&rect, scale, color, shader, textStyle);
        break;
    case CG_PLAYER_AMMO_VALUE:
        CG_DrawPlayerAmmoValue(&rect, scale, color, shader, textStyle);
        break;
    case CG_PLAYER_HEAD:
        CG_DrawPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
        break;
    case CG_PLAYER_ITEM: 
        break;
    case CG_PLAYER_SCORE:
        CG_DrawPlayerScore(&rect, scale, color, shader, textStyle);
        break;
    case CG_PLAYER_HEALTH:
        CG_DrawPlayerHealth(&rect, scale, color, shader, textStyle);
        break;
    case CG_RED_SCORE:
        CG_DrawRedScore(&rect, scale, color, shader, textStyle);
        break;
    case CG_BLUE_SCORE:
        CG_DrawBlueScore(&rect, scale, color, shader, textStyle);
        break;
    case CG_PLAYER_LOCATION:
        CG_DrawPlayerLocation(&rect, scale, color, textStyle);
        break;
    case CG_TEAM_COLOR:
        CG_DrawTeamColor(&rect, color);
        break;
    case CG_AREA_POWERUP:
        CG_DrawAreaPowerUp(&rect, align, special, scale, color);
        break;
#ifdef IS_REDUNDANT
    case CG_PLAYER_STATUS:
        CG_DrawPlayerStatus(&rect);
        break;
#endif
    case CG_AREA_SYSTEMCHAT:
        CG_DrawAreaSystemChat(&rect, scale, color, shader);
        break;
    case CG_AREA_TEAMCHAT:
        CG_DrawAreaTeamChat(&rect, scale, color, shader);
        break;
    case CG_AREA_CHAT:
        CG_DrawAreaChat(&rect, scale, color, shader);
        break;
    case CG_GAME_TYPE:
        CG_DrawGameType(&rect, scale, color, shader, textStyle);
        break;
    case CG_GAME_STATUS:
        CG_DrawGameStatus(&rect, scale, color, shader, textStyle);
        break;
    case CG_KILLER:
        CG_DrawKiller(&rect, scale, color, shader, textStyle);
        break;
    case CG_SPECTATORS:
        CG_DrawTeamSpectators(&rect, scale, color, shader);
        break;
    case CG_CAPFRAGLIMIT:
        CG_DrawCapFragLimit(&rect, scale, color, shader, textStyle);
        break;
    case CG_1STPLACE:
        CG_Draw1stPlace(&rect, scale, color, shader, textStyle);
        break;
    case CG_2NDPLACE:
        CG_Draw2ndPlace(&rect, scale, color, shader, textStyle);
        break;
    default:
        break;
    }
}

void CG_MouseEvent(int x, int y) {
    int n;
    /*
    if ( (cg.predictedPlayerState.pm_type == PM_NORMAL || cg.predictedPlayerState.pm_type == PM_SPECTATOR) && cg.showScores == qfalse) {
    trap_Key_SetCatcher(0);
    return;
    }
    */
    cgs.cursorX+= x;
    if (cgs.cursorX < 0)
        cgs.cursorX = 0;
    else if (cgs.cursorX > 640)
        cgs.cursorX = 640;

    cgs.cursorY += y;
    if (cgs.cursorY < 0)
        cgs.cursorY = 0;
    else if (cgs.cursorY > 480)
        cgs.cursorY = 480;

    n = Display_CursorType(cgs.cursorX, cgs.cursorY);
    cgs.activeCursor = 0;
    if (n == CURSOR_ARROW) {
        cgs.activeCursor = cgs.media.selectCursor;
    } else if (n == CURSOR_SIZER) {
        cgs.activeCursor = cgs.media.sizeCursor;
    }

    if (cgs.capturedItem) {
        Display_MouseMove(cgs.capturedItem, x, y);
    } else {
        Display_MouseMove(NULL, cgs.cursorX, cgs.cursorY);
    }

}

/*
==================
CG_HideRadioMenus
==================

*/
void CG_HideRadioMenu() {
    Menus_CloseByName("ingame_radio");
}

/*
==================
CG_ShowRadioMenus
==================

*/
void CG_ShowRadioMenus() {
    Menus_OpenByName("ingame_radio");
}




/*
==================
CG_EventHandling
==================
type 0 - no event handling
1 - team menu
2 - hud editor

*/
void CG_EventHandling(int type) {
    cgs.eventHandling = type;
    if (type == CGAME_EVENT_NONE) {
        CG_HideRadioMenu();
    } else if (type == CGAME_EVENT_RADIOMENU) {
        CG_ShowRadioMenus();
    } else if (type == CGAME_EVENT_SCOREBOARD) {
    }

}

void CG_EditRadar_HandleKey ( int key );
extern	vmCvar_t cg_hudStyle;
extern	vmCvar_t	cg_hud1PosX;
extern	vmCvar_t	cg_hud1PosY;
extern	vmCvar_t	cg_hud2PosX;
extern	vmCvar_t	cg_hud2PosY;

void CG_KeyEvent(int key, qboolean down) {

    if (!down) {
        return;
    }

    if ( cgs.eventHandling == CGAME_EVENT_EDITHUD )
    {


        if ( down )
        {
            if ( key == K_MOUSE1 )
            {
                int x = cgs.cursorX * ( 1024.0f/640 );
                int y = cgs.cursorY * ( 768.0f/480 );

                if ( cg_hudStyle.integer == 1 )
                {
                    trap_Cvar_Set( "cg_hud1PosX", va("%i", x ) );
                    trap_Cvar_Set( "cg_hud1PosY", va("%i", y ) );
                }
                else if ( cg_hudStyle.integer == 2 )
                {
                    trap_Cvar_Set( "cg_hud2PosX", va("%i", x ) );
                    trap_Cvar_Set( "cg_hud2PosY", va("%i", y ) );
                }
            }
            else if ( key == K_MOUSE2 )
            {
                CG_EventHandling(CGAME_EVENT_NONE);
                trap_Key_SetCatcher(0);
            }
        }
    }
    if ( cgs.eventHandling == CGAME_EVENT_EDITRADARPOS )
    {

        if ( down )
        {
            if ( key == K_MOUSE1 )
            {
                trap_Cvar_Set( "cg_radarX", va("%i",cgs.cursorX ) );
                trap_Cvar_Set( "cg_radarY", va("%i",cgs.cursorY ) );
            }
            else if ( key == K_MOUSE2 )
            {
                CG_EventHandling(CGAME_EVENT_NONE);
                trap_Key_SetCatcher(0);
            }
        }
    }

    if ( cgs.eventHandling == CGAME_EVENT_EDITRADAR && key == K_MOUSE1 && down )
    {
        CG_EditRadar_HandleKey( key );
        return;
    }
    if ( cgs.eventHandling == CGAME_EVENT_EDITQCMD && key == K_MOUSE1 )
    {
        cg_qcmd_posx.integer = cgs.cursorX;
        cg_qcmd_posy.integer = cgs.cursorY;
        CG_Printf("Set X^2%i^7 Y^2%i^7 as the Top-Left Corner for the Q-Command Menu.\n", cg_qcmd_posx.integer , cg_qcmd_posy.integer );
        CG_EventHandling(CGAME_EVENT_NONE);
        trap_Key_SetCatcher(0);
        trap_Cvar_Set( "cg_qcmd_posx", va("%i",cg_qcmd_posx.integer ) );
        trap_Cvar_Set( "cg_qcmd_posy", va("%i",cg_qcmd_posy.integer ) );
        return;
    }
    /*
    if ( cg.predictedPlayerState.pm_type == PM_NORMAL || (cg.predictedPlayerState.pm_type == PM_SPECTATOR && cg.showScores == qfalse)) {
    CG_EventHandling(CGAME_EVENT_NONE);
    trap_Key_SetCatcher(0);
    return;
    }
    */
    //if (key == trap_Key_GetKey("teamMenu") || !Display_CaptureItem(cgs.cursorX, cgs.cursorY)) {
    // if we see this then we should always be visible
    //  CG_EventHandling(CGAME_EVENT_NONE);
    //  trap_Key_SetCatcher(0);
    //}



    //	Display_HandleKey(key, down, cgs.cursorX, cgs.cursorY);

    if (Menu_Count() > 0) {
        menuDef_t *menu = Menu_GetFocused();
        if (menu) {
            if (key == K_ESCAPE && down && !Menus_AnyFullScreenVisible()) {
                Menus_CloseAll();
                CG_EventHandling(CGAME_EVENT_NONE);
                trap_Key_SetCatcher(0);
            } else {
                Menu_HandleKey(menu, key, down );
            }
        } else {
            // dumm....
        }
    }
    else
    {
        CG_EventHandling(CGAME_EVENT_NONE);
        trap_Key_SetCatcher(0);
    }

    if (cgs.capturedItem) {
        cgs.capturedItem = NULL;
    }	else {
        if (key == K_MOUSE2 && down) {
        cgs.capturedItem = Display_CaptureItem(cgs.cursorX, cgs.cursorY);
        }
    }
}

int CG_ClientNumFromName(const char *p) {
    int i;
    for (i = 0; i < cgs.maxclients; i++) {
        if (cgs.clientinfo[i].infoValid && Q_stricmp(cgs.clientinfo[i].name, p) == 0) {
            return i;
        }
    }
    return -1;
}

void CG_GetTeamColor(vec4_t *color) {
    if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
        (*color)[0] = 1;
        (*color)[3] = .25f;
        (*color)[1] = (*color)[2] = 0;
    } else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
        (*color)[0] = (*color)[1] = 0;
        (*color)[2] = 1;
        (*color)[3] = .25f;
    } else {
        (*color)[0] = (*color)[2] = 0;
        (*color)[1] = .17f;
        (*color)[3] = .25f;
    }
}

