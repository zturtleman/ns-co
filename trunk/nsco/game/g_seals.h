// Copyright (C) 1999-2000 Team Mirage / dX
//

/*****************************************************************************
* name:		general_ns.h
*
* desc:		defines and stuff for NS
*
* -Archive: /source/game/general_ns.h
* -Author: dX -
* -Revision: 1
* -Modtime: 14/feb/2k 3:32p (last modification)
* -Date: 08/feb/2k (creation)
*
*****************************************************************************/

// used as return value in g_combat.c / general_ns.c
// i also use to see wheter a player (client) bleeds at a location
// if (ent->client->bleeding[LOC_HEAD] == qtrue)
// understood?

#define LOC_NULL			0
#define LOC_HEAD			1
#define LOC_FACE			2
#define LOC_CHEST			3
#define LOC_STOMACH			4
#define LOC_BACK			5
#define LOC_RIGHTARM		6
#define LOC_LEFTARM			7
#define LOC_RIGHTLEG		8
#define LOC_LEFTLEG			9
#define LOC_BLEED			10
#define LOC_THROAT    11
#define LOC_KNEE      12

#define DEBUG_BUILD 0

#define ARMOR_NONE			0
#define ARMOR_LIGHT			1
#define ARMOR_MEDIUM		2
#define ARMOR_HEAVY			3

#define CLASS_NONE			0
#define	CLASS_RECON			1
#define CLASS_ASSAULT		2
#define CLASS_COMMANDER		3
#define CLASS_HEAVYSUPPORT	4
#define CLASS_SNIPER		5
#define CLASS_DEMOMAN		6
#define CLASS_CUSTOM		7

#define VIP_NONE			0
#define VIP_ESCAPE			1
#define VIP_STAYALIVE		2

#define RESPAWN_INVUNERABILITY_TIME 4500

// could make the code more readable
#define ONE_SECOND	1000

#define BANDAGE_MINTIME		ONE_SECOND

#define	REWARD_NONE					0x00000000

#define	REWARD_VIP_KILL				0x00000001
#define	REWARD_VIP_ALIVE			0x00000002

#define REWARD_BRIEFCASE_CAPTURE	0x00000004

#define	REWARD_ASSAULT_TAKEN		0x00000100
#define REWARD_2ASSAULT_TAKEN		0x00000200
#define REWARD_3ASSAULT_TAKEN		0x00000400
#define REWARD_4ASSAULT_TAKEN		0x00000800

#define	REWARD_2KILL				0x00001000
#define	REWARD_4KILL				0x00002000
#define	REWARD_6KILL				0x00004000
#define	REWARD_2KILL_GRENADE		0x00010000
#define	REWARD_4KILL_GRENADE		0x00020000

#define	REWARD_HS_KILL				0x00040000
#define	REWARD_HS_2KILL				0x00080000

#define	REWARD_BOMB_EXPLODE			0x00100000
#define	REWARD_BOMB_DEFUSE			0x00200000
#define REWARD_BRIEFCASE_KILL		0x00400000
#define REWARD_BRIEFCASE_2ND		0x00800000

// cvars
extern	vmCvar_t	g_keepCharacter; // keeping the character over multiple games?
extern	vmCvar_t	g_overrideGoals;
extern	vmCvar_t	g_aimCorrect;
extern  vmCvar_t	g_invenTime;
extern  vmCvar_t	g_bombTime;
extern	vmCvar_t	g_riflemod;
extern	vmCvar_t	g_pistolmod;
extern	vmCvar_t	g_snipermod;
extern	vmCvar_t	g_silentBullets;
extern	vmCvar_t	g_realLead;
extern	vmCvar_t	g_noPrimary;
extern	vmCvar_t	g_noSecondary;
extern	vmCvar_t	g_noGrenades;
extern	vmCvar_t	g_allowMapVote;
extern	vmCvar_t	g_allowKickVote;
extern	vmCvar_t	g_allowTimelimitVote;
extern	vmCvar_t	g_allowTeampointlimitVote;
extern	vmCvar_t	g_updateServerInfoTime;
extern	vmCvar_t	g_TeamPlayers;
extern	vmCvar_t	g_TeamScores;
extern	vmCvar_t	g_TeamKillRemoveTime;
extern	vmCvar_t	g_firstCountdown;
extern	vmCvar_t	g_mapcycle;
extern	vmCvar_t	g_matchLockXP;
extern	vmCvar_t	g_LockXP;


extern int GameState;
extern int	lastvip[TEAM_NUM_TEAMS];


////////////////////////////////////////////////////////////////////////////////////////
//
// antilag code, bulletprediction
//
////////////////////////////////////////////////////////////////////////////////////////
#define MAX_SERVER_FPS		40
#define MAX_ANTILAG			MAX_SERVER_FPS

// Antilag information
typedef struct gantilag_s
{
    vec3_t	rOrigin;				// entity.r.currentOrigin
    vec3_t	rAngles;				// entity.r.currentAngles
    vec3_t	mins;					// entity.r.mins
    vec3_t	maxs;					// entity.r.maxs

    int		time;					// time history item was saved
    int		leveltime;

    int		pm_flags;				// entity.client.ps.pm_flags

} gantilag_t;

// misc structurs / stuff
typedef enum {false,true} boolean;
// functions

////////////////////////////////////////////////////////////////////////////////////////
//
// Radio Code
//
////////////////////////////////////////////////////////////////////////////////////////

typedef struct radio_msg_s
{
    char soundAlias[128];      // the msg name [alias]

    int soundIndex; //  sound index, for clientsystem
    int soundLength; // length in server frames (thousand of a second), rounded up
    char chatString[256]; // chat message
    int	signalType;
} radio_msg_t;

#define RADIO_CLICK                     "sound/radio/click.wav"
#define RADIO_CLICK_LEN                 200

#define MAX_SOUNDFILE_PATH_LEN          32 // max length of a sound file path
#define MAX_RADIO_MSG_QUEUE_SIZE        4
#define MAX_RADIO_QUEUE_SIZE            6  // this must be at least 2 greater than the above

// SEALS - RADIO [org: fireblade]
typedef struct radio_queue_entry_s
{
    int soundIndex;
    gentity_t *from_player;
    qboolean now_playing;
    int length;
    int	gesture;
    char chatString[256];
    qboolean click;
} radio_queue_entry_t;

typedef struct navyseals_s navyseals_t;

////////////////////////////////////////////////////////////////////////////////////////
//
// NS:CO playerinfo structure - cleared on each respawn
//
////////////////////////////////////////////////////////////////////////////////////////
typedef struct navyseals_s{
    int rounds[WP_NUM_WEAPONS*3];	// hack...
    int weaponmode[WP_NUM_WEAPONS];
    int	reload_tries;
    int weaponmode_tries[3];
    int	shots;
    int	weigth;
    int opendoor;
    int	bandagepoints;
    qboolean is_driving;
    int	num_killed;
    int	rewards;

    qboolean is_vip;
    qboolean is_vipWithBriefcase;
    qboolean got_defusekit;

    gentity_t	*lasersight;
    int	locationOfDeath;

    int			weapon_angles[3];	// add to command angles to get view direction

    int radio_delay;
    int	radio_power_off;
    radio_queue_entry_t radio_queue[MAX_RADIO_QUEUE_SIZE];
    int radio_queue_size;
    int radio_channel;

    gentity_t *bomb_parent;
    gentity_t *bomb_world; // placed bomb
    int		   bomb_wireTime;
    int		   bomb_wires[8];

    qboolean	test;
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  headgear/playermodel handling
//
///////////////////////////////////////////////////////////////////////////////////////
#define MAX_HEADGEAR		64
#define MAX_FACESKINS		64
#define MAX_PLAYERMODELS	4

typedef struct heargear_s headgear_t;

typedef struct heargear_s{
    char	e_head[MAX_HEADGEAR][MAX_QPATH],
    e_eyes[MAX_HEADGEAR][MAX_QPATH],
    e_mouth[MAX_HEADGEAR][MAX_QPATH];

    char	playerModel[MAX_PLAYERMODELS][MAX_QPATH];
    char	faceSkin[MAX_FACESKINS][MAX_QPATH];

    int		numHead;
    int		numEyes;
    int		numMouth;
    int		numPPM;
    int		numFaceSkin;
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  Prototypes
//
///////////////////////////////////////////////////////////////////////////////////////

void NS_SetGameState( int state );
void NS_TMequip( gentity_t *me );
void NS_Itsame( gentity_t *me );

void Pickup_Briefcase( gentity_t *ent, gentity_t *other );
void PrecacheRadioSounds();
void RadioThink(gentity_t *ent);
void Cmd_Radioteam_f(gentity_t *ent);
void Cmd_Radio_power_f(gentity_t *ent);
void RadioBroadcast(gentity_t *ent, char *msg, qboolean useinput);
void Weapon_C4 ( gentity_t *ent );
qboolean PI_CheckWeapon ( int team , int weapon, int acc, int str, int sta, int stl, qboolean stolen );
qboolean Is_Spectator(gentity_t *ent);
qboolean NS_IsBot(gentity_t *ent);
qboolean Is_OnBrushTeam(gentity_t *brush, gentity_t *client);
qboolean NS_GotPowerup ( gentity_t *ent, int powerup );
qboolean NS_CanShotgunBlowHead( gentity_t *targ, gentity_t *attacker, int weapon );
int NS_IsVipAlive( int ignoreClientNum, team_t team );
qboolean BG_GotSecondary( const playerState_t *ps );

// Game Stuff
void Fire_Lead ( gentity_t *ent, int caliber , int damage );
void NS_CheckRemoveTeamKill( gentity_t *ent );
void G_RunActor( gentity_t *ent );
void NS_SetPrimary( gentity_t *ent, int primary );
void NS_SetSecondary( gentity_t *ent, int secondary );
void Cmd_Reload_f (gentity_t *ent);
void NS_SprayBlood (gentity_t *ent, vec3_t start, vec3_t dir, int damage, qboolean brain );
void NS_CauseBleeding(gentity_t *ent);
void NS_NavySeals_ClientInit(gentity_t *ent, qboolean roundbegin);
void NS_Bandaging (gentity_t *ent);
void NS_StartBandage (gentity_t *ent);
void NS_DropWeapon (gentity_t *ent);
void NS_RemoveItems ();
void NS_ClearInventory (gentity_t *ent);
void NS_WonRound( team_t team );
void NS_GiveXP ( gentity_t *ent , int num, qboolean take );
void NS_WeaponMode ( gentity_t *ent, int mode );
void NS_EndRound ();
void Reset_Briefcase();
void NS_HolsterWeapon( gentity_t *ent );
void NS_OpenDoor ( gentity_t *ent, qboolean direct );
void NS_EndRoundForTeam ( int team );
void NS_Gesture ( gentity_t *ent );
void NS_AdjustClientVWeap( int weaponmode, int powerups[ ] );
void NS_DirectMenuSelect( gentity_t *ent );
void G_LocalSound( gentity_t *ent, int channel, int soundIndex );
void NS_KillMenu( gentity_t *ent );
void Pick_Item (gentity_t *ent, gentity_t *other, trace_t *trace );
void NS_SpawnFlare( gentity_t *ent );
void NS_PlayerInventory( gentity_t *ent );
void NS_RaiseCharacterLevel( gentity_t *ent );
void G_SetupServerInfo( void );
void NS_InitHeadBBoxes( void );
void NS_ModifyClientBBox( gentity_t *ent );
void assault_field_stopall( );
qboolean NS_InitMapCycle( void );
char *NS_GetNextMap( void );
char *NS_GetPrevMap( void );
float	NS_GetAccuracy(gentity_t *ent, int spread);
float BG_MaximumWeaponRange( int weapon );

int NS_CheckLocationDamage(gentity_t *targ,vec3_t point, int mod);
int NS_GetRounds( gentity_t *ent );
int BG_GetMaxRoundForWeapon( int weapon );
int NS_GotWounds( gentity_t *ent );
int NS_BulletHoleTypeForSurface( int surface );
int NS_CalcDamageOnRange( vec3_t start, vec3_t end, int damage, int attackerweapon );
int NS_CalculateStartingXP( int team );
int AliveTeamCount( int ignoreClientNum, team_t team );
float NS_CalcSpeed( gentity_t *ent );
qboolean NS_BombExistForTeam ( team_t team );
qboolean NS_GotEnoughXPfornextLevel ( gentity_t *ent , int cur_level );
//bomb functions
void bomb_checkremovewire( gentity_t *ent );
void bomb_defused( gentity_t *ent,  gentity_t *activator );
void bomb_explode_instantly ( gentity_t *ent );
void bomb_drop( gentity_t *ent );
void bomb_explode_instantly ( gentity_t *ent );
void bomb_explode( gentity_t *ent );


qboolean NS_CheckEndRound( );
float NS_CalcWeight(gentity_t *ent);

void CheckTeamplay ();

// remove me later
void Laser_Gen(gentity_t *ent, int type);

// Entities
void Touch_Goal (gentity_t *ent, gentity_t *other, trace_t *trace);
void Think_Goal (gentity_t *self);
void Door_ResetState ( gentity_t *door );
void TriggerToggle_ResetState ( gentity_t *door );
void DoorRotating_ResetState ( gentity_t *door );

qboolean is_func_explosive(gentity_t *ent);
qboolean	pointinback( gentity_t *ent, vec3_t origin );
// Menu's
void NS_MenuSelect (gentity_t *ent);
void NS_OpenTeamMenu (gentity_t *ent);
void NS_OpenClassMenu (gentity_t *ent);
void NS_OpenMainMenu (gentity_t *ent);
void NS_OpenRadioMenu(gentity_t *ent);
void NS_OpenPrimaryWeaponMenu ( gentity_t *ent );
void NS_OpenSpectatorMenu ( gentity_t *ent );
void NS_OpenMainMenu ( gentity_t *ent );
void NS_OpenClassMenu ( gentity_t *ent );
void NS_OpenEquipmentMenu ( gentity_t *ent );
void NS_OpenCreateClassMenu ( gentity_t *ent );
void NS_OpenPlayerMenu ( gentity_t *ent );

void SetClass( gentity_t *ent, int des_class );

void NS_HandlePrimaryWeaponMenu ( gentity_t *ent, int menuSlot );
void NS_HandleTeamMenu ( gentity_t *ent, int menuSlot );
void NS_HandleSecondaryWeaponMenu ( gentity_t *ent, int menuSlot );
void NS_HandleMainMenu (gentity_t *ent, int menuSlot );
void NS_HandleAmmoMenu ( gentity_t *ent, int menuSlot );
void NS_HandleCharacterMenu ( gentity_t *ent, int menuSlot );
void NS_HandleClassMenu ( gentity_t *ent, int menuSlot );
void NS_HandleRadioMenu ( gentity_t *ent, int menuSlot );
void NS_HandleEquipmentMenu ( gentity_t *ent, int menuSlot );
void NS_HandleCreateClassMenu ( gentity_t *ent, int menuSlot );
void NS_HandlePlayerMenu ( gentity_t *ent, int menuSlot );

// Miscs
gentity_t	*SelectTeamSpawnPoint ( team_t team, int teamstate, vec3_t origin, vec3_t angles );
gentity_t	*NS_FindRadius (gentity_t *ent, vec3_t org, float rad);
gentity_t	*fire_smoke_grenade (gentity_t *self, vec3_t start, vec3_t dir);
gentity_t	*Drop_Weapon( gentity_t *ent, gitem_t *item, float angle, int clips );
qboolean	NS_IsBot(gentity_t *ent);
char		*NS_GetNameForClass( int class_num );
void		SetClass(gentity_t *ent, int des_class );

// id Predefines / id hadn't got it somewhere
void G_ExplodeMissile(gentity_t *ent);
void CopyToBodyQue (gentity_t *ent);
void BodySink (gentity_t *ent);
void QDECL PrintMsg( gentity_t *ent, const char *fmt, ... );
void G_Say ( gentity_t *ent, gentity_t *target, int mode, const char * chatText );
void CenterPrintAll(char *message);
void PrintMsg (gentity_t *ent, const char *fmt, ... );
void Use_BinaryMover2( gentity_t *ent, gentity_t *other, gentity_t *activator );
void Use_BinaryMover ( gentity_t *ent, gentity_t *other, gentity_t *activator );
void NS_PlayerAnimation(gentity_t *ent, int anim, int timer, qboolean legs );
void QDECL PrintMsgToAllAlive( qboolean toallwaiting, const char *fmt, ... );
void NS_SetClientCrosshairState ( gentity_t *ent );
void NS_GiveReward ( gentity_t *ent, int reward, int team );
void NS_TeamKill( gentity_t *killer, gentity_t *targ );
void NS_FreeXP( gentity_t *ent );

void G_UpdateClientAntiLag	( gentity_t* ent );
void G_UndoAntiLag			( void );
void G_ApplyAntiLag			( gentity_t* ref );

void NS_CalculateRadar( gentity_t *ent );
void NS_ValidatePlayerLooks( int team, char *headGear,
                             char *eyeGear,
                             char *mouthGear,
                             char *playerModel,
                             char *faceSkin );
void NS_RecalcCharacter( gentity_t *ent );
void Think_SetupTrainTargets( gentity_t *ent );
qboolean NS_ActiveRound( void );
char *NS_GetMapName( void );

qboolean NS_InitHeadGear( void );

void NS_SendMessageToTeam( int team, char *message );

void NS_FixHitboxes( void );
void NS_RestoreHitboxes( void );
int NS_GetTime( void );
void ElevatorReset( gentity_t *ent );
