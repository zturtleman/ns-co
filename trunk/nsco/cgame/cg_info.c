// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_info.c -- display information while data is being loading

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "cg_local.h"

#include "../ui/ui_shared.h"

/*
======================
CG_LoadingBarInit

======================
*/
void CG_LoadBarInit( void )
{
    cg.loadingbarState = 0;
    cg.loadingbarMax = 0;
}
/*
======================
CG_LoadingBarUpdate

======================
*/

void CG_LoadingBarUpdate( int amount )
{
    cg.loadingbarState += amount;
    //	CG_Printf("loadingbarState : %i\n",cg.loadingbarState );
    trap_UpdateScreen();
}
/*
======================
CG_LoadingBarSetMax

======================
*/
void CG_LoadingBarSetMax( int maximum )
{
    cg.loadingbarMax = maximum;
}
/*
======================
CG_DrawLoadingBar

======================
*/
void CG_DrawLoadingBar( int x, int y, int w, int h )
{
    float real_width = 0;
    char *text;
    vec4_t color;
	qhandle_t	hShader = trap_R_RegisterShader( "ui/assets/loadingbar.tga" );

    real_width = (float)( (float)cg.loadingbarState / (float)cg.loadingbarMax );

    color[0] = color[1] = color[2] = real_width;
    color[3] = 0.8f;

    if ( real_width > 1.0f )
        real_width = 1.0f;

    real_width = real_width * (float)w;


    CG_DrawPic( x, y, real_width, h, hShader );

    // defcon: only show the awaiting for snapshot thingy.
    if ( cg.infoScreenText[0] )
        text = va("Precaching : %s ", cg.infoScreenText );
    else
    {
        text = "Waiting for Snapshot...";
        x = x + ( ( w/2 ) - ( CG_Text_Width( text,0.25f,0)/2 ) );
        CG_Text_Paint( x,y+11, 0.25f, colorWhite, text, 0,0,ITEM_TEXTSTYLE_SHADOWED);
    }
}

/*
======================
CG_LoadingString

======================
*/
void CG_LoadingString( const char *s ) {
    Q_strncpyz( cg.infoScreenText, s, sizeof( cg.infoScreenText ) );

    trap_UpdateScreen();
}

/*
===================
CG_LoadingItem
===================
*/
void CG_LoadingItem( int itemNum ) {
    gitem_t		*item;

    item = &bg_itemlist[itemNum];

    CG_LoadingString( item->pickup_name );
}

/*
===================
CG_LoadingClient
===================
*/
void CG_LoadingClient( int clientNum ) {
    const char		*info;
    char			personality[MAX_QPATH];

    info = CG_ConfigString( CS_PLAYERS + clientNum );

    Q_strncpyz( personality, Info_ValueForKey( info, "n" ), sizeof(personality) );
    Q_CleanStr( personality );

    /*
    if( cgs.gametype == GT_TRAINING ) {
    trap_S_RegisterSound( va( "sound/player/announce/%s.wav", personality ), qtrue );
    }
    */
    CG_LoadingString( personality );
}

qboolean	CG_ParseHelpFile( void ) {
    char		*text_p;
    int			len;
    //	int			lines = 0;
    char		*token;
    //	float		fps;
    int			skip;
    char		text[20000];
    char		filename[128];
    fileHandle_t	f;

    Com_sprintf(filename, sizeof( filename ), "scripts/script_help.cfg" );

    //
    // set defaults
    //
    cgs.helpNumGameMessages = 0;
    cgs.helpNumMissionMessages = 0;

    memset( &cgs.helpGameMessages , 0, sizeof( cgs.helpGameMessages ) );
    memset( &cgs.helpMissionMessages , 0, sizeof( cgs.helpMissionMessages ) );

    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( len <= 0 ) {
        return qfalse;
    }
    if ( len >= sizeof( text ) - 1 ) {
        CG_Printf( "File %s (%i>%i)too long\n", text, len, sizeof( text) );
        return qfalse;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;
    skip = 0;	// quite the compiler warning


    // parse
    while ( 1 ) {

        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token ) {
            break;
        }

        // reached end of file
        if ( !Q_stricmp( token, "$EOF" ) )
            break;

        // start parsing seals part
        if ( !Q_stricmp( token, "[gameplay]" ) && cgs.helpNumGameMessages <= 0)
        {
            while ( 1 )
            {
                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                if ( !Q_stricmp( token , "[end]" ) )
                    break;

                if ( !Q_stricmp( token , "{" ) )
                {
                    while ( 1 )
                    {
                        // get new token
                        token = COM_Parse( &text_p );

                        if ( !token )
                        {
                            CG_Error("Unexpected end of file (file:%s)\n", filename );
                            break;
                        }

                        if ( token[0] == '@' || ( token[0] =='\\' && token[1] == 'n' ) )
                        {
                            strcat(  cgs.helpGameMessages[ cgs.helpNumGameMessages ], "\n");
                            continue;
                        }
                        if ( token[0] == '}' ) {
                            //							CG_Printf("Parsed (help line %i) : %s\n", cgs.helpNumGameMessages, cgs.helpGameMessages[cgs.helpNumGameMessages] );

                            cgs.helpNumGameMessages++;
                            if ( cgs.helpNumGameMessages >= MAX_HELP_LINES )
                                return qtrue;
                            break;
                        }
                        else {
                            char temp[MAX_CHARS_PER_LINE];

                            Com_sprintf( temp, sizeof( temp ), "%s %s", cgs.helpGameMessages[ cgs.helpNumGameMessages ],  token );

                            strcpy( cgs.helpGameMessages[ cgs.helpNumGameMessages ], temp );
                        }
                    }
                }
            }
        }

        if (
            ( !Q_stricmp( token, "[vip-r]" ) && cgs.mi_vipRescue ) ||
            ( !Q_stricmp( token, "[vip-t]" ) && cgs.mi_vipTime ) ||
            ( !Q_stricmp( token, "[bc]" ) && cgs.mi_sealBriefcase ) ||
            ( !Q_stricmp( token, "[bomb]" ) && cgs.mi_bombSpot ) ||
            ( !Q_stricmp( token, "[assault]" ) && cgs.mi_assaultFields )
        )
        {
            while ( 1 )
            {
                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                if ( !Q_stricmp( token , "[end]" ) )
                    break;

                if ( !Q_stricmp( token , "{" ) )
                {
                    while ( 1 )
                    {
                        // get new token
                        token = COM_Parse( &text_p );

                        if ( !token )
                        {
                            CG_Error("Unexpected end of file (file:%s)\n", filename );
                            break;
                        }

                        if ( token[0] == '@' || ( token[0] =='\\' && token[1] == 'n' ) )
                        {
                            strcat(  cgs.helpMissionMessages[ cgs.helpNumMissionMessages ], "\n");
                            continue;
                        }
                        if ( token[0] == '}' ) {
                            //							CG_Printf("Parsed (gamehelp line %i) : %s\n", cgs.helpNumMissionMessages, cgs.helpMissionMessages[cgs.helpNumMissionMessages] );

                            cgs.helpNumMissionMessages++;
                            if ( cgs.helpNumMissionMessages >= MAX_HELP_LINES )
                                return qtrue;
                            break;
                        }
                        else {
                            char temp[MAX_CHARS_PER_LINE];

                            Com_sprintf( temp, sizeof( temp ), "%s %s", cgs.helpMissionMessages[ cgs.helpNumMissionMessages ],  token );

                            strcpy( cgs.helpMissionMessages[ cgs.helpNumMissionMessages ], temp );
                        }
                    }
                }
            }
        }
    }

    return qtrue;
}

void CG_HandleHelp( void )
{
    float	rnd = random();
    int		numHelp; // which message to be displayed
    int		count = MAX_HELP_LINES;	// so we don't get stuck in a loop
    qboolean mission = qfalse;	 // display a mission related help message?

    if ( cgs.helpLastMessageTime > cg.time ||
            cgs.helpNumGameMessages <= 0 ||
            cg_newbeeTime.value <= 0 ||
            cg.predictedPlayerState.pm_type != PM_NORMAL )
        return;

    if ( cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_RED &&
            cg.predictedPlayerState.persistant[PERS_TEAM] != TEAM_BLUE )
        return;

    if ( rnd < 0.5 && cgs.helpNumMissionMessages > 0 )
        mission = qtrue;

    if ( mission )
    {
        numHelp = random() * ( cgs.helpNumMissionMessages-1 );
    }
    else
    {
        numHelp = random() * ( cgs.helpNumGameMessages-1 );
    }

    if ( mission )
    {
        count = cgs.helpNumMissionMessages*2;

        while ( cgs.helpNumDisplayedMissionMessage[ numHelp ] && count )
        {
            numHelp++;
            count--;

            if( numHelp >= cgs.helpNumMissionMessages  )
                numHelp = 0;
        }
        if ( count == 0 ) // already disaplyed everything
            cgs.helpNumMissionMessages = 0;
    }
    else
    {
        count = cgs.helpNumGameMessages*2;

        while ( cgs.helpNumDisplayedGameMessage[ numHelp ] && count )
        {
            numHelp++;
            count--;

            if( numHelp >= cgs.helpNumGameMessages )
                numHelp = 0;
        }
        if ( count == 0 ) // we have to reset it, since all messages seem to be displayed.
            cgs.helpNumGameMessages = 0;
    }

    /*	CG_Printf("Message: %s curnum: %i maxnum: %i mission: %i\n",cgs.helpMissionMessages[ numHelp ]
    ,numHelp, cgs.helpNumGameMessages, mission );*/
    if ( mission )
    {
        cgs.helpNumDisplayedMissionMessage[ numHelp ] = qtrue;
        CG_NewbieMessage( va("%s%s",S_COLOR_GREEN, cgs.helpMissionMessages[ numHelp ] ) , SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
    }
    else
    {
        cgs.helpNumDisplayedGameMessage[ numHelp ] = qtrue;
        CG_NewbieMessage( va("%s%s",S_COLOR_GREEN, cgs.helpGameMessages[ numHelp ] ) , SCREEN_HEIGHT * 0.60, cg_newbeeHeight.value );
    }

    cgs.helpLastMessageTime = cg.time + (10000 + random()*5000 );
}
/*
======================
CG_ParseBriefingFile

Reads a briefing file containing information about the map that is beeing loaded
briefing/map_%s.brf,
======================
*/
#define MAX_BRIEFING_LINES 128
#define	LINEBREAK_WIDTH		42 
char SealBriefing[ MAX_BRIEFING_LINES ][ MAX_CHARS_PER_LINE ]; // max chars per line
int	sealBriefingLines = 0;

char TangoBriefing[ MAX_BRIEFING_LINES ][ MAX_CHARS_PER_LINE ]; // max chars per line
int	tangoBriefingLines = 0;

int CG_GetCamoTypeForString( char *camostring ) {
    // return default
    if (!camostring)
        return 0;

    if ( !Q_stricmp( camostring , "urban" ) )
        return CAMO_URBAN;
    else if ( !Q_stricmp( camostring , "arctic" ) )
        return CAMO_ARCTIC;
    else if ( !Q_stricmp( camostring , "jungle" ) )
        return CAMO_JUNGLE;
    else if ( !Q_stricmp( camostring , "desert" ) )
        return CAMO_DESERT;
    else
        return -1;
}

char *CG_GetCamoStringForType( int camoType ) {
    // return default
    if (!camoType)
        return "";

    if ( camoType == CAMO_URBAN )
        return "_urban";
    else if ( camoType == CAMO_ARCTIC )
        return "_arctic";
    else if ( camoType == CAMO_JUNGLE )
        return "_jungle";
    else if ( camoType == CAMO_DESERT )
        return "_desert";

    return "";
}

qboolean	CG_ParseBriefingFile(  char *mapstring  ) {
    char		*text_p;
    int			len;
    //	int			lines = 0;
    char		*token;
    //	float		fps;
    int			skip;
    char		text[20000];
    char		filename[128];
    fileHandle_t	f;

    sealBriefingLines = tangoBriefingLines = 0;
    Com_sprintf(filename, sizeof( filename), "briefing/map_%s.brf", mapstring );

    //
    // set defaults
    //
    cgs.camoType = CAMO_URBAN;
    strcpy( cgs.vipType, "arab" );

    cgs.mi_assaultFields = 0;
    cgs.mi_bombSpot = qfalse;
    cgs.mi_vipRescue = qfalse;
    cgs.mi_vipTime = qfalse;
    cgs.mi_sealBriefcase = qfalse;

    cg.mapOriginX = cg.mapOriginY = 256;

    cg.radarNumObjects = 0;
    memset( &cg.radarObjects, sizeof(cg.radarObjects), 0 );

    memset( &cgs.mi_helpers , 0, sizeof( cgs.mi_helpers ) );

    // load the file
    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( len <= 0 ) {
        return qfalse;
    }
    if ( len >= sizeof( text ) - 1 ) {
        CG_Printf( "File %s (%i>%i)too long\n", text, len, sizeof( text) );
        return qfalse;
    }
    trap_FS_Read( text, len, f );
    text[len] = 0;
    trap_FS_FCloseFile( f );

    // parse the text
    text_p = text;
    skip = 0;	// quite the compiler warning

    // parse
    while ( 1 ) {

        token = COM_Parse( &text_p );

        // no token? stop parsing...
        if ( !token ) {
            break;
        }

        // reached end of file
        if ( !Q_stricmp( token, "$EOF" ) )
            break;

        if ( !Q_stricmp( token, "[radar]" ) )
        {
            cg.radarNumObjects = 0;
            memset( &cg.radarObjects, sizeof(cg.radarObjects), 0 );

            while ( 1 )
            {

                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                if ( !Q_stricmp( token , "[end]" ) )
                    break;

                cg.radarObjects[cg.radarNumObjects].type = token[0];

                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                cg.radarObjects[cg.radarNumObjects].origin[0] = atof( token );
                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                cg.radarObjects[cg.radarNumObjects].origin[1] = atof( token );
                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                cg.radarObjects[cg.radarNumObjects].origin[2] = atof( token );
                cg.radarNumObjects++;
            }
            continue;
        }

        if ( !Q_stricmp( token, "[mapspecs]" ) )
        {
            while ( 1 )
            {

                // get new token
                token = COM_Parse( &text_p );


                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                if ( !Q_stricmp( token , "[end]" ) )
                    break;

                if ( !Q_stricmp( token , "missioninfo" ) )
                {

                    // get new token
                    token = COM_Parse( &text_p ); // parse out {

                    if ( !Q_stricmp( token, "{" ) )
                    {
                        while ( 1 )
                        {

                            // get new token
                            token = COM_Parse( &text_p );

                            if ( !token )
                            {
                                CG_Error("Unexpected end of file (file:%s)\n", filename );
                                break;
                            }

                            // end parsing
                            if ( !Q_stricmp( token , "}" ) )
                                break;

                            if ( !Q_stricmp( token, "assaultfield" ) )
                            {
                                cgs.mi_assaultFields++;

                                if( cgs.mi_assaultFields > 4 )
                                    CG_Error("num assaultfields > MAX_ASSAULTFIELDS\n");
                                continue;
                            }
                            else if ( !Q_stricmp( token, "viprescue" ) )
                            {
                                cgs.mi_vipRescue = qtrue;

                                if( cgs.mi_vipTime )
                                    CG_Error("VIP type not allowed ( viptime viprescue )\n");
                                continue;
                            }
                            else if ( !Q_stricmp( token, "viptime" ) )
                            {
                                cgs.mi_vipTime = qtrue;

                                if( cgs.mi_vipRescue )
                                    CG_Error("VIP type not allowed ( viprescue viptime )\n");
                                continue;
                            }
                            else if ( !Q_stricmp( token, "blowup" ) )
                            {
                                if( cgs.mi_bombSpot )
                                    CG_Error("Bombmode already set ( blowup blowup )\n");

                                cgs.mi_bombSpot = qtrue;
                                continue;
                            }
                            else if ( !Q_stricmp( token, "bc" ) )
                            {
                                if( cgs.mi_sealBriefcase )
                                    CG_Error("SealBriefcase already set ( bc bc )\n");

                                cgs.mi_sealBriefcase = qtrue;
                                continue;
                            }
                        }
                    }
                }
                if ( !Q_stricmp( token , "camouflage" ) )
                {
                    token = COM_Parse( &text_p );

                    if (!token)
                    {
                        CG_Error("No Camouflage type given(file:%s)\n", filename );
                        break;
                    }

                    // parse camotype
                    cgs.camoType = CG_GetCamoTypeForString( token );

                    if ( cgs.camoType < 0 ) {
                        CG_Error("Illegal Camo type : %s", token );
                        break;
                    }
                    //					CG_Printf("Parsed camo type: %s\n", token );
                }
                if ( !Q_stricmp( token , "viptype" ) )
                {
                    token = COM_Parse( &text_p );

                    if (!token)
                    {
                        CG_Error("No Camouflage type given(file:%s)\n", filename );
                        break;
                    }

                    // parse camotype
                    strcpy( cgs.vipType, token );

                    if ( strlen( cgs.vipType) <= 0 ) {
                        CG_Error("Illegal Camo type : %s", token );
                        break;
                    }
                    //					CG_Printf("Parsed VIP type: %s\n", token );
                }
                if ( !Q_stricmp( token , "maporigin" ) )
                {
                    token = COM_Parse( &text_p );

                    if (!token)
                    {
                        CG_Error("not enough parameters for maporigin token(file:%s)\n", filename );
                        break;
                    }

                    // parse camotype
                    cg.mapOriginX = atoi( token );

                    token = COM_Parse( &text_p );

                    if (!token)
                    {
                        CG_Error("not enough parameters for maporigin token(file:%s)\n", filename );
                        break;
                    }

                    // parse camotype
                    cg.mapOriginY = atoi( token );

                    //					CG_Printf("Parsed Map Origin: %i %i\n", cg.mapOriginX , cg.mapOriginY );
                }

                if ( !Q_stricmp( token , "$enviroment" ) )
                {
                    token = COM_Parse( &text_p );

                    if (!token)
                    {
                        CG_Error("Found '$enviroment' string but no following string");
                        break;
                    }

                    if ( !Q_stricmp( token, "{" ) )
                    {
                        CG_EnviromentParse( text_p );
                    }
                }
            }
        }
        // start parsing seals part
        if ( !Q_stricmp( token, "[seals]" ) && sealBriefingLines <= 0)
        {
            while ( 1 )
            {
                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                if ( !Q_stricmp( token , "[end]" ) )
                    break;

                if ( !Q_stricmp( token , "$EOL" ) ) {
                    //					CG_Printf("Parsed (S line %i) : %s\n", sealBriefingLines, SealBriefing[ sealBriefingLines ] );
                    sealBriefingLines++;
                }
                else {
                    // create automated linebreak
                    if ( strlen ( SealBriefing[ sealBriefingLines ] ) + strlen ( token ) > LINEBREAK_WIDTH )
                        sealBriefingLines++;

                    if ( strlen( SealBriefing[ sealBriefingLines ] ) <= 0 )
                        strcat( SealBriefing[ sealBriefingLines ], token );
                    else
                        strcat( SealBriefing[ sealBriefingLines ], va(" %s",token ) );
                }

            }
        }
        if ( !Q_stricmp( token, "[tangos]" ) && tangoBriefingLines <= 0)
        {
            while ( 1 )
            {

                // get new token
                token = COM_Parse( &text_p );

                if ( !token )
                {
                    CG_Error("Unexpected end of file (file:%s)\n", filename );
                    break;
                }

                if ( !Q_stricmp( token , "[end]" ) )
                    break;

                if ( !Q_stricmp( token , "$EOL" ) ) {
                    //					CG_Printf("Parsed (T line %i) : %s\n", tangoBriefingLines, TangoBriefing[ tangoBriefingLines ] );
                    tangoBriefingLines++;
                }
                else {
                    // create automated linebreak
                    if ( strlen ( TangoBriefing[ tangoBriefingLines ] ) + strlen ( token ) > LINEBREAK_WIDTH )
                        tangoBriefingLines++;

                    if ( strlen( TangoBriefing[ tangoBriefingLines ] ) <= 0 )
                        strcat( TangoBriefing[ tangoBriefingLines ], token );
                    else
                        strcat( TangoBriefing[ tangoBriefingLines ], va(" %s",token ) );
                }

            }
        }
    }

    return qtrue;
}

/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/

#define INFO_STRING_START	15
#define INFO_STRING_DIST	12

void CG_DrawInformation( void ) {
    const char	*s;
    const char	*info;
    const char	*sysInfo;
    int			x,y;
    int			value;
    qhandle_t	levelshot;
    qhandle_t	loadingscreen;
    qhandle_t	detail;
    char		buf[1024];
	float		textscale = 0.18f;

    info = CG_ConfigString( CS_SERVERINFO );
    sysInfo = CG_ConfigString( CS_SYSTEMINFO );
 
    s = Info_ValueForKey( info, "mapname" );
    levelshot = trap_R_RegisterShader( va( "levelshots/%s.tga", s ) );
    if ( !levelshot ) {
        levelshot = trap_R_RegisterShader( "gfx/2d/unknownmap" );
    } 
	loadingscreen = trap_R_RegisterShaderNoMip( "gfx/2d/ns_briefing.tga" );

    trap_R_SetColor( NULL );

    // draw loadingscreen
    CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, loadingscreen );
 
    // draw levelshot
    CG_DrawPic( 76, 96, 241, 124, levelshot );

    // blend a detail texture over it
    detail = trap_R_RegisterShader( "levelShotDetail" );
    CG_DrawPic( 76, 96, 241, 124, detail );

	// darw the loading bar
    CG_DrawLoadingBar( 76, 432, 492, 17 );
#if 1
    // draw info string information
    x = 327;
	y = 106;
	textscale = 0.18f;

    // don't print server lines if playing a local game
    trap_Cvar_VariableStringBuffer( "sv_running", buf, sizeof( buf ) );
    if ( !atoi( buf ) || 1) {
        // server hostname
        Q_strncpyz(buf, Info_ValueForKey( info, "sv_hostname" ), 1024);
        Q_CleanStr(buf);
        CG_Text_Paint( x,y, textscale, colorWhite, buf, 0,0, ITEM_TEXTSTYLE_SHADOWED); 
        y += CG_Text_Height( "A", textscale, 0 ) + 3;

        // pure server
        s = Info_ValueForKey( sysInfo, "sv_pure" );
        if ( s[0] == '1' ) {
            CG_Text_Paint( x,y, textscale, colorWhite, "Pure Server", 0,0, ITEM_TEXTSTYLE_SHADOWED); 
            y += CG_Text_Height( "A", textscale, 0 ) + 3;
        }

        // server-specific message of the day
        s = CG_ConfigString( CS_MOTD );
        if ( s[0] ) {
            CG_Text_Paint( x,y, textscale, colorWhite,s, 0,0, ITEM_TEXTSTYLE_SHADOWED); 
            y += CG_Text_Height( "A", textscale, 0 ) + 3;
        }

        // some extra space after hostname and motd
        y += ( CG_Text_Height( "A", textscale, 0 ) + 3 ) * 2;
    }

    // map-specific message (long map name)
    s = CG_ConfigString( CS_MESSAGE );
    if ( s[0] || 1 ) {
        CG_Text_Paint( x,y, textscale, colorWhite, s, 0,0, ITEM_TEXTSTYLE_SHADOWED); 
        y += CG_Text_Height( "A", textscale, 0 ) + 3;
    }

    // cheats warning
    s = Info_ValueForKey( sysInfo, "sv_cheats" );

    cg.cheatsEnabled = qfalse;
    if ( s[0] == '1' ) {
        CG_Text_Paint( x,y, textscale, colorWhite, va("Cheats are enabled" ), 0,0, ITEM_TEXTSTYLE_SHADOWED);
        y += CG_Text_Height( "A", textscale, 0 ) + 3;
        cg.cheatsEnabled = qtrue;
    }

    // game type
    switch ( cgs.gametype ) {
    case GT_FFA:
        s = "Training";
        break;
    case GT_TEAM:
        s = "Free Teamplay";
        break;
    case GT_LTS:
        s = "Mission Based";
        break;
    default:
        s = "Unknown Gametype";
        break;
    }
    CG_Text_Paint( x,y, textscale, colorWhite,  va("Gametype is %s", s ), 0,0, ITEM_TEXTSTYLE_SHADOWED);
    y += CG_Text_Height( "A", textscale, 0 ) + 3;

    value = atoi( Info_ValueForKey( info, "timelimit" ) );
    if ( value ) {
        CG_Text_Paint( x,y, textscale, colorWhite,  va("Timelimit is set to: %i", value ), 0,0, ITEM_TEXTSTYLE_SHADOWED);
        y += CG_Text_Height( "A", textscale, 0 ) + 3;
    }

    if (cgs.gametype < GT_TEAM ) {
        value = atoi( Info_ValueForKey( info, "fraglimit" ) );
        if ( value ) {
            CG_Text_Paint( x,y, textscale, colorWhite,  va("Fraglimit is set to: %i", value ), 0,0, ITEM_TEXTSTYLE_SHADOWED);
            y += CG_Text_Height( "A", textscale, 0 ) + 3;
        }
    }

    if (cgs.gametype >= GT_TEAM) {
        value = atoi( Info_ValueForKey( info, "teampointlimit" ) );
        if ( value ) {
            CG_Text_Paint( x,y, textscale, colorWhite,  va("Pointlimit is set to: %i", value ), 0,0, ITEM_TEXTSTYLE_SHADOWED);
            y += CG_Text_Height( "A", textscale, 0 ) + 3;
        }
    }
#endif
    if ( sealBriefingLines > 0 ) {
		textscale = 0.18f;

        y = 242 + CG_Text_Height( "A" , textscale, 0 )  + 3;

        for ( value = 0; value <= sealBriefingLines; value ++ )
        {
            CG_Text_Paint( 80, y, textscale, colorWhite, SealBriefing[value], 0,0,ITEM_TEXTSTYLE_SHADOWED);
            y += CG_Text_Height( SealBriefing[value], textscale, 0 ) + 3;
        }
    }
    if ( tangoBriefingLines > 0 ) {
		textscale = 0.18f;

        y = 242 + CG_Text_Height( "A" , textscale, 0 ) + 3;

        for ( value = 0; value <= tangoBriefingLines; value ++ )
        {
            CG_Text_Paint( 328, y, textscale, colorWhite, TangoBriefing[value], 0,0,ITEM_TEXTSTYLE_SHADOWED);
            y += CG_Text_Height( TangoBriefing[value], textscale, 0 ) + 3;
        }
    }  
}

