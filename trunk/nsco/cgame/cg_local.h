// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "../game/q_shared.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#ifdef MISSIONPACK
#define CG_FONT_THRESHOLD 0.1
#endif

#define	POWERUP_BLINKS		5

#define	POWERUP_BLINK_TIME	1000
#define	FADE_TIME			200
#define	PULSE_TIME			200


#define	LAND_DEFLECT_TIME	500
#define	LAND_RETURN_TIME	500
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	PAIN_TWITCH_TIME	200
#define	WEAPON_SELECT_TIME	1400
#define	ITEM_SCALEUP_TIME	1000
#define	ZOOM_TIME			150
#define	ITEM_BLOB_TIME		200
#define	MUZZLE_FLASH_TIME	20
#define	SINK_TIME			250		// time for fragments to sink into ground before going away
#define	ATTACKER_HEAD_TIME	10000
#define	REWARD_TIME			3000

#define	PULSE_SCALE			1.5			// amount to scale up the icons when activating

#define	MAX_STEP_CHANGE		32

#define	MAX_VERTS_ON_POLY	10
#define	MAX_MARK_POLYS		256

#define STAT_MINUS			10	// num frame for '-' stats digit

#define	ICON_SIZE			32 // 48
#define	CHAR_WIDTH			24 // ^^^^old == 32
#define	CHAR_HEIGHT			24 // 48
#define	TEXT_ICON_SPACE		4
/*
#define	ICON_SIZE			48
#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48
#define	TEXT_ICON_SPACE		4
*/
#define	TEAMCHAT_WIDTH		80
#define TEAMCHAT_HEIGHT		15

// very large characters
#define	GIANT_WIDTH			32
#define	GIANT_HEIGHT		48

#define	NUM_CROSSHAIRS		10

#define TEAM_OVERLAY_MAXNAME_WIDTH	12
#define TEAM_OVERLAY_MAXLOCATION_WIDTH	16

#define	DEFAULT_MODEL			"none"

#define DEFAULT_REDTEAM_NAME		"Stroggs"
#define DEFAULT_BLUETEAM_NAME		"Pagans"

typedef enum {
    FOOTSTEP_NORMAL,
    FOOTSTEP_METAL,
    FOOTSTEP_SPLASH,

    // Navy Seals ++
    FOOTSTEP_DIRT,
    FOOTSTEP_WOOD,
    FOOTSTEP_SNOW,
    // Navy Seals --

    FOOTSTEP_TOTAL
} footstep_t;



typedef enum {
    IMPACTSOUND_DEFAULT,
    IMPACTSOUND_METAL,
    IMPACTSOUND_DIRT,
	IMPACTSOUND_WOOD,
	IMPACTSOUND_SNOW,
	IMPACTSOUND_GLASS,
	IMPACTSOUND_TOTAL
} impactSound_t;

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

// when changing animation, set animationTime to frameTime + lerping time
// The current lerp will finish out, then it will lerp to the new animation
typedef struct {
    int			oldFrame;
    int			oldFrameTime;		// time when ->oldFrame was exactly on

    int			frame;
    int			frameTime;			// time when ->frame will be exactly on

    float		backlerp;

    float		yawAngle;
    qboolean	yawing;
    float		pitchAngle;
    qboolean	pitching;

    int			animationNumber;	// may include ANIM_TOGGLEBIT
    animation_t	*animation;
    int			animationTime;		// time when the first frame of the animation will be exact
} lerpFrame_t;


typedef struct {
    lerpFrame_t		legs, torso;
    int				painTime;
    int				painDirection;	// flip from 0 to 1

    // Navy Seals ++
    lerpFrame_t		hand_weapon;

    int				weapAnim;
    int				nextweapAnim;
    // Navy Seals --
} playerEntity_t;

//=================================================



// centity_t have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
typedef struct centity_s {
    entityState_t	currentState;	// from cg.frame
    entityState_t	nextState;	// from cg.nextFrame, if available
    qboolean		interpolate;	// true if next is valid to interpolate to
    qboolean		currentValid;	// true if cg.frame holds this entity

    int				muzzleFlashTime;	// move to playerEntity?
    // Navy Seals ++
    int				gunSmokeTime;
    int				brassEjected;		// # of brass already ejected this time firing
    qboolean		footstepSpawn;				// 0 spawns no, 1 spawns footprint it will automatically decide which footstep spawns
    int				leftWaterTime;
    int				lastfootStep;		// last foot step
    // Navy Seals --
    int				previousEvent;
    int				teleportFlag;

    int				trailTime;		// so missile trails can handle dropped initial packets
    int				dustTrailTime;
    int				miscTime;

    int				wakemarkTime;

    playerEntity_t	pe;

    int				errorTime;		// decay the error from this time
    vec3_t			errorOrigin;
    vec3_t			errorAngles;

    qboolean		extrapolated;	// false if origin / angles is an interpolation
    vec3_t			rawOrigin;
    vec3_t			rawAngles;

    vec3_t			beamEnd;

    // exact interpolated position of entity on this frame
    vec3_t			lerpOrigin;
    vec3_t			lerpAngles;

    qboolean		gunSmokePuff;
} centity_t;


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

typedef struct markPoly_s {
    struct markPoly_s	*prevMark, *nextMark;
    int			time;
    qhandle_t	markShader;
    qboolean	alphaFade;		// fade alpha instead of rgb
    float		color[4];
    poly_t		poly;
    vec3_t		org;
    vec3_t		plane;
    polyVert_t	verts[MAX_VERTS_ON_POLY];
} markPoly_t;


typedef enum {
    LE_MARK,
    LE_EXPLOSION,
    LE_SPRITE_EXPLOSION,
    LE_FRAGMENT,
    LE_MOVE_SCALE_FADE,
    LE_FALL_SCALE_FADE,
    LE_FADE_RGB,
    LE_SCALE_FADE,
    LE_SCOREPLUM,
    // Navy Seals ++
    LE_SHRAPNEL,
    LE_PARTICLE,
    LE_TRACER,

    LE_SHOWREFENTITY
    // Navy Seals --

} leType_t;

typedef enum {
    LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
    LEF_TUMBLE			 = 0x0002,		// tumble over time, used for ejecting shells
    LEF_SOUND1			 = 0x0004,		// sound 1 for kamikaze
    LEF_SOUND2			 = 0x0008,		// sound 2 for kamikaze
    LEF_3RDPERSON		 = 0x0010,
    LEF_PUFF_DONT_FADE	 = 0x0020,
} leFlag_t;

typedef enum {
    LEMT_NONE,
    LEMT_BURN,
    // Navy Seals ++
    LEMT_BLEEDER,
    // Navy Seals --
    LEMT_BLOOD
} leMarkType_t;			// fragment local entities can leave marks on walls

typedef enum {
    LEBS_NONE,
    LEBS_BLOOD,
    LEBS_BRASS,
    // Navy Seals ++
    LEBS_BLEEDER,
    LEBS_SPARK
    // Navy Seals --
} leBounceSoundType_t;	// fragment local entities can make sounds on impacts

typedef struct localEntity_s {
    struct localEntity_s	*prev, *next;
    leType_t		leType;
    int				leFlags;

    int				startTime;
    int				endTime;
    int				fadeInTime;

    float			lifeRate;			// 1.0 / (endTime - startTime)

    trajectory_t	pos;
    trajectory_t	angles;

    float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

    float			color[4];

    float			radius;

    float			light;
    vec3_t			lightColor;

    leMarkType_t		leMarkType;		// mark to leave on fragment impact
    leBounceSoundType_t	leBounceSoundType;

    refEntity_t		refEntity;
} localEntity_t;

//======================================================================


typedef struct {
    int				client;
    int				score;
    int				ping;
    int				time;
    int				scoreFlags;
    int				powerUps;
    int				team;

    int				status;
} score_t;

typedef struct {
    int		step_walkl;
    int		step_walkr;
    int		step_runr;
    int		step_runl;
    int		step_limpl;
    int		step_limpr;
    int		step_backl;
    int		step_backr;
} printTimer_t;

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
#define	MAX_CUSTOM_SOUNDS	32

typedef struct {
    qboolean		infoValid;

    char			name[MAX_QPATH];
    team_t			team;

    int				botSkill;		// 0 = not bot, 1-5 = bot

    vec3_t			color;

    int				score;			// updated by score servercmds
    int				location;		// location index for team mode
    int				health;			// you only get this info about your teammates
    int				armor;
    int				curWeapon;

    int				handicap;
    int				wins, losses;	// in tourney mode

    int				teamTask;		// task in teamplay (offence/defence)
    qboolean		teamLeader;				// true when this is a team leader

    int				powerups;		// so can display quad/flag status

    int				medkitUsageTime;
    int				invulnerabilityStartTime;
    int				invulnerabilityStopTime;

    int				breathPuffTime;

    // when clientinfo is changed, the loading of models/skins/sounds
    // can be deferred until you are dead, to prevent hitches in
    // gameplay
    char			modelName[MAX_QPATH];
    char			headName[MAX_QPATH];

    char			skinName[MAX_QPATH];
    char			headModelName[MAX_QPATH];
    char			headSkinName[MAX_QPATH];
    char			redTeam[MAX_TEAMNAME];
    char			blueTeam[MAX_TEAMNAME];
    qboolean		deferred;

    qboolean		newAnims;		// true if using the new mission pack animations

    vec3_t			headOffset;		// move head in icon views
    int				footsteps;
    gender_t		gender;			// from model

    qhandle_t		legsModel;
    qhandle_t		legsSkin;

    qhandle_t		torsoModel;
    qhandle_t		torsoSkin;
    // Navy Seals ++
    qhandle_t		torsoVestSkin;

    qhandle_t		headModel;
    qhandle_t		headSkin;

    qhandle_t		leftArmModel;
    qhandle_t		rightArmModel;

    //	qhandle_t		helmetModel;
    //	qhandle_t		helmetSkin;
    // Navy Seals --
    //	qhandle_t		modelIcon;

    qhandle_t		equipmentMouth;
    qhandle_t		equipmentHead;
    qhandle_t		equipmentEyes;

    char			equipmentMouthName[MAX_QPATH];
    char			equipmentHeadName[MAX_QPATH];
    char			equipmentEyesName[MAX_QPATH];

    animation_t		animations[MAX_ANIMATIONS];

    printTimer_t	footprintFrameTimer;
} clientInfo_t;



// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
typedef struct {
    qboolean		registered;
    qhandle_t		models[MAX_ITEM_MODELS];
    qhandle_t		icon;
} itemInfo_t;


typedef struct {
    int				itemNum;
} powerupInfo_t;


typedef struct {

    qboolean	w_gotBomb,w_bombRange,w_bombPlaceTeam,w_bombPlaceEnemy,w_vipTime,w_vipRescue;
    qboolean	w_gotlotsXp, w_reloadRifle, w_reloadShotgun, w_assaultRange, w_weaponEmpty;
    qboolean	w_gotBriefcase, w_briefcaseTaken_Seal, w_briefcaseTaken_Tango, w_firstHit;
    qboolean	w_assaultBlocked;

    qboolean	w_getScope,w_getLasersight,w_getSilencer,w_getGl,w_getBayonet;
    qboolean	w_getLasersight_sec, w_getSilencer_sec;

    int			w_enemySpotted, w_friendSpotted, w_doorSpotted;

    int			stringTime;
    int			stringCharWidth;
    float		stringCharHeight;
    int			stringY;
    char		string[1024];
    int			stringLines;
} newbeehelp_t;

#define MAX_BLOCKEDCVARS	64

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
    char		maxvalue[MAX_CVAR_VALUE_STRING];
    char		minvalue[MAX_CVAR_VALUE_STRING];
    qboolean	restart_video;
    char		string[MAX_CVAR_VALUE_STRING];
} vmBlockedCvar_t;

typedef struct {
    char		type;
    int			x;
    int			y;
    int			height;
    vec3_t		origin;
} radarEntity_t;

#define MAX_REWARDSTACK		10
#define MAX_SOUNDBUFFER		20

//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

#define MAX_PREDICTED_EVENTS	16


typedef struct {
    int			clientFrame;		// incremented each frame

    int			clientNum;

    qboolean	demoPlayback;
    qboolean	levelShot;			// taking a level menu screenshot
    int			deferredPlayerLoading;
    qboolean	loading;			// don't defer players at initial startup
    qboolean	intermissionStarted;		// don't play voice rewards, because game will end shortly

    // there are only one or two snapshot_t that are relevent at a time
    int			latestSnapshotNum;	// the number of snapshots the client system has received
    int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

    snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
    snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
    snapshot_t	activeSnapshots[2];

    float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

    qboolean	thisFrameTeleport;
    qboolean	nextFrameTeleport;

    int			frametime;		// cg.time - cg.oldTime

    int			time;			// this is the time value that the client
    // is rendering at.
    int			oldTime;		// time at last frame, used for missile trails and prediction checking

    int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

    int			timelimitWarnings;	// 5 min, 1 min, overtime
    int			fraglimitWarnings;

    qboolean	mapRestart;			// set on a map restart to set back the weapon

    qboolean	renderingThirdPerson;		// during deaths, chasecams, etc

    // prediction state
    qboolean	hyperspace;				// true if prediction has hit a trigger_teleport
    playerState_t	predictedPlayerState;
    centity_t		predictedPlayerEntity;
    qboolean	validPPS;				// clear until the first call to CG_PredictPlayerState
    int			predictedErrorTime;
    vec3_t		predictedError;

    int			eventSequence;
    int			predictableEvents[MAX_PREDICTED_EVENTS];

    float		stepChange;				// for stair up smoothing
    int			stepTime;

    float		duckChange;				// for duck viewheight smoothing
    int			duckTime;

    float		landChange;				// for landing hard
    int			landTime;

    // input state sent to server
    int			weaponSelect;

    // auto rotating items
    vec3_t		autoAngles;
    vec3_t		autoAxis[3];
    vec3_t		autoAnglesFast;
    vec3_t		autoAxisFast[3];

    // view rendering
    refdef_t	refdef;
    vec3_t		refdefViewAngles;		// will be converted to refdef.viewaxis

    // weapon rendering
    refdef_t	weaponrefdef;

    // zoom key
    qboolean	zoomed;
    int			zoomTime;
    float		zoomSensitivity;

    // information screen text during loading
    char		infoScreenText[MAX_STRING_CHARS];

    // scoreboard
    int			scoresRequestTime;
    int			numScores;
    int			selectedScore;
    int			teamScores[2];
    score_t		scores[MAX_CLIENTS];
    qboolean	showScores;
    qboolean	scoreBoardShowing;
    int			scoreFadeTime;
    char		killerName[MAX_NAME_LENGTH];
    char			spectatorList[MAX_STRING_CHARS];		// list of names
    int				spectatorLen;				// length of list
    float			spectatorWidth;					// width in device units
    int				spectatorTime;				// next time to offset
    int				spectatorPaintX;			// current paint x
    int				spectatorPaintX2;			// current paint x
    int				spectatorOffset;			// current offset from start
    int				spectatorPaintLen;			// current offset from start


    // centerprinting
    int			centerPrintTime;
    int			centerPrintCharWidth;
    int			centerPrintY;
    char		centerPrint[1024];
    int			centerPrintLines;

    // kill timers for carnage reward
    int			lastKillTime;

    // crosshair client ID
    int			crosshairClientNum;
    int			crosshairClientTime;

    // powerup active flashing
    int			powerupActive;
    int			powerupTime;

    // attacking player
    int			attackerTime;
    int			voiceTime;

    // reward medals
    int			rewardStack;
    int			rewardTime;
    int			rewardCount[MAX_REWARDSTACK];
    qhandle_t	rewardShader[MAX_REWARDSTACK];
    qhandle_t	rewardSound[MAX_REWARDSTACK];

    // sound buffer mainly for announcer sounds
    int			soundBufferIn;
    int			soundBufferOut;
    int			soundTime;
    qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

    // warmup countdown
    int			warmup;
    int			warmupCount;

    //==========================

    int			itemPickup;
    int			itemPickupTime;
    int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

    int			weaponSelectTime;
    int			weaponAnimation;
    int			weaponAnimationTime;

    // blend blobs
    float		damageTime;
    float		damageX, damageY, damageValue;

    // status bar head
    float		headYaw;
    float		headEndPitch;
    float		headEndYaw;
    int			headEndTime;
    float		headStartPitch;
    float		headStartYaw;
    int			headStartTime;

    // view movement
    float		v_dmg_time;
    float		v_dmg_pitch;
    float		v_dmg_roll;

    vec3_t		kick_angles;	// weapon kicks
    vec3_t		kick_origin;

    // temp working variables for player view
    float		bobfracsin;
    int			bobcycle;
    float		xyspeed;
    int     nextOrbitTime;

    // development tool
    refEntity_t		testModelEntity;
    char			testModelName[MAX_QPATH];
    qboolean		testGun;

    // Navy Seals ++
    // menu action
    const char		*menuLines[20];
    int			inMenu;
    int			num_menuLines;
    int			menuValidSlots;

    // XPTIME
    int			xpPoints;
    float		xpTime;

    // crosshair movement fadeout / still fadein
    int			crosshairTime;

    // inwater fading rings
    int			WaterTime;

    // smoking gun barrels
    int			gunSmokeTime;

    // inventory
    int			activeInventory;
    int			curInventory;
    int			InventoryTime;
    int Inventory[5][WP_NUM_WEAPONS];
    int Inventorypos[5];
    int			InventoryFadeTime;

    // flashbang stuff
    int			FlashTime;
    int			ConcussionTime;
    int     DeafTime;
    int			flashedVisionTime;
    int			flashedVision_x;
    int			flashedVision_y;

    // weaponhold
    float		weaponPos;
    qboolean		roundStarted;
    int			roundlimitWarnings;
    int	bombcaseWires[8];
    qboolean	crosshairState;

    int			loadingbarState;
    int			loadingbarMax;

    int			crosshairMod;
    int			crosshairModTime;
    qboolean	crosshairFinishedChange;
    qboolean	crosshairFadeIn;

    int			ladderWeaponTime;

    int			playerStatus[MAX_CLIENTS];

    vec3_t		cameraOrigin;
    qboolean	cameraActive;
    int			cameraRemainTime;
    int			cameraFollowNumber;
    qboolean	cameraChase;
    float		cameraZoom;
    qboolean	cameraUsed;
    qboolean	cameraSpectator;
    float		cameraAngle;
    qboolean	usebuttonDown;
    int			ns_ironsightState;
    float		ns_ironsightX;
    float		ns_ironsightY;
    float		ns_ironsightZ;
    vec3_t		ns_ironsightAngles;
    int			ns_ironsightTimer;
    qboolean	ns_ironsightDeactivate;
    newbeehelp_t ns_newbiehelp;

    int			damageDuration;
    vec3_t		weaponAngles;
    float		blendAlpha;
    float		savedblendAlpha;
    int			blendFadeTime;

    int	raise_time_acc;
    int	raise_time_spd;
    int	raise_time_str;
    int	raise_time_stl;
    int	raise_time_sta;
    int	raise_time_tec;
    int raise_level_acc ;
    int raise_level_spd  ;
    int raise_level_str  ;
    int raise_level_stl ;
    int raise_level_sta ;
    int raise_level_tec ;

    int	flashDmgLocTime[5];

    vmBlockedCvar_t	blockedCvars[MAX_BLOCKEDCVARS];
    int				num_blockedCvars;

    int	mapOriginX;
    int	mapOriginY;
    qboolean	mapVisible;

    qboolean	noMarkEntities[MAX_GENTITIES]; // don't create a mark for those entities
    int			DeathBlendTime;

    qboolean		thirdpersonCamera;
    qboolean		button3pushed;
    // Navy Seals --
    qboolean		viewCmd;
    qboolean		viewMissionInfo;

    int					radarNumEntities;
    radarEntity_t		radarEntities[MAX_CLIENTS];

    int					radarNumObjects;
    radarEntity_t		radarObjects[MAX_GENTITIES];

    qboolean		cheatsEnabled;
    int				deathTime;

    int				deathRotation;
    int				deathZoom;
    qboolean		deathCam;

    float			smokeBlendAlpha;

} cg_t;

#define IS_NONE		0
#define IS_PUTUP	1
#define IS_PUTAWAY	2
#define IS_IDLE		3

#define IS_RIFLE_TIME	400
#define IS_TIME			250

#define MAX_PLAYERMODELS	1

// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
    qhandle_t	charsetShader;
    qhandle_t	charsetProp;
    qhandle_t	charsetPropGlow;
    qhandle_t	charsetPropB;
    qhandle_t	whiteShader;

    qhandle_t	teamStatusBar;

    qhandle_t	deferShader;

    qhandle_t	friendShader;
    qhandle_t	connectionShader;

    qhandle_t	selectShader;
    qhandle_t	viewBloodShader;
    qhandle_t	tracerShader;
    qhandle_t	hdrShader;
	qhandle_t	motionblurShader; 
	qhandle_t	blurShader; 
    qhandle_t	lagometerShader;
    qhandle_t	backTileShader;

    qhandle_t	smokePuffShader;
    qhandle_t	smokePuffRageProShader;
    qhandle_t	shotgunSmokePuffShader;
    qhandle_t	waterBubbleShader;
 
    // wall mark shaders
    qhandle_t	wakeMarkShader;
    qhandle_t	burnMarkShaders[3];
    qhandle_t	shadowMarkShader;

    // weapon effect models
    qhandle_t	bulletFlashModel;

    // weapon effect shaders
    qhandle_t	bulletExplosionShader;
    qhandle_t	rocketExplosionShader;
    qhandle_t	grenadeExplosionShader;
    qhandle_t	bloodExplosionShader;
    qhandle_t	waterExplosionShader;

    // special effects models
    qhandle_t	teleportEffectModel;
    qhandle_t	teleportEffectShader;

    // Navy Seals ++
    qhandle_t	ammoMag;
    qhandle_t	ammoShell;

    qhandle_t	woodSmall;
    qhandle_t	woodMedium;
    qhandle_t	woodBig;
    qhandle_t	glassSmall;
    qhandle_t	glassMedium;
    qhandle_t	glassBig;
    qhandle_t	metalSmall;
    qhandle_t	metalMedium;
    qhandle_t	metalBig;
    qhandle_t	stoneSmall;
    qhandle_t	stoneMedium;
    qhandle_t	stoneBig;

    qhandle_t	nullModel;

    qhandle_t	ns_bloodtrailShader;

    qhandle_t	shellRifle;
    qhandle_t	shellPistol;
    qhandle_t	shellShotgun;

    qhandle_t	ns_bloodStain[15];
    qhandle_t	ns_bloodStainSmall[5];
    qhandle_t	ns_brainStain[5];
    qhandle_t	ns_bloodPool;

    qhandle_t	flare_circle_blue;
    qhandle_t	flare_circle_fadein;
    qhandle_t	flare_flare_green;
    qhandle_t	flare_flare_turkis;
    qhandle_t	flare_circle_green;
    qhandle_t	flare_circle_orange;
    qhandle_t	flare_circle_rainbow;

    qhandle_t	smalldigitalNumberShaders[11];
    qhandle_t	digitalNumberShaders[11];
    qhandle_t	bulletIcon;
    qhandle_t	clipIcon[5];
    qhandle_t	clockIcon;
    qhandle_t	colonIcon; // timer icons.
    qhandle_t	bombIcon;
    qhandle_t	assaultIcon;
    qhandle_t	vipIcon;

    qhandle_t	slashIcon;
    qhandle_t	healthIcon;
    qhandle_t	vestShaders[3];

    qhandle_t	loc_headIcon;
    qhandle_t	loc_leftArmIcon;
    qhandle_t	loc_rightArmIcon;
    qhandle_t	loc_leftLegIcon;
    qhandle_t	loc_rightLegIcon;
    qhandle_t	loc_stomachIcon;
    qhandle_t	loc_chestIcon;
    qhandle_t	loc_bodyLines;

    qhandle_t	laserShader;
    qhandle_t	radioIcon;
    qhandle_t	sniperScopeShader[6];
    qhandle_t	sphereFlashModel;

    qhandle_t	briefcaseModel;
    qhandle_t	briefcaseModel_vweap;

#ifdef IS_REDUNDANT
    qhandle_t	bloodHitShader;
#endif

    qhandle_t	bloodparticleShaders[5];
    qhandle_t	thermalGogglesShader;
    qhandle_t	thermalGogglesNoise;
    qhandle_t	flare;
    qhandle_t	flashedSpot;
    qhandle_t	smallFlare;

    qhandle_t	playerSealHelmet[4]; // arctic,desert,jungle,urban
    qhandle_t	playerTangoHelmet[4]; // arctic,desert,jungle,urban

    qhandle_t	woodSplinter;
    qhandle_t	glassSplinter;
    qhandle_t	softSplinter;

    qhandle_t	ammoMag_back;
    qhandle_t	ammoMag_bullet[3]; // 0 = pistol, 1 = rifle, 2 = shotgun
    qhandle_t	ammoMag_stripes[6];

    qhandle_t	weaponMenuActive[5];
    qhandle_t	weaponMenuInactive[5];
    qhandle_t	weaponSelInactiveBig;
    qhandle_t	weaponSelInactiveSmall;

    qhandle_t	bombCaseDigitModels[3];
    qhandle_t	bombCaseDigitShaders[11];

    qhandle_t	bombCaseWireModels[8];
    qhandle_t	bombCaseWireShaders[3]; // default, green,red

    qhandle_t	bombCaseModel;
    qhandle_t	bombCaseTangoSkin;

    sfxHandle_t	bulletHitFlesh[4];
    sfxHandle_t	bulletHitKevlar[3];
    sfxHandle_t	bulletHitHelmet[4];

    // debris sounds
    sfxHandle_t	sfxWood[4];
    sfxHandle_t sfxGlass[4];
    sfxHandle_t sfxMetal[4];
    sfxHandle_t	sfxShellHitWall[3];
    qhandle_t	dustPuffShader;
    qhandle_t	sparkShader;
    qhandle_t	metalsparkShader;

    qhandle_t	cursor;
    qhandle_t	sizeCursor;
    qhandle_t	selectCursor;


    // bulletholes, 5different for each surface
    qhandle_t	bulletholes[BHOLE_TOTAL][5];

    sfxHandle_t	mk26_explode;
    sfxHandle_t	c4_explode;
    sfxHandle_t	flashbang_explode;
    sfxHandle_t	flashbanged;
    sfxHandle_t	flash_2sec;
    sfxHandle_t	flash_4sec;
    sfxHandle_t	flash_6sec;
    sfxHandle_t	flash_8sec;
    sfxHandle_t newbeeMsgSound;
    // Navy Seals --
    // sounds
    sfxHandle_t	tracerSound;
    sfxHandle_t	selectSound;
    sfxHandle_t	footsteps[FOOTSTEP_TOTAL][4];

    sfxHandle_t	sfx_ric[IMPACTSOUND_TOTAL];

    sfxHandle_t	deathSounds[3];

    sfxHandle_t	hgrenb1aSound;
    sfxHandle_t	hgrenb2aSound;

    sfxHandle_t	teleInSound;
    sfxHandle_t	teleOutSound;
    sfxHandle_t	noAmmoSound;
    sfxHandle_t talkSound;
    sfxHandle_t landSound;
    sfxHandle_t fallSound;

    sfxHandle_t watrInSound;
    sfxHandle_t watrOutSound;
    sfxHandle_t watrUnSound;

    sfxHandle_t	roundDrawSound;

    // teamplay sounds
    sfxHandle_t redScoredSound;
    sfxHandle_t blueScoredSound;
    sfxHandle_t redLeadsSound;
    sfxHandle_t blueLeadsSound;
#ifdef IS_REDUNDANT
    sfxHandle_t	captureYourTeamSound;
#endif

#define MAX_PLAYER_MODELS	1

    char *SealPlayerModelNames[MAX_PLAYER_MODELS+1];
    char *TangoPlayerModelNames[MAX_PLAYER_MODELS+1];
    char *MiscPlayerModelNames[MAX_PLAYER_MODELS+1];

    clientInfo_t SealPlayerModels[MAX_PLAYER_MODELS];
    clientInfo_t TangoPlayerModels[MAX_PLAYER_MODELS];
    clientInfo_t MiscPlayerModels[MAX_PLAYER_MODELS];

} cgMedia_t;

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
    gameState_t		gameState;			// gamestate from server
    glconfig_t		glconfig;				// rendering configuration
    float			screenXScale;			// derived from glconfig
    float			screenYScale;
    float			screenXBias;

    int				serverCommandSequence;	// reliable command stream counter
    int				processedSnapshotNum;	// the number of snapshots cgame has requested

    qboolean		localServer;		// detected on startup by checking sv_running

    // parsed from serverinfo
    gametype_t		gametype;
    int				dmflags;
    int				teamflags;
    int				fraglimit;
    //	int				capturelimit;
    int				timelimit;
    int				maxclients;
    char			mapname[MAX_QPATH];
    char			redTeam[MAX_QPATH];
    char			blueTeam[MAX_QPATH];

    int				voteTime;
    int				voteYes;
    int				voteNo;
    qboolean		voteModified;			// beep whenever changed
    char			voteString[MAX_STRING_TOKENS];

    int				teamVoteTime[2];
    int				teamVoteYes[2];
    int				teamVoteNo[2];
    qboolean		teamVoteModified[2];	// beep whenever changed
    char			teamVoteString[2][MAX_STRING_TOKENS];

    int				levelStartTime;

    int				scores1, scores2;		// from configstrings
    int				redflag, blueflag;		// flag status from configstrings
    int				flagStatus;

    qboolean  newHud;

    //
    // locally derived information from gamestate
    //
    qhandle_t		gameModels[MAX_MODELS];
    sfxHandle_t		gameSounds[MAX_SOUNDS];

    int				numInlineModels;
    qhandle_t		inlineDrawModel[MAX_MODELS];
    vec3_t			inlineModelMidpoints[MAX_MODELS];

    clientInfo_t	clientinfo[MAX_CLIENTS];

    char			ChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
    int				ChatMsgTimes[TEAMCHAT_HEIGHT];
    int				ChatPos;
    int				LastChatPos;
    // teamchat width is *3 because of embedded color codes
    char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
    int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
    int				teamChatPos;
    int				teamLastChatPos;

    int cursorX;
    int cursorY;
    qboolean eventHandling;
    qboolean mouseCaptured;
    qboolean sizingHud;
    void *capturedItem;
    qhandle_t activeCursor;

    // orders
    int currentOrder;
    qboolean orderPending;
    int orderTime;
    int currentVoiceClient;
    char acceptVoice[MAX_NAME_LENGTH];

    // media
    cgMedia_t		media;
    // Navy Seals ++
    int				teampointlimit;
    int				levelRoundStartTime;	// from Configstrings
    int				levelServerTime;
    int				levelVipStartTime;
    int				levelAssaultStartTime[4];
    int				levelBombStartTime;
    int				teamRespawn;
    int				squadAssault;
    qboolean		assaultFieldsCaptured[4];

#define MAX_HELPERS 3
    vec3_t			mi_helpers[MAX_HELPERS];

    // mapinfo
    int				mi_assaultFields;
    qboolean		mi_bombSpot;
    qboolean		mi_vipRescue;
    qboolean		mi_vipTime;
    qboolean		mi_sealBriefcase;

    int				roundtime;
    int				WaterTime;

    int				camoType;
    char			vipType[MAX_NAME_LENGTH];
    char			cleanMapName[MAX_QPATH];
    char			cleanLastMap[MAX_QPATH];

#define MAX_HELP_LINES		256
#define MAX_CHARS_PER_LINE	512
    char			helpGameMessages[ MAX_HELP_LINES ][ MAX_CHARS_PER_LINE ]; // max chars per line
    char			helpMissionMessages[ MAX_HELP_LINES*2 ][ MAX_CHARS_PER_LINE ]; // max chars per line
    int				helpNumGameMessages;
    int				helpNumMissionMessages;
    qboolean		helpNumDisplayedGameMessage[ MAX_HELP_LINES ];
    qboolean		helpNumDisplayedMissionMessage[ MAX_HELP_LINES*2 ];
    int				helpLastMessageTime;
    qhandle_t		infoPicLeft,infoPicMiddle,infoPicRight;
    int				matchlockMode;

    // Navy Seals --
} cgs_t;

void CG_NewbieMessage( const char *str, int y, float charHeight );
void CG_HandleHelp( void );
qboolean CG_ParseHelpFile( void );
//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[WP_NUM_WEAPONS*2];//[MAX_WEAPONS]; had to fix this
extern	itemInfo_t		cg_items[MAX_ITEMS];
extern	markPoly_t		cg_markPolys[MAX_MARK_POLYS];

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_bobup;
extern	vmCvar_t		cg_bobpitch;
extern	vmCvar_t		cg_bobroll;
extern	vmCvar_t		cg_swingSpeed;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_gibs;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawAmmoWarning;
extern	vmCvar_t		cg_drawCrosshair;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_teamOverlayUserinfo;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_crosshairHealth;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_animSpeed;
extern	vmCvar_t		cg_debugAnim;
extern	vmCvar_t		cg_debugPosition;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_noPlayerAnims;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_footsteps;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_brassTime;
extern	vmCvar_t		cg_gun_frame;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_drawGun;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_tracerChance;
extern	vmCvar_t		cg_tracerWidth;
extern	vmCvar_t		cg_tracerLength;
extern	vmCvar_t		cg_autoswitch;
extern	vmCvar_t		cg_ignore;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_zoomFov;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_drawAttacker;
extern	vmCvar_t		cg_synchronousClients;
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
extern	vmCvar_t		cg_stats;
extern	vmCvar_t 		cg_buildScript;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_blood;
extern	vmCvar_t		cg_deferPlayers;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_teamChatsOnly;
extern	vmCvar_t		cg_noVoiceChats;
extern	vmCvar_t		cg_noVoiceText;
extern  vmCvar_t		cg_scorePlum;
extern	vmCvar_t		cg_smoothClients;
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
extern	vmCvar_t		cg_cameraOrbit;
extern	vmCvar_t		cg_cameraOrbitDelay;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern	vmCvar_t		cg_cameraMode;
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;
extern	vmCvar_t		cg_debugAlloc;
extern	vmCvar_t		r_hdr;
extern	vmCvar_t		r_motionblur;
extern	vmCvar_t		r_blur;
// Navy Seals ++
// i use these for debbuging my hud
extern	vmCvar_t	test_x;
extern	vmCvar_t	test_y;
extern	vmCvar_t	test_h;
extern	vmCvar_t	test_w;
extern  vmCvar_t	cg_gunSmoke;
extern  vmCvar_t	pmodel_o;
extern	vmCvar_t	cg_enableBreath;
extern	vmCvar_t	cg_enableDust;
extern	vmCvar_t	cg_enableTimeSelect;
extern  vmCvar_t  	cg_atmosphericEffects;
extern  vmCvar_t  	cg_lowEffects;
extern  vmCvar_t	cg_gunSmokeTime;
extern  vmCvar_t	cg_particleTime;

extern  vmCvar_t	char_stamina;
extern  vmCvar_t	char_stealth;
extern  vmCvar_t	char_speed;
extern  vmCvar_t	char_accuracy;
extern  vmCvar_t	char_technical;
extern  vmCvar_t	char_strength;
extern  vmCvar_t	char_xp;
extern  vmCvar_t	ui_team;

extern  vmCvar_t	cg_bulletTracerLength;
extern  vmCvar_t	cg_bulletTracerWidth;

extern  vmCvar_t	cg_wakemarkTime;
extern  vmCvar_t	cg_wakemarkDistantTime;

extern  vmCvar_t	mi_viprescue;
extern  vmCvar_t	mi_viptime;
extern  vmCvar_t	mi_blowup;
extern  vmCvar_t	mi_assaultfield;

extern  vmCvar_t	ui_gotbomb;
extern  vmCvar_t	ui_isvip;
extern  vmCvar_t	ui_gotbriefcase;

extern	vmCvar_t	cg_grenadeSparks;
extern	vmCvar_t	cg_correctgunFov;

extern	vmCvar_t	cg_newbeeHeight;

extern	vmCvar_t	cg_disableHeadstuff;
extern  vmCvar_t	cg_disableTangoHandSkin;
extern vmCvar_t    raise_acc;
extern vmCvar_t    raise_spd;
extern vmCvar_t    raise_str;
extern vmCvar_t    raise_stl;
extern vmCvar_t    raise_sta;
extern vmCvar_t    raise_tec;

extern	vmCvar_t	ui_teampointlimit;
extern	vmCvar_t	ui_timelimit;
extern	vmCvar_t	ui_roundtime;
extern	vmCvar_t	ui_friendlyfire;

extern	vmCvar_t	cg_chatTime;
extern	vmCvar_t	cg_chatHeight;
extern	vmCvar_t	cg_showConsole;
extern	vmCvar_t	cg_chatBeep;

extern	vmCvar_t	cg_goreLevel;
extern	vmCvar_t	cg_antiLag;

extern	vmCvar_t	cg_autoReload;

extern	vmCvar_t	cg_newbeeTime;

extern vmCvar_t	cg_qcmd_posx;
extern vmCvar_t	cg_qcmd_posy;
extern vmCvar_t	cg_qcmd_cmd1;
extern vmCvar_t	cg_qcmd_cmd2;
extern vmCvar_t	cg_qcmd_cmd3;
extern vmCvar_t	cg_qcmd_cmd4;
extern vmCvar_t	cg_qcmd_cmd5;
extern vmCvar_t	cg_qcmd_cmd6;
extern vmCvar_t	cg_qcmd_cmd7;
extern vmCvar_t	cg_qcmd_cmd8;
extern vmCvar_t	cg_qcmd_cmd9;
extern vmCvar_t	cg_qcmd_cmd0;
extern vmCvar_t	cg_qcmd_dscr1;
extern vmCvar_t	cg_qcmd_dscr2;
extern vmCvar_t	cg_qcmd_dscr3;
extern vmCvar_t	cg_qcmd_dscr4;
extern vmCvar_t	cg_qcmd_dscr5;
extern vmCvar_t	cg_qcmd_dscr6;
extern vmCvar_t	cg_qcmd_dscr7;
extern vmCvar_t	cg_qcmd_dscr8;
extern vmCvar_t	cg_qcmd_dscr9;
extern vmCvar_t	cg_qcmd_dscr0;
extern vmCvar_t	cg_qcmd_size;
extern vmCvar_t	cg_qcmd_r;
extern vmCvar_t	cg_qcmd_g;
extern vmCvar_t	cg_qcmd_b;
extern vmCvar_t	cg_qcmd_a;
 
extern vmCvar_t cg_crosshairFade;
extern vmCvar_t cg_lowAmmoWarning;

// Navy Seals --
//
// cg_main.c
//
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... );
void QDECL CG_Error( const char *msg, ... );

void CG_StartMusic( void );
void CG_PrecacheWeapons( );
void CG_UpdateCvars( void );

int CG_CrosshairPlayer( void );
void CG_LoadMenus(const char *menuFile);
void CG_KeyEvent(int key, qboolean down);
void CG_MouseEvent(int x, int y);
void CG_EventHandling(int type);
void CG_RankRunFrame( void );
void CG_SetScoreSelection(void *menu);
score_t *CG_GetSelectedScore();
void CG_BuildSpectatorString();
void CG_ForceCvar( const char *cvar, int value );

//
// cg_view.c
//
void CG_TestModel_f (void);
void CG_TestGun_f (void);
void CG_TestModelNextFrame_f (void);
void CG_TestModelPrevFrame_f (void);
void CG_TestModelNextSkin_f (void);
void CG_TestModelPrevSkin_f (void);
void CG_ZoomDown_f( void );
void CG_ZoomUp_f( void );
void CG_AddBufferedSound( sfxHandle_t sfx);

void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback );


//
// cg_drawtools.c
//
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
void CG_FillRect( float x, float y, float width, float height, const float *color );
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader );
void CG_DrawString( float x, float y, const char *string,
                    float charWidth, float charHeight, const float *modulate );


void CG_DrawStringExt( int x, int y, const char *string, const float *setColor,
                       qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars );
void CG_DrawStringExt2( int x, int y, const char *string, const float *setColor,
                        qboolean forceColor, qboolean outline, int charWidth, int charHeight, int maxChars );
void CG_DrawBigString( int x, int y, const char *s, float alpha );
void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color );
// Navy Seals ++
void CG_DrawStringOutline( int x, int y, const char *s, vec4_t color );
void CG_DrawTinyStringColor( int x, int y, const char *s, vec4_t color );
// Navy Seals --
int CG_DrawStrlen( const char *str );

float	*CG_FadeColor( int startMsec, int totalMsec );
float *CG_TeamColor( int team );
void CG_TileClear( void );
void CG_ColorForHealth( vec4_t hcolor );
void CG_GetColorForHealth( int health, int armor, vec4_t hcolor );

void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color );
void CG_DrawSides(float x, float y, float w, float h, float size);
void CG_DrawTopBottom(float x, float y, float w, float h, float size);


//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;
extern  char systemChat[256];
extern  char teamChat1[256];
extern  char teamChat2[256];

#define SAME_WEAPONPIPE

void CG_AddLagometerFrameInfo( void );
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_CenterPrint( const char *str, int y, int charWidth );
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles );
void CG_DrawActive( stereoFrame_t stereoView );
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D );
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team );
void CG_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle);
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style);
int CG_Text_Width(const char *text, float scale, int limit);
int CG_Text_Height(const char *text, float scale, int limit);
void CG_SelectPrevPlayer();
void CG_SelectNextPlayer();
float CG_GetValue(int ownerDraw);
qboolean CG_OwnerDrawVisible(int flags);
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat();
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetGameStatusText();
const char *CG_GetKillerText();
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles );
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
void CG_CheckOrderPending();
const char *CG_GameTypeString();
qboolean CG_YourTeamHasFlag();
qboolean CG_OtherTeamHasFlag();
qhandle_t CG_StatusHandle(int task);



//
// cg_player.c
//
void CG_Player( centity_t *cent );
void CG_ResetPlayerEntity( centity_t *cent );
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team );
void CG_NewClientInfo( int clientNum );
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName );

//
// cg_predict.c
//
void CG_BuildSolidList( void );
int	CG_PointContents( const vec3_t point, int passEntityNum );
void CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
               int skipNumber, int mask );
void CG_PredictPlayerState( void );
void CG_LoadDeferredPlayers( void );


//
// cg_events.c
//
void CG_CheckEvents( centity_t *cent );
const char	*CG_PlaceString( int rank );
void CG_EntityEvent( centity_t *cent, vec3_t position );
void CG_PainEvent( centity_t *cent, int health );


//
// cg_ents.c
//
void CG_SetEntitySoundPosition( centity_t *cent );
void CG_AddPacketEntities( void );
void CG_Beam( centity_t *cent );
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );

void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
                             qhandle_t parentModel, char *tagName );
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent,
                                    qhandle_t parentModel, char *tagName );



//
// cg_mem.c
//
void CG_GameMem_f( void );
void *CG_Alloc( int size );
void CG_InitMemory( void );

//
//
// cg_weapons.c
//
void CG_NextWeapon_f( void );
void CG_PrevWeapon_f( void );
void CG_Weapon_f( void );

void CG_RegisterWeapon( int weaponNum );
void CG_RegisterItemVisuals( int itemNum );

void CG_FireWeapon( centity_t *cent , qboolean othermode );
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, int type, int surfaceparms);
void CG_ShotgunFire( entityState_t *es );
void CG_Bullet( vec3_t end, vec3_t start, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, int damage, int soundtype, int bholetype, int weapon );
void CG_DrawWeaponSelect( void );
void CG_OutOfAmmoChange( void );	// should this be in pmove?
void CG_40mmGrenadeTrail( centity_t *ent, const weaponInfo_t *wi );

//
// cg_marks.c
//
void	CG_InitMarkPolys( void );
void	CG_AddMarks( void );
void	CG_ImpactMark( qhandle_t markShader,
                    const vec3_t origin, const vec3_t dir,
                    float orientation,
                    float r, float g, float b, float a,
                    qboolean alphaFade,
                    float radius, qboolean temporary );

//
// cg_localents.c
//
void	CG_InitLocalEntities( void );
localEntity_t	*CG_AllocLocalEntity( void );
void	CG_AddLocalEntities( qboolean thirdperson );

//
// cg_effects.c
//
localEntity_t *CG_SmokePuff( const vec3_t p,
                             const vec3_t vel,
                             float radius,
                             float r, float g, float b, float a,
                             float duration,
                             int startTime,
                             int fadeInTime,
                             int leFlags,
                             qhandle_t hShader );
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing );
void CG_SpawnEffect( vec3_t org );
#ifdef MISSIONPACK
void CG_KamikazeEffect( vec3_t org );
void CG_ObeliskExplode( vec3_t org, int entityNum );
void CG_ObeliskPain( vec3_t org );
void CG_InvulnerabilityImpact( vec3_t org, vec3_t angles );
void CG_InvulnerabilityJuiced( vec3_t org );
void CG_LightningBoltBeam( vec3_t start, vec3_t end );
#endif
void CG_ScorePlum( int client, vec3_t org, int score );

void CG_GibPlayer( vec3_t playerOrigin );

void CG_Bleed( vec3_t origin, int entityNum );

localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir,
                                 qhandle_t hModel, qhandle_t shader, int msec,
                                 qboolean isSprite );

//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );

//
// cg_info.c
//
void CG_LoadingString( const char *s );
void CG_LoadingItem( int itemNum );
void CG_LoadingClient( int clientNum );
void CG_DrawInformation( void );

//
// cg_scoreboard.c
//
qboolean CG_DrawOldScoreboard( void );
void CG_DrawOldTourneyScoreboard( void );

//
// cg_consolecmds.c
//
qboolean CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );
void CG_LoadVoiceChats( void );
void CG_ShaderStateChanged(void);
void CG_VoiceChatLocal( int mode, qboolean voiceOnly, int clientNum, int color, const char *cmd );
void CG_PlayBufferedVoiceChats( void );

//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_t *ps, playerState_t *ops );
void CG_CheckChangedPredictableEvents( playerState_t *ps );


//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// print message on the local console
void		trap_Print( const char *fmt );

// abort the game
void		trap_Error( const char *fmt );

// milliseconds should only be used for performance tuning, never
// for anything game related.  Get time from the CG_DrawActiveFrame parameter
int			trap_Milliseconds( void );

// console variable interaction
void		trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void		trap_Cvar_Update( vmCvar_t *vmCvar );
void		trap_Cvar_Set( const char *var_name, const char *value );
void		trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

// ServerCommand and ConsoleCommand parameter access
int			trap_Argc( void );
void		trap_Argv( int n, char *buffer, int bufferLength );
void		trap_Args( char *buffer, int bufferLength );

// filesystem access
// returns length of file
int			trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void		trap_FS_Read( void *buffer, int len, fileHandle_t f );
void		trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void		trap_FS_FCloseFile( fileHandle_t f );

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

// force a screen update, only used during gamestate load
void		trap_UpdateScreen( void );

// model collision
void		trap_CM_LoadMap( const char *mapname );
int			trap_CM_NumInlineModels( void );
clipHandle_t trap_CM_InlineModel( int index );		// 0 = world, 1+ = bmodels
clipHandle_t trap_CM_TempBoxModel( const vec3_t mins, const vec3_t maxs );
int			trap_CM_PointContents( const vec3_t p, clipHandle_t model );
int			trap_CM_TransformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
void		trap_CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
                        const vec3_t mins, const vec3_t maxs,
                        clipHandle_t model, int brushmask );
void		trap_CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
                                   const vec3_t mins, const vec3_t maxs,
                                   clipHandle_t model, int brushmask,
                                   const vec3_t origin, const vec3_t angles );

// Returns the projection of a polygon onto the solid brushes in the world
int			trap_CM_MarkFragments( int numPoints, const vec3_t *points,
                             const vec3_t projection,
                             int maxPoints, vec3_t pointBuffer,
                             int maxFragments, markFragment_t *fragmentBuffer );

// normal sounds will have their volume dynamically changed as their entity
// moves and the listener moves
void		trap_S_StartSound( vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx );
void		trap_S_StartSoundExtended( vec3_t origin, float volume, float rolloff, float pitch, int entityNum, int entchannel, sfxHandle_t sfx );
void		trap_S_StopLoopingSound(int entnum);

// a local sound is always played full volume
void		trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
void		trap_S_ClearLoopingSounds( qboolean killall );
void		trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void		trap_S_UpdateEntityPosition( int entityNum, const vec3_t origin );

// repatialize recalculates the volumes of sound as they should be heard by the
// given entityNum and position
void		trap_S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );
sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed );		// returns buzz if not found
void		trap_S_StartBackgroundTrack( const char *intro, const char *loop );		// empty name stops music
void	trap_S_StopBackgroundTrack( void );


void		trap_R_LoadWorldMap( const char *mapname );

// all media should be registered during level startup to prevent
// hitches during gameplay
qhandle_t	trap_R_RegisterModel( const char *name );			// returns rgb axis if not found
qhandle_t	trap_R_RegisterPermanentModel( const char *name );			// returns rgb axis if not found 
qhandle_t	trap_R_RegisterSkin( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShader( const char *name );			// returns all white if not found
qhandle_t	trap_R_RegisterShaderNoMip( const char *name );			// returns all white if not found

// a scene is built up by calls to R_ClearScene and the various R_Add functions.
// Nothing is drawn until R_RenderScene is called.
void		trap_R_ClearScene( void );
void		trap_R_AddRefEntityToScene( const refEntity_t *re );

// polys are intended for simple wall marks, not really for doing
// significant construction
void		trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts );
void		trap_R_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void		trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
int			trap_R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
void		trap_R_RenderScene( const refdef_t *fd );
void		trap_R_SetColor( const float *rgba );	// NULL = 1,1,1,1
void		trap_R_DrawStretchPic( float x, float y, float w, float h,
                             float s1, float t1, float s2, float t2, qhandle_t hShader );
void		trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
int			trap_R_LerpTag( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame,
                      float frac, const char *tagName );
void		trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );

// The glconfig_t will not change during the life of a cgame.
// If it needs to change, the entire cgame will be restarted, because
// all the qhandle_t are then invalid.
void		trap_GetGlconfig( glconfig_t *glconfig );

// the gamestate should be grabbed at startup, and whenever a
// configstring changes
void		trap_GetGameState( gameState_t *gamestate );

// cgame will poll each frame to see if a newer snapshot has arrived
// that it is interested in.  The time is returned seperately so that
// snapshot latency can be calculated.
void		trap_GetCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );

// a snapshot get can fail if the snapshot (or the entties it holds) is so
// old that it has fallen out of the client system queue
qboolean	trap_GetSnapshot( int snapshotNumber, snapshot_t *snapshot );

// retrieve a text command from the server stream
// the current snapshot will hold the number of the most recent command
// qfalse can be returned if the client system handled the command
// argc() / argv() can be used to examine the parameters of the command
qboolean	trap_GetServerCommand( int serverCommandNumber );

// returns the most recent command number that can be passed to GetUserCmd
// this will always be at least one higher than the number in the current
// snapshot, and it may be quite a few higher if it is a fast computer on
// a lagged connection
int			trap_GetCurrentCmdNumber( void );

qboolean	trap_GetUserCmd( int cmdNumber, usercmd_t *ucmd );

// used for the weapon select and zoom
void		trap_SetUserCmdValue( int stateValue, float sensitivityScale );

// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );

int			trap_MemoryRemaining( void );
void		trap_R_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);
qboolean	trap_Key_IsDown( int keynum );
int			trap_Key_GetCatcher( void );
void		trap_Key_SetCatcher( int catcher );
int			trap_Key_GetKey( const char *binding );


typedef enum {
    SYSTEM_PRINT,
    CHAT_PRINT,
    TEAMCHAT_PRINT
};


int trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status trap_CIN_StopCinematic(int handle);
e_status trap_CIN_RunCinematic (int handle);
void trap_CIN_DrawCinematic (int handle);
void trap_CIN_SetExtents (int handle, int x, int y, int w, int h);

void trap_SnapVector( float *v );

#include "cg_seals.h"
