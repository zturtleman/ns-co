// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// gameinfo.c
//

#include "ui_local.h"


//
// arena and bot info
//


int				ui_numBots;
static char		*ui_botInfos[MAX_BOTS];

static int		ui_numArenas;
static char		*ui_arenaInfos[MAX_ARENAS];

static int		ui_numSinglePlayerArenas;
static int		ui_numSpecialSinglePlayerArenas;

/*
===============
UI_ParseInfos
===============
*/
int UI_ParseInfos( char *buf, int max, char *infos[] ) {
    char	*token;
    int		count;
    char	key[MAX_TOKEN_CHARS];
    char	info[MAX_INFO_STRING];

    count = 0;

    while ( 1 ) {
        token = COM_Parse( &buf );
        if ( !token[0] ) {
            break;
        }
        if ( strcmp( token, "{" ) ) {
            Com_Printf( "Missing { in info file\n" );
            break;
        }

        if ( count == max ) {
            Com_Printf( "Max infos exceeded\n" );
            break;
        }

        info[0] = '\0';
        while ( 1 ) {
            token = COM_ParseExt( &buf, qtrue );
            if ( !token[0] ) {
                Com_Printf( "Unexpected end of info file\n" );
                break;
            }
            if ( !strcmp( token, "}" ) ) {
                break;
            }
            Q_strncpyz( key, token, sizeof( key ) );

            token = COM_ParseExt( &buf, qfalse );
            if ( !token[0] ) {
                strcpy( token, "<NULL>" );
            }
            Info_SetValueForKey( info, key, token );
        }
        //NOTE: extra space for arena number
        infos[count] = UI_Alloc(strlen(info) + strlen("\\num\\") + strlen(va("%d", MAX_ARENAS)) + 1);
        if (infos[count]) {
            strcpy(infos[count], info);
            count++;
        }
    }
    return count;
}

/*
===============
UI_LoadArenasFromFile
===============
*/
static void UI_LoadArenasFromFile( char *filename ) {
    int				len;
    fileHandle_t	f;
    char			buf[MAX_ARENAS_TEXT];

    len = trap_FS_FOpenFile( filename, &f, FS_READ );
    if ( !f ) {
        trap_Print( va( S_COLOR_RED "file not found: %s\n", filename ) );
        return;
    }
    if ( len >= MAX_ARENAS_TEXT ) {
        trap_Print( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_ARENAS_TEXT ) );
        trap_FS_FCloseFile( f );
        return;
    }

    trap_FS_Read( buf, len, f );
    buf[len] = 0;
    trap_FS_FCloseFile( f );

    ui_numArenas += UI_ParseInfos( buf, MAX_ARENAS - ui_numArenas, &ui_arenaInfos[ui_numArenas] );
}

/*
===============
UI_LoadArenas
===============
*/
void UI_LoadArenas( void ) {
    int			numdirs;
    vmCvar_t	arenasFile;
    char		filename[128];
    char		dirlist[1024];
    char*		dirptr;
    int			i, n;
    int			dirlen;
    char		*type;

    ui_numArenas = 0;
    uiInfo.mapCount = 0;

    trap_Cvar_Register( &arenasFile, "g_arenasFile", "", CVAR_INIT|CVAR_ROM );
    if( *arenasFile.string ) {
        UI_LoadArenasFromFile(arenasFile.string);
    }
    else {
        UI_LoadArenasFromFile("scripts/maps.txt");
    }

    // get all maps from .map files
    numdirs = trap_FS_GetFileList("scripts", ".map", dirlist, 1024 );
    dirptr  = dirlist;
    for (i = 0; i < numdirs; i++, dirptr += dirlen+1) {
        dirlen = strlen(dirptr);
        strcpy(filename, "scripts/");
        strcat(filename, dirptr);
        UI_LoadArenasFromFile(filename);
    }
    trap_Print( va( "%i maps parsed\n", ui_numArenas ) );
    if (UI_OutOfMemory()) {
        trap_Print(S_COLOR_YELLOW"WARNING: not anough memory in pool to load all arenas\n");
    }

    for( n = 0; n < ui_numArenas; n++ ) {
        // determine type

        uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
        uiInfo.mapList[uiInfo.mapCount].mapLoadName = String_Alloc(Info_ValueForKey(ui_arenaInfos[n], "map"));
        uiInfo.mapList[uiInfo.mapCount].mapName = String_Alloc(Info_ValueForKey(ui_arenaInfos[n], "longname"));
        uiInfo.mapList[uiInfo.mapCount].levelShot = -1;
        uiInfo.mapList[uiInfo.mapCount].imageName = String_Alloc(va("levelshots/%s", uiInfo.mapList[uiInfo.mapCount].mapLoadName));
        uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

        type = Info_ValueForKey( ui_arenaInfos[n], "type" );
        // if no type specified, it will be treated as "ffa"
        if( *type ) {
            if( strstr( type, "ffa" ) ) {
                uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_FFA);
            }
            if( strstr( type, "team") ) {
                uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_TEAM );
            }
            if( strstr( type, "opscmp") ) {
                //uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_CAMPAING );
            }
            if( strstr( type, "ops" ) )  {
                uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_LTS );
            }
        } else {
            uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_FFA);
        }

        uiInfo.mapCount++;
        if (uiInfo.mapCount >= MAX_MAPS) {
            break;
        }
    }
}


