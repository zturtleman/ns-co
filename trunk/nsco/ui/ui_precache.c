#include "ui_local.h"

weaponInfo_t ui_weapons[MAX_WEAPONS*2];

/*
=================
UI_RegisterWeapon

Precache a weapon
=================
*/
void UI_RegisterWeapon( int weaponNum )
{
    weaponInfo_t	*weaponInfo;
    gitem_t			*item;
    char			path[MAX_QPATH];
    //	vec3_t			mins, maxs;
    int				i;

    weaponInfo = &ui_weapons[weaponNum];

    if ( weaponNum == 0 ) {
        return;
    }

    if ( weaponInfo->registered ) {
        return;
    }

    memset( weaponInfo, 0, sizeof( *weaponInfo ) );
    weaponInfo->registered = qtrue;

    for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
        if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
            weaponInfo->item = item;
            break;
        }
    }
    if ( !item->classname ) {
        Com_Error( ERR_FATAL, "Couldn't find weapon %i", weaponNum );
    } 
 
    /* ==========
    Muzzle Flash
    ==========
    */
    if ( BG_IsPrimary( weaponNum) || BG_IsSecondary( weaponNum ) && weaponNum != WP_SL8SD )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_flash.md3" );
        weaponInfo->flashModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->flashModel ) {
            // new default ;
            Com_Printf("Couldn't register: Flash Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Scope
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_SCOPE ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_scope.md3" );
        weaponInfo->scopeModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_scope_vweap.md3" );
        weaponInfo->v_scopeModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->scopeModel  ) {
            // new default ;
            Com_Printf("Couldn't register: Scope Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Laser sight Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_LASER ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_laser.md3" );
        weaponInfo->lasersightModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_laser_vweap.md3" );
        weaponInfo->v_lasersightModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->lasersightModel  ) {
            // new default ;
            Com_Printf("Couldn't register: Lasersight Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Silencer Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_SILENCER ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_silencer.md3" );
        weaponInfo->silencerModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_silencer_vweap.md3" );
        weaponInfo->v_silencerModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->silencerModel ) {
            // new default ;
            Com_Printf("Couldn't register: Silencer Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Flashlight Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_FLASHLIGHT ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_flashlight.md3" );
        weaponInfo->glModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_flashlight_vweap.md3" );
        weaponInfo->v_glModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->glModel ) {
            // new default ;
            Com_Printf("Couldn't register: Flashlight Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Duckbill Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_DUCKBILL ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_duckbill.md3" );
        weaponInfo->silencerModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_duckbill_vweap.md3" );
        weaponInfo->v_silencerModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->silencerModel ) {
            // new default ;
            Com_Printf("Couldn't register: Duckbill Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    Bayonet Model
    ==========
    */
    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_BAYONET ) )
    {

        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_bayonet.md3" );
        weaponInfo->bayonetModel = trap_R_RegisterPermanentModel( path );
        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_bayonet_vweap.md3" );
        weaponInfo->v_bayonetModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->bayonetModel ) {
            // new default ;
            Com_Printf("Couldn't register: Bayonet Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    GrenadeLauncher Model
    ==========
    */

    if ( BG_WeaponMods( weaponNum ) & ( 1 << WM_GRENADELAUNCHER ) )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        if ( item->giTag == WP_M4 )
            strcat( path, "_m203_1stperson.md3" );
        else
            strcat( path, "_bg15_1stperson.md3" );

        weaponInfo->glModel  = trap_R_RegisterPermanentModel( path );

        // vweap
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        if ( item->giTag == WP_M4 )
            strcat( path, "_m203_vweap.md3" );
        else
            strcat( path, "_bg15_vweap.md3" );

        weaponInfo->v_glModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->glModel ) {
            // new default ;
            Com_Printf("Couldn't register: GrenadeLauncher Model (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }



    /* ==========
    1st Person Model [ Weapon ]
    ==========
    */
    if ( weaponNum != WP_C4 )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_1stperson_body.md3" );
        weaponInfo->weaponModel = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->weaponModel ) {
            Com_Printf("Couldn't register: 1rd Person Model[WEAPON] (for: %s)\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    1st Person Model [ Hands ]
    ==========
    */
    strcpy( path, item->world_model[0] );
    COM_StripExtension( path, path );
    strcat( path, "_1stperson.md3" );
    weaponInfo->handsModel = trap_R_RegisterPermanentModel( path );

    if ( !weaponInfo->handsModel ) {
        Com_Printf("Couldn't register: 1rd Person Model (for: %s)\n", weaponInfo->item->pickup_name);
    }

    /* ==========
    1st Person Model Skins [ Tango Hand Skin]
    ==========
    */
 //   if ( !cg_disableTangoHandSkin.integer )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );
        strcat( path, "_t.skin" );
        weaponInfo->t_viewweaponSkin = trap_R_RegisterSkin( path );

        if ( !weaponInfo->t_viewweaponSkin ) {
            Com_Printf("Couldn't register Tango Hand Skin for: %s\n", weaponInfo->item->pickup_name);
        }
    }

    /* ==========
    View Weapon
    ==========
    */
    strcpy( path, item->world_model[0] );
    COM_StripExtension( path, path );
    strcat( path, "_vweap.md3" );
    weaponInfo->viewweaponModel  = trap_R_RegisterPermanentModel( path );

    if ( !weaponInfo->viewweaponModel ) {
        Com_Printf("Couldn't register: View Weapon Model (for: %s)\n", weaponInfo->item->pickup_name);
    } 
 
    weaponInfo->partTags[0] = "";

    switch ( weaponNum ) {
    case WP_SEALKNIFE:
    case WP_KHURKURI: 
        break; 
    case WP_MACMILLAN: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = ""; 
        break; 
    case WP_SL8SD: // WP_SL8SD 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[3] = "mageject";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[5] = "cock2";
        weaponInfo->partTags[4] = ""; 
        break; 
    case WP_PSG1: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = ""; 
        break;
    case WP_MAC10: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = ""; 
        break;
    case WP_PDW: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "catcher";
        weaponInfo->partTags[4] = ""; 
        break;
    case WP_MP5: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = ""; 
        break;
    case WP_AK47: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "gl";
        weaponInfo->partTags[4] = ""; 
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/misc/bullets/bullet_40mm.md3" );  
        break;
    case WP_M14: 
        weaponInfo->partTags[0] = "cock";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = ""; 
        break;

    case WP_M249: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "lid";
        weaponInfo->partTags[3] = "bullet";
        weaponInfo->partTags[4] = "belt";
        weaponInfo->partTags[5] = ""; 
        break;
    case WP_M4: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "m203gren";
        weaponInfo->partTags[3] = "m203rel";
        weaponInfo->partTags[4] = "m203";
        weaponInfo->partTags[5] = "handle";
        weaponInfo->partTags[6] = ""; 
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/misc/bullets/bullet_40mm.md3" );  
        break;


    case WP_GLOCK:
		weaponInfo->partTags[0] = "trigger";
		weaponInfo->partTags[1] = "mag";
		weaponInfo->partTags[2] = "cock";
		weaponInfo->partTags[3] = "sled";
		weaponInfo->partTags[4] = ""; 
        break;
    case WP_P9S:  
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "sled";
        weaponInfo->partTags[4] = "";
        break;
    case WP_MK23: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "bolt";
        weaponInfo->partTags[3] = "sled";
        weaponInfo->partTags[4] = ""; 
        break;
    case WP_SW629: 
        weaponInfo->partTags[0] = "barrel";
        weaponInfo->partTags[1] = "bolt";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "mag";
        weaponInfo->partTags[4] = "sled";
        weaponInfo->partTags[5] = ""; 
        break;

    case WP_DEAGLE: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "sled";
        weaponInfo->partTags[3] = ""; 
        break;
    case WP_SW40T: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = "sled";
        weaponInfo->partTags[4] = ""; 
        break;
    case WP_SPAS15: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "mag";
        weaponInfo->partTags[2] = "cock";
        weaponInfo->partTags[3] = ""; 
        break;
    case WP_M590: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "pump";
        weaponInfo->partTags[2] = "mag";
        weaponInfo->partTags[3] = ""; 
        break;
    case WP_870: 
        weaponInfo->partTags[0] = "trigger";
        weaponInfo->partTags[1] = "pump";
        weaponInfo->partTags[2] = ""; 
        break;
        // NS -
    case WP_FLASHBANG:
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/weapons/flashbang/flashbang_thrown.md3" ); 
        weaponInfo->partTags[0] = "pin";
        weaponInfo->partTags[1] = "";

        break;
    case WP_GRENADE:
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/weapons/mk26/mk26_thrown.md3" );  
        weaponInfo->partTags[0] = "pin";
        weaponInfo->partTags[1] = "";
        break;
    case WP_SMOKE:
        weaponInfo->missileModel = trap_R_RegisterPermanentModel( "models/weapons/smoke/smoke_thrown.md3" ); 
        weaponInfo->partTags[0] = "pin";
        weaponInfo->partTags[1] = "";
        break;


    default:
        weaponInfo->kickBack = 0;
        MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
        break;
    }

    i = 0;

    while ( strlen(weaponInfo->partTags[i]) > 0 && i < MAX_WEAPONPARTS )
    {
        strcpy( path, item->world_model[0] );
        COM_StripExtension( path, path );

        if ( !Q_stricmp( "handle", weaponInfo->partTags[i] ) ) {
            weaponInfo->v_flashModel = trap_R_RegisterPermanentModel( va("%s_handle_vweap.md3",path) );
            strcat( path, va("_%s.md3", weaponInfo->partTags[i] )  );
        }
        else
            strcat( path, va("_1stperson_%s.md3", weaponInfo->partTags[i] )  );

        weaponInfo->weaponParts[i] = trap_R_RegisterPermanentModel( path );

        if ( !weaponInfo->partTags[i]  /*|| 1*/ ) {

            if ( !weaponInfo->partTags[i] )
                Com_Error( ERR_DROP, "Couldn't register Modelpart %s (for: %s)\n", path, weaponInfo->item->pickup_name);
        }
        i++;
    }
}
/*
==========================
UI_RegisterPlayerModel
==========================
*/
void UI_RegisterPlayerModel( char *modelName ) 
{
    char		filename[MAX_QPATH];

    Com_sprintf( filename, sizeof( filename ), "models/players/%s/legs.md3", modelName );
	if ( !trap_R_RegisterPermanentModel( filename ) ) {
		Com_Error( ERR_FATAL, "Failed to load model file %s (wrong lower.md3)\n", filename );
    }

    Com_sprintf( filename, sizeof( filename ), "models/players/%s/torso.md3", modelName );
	if ( !trap_R_RegisterPermanentModel( filename ) ) {
		Com_Error( ERR_FATAL, "Failed to load model file %s (wrong torso.md3)\n", filename );
    }

    Com_sprintf( filename, sizeof( filename ), "models/players/%s/arml.md3", modelName );
	if ( !trap_R_RegisterPermanentModel( filename ) ) {
		Com_Error( ERR_FATAL, "Failed to load model file %s (wrong arml.md3)\n", filename );
    }

	Com_sprintf( filename, sizeof( filename ), "models/players/%s/armr.md3", modelName );
   	if ( !trap_R_RegisterPermanentModel( filename ) ) {
		Com_Error( ERR_FATAL, "Failed to load model file %s (wrong armr.md3)\n", filename );
    }
 
    Com_sprintf( filename, sizeof( filename ), "models/players/heads/head.md3" );
   	if ( !trap_R_RegisterPermanentModel( filename ) ) {
		Com_Error( ERR_FATAL, "Failed to load model file %s (wrong head.md3)\n", filename );
    }  
}

/*
==========================
UI_RegisterHeadstyle
==========================
*/
void UI_RegisterHeadstyle( char *mouthName, char *eyesName, char *headName ) 
{
    char		filename[MAX_QPATH];

    // disabled.
    if ( trap_Cvar_VariableValue("cg_disableHeadstuff") )
        return;

    // load cmodels before models so filecache works
	if ( mouthName ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/m_%s.md3", mouthName );
		trap_R_RegisterPermanentModel( filename );
	}

	if ( headName ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/h_%s.md3", headName );
		trap_R_RegisterPermanentModel( filename );
	}

	if ( eyesName ) {
		Com_sprintf( filename, sizeof( filename ), "models/players/heads/accessoires/e_%s.md3", eyesName );
		trap_R_RegisterPermanentModel( filename );
	}
}