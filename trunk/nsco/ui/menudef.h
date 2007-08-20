
#define ITEM_TYPE_TEXT 0                  // simple text
#define ITEM_TYPE_BUTTON 1                // button, basically text with a border 
#define ITEM_TYPE_RADIOBUTTON 2           // toggle button, may be grouped 
#define ITEM_TYPE_CHECKBOX 3              // check box
#define ITEM_TYPE_EDITFIELD 4             // editable text, associated with a cvar
#define ITEM_TYPE_COMBO 5                 // drop down list
#define ITEM_TYPE_LISTBOX 6               // scrollable list  
#define ITEM_TYPE_MODEL 7                 // model
#define ITEM_TYPE_OWNERDRAW 8             // owner draw, name specs what it is
#define ITEM_TYPE_NUMERICFIELD 9          // editable text, associated with a cvar
#define ITEM_TYPE_SLIDER 10               // mouse speed, volume, etc.
#define ITEM_TYPE_YESNO 11                // yes no cvar setting
#define ITEM_TYPE_MULTI 12                // multiple list setting, enumerated
#define ITEM_TYPE_BIND 13		          // multiple list setting, enumerated
    
#define ITEM_ALIGN_LEFT 0                 // left alignment
#define ITEM_ALIGN_CENTER 1               // center alignment
#define ITEM_ALIGN_RIGHT 2                // right alignment

#define ITEM_TEXTSTYLE_NORMAL 0           // normal text
#define ITEM_TEXTSTYLE_BLINK 1            // fast blinking
#define ITEM_TEXTSTYLE_PULSE 2            // slow pulsing
#define ITEM_TEXTSTYLE_SHADOWED 3         // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_OUTLINED 4         // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_OUTLINESHADOWED 5  // drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_SHADOWEDMORE 6     // drop shadow ( need a color for this )
                          
#define WINDOW_BORDER_NONE 0              // no border
#define WINDOW_BORDER_FULL 1              // full border based on border color ( single pixel )
#define WINDOW_BORDER_HORZ 2              // horizontal borders only
#define WINDOW_BORDER_VERT 3              // vertical borders only 
#define WINDOW_BORDER_KCGRADIENT 4        // horizontal border using the gradient bars
  
#define WINDOW_STYLE_EMPTY 0              // no background
#define WINDOW_STYLE_FILLED 1             // filled with background color
#define WINDOW_STYLE_GRADIENT 2           // gradient bar based on background color 
#define WINDOW_STYLE_SHADER   3           // gradient bar based on background color 
#define WINDOW_STYLE_TEAMCOLOR 4          // team color
#define WINDOW_STYLE_CINEMATIC 5          // cinematic

#define MENU_TRUE 1                       // uh.. true
#define MENU_FALSE 0                      // and false

#define HUD_VERTICAL				0x00
#define HUD_HORIZONTAL				0x01

// list box element types
#define LISTBOX_TEXT  0x00
#define LISTBOX_IMAGE 0x01

// list feeders
#define FEEDER_HEADS						0x00			// model heads
#define FEEDER_MAPS							0x01			// text maps based on game type
#define FEEDER_SERVERS						0x02			// servers
#define FEEDER_CLANS						0x03			// clan names
#define FEEDER_ALLMAPS						0x04			// all maps available, in graphic format
#define FEEDER_REDTEAM_LIST					0x05			// red team members
#define FEEDER_BLUETEAM_LIST				0x06			// blue team members
#define FEEDER_PLAYER_LIST					0x07			// players
#define FEEDER_TEAM_LIST					0x08			// team members for team voting
#define FEEDER_MODS							0x09			// team members for team voting
#define FEEDER_DEMOS 						0x0a			// team members for team voting
#define FEEDER_SCOREBOARD					0x0b			// team members for team voting
#define FEEDER_Q3HEADS		 				0x0c			// model heads
#define FEEDER_SERVERSTATUS					0x0d			// server status
#define FEEDER_FINDPLAYER					0x0e			// find player
#define FEEDER_CINEMATICS					0x0f			// cinematics
#define FEEDER_SCRIPTS						0x10			// scripts in nssl/

// display flags
#define CG_SHOW_BLUE_TEAM_HAS_REDFLAG     0x00000001
#define CG_SHOW_RED_TEAM_HAS_BLUEFLAG     0x00000002
#define CG_SHOW_ANYTEAMGAME               0x00000004
#define CG_SHOW_HARVESTER                 0x00000008
#define CG_SHOW_ONEFLAG                   0x00000010
#define CG_SHOW_CTF                       0x00000020
#define CG_SHOW_OBELISK                   0x00000040
#define CG_SHOW_HEALTHCRITICAL            0x00000080
#define CG_SHOW_SINGLEPLAYER              0x00000100
#define CG_SHOW_TOURNAMENT                0x00000200
#define CG_SHOW_DURINGINCOMINGVOICE       0x00000400
#define CG_SHOW_IF_PLAYER_HAS_FLAG	  0x00000800
#define CG_SHOW_LANPLAYONLY		  0x00001000
#define CG_SHOW_MINED			  0x00002000
#define CG_SHOW_HEALTHOK		  0x00004000
#define CG_SHOW_TEAMINFO		  0x00008000
#define CG_SHOW_NOTEAMINFO		  0x00010000
#define CG_SHOW_OTHERTEAMHASFLAG	  0x00020000
#define CG_SHOW_YOURTEAMHASENEMYFLAG	  0x00040000
#define CG_SHOW_ANYNONTEAMGAME		  0x00080000
#define CG_SHOW_2DONLY			  0x10000000
#define CG_SHOW_SPECTATOR		  0x20000000
#define CG_SHOW_NO_SPECTATOR		  0x40000000

#define UI_SHOW_LEADER				                0x00000001
#define UI_SHOW_NOTLEADER				        0x00000002
#define UI_SHOW_FAVORITESERVERS					0x00000004
#define UI_SHOW_ANYNONTEAMGAME					0x00000008
#define UI_SHOW_ANYTEAMGAME					0x00000010
#define UI_SHOW_NEWHIGHSCORE					0x00000020
#define UI_SHOW_DEMOAVAILABLE					0x00000040
#define UI_SHOW_NEWBESTTIME					0x00000080
#define UI_SHOW_FFA						0x00000100
#define UI_SHOW_NOTFFA						0x00000200
#define UI_SHOW_NETANYNONTEAMGAME	 			0x00000400
#define UI_SHOW_NETANYTEAMGAME		 			0x00000800
#define UI_SHOW_NOTFAVORITESERVERS				0x00001000 
 




// owner draw types
// ideally these should be done outside of this file but
// this makes it much easier for the macro expansion to 
// convert them for the designers ( from the .menu files )
#define CG_OWNERDRAW_BASE			1

#define CG_PLAYER_ROUNDS_ICON		1              
#define CG_PLAYER_ROUNDS_VALUE		2

#define CG_PLAYER_HEAD				3
#define CG_PLAYER_HEALTH			4

#define CG_PLAYER_AMMO_ICON			5
#define CG_PLAYER_AMMO_VALUE		6

#define CG_SELECTEDPLAYER_HEAD		7

#define CG_SELECTEDPLAYER_NAME		8
#define CG_SELECTEDPLAYER_LOCATION	9
#define CG_SELECTEDPLAYER_STATUS	10
#define CG_SELECTEDPLAYER_WEAPON	11
#define CG_SELECTEDPLAYER_POWERUP	12

#define CG_FLAGCARRIER_HEAD 13
#define CG_FLAGCARRIER_NAME 14
#define CG_FLAGCARRIER_LOCATION 15
#define CG_FLAGCARRIER_STATUS 16
#define CG_FLAGCARRIER_WEAPON 17
#define CG_FLAGCARRIER_POWERUP 18

#define CG_PLAYER_ITEM 19
#define CG_PLAYER_SCORE 20

#define CG_BLUE_FLAGHEAD 21
#define CG_BLUE_FLAGSTATUS 22
#define CG_BLUE_FLAGNAME 23
#define CG_RED_FLAGHEAD 24
#define CG_RED_FLAGSTATUS 25
#define CG_RED_FLAGNAME 26

#define CG_BLUE_SCORE 27
#define CG_RED_SCORE 28
#define CG_RED_NAME 29
#define CG_BLUE_NAME 30
#define CG_HARVESTER_SKULLS 31					// only shows in harvester
#define CG_ONEFLAG_STATUS 32						// only shows in one flag
#define CG_PLAYER_LOCATION 33
#define CG_TEAM_COLOR 34
#define CG_CTF_POWERUP 35
                                        
#define CG_AREA_POWERUP	36
#define CG_AREA_LAGOMETER	37            // painted with old system
#define CG_PLAYER_HASFLAG 38            
#define CG_GAME_TYPE 39                 // not done

#define CG_SELECTEDPLAYER_ARMOR 40      
#define CG_SELECTEDPLAYER_HEALTH 41
#define CG_PLAYER_STATUS 42
#define CG_FRAGGED_MSG 43               // painted with old system
#define CG_PROXMINED_MSG 44             // painted with old system
#define CG_AREA_FPSINFO 45              // painted with old system
#define CG_AREA_SYSTEMCHAT 46           // painted with old system
#define CG_AREA_TEAMCHAT 47             // painted with old system
#define CG_AREA_CHAT 48                 // painted with old system
#define CG_GAME_STATUS 49
#define CG_KILLER 50
#define CG_PLAYER_ARMOR_ICON2D 51              
#define CG_PLAYER_AMMO_ICON2D 52
#define CG_ACCURACY 53
#define CG_ASSISTS 54
#define CG_DEFEND 55
#define CG_EXCELLENT 56
#define CG_IMPRESSIVE 57
#define CG_PERFECT 58
#define CG_GAUNTLET 59
#define CG_SPECTATORS 60
#define CG_TEAMINFO 61
#define CG_VOICE_HEAD 62
#define CG_VOICE_NAME 63
#define CG_PLAYER_HASFLAG2D 64            
#define CG_HARVESTER_SKULLS2D 65					// only shows in harvester
#define CG_CAPFRAGLIMIT 66	 
#define CG_1STPLACE 67
#define CG_2NDPLACE 68
#define CG_CAPTURES 69

#define UI_OWNERDRAW_BASE 200
#define UI_HANDICAP 200
#define UI_EFFECTS 201
#define UI_PLAYERMODEL 202
#define UI_CLANNAME 203
#define UI_CLANLOGO 204
#define UI_GAMETYPE 205
#define UI_MAPPREVIEW 206
#define UI_SKILL 207
#define UI_BLUETEAMNAME 208
#define UI_REDTEAMNAME 209
#define UI_NETSOURCE 220
#define UI_NETMAPPREVIEW 221
#define UI_NETFILTER 222
#define UI_TIER 223
#define UI_OPPONENTMODEL 224
#define UI_TIERMAP1 225
#define UI_TIERMAP2 226
#define UI_TIERMAP3 227
#define UI_PLAYERLOGO 228
#define UI_OPPONENTLOGO 229
#define UI_PLAYERLOGO_METAL 230
#define UI_OPPONENTLOGO_METAL 231
#define UI_PLAYERLOGO_NAME 232
#define UI_OPPONENTLOGO_NAME 233
#define UI_TIER_MAPNAME 234
#define UI_TIER_GAMETYPE 235
#define UI_ALLMAPS_SELECTION 236
#define UI_OPPONENT_NAME 237
#define UI_VOTE_KICK 238
#define UI_BOTNAME 239
#define UI_BOTSKILL 240
#define UI_REDBLUE 241
#define UI_CROSSHAIR 242
#define UI_SELECTEDPLAYER 243
#define UI_MAPCINEMATIC 244
#define UI_NETGAMETYPE 245
#define UI_NETMAPCINEMATIC 246
#define UI_SERVERREFRESHDATE 247
#define UI_SERVERMOTD 248
#define UI_GLINFO  249
#define UI_KEYBINDSTATUS 250
#define UI_CLANCINEMATIC 251
#define UI_MAP_TIMETOBEAT 252
#define UI_JOINGAMETYPE 253
#define UI_PREVIEWCINEMATIC 254
#define UI_STARTMAPCINEMATIC 255
#define UI_MAPS_SELECTION 256
#define CG_PICKUP 257
#define CG_PRIMARY	258
#define CG_SECONDARY	259
#define CG_CHARACTERBUTTON 260
#define UI_SEAL_HEADMODEL 261
#define UI_TANGO_HEADMODEL 262
#define UI_SEAL_PLAYERMODEL 263
#define UI_TANGO_PLAYERMODEL 264

// height scalable, rounds bar, drawed using bullet graphics
#define CG_PLAYER_CLIPGFX	265
// value for clipgfx this!
#define CG_PLAYER_CLIPVALUE	266

// damage locator , scalable
#define CG_PLAYER_DAMAGELOC	267
// rounds bar hor, scalable
#define CG_PLAYER_AMMOBAR_HOR	268
// rounds bar vert, scalable
#define CG_PLAYER_AMMOBAR_VERT	269

// clip icon ( grenade/rifle/shotgun etc...), scalable
#define CG_PLAYER_CLIPS_ICON	270
// clip value amount of clips left draw using digits, scalable
#define CG_PLAYER_CLIPS_VALUE	271

#define CG_TIMER_WORLD		272 // world timer, roundtime/sp server time
#define CG_TIMER_VIP		273 // vip timer
#define CG_TIMER_ASSAULT1	274 // assault timer (more required!!!!!!!!!!!!!!!!!!!!!)
#define CG_TIMER_ASSAULT2   295
#define CG_TIMER_ASSAULT3   296
#define CG_TIMER_ASSAULT4   297
#define CG_TIMER_BOMB		298


// stamina bar, no border, supply with shader!, both scalable
#define CG_PLAYER_STAMINABAR_HOR	275
#define CG_PLAYER_STAMINABAR_VERT	276

#define CG_PLAYER_WEAPONSTATUS		277

#define UI_PRIMARY_ADDON1		278
#define UI_PRIMARY_ADDON2		279
#define UI_PRIMARY_ADDON3		280

#define UI_SIDEARM_ADDON1		281
#define UI_SIDEARM_ADDON2		282

#define UI_TOTALWEIGHT			283

#define UI_TEXT_PRIMARY			284
#define UI_TEXT_SIDEARM			285

#define CG_TEXT_PICKUP			286 // display not pickup icon /do text
 

#define CG_CHARACTER_STRENGTH		287
#define CG_CHARACTER_TECHINCAL		288
#define CG_CHARACTER_STAMINA		289
#define CG_CHARACTER_ACCURACY		290
#define CG_CHARACTER_SPEED		291
#define CG_CHARACTER_STEALTH		292

#define UI_BLUETEAM1 300
#define UI_BLUETEAM2 301
#define UI_BLUETEAM3 302
#define UI_BLUETEAM4 303
#define UI_BLUETEAM5 304
#define UI_BLUETEAM6 305
#define UI_REDTEAM1 306
#define UI_REDTEAM2 307
#define UI_REDTEAM3 308
#define UI_REDTEAM4 309
#define UI_REDTEAM5 310
#define UI_REDTEAM6	311

#define CG_PLAYER_ENEMY 322

#define CG_INVEN_AMMO_FLASH 330
#define CG_INVEN_AMMO_SMOKE 331
#define CG_INVEN_AMMO_MK26  332
#define CG_INVEN_AMMO_40MM  333

// list delimit at: 311
 

// do not change these defines! they are synced with the gamecode ( both, CGAME & GAME )

#ifndef CGAME
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
#endif


// these defines are important for the 'setItem' uiScript
// items are toggable.
// if you want to remove all weaponitems at once use the
// uiScript 'removeWeaponItems'
// set as ownerParam
// 
// armor 
//
#define ITEM_HELMET					1
#define ITEM_VEST					2
// 
// weapon equipment
//
#define ITEM_SILENCER				3
#define ITEM_LASERSIGHT				4
#define ITEM_GRENADELAUNCHER		5
#define ITEM_BAYONET				6
#define ITEM_SCOPE					7 
#define ITEM_SILENCER_SECONDARY		8
#define ITEM_LASERSIGHT_SECONDARY			9
#define ITEM_GRENADELAUNCHER_SECONDARY		10
#define ITEM_BAYONET_SECONDARY				11
#define ITEM_SCOPE_SECONDARY				12
#define ITEM_DUCKBILL						13
#define ITEM_FLASHLIGHT						14
#define ITEM_FLASHLIGHT_SECONDARY			15 // not implemented yet.

//
// player character
//  ownerParm flags for the 'raiseCharacter' uiScript
// and for their responsible buttons... 
// they should get the same ownerParam
// and it should be ownerdraw CG_CHARACTERBUTTON
// done
#define PC_STRENGTH		1
// technical
#define PC_TECHNICAL		2
// done
#define PC_STAMINA		3
// done
#define PC_ACCURACY		4
// done
#define PC_SPEED		5
// done
#define PC_STEALTH		6 

//
// todo:
//
//  -6 buttons with ownerdraw CG_CHARACTERBUTTON and ownerParam 
//        ownerparm = if the button should change stamina, add as
//                      ownerParm PC_STAMINA
//        ownerDraw = CG_CHARACTERBUTTON
//     **this button will then turn red if there are not enough XP
//     ** and green if it's possible
//       ** set as 'action' { uiScript "raiseCharacter" }
//
//  -to show current XP use cvarshow "char_xp"  ( same as ammo show )
//
//

// cvar to show xperience points: "char_xp"

#define VOICECHAT_GETFLAG			"getflag"				// command someone to get the flag
#define VOICECHAT_OFFENSE			"offense"				// command someone to go on offense
#define VOICECHAT_DEFEND			"defend"				// command someone to go on defense
#define VOICECHAT_DEFENDFLAG		"defendflag"			// command someone to defend the flag
#define VOICECHAT_PATROL			"patrol"				// command someone to go on patrol (roam)
#define VOICECHAT_CAMP				"camp"					// command someone to camp (we don't have sounds for this one)
#define VOICECHAT_FOLLOWME			"followme"				// command someone to follow you
#define VOICECHAT_RETURNFLAG		"returnflag"			// command someone to return our flag
#define VOICECHAT_FOLLOWFLAGCARRIER	"followflagcarrier"		// command someone to follow the flag carrier
#define VOICECHAT_YES				"yes"					// yes, affirmative, etc.
#define VOICECHAT_NO				"no"					// no, negative, etc.
#define VOICECHAT_ONGETFLAG			"ongetflag"				// I'm getting the flag
#define VOICECHAT_ONOFFENSE			"onoffense"				// I'm on offense
#define VOICECHAT_ONDEFENSE			"ondefense"				// I'm on defense
#define VOICECHAT_ONPATROL			"onpatrol"				// I'm on patrol (roaming)
#define VOICECHAT_ONCAMPING			"oncamp"				// I'm camping somewhere
#define VOICECHAT_ONFOLLOW			"onfollow"				// I'm following
#define VOICECHAT_ONFOLLOWCARRIER	"onfollowcarrier"		// I'm following the flag carrier
#define VOICECHAT_ONRETURNFLAG		"onreturnflag"			// I'm returning our flag
#define VOICECHAT_INPOSITION		"inposition"			// I'm in position
#define VOICECHAT_IHAVEFLAG			"ihaveflag"				// I have the flag
#define VOICECHAT_BASEATTACK		"baseattack"			// the base is under attack
#define VOICECHAT_ENEMYHASFLAG		"enemyhasflag"			// the enemy has our flag (CTF)
#define VOICECHAT_STARTLEADER		"startleader"			// I'm the leader
#define VOICECHAT_STOPLEADER		"stopleader"			// I resign leadership
#define VOICECHAT_TRASH				"trash"					// lots of trash talk
#define VOICECHAT_WHOISLEADER		"whoisleader"			// who is the team leader
#define VOICECHAT_WANTONDEFENSE		"wantondefense"			// I want to be on defense
#define VOICECHAT_WANTONOFFENSE		"wantonoffense"			// I want to be on offense
#define VOICECHAT_KILLINSULT		"kill_insult"			// I just killed you
#define VOICECHAT_TAUNT				"taunt"					// I want to taunt you
#define VOICECHAT_DEATHINSULT		"death_insult"			// you just killed me
#define VOICECHAT_KILLGAUNTLET		"kill_gauntlet"			// I just killed you with the gauntlet
#define VOICECHAT_PRAISE			"praise"				// you did something good
