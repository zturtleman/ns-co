
// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#define SL8SD 1

#define GAME_VERSION            "seals-1.93"

#define	DEFAULT_GRAVITY		800
#define	GIB_HEALTH			-40

#define MAX_BULLET_HITS		3	// max people the bullet can hit
#define MAX_BULLET_SHOTS	3	// how many times through walls
#define MAX_BULLET_THICKN	4	// how big a wall can be before it won't go through

#define MAX_9MM_BULLET_THICKN 10 // 9mm
#define MAX_7MM_BULLET_THICKN 25 // 7.62mm
#define MAX_50CAL_BULLET_THICKN 25 // .50cal
#define MAX_556MM_BULLET_THICKN	20

#define SPAS15_SPREAD	45
#define SPAS15_PELLETS	8

#define M590_SPREAD		80
#define M590_PELLETS	8

#define M870_SPREAD		80
#define M870_PELLETS	8

#define DUCKBILL_HOR_MOD	3
#define DUCKBILL_VERT_DIV	1

#define	MAX_ITEMS			256

#define	RANK_TIED_FLAG		0x4000

//#define DEFAULT_SHOTGUN_SPREAD	700
//#define DEFAULT_SHOTGUN_COUNT	11

#define	ITEM_RADIUS			10		// item sizes are needed for client side pickup detection

#define	LIGHTNING_RANGE		768

#define	SCORE_NOT_PRESENT	-9999	// for the CS_SCORES[12] when only one player is present

#define	VOTE_TIME			30000	// 30 seconds before vote times out

#define MAXS_Z				31
#define	MINS_Z				-24
#define	DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12
#define	DEAD_VIEWHEIGHT		-16

#define LEN_SCORE_STRING	6		// length of a seperate playerinfo string that is includedin the scoreboard message.
//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
#define	CS_MUSIC				2
#define	CS_MESSAGE				3		// from the map worldspawn's message field
#define	CS_MOTD					4		// g_motd string for server message of the day
#define	CS_WARMUP				5		// server time when the match will be restarted
#define	CS_SCORES1				6
#define	CS_SCORES2				7
#define CS_VOTE_TIME			8
#define CS_VOTE_STRING			9
#define	CS_VOTE_YES				10
#define	CS_VOTE_NO				11

#define CS_TEAMVOTE_TIME		12
#define CS_TEAMVOTE_STRING		14
#define	CS_TEAMVOTE_YES			16
#define	CS_TEAMVOTE_NO			18

#define	CS_GAME_VERSION			20
#define	CS_LEVEL_START_TIME		21		// so the timer only shows the current level
#define	CS_INTERMISSION			22		// when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
#define CS_FLAGSTATUS			23		// string indicating flag status in CTF
#define CS_SHADERSTATE			24
#define CS_BOTINFO				25

#define	CS_ITEMS				27		// string of 0's and 1's that tell which items are present
// Navy Seals ++
#define CS_ROUND_START_TIME		28		// round start time
#define CS_VIP_START_TIME		29
#define CS_BOMB_START_TIME		30
#define CS_SEALINFO				31		// contains misc information like lastmap similar to CS_SYSTEMINFO or CS_SERVERINFO
// Navy Seals --
#define CS_ASSAULT_START_TIME	32		// vip time
#define CS_ASSAULT2_START_TIME	33
#define CS_ASSAULT3_START_TIME	34
#define CS_ASSAULT4_START_TIME	35 // max of 4 assaultfields

#define	CS_MODELS				36

#define	CS_SOUNDS				(CS_MODELS+MAX_MODELS)
#define	CS_PLAYERS				(CS_SOUNDS+MAX_SOUNDS)
#define CS_LOCATIONS			(CS_PLAYERS+MAX_CLIENTS)

#define CS_MAX					(CS_LOCATIONS+MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

typedef enum {
    GT_FFA,				// free for all
    //-- team games go after this --

    GT_TEAM,			// team deathmatch

    GT_TRAIN,
    // Navy Seals ++
    GT_LTS,				// shit

    // Navy Seals --
    GT_MAX_GAME_TYPE
} gametype_t;

typedef enum { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER } gender_t;

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a new player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

typedef enum {
    PM_NORMAL,		// can accelerate and turn
    PM_NOCLIP,		// noclip movement
    PM_SPECTATOR,	// still run into walls
    PM_DEAD,		// no acceleration or turning, but free falling
    PM_FREEZE,		// stuck in place with no control
    PM_INTERMISSION,	// no movement or status bar
    PM_SPINTERMISSION	// no movement or status bar
} pmtype_t;
// Navy Seals ++
typedef enum {
    WEAPON_READY,				// idling
    WEAPON_RAISING,				//	raise the gun
    WEAPON_DROPPING,				// holster the gun
    WEAPON_FIRING,				// fire the gun
    WEAPON_FIRING2,
    WEAPON_FIRING3,
    WEAPON_FIREEMPTY,			// firing an empty gun
    WEAPON_RELOADING,			// start reloading
    WEAPON_RELOADING_CYCLE,		// loading in (hand up / hand down)
    WEAPON_RELOADING_STOP,		// putting shotgun down
    WEAPON_RELOADING_EMPTY,		// reloading an empty gun (not required)
    WEAPON_LASTRND,			// firing the last rnd (then moving to IDLE_EMPTY)
    WEAPON_BANDAGING_START,		// wepaon moving down
    WEAPON_BANDAGING_END,		// weapon moving up
    WEAPON_BANDAGING,			// in bandage
    WEAPON_MELEE,
    WEAPON_THROW,
    WEAPON_FIRING21,
    WEAPON_FIRING22,
    WEAPON_FIRING23,
    WEAPON_IDLEMODE2,
    WEAPON_HOLSTERING			// same as weapon_dropping
} weaponstate_t;
// Navy Seals --
// pmove->pm_flags
#define	PMF_DUCKED			1
#define	PMF_JUMP_HELD		2
#define	PMF_BACKWARDS_JUMP	8		// go into backwards land
#define	PMF_BACKWARDS_RUN	16		// coast down to backwards run
#define	PMF_TIME_LAND		32		// pm_time is time before rejump
#define	PMF_TIME_KNOCKBACK	64		// pm_time is an air-accelerate only time
#define	PMF_TIME_WATERJUMP	256		// pm_time is waterjump
#define	PMF_RESPAWNED		512		// clear after attack and jump buttons come up
#define	PMF_BOMBRANGE			1024
#define PMF_FOLLOW			4096	// spectate following another player
#define PMF_SCOREBOARD		8192	// spectate as a scoreboard
#define PMF_BOMBCASE		16384	// bombcase in hand
// Navy Seals ++
#define PMF_BLOCKED		128		// is in range of a bomb
#define PMF_SHOT_LOCKED		4
#define PMF_ASSAULTRANGE	32768
#define PMF_CLIMB			2048	// climbing up / down a ladder
// Navy Seals --
#define	PMF_ALL_TIMES	(PMF_TIME_WATERJUMP|PMF_TIME_LAND|PMF_TIME_KNOCKBACK)

#define	MAXTOUCH	32
typedef struct {
    // state (in / out)
    playerState_t	*ps;

    // command (in)
    usercmd_t	cmd;
    int			tracemask;			// collide against these types of surfaces
    int			debugLevel;			// if set, diagnostic output will be printed
    qboolean	noFootsteps;					// if the game is setup for no footsteps by the server
    qboolean	gauntletHit;					// true if a gauntlet attack would actually hit something

    int			framecount;

    // results (out)
    int			numtouch;
    int			touchents[MAXTOUCH];

    vec3_t		mins, maxs;			// bounding box size

    int			watertype;
    int			waterlevel;

    float		xyspeed;

    // for fixed msec Pmove
    int			pmove_fixed;
    int			pmove_msec;

    // callbacks to test the world
    // these will be different functions during game and cgame
    void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask );
    int			(*pointcontents)( const vec3_t point, int passEntityNum );
} pmove_t;

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================


// player_state->stats[] indexes
// NOTE: may not have more than 16
typedef enum {
    // Navy Seals ++
    STAT_HEALTH,
    STAT_WEAPONS, 					// 16 bit fields
    STAT_WEAPONS2,

    STAT_CLIENTS_READY, 			// bit mask of clients wishing to exit the intermission (FIXME: configstring?)

    STAT_ARM_DAMAGE, 				// arm damage
    STAT_LEG_DAMAGE,				// player walks slower when he got some legdamage,
    STAT_CHEST_DAMAGE,				// chest damage
    STAT_STOMACH_DAMAGE,			// stomach damage
    STAT_HEAD_DAMAGE,

    STAT_WEAPONMODE,				// current weapon mode

    STAT_STAMINA,					// stamina
    STAT_STEALTH,					// stealth
    STAT_ACTIVE_ITEMS,					// which powerups are active?

    STAT_ROUNDS,					// seals define, for rounds in gun

    STAT_SEED						// for antilag
    // Navy Seals --
} statIndex_t;


// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
typedef enum {
    PERS_XP,	// i changed this so the launchers/serverbrowsers show the XP instead of the kills
    PERS_SCORE, // !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
    PERS_RANK,	// player rank or team rank
    PERS_TEAM,	// player team
    PERS_SPAWN_COUNT, // incremented every respawn
    PERS_PLAYEREVENTS,	// 16 bits that can be flipped for events
    // player character
    PERS_STRENGTH,
    PERS_TECHNICAL,
    PERS_STAMINA,
    PERS_ACCURACY,
    PERS_SPEED,
    PERS_STEALTH
    // Navy Seals --
} persEnum_t;


//#define AWARDS

// entityState_t->eFlags
//#ifdef MISSIONPACK
//#define EF_TICKING			0x00000002		// used to make players play the prox mine ticking sound
//#endif

#ifdef AWARDS
#define	EF_AWARD_EXCELLENT	0x00000008		// draw an excellent sprite
#define	EF_AWARD_IMPRESSIVE	0x00008000		// draw an impressive sprite
#define	EF_AWARD_DEFEND		0x00010000		// draw a defend sprite
#define	EF_AWARD_ASSIST		0x00020000		// draw a assist sprite
#define EF_AWARD_DENIED		0x00040000		// denied
#define	EF_AWARD_GAUNTLET	0x00000040		// draw a gauntlet sprite
#define EF_AWARD_CAP		0x00000800		// draw the capture sprite
#endif


#define	EF_DEAD				0x00000001		// don't draw a foe marker over players with EF_DEAD
#define EF_REDGLOW			0x00000002
#define	EF_TELEPORT_BIT		0x00000004		// toggled every time the origin abruptly changes
#define EF_SILENCED			0x00000008		// uses a silenced weapon?

#define	EF_BOUNCE			0x00000010		// for missiles
#define	EF_BOUNCE_HALF		0x00000020		// for missiles
#define	EF_NODRAW			0x00000080		// may have an event, but no model (unspawned items)

#define	EF_FIRING			0x00000100		// for lightning gun
#define EF_LASERSIGHT		0x00000200		// using lasersight
#define	EF_MOVER_STOP		0x00000400		// will push otherwise
#define EF_VIP				0x00000800		// VIP

#define	EF_TALK				0x00001000		// draw a talk balloon
#define	EF_CONNECTION		0x00002000		// draw a connection trouble sprite
#define	EF_VOTED			0x00004000		// already cast a vote
#define EF_WEAPONS_LOCKED	0x00008000		// will lock weapons

#define EF_RADIO_TALK		0x00010000
#define EF_WEAPONMODE3		0x00020000
#define EF_IRONSIGHT		0x00040000		// denied
#define EF_TEAMVOTED		0x00080000		// already cast a team vote

//#define EF_HEADBLOW			0x02000000		// headblow
// Navy Seals --
// Navy Seals ++
// NOTE: may not have more than 16
typedef enum {
    PW_NONE,

    PW_SCOPE,
    PW_M203GL,
    PW_BAYONET,
    PW_FLASHLIGHT,
    PW_DUCKBILL,

    // navy seals +
    PW_VEST,
    PW_HELMET,

    PW_BRIEFCASE,
    PW_BRIEFCASE_RETURN,
    PW_LASERSIGHT, // hack
    // navy seals -
    PW_NVG,


    PW_NUM_POWERUPS

} powerup_t;

typedef enum {
    UI_NONE,

    // navy seals -
    UI_NVG,


    UI_NUM_ITEMS

} useableItems_t;


typedef enum {
    HI_NONE,

    HI_TELEPORTER,
    HI_MEDKIT,
    HI_KAMIKAZE,
    HI_PORTAL,
    HI_INVULNERABILITY,

    HI_NUM_HOLDABLE
} holdable_t;

/*
#define WP_NONE			0
#define WP_KHURKURI		1
#define	WP_SEALKNIFE	2
#define WP_C4			3
#define WP_GRENADE		4
#define	WP_FLASHBANG	5
#define WP_MK23			6
#define WP_GLOCK		7
#define	WP_SW40T		8
#define	WP_P9S			9
#define WP_DEAGLE   10
#define WP_SW629    11
#define	WP_PDW			12
#define WP_MAC10		13
#define WP_MP5			14
#define	WP_AK47			15
#define	WP_M4			  16
#define	WP_PSG1			17
#define WP_MACMILLAN	18
#define	WP_870			19
#define	WP_M590			20
#define WP_SPAS15		21
#define WP_M14      22
#define WP_M249     23
#define WP_SL8SD    24
#define WP_SMOKE    25
#define WP_NUM_WEAPONS	26
*/

/*
SYNC THIS CRAP WITH ../../ui/menudef.h !!!!
*/
typedef enum {
    WP_NONE, // 0

    // ns
    WP_KHURKURI,
    WP_SEALKNIFE, // must be first

    // explosives
    WP_C4,
    WP_GRENADE,
    WP_FLASHBANG,

    // pistols
    WP_MK23,
    WP_GLOCK,
    WP_SW40T,
    WP_P9S,
    WP_DEAGLE,
    WP_SW629,

    // smgs
    WP_PDW,
    WP_MAC10,
    WP_MP5,

    // rifles
    WP_AK47,
    WP_M4,

    // sniper rifles
    WP_PSG1,
    WP_MACMILLAN,

    // machineguns
    // grenadelaunchers

    // shotguns
    WP_870,
    WP_M590,
    // assault shotguns
    WP_SPAS15,

    WP_M14,
    WP_M249,
    WP_SL8SD,

    WP_SMOKE,

    // misc
    WP_NUM_WEAPONS
} weapon_t;

// weaponmodes
typedef enum {
    WM_NONE,

    WM_BURST,
    WM_SINGLE,
    WM_SCOPE,
    WM_LASER, // determines wheter we got one attached or not
    WM_SILENCER,
    WM_GRENADEROLL, //
    WM_DUCKBILL,    // db attached
    WM_FLASHLIGHT,  // fl attached
    WM_GRENADELAUNCHER, // gl attached

    // --- //

    WM_BAYONET,
    WM_WEAPONMODE2,	// for m4,ak47(gl/bayonet),shotgun:870,m590 (flashlight)
    WM_LACTIVE,	// determines wheter the lasersight is active or not
    WM_ZOOM2X,		// zoom...
    WM_ZOOM4X,

    WM_MUZZLEHIDER,

    // --- //

} weaponmode_t;

typedef enum {

    // shotgun shells
    AM_SHOTGUN,
    AM_SHOTGUNMAG,

    // pistol
    AM_LIGHT_PISTOL,
    AM_MEDIUM_PISTOL,
    AM_LARGE_PISTOL,

    // other weapons / smg / rifle
    AM_SMG,
    AM_RIFLE,
    AM_MG,
    AM_MEDIUM_SNIPER,
    AM_LARGE_SNIPER,

    // grenades
    AM_GRENADES,
    AM_FLASHBANGS,
    AM_SMOKE,
    AM_40MMGRENADES,

    AM_NUM_AMMO

} ammo_t;

typedef enum {

    // shotgun shells
    MF_NONE,

    MF_SMOKE
} missleFunc_t;
// Navy Seals --

// reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
#define	PLAYEREVENT_DENIEDREWARD		0x0001
#define	PLAYEREVENT_GAUNTLETREWARD		0x0002
#define PLAYEREVENT_HOLYSHIT			0x0004

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
#define	EV_EVENT_BIT1		0x00000100
#define	EV_EVENT_BIT2		0x00000200
#define	EV_EVENT_BITS		( EV_EVENT_BIT1 | EV_EVENT_BIT2 )

typedef enum {
    EV_NONE,

    EV_FOOTSTEP,
    EV_FOOTSTEP_METAL,
    EV_FOOTSTEP_DIRT,
    EV_FOOTSTEP_SNOW,
    EV_FOOTSTEP_WOOD,

    EV_FOOTSPLASH,
    EV_FOOTWADE,
    EV_SWIM,

    EV_STEP_4,
    EV_STEP_8,
    EV_STEP_12,
    EV_STEP_16,

    EV_FALL_SHORT, // only footstep sound
    EV_FALL_LIGHT, // light damage
    EV_FALL_MEDIUM,
    EV_FALL_FAR,   // heavy damage
    EV_FALL_DEATH, // immediate death!

    EV_STOLENWEAPON,
    EV_JUMP_PAD,			// boing sound at origin, jump sound on player

    EV_JUMP,
    EV_WATER_TOUCH,	// foot touches
    EV_WATER_LEAVE,	// foot leaves
    EV_WATER_UNDER,	// head touches
    EV_WATER_CLEAR,	// head leaves

    EV_ITEM_PICKUP,			// normal item pickups are predictable
    EV_GLOBAL_ITEM_PICKUP,		// powerup / team sounds are broadcast to everyone

    EV_NOAMMO,
    EV_CHANGE_WEAPON,
    EV_FIRE_WEAPON,
    EV_FIRE_WEAPON_OTHER,

    EV_USE_ITEM0,
    EV_USE_ITEM1,
    EV_USE_ITEM2,
    EV_USE_ITEM3,
    EV_USE_ITEM4,
    EV_USE_ITEM5,
    EV_USE_ITEM6,
    EV_USE_ITEM7,
    EV_USE_ITEM8,
    EV_USE_ITEM9,
    EV_USE_ITEM10,
    EV_USE_ITEM11,
    EV_USE_ITEM12,
    EV_USE_ITEM13,
    EV_USE_ITEM14,
    EV_USE_ITEM15,

    EV_ITEM_RESPAWN,
    EV_ITEM_POP,
    EV_PLAYER_TELEPORT_IN,
    EV_PLAYER_TELEPORT_OUT,

    EV_GRENADE_BOUNCE,		// eventParm will be the soundindex

    EV_GENERAL_SOUND,
    EV_GLOBAL_SOUND,		// no attenuation
    EV_GLOBAL_TEAM_SOUND,

    EV_BULLET_HIT_FLESH,
    EV_BULLET_HIT_WALL,

    EV_MISSILE_HIT,
    EV_MISSILE_MISS,
    EV_MISSILE_MISS_METAL,
    EV_RAILTRAIL,
    EV_SHOTGUN,
    EV_BULLET,				// otherEntity is the shooter

    EV_PAIN,
    EV_DEATH,
    EV_OBITUARY,

    EV_POWERUP_QUAD,
    EV_POWERUP_BATTLESUIT,
    EV_POWERUP_REGEN,

    EV_GIB_PLAYER,			// gib a previously living player
    EV_SCOREPLUM,			// score plum

    //#ifdef MISSIONPACK
    EV_PROXIMITY_MINE_STICK,
    EV_PROXIMITY_MINE_TRIGGER,
    EV_KAMIKAZE,			// kamikaze explodes
    EV_OBELISKEXPLODE,			// obelisk explodes
    EV_OBELISKPAIN,			// obelisk is in pain
    EV_INVUL_IMPACT,			// invulnerability sphere impact
    EV_JUICED,				// invulnerability juiced effect
    EV_LIGHTNINGBOLT,			// lightning bolt bounced of invulnerability sphere
    //#endif

    EV_DEBUG_LINE,
    EV_STOPLOOPINGSOUND,
    EV_TAUNT,
    EV_TAUNT_YES,
    EV_TAUNT_NO,
    EV_TAUNT_FOLLOWME,
    EV_TAUNT_GETFLAG,
    EV_TAUNT_GUARDBASE,

    EV_GAMESTATE,

    EV_BLOODER,
    EV_BLOOD_ON_WALL,

    EV_EMPTYCLIP,

    EV_BANDAGING,
    EV_EXPLOSION,
    EV_C4DEPLOY,

    EV_RELOAD,	// reload
    EV_RELOAD_EMPTY,

    EV_FUNCEXPLOSIVE,
    EV_FLASHBANG,

    EV_BLOODPOOL,
    EV_LOCALSOUND,

    // clipping wire
    EV_CLIPWIRE_1,
    EV_CLIPWIRE_2,
    EV_CLIPWIRE_3,
    EV_CLIPWIRE_4,
    EV_CLIPWIRE_5,
    EV_CLIPWIRE_6,
    EV_CLIPWIRE_7,
    EV_CLIPWIRE_8,

    EV_BREAKLOCK
} entity_event_t;


typedef enum {
    GTS_REDTEAM_SCORED,
    GTS_BLUETEAM_SCORED,
    GTS_REDTEAM_TOOK_LEAD,
    GTS_BLUETEAM_TOOK_LEAD,
    GTS_TEAMS_ARE_TIED, // still missing
    GTS_DRAW_ROUND,
    GTS_BRIEFCASE_TAKEN,
    GTS_RED_BRIEFCASE_RETURN,
    GTS_BLUE_BRIEFCASE_RETURN,

    GTS_AF1_CAPTURED,
    GTS_AF1_BLOCKED,
    GTS_AF1_TAPPED,

    GTS_AF2_CAPTURED,
    GTS_AF2_BLOCKED,
    GTS_AF2_TAPPED,

    GTS_AF3_CAPTURED,
    GTS_AF3_BLOCKED,
    GTS_AF3_TAPPED,

    GTS_AF4_CAPTURED,
    GTS_AF4_BLOCKED,
    GTS_AF4_TAPPED,


} global_team_sound_t;

// animations
typedef enum {

    BOTH_DEATH_FACE,
    BOTH_DEAD_FACE,
    BOTH_DEATH_CHEST,
    BOTH_DEAD_CHEST,
    BOTH_DEATH_STOMACH,
    BOTH_DEAD_STOMACH,
    BOTH_DEATH_BLEED,
    BOTH_DEAD_BLEED,
    BOTH_DEATH_NECK,
    BOTH_DEAD_NECK,
    BOTH_DEATH_BACK,
    BOTH_DEAD_BACK,
    BOTH_DEATH_LEGS,
    BOTH_DEAD_LEGS,

    TORSO_GESTURE1,
    TORSO_GESTURE2,
    TORSO_GESTURE3,

    TORSO_OPEN_DOOR,

    TORSO_RELOAD_RIFLE,
    TORSO_RELOAD_PISTOL,

    TORSO_STAND_RIFLE,
    TORSO_ATTACK_RIFLE,

    TORSO_STAND_RIFLE_SCOPED,
    TORSO_ATTACK_RIFLE_SCOPED,

    TORSO_STAND_PISTOL,
    TORSO_ATTACK_PISTOL,

    TORSO_STAND_SUITCASE,
    TORSO_ATTACK_SUITCASE,

    TORSO_STAND_ITEM,
    TORSO_ATTACK_MELEE,
    TORSO_THROW,

    TORSO_USE,

    TORSO_STEER_LEFT,
    TORSO_STEER_IDLE,
    TORSO_STEER_RIGHT,

    TORSO_RAISE_RIFLE,
    TORSO_DROP_RIFLE,
    TORSO_RAISE_PISTOL,
    TORSO_DROP_PISTOL,

    TORSO_CLIMB,
    TORSO_CLIMB_IDLE,

    LEGS_IDLECR,
    LEGS_WALKCR,

    LEGS_IDLE,
    LEGS_WALK,
    LEGS_RUN,
    LEGS_BACK,
    LEGS_LIMP,
    LEGS_TURN,

    LEGS_SWIM,

    LEGS_JUMP,
    LEGS_LAND,

    LEGS_CLIMB,
    LEGS_CLIMB_IDLE,

    LEGS_SIT_CAR,
    // must be last - hardcoded
    LEGS_BACKCR,
    LEGS_BACKLIMB,
    LEGS_IDLE_RIFLE_SCOPED,
    LEGS_IDLECR_RIFLE_SCOPED,

    MAX_ANIMATIONS
} animNumber_t;

// animations
typedef enum {
    WANIM_PUTUP,
    WANIM_PUTAWAY,
    WANIM_ATTACK,
    WANIM_ATTACK2,
    WANIM_ATTACK3,
    WANIM_LASTRND,
    WANIM_IDLE,
    WANIM_IDLE_EMPTY,
    WANIM_RELOAD,
    WANIM_RELOAD_EMPTY,
    WANIM_RELOAD_CYCLE,
    WANIM_RELOAD_STOP,
    WANIM_SPIN1,
    WANIM_MELEE,
    WANIM_THROW,
    WANIM_SPIN2,
    WANIM_ATTACKMODE21,
    WANIM_ATTACKMODE22,
    WANIM_ATTACKMODE23,

    WANIM_IRONSIGHT_UP,
    WANIM_IRONSIGHTIDLE,
    WANIM_IRONSIGHT_DN,
    WANIM_IRONSIGHT_ATK1,// fixed to the same length as the normal attack frames
    WANIM_IRONSIGHT_ATK2,// there don't have to be 2,3 only if the normal
    WANIM_IRONSIGHT_ATK3,//fire-animation

    WANIM_UNKNOWN,	// should be last... always
    MAX_WEAPON_ANIMATIONS
} weaponAnimNumber_t;


typedef struct animation_s {
    int		firstFrame;
    int		numFrames;
    int		loopFrames;			// 0 to numFrames
    int		frameLerp;			// msec between frames
    int		initialLerp;			// msec to get to first frame
    int		reversed;			// true if animation is reversed
    int		flipflop;			// true if animation should flipflop back to base
} animation_t;


// flip the togglebit every time an animation
// changes so a restart of the same anim can be detected
#define	ANIM_TOGGLEBIT		128


typedef enum {
    TEAM_FREE,
    TEAM_RED,
    TEAM_BLUE,
    TEAM_SPECTATOR,

    TEAM_NUM_TEAMS
} team_t;

typedef enum {
    /*	STATE_WAITING,
    STATE_INPROGRESS,
    STATE_ROUNDOVER*/
    STATE_OPEN,
    STATE_OVER,
    STATE_LOCKED
} gamestate_t;

// Time between location updates
#define TEAM_LOCATION_UPDATE_TIME		1000

// How many players on the overlay
#define TEAM_MAXOVERLAY		32

//team task
typedef enum {
    TEAMTASK_NONE,
    TEAMTASK_OFFENSE,
    TEAMTASK_DEFENSE,
    TEAMTASK_PATROL,
    TEAMTASK_FOLLOW,
    TEAMTASK_RETRIEVE,
    TEAMTASK_ESCORT,
    TEAMTASK_CAMP
} teamtask_t;

// means of death
typedef enum {
    MOD_UNKNOWN,
    MOD_SHOTGUN,
    MOD_GAUNTLET,
    MOD_MACHINEGUN,
    MOD_GRENADE,
    MOD_GRENADE_SPLASH,
    MOD_ROCKET,
    MOD_ROCKET_SPLASH,
    MOD_PLASMA,
    MOD_PLASMA_SPLASH,
    MOD_RAILGUN,
    MOD_LIGHTNING,
    MOD_BFG,
    MOD_BFG_SPLASH,
    MOD_WATER,
    MOD_SLIME,
    MOD_LAVA,
    MOD_CRUSH,
    MOD_TELEFRAG,
    MOD_FALLING,
    MOD_SUICIDE,
    MOD_TARGET_LASER,
    MOD_TRIGGER_HURT,
#ifdef MISSIONPACK
    MOD_NAIL,
    MOD_CHAINGUN,
    MOD_PROXIMITY_MINE,
    MOD_KAMIKAZE,
    MOD_JUICED,
#endif
    MOD_GRAPPLE,
    MOD_BLEED,
    MOD_HEADSHOT,
    MOD_LEAD
} meansOfDeath_t;


//---------------------------------------------------------

// gitem_t->type
typedef enum {
    IT_BAD,
    IT_WEAPON,				// EFX: rotate + upscale + minlight
    IT_AMMO,				// EFX: rotate
    IT_ARMOR,				// EFX: rotate + minlight
    IT_HEALTH,				// EFX: static external sphere + rotating internal
    IT_POWERUP,			// instant on, timer based
    // EFX: rotate + external ring that rotates
    IT_HOLDABLE,			// single use, holdable item
    // EFX: rotate + bob
    IT_BOTROAM,			// for bots 
    IT_TEAM
} itemType_t;

// message status
typedef enum {
    MS_NONE,

    MS_HEALTH1, // good (>100-80)
    MS_HEALTH2,
    MS_HEALTH3,
    MS_HEALTH4,
    MS_HEALTH5, // poor ( 0-20)

    MS_DEAD,
    MS_BOMB

} messageStatus_t;

#define MAX_ITEM_MODELS 4

typedef struct gitem_s {
    char		*classname;	// spawning name
    char		*pickup_sound;
    char		*world_model[MAX_ITEM_MODELS];

    char		*icon;
    char		*pickup_name;	// for printing on pickup

    int			quantity;		// for ammo how much, or duration of powerup
    itemType_t  giType;				// IT_* flags

    int			giTag;
    int			giAmmoTag;

    char		*precaches;		// string of all models and images this item will use
    char		*sounds;		// string of all sounds this item will use
} gitem_t;

// included in both the game dll and the client
extern	gitem_t	bg_itemlist[];
extern	int		bg_numItems;

gitem_t	*BG_FindItem( const char *pickupName );
gitem_t	*BG_FindItemForWeapon( int i );
gitem_t	*BG_FindItemForPowerup( powerup_t pw );
gitem_t	*BG_FindItemForHoldable( holdable_t pw );
char *BG_SurfaceToString( int surfaceFlags );
#define	ITEM_INDEX(x) ((x)-bg_itemlist)

qboolean	BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps );


// g_dmflags->integer flags
#define	DF_NO_FALLING			8
#define DF_FIXED_FOV			16
#define	DF_NO_FOOTSTEPS			32

// content masks
#define	MASK_ALL				(-1)
#define	MASK_SOLID				(CONTENTS_SOLID)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER				(CONTENTS_WATER)
#define	MASK_OPAQUE				(CONTENTS_SOLID)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)


//
// entityState_t->eType
//
typedef enum {
    ET_GENERAL,
    ET_PLAYER,
    ET_ITEM,
    ET_MISSILE,
    ET_MOVER,
    ET_BEAM,
    ET_PORTAL,
    ET_SPEAKER,
    ET_PUSH_TRIGGER,
    ET_TELEPORT_TRIGGER,
    ET_INVISIBLE,
    ET_GRAPPLE,				// grapple hooked on wall
    ET_TEAM,
    ET_FLARE,
    ET_BULLET,

    ET_DOOR,
    ET_FUNCEXPLOSIVE,
#define PARTICLEHOST 1

#ifdef PARTICLEHOST
    ET_PARTICLEHOST,
#endif
    ET_ELEVBUT0,
    ET_ELEVBUT1,
    ET_ELEVBUT2,
    ET_ELEVBUT3,
    ET_ELEVBUT4,
    ET_ELEVBUT5,
    ET_ELEVBUT6,
    ET_ELEVBUT7,
    //	ET_ACTOR, // actors like hostages

    ET_EVENTS				// any of the EV_* events can be added freestanding
    // by setting eType to ET_EVENTS + eventNum
    // this avoids having to set eFlags and eventNum
} entityType_t;



void	BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void	BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );

void	BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad );

void	BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap );
void	BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap );

qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );
float	BG_LeadGetBreakValueForSurface( trace_t *tr );

#define ARENAS_PER_TIER		4
#define MAX_ARENAS			1024
#define	MAX_ARENAS_TEXT		8192

#define MAX_BOTS			1024
#define MAX_BOTS_TEXT		8192


// Kamikaze

// 1st shockwave times
#define KAMI_SHOCKWAVE_STARTTIME		0
#define KAMI_SHOCKWAVEFADE_STARTTIME	1500
#define KAMI_SHOCKWAVE_ENDTIME			2000
// explosion/implosion times
#define KAMI_EXPLODE_STARTTIME			250
#define KAMI_IMPLODE_STARTTIME			2000
#define KAMI_IMPLODE_ENDTIME			2250
// 2nd shockwave times
#define KAMI_SHOCKWAVE2_STARTTIME		2000
#define KAMI_SHOCKWAVE2FADE_STARTTIME	2500
#define KAMI_SHOCKWAVE2_ENDTIME			3000
// radius of the models without scaling
#define KAMI_SHOCKWAVEMODEL_RADIUS		88
#define KAMI_BOOMSPHEREMODEL_RADIUS		72
// maximum radius of the models during the effect
#define KAMI_SHOCKWAVE_MAXRADIUS		1320
#define KAMI_BOOMSPHERE_MAXRADIUS		720
#define KAMI_SHOCKWAVE2_MAXRADIUS		704


// Navy Seals ++
// navy seals +
qboolean BG_IsSemiAutomatic( int weapon );
qboolean BG_IsRifle( int weapon );
qboolean BG_IsSmg( int weapon );
qboolean BG_IsMelee( int weapon );
qboolean BG_IsPistol( int weapon );
qboolean BG_GotPrimary ( const playerState_t *ps );
qboolean BG_IsPrimary ( int weapon );
qboolean BG_IsSecondary ( int weapon );
qboolean BG_IsSMG ( int weapon );
qboolean BG_IsGrenade( int weapon );
qboolean BG_IsZooming( int weaponmode );
qboolean BG_HasLaser( int weaponmode );
qboolean BG_IsShotgun( int weapon );
qboolean BG_IsInGLMode( int weaponmode );
int BG_CalcSpeed( playerState_t ps );
float BG_GetSpeedMod( int techLevel );
int BG_WeaponMods( int weapon ) ;
void BG_PackWeapon( int weapon, int stats[ ] );
void BG_ClearWeapons( int stats[ ] );
void BG_RemoveWeapon( int weapon, int stats[ ] );
qboolean BG_GotWeapon( int weapon, int stats[ ] );
// navy seals-

// Camoflage
typedef enum {
    CAMO_NONE, // 0

    CAMO_DESERT, // 1
    CAMO_JUNGLE, // 2
    CAMO_ARCTIC, // 3
    CAMO_URBAN,  // 4

    CAMO_TOTAL
} camotypes_t;

// Player Equipment
typedef enum {
    EQ_NONE,

    EQ_STORMGOGGLES,
    EQ_JOINT,
    EQ_PIECEOFHAY,
    EQ_NVGOGGLES,
    EQ_SEALHAT,
    EQ_TURBAN,
    EQ_HELMET,

    EQ_TOTAL
} equipmenttypes_t;

// Navy Seals ++
typedef enum {
    BHOLE_NORMAL, // stone - default
    BHOLE_GLASS,
    BHOLE_METAL,
    BHOLE_SAND,
    BHOLE_SNOW,
    BHOLE_SOFT,
    BHOLE_WOOD,
    BHOLE_STONE,
    BHOLE_DIRT,

    BHOLE_TOTAL
} bullethole_t;
// Navy Seals --


// brasstype
typedef enum {
    BRASS_NONE,

    BRASS_PISTOL,
    BRASS_PISTOL_LEFT,

    BRASS_RIFLE,
    BRASS_RIFLE_LEFT,

    BRASS_SHOTGUN,
    BRASS_SHOTGUN_LEFT,

    BRASS_COLT,

    BRASS_TOTAL
} brasstype_t;

#define MAX_WEAPONPARTS		12

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
typedef struct weaponInfo_s {
    qboolean		registered;
    gitem_t			*item; 
    qhandle_t		handsModel;			// the hands don't actually draw, they just position the weapon
    qhandle_t		weaponModel; 
    qhandle_t		silencerModel;
    qhandle_t		scopeModel;
    qhandle_t		lasersightModel;
    qhandle_t		flashModel;
    qhandle_t		bayonetModel;
    qhandle_t		v_silencerModel;
    qhandle_t		v_scopeModel;
    qhandle_t		v_lasersightModel;
    qhandle_t		v_flashModel;
    qhandle_t		v_glModel;
    qhandle_t		v_bayonetModel;

    qhandle_t		viewweaponModel;
    qhandle_t		t_viewweaponSkin;
    qhandle_t		glModel;
    qhandle_t		weaponIcon; 

    qhandle_t		weaponParts[MAX_WEAPONPARTS];
    char			*partTags[MAX_WEAPONPARTS];
 
    float			flashDlight;
    vec3_t			flashDlightColor;
    sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose 
    sfxHandle_t		sil_flashSound[4];	// sileneced..
    sfxHandle_t		otherflashSound[4];	// sileneced..  
	sfxHandle_t		reloadSound;
    sfxHandle_t		reloadEmptySound;

    qhandle_t		missileModel;
    sfxHandle_t		missileSound; 
    float			missileDlight;
    vec3_t			missileDlightColor; 

    brasstype_t		ejectBrassType;		// type of brass to be ejected
    int				ejectBrassNum;		// amount of brass to be ejected

    float			trailRadius;
    float			wiTrailTime;

    int				kickBack;
} weaponInfo_t;