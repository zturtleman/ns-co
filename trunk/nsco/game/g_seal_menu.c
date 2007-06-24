#include "g_local.h"
//
// all code in this file is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

/*
==============

MENUING SYSTEM

==============
*/

/*
=================
NSQ3 Call Menu
author: dX
date: 10-05-2k
description: calls menu
=================
*/

// menu
#define MENU_NONE		0

#define MENU_TEAM		1
#define MENU_PRIM_WEAP	2
#define MENU_SPECTATOR	3
#define MENU_SECO_WEAP	4
#define MENU_MAIN		5
#define MENU_AMMO		6
#define MENU_CHARACTER	7
#define MENU_CLASS		8
#define MENU_RADIO		9
#define MENU_EQUIP		10
#define MENU_OWNCLASS	11
#define MENU_PLAYER		12
#define MENU_MAX		13

#define SLOTS_TEAMMENU	3
#define SLOTS_PRIM_WEAP	10
#define SLOTS_SPECTATOR	1
#define SLOTS_SECO_WEAP	5
#define SLOTS_MAIN		7
#define SLOTS_AMMO		6
#define SLOTS_CHARACTER	1
#define	SLOTS_CLASSMENU	7
#define SLOTS_RADIO		9
#define SLOTS_EQUIP		9
#define SLOTS_OWNCLASS	7
#define SLOTS_PLAYER	5



void NS_Menu (gentity_t *ent,char *string, int avaiable_options, int menu)
{
    // no slots set? set to max
    if (avaiable_options <= 0)
        avaiable_options = 9;

    // set menu
    ent->client->pers.activeMenu = menu;

    trap_SendServerCommand( ent-g_entities, va("menu %i %s", avaiable_options, string) );
}

void NS_CallMenu( gentity_t *ent, int menu, int menuSlot )
{
    if ( !menu || !menuSlot || !ent || menu >= MENU_MAX )
        return;

    switch (menu) {
    case MENU_TEAM:
        NS_HandleTeamMenu( ent, menuSlot);
        break;
    case MENU_PRIM_WEAP:
        NS_HandlePrimaryWeaponMenu ( ent, menuSlot );
        break;
    case MENU_SPECTATOR:
        break;
    case MENU_SECO_WEAP:
        NS_HandleSecondaryWeaponMenu (ent, menuSlot );
        break;
    case MENU_MAIN:
        NS_HandleMainMenu(ent, menuSlot );
        break;
    case MENU_AMMO:
        NS_HandleAmmoMenu(ent, menuSlot);
        break;
    case MENU_CHARACTER:
        NS_HandleCharacterMenu(ent,menuSlot);
        break;
    case MENU_CLASS:
        NS_HandleClassMenu(ent,menuSlot);
        break;
    case MENU_RADIO:
        NS_HandleRadioMenu(ent,menuSlot);
        break;
    case MENU_EQUIP:
        NS_HandleEquipmentMenu(ent, menuSlot);
        break;
    case MENU_OWNCLASS:
        NS_HandleCreateClassMenu( ent, menuSlot );
        break;
    case MENU_PLAYER:
        NS_HandlePlayerMenu( ent, menuSlot );
        break;
    default:
        break;
    }
}
void NS_MenuSelect ( gentity_t *ent )
{
    char		arg[MAX_TOKEN_CHARS];
    int menuSlot = 0;
    int	menu;

    // no Active Menu?
    if (ent->client->pers.activeMenu == MENU_NONE)
        return;

    // smaller then 1 command?
    if ( trap_Argc () < 1 ) {
        return;
    }

    trap_Argv( 1, arg, sizeof( arg ) );
    menuSlot = atoi( arg );
    menu = ent->client->pers.activeMenu;

    ent->client->pers.activeMenu = MENU_NONE;

    NS_CallMenu( ent, menu, menuSlot );
}

/*
====================================
Radio Menu Handling
====================================
*/

void NS_OpenRadioMenu ( gentity_t *ent )
{
    char *radio_menu = "\"1. Go\" \"2. Team Report In\" \"3. Enemy Down\" \"4. Taking Fire - Need Assistance\" \"5. Teammate Down\" \"6. Cover Me\" \"7. I Am Hit\" \"8. Get in position\" \" \" \"9. Exit\"";

    // if not on teamplay
    if (g_gametype.integer < GT_TEAM)
        return;

    NS_Menu(ent,radio_menu,SLOTS_RADIO, MENU_RADIO);
}

void NS_HandleRadioMenu ( gentity_t *ent, int menuSlot )
{
}


/*
====================================
Team Selection Menu Handling
====================================
*/

void NS_OpenTeamMenu ( gentity_t *ent )
{
    char *team_menu = "\"Welcome to Navy Seals : Covert Operations\" \" \" \"Please select a Team using the Weapon keys\" \"?\" \"1. Join Seals\" \"2. Join Tangos\" \" \" \"3. Spectate\"";

    if ( g_gametype.integer < GT_TEAM )
    {
        team_menu = "\"Welcome to Navy Seals : Covert Operations\" \" \" \"Please select a Team using the Weapon keys\" \"?\" \"1. Join Game\" \" \" \"2. Spectate\"";
        NS_Menu(ent,team_menu,2, MENU_TEAM);
    }
    else
        NS_Menu(ent,team_menu,SLOTS_TEAMMENU, MENU_TEAM);
}

void NS_HandleTeamMenu ( gentity_t *ent, int menuSlot )
{
    if ( g_gametype.integer < GT_TEAM )
    {
        switch (menuSlot) {
        case 1:
            SetTeam(ent, "free" );
            NS_NavySeals_ClientInit(ent, qtrue );
            break;
        case 2:
            SetTeam(ent, "spectator" );
            break;
        default:
            break;
        }
        return;
    }
    switch (menuSlot) {
    case 1:
        // if we're in LTS mode, wait for the next round
        if (g_gametype.integer == GT_LTS)
            SetTeam(ent, "seals" );
        else // else place us immediately
            SetTeam(ent, "tangos" );

        // first time we played. set custom class

        break;
    case 2:
        if (g_gametype.integer == GT_LTS)
            SetTeam(ent, "seals" );
        else
            SetTeam(ent, "tangos" );

        // first time we played. set custom class

        break;
    case 3:
        // respawn as spectator
        SetTeam(ent, "spectator" );
        SetClass( ent, CLASS_NONE );
        NS_OpenSpectatorMenu ( ent );
        break;
    default:
        break;
    }
}

/*
====================================
Create Custom Class Menu Handling

valid abilities ( max class : 10 ) points to share 3:


accuracy
speed
stamina
stealth
strength
technical

====================================
*/

/*

returns no of used points

*/

#define CLASS_ABILITYPOINTS			4
#define CLASS_MAXSTARTABILITYPOINTS	10

int NS_AbilityPointsUsed ( gentity_t *ent )
{
    int points = 0;

    if ( ent->client->pers.nsPC.speed > 1 )
        points += ent->client->pers.nsPC.speed - 1; // 1 = base point
    if ( ent->client->pers.nsPC.accuracy > 1 )
        points += ent->client->pers.nsPC.accuracy - 1;
    if ( ent->client->pers.nsPC.stamina > 1 )
        points += ent->client->pers.nsPC.stamina - 1;
    if ( ent->client->pers.nsPC.stealth > 1 )
        points += ent->client->pers.nsPC.stealth - 1;
    if ( ent->client->pers.nsPC.strength > 1 )
        points += ent->client->pers.nsPC.strength - 1;
    if ( ent->client->pers.nsPC.technical > 1 )
        points += ent->client->pers.nsPC.technical - 1;

    return points;
}

qboolean NS_GotEnoughXPfornextLevel ( gentity_t *ent , int cur_level )
{
    if ( ( ent->client->pers.nsPC.xp - ( cur_level + 1 ) ) >= 0 ) // got enough
        return qtrue;

    return qfalse;
}

void NS_OpenCreateClassMenu ( gentity_t *ent )
{
    char create_class_menu[512];

    Com_sprintf( create_class_menu, sizeof ( create_class_menu ),
                 "\"Please create your Character.\" \"To reach next level it costs you x XP ,\" \"where x is the level you want to reach.\" \"You got [%i] XP Points left.\" \"?\" \"1. Accuracy (Cur:%i/10)\" \"2. Speed (Cur:%i/10)\" \"3. Stamina (Cur:%i/10)\" \"4. Stealth (Cur: %i/10)\" \"5. Strength (Cur: %i/10)\" \"6. Technical (Cur: %i/10)\" \" \" \"7. Exit\""
                 , ent->client->pers.nsPC.xp, ent->client->pers.nsPC.accuracy , ent->client->pers.nsPC.speed, ent->client->pers.nsPC.stamina , ent->client->pers.nsPC.stealth , ent->client->pers.nsPC.strength, ent->client->pers.nsPC.technical    );

    NS_Menu(ent,create_class_menu,SLOTS_OWNCLASS, MENU_OWNCLASS);
}
// 1 accuracy
// 2 speed
// 3 stamina
// 4 stealth
// 5 strength
// 6 technical
void NS_HandleCreateClassMenu ( gentity_t *ent, int menuSlot )
{
    switch (menuSlot) {
    case 1:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.accuracy ) && ent->client->pers.nsPC.accuracy < 10 )  {
            ent->client->pers.nsPC.accuracy ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.accuracy , qtrue );
        }
        else
            PrintMsg( ent, "Not enough Experience Points\n" );

        NS_OpenCreateClassMenu(ent);
        break;
    case 2:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.speed ) && ent->client->pers.nsPC.speed < 10 ) {
            ent->client->pers.nsPC.speed ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.speed, qtrue);
        }
        else
            PrintMsg( ent, "Not enough Experience Points\n" );

        NS_OpenCreateClassMenu(ent);
        break;
    case 3:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.stamina ) && ent->client->pers.nsPC.stamina < 10 ) {
            ent->client->pers.nsPC.stamina ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.stamina , qtrue );
        }
        else
            PrintMsg( ent, "Not enough Experience Points\n" );

        NS_OpenCreateClassMenu(ent);
        break;
    case 4:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.stealth ) && ent->client->pers.nsPC.stealth < 10 ) {
            ent->client->pers.nsPC.stealth ++;
            ent->client->ps.stats[STAT_STEALTH] ++ ;

            NS_GiveXP( ent, ent->client->pers.nsPC.stealth , qtrue );
        }
        else
            PrintMsg( ent, "Not enough Experience Points\n" );

        NS_OpenCreateClassMenu(ent);
        break;
    case 5:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.strength ) && ent->client->pers.nsPC.strength < 10 ) {
            ent->client->pers.nsPC.strength ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.strength , qtrue );
        }
        else
            PrintMsg( ent, "Not enough Experience Points\n" );

        NS_OpenCreateClassMenu(ent);
        break;
    case 6:
        if ( NS_GotEnoughXPfornextLevel ( ent, ent->client->pers.nsPC.technical ) && ent->client->pers.nsPC.technical < 10 ) {
            ent->client->pers.nsPC.technical ++;

            NS_GiveXP( ent, ent->client->pers.nsPC.technical  , qtrue );
        }
        else
            PrintMsg( ent, "Not enough Experience Points\n" );

        NS_OpenCreateClassMenu(ent);
        break;
    case 7:
        NS_OpenMainMenu(ent);
        break;
    default:
        break;
    }
}

/*
====================================
Class Selection Menu Handling
====================================
*/

void NS_OpenClassMenu ( gentity_t *ent )
{
    char *class_menu = "\"1. Recon\" \"2. Assault\" \"3. Commander\" \"4. Heavy Support\" \"5. Sniper\" \"6. Demolishions Expert\" \"7. Create Custom Class\" ";

    NS_Menu(ent,class_menu,SLOTS_CLASSMENU, MENU_CLASS);
}

void NS_HandleClassMenu ( gentity_t *ent, int menuSlot )
{
    switch (menuSlot) {
    case 1:
        SetClass(ent, CLASS_RECON);
        NS_OpenMainMenu(ent);
        break;
    case 2:
        SetClass(ent, CLASS_ASSAULT);
        NS_OpenMainMenu(ent);
        break;
    case 3:
        SetClass(ent, CLASS_COMMANDER);
        NS_OpenMainMenu(ent);
        break;
    case 4:
        SetClass(ent, CLASS_HEAVYSUPPORT);
        NS_OpenMainMenu(ent);
        break;
    case 5:
        SetClass(ent, CLASS_SNIPER);
        NS_OpenMainMenu(ent);
        break;
    case 6:
        SetClass(ent, CLASS_DEMOMAN);
        NS_OpenMainMenu(ent);
        break;
    case  7:
        // choose custom class
        SetClass(ent, CLASS_CUSTOM );
        NS_OpenCreateClassMenu( ent );
        break;

    default:
        break;
    }
}

/*
====================================
Primary Weapon Menu Handling
====================================
*/
void NS_OpenPrimaryWeaponMenu ( gentity_t *ent )
{
    // mp5,m4,remington870
    // spas-15 str > 6
    // psg-1 accuracy > 6
    // pdw str > 3 & accuracy > 4
    // macmillan accuracy > 8 & str > 4
    char *line_1 = "\"1. Heckler & Koch Mp5-A5\"";
    char *line_2 = "\"2. Colt M4-A1\"";
    char *line_3 = "\"3. Remington 870-Mark 1\"";
    char *line_4 = "\"4. Frenchi Spas-15 ( str > 6 )\"";
    char *line_5 = "\"5. Heckler & Koch PSG-1 ( acc > 6 )\"";
    char *line_6 = "\"6. Heckler & Koch PDW ( str > 3 & acc > 4 )\"";
    char *line_7 = "\"7. Mac Millan .50 ( str > 4 & acc > 8 )\"";
    char *primary_weapon_menu;

    if ( ent->client->sess.sessionTeam == TEAM_BLUE )
    {
        line_1 = "\"1. Ingram Mac-10\"";
        line_2 = "\"2. Avtomat Kalashnikov 47.223\"";
    }

    primary_weapon_menu = va("\"Please select your primary Weapon.\" \"?\" %s %s %s %s %s %s %s \" \"		\"0. Exit\"", line_1,line_2,line_3,line_4,line_5,line_6,line_7 );


    NS_Menu(ent,primary_weapon_menu,SLOTS_PRIM_WEAP, MENU_PRIM_WEAP);
    //3. Ak47 .233\"
}


void NS_SetPrimary( gentity_t *ent , int primary )
{
    gentity_t *temp;

    temp = G_TempEntity( ent->client->ps.origin, EV_STOLENWEAPON );
    temp->s.weapon = primary;
    temp->s.otherEntityNum = ent->client->ps.clientNum;

    // copy old primary modifications to the new one [only for some weapon - some can't handle weaponmods]
    if ( BG_WeaponMods( primary ) & ( 1 << WM_SILENCER ) )
        ent->client->pers.nsInven.weapon_mods[primary].silencer = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer;
    if ( BG_WeaponMods( primary ) & ( 1 << WM_LASER ) )
        ent->client->pers.nsInven.weapon_mods[primary].lasersight = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight;
    if ( BG_WeaponMods( primary ) & ( 1 << WM_SCOPE ) )
        ent->client->pers.nsInven.weapon_mods[primary].scope = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope;
    if ( BG_WeaponMods( primary ) & ( 1 << WM_BAYONET ) )
        ent->client->pers.nsInven.weapon_mods[primary].bayonet = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet;
    if ( BG_WeaponMods( primary ) & ( 1 << WM_GRENADELAUNCHER ) )
        ent->client->pers.nsInven.weapon_mods[primary].gl = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl;
    if ( BG_WeaponMods( primary ) & ( 1 << WM_DUCKBILL ) )
        ent->client->pers.nsInven.weapon_mods[primary].duckbill = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].duckbill;
    if ( BG_WeaponMods( primary ) & ( 1 << WM_FLASHLIGHT ) )
        ent->client->pers.nsInven.weapon_mods[primary].flashlight = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].flashlight;

    // delete old mods
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer = 0;
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight = 0;
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope = 0;
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet = 0;
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl = 0;


    // now set the new primary weapon
    ent->client->pers.nsInven.primaryweapon = primary;
}

void NS_HandlePrimaryWeaponMenu ( gentity_t *ent, int menuSlot )
{
    qboolean seals = ( ent->client->sess.sessionTeam == TEAM_RED );
    int acc = ent->client->pers.nsPC.accuracy;
    int str = ent->client->pers.nsPC.strength;
    // mp5,m4,remington870
    // spas-15 str > 6
    // psg-1 accuracy > 6
    // pdw str > 3 & accuracy > 4
    // macmillan accuracy > 8 & str > 4

    switch (menuSlot) {
    case 1:
        if (seals)
            NS_SetPrimary( ent, WP_MP5 );
        else
            NS_SetPrimary( ent, WP_MAC10 );
        NS_OpenMainMenu( ent );
        break;
    case 2:
        if (seals)
            NS_SetPrimary( ent, WP_M4 );
        else
            NS_SetPrimary( ent, WP_AK47 );
        NS_OpenMainMenu( ent );
        break;
    case 3:
        NS_SetPrimary( ent, WP_870 );
        NS_OpenMainMenu( ent );
        break;
    case 4:
        if ( str > 6 ) {
            NS_SetPrimary( ent, WP_SPAS15 );
            NS_OpenMainMenu( ent );
        }
        else
        {
            PrintMsg(ent,"You're not good enough for this weapon.\n");
            NS_OpenPrimaryWeaponMenu(ent);
        }
        break;
    case 6:
        if ( str > 3 && acc > 4 ) {
            NS_SetPrimary( ent, WP_PDW );
            NS_OpenMainMenu( ent );
        }
        else
        {
            PrintMsg(ent,"You're not good enough for this weapon.\n");
            NS_OpenPrimaryWeaponMenu(ent);
        }
        break;
    case 5:
        if ( acc > 6 ) {
            NS_SetPrimary( ent, WP_PSG1 );
            NS_OpenMainMenu( ent );
        }
        else
        {
            PrintMsg(ent,"You're not good enough for this weapon.\n");
            NS_OpenPrimaryWeaponMenu(ent);
        }
        break;
    case 7:
        if ( acc > 8 && str > 4 )
        {
            NS_SetPrimary( ent, WP_MACMILLAN );
            NS_OpenMainMenu( ent );
        }
        else
        {
            PrintMsg(ent,"You're not good enough for this weapon.\n");
            NS_OpenPrimaryWeaponMenu(ent);
        }
        break;
    case 10:
        NS_OpenMainMenu( ent );
        break;
    default:
        NS_OpenPrimaryWeaponMenu( ent );
        break;
    }
}

/*
====================================
Secondary Weapon Loadout Menu
====================================
*/
void NS_OpenSecondaryWeaponMenu ( gentity_t *ent )
{
    char *secondary_weapon_menu =
        "\"Please select your secondary Weapon.\" 		\"?\" 		\"1. H&K P9s\"		\"2. H&K Mk23 Socom Pistol\" \"3. Desert Eagle ( strength > 3 )\" 			\" \" 		\"0. Exit\"";

    if ( ent->client->sess.sessionTeam == TEAM_BLUE )
        secondary_weapon_menu = "\"Please select your secondary Weapon.\" 		\"1. Glock-18 (20 Round)\" 		\"2. S&W 40t\" \"3. Desert Eagle ( strength > 3)\"		\" \" 		 \"0. Exit\"";

    NS_Menu(ent,secondary_weapon_menu,SLOTS_SECO_WEAP, MENU_SECO_WEAP);
}
void NS_SetSecondary( gentity_t *ent , int secondary )
{
    gentity_t *temp;

    temp = G_TempEntity( ent->client->ps.origin, EV_STOLENWEAPON );
    temp->s.weapon = secondary;
    temp->s.otherEntityNum = ent->client->ps.clientNum;

    // copy old primary modifications to the new one [only for some weapon - some can't handle weaponmods]
    if ( BG_WeaponMods( secondary ) & ( 1 << WM_SILENCER ) )
        ent->client->pers.nsInven.weapon_mods[secondary].silencer = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer;
    if ( BG_WeaponMods( secondary ) & ( 1 << WM_LASER ) )
        ent->client->pers.nsInven.weapon_mods[secondary].lasersight = ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight;

    //	 delete old mods
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer = 0;
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight = 0;
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].scope = 0;
    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].bayonet = 0;

    // now set the new primary weapon
    ent->client->pers.nsInven.secondaryweapon = secondary;
}

void NS_HandleSecondaryWeaponMenu ( gentity_t *ent, int menuSlot )
{
    qboolean seals = (ent->client->sess.sessionTeam == TEAM_RED);

    switch (menuSlot) {
    case 1:
        if (seals)
            NS_SetSecondary(ent, WP_P9S);
        else
            NS_SetSecondary(ent, WP_GLOCK);
        NS_OpenMainMenu( ent );
        break;
    case 2:
        if (seals)
            NS_SetSecondary(ent, WP_MK23);
        else
            NS_SetSecondary(ent, WP_SW40T);
        NS_OpenMainMenu( ent );
        break;
    case 3:
        if ( ent->client->pers.nsPC.strength > 3 )
        {
            NS_SetSecondary(ent, WP_DEAGLE );
            NS_OpenMainMenu( ent );
        }
        else
            NS_OpenSecondaryWeaponMenu(ent);
        break;
    case 0:
        NS_OpenMainMenu( ent );
        break;
    default:
        NS_OpenSecondaryWeaponMenu(ent);
        break;
    }
}

// tango |  seals
// ------+-------
// bruce |  curtis, jamal
//

#if 0
"Configurate your Playerstyle"
""
"Use your Weaponkeys."
"----------------------------"
"1. Faceskin: %s"
"2. -Camouflage: %s"
"3. -Facemask:   %s"
"4. Headequipment: %s"
" "
"5. Exit"
#endif

char *return_headstuffname( int headstuff )
            {
                switch ( headstuff ) {
                case EQ_STORMGOGGLES:
                    return "Storm Goggles";
                    break;
                case EQ_JOINT:
                    return "Joint";
                    break;
                case EQ_PIECEOFHAY:
                    return "Piece of Hay";
                    break;
                case EQ_NVGOGGLES:
                    return "Night Vision Goggles";
                    break;
                case EQ_SEALHAT:
                    return "Seal Hat";
                    break;
                case EQ_TURBAN:
                    return "Turban";
                    break;
                case EQ_HELMET:
                    return "Helmet";
                    break;
                default:
                    break;
                }

                return "None";
            }

            char *return_skinname( int skin, int team )
                        {
                            if ( team == TEAM_RED )
                                switch ( skin ) {
                                case SKIN_S_BRUCE:
                                    return "Bruce W.";
                                    break;
                                case SKIN_S_CURTIS:
                                    return "Curits S.";
                                    break;
                                }
                            if ( team == TEAM_BLUE )
                                switch ( skin ) {
                                case SKIN_T_MUSTAFA:
                                    return "Mustafa B.";
                                    break;
                                case SKIN_T_JAMAL:
                                    return "Jamal C.";
                                    break;

                                }

                            return "Default";
                        }

                        void NS_CheckSkinValid( gentity_t *ent )
                        {
                            if ( ent->client->pers.nsPC.style == EQ_NVGOGGLES )
                                ent->client->pers.nsPC.style++;


                            if ( ent->client->sess.sessionTeam == TEAM_RED )
                            {
                                if ( ent->client->pers.nsInven.faceskin >= SKIN_S_MAXSKINS )
                                    ent->client->pers.nsInven.faceskin = 0;
                                /*
                                new skins: disable
                                if ( ent->client->pers.nsInven.face_tatoo )
                                ent->client->pers.nsInven.face_tatoo = qfalse;
                                */

                                if ( ent->client->pers.nsPC.style == EQ_TURBAN )
                                    ent->client->pers.nsPC.style = 0;
                                if ( ent->client->pers.nsPC.style == EQ_SEALHAT && ent->client->pers.nsInven.powerups[PW_HELMET] )
                                    ent->client->pers.nsPC.style = 0;
                            }
                            if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                            {
                                if ( ent->client->pers.nsInven.faceskin >= SKIN_T_MAXSKINS )
                                    ent->client->pers.nsInven.faceskin = 0;
                                /*
                                new skins: disable
                                if ( ent->client->pers.nsInven.face_camo )
                                ent->client->pers.nsInven.face_camo = qfalse;
                                */
                                if ( ent->client->pers.nsPC.style == EQ_SEALHAT )
                                    ent->client->pers.nsPC.style = EQ_TURBAN;

                                if ( ent->client->pers.nsPC.style == EQ_TURBAN && ent->client->pers.nsInven.powerups[PW_HELMET] )
                                    ent->client->pers.nsPC.style = 0;
                            }

                            if ( ent->client->pers.nsPC.style > EQ_TURBAN )
                                ent->client->pers.nsPC.style = 0;
                        }

                        /*
                        ====================================
                        Player Menu
                        ====================================
                        */
                        void NS_OpenPlayerMenu ( gentity_t *ent )
                        {
                            char player_menu[512];

                            NS_CheckSkinValid( ent );

                            Com_sprintf( player_menu, sizeof ( player_menu ),
                                         "\"Configurate your Playerstyle\""
                                         "\" \""
                                         "\"Use your Weaponkeys.\""
                                         "\"----------------------------\""
                                         "\"1. Faceskin: %s\""
                                         "\"2. -Camouflage: %s\""
                                         "\"3. -Facemask:   %s\""
                                         "\"4. Headequipment: %s\""
                                         "\" \""
                                         "\"5. Exit\""

                                         , return_skinname( ent->client->pers.nsInven.faceskin, ent->client->sess.sessionTeam ) , ent->client->pers.nsInven.face_camo?"Painted!":"Nope", ent->client->pers.nsInven.face_mask?"Dressed!":"No", return_headstuffname(ent->client->pers.nsPC.style) );

                            NS_Menu(ent,player_menu,SLOTS_PLAYER, MENU_PLAYER);
                        }

                        void NS_HandlePlayerMenu ( gentity_t *ent, int menuSlot )
                        {
                            int team = ent->client->sess.sessionTeam;

                            switch (menuSlot) {
                            case 1:
                                if ( team == TEAM_RED ) {
                                    if ( ent->client->pers.nsInven.faceskin == SKIN_S_MAXSKINS )
                                        ent->client->pers.nsInven.faceskin = 0;
                                    else
                                        ent->client->pers.nsInven.faceskin ++;
                                }
                                else {
                                    if ( ent->client->pers.nsInven.faceskin == SKIN_T_MAXSKINS )
                                        ent->client->pers.nsInven.faceskin = 0;
                                    else
                                        ent->client->pers.nsInven.faceskin ++;
                                }
                                NS_OpenPlayerMenu( ent );
                                break;
                            case 2:
                                if ( ent->client->pers.nsInven.face_camo )
                                    ent->client->pers.nsInven.face_camo = qfalse;
                                else
                                    ent->client->pers.nsInven.face_camo = qtrue;

                                NS_OpenPlayerMenu( ent );
                                break;
                            case 3:
                                if ( ent->client->pers.nsInven.face_mask )
                                    ent->client->pers.nsInven.face_mask = qfalse;
                                else
                                    ent->client->pers.nsInven.face_mask = qtrue;

                                NS_OpenPlayerMenu( ent );
                                break;
                            case 4:
                                ent->client->pers.nsPC.style++;
                                NS_OpenPlayerMenu( ent );
                                break;
                            case 5:
                                ClientUserinfoChanged( ent->client->ps.clientNum );
                                trap_SendServerCommand( -1, "loaddefered\n" );	// FIXME: spelled wrong, but not changing for demo
                                NS_OpenMainMenu( ent );
                                break;
                            default:
                                break;
                            }
                        }

                        /*
                        ====================================
                        Ammonition Menu
                        ====================================
                        */
                        void NS_OpenAmmoMenu ( gentity_t *ent )
                        {
                            char ammo_menu[512];

                            gitem_t *pri,*sec;

                            pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                            sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

                            Com_sprintf( ammo_menu, sizeof ( ammo_menu ),
                                         "\"Please select your Ammo.\""
                                         "\"?\""
                                         "\"1. Primary Ammo (Cur:%i/6)(+1)\""
                                         "\"2. Secondary Ammo (Cur:%i/10)(+1)\""
                                         "\"3. Grenades    (Cur:%i/3)\""
                                         "\"4. Flash Bangs (Cur:%i/3)\""
                                         "\"5. 40mm Grenades (Cur:%i/3)\""
                                         "\" \""
                                         "\"6. Exit\""
                                         , ent->client->pers.nsInven.ammo[pri->giAmmoTag],ent->client->pers.nsInven.ammo[sec->giAmmoTag]
                                         , ent->client->pers.nsInven.ammo[AM_GRENADES], ent->client->pers.nsInven.ammo[AM_FLASHBANGS], ent->client->pers.nsInven.ammo[AM_40MMGRENADES] );

                            NS_Menu(ent,ammo_menu,SLOTS_AMMO, MENU_AMMO);
                        }

                        void NS_HandleAmmoMenu ( gentity_t *ent, int menuSlot )
                        {
                            gitem_t *pri = NULL,*sec = NULL;
                            int tag = AM_GRENADES;

                            if ( menuSlot == 4 )
                                tag = AM_FLASHBANGS;
                            else if ( menuSlot == 5)
                                tag = AM_40MMGRENADES;

                            if (ent->client->pers.nsInven.primaryweapon)
                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                            if (ent->client->pers.nsInven.secondaryweapon)
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );


                            switch (menuSlot) {
                            case 1:
                                if (!pri) {
                                    NS_OpenAmmoMenu( ent );
                                    break;
                                }
                                if (ent->client->pers.nsInven.ammo[pri->giAmmoTag] < 0 || ent->client->pers.nsInven.ammo[pri->giAmmoTag] >= 6)
                                    ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 0;
                                else
                                    ent->client->pers.nsInven.ammo[pri->giAmmoTag]++;
                                NS_OpenAmmoMenu( ent );
                                break;
                            case 2:
                                if (!sec) {
                                    NS_OpenAmmoMenu( ent );
                                    break;
                                }
                                if (ent->client->pers.nsInven.ammo[pri->giAmmoTag] < 0 || ent->client->pers.nsInven.ammo[sec->giAmmoTag] >= 10)
                                    ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 0;
                                else
                                    ent->client->pers.nsInven.ammo[sec->giAmmoTag]++;
                                NS_OpenAmmoMenu( ent );
                                break;
                            case 3:
                            case 4:
                            case 5:
                                if (ent->client->pers.nsInven.ammo[tag] < 0 || ent->client->pers.nsInven.ammo[tag] >= 3)
                                    ent->client->pers.nsInven.ammo[tag] = 0;
                                else
                                    ent->client->pers.nsInven.ammo[tag]++;
                                NS_OpenAmmoMenu( ent );
                                break;
                            case 6:
                                NS_OpenMainMenu( ent );
                                break;
                            default:
                                break;
                            }
                        }

                        /*
                        ====================================
                        Equipment Menu
                        ====================================
                        */
                        void NS_OpenEquipmentMenu ( gentity_t *ent )
                        {
                            char equipment_menu[512];

                            // these both got no lasersight, they got a gl
                            if ( ent->client->pers.nsInven.primaryweapon && BG_WeaponMods(ent->client->pers.nsInven.primaryweapon ) & ( 1 << WM_GRENADELAUNCHER ) )
                            {
                                Com_sprintf( equipment_menu, sizeof ( equipment_menu ), "\"Customize your Equipment\" \"?\" \"1. Kevlar Vest (%i/1)\" \"2. Helmet (%i/1)\" \"3. Silencer for Primary (min: Stealth 5 - %s)\" \"4. Scope for Primary (min: Accuracy 6 - %s)\" \"5. GrenadeLauncher for Primary (min: Technical 4 - %s)\" \"6. Bayonet for Primary (min: Strength 4 - %s)\" \"7. Silencer for Secondary (min: Stealth 3 - %s)\"  \"8. Lasersight for Secondary (min: Accuracy 3 - %s)\"\" \" \"9. Exit\" ",
                                             ent->client->pers.nsInven.powerups[PW_VEST],
                                             ent->client->pers.nsInven.powerups[PW_HELMET],
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight ? "Active" : "Off");

                            } else {
                                Com_sprintf( equipment_menu, sizeof ( equipment_menu ), "\"Customize your Equipment\" \"?\" \"1. Kevlar Vest (%i/1)\" \"2. Helmet (%i/1)\" \"3. Silencer for Primary (min: Stealth 5 - %s)\" \"4. Scope for Primary (min: Accuracy 6 - %s)\" \"5. Lasersight for Primary (min: Accuracy 4 - %s)\" \"6. Bayonet for Primary (min: Strength 4 - %s)\" \"7. Silencer for Secondary (min: Stealth 3 - %s)\"  \"8. Lasersight for Secondary (min: Accuracy 3 - %s)\"\" \" \"9. Exit\" ",
                                             ent->client->pers.nsInven.powerups[PW_VEST],
                                             ent->client->pers.nsInven.powerups[PW_HELMET],
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer ? "Active" : "Off",
                                             ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight ? "Active" : "Off");
                            }
                            NS_Menu(ent,equipment_menu,SLOTS_EQUIP, MENU_EQUIP);
                        }

                        void NS_HandleEquipmentMenu ( gentity_t *ent, int menuSlot )
                        {
                            switch (menuSlot) {
                            case 1:
                                if (ent->client->pers.nsInven.powerups[PW_VEST])
                                    ent->client->pers.nsInven.powerups[PW_VEST] = 0;
                                else
                                    ent->client->pers.nsInven.powerups[PW_VEST] = 1;
                                NS_OpenEquipmentMenu(ent); // !fixme
                                break;
                            case 2:
                                if (ent->client->pers.nsInven.powerups[PW_HELMET])
                                    ent->client->pers.nsInven.powerups[PW_HELMET] = 0;
                                else
                                    ent->client->pers.nsInven.powerups[PW_HELMET] = 1;
                                NS_OpenEquipmentMenu(ent); // !fixme
                                break;
                            case 3: // silencer
                                if ( ! ( BG_WeaponMods(ent->client->pers.nsInven.primaryweapon ) & ( 1 << WM_SILENCER ) ) )
                                {
                                    NS_OpenEquipmentMenu(ent);
                                    return;
                                }
                                if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer = 0;
                                else if ( ent->client->pers.nsPC.stealth >= 5 )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].silencer = 1;

                                NS_OpenEquipmentMenu(ent);
                                break;
                            case 4: // scope
                                if ( ! ( BG_WeaponMods(ent->client->pers.nsInven.primaryweapon ) & ( 1 << WM_SCOPE ) ) )
                                {
                                    NS_OpenEquipmentMenu(ent);
                                    return;
                                }
                                if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope = 0;
                                else if ( ent->client->pers.nsPC.accuracy >= 6 )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].scope = 1;

                                NS_OpenEquipmentMenu(ent); // !fixme
                                break;
                            case 5: // laser
                                {
                                    int type = WM_LASER;

                                    if ( BG_WeaponMods(ent->client->pers.nsInven.primaryweapon ) & ( 1 << WM_GRENADELAUNCHER ) )
                                        type = WM_GRENADELAUNCHER;

                                    if ( ! ( BG_WeaponMods(ent->client->pers.nsInven.primaryweapon ) & ( 1 << type ) ) )
                                    {
                                        NS_OpenEquipmentMenu(ent);
                                        return;
                                    }
                                    if ( type == WM_GRENADELAUNCHER )
                                    {
                                        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl )
                                            ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl = 0;
                                        else if ( ent->client->pers.nsPC.technical >= 5 ) {

                                            // disable bayonet if GL.. both causes trouble
                                            if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet )
                                                ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet = 0;

                                            ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl = 1;
                                        }
                                    }
                                    else {
                                        if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight )
                                            ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight = 0;
                                        else if ( ent->client->pers.nsPC.accuracy >= 4 )
                                            ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].lasersight = 1;
                                    }

                                    NS_OpenEquipmentMenu(ent);
                                    break;
                                }
                            case 6: // bayonet
                                if ( ! ( BG_WeaponMods(ent->client->pers.nsInven.primaryweapon ) & ( 1 << WM_BAYONET ) ) )
                                {
                                    NS_OpenEquipmentMenu(ent);
                                    return;
                                }
                                if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet = 0;
                                else if ( ent->client->pers.nsPC.strength >= 4 ) {
                                    // disable GL with bayonet... causes trouble
                                    if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl )
                                        ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].gl = 0;

                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.primaryweapon].bayonet = 1;
                                }
                                NS_OpenEquipmentMenu(ent);
                                break;
                            case 7: // silencer
                                if ( ! ( BG_WeaponMods(ent->client->pers.nsInven.secondaryweapon ) & ( 1 << WM_SILENCER ) ) )
                                {
                                    NS_OpenEquipmentMenu(ent);
                                    return;
                                }
                                if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer = 0;
                                else if ( ent->client->pers.nsPC.stealth >= 3 )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].silencer = 1;

                                NS_OpenEquipmentMenu(ent);
                                break;
                            case 8: // laser
                                if ( ! ( BG_WeaponMods(ent->client->pers.nsInven.secondaryweapon ) & ( 1 << WM_LASER ) ) )
                                {
                                    NS_OpenEquipmentMenu(ent);
                                    return;
                                }
                                if ( ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight = 0;
                                else if ( ent->client->pers.nsPC.accuracy >= 3 )
                                    ent->client->pers.nsInven.weapon_mods[ent->client->pers.nsInven.secondaryweapon].lasersight = 1;

                                NS_OpenEquipmentMenu(ent);
                                break;

                            case 9:
                                NS_OpenMainMenu(ent);
                            default:
                                break;
                            }
                        }


                        /*
                        ====================================
                        View Character
                        ====================================
                        */
                        void NS_OpenCharacterMenu ( gentity_t *ent )
                        {
                            char character_menu[512];

                            Com_sprintf( character_menu, sizeof ( character_menu ), "\"Here you can view your character\"  \"Experience Points : %i\" \"?\" \"Strength: %i\" \"Speed: %i\" \"Stamina: %i\" \"Accuracy: %i\" \"Stealth: %i\" \" \" \"1. Exit\" ", ent->client->pers.nsPC.xp,ent->client->pers.nsPC.strength,ent->client->pers.nsPC.speed,ent->client->pers.nsPC.stamina,ent->client->pers.nsPC.accuracy,ent->client->pers.nsPC.stealth );

                            NS_Menu(ent,character_menu,SLOTS_CHARACTER, MENU_CHARACTER);
                        }

                        void NS_HandleCharacterMenu ( gentity_t *ent, int menuSlot )
                        {
                            switch (menuSlot) {
                            case 1:
                                NS_OpenMainMenu( ent );
                                break;
                            default:
                                break;
                            }
                        }


                        /*
                        ====================================
                        Main Menu (select weaps/ammo/equip)
                        ====================================
                        */
                        void NS_OpenMainMenu ( gentity_t *ent )
                        {
                            char *main_menu = "\"Please Choose:\" \"?\" \"1. Primary Weapon\" \"2. Secondary Weapon\" \"3. Ammunition\" \"4. Equipment\" \"5. Modify Character Details\" \"6. Player Style\" \" \" \"7. Exit\"";

                            if ( g_gametype.integer < GT_TEAM )
                            {
                                return;
                            }
                            if (ent->client->sess.sessionTeam != TEAM_RED && ent->client->sess.sessionTeam != TEAM_BLUE)
                                return;

                            NS_Menu(ent,main_menu,SLOTS_MAIN, MENU_MAIN);
                        }

                        void NS_HandleMainMenu ( gentity_t *ent, int menuSlot )
                        {
                            switch (menuSlot) {
                            case 1: // 1st weapon
                                NS_OpenPrimaryWeaponMenu(ent);
                                break;
                            case 2: // 2nd weapon
                                NS_OpenSecondaryWeaponMenu(ent);
                                break;
                            case 3: // ammo
                                NS_OpenAmmoMenu(ent);	// !fixme
                                break;
                            case 4: // equipment
                                NS_OpenEquipmentMenu(ent); // !fixme
                                break;
                            case 5: // character view
                                NS_OpenCreateClassMenu(ent); // !fixme
                                break;
                            case 6: // exit
                                NS_OpenPlayerMenu(ent );
                                break;
                            default:
                                break;
                            }
                        }

                        /*
                        ====================================
                        Spectator Menu
                        ====================================
                        */

                        void NS_OpenSpectatorMenu ( gentity_t *ent )
                        {
                            char *spectator_menu = "\"#You are spectating!\" \"Remember that you can only chat with other spectators!\" \"Press your Attack key to open the Join menu again\" \" \"???\"1. Exit\"";

                            NS_Menu(ent,spectator_menu,SLOTS_SPECTATOR, MENU_SPECTATOR);
                        }


                        char* NS_GetNameForClass ( int class_num )
                        {
                            char *classname;

                            switch (class_num) {
                            case CLASS_RECON:
                                classname = "Recon";
                                break;
                            case CLASS_ASSAULT:
                                classname = "Assault";
                                break;
                            case CLASS_COMMANDER:
                                classname = "Commander";
                                break;
                            case CLASS_HEAVYSUPPORT:
                                classname = "Heavy Support";
                                break;
                            case CLASS_SNIPER:
                                classname = "Sniper";
                                break;
                            case CLASS_DEMOMAN:
                                classname = "Demolishions Expert";
                                break;
                            case CLASS_CUSTOM:
                                classname = "Custom Class";
                                break;
                            case CLASS_NONE:
                                classname = "no class";
                                break;
                            default:
                                classname = "UNKNOWN";
                                break;
                            }
                            return classname;
                        }

                        void SetClass ( gentity_t *ent, int des_class )
                        {
                            gitem_t *pri,*sec;


                            switch (des_class) {
                            case CLASS_RECON:
                                // set playerclass
                                ent->client->pers.nsPC.stealth = 3;
                                ent->client->pers.nsPC.strength = 1;
                                NS_GiveXP( ent, 666, qtrue );
                                ent->client->pers.nsPC.stamina = 2;
                                ent->client->pers.nsPC.speed = 2;
                                ent->client->pers.nsPC.accuracy = 1;
                                ent->client->pers.nsPC.technical = 2;
                                ent->client->pers.nsPC.playerclass = CLASS_RECON;
                                // set inventory

                                // primary weapon
                                if (ent->client->sess.sessionTeam == TEAM_RED)
                                    ent->client->pers.nsInven.primaryweapon = WP_MP5;
                                else
                                    ent->client->pers.nsInven.primaryweapon = WP_MAC10;

                                // secondary
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                else
                                    ent->client->pers.nsInven.secondaryweapon = WP_MK23;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );


                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 6;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 3;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = ARMOR_LIGHT;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = 0;
                                ent->client->pers.nsPC.style = 0;
                                break;
                            case CLASS_ASSAULT:
                                ent->client->pers.nsPC.stealth = 2;
                                ent->client->pers.nsPC.strength = 2;
                                ent->client->pers.nsPC.stamina = 2;
                                ent->client->pers.nsPC.speed = 2;
                                ent->client->pers.nsPC.accuracy = 2;
                                ent->client->pers.nsPC.technical = 1;
                                ent->client->pers.nsPC.playerclass = CLASS_ASSAULT;
                                NS_GiveXP( ent, 666, qtrue );
                                // set inventory
                                if ( ent->client->sess.sessionTeam == TEAM_RED )
                                    ent->client->pers.nsInven.primaryweapon = WP_MP5;
                                else
                                    ent->client->pers.nsInven.primaryweapon = WP_MAC10;

                                // give ammo
                                ent->client->pers.nsPC.style = 0;

                                // secondary
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                else
                                    ent->client->pers.nsInven.secondaryweapon = WP_MK23;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );


                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 6;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 3;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = ARMOR_MEDIUM;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = ARMOR_LIGHT;
                                break;
                            case CLASS_COMMANDER:
                                ent->client->pers.nsPC.stealth = 1;
                                ent->client->pers.nsPC.strength = 3;
                                ent->client->pers.nsPC.stamina = 2;
                                ent->client->pers.nsPC.speed = 1;
                                ent->client->pers.nsPC.accuracy = 2;
                                ent->client->pers.nsPC.technical = 2;
                                NS_GiveXP( ent, 666, qtrue );
                                ent->client->pers.nsPC.playerclass = CLASS_COMMANDER;
                                // set inventory

                                // primary weapon
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.primaryweapon = WP_AK47;
                                else
                                    ent->client->pers.nsInven.primaryweapon = WP_M4;

                                // secondary
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                else
                                    ent->client->pers.nsInven.secondaryweapon = WP_MK23;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 6;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 3;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = ARMOR_HEAVY;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = ARMOR_LIGHT;
                                ent->client->pers.nsPC.style = 0;
                                break;
                            case CLASS_HEAVYSUPPORT:
                                ent->client->pers.nsPC.stealth = 1;
                                ent->client->pers.nsPC.strength = 3;
                                ent->client->pers.nsPC.stamina = 3;
                                ent->client->pers.nsPC.speed = 2;
                                ent->client->pers.nsPC.accuracy = 2;
                                ent->client->pers.nsPC.technical = 2;
                                NS_GiveXP( ent, 666, qtrue );
                                ent->client->pers.nsPC.playerclass = CLASS_HEAVYSUPPORT;

                                // set inventory

                                // primary weapon
                                if (ent->client->sess.sessionTeam == TEAM_RED)
                                    ent->client->pers.nsInven.primaryweapon = WP_M4;
                                else
                                    ent->client->pers.nsInven.primaryweapon = WP_AK47;

                                // secondary
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                else
                                    ent->client->pers.nsInven.secondaryweapon = WP_MK23;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );


                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 5;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 4;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = ARMOR_HEAVY;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = ARMOR_LIGHT;
                                ent->client->pers.nsPC.style = 0;
                                break;
                            case CLASS_SNIPER:
                                ent->client->pers.nsPC.stealth = 2;
                                ent->client->pers.nsPC.strength = 1;
                                ent->client->pers.nsPC.stamina = 1;
                                ent->client->pers.nsPC.speed = 1;
                                ent->client->pers.nsPC.accuracy = 4;
                                ent->client->pers.nsPC.technical = 2;
                                NS_GiveXP( ent, 666, qtrue );
                                ent->client->pers.nsPC.playerclass = CLASS_SNIPER;
                                ent->client->pers.nsPC.style = 0;
                                // set inventory

                                // primary weapon
                                if ( ent->client->sess.sessionTeam == TEAM_RED )
                                    ent->client->pers.nsInven.primaryweapon = WP_MP5;
                                else
                                    ent->client->pers.nsInven.primaryweapon = WP_MAC10;

                                // secondary
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                else
                                    ent->client->pers.nsInven.secondaryweapon = WP_P9S;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 6;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 2;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = ARMOR_MEDIUM;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = 0;
                                ent->client->pers.nsPC.style = 0;
                                break;
                            case CLASS_DEMOMAN:
                                ent->client->pers.nsPC.stealth = 2;
                                ent->client->pers.nsPC.strength = 2;
                                ent->client->pers.nsPC.stamina = 2;
                                ent->client->pers.nsPC.speed = 1;
                                ent->client->pers.nsPC.accuracy = 2;
                                ent->client->pers.nsPC.technical = 3;
                                ent->client->pers.nsPC.playerclass = CLASS_DEMOMAN;
                                NS_GiveXP( ent, 666, qtrue );

                                // set inventory

                                // secondary


                                // primary weapon
                                if ( ent->client->sess.sessionTeam == TEAM_RED )
                                {
                                    ent->client->pers.nsInven.primaryweapon = WP_870;
                                    ent->client->pers.nsInven.secondaryweapon = WP_MK23;
                                }
                                else
                                {
                                    ent->client->pers.nsInven.primaryweapon = WP_M590;
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                }


                                ent->client->pers.nsPC.style = 0;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 6;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 3;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = ARMOR_MEDIUM;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = ARMOR_LIGHT;

                                break;
                            case CLASS_CUSTOM:
                                ent->client->pers.nsPC.xp = 0;
                                ent->client->pers.nsPC.entire_xp = 0;

                                // and we will reset all these values
                                ent->client->pers.nsPC.stealth = 1;
                                ent->client->pers.nsPC.strength = 1;
                                ent->client->pers.nsPC.stamina = 1;
                                ent->client->pers.nsPC.speed = 1;
                                ent->client->pers.nsPC.accuracy = 1;
                                ent->client->pers.nsPC.technical = 1;

                                if ( g_matchLockXP.integer )
                                {
                                    ent->client->pers.nsPC.stealth =
                                        ent->client->pers.nsPC.strength =
                                            ent->client->pers.nsPC.stamina =
                                                ent->client->pers.nsPC.speed =
                                                    ent->client->pers.nsPC.accuracy =
                                                        ent->client->pers.nsPC.technical = g_matchLockXP.integer;

                                    if ( ent->client->pers.nsPC.stealth > 5 )
                                        ent->client->pers.nsPC.stealth = 5;
                                } else 		// we will give him  xp
                                    NS_GiveXP( ent, NS_CalculateStartingXP( ent->client->sess.sessionTeam ) , qfalse);


                                // primary weapon
                                if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                                    ent->client->pers.nsInven.primaryweapon = WP_MAC10;
                                else
                                    ent->client->pers.nsInven.primaryweapon = WP_MP5;

                                // secondary
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                else
                                    ent->client->pers.nsInven.secondaryweapon = WP_P9S;

                                ent->client->pers.nsPC.style = 0;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 4;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 4;
                                ent->client->pers.nsInven.ammo[AM_GRENADES] = 1;
                                ent->client->pers.nsInven.ammo[AM_FLASHBANGS] = 1;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = 1;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = 0;

                                ent->client->pers.nsPC.playerclass = CLASS_CUSTOM;
                                break;
                            case CLASS_NONE: // this is more or less the same as custom - except you don't get any XPs at all
                            default:
                                ent->client->pers.nsPC.xp = 0;
                                ent->client->pers.nsPC.entire_xp = 0;
                                // we will give him  xp

                                // and we will reset all these values
                                ent->client->pers.nsPC.stealth = 1;
                                ent->client->pers.nsPC.strength = 1;
                                ent->client->pers.nsPC.stamina = 1;
                                ent->client->pers.nsPC.speed = 1;
                                ent->client->pers.nsPC.accuracy = 1;
                                ent->client->pers.nsPC.technical = 1;

                                if ( g_matchLockXP.integer )
                                {
                                    ent->client->pers.nsPC.stealth =
                                        ent->client->pers.nsPC.strength =
                                            ent->client->pers.nsPC.stamina =
                                                ent->client->pers.nsPC.speed =
                                                    ent->client->pers.nsPC.accuracy =
                                                        ent->client->pers.nsPC.technical = g_matchLockXP.integer;

                                    if ( ent->client->pers.nsPC.stealth > 5 )
                                        ent->client->pers.nsPC.stealth = 5;
                                } else 		// we will give him  xp
                                    NS_GiveXP( ent, NS_CalculateStartingXP( ent->client->sess.sessionTeam ) , qfalse);


                                // primary weapon
                                if ( ent->client->sess.sessionTeam == TEAM_BLUE )
                                    ent->client->pers.nsInven.primaryweapon = WP_MAC10;
                                else
                                    ent->client->pers.nsInven.primaryweapon = WP_MP5;

                                // secondary
                                if (ent->client->sess.sessionTeam == TEAM_BLUE)
                                    ent->client->pers.nsInven.secondaryweapon = WP_GLOCK;
                                else
                                    ent->client->pers.nsInven.secondaryweapon = WP_P9S;
                                ent->client->pers.nsPC.style = 0;

                                pri = BG_FindItemForWeapon( ent->client->pers.nsInven.primaryweapon );
                                sec = BG_FindItemForWeapon( ent->client->pers.nsInven.secondaryweapon );

                                ent->client->pers.nsInven.ammo[pri->giAmmoTag] = 4;
                                ent->client->pers.nsInven.ammo[sec->giAmmoTag] = 4;

                                // powerups
                                ent->client->pers.nsInven.powerups[PW_VEST] = 1;
                                ent->client->pers.nsInven.powerups[PW_HELMET] = 0;

                                ent->client->pers.nsPC.playerclass = CLASS_CUSTOM;
                                break;
                            }

                            // update character if anything changed - so knows about any changes
                            if ( ent->client->ps.persistant[PERS_STRENGTH] != ent->client->pers.nsPC.strength )
                                ent->client->ps.persistant[PERS_STRENGTH] = ent->client->pers.nsPC.strength;

                            if ( ent->client->ps.persistant[PERS_TECHNICAL] != ent->client->pers.nsPC.technical )
                                ent->client->ps.persistant[PERS_TECHNICAL] = ent->client->pers.nsPC.technical;

                            if ( ent->client->ps.persistant[PERS_STAMINA] != ent->client->pers.nsPC.stamina )
                                ent->client->ps.persistant[PERS_STAMINA] = ent->client->pers.nsPC.stamina;

                            if ( ent->client->ps.persistant[PERS_SPEED] != ent->client->pers.nsPC.speed )
                                ent->client->ps.persistant[PERS_SPEED] = ent->client->pers.nsPC.speed;

                            if ( ent->client->ps.persistant[PERS_STEALTH] != ent->client->pers.nsPC.stealth )
                                ent->client->ps.persistant[PERS_STEALTH] = ent->client->pers.nsPC.stealth;

                            if ( ent->client->ps.persistant[PERS_ACCURACY] != ent->client->pers.nsPC.accuracy )
                                ent->client->ps.persistant[PERS_ACCURACY] = ent->client->pers.nsPC.accuracy;

                            // send xp
                            trap_SendServerCommand( ent-g_entities, va("uxp %i", ent->client->pers.nsPC.xp ) );
                        }

                        /*
                        =================
                        NSQ3 Direct Menuselect
                        author: Defcon-X
                        date: 24-08-2000
                        description: directly selects a menu.
                        =================
                        */
                        void NS_DirectMenuSelect( gentity_t *ent )
                        {
                            char	str[MAX_TOKEN_CHARS];
                            int		menu,menuSlot;

                            if ( trap_Argc () < 2 ) {
                                PrintMsg( ent, "Usage:\ncmd dms <menunum> <slotnum>\n");
                                return;
                            }

                            trap_Argv( 1, str, sizeof( str ) );
                            menu = atoi( str );

                            trap_Argv( 2, str, sizeof( str ) );
                            menuSlot = atoi( str );

                            if (!menu || !menuSlot) {
                                PrintMsg( ent, "Usage:\ncmd dms <menunum> <slotnum>\n");
                                return;
                            }

                            if ( menu >= MENU_MAX ) {
                                PrintMsg( ent, "<menunum> out of range\n");
                                return;
                            }

                            NS_CallMenu( ent, menu, menuSlot );
                        }
                        /*
                        =================
                        NSQ3 Kill Menu
                        author: Defcon-X
                        date: 24-08-2000
                        description: directly selects a menu.
                        =================
                        */
                        void NS_KillMenu( gentity_t *ent )
                        {

                        }
