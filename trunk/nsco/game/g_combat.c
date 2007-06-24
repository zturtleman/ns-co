// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_combat.c

// every line of code that differs from the quake3:arena SDK
// is property of manfred nerurkar
// no commercial explotation allowed
// you are only allowed to use this code in navy seals: covert operations
// a quake3 arena modifiation
// defcon-x@ns-co.net

#include "g_local.h"

int CanTeamSeeOrigin( vec3_t vTestPoint, int team, int ignoreClientNum );
void NS_BotRadioMsg ( gentity_t *ent, char *msg );
void NS_SendStatusMessageToTeam ( gentity_t *affected, int status, int team );

/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
    gentity_t *plum;

    plum = G_TempEntity( origin, EV_SCOREPLUM );
    // only send this temp entity to a single client
    plum->r.svFlags |= SVF_SINGLECLIENT;
    plum->r.singleClient = ent->s.number;

    //
    plum->s.otherEntityNum = ent->s.number;
    plum->s.time = score;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
    if ( !ent->client ) {
        return;
    }
    if ( ent->client->ps.pm_flags & PMF_FOLLOW )
        return;

    // no scoring during pre-match warmup
    //	if ( level.warmupTime ) {
    //		return;
    //	}
    // show score plum
    //	ScorePlum(ent, origin, score);
    //
    ent->client->ps.persistant[PERS_SCORE] += score;
    if ( score > 0 ) // prevents bugs
        ent->client->ns.num_killed += score;

    if ( g_gametype.integer == GT_TEAM )
        level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
    CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
    gitem_t		*item;
    int			weapon;
    //	float		angle;
    int			i;
    //	gentity_t	*drop;
    // Navy Seals ++
    // drop the weapon if not a gauntlet or machinegun
    weapon = self->s.weapon;

    // make a special check to see if they are changing to a new
    // weapon that isn't the mg or gauntlet.  Without this, a client
    // can pick up a weapon, be killed, and not drop the weapon because
    // their weapon change hasn't completed yet and they are still holding the MG.

    if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
        weapon = self->client->pers.cmd.weapon;
    }

    if ( BG_GotPrimary ( &self->client->ps ) )
    {
        for ( i = 0; i < WP_NUM_WEAPONS ; i ++ )
        {
            // if we got the weapon
            if ( ( BG_GotWeapon( i, self->client->ps.stats ) ) )
            {
                //  if it's primary
                if ( BG_IsPrimary( i ) ) // then : drop it
                {
                    weapon = i;
                    break;
                }
            }
        }
    }

    // if we got a weapon to drop
    if ( weapon > WP_NONE ) {

        // find the item type for this weapon
        item = BG_FindItemForWeapon( weapon );

        // spawn the item
        Drop_Weapon(self, item, random()*360, self->client->ns.rounds[weapon] );

        // remove that weapon!
        BG_RemoveWeapon( weapon, self->client->ps.stats );

        self->client->ps.weapon = WP_NONE;
    }

    weapon = WP_NONE;

    if ( BG_GotSecondary( &self->client->ps ) )
    {
        for ( i = 0; i < WP_NUM_WEAPONS ; i ++ )
        {
            // if we got the weapon
            if ( ( BG_GotWeapon( i, self->client->ps.stats ) ) )
            {
                //  if it's primary
                if ( BG_IsSecondary( i ) ) // then : drop it
                {
                    weapon = i;
                    break;
                }
            }
        }
    }

    // if we got a weapon to drop
    if ( weapon > WP_NONE ) {

        // find the item type for this weapon
        item = BG_FindItemForWeapon( weapon );

        self->client->ps.viewangles[YAW] += 45;

        // spawn the item
        Drop_Weapon(self, item, random()*360, self->client->ns.rounds[weapon] );

        self->client->ps.viewangles[YAW] -= 45;

        // remove that weapon!
        BG_RemoveWeapon( weapon, self->client->ps.stats );

        self->client->ps.weapon = WP_NONE;
    }

    // drop c4
    if ( BG_GotWeapon( WP_C4, self->client->ps.stats ) ) {
        gentity_t *ent;

        item = BG_FindItemForWeapon( WP_C4 );

        ent = Drop_Item( self, item, random()*360 );

        ent->ns_team = self->client->sess.sessionTeam;
    }
    // briefcase
    if ( self->client->ps.powerups[PW_BRIEFCASE] ) {
        item = BG_FindItemForPowerup(PW_BRIEFCASE);

        Drop_Item( self, item, random()*360 );
        self->client->ps.powerups[PW_BRIEFCASE] = 0;
    }
    // Navy Seals --
}



/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
    if ( self->health > GIB_HEALTH ) {
        return;
    }
    if ( !g_blood.integer ) {
        self->health = GIB_HEALTH+1;
        return;
    }

}




/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
    gentity_t	*ent;
    int			anim;
    int			contents;
    int			killer;
    int			i;
    char		*killerName;

    if ( self->client->ps.pm_type == PM_DEAD ) {
        return;
    }

    if ( level.intermissiontime ) {
        return;
    }

    if (BG_IsGrenade( self->s.weapon ) )
    {
        // grenade IS primed
        if ( self->client->ps.weaponstate == WEAPON_FIRING )
            FireWeapon( self );
    }

    self->client->ps.pm_type = PM_DEAD;

    if ( attacker ) {
        killer = attacker->s.number;
        if ( attacker->client ) {
            killerName = attacker->client->pers.netname;
        } else {
            killerName = "<non-client>";
        }
    } else {
        killer = ENTITYNUM_WORLD;
        killerName = "<world>";
    }

    if ( killer < 0 || killer >= MAX_CLIENTS ) {
        killer = ENTITYNUM_WORLD;
        killerName = "<world>";
    }

    // Navy Seals ++
    if ( g_gametype.integer >= GT_LTS )
    {
        if (NS_GotPowerup( self, PW_BRIEFCASE ) && !OnSameTeam(self,attacker) )
        {
            NS_GiveReward( attacker, REWARD_BRIEFCASE_KILL, OtherTeam( self->client->sess.sessionTeam ) );
        }
        if (self->client->ns.is_vip)
        {
            if (self->client->sess.sessionTeam == TEAM_RED)
                level.done_objectives[TEAM_BLUE]++;
            else if (self->client->sess.sessionTeam == TEAM_BLUE)
                level.done_objectives[TEAM_RED]++;

            CenterPrintAll ( "The V.I.P. has been killed!\n");

            if ( !OnSameTeam( attacker, self ) && attacker != self )
                NS_GiveReward( attacker, REWARD_VIP_KILL, OtherTeam( self->client->sess.sessionTeam ) );
        }

        if (self->client->ps.pm_flags & PMF_BOMBCASE)
            bomb_drop(self);
    }
    // Navy Seals --

    G_LogPrintf("Kill: %i %i %i: %s killed %s by %i\n",
                killer, self->s.number, meansOfDeath, killerName,
                self->client->pers.netname, attacker->s.weapon );

    // broadcast the death event to everyone
    ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
    ent->s.eventParm = meansOfDeath;
    ent->s.otherEntityNum = self->s.number;
    ent->s.otherEntityNum2 = killer;
    ent->r.singleClient = self->client->ps.clientNum;
    ent->r.svFlags = SVF_SINGLECLIENT;	// send to self only

    self->enemy = attacker;

    if (attacker && attacker->client) {
        attacker->client->lastkilled_client = self->s.number;

        if ( attacker != self && !g_teamlockcamera.integer )
            self->client->sess.spectatorClient = attacker->client->ps.clientNum;

        if ( OnSameTeam (self, attacker ) )
        {
            // team kill kick
            if ( g_maxTeamKill.integer ) {
                if ( attacker == self )
                    AddScore( attacker ,self->r.currentOrigin, - 1);
                else
                {
                    NS_TeamKill( attacker, self );

                    // add a team kill
                    attacker->client->pers.teamKills++;
                    attacker->client->pers.lastTeamKill = level.time + ( g_TeamKillRemoveTime.integer * ONE_SECOND );

                    // we reached max team-kills
                    if ( attacker->client->pers.teamKills >= g_maxTeamKill.integer )
                    {
                        PrintMsg( NULL, S_COLOR_RED"%s"S_COLOR_WHITE" has been kicked because of team killing.\n",attacker->client->pers.netname );
                        //	PrintMsg( attacker, "You killed %i Team Mates! \n", g_maxTeamKill.integer );

                        //	G_Say( attacker, NULL, 0, "quit (reason: team kill)");

                        trap_DropClient( attacker->s.clientNum , "Dropped due to team kills");
                    }
                    else {
                        PrintMsg( NULL, S_COLOR_RED"%s"S_COLOR_WHITE" killed his teammate "S_COLOR_RED"%s"S_COLOR_WHITE".\nHe is allowed to do %i more kills until he is dropped.\n",
                                  attacker->client->pers.netname, self->client->pers.netname, g_maxTeamKill.integer - attacker->client->pers.teamKills );
                        // tell us that we suck
                        // PrintMsg( attacker, "Kill %i more Team Mates and you will be kicked!\n", g_maxTeamKill.integer - attacker->client->pers.teamKills );

                        // killing team mates sucks
                        AddScore( attacker, self->r.currentOrigin,-1 );
                    }
                }
            }
        }
        else if ( attacker == self )
        {
            // self killing.. we will remove a point for that one
            AddScore( attacker, self->r.currentOrigin,-1 );
        }
        else
        {
            AddScore( attacker, self->r.currentOrigin, 1 );

            attacker->client->lastKillTime = level.time;
        }
    } else {
        AddScore( self, self->r.currentOrigin, -1 );
    }

    if ( meansOfDeath == MOD_GAUNTLET && attacker->client && g_gametype.integer == GT_LTS )
        attacker->client->knife_kills++;

    // if client is in a nodrop area, don't drop anything (but return CTF flags!)
    contents = trap_PointContents( self->r.currentOrigin, -1 );
    if ( !( contents & CONTENTS_NODROP )) {
        TossClientItems( self );
    }
    else {
        // if we're in a nodrop area reset the briefcase
        if ( self->client->ps.powerups[PW_BRIEFCASE] ) {
            // reset briefcase
            Reset_Briefcase();
        }
    }

    // show scores
    Cmd_Score_f( self );

    // send updated scores to any clients that are following this one,
    // or they would get stale scoreboards
    for ( i = 0 ; i < level.maxclients ; i++ ) {
        gclient_t	*client;

        client = &level.clients[i];
        if ( client->pers.connected != CON_CONNECTED ) {
            continue;
        }
        if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
            continue;
        }
        if ( client->sess.spectatorClient == self->s.number ) {
            Cmd_Score_f( g_entities + i );
        }
    }

    self->takedamage = qtrue;	// can still be gibbed

    self->s.weapon = WP_NONE;
    //	self->s.powerups = 0;
    self->r.contents = CONTENTS_CORPSE;


    //	self->s.angles[0] = 0;
    self->s.angles[PITCH] = 0;
    self->s.angles[YAW] = self->client->ps.viewangles[YAW];
    self->s.angles[ROLL] = 0;

    VectorCopy( self->s.angles, self->client->ps.viewangles );

    self->s.loopSound = 0;

    self->r.maxs[2] = -12;

    // don't allow respawn until the death anim is done
    // g_forcerespawn may force spawning at some later time
    self->client->respawnTime = level.time + 2000;

    if ( g_teamRespawn.value > 0.0f && g_gametype.integer == GT_TEAM )
    {
        if ( g_squadAssault.integer )
        {
            if ( level.squadRespawnTime < level.time )
                level.squadRespawnTime = level.time + g_teamRespawn.value * ONE_SECOND;

            self->client->respawnTime = level.squadRespawnTime;
            trap_SetConfigstring( CS_ROUND_START_TIME, va("%i", level.squadRespawnTime) );

        }
        else
            self->client->respawnTime = level.time + g_teamRespawn.value * ONE_SECOND;
    }

    // remove powerups
    //	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

    // never gib in a nodrop
    switch ( self->client->ns.locationOfDeath )
    {
    case LOC_HEAD:
        anim = BOTH_DEATH_NECK;
        break;
    case LOC_FACE:
        anim = BOTH_DEATH_FACE;
        break;
    case LOC_CHEST:
        anim = BOTH_DEATH_CHEST;
        break;
    case LOC_BACK:
        anim = BOTH_DEATH_BACK;
        break;
    case LOC_STOMACH:
        anim = BOTH_DEATH_STOMACH;
        break;
    case LOC_RIGHTARM:
    case LOC_LEFTARM:
        anim = BOTH_DEATH_CHEST;
        break;
    case LOC_RIGHTLEG:
    case LOC_LEFTLEG:
        anim = BOTH_DEATH_LEGS;
        break;
    case LOC_BLEED:
        anim = BOTH_DEATH_BLEED;
        break;
    default:
        anim = BOTH_DEATH_BLEED;
        break;
    }
    // for the no-blood option, we need to prevent the health
    // from going to gib level
    if ( self->health <= GIB_HEALTH ) {
        self->health = GIB_HEALTH+1;
    }

    self->client->ps.legsAnim =
        ( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
    self->client->ps.torsoAnim =
        ( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

    if ( self->waterlevel != 3 )
        G_AddEvent( self, EV_DEATH, killer );

    // the body can still be gibbed
    self->die = body_die;

    trap_LinkEntity (self);

    // did anybody see from my team saw dying?
    if ( g_gametype.integer >= GT_TEAM )
    {
        i = -1;
        i = CanTeamSeeOrigin( self->client->ps.origin, self->client->sess.sessionTeam, self->client->ps.clientNum );

        // yeah...  so he should definitfly tell the others!
        if ( i != -1 )
        {
            if ( self->client->ns.is_vip )
                NS_BotRadioMsg( &g_entities[i], "vdown" );
            else
                NS_BotRadioMsg( &g_entities[i], "mdown" );

            NS_SendStatusMessageToTeam( self, MS_DEAD, self->client->sess.sessionTeam );
        }

        if ( attacker->client && attacker->client->sess.sessionTeam != self->client->sess.sessionTeam )
        {
            i = -1;
            // did anybody see from the other team saw dying?
            i = CanTeamSeeOrigin( self->client->ps.origin, attacker->client->sess.sessionTeam,  self->client->ps.clientNum );

            // yeah...  so he should definitfly tell his comrads :)
            if ( i != -1 )
            {
                if ( self->client->ns.is_vip )
                    NS_BotRadioMsg( &g_entities[i], "vdown" );
                else if ( random() < 0.5 )
                    NS_BotRadioMsg( &g_entities[i], "edown" );
                else
                    NS_BotRadioMsg( &g_entities[i], "edown2" );

                NS_SendStatusMessageToTeam( self, MS_DEAD, attacker->client->sess.sessionTeam );
            }
        }
    }
}

qboolean NS_GotPowerup( gentity_t *ent, int powerup )
{
    if (!ent)
        return qfalse;
    if (!ent->client)
        return qfalse;

    // got powerup?
    if (ent->client->ps.powerups[powerup] > 0 )
        return qtrue; // yup

    // nope
    return qfalse;
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
DAMAGE_NO_ARMOR			armor does not protect from this damage
DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/
qboolean doorlock_damage( gentity_t *ent , int attackerweapon);

// Navy Seals ++
int G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
              vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
    // Navy Seals --
    gclient_t	*client;
    int			take = 0, save = 0, knockback = 0, dummy = 0, armorhit = 0;
    int			HitLocation = LOC_NULL;
    qboolean	bleeding = qfalse;
    qboolean	headblown = qfalse;
    qboolean	spray_blood = qfalse;
    float		through_vest = 1;
    vec3_t		newdir;

    //	PrintMsg( NULL, "targ: %s|inflictor: %s|attack: %s\n", targ->classname, inflictor->classname,attacker->classname );

    if ( !Q_stricmp( targ->classname, "player_bbox_head") ) {
        // PrintMsg( &g_entities[ targ->client->ps.clientNum  ], "Redirecting damage from %s to %i\n", targ->classname, targ->client->ps.clientNum );
        return G_Damage( &g_entities[ targ->client->ps.clientNum  ], inflictor, attacker, dir, point, damage, dflags, MOD_HEADSHOT );
    }

    if (!targ->takedamage)
        return -1;

    if (damage <= 0) return -1;

    // check non-client stuff, just do 4times damage
    if (!targ->client) {
        targ->health -= damage;
        if (targ->health < 0) {
            targ->health = 0;

            // flying away grenades
            if ( !Q_stricmp(targ->classname, "40mmgrenade") ||
                    !Q_stricmp(targ->classname, "smokegrenade") ||
                    !Q_stricmp(targ->classname, "flashbang") ||
                    !Q_stricmp(targ->classname, "grenade") ) {

                VectorCopy(dir, newdir);
                VectorScale( dir, damage/2.0, targ->s.pos.trDelta);
                targ->s.pos.trType = TR_MOREGRAVITY;
                targ->s.pos.trTime = level.time;
                VectorCopy(targ->r.currentOrigin, targ->s.pos.trBase);
                SnapVector(targ->s.pos.trDelta);
                VectorCopy(targ->r.currentOrigin, targ->pos1);
                targ->s.eFlags = EF_BOUNCE_HALF;
            }

            if ( targ->die )
                targ->die (targ, inflictor, attacker, take, mod);
        }
        return damage;
    }


    // no damage to clients in warmup mode
    if ( GameState == STATE_OPEN && g_gametype.integer == GT_LTS && targ->client )
        return -1;

    // no damage to clients in TEAM mode right after being spawned
    if ( ( targ->client ) &&
            ( ( level.time - targ->client->respawnTime ) < RESPAWN_INVUNERABILITY_TIME ) &&
            targ->health == targ->client->pers.maxHealth &&
            g_gametype.integer == GT_TEAM )
        return -1;


    // the intermission has allready been qualified for, so don't
    // allow any extra scoring
    if ( level.intermissionQueued )
        return -1;

    if ( !inflictor )
        inflictor = &g_entities[ENTITYNUM_WORLD];

    if ( !attacker )
        attacker = &g_entities[ENTITYNUM_WORLD];

    // no attack at all
    if ( !Q_stricmp( "misc_doorlock", targ->classname ) &&
            !doorlock_damage( targ, attacker->s.weapon ) &&
            !doorlock_damage( targ, inflictor->s.weapon ) )
        return -1;

    // shootable doors / buttons don't actually have any health
    if ( ( targ->s.eType == ET_MOVER ||
            targ->s.eType == ET_DOOR ||
            targ->s.eType == ET_FUNCEXPLOSIVE ) &&
            !is_func_explosive(targ) ) {

        // shoot the door open
        if ( targ->use && targ->moverState == MOVER_POS1 && targ->health > 0)
            targ->use( targ, inflictor, attacker );

        return -1;
    }

    client = targ->client;

    if ( client &&
            ( client->noclip || client->sess.waiting) )
        return -1;

    // check for completely getting out of the damage
    if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

        // if g_friendlyFire is set, don't do damage to the target
        // if the attacker was on the same team
        if ( targ != attacker &&
                OnSameTeam (targ, attacker) &&
                !g_friendlyFire.integer )
            return -1;

        // check for godmode
        if ( targ->flags & FL_GODMODE )
            return -1;
    }


    // modify the damage inflicted from firearms over the range
    if ( attacker->client && !(dflags & DAMAGE_RADIUS) ) {

        // calculate damage on range
        damage = NS_CalcDamageOnRange( attacker->client->ps.origin, point,damage, attacker->s.weapon );

        // we always have a damage of one if someone is hit!
        if ( damage < 1 ) {
            damage = 1;
        }
    }

    // initialize the take variable, later this will indicate how much of the damage is done
    take = damage;

    save = 0;

    // check for falldamage
    if ( mod == MOD_FALLING ) {
        // add damage to both legs...
        targ->client->ps.stats[STAT_LEG_DAMAGE] += take;

        // remove stamina
        dummy = take*2;
        if (targ->client->ps.stats[STAT_STAMINA] < dummy) {
            dummy -= targ->client->ps.stats[STAT_STAMINA];
            targ->client->ps.stats[STAT_STAMINA] = 0;
            targ->client->ps.stats[STAT_LEG_DAMAGE] += dummy*0.5;
            targ->client->ps.stats[STAT_CHEST_DAMAGE] += dummy*0.4;
            targ->client->ps.stats[STAT_HEAD_DAMAGE] += dummy*0.1;
            take += dummy;
        } else {
            targ->client->ps.stats[STAT_STAMINA] -= dummy;
        }

    } else if ( targ->client &&
                ( attacker && attacker->client ) &&
                point &&
                !(dflags & DAMAGE_NO_BLEEDING) ) {

        // if dead - ignore
        if ( targ->r.contents == CONTENTS_CORPSE )
            return -1;

        if ( attacker->client->ps.weapon == WP_PSG1 ||
                attacker->client->ps.weapon == WP_SL8SD ||
                attacker->client->ps.weapon == WP_MACMILLAN )
            through_vest = 0;
        else if ( NS_GotPowerup(targ, PW_VEST ) && ( BG_IsRifle( attacker->client->ps.weapon ) ) )
            through_vest = 0;//random();
        else if ( !NS_GotPowerup( targ, PW_VEST ) )
            through_vest = 0;

        // get the hit location
        if (g_debugDamage.integer == 1) G_Printf("getting Hitlocation\n");
        HitLocation = NS_CheckLocationDamage( targ,point, mod );
        if (g_debugDamage.integer == 1) G_Printf("got Hitlocation: %i\n", HitLocation);

        // if nothing was hit, return
        if (HitLocation == LOC_NULL) return -1;

        // extra handling for grenades
        if (dflags & DAMAGE_RADIUS) {

            // if we have traced the head or face, we assume partial cover.
            // No damage to other bodyparts (except arms)
            if (HitLocation == LOC_FACE) {
                spray_blood = qtrue;
                take *= 2;
                armorhit = 2;
                targ->client->ps.stats[STAT_HEAD_DAMAGE] += take*0.8;
                targ->client->ps.stats[STAT_ARM_DAMAGE] += take*0.2;
            } else if (HitLocation == LOC_HEAD) {
                spray_blood = qtrue;
                if (NS_GotPowerup( targ, PW_HELMET ) ) take *= 1;
                else take *= 2;
                armorhit = 2;
                targ->client->ps.stats[STAT_HEAD_DAMAGE] += take*0.8;
                targ->client->ps.stats[STAT_ARM_DAMAGE] += take*0.2;
            } else {
                // we traced chest, back, stomach, arms or legs. We assume there is
                // no partial cover. damage will be distributed to the damage areas.
                if (NS_GotPowerup( targ, PW_VEST ) ) take *= 0.7;
                else take *= 1;
                armorhit = 1;
                spray_blood = qtrue;
                if ( (random() < 0.4) || NS_GotPowerup(targ, PW_VEST) ) bleeding = qfalse;
                else bleeding = qtrue;
                targ->client->ps.stats[STAT_CHEST_DAMAGE] += take*0.45;
                targ->client->ps.stats[STAT_STOMACH_DAMAGE] += take*0.15;
                targ->client->ps.stats[STAT_LEG_DAMAGE] += take*0.2;
                targ->client->ps.stats[STAT_ARM_DAMAGE] += take*0.1;
                targ->client->ps.stats[STAT_HEAD_DAMAGE] += take*0.1;
            }

            // here goes the calculation for bullets / melee
        } else switch ( HitLocation ) {

            case LOC_FACE:
                spray_blood = qtrue;
                armorhit = 2;
                take *= 10;
                if ( (take > targ->health) && (NS_CanShotgunBlowHead( targ, attacker, attacker->s.weapon ) ) ) {
                    headblown = qtrue;

                    if ( attacker->client->ns.rewards & REWARD_HS_KILL )
                        attacker->client->ns.rewards |= REWARD_HS_2KILL;
                    else
                        attacker->client->ns.rewards |= REWARD_HS_KILL;

                    take = 999;
                    if(! (dflags & DAMAGE_RADIUS) )
                        PrintMsg(NULL, S_COLOR_RED"%ss face was blown away by %s!\n", targ->client->pers.netname, attacker->client->pers.netname);
                }
                targ->client->ps.stats[STAT_HEAD_DAMAGE] += take;
                break;

            case LOC_HEAD:
                spray_blood = qtrue;
                armorhit = 2;
                if (NS_GotPowerup(targ, PW_HELMET)) take *= 4;
                else take *= 8;
                if ( (take > targ->health) && (NS_CanShotgunBlowHead( targ, attacker, attacker->s.weapon ) ) ) {
                    headblown = qtrue;

                    if ( attacker->client->ns.rewards & REWARD_HS_KILL )
                        attacker->client->ns.rewards |= REWARD_HS_2KILL;
                    else
                        attacker->client->ns.rewards |= REWARD_HS_KILL;

                    take = 999;
                    if(! (dflags & DAMAGE_RADIUS) )
                        PrintMsg(NULL, S_COLOR_RED"%ss head was blown away by %s!\n", targ->client->pers.netname, attacker->client->pers.netname);
                }
                targ->client->ps.stats[STAT_HEAD_DAMAGE] += take;
                break;

            case LOC_CHEST:
                spray_blood = qtrue;
                take *= 2.5;
                if ( (random() < 0.4) || NS_GotPowerup(targ, PW_VEST) ) bleeding = qfalse;
                else bleeding = qtrue;
                targ->client->ps.stats[STAT_CHEST_DAMAGE] += take;
                break;

            case LOC_BACK:
                spray_blood = qtrue;
                take *= 2.0;
                if ( (random() < 0.4) || NS_GotPowerup(targ, PW_VEST) ) bleeding = qfalse;
                else bleeding = qtrue;
                targ->client->ps.stats[STAT_CHEST_DAMAGE] += take;
                break;

            case LOC_STOMACH:
                spray_blood = qtrue;
                take *= 2.5;
                if ( NS_GotPowerup(targ, PW_VEST) ) bleeding = qfalse;
                else bleeding = qtrue;
                targ->client->ps.stats[STAT_STOMACH_DAMAGE] += take;
                break;

            case LOC_RIGHTARM:
                spray_blood = qtrue;
                take *= 1.63;
                if ( random() < 0.6 ) bleeding = qfalse;
                else bleeding = qtrue;
                if ( (random() < 0.1) &&
                        !(targ->client->ps.eFlags & EF_VIP) &&
                        !(targ->s.weapon <= WP_NONE) ) NS_DropWeapon(targ);
                targ->client->ps.stats[STAT_ARM_DAMAGE] += take;
                break;

            case LOC_LEFTARM:
                spray_blood = qtrue;
                take *= 1.63;
                if ( random() < 0.6 ) bleeding = qfalse;
                else bleeding = qtrue;
                targ->client->ps.stats[STAT_ARM_DAMAGE] += take;
                break;

            case LOC_RIGHTLEG:
                spray_blood = qtrue;
                take *= 1.75;
                if ( random() < 0.6 ) bleeding = qfalse;
                else bleeding = qtrue;
                targ->client->ps.stats[STAT_LEG_DAMAGE] += take;
                break;

            case LOC_LEFTLEG:
                spray_blood = qtrue;
                take *= 1.75;
                if ( random() < 0.6 ) bleeding = qfalse;
                else bleeding = qtrue;
                targ->client->ps.stats[STAT_LEG_DAMAGE] += take;
                break;


            }

        // get the base factor if the target wears a vest and the chest was hit
        if ( NS_GotPowerup( targ, PW_VEST ) &&
                (HitLocation == LOC_CHEST) &&
                (HitLocation == LOC_BACK) &&
                (HitLocation == LOC_STOMACH) ) switch (attacker->client->ps.weapon) {
            case WP_GLOCK:
                take *= SEALS_DMG_VEST_GLOCK;
                armorhit = 1;
                break;
            case WP_P9S:
                take *= SEALS_DMG_VEST_P9S;
                armorhit = 1;
                break;
            case WP_MK23:
                take *= SEALS_DMG_VEST_MK23;
                armorhit = 1;
                break;
            case WP_SW40T:
                take *= SEALS_DMG_VEST_SW40T;
                armorhit = 1;
                break;
            case WP_SW629:
                take *= SEALS_DMG_VEST_SW629;
                armorhit = 1;
                break;
            case WP_DEAGLE:
                take *= SEALS_DMG_VEST_DEAGLE;
                armorhit = 1;
                break;
            case WP_MP5:
                take *= SEALS_DMG_VEST_MP5;
                armorhit = 1;
                break;
            case WP_MAC10:
                take *= SEALS_DMG_VEST_MAC10;
                armorhit = 1;
                break;
            case WP_PDW:
                take *= SEALS_DMG_VEST_PDW;
                armorhit = 1;
                break;
            case WP_M14:
                take *= SEALS_DMG_VEST_M14;
                armorhit = 1;
                break;
            case WP_M4:
                take *= SEALS_DMG_VEST_M4;
                armorhit = 1;
                break;
            case WP_AK47:
                take *= SEALS_DMG_VEST_AK47;
                armorhit = 1;
                break;
            case WP_M249:
                take *= SEALS_DMG_VEST_M249;
                armorhit = 1;
                break;
            case WP_SPAS15:
                take *= SEALS_DMG_VEST_SPAS15;
                armorhit = 1;
                break;
            case WP_M590:
                take *= SEALS_DMG_VEST_M590;
                armorhit = 1;
                break;
            case WP_870:
                take *= SEALS_DMG_VEST_870;
                armorhit = 1;
                break;
            case WP_MACMILLAN:
                take *= SEALS_DMG_VEST_MACMILLAN;
                armorhit = 1;
                break;
            case WP_PSG1:
                take *= SEALS_DMG_VEST_PSG1;
                armorhit = 1;
                break;
            case WP_SL8SD:
                take *= SEALS_DMG_VEST_SL8SD;
                armorhit = 1;
                break;
            }

        // if a grenade hit was encountered, remove stamina
        if(dflags & DAMAGE_RADIUS) {
            dummy = take*4;
            if ( targ->client->ps.stats[STAT_STAMINA] > dummy )
                targ->client->ps.stats[STAT_STAMINA] -= dummy;
            else {
                dummy -= targ->client->ps.stats[STAT_STAMINA];
                targ->client->ps.stats[STAT_STAMINA] = 0;
                targ->client->ps.stats[STAT_HEAD_DAMAGE] += dummy*0.2;
                targ->client->ps.stats[STAT_CHEST_DAMAGE] += dummy*0.3;
                targ->client->ps.stats[STAT_STOMACH_DAMAGE] += dummy*0.1;
                targ->client->ps.stats[STAT_ARM_DAMAGE] += dummy*0.2;
                targ->client->ps.stats[STAT_LEG_DAMAGE] += dummy*0.2;
                take += dummy;
            }
        }

        // not slashing with a knife then do a bloodsplat on wall
        if (spray_blood &&  !BG_IsMelee( attacker->s.weapon ) && take > 0 && !(dflags & DAMAGE_RADIUS) )
            NS_SprayBlood(targ,point,dir,damage, headblown );
    }

    if ( !dir ) {
        dflags |= DAMAGE_NO_KNOCKBACK;
    } else {
        VectorNormalize(dir);
    }

    knockback = damage = take;

    if ( attacker && attacker->client && BG_IsShotgun(attacker->client->ps.weapon) )
        knockback *= 2;

    if ( knockback > SEALS_MAXKNOCKBACK ) {
        knockback = SEALS_MAXKNOCKBACK;
    }
    if ( targ->flags & FL_NO_KNOCKBACK ) {
        knockback = 0;
    }
    if ( dflags & DAMAGE_NO_KNOCKBACK ) {
        knockback = 0;
    }

    // figure momentum add, even if the damage won't be taken
    if ( g_knockback.value && knockback && targ->client/* && ( targ->health - take > 0 )'*/ ) {
        vec3_t	kvel;
        float	mass;

        mass = 200;

        if  (dflags & DAMAGE_RADIUS)
        {
            mass = 75;
            VectorScale (dir, 400 * (float)knockback / mass, kvel);
        }
        else
            VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
        VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

        // set the timer so that the other client can't cancel
        // out the movement immediately
        if ( !targ->client->ps.pm_time ) {
            int		t;

            t = knockback * 2;
            if ( t < 50 ) {
                t = 50;
            }
            if ( t > 1000 ) {
                t = 1000;
            }
            targ->client->ps.pm_time = t;
            targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
        }
    }


    if ( g_debugDamage.integer ) {
        PrintMsg(NULL, "%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number,
                 targ->health, take, NS_GotPowerup( targ, PW_VEST ) + NS_GotPowerup( targ, PW_HELMET ) );
    }

    // if we're using a shotgun... and the target is wearing armor.. damage = 0
    if ( attacker && attacker->client && BG_IsShotgun( attacker->s.weapon ) )
        if ( HitLocation == LOC_CHEST || HitLocation == LOC_STOMACH || HitLocation == LOC_BACK )
        {
            if ( NS_GotPowerup(targ, PW_VEST ) ) {
                damage = 0;
                // BLUTENGEL_XXX: remove vest if damage > 50 ??
            }
        }

    save = 0;
    // Navy Seals ++
    // save some from armor
    //	asave = CheckArmor (targ, take, dflags);
    //	take -= asave;
    // Navy Seals -- : nope

    // add to the damage inflicted on a player this frame
    // the total will be turned into screen blends and view angle kicks
    // at the end of the frame
    if ( client ) {
        if (armorhit == 2)
            client->damage_blood += take * 10;
        else if (armorhit == 1)
            client->damage_blood += take * 5;
        else
            client->damage_blood += take * 2;
        client->damage_knockback += knockback;
        if ( dir ) {
            VectorCopy ( dir, client->damage_from );
            client->damage_fromWorld = qfalse;
        } else {
            VectorCopy ( targ->r.currentOrigin, client->damage_from );
            client->damage_fromWorld = qtrue;
        }
        if (!client->damage_fromWorld) // bleeding can only be caused by other clients / bots
        {
            if ( bleeding )
            {
                client->bleed_num++; // new bleeding

                {
                    int dag = damage;
                    if (dag>45) dag=45;

                    client->bleed_delay[client->bleed_num] = 5000 - dag * 100 + client->pers.nsPC.strength * 50 + random()*100; // formula !
                }
                client->bleed_point[client->bleed_num] = damage / 2;
                client->bleed_causer[client->bleed_num] = attacker;
                client->bleed_methodOfDeath[client->bleed_num] = mod;
                client->bleeding = qtrue;

                //	VectorCopy(point, client->bleed_loc[client->bleed_num] ); // fix?
                VectorSubtract (point, targ->r.absmax, client->bleed_loc[client->bleed_num]);
            }
        }
    }

    if (targ->client) {
        // set the last client who damaged the target
        targ->client->lasthurt_client = attacker->s.number;
        targ->client->lasthurt_mod = mod;
    }

    // do the damage
    if (take) {
        // stop here if we're a corspe, we can't recieve no damage
        if ( targ->r.contents == CONTENTS_CORPSE )
            return take;

        if ( is_func_explosive(targ) && !(dflags & DAMAGE_RADIUS) ) {
            targ->health -= take;

            // if we got hits left
            targ->ns_flags--;  // remove one

            // didn't get destroyed
            if ( targ->ns_flags > 0 )
                return take;
        } else targ->health = targ->health - take;

        if ( targ->client ) {
            targ->client->ps.stats[STAT_HEALTH] = targ->health;
        }

        if ( targ->health <= 0 ) {
            if ( client ) {
                //targ->flags |= FL_NO_KNOCKBACK;
                if ( mod == MOD_BLEED )
                    HitLocation = LOC_BLEED;

                targ->client->ns.locationOfDeath = HitLocation;
            }

            if (targ->health < -999)
                targ->health = -999;

            targ->enemy = attacker;
            targ->die (targ, inflictor, attacker, take, mod);
            return take;
        } else if ( targ->pain ) {
            targ->pain (targ, attacker, take);
        }
    }

    return take;

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin) {
    vec3_t	dest;
    trace_t	tr;
    vec3_t	midpoint;

    // use the midpoint of the bounds instead of the origin, because
    // bmodels may have their origin is 0,0,0
    VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
    VectorScale (midpoint, 0.5, midpoint);

    VectorCopy (midpoint, dest);
    trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

    if( tr.fraction == 1.0 )
        return qtrue;


    // this should probably check in the plane of projection,
    // rather than in world coordinate, and also include Z
    VectorCopy (midpoint, dest);
    dest[0] += 15.0;
    dest[1] += 15.0;
    trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

    if( tr.fraction == 1.0 )
        return qtrue;

    VectorCopy (midpoint, dest);
    dest[0] += 15.0;
    dest[1] -= 15.0;
    trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

    if( tr.fraction == 1.0 )
        return qtrue;

    VectorCopy (midpoint, dest);
    dest[0] -= 15.0;
    dest[1] += 15.0;
    trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

    if( tr.fraction == 1.0 )
        return qtrue;

    VectorCopy (midpoint, dest);
    dest[0] -= 15.0;
    dest[1] -= 15.0;
    trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);

    if( tr.fraction == 1.0 )
        return qtrue;

    return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod) {
    gentity_t	*ent;
    int			i, e, dmg, dist;
    int			killedClients = 0;
    vec3_t		oldOrg, tracefrom;
    qboolean	hitClient = qfalse;
    trace_t tr;

    // unlink all assault fields
    assault_link_all( qtrue );

    // get the radius to a reasonable amount
    if ( radius < 1.0 ) {
        radius = 1.0;
    }

    // remember the origin of the grenade
    VectorCopy( origin, oldOrg );

    // raise it a lil bit over the ground
    origin[2] += 10.0f;

    // get a point that's not in any wall
    while ( trap_PointContents( origin, ENTITYNUM_NONE ) & CONTENTS_SOLID )
    {
        origin[2] -= 0.5f;

        if ( origin[2] < oldOrg[2] ) { // don't get stuck in a loop
            origin[2] = oldOrg[2];	 // and don't get lost in the ground
            break;
        }
    }

    // check every entity in the level
    for (i = 0; i < level.num_entities ; i++) {
        ent = &g_entities[ i ];

        // the following entities will just be ignored
        if (! ent->inuse) continue; // ignoring entities not in use
        if (ent == ignore) continue; // ignoring as requested
        if (! ent->takedamage) continue; // cannot take damage, i'm not interested
        // BLUTENGEL_XXX 08.01.2004
        // why are there lots of player_bbox_head entities????
        if (! Q_stricmp(ent->classname, "player_bbox_head") ) continue; // ignore the players head bounding box

        // check the distance
        for (e=0; e<3; e++) {
            oldOrg[e] = origin[e] - (ent->r.currentOrigin[e] + (ent->r.mins[e] + ent->r.maxs[e])*0.5 );
        }
        // too far away
        if ( (dist = VectorLength(oldOrg)) > radius) continue;

        // BLUTENGEL_XXX:
        // perhaps i should handle the player like a normal entity!
        // dunno yet.

        // trace to the entity if it's a player
        if (ent->client) {

            // get the origin of the player
            VectorCopy(ent->r.currentOrigin, tracefrom);

            // trace to the player's feet
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, (CONTENTS_SOLID | CONTENTS_BODY) );

            // feet not hit, trace to the head
            if (tr.fraction < 1.0) {

                // move the Z value of tracefrom up to the eye spot of player
                tracefrom[2] += ent->client->ps.viewheight;

                // trace to the player's head
                trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, (CONTENTS_SOLID | CONTENTS_BODY) );

                // head not hit, feet not hit, trace to stomach
                if (tr.fraction < 1.0) {
                    // move the Z value to the stomach
                    tracefrom[2] -= ((float)ent->client->ps.viewheight)/2.0f;

                    // trace to the player's head
                    trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, (CONTENTS_SOLID | CONTENTS_BODY) );

                    // ok, no important part of the player hit -> continue
                    if (tr.fraction < 1.0) continue;
                }
            }

            // if we get to here, a player is hit!
            hitClient = qtrue;

        } else if (BG_IsGrenade(ent->s.weapon) ) { // if (ent->client) {

            // get the origin of the entity
            VectorCopy(ent->r.currentOrigin, tracefrom);

            // trace to the entities origin
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, (CONTENTS_SOLID | CONTENTS_BODY) );

            // not hit, try to hit higher
            if (tr.fraction < 1.0) {

                // move the Z value of tracefrom a lil bit higher
                tracefrom[2] += ent->r.mins[2] + (ent->r.maxs[2] - ent->r.mins[2])*0.9;

                // trace
                trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, (CONTENTS_SOLID | CONTENTS_BODY) );

                // not hit
                if (tr.fraction < 1.0) continue;

            }
        } else {
            // normal entities will be hit iff the bounding box is hit
            // first calculate the middle of the entity

            oldOrg[0] = ent->r.mins[0] + 0.5*(ent->r.maxs[0] - ent->r.mins[0]);
            oldOrg[1] = ent->r.mins[1] + 0.5*(ent->r.maxs[1] - ent->r.mins[1]);
            oldOrg[2] = ent->r.mins[2] + 0.5*(ent->r.maxs[2] - ent->r.mins[2]);

            // now check if the middle of any of the 6 bounding box planes
            // is reachable

            tracefrom[0] = ent->r.mins[0];
            tracefrom[1] = oldOrg[1];
            tracefrom[2] = oldOrg[2];
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, CONTENTS_SOLID);
            if (tr.fraction == 1.0) goto ENTITY_HIT;

            tracefrom[0] = oldOrg[0];
            tracefrom[1] = ent->r.mins[1];
            tracefrom[2] = oldOrg[2];
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, CONTENTS_SOLID);
            if (tr.fraction == 1.0) goto ENTITY_HIT;

            tracefrom[0] = oldOrg[0];
            tracefrom[1] = oldOrg[1];
            tracefrom[2] = ent->r.mins[2];
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, CONTENTS_SOLID);
            if (tr.fraction == 1.0) goto ENTITY_HIT;

            tracefrom[0] = ent->r.maxs[0];
            tracefrom[1] = oldOrg[1];
            tracefrom[2] = oldOrg[2];
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, CONTENTS_SOLID);
            if (tr.fraction == 1.0) goto ENTITY_HIT;

            tracefrom[0] = oldOrg[0];
            tracefrom[1] = ent->r.maxs[1];
            tracefrom[2] = oldOrg[2];
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, CONTENTS_SOLID);
            if (tr.fraction == 1.0) goto ENTITY_HIT;

            tracefrom[0] = oldOrg[0];
            tracefrom[1] = oldOrg[1];
            tracefrom[2] = ent->r.maxs[2];
            trap_Trace(&tr, tracefrom, NULL, NULL, origin, i, CONTENTS_SOLID);
            if (tr.fraction == 1.0) goto ENTITY_HIT;

            // if we have not left the if statement, the entity wasn't hit.
            continue;

        }

ENTITY_HIT:

        // calculate damage
        if (dist < SEALS_DEATHNADERADIUS) {
            dmg = 260;
        } else {
            dmg = damage * ( 1.0 - ((float)dist) / radius);

            // reduce damage if it's a client and the client is ducked
            if (ent->client && (ent->client->ps.pm_flags & PMF_DUCKED) )
                dmg = dmg - (((float)(dmg)) / 3.0);
        }

        // BLUTENGEL_XXX:
        // debug purposes only
        //if (ent->client) PrintMsg(NULL, "Client %s hit by grenade with %i base damage!\n", ent->client->pers.netname, dmg);
        //else PrintMsg( NULL, "Entity %s hit by grenade with %i base damage!\n", ent->classname, dmg);

        // send the damage
        VectorSubtract(ent->r.currentOrigin, origin, oldOrg);

        G_Damage(ent, ignore, attacker, oldOrg, origin, dmg, DAMAGE_RADIUS, mod);

        // check if a player died
        if ( ent->health <= 0 )
            if (ent->client && attacker && attacker->client)
                if (ent->client->sess.sessionTeam != attacker->client->sess.sessionTeam)
                    killedClients++;
                else
                    killedClients--;
    }

    if (g_gametype.integer == GT_LTS && attacker && attacker->client )
        if ( killedClients >= 4 )
            attacker->client->ns.rewards |= REWARD_4KILL_GRENADE;
        else if ( killedClients >= 2 )
            attacker->client->ns.rewards |= REWARD_2KILL_GRENADE;

    // relink all assault fields
    assault_link_all( qfalse );

    return hitClient;
}
