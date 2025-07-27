#include "../../include/types.h"
#include "../../include/config.h"
#include "../../include/battle.h"
#include "../../include/mega.h"
#include "../../include/pokemon.h"
#include "../../include/trainer_ai.h"
#include "../../include/constants/ability.h"
#include "../../include/constants/move_effects.h"
#include "../../include/constants/species.h"
#include "../../include/constants/battle_script_constants.h"
#include "../../include/constants/battle_message_constants.h"
#include "../../include/constants/item.h"
#include "../../include/item.h"
#include "../../include/custom/custom_ai.h"
#include "../../include/overlay.h"
#include "../../include/constants/file.h"


#define BATLLE_DEBUG_OUTPUT 1
//#define ATTRACT_WORK_ON_ALL_SEXES 1

#define IMPOSSIBLE_MOVE 40
#define NEVER_USE_MOVE_20 20


struct PACKED AIContext {
    struct AI_sDamageCalc attackerMon;
    struct AI_sDamageCalc defenderMon; //"player"
    //struct AI_sDamageCalc defenderAlly;

    BOOL isDoubleBattle;
    BOOL isPartnerGrounded;

    BOOL defenderImmuneToPoison;
    BOOL defenderImmuneToParalysis;
    BOOL defenderImmuneToBurn;
    BOOL defenderImmuneToSleep;

    u8 attackerMovesFirst;
    u8 defenderMovesFirst;
    u8 isSpeedTie;

    int attacker;
    int defender;
    int defenderSide;
    int attackerSide;
    
    int attackerSpeed;
    int defenderSpeed;

    u8 partySizeAttacker;
    u8 livingMembersAttacker;

    u8 partySizeDefender;
    u8 livingMembersDefender;

    u8 attackerMovesKnown;

    u32 attackerLastUsedMove;
    u32 defenderLastUsedMove;
    u32 defenderLastUsedMoveEffect;
    u32 defenderTurnsOnField;
    u32 attackerTurnsOnField;

	u32 attackerMove;
    u32 attackerMoveEffect;

    BOOL isDefenderIncapacitated;
    BOOL defenderKnowsThawingMove;

    u32 maxDamageReceived;

    BOOL defenderHasAtleastOnePhysicalMove;
    BOOL defenderHasAtleastOneSpecialMove;
    BOOL playerCanOneShotMonWithMove[4];
    BOOL playerCanOneShotMonWithAnyMove;
    BOOL monCanOneShotPlayerWithAnyMove;


    BOOL monCanOneShotPlayerWithMove[4];
    u32 attackerRolledMoveDamages[4];
    u32 effectivenessOnPlayer[4];
    u32 attackerRolledMaxDamage;
};


int LONG_CALL BasicScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai);
int LONG_CALL DamagingMoveScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai);
int LONG_CALL SetupScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai);
int LONG_CALL RecoveryScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai);
int LONG_CALL HarassmentScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai);


/*Helper Functions*/
BOOL LONG_CALL battlerHasSoundBasedMove(struct BattleSystem* bsys, u32 battler, struct AIContext* ai);
BOOL LONG_CALL battlerKnowsFlinchingMove(struct BattleSystem* bsys, u32 battler, struct AIContext* ai);
BOOL LONG_CALL BattlerKnowsMove(struct BattleSystem *bsys, u32 battler, u32 move, struct AIContext* ai);
int LONG_CALL BattlerPositiveStatChangesSum(struct BattleSystem* bsys, u32 battler, struct AIContext* ai);
BOOL LONG_CALL shouldRecover(struct BattleSystem* bsys, u32 attacker, u32 attackerMoveEffect, struct AIContext* ai);
BOOL LONG_CALL battlerKnowsThawingMove(struct BattleSystem* bsys, u32 battler, struct AIContext* ai);
int LONG_CALL offensiveSetup(struct BattleSystem* bsys, u32 attacker, int i, struct AIContext* ai);
int LONG_CALL defensiveSetup(struct BattleSystem* bsys, u32 attacker, int i, struct AIContext* ai);
BOOL LONG_CALL isMoveSpecialAiAttackingMove(u32 attackerMove);
int LONG_CALL SpecialAiAttackingMove(struct BattleSystem* bsys, u32 attacker, int i, struct AIContext* ai);
BOOL LONG_CALL IsMoveMultihit(u32 move);
BOOL LONG_CALL canMoveKillBattler(u32 move, u32 damage, u32 defenderHP, u32 defenderMaxHP, BOOL attackerHasMoldBreaker, u32 defenderAbility, u32 defenderItem);
BOOL LONG_CALL monDiesFromResidualDamage(struct BattleStruct* ctx, u32 attacker, u32 attackerCondition, BOOL isSeeded);

BOOL LONG_CALL IsInList(u32 moveEffect, const u16 StatList[], u16 ListLength);
BOOL LONG_CALL BattlerKnowsMoveInList(struct BattleSystem *bsys, u32 battler, const u16 MoveList[], u16 listLength, struct AIContext* ai);
BOOL LONG_CALL BattlerMovesFirstDoubles(struct BattleSystem *bsys, struct BattleStruct *ctx, int mainBattler, int flag, struct AIContext* ai);

void LONG_CALL SetupStateVariables(struct BattleSystem *bsys, u32 attacker, u32 defender, struct AIContext* ai);

int LONG_CALL scoreMovesAgainstDefender(struct BattleSystem* bsys, u32 attacker, u32 target, int moveScores[4][4], struct AIContext* ai)
{
    struct BattleStruct* ctx = bsys->sp;
	int highestScoredMove = 0;
    for (int i = 0; i < GetBattlerLearnedMoveCount(bsys, ctx, attacker); i++)
    {
        u32 attackerMove = ctx->battlemon[attacker].move[i];

        if (attackerMove != MOVE_NONE &&
            (attackerMove == ctx->battlemon[attacker].moveeffect.moveNoChoice ||
             attackerMove == ctx->battlemon[attacker].moveeffect.encoredMove)) // if the attacker has a move that is forced, use it
        {
#ifdef BATLLE_DEBUG_OUTPUT
            debug_printf("Attacker has choiced move %d:%d\n", i, ctx->battlemon[attacker].moveeffect.moveNoChoice);
#endif // BATLLE_DEBUG_OUTPUT
            moveScores[target][i] += 1000;
        }

        moveScores[target][i] += 1000;  //don't want to get negative numbers, so start high at 1000, MOVE_NONE will stay at 0
        moveScores[target][i] += BasicScoring(bsys, attacker, i, ai);
        moveScores[target][i] += DamagingMoveScoring(bsys, attacker, i, ai);
        moveScores[target][i] += SetupScoring(bsys, attacker, i, ai);
        moveScores[target][i] += RecoveryScoring(bsys, attacker, i, ai);
        moveScores[target][i] += HarassmentScoring(bsys, attacker, i, ai);

        if (highestScoredMove < moveScores[target][i])
            highestScoredMove = moveScores[target][i];
    }

	return highestScoredMove;
}


enum AIActionChoice __attribute__((section (".init"))) TrainerAI_Main(struct BattleSystem *bsys, u32 attacker)
{
#ifdef BATLLE_DEBUG_OUTPUT
    debug_printf("TrainerAI_Main:\n");
#endif // BATLLE_DEBUG_OUTPUT

	int score = 0;
    if (attacker >= 10)
		return BattleAI_PostKOSwitchIn_Internal(bsys, attacker - 10, &score);
    
    struct BattleStruct *ctx = bsys->sp;
    struct AIContext aictx = {0};
    struct AIContext* ai = &aictx;
    enum AIActionChoice result = AI_ENEMY_ATTACK_1;


    int highestScoredMove = 0;
    int highestScoredMoveAcross = 0;
	int moveScores[4][4] = { 0 };   //account for BATTLER_OPPONENT (0), attacker (1), BATTLER_ACROSS(2), BATTLER_ALLY(3),  4 moves each
                                    //account for BATTLER_OPPONENT (2), attacker (3), BATTLER_ACROSS(0), BATTLER_ALLY(1),  4 moves each
	int targets[4] = { 0 };
    int targetsSize = 0;
    int tiedMoveIndices[4] = {0};
    u32 target = 0;

    u32 defender = BATTLER_OPPONENT(attacker);   //default for singles
    target = defender;

    if(BattleTypeGet(bsys) & (BATTLE_TYPE_MULTI | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TAG))
    {
#ifdef BATLLE_DEBUG_OUTPUT
        debug_printf("att %d, ally %d, defendOp %d, defendCross %d\n", attacker, BATTLER_ALLY(attacker), BATTLER_OPPONENT(attacker), BATTLER_ACROSS(attacker));
        debug_printf("att %d, ally %d, defendOp %d, defendCross %d\n", ctx->battlemon[attacker].species, ctx->battlemon[BATTLER_ALLY(attacker)].species, ctx->battlemon[BATTLER_OPPONENT(attacker)].species, ctx->battlemon[BATTLER_ACROSS(attacker)].species);
#endif // BATLLE_DEBUG_OUTPUT

        //BATTLER_OPPONENT
        SetupStateVariables(bsys, attacker, defender, ai);
 
        BOOL playerCanOneShotMonWithAnyMove = ai->playerCanOneShotMonWithAnyMove;
		highestScoredMove = scoreMovesAgainstDefender(bsys, attacker, target, moveScores, ai);
        //BATTLER_ACROSS
        defender = BATTLER_ACROSS(attacker);
        if (ctx->battlemon[defender].hp > 0)
        {
            target = defender;
            SetupStateVariables(bsys, attacker, defender, ai);
            if (playerCanOneShotMonWithAnyMove)
                ai->playerCanOneShotMonWithAnyMove = playerCanOneShotMonWithAnyMove; //keep the value from the first target

            highestScoredMoveAcross = scoreMovesAgainstDefender(bsys, attacker, target, moveScores, ai);
            if (highestScoredMoveAcross > highestScoredMove)
				highestScoredMove = highestScoredMoveAcross;
        }
		// defender = BATTLER_ALLY(attacker);
        // target = defender;

        targetsSize = 0;
        for (u8 k = 0; k < 4; k++) // find targets with highestScoredMove
        {
            for (u8 i = 0; i < 4; i++) //movesScore
            {
                if (moveScores[k][i] == highestScoredMove)
                {
                    //debug_printf("found target %d\n", k)
                    targets[targetsSize] = k;
                    targetsSize++;
                    break;
                }
            }
        }

        target = targets[(BattleRand(bsys) % targetsSize)];

#ifdef BATLLE_DEBUG_OUTPUT
        debug_printf("picked target slot %d\n", target); 
#endif // BATLLE_DEBUG_OUTPUT
    }
    else  //single battles
    {
        //BATTLER_OPPONENT
        SetupStateVariables(bsys, attacker, defender, ai);
        highestScoredMove = scoreMovesAgainstDefender(bsys, attacker, target, moveScores, ai);
    }
    ctx->aiWorkTable.ai_dir_select_client[attacker] = target;

#ifdef BATLLE_DEBUG_OUTPUT
    u8 j = 0;
    for (int k = 0; k < 4; k++)
    {
        for (u8 i = 0; i < 4; i++) //movesScore
        {
            debug_printf("%4d  ", moveScores[k][i]);
        }

        if (targets[j] == k)
        {
            j++;
            debug_printf("x");
        }
        debug_printf("\n");
    }
#endif // BATLLE_DEBUG_OUTPUT

    for (u8 i = 0; i < 4; i++)
    {
        if (moveScores[target][i] == highestScoredMove)
        {
            result = i;
        }
    }

#ifdef BATLLE_DEBUG_OUTPUT
    for (u8 i = 0; i < 4; i++)
    {
        debug_printf("%i ", moveScores[target][i]);
    }
    debug_printf("-> highest %i:%i\n", result, highestScoredMove);
#endif // BATLLE_DEBUG_OUTPUT


    int tieMoveCount = 0;

    for (u8 i = 0; i < 4; i++)
    {                                         //check for ties
        if(moveScores[target][i] == highestScoredMove)
        {
            tiedMoveIndices[tieMoveCount] = i;
            tieMoveCount++;
        }
    }
	u8 tieMoveIndex = (BattleRand(bsys) % tieMoveCount);

    result = tiedMoveIndices[tieMoveIndex % 4];             //randomly pick a move among the tie
#ifdef BATLLE_DEBUG_OUTPUT
	debug_printf("got tieMoveIndex %d -> Resulting move: %d\n", tieMoveIndex,  result);
#endif // BATLLE_DEBUG_OUTPUT
    return result;
}



/*Heavily penalize stupid decisions that would fail, do nothing, or objectively hurt the user.*/
int LONG_CALL BasicScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai)
{
    int moveScore = 0;
    struct BattleStruct *ctx = bsys->sp;

    ai->attackerMove = ctx->battlemon[attacker].move[i];
    ai->attackerMoveEffect = ctx->moveTbl[ai->attackerMove].effect;

    if(ctx->battlemon[ai->attacker].pp[i] == 0){
        moveScore -= IMPOSSIBLE_MOVE;
    }
    if(ctx->battlemon[ai->attacker].moveeffect.tauntTurns > 0 && 
        ctx->moveTbl[ai->attackerMove].split == SPLIT_STATUS)
    {
        moveScore -= IMPOSSIBLE_MOVE; //taunted, so no status moves
    }
    if ((IsPowderMove(ai->attackerMove) || ai->attackerMove == MOVE_LEECH_SEED) && HasType(ctx, ai->defender, TYPE_GRASS))
        moveScore -= IMPOSSIBLE_MOVE;

    // if (ctx->clientPriority[ctx->attack_client] > 0 && GetBattlerAbility(ctx, ctx->attack_client) == ABILITY_PRANKSTER && HasType(ctx, defender, TYPE_DARK) && (ctx->attack_client & 1) != (defender & 1)) // used on an enemy
    if (ctx->moveTbl[ai->attackerMove].split == SPLIT_STATUS && ctx->clientPriority[ai->attacker] > 0 && ai->attackerMon.ability == ABILITY_PRANKSTER && HasType(ctx, ai->defender, TYPE_DARK) &&
        (ctx->moveTbl[ai->attackerMove].target == RANGE_ADJACENT_OPPONENTS || (ctx->moveTbl[ai->attackerMove].target == RANGE_SINGLE_TARGET && BATTLERS_ON_DIFFERENT_SIDE(ai->attacker, ai->defender))))
    {
        moveScore -= IMPOSSIBLE_MOVE; // TODO check
    }

    if (ai->effectivenessOnPlayer[i] == TYPE_MUL_NO_EFFECT) // immunity
    {
        moveScore -= IMPOSSIBLE_MOVE;
    }

    if((ctx->terrainOverlay.type == PSYCHIC_TERRAIN && ctx->terrainOverlay.numberOfTurnsLeft > 0)
        && ai->defenderMon.isGrounded
        && ctx->moveTbl[ai->attackerMove].priority > 0)
    {
        moveScore -= IMPOSSIBLE_MOVE;
	}

    switch (ai->attackerMoveEffect)
    {
        case MOVE_EFFECT_STEALTH_ROCK:
            if(ctx->side_condition[ai->defenderSide] & SIDE_STATUS_STEALTH_ROCK || ai->livingMembersDefender == 1)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_SET_SPIKES:
            if (ctx->scw[ai->defenderSide].spikesLayers >= 3)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_TOXIC_SPIKES:
            if (ctx->scw[ai->defenderSide].toxicSpikesLayers >= 2)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_STICKY_WEB:
            if(ctx->side_condition[ai->defenderSide] & SIDE_STATUS_STICKY_WEB)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_APPLY_TERRAINS:
        {
            switch (ai->attackerMove)
            {
                case MOVE_GRASSY_TERRAIN:
                    if (ctx->terrainOverlay.type == GRASSY_TERRAIN && ctx->terrainOverlay.numberOfTurnsLeft > 0)
                        moveScore -= NEVER_USE_MOVE_20;
                    break;
                case MOVE_MISTY_TERRAIN:
                    if (ctx->terrainOverlay.type == MISTY_TERRAIN && ctx->terrainOverlay.numberOfTurnsLeft > 0)
                        moveScore -= NEVER_USE_MOVE_20;
                    break;
                case MOVE_ELECTRIC_TERRAIN:
                    if (ctx->terrainOverlay.type == ELECTRIC_TERRAIN && ctx->terrainOverlay.numberOfTurnsLeft > 0)
                        moveScore -= NEVER_USE_MOVE_20;
                    break;
                case MOVE_PSYCHIC_TERRAIN:
                    if (ctx->terrainOverlay.type == PSYCHIC_TERRAIN && ctx->terrainOverlay.numberOfTurnsLeft > 0)
                        moveScore -= NEVER_USE_MOVE_20;
                    break;
            }
            break;
        }
        case MOVE_EFFECT_PARALYZE_HIT:
            if (ctx->moveTbl[ai->attackerMove].secondaryEffectChance == 100) //nuzzle
            {
                if(ai->defenderImmuneToParalysis || ai->attackerRolledMoveDamages[i] == 0)
                    moveScore -= NEVER_USE_MOVE_20;
            }
            break;
        case MOVE_EFFECT_STATUS_PARALYZE:
            if (ai->defenderImmuneToParalysis)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_STATUS_BURN:
            if (ai->defenderImmuneToBurn)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_SWITCH_HELD_ITEMS:
            if (ai->attackerMon.item == ITEM_NONE && ai->defenderMon.item == ITEM_NONE)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_STATUS_SLEEP:
        case MOVE_EFFECT_STATUS_SLEEP_NEXT_TURN:
            if (ai->defenderImmuneToSleep || (ai->defenderMon.effect_of_moves & MOVE_EFFECT_YAWN_COUNTER))
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_STATUS_POISON:
        case MOVE_EFFECT_STATUS_BADLY_POISON:
            if (ai->defenderImmuneToPoison)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_STATUS_NIGHTMARE:
           if (ctx->battlemon[ai->defender].condition2 & STATUS2_NIGHTMARE || ai->defenderMon.ability == ABILITY_MAGIC_GUARD)
               moveScore -= NEVER_USE_MOVE_20;
           break;
        case MOVE_EFFECT_FAIL_IF_NOT_USED_ALL_OTHER_MOVES:
            if (ctx->battlemon[attacker].moveeffect.lastResortCount < ai->attackerMovesKnown - 1)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_WEATHER_SANDSTORM:
             if (ctx->field_condition & WEATHER_SANDSTORM_ANY)
                 moveScore -= NEVER_USE_MOVE_20;
             break;
        case MOVE_EFFECT_WEATHER_HAIL:
            if (ctx->field_condition & WEATHER_HAIL_ANY)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_WEATHER_SNOW:
            if (ctx->field_condition & WEATHER_SNOW_ANY)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_WEATHER_RAIN:
            if (ctx->field_condition & WEATHER_RAIN_ANY)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_WEATHER_SUN:
            if (ctx->field_condition & WEATHER_SUNNY_ANY)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        default:
            break;
    }

    return moveScore;
}

BOOL LONG_CALL isMoveSpecialAiAttackingMove(u32 attackerMove)
{
    BOOL isSpecialAIMove = FALSE;
    switch (attackerMove)
    {
        case MOVE_SELF_DESTRUCT:
        case MOVE_EXPLOSION:
        case MOVE_MISTY_EXPLOSION:
        case MOVE_FINAL_GAMBIT:
        case MOVE_ROLLOUT:
        case MOVE_METEOR_BEAM:
        case MOVE_ELECTRO_SHOT:
		case MOVE_BIND:
		case MOVE_CLAMP:
		case MOVE_FIRE_SPIN:
        case MOVE_WRAP:
        case MOVE_WHIRLPOOL:
		//case MOVE_INFESTATION:
            isSpecialAIMove = TRUE;
            break;
        default:
            break;
    }
	return isSpecialAIMove;
}

int LONG_CALL SpecialAiAttackingMove(struct BattleSystem* bsys, u32 attacker, int i, struct AIContext* ai)
{
    int moveScore = 0;
    struct BattleStruct* ctx = bsys->sp;
	ai->attackerMove = ctx->battlemon[attacker].move[i];

    switch (ai->attackerMove)
    {
    case MOVE_SELF_DESTRUCT:
    case MOVE_EXPLOSION:
    case MOVE_MISTY_EXPLOSION:
        if ((ai->defenderMon.ability != ABILITY_DAMP || ai->attackerMon.hasMoldBreaker) && ai->attackerRolledMoveDamages[i] > 0) /*TODO: immunity handled? */
        {
            if (ai->attackerMon.percenthp < 10)
            {
                moveScore += 10;
            }
            else if (ai->attackerMon.percenthp < 33)
            {
                if (BattleRand(bsys) % 10 < 7) //70%
                    moveScore += 8;
            }
            else if (ai->attackerMon.percenthp < 66)
            {
                if (BattleRand(bsys) % 2) //50%
                    moveScore += 7;
            }
            else
            {
                if (BattleRand(bsys) % 20 == 0) //5%
                    moveScore += 7;
            }

            if (ai->livingMembersAttacker == 1 && ai->livingMembersDefender > 1)
                moveScore -= NEVER_USE_MOVE_20;
            if (ai->livingMembersAttacker == 1 && ai->livingMembersDefender == 1)
                moveScore -= 1;
        }
        break;
    case MOVE_FINAL_GAMBIT:
        if (ai->attackerMovesFirst && ai->attackerMon.hp > ai->defenderMon.hp)
            moveScore += 8;
        else if (ai->attackerMovesFirst && ai->playerCanOneShotMonWithAnyMove)
            moveScore += 7;
        else
            moveScore += 6;
        break;
    case MOVE_ROLLOUT:
        moveScore += 7;
        break;
    case MOVE_METEOR_BEAM:
        if (ai->attackerMon.item == ITEM_POWER_HERB)
            moveScore += 9;
        else
            moveScore -= NEVER_USE_MOVE_20;
        break;
    case MOVE_ELECTRO_SHOT:
        if (ai->attackerMon.item == ITEM_POWER_HERB || ctx->field_condition & WEATHER_RAIN_ANY)
            moveScore += 9;
        else
            moveScore -= NEVER_USE_MOVE_20;
        break;
    case MOVE_BIND:
    case MOVE_CLAMP:
    case MOVE_FIRE_SPIN:
    case MOVE_WRAP:
    case MOVE_WHIRLPOOL:
		//case MOVE_INFESTATION:
        moveScore += 6;
        if (BattleRand(bsys) % 10 < 2)
            moveScore += 2;
        break;
    default:
        break;
    }

	return moveScore;
}


int LONG_CALL DamagingMoveScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai)
{
    int moveScore = 0;
    struct BattleStruct *ctx = bsys->sp;
    BOOL isMoveHighestDamage = 0;

     if(ctx->moveTbl[ai->attackerMove].split == SPLIT_STATUS)
     {
        return 0;
     }

    ai->attackerMove = ctx->battlemon[attacker].move[i];
    ai->attackerMoveEffect = ctx->moveTbl[ai->attackerMove].effect;

    //TODO deal with contrary

    if (isMoveSpecialAiAttackingMove(ai->attackerMove))
    {
		moveScore += SpecialAiAttackingMove(bsys, attacker, i, ai);
    }
    else if (ai->attackerMoveEffect == MOVE_EFFECT_ONE_HIT_KO)
    {
        switch (ai->attackerMove)
        {
        case MOVE_SHEER_COLD:
            if (HasType(ctx, ai->defender, TYPE_ICE))
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_GUILLOTINE:
        case MOVE_HORN_DRILL:
            if (HasType(ctx, ai->defender, TYPE_NORMAL) && ai->attackerMon.ability == ABILITY_SCRAPPY)
                moveScore -= NEVER_USE_MOVE_20;
             break;
        case MOVE_FISSURE:
            if (ai->attackerMove == MOVE_FISSURE && (HasType(ctx, ai->defender, TYPE_FLYING) || (!ai->attackerMon.hasMoldBreaker && ai->defenderMon.ability == ABILITY_LEVITATE)))
                moveScore -= NEVER_USE_MOVE_20;
             break;
        default:
            break;
        }
        if (ai->attackerMon.level > ai->defenderMon.level)
        {
            if (BattleRand(bsys) % 3 == 0)
                moveScore += 9;
        }
        else
        {
            moveScore -= NEVER_USE_MOVE_20;
        }
    }
    else if (ai->attackerRolledMaxDamage == ai->attackerRolledMoveDamages[i])
    {
        isMoveHighestDamage = 1;
        moveScore += 6;
        if(BattleRand(bsys) % 10 < 2)
            moveScore += 2;
    }

    if (ai->monCanOneShotPlayerWithMove[i]) // if ai sees kill with this move
    {
        if (ai->attackerMovesFirst || (ai->defenderMovesFirst && ctx->moveTbl[ai->attackerMove].priority > 0))
        {
            moveScore += 6; //and is faster/has priority
        }
        else if (ai->defenderMovesFirst)
        {
            moveScore += 3;
        }
        if (ai->attackerMon.ability == ABILITY_MOXIE || ai->attackerMon.ability == ABILITY_BEAST_BOOST || 
            ai->attackerMon.ability == ABILITY_CHILLING_NEIGH || ai->attackerMon.ability == ABILITY_GRIM_NEIGH)
        {
             moveScore += 1;
        }
    }

    if (ai->attackerMoveEffect == MOVE_EFFECT_HIGH_CRITICAL && (ai->effectivenessOnPlayer[i] == TYPE_MUL_SUPER_EFFECTIVE || ai->effectivenessOnPlayer[i] == TYPE_MUL_DOUBLE_SUPER_EFFECTIVE || ai->effectivenessOnPlayer[i] == TYPE_MUL_TRIPLE_SUPER_EFFECTIVE))
    {
        if(BattleRand(bsys) % 2 == 0)
            moveScore += 2;
    }

    if (ai->defenderMovesFirst && ai->playerCanOneShotMonWithAnyMove)
    {
        if (ctx->moveTbl[ai->attackerMove].priority > 0)
        {
            moveScore += 11;
        }
    }

    if (!isMoveHighestDamage && ai->attackerMoveEffect == MOVE_EFFECT_LOWER_SPEED_HIT && ctx->moveTbl[ai->attackerMove].secondaryEffectChance == 100)
    {
        if (ai->defenderMovesFirst && ((ai->defenderMon.ability != ABILITY_CLEAR_BODY && ai->defenderMon.ability != ABILITY_WHITE_SMOKE && ai->defenderMon.ability != ABILITY_CONTRARY)
            || ai->attackerMon.hasMoldBreaker))
        {
            moveScore += 6;
        }
        else
        {
            moveScore += 5;
        }
        if (BattleTypeGet(bsys) & (BATTLE_TYPE_MULTI | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TAG))
        {
            if (ai->attackerMove == MOVE_ICY_WIND || ai->attackerMove == MOVE_ELECTROWEB)
                moveScore += 1;
        }
    }

    if (!isMoveHighestDamage && ctx->moveTbl[ai->attackerMove].secondaryEffectChance == 100
        && ((ai->attackerMoveEffect == MOVE_EFFECT_LOWER_SP_ATK_HIT && ai->defenderHasAtleastOneSpecialMove)
        ||  (ai->attackerMoveEffect == MOVE_EFFECT_LOWER_ATTACK_HIT && ai->defenderHasAtleastOnePhysicalMove)))
    {
        if ((ai->defenderMon.ability != ABILITY_CLEAR_BODY && ai->defenderMon.ability != ABILITY_WHITE_SMOKE && ai->defenderMon.ability != ABILITY_CONTRARY) || 
            ai->attackerMon.hasMoldBreaker)
        {
            moveScore += 6;
        }
        else
        {
            moveScore += 5;
        }
        if (BattleTypeGet(bsys) & (BATTLE_TYPE_MULTI | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TAG))
        {
            if (ctx->moveTbl[ai->attackerMove].target == RANGE_ADJACENT_OPPONENTS)
                moveScore += 1;
        }
    }

    if (!isMoveHighestDamage && ai->attackerMoveEffect == MOVE_EFFECT_SWITCH_HIT)
    {
        if (ai->livingMembersAttacker > 1 && ai->attackerRolledMoveDamages[i] > 0) //no immunity
        {
            moveScore += 6;
            if (BattleRand(bsys) % 10 < 2)
                moveScore += 1;

            BOOL onlyIneffectiveMoves = BattleAI_AttackerHasOnlyIneffectiveMoves(ctx, attacker, ai->attackerMovesKnown, ai->effectivenessOnPlayer);
            if (onlyIneffectiveMoves)
                moveScore += ai->livingMembersAttacker;
        }
        /*
        if (ai->attackerMon.ability == ABILITY_REGENERATOR && ai->attackerMon.percenthp < 67)
            moveScore += 1;
        */
    }

    switch(ai->attackerMove)
    {
        case MOVE_RELIC_SONG: //TODO
            break;
        case MOVE_FUTURE_SIGHT:
            if (ai->attackerMovesFirst && ai->playerCanOneShotMonWithAnyMove)
                moveScore += 8;
            else
                moveScore += 6;
            break;
        case MOVE_ACID_SPRAY:
        {
            if ((ai->defenderMon.ability != ABILITY_CLEAR_BODY && ai->defenderMon.ability != ABILITY_WHITE_SMOKE && ai->defenderMon.ability != ABILITY_CONTRARY && ai->defenderMon.ability != ABILITY_BULLETPROOF) || 
                ai->attackerMon.hasMoldBreaker)
            {
                moveScore += 6;
            }
            break;
        }
        case MOVE_SUCKER_PUNCH:
        {
            if (ai->attackerLastUsedMove == MOVE_SUCKER_PUNCH)
            {
                if(BattleRand(bsys) % 2 == 0)
                    moveScore -= NEVER_USE_MOVE_20;
            }
            break;
        }
        case MOVE_FAKE_OUT:
        {
            if (ai->attackerTurnsOnField == 0 && ((ai->defenderMon.ability != ABILITY_SHIELD_DUST && ai->defenderMon.ability != ABILITY_INNER_FOCUS) ||
                ai->attackerMon.hasMoldBreaker))
            {
                moveScore += 9;
            }
            else
            {
                moveScore -= NEVER_USE_MOVE_20;
			}
            break;
        }
        case MOVE_PURSUIT:
        {
            if (ai->attackerMovesFirst)
                moveScore += 3;

            if (ai->monCanOneShotPlayerWithMove[i])
                moveScore += 10;
            else if (ai->defenderMon.percenthp < 20)
                moveScore += 10;
            else if (ai->defenderMon.percenthp < 40)
            {
                if(BattleRand(bsys) % 2 == 0)
                    moveScore += 8;
            }
            break;
        }
        case MOVE_FELL_STINGER:
        {
            if (ctx->battlemon[ai->attacker].states[STAT_ATTACK] < 12 && ai->monCanOneShotPlayerWithMove[i])
            {
                if (ai->attackerMovesFirst)
                {
                    moveScore += 10;
                }
                //TODO
            }
            break;
        }
        case MOVE_EARTHQUAKE:
        case MOVE_MAGNITUDE:
        {
            if (BattleTypeGet(bsys) & (BATTLE_TYPE_MULTI | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TAG))
            {
                if (ai->isPartnerGrounded) //or about to use magnetrise && faster
                {
                    if (ctx->battlemon[BATTLER_ALLY(ai->attacker)].hp)
                    {
                        if (HasType(ctx, BATTLER_ALLY(ai->attacker), TYPE_POISON) ||
                            HasType(ctx, BATTLER_ALLY(ai->attacker), TYPE_STEEL) ||
                            HasType(ctx, BATTLER_ALLY(ai->attacker), TYPE_FIRE) ||
                            HasType(ctx, BATTLER_ALLY(ai->attacker), TYPE_ROCK))
                        {
                            moveScore -= 10;
                        }  
                        else
                            moveScore -= 3;
                    }
                }
                else
                    moveScore += 2;
            }
        }
        default:
            break;
    }

    return moveScore;
}

int LONG_CALL offensiveSetup(struct BattleSystem* bsys UNUSED, u32 attacker UNUSED, int i UNUSED, struct AIContext* ai)
{
	int moveScore = 0;
    if (ai->isDefenderIncapacitated)
        moveScore += 3;
    if (ai->attackerMovesFirst)
    {
        if (!ai->playerCanOneShotMonWithAnyMove)
            moveScore += 3;
    }
    else
    {
        if (2*ai->maxDamageReceived >= ai->attackerMon.hp)
            moveScore -= 5;
    }
    
    return moveScore;
}
int LONG_CALL defensiveSetup(struct BattleSystem* bsys UNUSED, u32 attacker UNUSED, int i UNUSED, struct AIContext* ai)
{
    int moveScore = 0;
    if (ai->isDefenderIncapacitated)
        moveScore += 2;
    if (ai->defenderMovesFirst && 2 * ai->maxDamageReceived >= ai->attackerMon.hp)
        moveScore -= 5;
    return moveScore;
}


int LONG_CALL SetupScoring(struct BattleSystem* bsys, u32 attacker, int i, struct AIContext* ai)
{
    int moveScore = 0;
    struct BattleStruct* ctx = bsys->sp;
    BOOL hasSitrus = (ai->attackerMon.item == ITEM_SITRUS_BERRY);
    BOOL shouldSetup = TRUE;
	BOOL isSetupMove = TRUE;

    ai->attackerMove = ctx->battlemon[attacker].move[i];
    ai->attackerMoveEffect = ctx->moveTbl[ai->attackerMove].effect;

    if (ai->playerCanOneShotMonWithAnyMove || (ai->defenderMon.ability == ABILITY_UNAWARE && (BattleRand(bsys) % 4 > 0)))
        shouldSetup = FALSE;

    switch (ai->attackerMoveEffect)
    {
        case MOVE_EFFECT_WEATHER_RAIN:
            if (ctx->field_condition & WEATHER_RAIN_ANY)
                moveScore -= NEVER_USE_MOVE_20;
            else
            {
                if (ai->attackerMon.ability == ABILITY_SWIFT_SWIM
                    || ai->attackerMon.ability == ABILITY_RAIN_DISH
                    || ai->attackerMon.ability == ABILITY_HYDRATION
                    || ai->attackerMon.ability == ABILITY_DRY_SKIN)
                {
                    moveScore += 9;
                }
                if (ai->attackerMon.item == ITEM_DAMP_ROCK)
                    moveScore += 1;
            }
			break;
        case MOVE_EFFECT_WEATHER_HAIL:
        case MOVE_EFFECT_WEATHER_SNOW:
            // case MOVE_EFFECT_SNOWSCAPE:
            if (ctx->field_condition & (WEATHER_HAIL_ANY | WEATHER_SNOW_ANY))
                moveScore -= NEVER_USE_MOVE_20;
            else
            {
                if (HasType(ctx, attacker, TYPE_ICE)
                    || ai->attackerMon.ability == ABILITY_ICE_BODY || ai->attackerMon.ability == ABILITY_SNOW_CLOAK
                    || ai->attackerMon.ability == ABILITY_SLUSH_RUSH)
                {
                    moveScore += 9;
                }
                if (ai->attackerMon.item == ITEM_ICY_ROCK)
                    moveScore += 1;
            }
            break;
        case MOVE_EFFECT_WEATHER_SUN:
            if (ctx->field_condition & WEATHER_SUNNY_ANY)
                moveScore -= NEVER_USE_MOVE_20;
            else
            {
				if (HasType(ctx, attacker, TYPE_FIRE) || ai->attackerMon.ability == ABILITY_SOLAR_POWER) //|| ai->attackerMon.ability == ABILITY_CHLOROPHYL
                    moveScore += 9;
                if (ai->attackerMon.item == ITEM_HEAT_ROCK)
                    moveScore += 1;
            }
            break;
        case MOVE_EFFECT_WEATHER_SANDSTORM:
            if (ctx->field_condition & WEATHER_SANDSTORM_ANY)
                moveScore -= NEVER_USE_MOVE_20;
            else
            {
                if (HasType(ctx, attacker, TYPE_ROCK) || HasType(ctx, attacker, TYPE_GROUND) || HasType(ctx, attacker, TYPE_STEEL)
                    || ai->attackerMon.ability == ABILITY_SAND_FORCE || ai->attackerMon.ability == ABILITY_SAND_RUSH || ai->attackerMon.ability == ABILITY_SAND_VEIL)
                {
                    moveScore += 9;
                }
                if (ai->attackerMon.item == ITEM_SMOOTH_ROCK)
                    moveScore += 1;
            }
            break;
        case MOVE_EFFECT_COPY_STAT_CHANGES:
            {
                u8 sumStatChange = BattlerPositiveStatChangesSum(bsys, ai->defender, ai);
                if (sumStatChange > 0)
                    moveScore += 5;
                if (sumStatChange > 1)
                    moveScore += sumStatChange;
                break;
            }
        case MOVE_EFFECT_EVA_UP:
        case MOVE_EFFECT_EVA_UP_2:
        case MOVE_EFFECT_EVA_UP_3:
        case MOVE_EFFECT_EVA_UP_2_MINIMIZE:
			moveScore += 6;
            if (ai->defenderMon.ability == ABILITY_NO_GUARD || ctx->battlemon[attacker].states[STAT_EVASION] >= 10)
            {
                moveScore -= NEVER_USE_MOVE_20;
            }
            else if (2 * ai->maxDamageReceived < ai->attackerMon.hp)
            {
                moveScore += 2;
            }
            if (ai->isDefenderIncapacitated)
                moveScore += 2;
            break;
        case MOVE_EFFECT_SPEED_UP_2:
        case MOVE_EFFECT_AUTOTOMIZE:
            if (ai->defenderMovesFirst)
                moveScore += 7;
            else
				moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_CRIT_UP_2:
            if (ai->defenderMon.ability == ABILITY_SHELL_ARMOR || ai->defenderMon.ability == ABILITY_BATTLE_ARMOR)
                moveScore -= NEVER_USE_MOVE_20;
            else if (ai->attackerMon.ability == ABILITY_SUPER_LUCK || ai->attackerMon.ability == ABILITY_SNIPER || ai->attackerMon.item == ITEM_SCOPE_LENS) //or hight crit moves
                moveScore += 7;
            else
                moveScore += 5;
            break;
        //case MOVE_EFFECT_ATK_SP_ATK_SPEED_UP_2_LOSE_HALF_MAX_HP: //filet away
        case MOVE_EFFECT_MAX_ATK_LOSE_HALF_MAX_HP: //belly drum
            moveScore += 4;
            if (ai->isDefenderIncapacitated)
                moveScore += 5;
            else if (2 * ai->attackerMon.hp > ai->attackerMon.maxhp)
            {
                if (hasSitrus)
                {
                    if (2 * ai->maxDamageReceived < (u32)(25 * ai->attackerMon.hp/10 - ai->attackerMon.maxhp)) //D < 1.125*Hp - 0.5maxHp
                        moveScore += 4;
                }
                else if (2 * ai->maxDamageReceived < (u32)(2 * ai->attackerMon.hp - ai->attackerMon.maxhp)) // D < Hp - 0.5maxHp
                    moveScore += 4;
            }
            break;
        case MOVE_EFFECT_RAISE_ALL_STATS_LOSE_THIRD_MAX_HP: //clangorous soul
        case MOVE_EFFECT_ATK_SP_ATK_SPEED_UP_2_DEF_SP_DEF_DOWN: //shell smash
            moveScore += 6;
            if (ai->isDefenderIncapacitated)
                moveScore += 3;
            if (ai->attackerMovesFirst) //if ai is slower, no defense drop for hit
            {
                if (ai->attackerMon.item == ITEM_WHITE_HERB || 2 * ai->maxDamageReceived < ai->attackerMon.hp)//simple heuristic to not calc exact dmg. 
                    moveScore += 2;
                else
                    moveScore -= 2;
            }
            if (ctx->battlemon[attacker].states[STAT_ATTACK] > 7 || ctx->battlemon[attacker].states[STAT_SPATK] > 7)
                moveScore -= NEVER_USE_MOVE_20;
            break;
        case MOVE_EFFECT_RAISE_SP_ATK_HIT: //charge beam
        case MOVE_EFFECT_RAISE_ATTACK_HIT: //PuP
            if (ctx->moveTbl[ai->attackerMove].secondaryEffectChance < 100 || ai->attackerRolledMaxDamage == ai->attackerRolledMoveDamages[i]) // if other move (like Metal Claw), or highest damage 
                break;
            FALLTHROUGH;
        case MOVE_EFFECT_ATK_UP:   //howl
		case MOVE_EFFECT_ATK_UP_2: //swords dance
        case MOVE_EFFECT_ATK_UP_3:
        case MOVE_EFFECT_ATK_SPEED_UP: //dragon dance
        case MOVE_EFFECT_SP_ATK_UP: //growth
        case MOVE_EFFECT_ATK_SP_ATK_UP: //work up
        case MOVE_EFFECT_ATK_ACC_UP: //hone claws
        case MOVE_EFFECT_SPEED_UP_2_ATK_UP: //shift gear
        case MOVE_EFFECT_TIDY_UP: //tidy up is basically ddance
		case MOVE_EFFECT_RANDOM_STAT_UP_2: //accupressure
        case MOVE_EFFECT_ATK_DEF_SPEED_UP: //victory dance
            moveScore += 6;
            moveScore += offensiveSetup(bsys, attacker, i, ai);
            break;
        case MOVE_EFFECT_DEF_UP:
        case MOVE_EFFECT_DEF_UP_2:
        case MOVE_EFFECT_DEF_UP_3:
        case MOVE_EFFECT_SP_DEF_UP:
        case MOVE_EFFECT_SP_DEF_UP_2: 
        case MOVE_EFFECT_SP_DEF_UP_3:
        case MOVE_EFFECT_DEF_UP_DOUBLE_ROLLOUT_POWER:
        case MOVE_EFFECT_SP_DEF_UP_DOUBLE_ELECTRIC_POWER: // Charge
        case MOVE_EFFECT_DEF_SP_DEF_UP:
        case MOVE_EFFECT_STOCKPILE:
            //case MOVE_EFFECT_STUFF_CHEEKS:
            moveScore += 6;
            moveScore += defensiveSetup(bsys, attacker, i, ai);
            break;
		case MOVE_EFFECT_SP_ATK_SP_DEF_SPEED_UP: //quiver dance
        case MOVE_EFFECT_SP_ATK_SP_DEF_UP: // Calm Mind
            moveScore += 6;
            if (ai->defenderHasAtleastOnePhysicalMove == FALSE && ai->defenderHasAtleastOneSpecialMove)
                moveScore += defensiveSetup(bsys, attacker, i, ai);
            else
                moveScore += offensiveSetup(bsys, attacker, i, ai);
            break;
        case MOVE_EFFECT_CURSE:
            if (HasType(ctx, attacker, TYPE_GHOST))
                break;
            FALLTHROUGH;
		case MOVE_EFFECT_ATK_DEF_UP: // Bulk Up
        case MOVE_EFFECT_ATK_DEF_ACC_UP: //coil
        //case MOVE_EFFECT_NO_RETREAT: // No Retreat
            moveScore += 6;
            if (ai->defenderHasAtleastOnePhysicalMove && ai->defenderHasAtleastOneSpecialMove == FALSE)
                moveScore += defensiveSetup(bsys, attacker, i, ai);
            else
                moveScore += offensiveSetup(bsys, attacker, i, ai);
            break;
        case MOVE_EFFECT_CHARGE_TURN_ATK_SP_ATK_SPEED_UP_2: //geomancy
            if (ai->attackerMon.item == ITEM_POWER_HERB)
                moveScore += 9;
            else
				moveScore -= NEVER_USE_MOVE_20;
            break;
        default:
            isSetupMove = FALSE;
            break;
    }
    if (!shouldSetup)
        moveScore = (0 - NEVER_USE_MOVE_20);
    if (!isSetupMove)
        moveScore = 0;

    return moveScore;
}

int LONG_CALL HarassmentScoring(struct BattleSystem* bsys, u32 attacker, int i, struct AIContext* ai)
{
    int moveScore = 0;
    BOOL sharesMoves = FALSE;
    struct BattleStruct* ctx = bsys->sp;

    ai->attackerMove = ctx->battlemon[attacker].move[i];
    ai->attackerMoveEffect = ctx->moveTbl[ai->attackerMove].effect;

    switch (ai->attackerMoveEffect)
    {
    case MOVE_EFFECT_STEALTH_ROCK:
    case MOVE_EFFECT_SET_SPIKES:
    case MOVE_EFFECT_TOXIC_SPIKES:
        if (ai->attackerTurnsOnField == 0)
            moveScore += 8;
        else
            moveScore += 6;
        if (BattleRand(bsys) % 4 != 0)
            moveScore += 1;
        if (ctx->scw[ai->defenderSide].spikesLayers > 0 && ai->attackerMoveEffect == MOVE_EFFECT_SET_SPIKES)
            moveScore -= 1;
        if (ctx->scw[ai->defenderSide].toxicSpikesLayers > 0 && ai->attackerMoveEffect == MOVE_EFFECT_TOXIC_SPIKES)
            moveScore -= 1;
        break;
    case MOVE_EFFECT_STICKY_WEB:
        if (ai->attackerTurnsOnField == 0)
            moveScore += 9;
        else
            moveScore += 6;
        if (BattleRand(bsys) % 4 != 0)
            moveScore += 3;
        break;
    case MOVE_EFFECT_SURVIVE_WITH_1_HP:
        if (ctx->protectSuccessTurns[ai->attacker] == 1)
        {
            if (BattleRand(bsys) % 2 == 0)
                moveScore -= NEVER_USE_MOVE_20;
        }
        else if (ctx->protectSuccessTurns[ai->attacker] > 1)
        {
            moveScore -= IMPOSSIBLE_MOVE;
        }
        else if(ai->defenderMovesFirst && (ai->playerCanOneShotMonWithAnyMove || !ai->monCanOneShotPlayerWithAnyMove))
        {
            moveScore += 6;
            if (ai->attackerMon.item == ITEM_CUSTAP_BERRY || ai->attackerMon.item == ITEM_SALAC_BERRY)
                moveScore += 2;
        }
        else
        {
            moveScore -= NEVER_USE_MOVE_20;
		}
        break;
    //case MOVE_EFFECT_KINGS_SHIELD:
    //case MOVE_EFFECT_OBSTRUCT:
    //case MOVE_EFFECT_SPIKEY_SHIELD:
    case MOVE_EFFECT_PROTECT: //TODO
    {
        //BOOL monDiesEndTurn = monDiesFromResidualDamage(ctx, ai->attacker, ai->attackerMon.condition, (ctx->battlemon[ai->attacker].effect_of_moves & MOVE_EFFECT_FLAG_LEECH_SEED_ACTIVE));
        moveScore += 6;
        if ((ai->attackerMon.condition & (STATUS_POISON | STATUS_BURN | STATUS_PARALYSIS)) || (ctx->battlemon[ai->attacker].effect_of_moves & MOVE_EFFECT_FLAG_LEECH_SEED_ACTIVE)) //TODO
            moveScore -= 1;
        if ((ai->defenderMon.condition & (STATUS_POISON | STATUS_BURN | STATUS_PARALYSIS)) || (ctx->battlemon[ai->defender].effect_of_moves & MOVE_EFFECT_FLAG_LEECH_SEED_ACTIVE)) //TODO
            moveScore += 1;
        if (ai->attackerTurnsOnField == 0 && ((BattleTypeGet(bsys) & (BATTLE_TYPE_MULTI | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TAG)) == 0))
            moveScore -= 1;
        if (ctx->protectSuccessTurns[ai->attacker] == 1)
        {
            if (BattleRand(bsys) % 2 == 0)
                moveScore -= NEVER_USE_MOVE_20;
        }
        else if (ctx->protectSuccessTurns[ai->attacker] > 1)
        {
            moveScore -= IMPOSSIBLE_MOVE;
        }
        break;
    }
    case MOVE_EFFECT_MAKE_SHARED_MOVES_UNUSEABLE:
        if (ctx->battlemon[ai->defender].effect_of_moves & MOVE_EFFECT_FLAG_IMPRISONED)
            moveScore -= NEVER_USE_MOVE_20;
        else
        {
            for (int j = 0; j < CLIENT_MAX; ++j)
            {
                for (int k = 0; k < CLIENT_MAX; ++k)
                {
                    if (ctx->battlemon[ai->attacker].move[k] == ctx->battlemon[ai->defender].move[j] && ctx->battlemon[ai->defender].move[j] != MOVE_NONE)
                    {
                        sharesMoves = TRUE;
                        break;
                    }
                }
                if (sharesMoves == TRUE)
                    break;
            }
            if (sharesMoves == TRUE)
                moveScore += 9;
        }
        break;
    case MOVE_EFFECT_PASS_STATS_AND_STATUS:
        if ((ai->isDoubleBattle && ai->livingMembersAttacker > 2) || (!ai->isDoubleBattle && ai->livingMembersAttacker > 1))
        {
            if ((BattlerPositiveStatChangesSum(bsys, ai->attacker, ai) >= 1) || ctx->battlemon[ai->attacker].condition2 & STATUS2_SUBSTITUTE)
                moveScore += 14;
        }
        if (ai->livingMembersAttacker == 1)
            moveScore -= NEVER_USE_MOVE_20;
        break;
    case MOVE_EFFECT_DOUBLE_SPEED_3_TURNS:
        if (ai->defenderMovesFirst) // or partner is slower
            moveScore += 9;
        else
            moveScore += 5;
        if (ctx->side_condition[ai->attackerSide] & SIDE_STATUS_TAILWIND)
            moveScore -= NEVER_USE_MOVE_20;
        break;
    case MOVE_EFFECT_TRICK_ROOM:
        if (ai->defenderMovesFirst) // or partner is slower
            moveScore += 10;
        else
            moveScore += 5;

        if (ctx->field_condition & FIELD_STATUS_TRICK_ROOM)
            moveScore -= NEVER_USE_MOVE_20;
        break;
    case MOVE_EFFECT_APPLY_TERRAINS:
        if (ai->attackerMon.item == ITEM_TERRAIN_EXTENDER)
            moveScore += 9;
        else
            moveScore += 8;
        break;
    case MOVE_EFFECT_SET_REFLECT:
        if (ctx->side_condition[ai->attackerSide] & SIDE_STATUS_REFLECT)
            moveScore -= NEVER_USE_MOVE_20;
        else
        {
            moveScore += 6;
            if (ai->defenderHasAtleastOnePhysicalMove)
            {
                if (ai->attackerMon.item == ITEM_LIGHT_CLAY)
                    moveScore += 1;
                if (BattleRand(bsys) % 2 == 0)
                    moveScore += 1;
            }
        }
        break;
    case MOVE_EFFECT_SET_LIGHT_SCREEN:
        if (ctx->side_condition[ai->attackerSide] & SIDE_STATUS_LIGHT_SCREEN)
            moveScore -= NEVER_USE_MOVE_20;
        else
        {
            moveScore += 6;
            if (ai->defenderHasAtleastOneSpecialMove)
            {
                if (ai->attackerMon.item == ITEM_LIGHT_CLAY)
                    moveScore += 1;
                if (BattleRand(bsys) % 2 == 0)
                    moveScore += 1;
            }
        }
        break;
    case MOVE_EFFECT_BOOST_ALLY_POWER_BY_50_PERCENT: //TODO
    case MOVE_EFFECT_MAKE_GLOBAL_TARGET:
        break;
    case MOVE_EFFECT_SET_SUBSTITUTE:
        moveScore += 6;
        if (ai->attackerMovesFirst && (ctx->battlemon[ai->defender].effect_of_moves & MOVE_EFFECT_FLAG_LEECH_SEED_ACTIVE))
            moveScore += 2;
        if (ai->defenderMon.condition & STATUS_SLEEP)
            moveScore += 2;
        if (BattleRand(bsys) % 2 == 0)
            moveScore -= 1;
        if (ai->defenderMon.ability == ABILITY_INFILTRATOR || ai->attackerMon.percenthp < 50)
            moveScore -= NEVER_USE_MOVE_20;
        if (battlerHasSoundBasedMove(bsys, ai->defender, ai))
            moveScore -= 8;
        break;
    case MOVE_EFFECT_FAINT_AND_ATK_SP_ATK_DOWN_2:
        moveScore += 6;
        if (ai->attackerMon.percenthp < 10)
        {
            moveScore += 10;
        }
        else if (ai->attackerMon.percenthp < 33)
        {
            if (BattleRand(bsys) % 10 < 7) //70%
                moveScore += 8;
        }
        else if (ai->attackerMon.percenthp < 66)
        {
            if (BattleRand(bsys) % 2) //50%
                moveScore += 7;
        }
        else
        {
            if (BattleRand(bsys) % 20 == 0) //5%
                moveScore += 7;
        }

        if (ai->livingMembersAttacker == 1)
            moveScore -= NEVER_USE_MOVE_20;
        break;
    case MOVE_EFFECT_PARALYZE_HIT:
        if (ctx->moveTbl[ai->attackerMove].secondaryEffectChance < 100 || ai->attackerRolledMoveDamages[i] > 0 /*no immunity*/) //nuzzle
            break;
        FALLTHROUGH;
    case MOVE_EFFECT_STATUS_PARALYZE:
        if (ai->defenderMovesFirst || // and slower after para
            (BattlerKnowsMove(bsys, attacker, MOVE_HEX, ai) == TRUE) ||
            (battlerKnowsFlinchingMove(bsys, attacker, ai) == TRUE) ||
            ctx->battlemon[ai->defender].condition2 & STATUS2_ATTRACT ||
            ctx->battlemon[ai->defender].condition2 & STATUS2_CONFUSION)
            moveScore += 8;
        else
            moveScore += 7;
        if (BattleRand(bsys) % 2) //50%
            moveScore -= 1;
        break;
    case MOVE_EFFECT_STATUS_BURN:
        moveScore += 6;
        if (BattleRand(bsys) % 3 == 0)
        {
            if ((BattlerKnowsMove(bsys, attacker, MOVE_HEX, ai) == TRUE)) // or partner
                moveScore += 1;
            if (ai->defenderHasAtleastOnePhysicalMove)
                moveScore += 1;
        }
        break;
    case MOVE_EFFECT_STATUS_SLEEP:
    case MOVE_EFFECT_STATUS_SLEEP_NEXT_TURN:
        moveScore += 6;
        if (!ai->monCanOneShotPlayerWithAnyMove && (BattleRand(bsys) % 4 == 0)) // no kill
        {
            if (!ai->defenderImmuneToSleep)
                moveScore += 1;
            if (((BattlerKnowsMove(bsys, attacker, MOVE_DREAM_EATER, ai) == TRUE) ||
                (BattlerKnowsMove(bsys, attacker, MOVE_NIGHTMARE, ai) == TRUE)) &&
                (BattlerKnowsMove(bsys, attacker, MOVE_SNORE, ai) == FALSE) &&
                (BattlerKnowsMove(bsys, attacker, MOVE_SLEEP_TALK, ai) == FALSE))
                moveScore += 1;
            if ((BattlerKnowsMove(bsys, attacker, MOVE_HEX, ai) == TRUE)) // or partner
                moveScore += 1;
        }
        break;
    case MOVE_EFFECT_STATUS_POISON:
    case MOVE_EFFECT_STATUS_BADLY_POISON:
        moveScore += 6;
        if (!ai->monCanOneShotPlayerWithAnyMove && (BattleRand(bsys) % 3 == 0)) // no kill
        {
            if (!ai->defenderImmuneToPoison && ai->defenderMon.percenthp > 20)
            {
                if (ai->maxDamageReceived < ai->attackerMon.hp / 2 &&
                    (BattlerKnowsMove(bsys, attacker, MOVE_HEX, ai) ||
                        BattlerKnowsMove(bsys, attacker, MOVE_VENOM_DRENCH, ai) ||
                        BattlerKnowsMove(bsys, attacker, MOVE_VENOSHOCK, ai) ||
                        ai->attackerMon.ability == ABILITY_MERCILESS))
                {
                    moveScore += 2;
                }
            }
        }
        break;
    case MOVE_EFFECT_SWITCH_HELD_ITEMS:
        moveScore += 5;
        if ((ai->attackerMon.item == ITEM_TOXIC_ORB && !ai->defenderImmuneToPoison) ||
            (ai->attackerMon.item == ITEM_FLAME_ORB && !ai->defenderImmuneToBurn) ||
            (ai->attackerMon.item == ITEM_BLACK_SLUDGE && !HasType(ctx, ai->defender, TYPE_POISON)))
        {
            moveScore += 1;
            if (BattleRand(bsys) % 2) //50%
                moveScore += 1;
		}
        else if (ai->attackerMon.item == ITEM_IRON_BALL || ai->attackerMon.item == ITEM_LAGGING_TAIL || ai->attackerMon.item == ITEM_STICKY_BARB)
        {
            moveScore += 2;
        }
        break;
    case MOVE_EFFECT_SP_ATK_UP_CAUSE_CONFUSION:
    case MOVE_EFFECT_ATK_UP_2_STATUS_CONFUSION:
    case MOVE_EFFECT_STATUS_CONFUSE:
        if ((ctx->battlemon[ai->defender].condition2 & STATUS2_CONFUSION)
            || (ctx->side_condition[ai->defenderSide] & SIDE_STATUS_SAFEGUARD) 
            || (!ai->attackerMon.hasMoldBreaker && ai->defenderMon.ability == ABILITY_OWN_TEMPO))
        {
            moveScore -= NEVER_USE_MOVE_20;
        }
        else
        {
            moveScore += 6;
            if (ai->defenderMon.condition & STATUS_PARALYSIS || ctx->battlemon[ai->defender].condition2 & STATUS2_ATTRACT)
                moveScore += 2;
        }
        break;
    case MOVE_EFFECT_INFATUATE:
        if (ctx->battlemon[ai->defender].condition2 & STATUS2_ATTRACT
            || (!ai->attackerMon.hasMoldBreaker && ai->defenderMon.ability == ABILITY_OBLIVIOUS)
#ifndef ATTRACT_WORK_ON_ALL_SEXES
            || ai->attackerMon.sex != POKEMON_GENDER_UNKNOWN
            || ai->attackerMon.sex == ai->defenderMon.sex
#endif // !ATTRACT_WORK_ON_ALL_SEXES
            )

        {
            moveScore -= NEVER_USE_MOVE_20;
        }
        else
        {
            moveScore += 6;
            if (ai->defenderMon.condition & STATUS_PARALYSIS || ctx->battlemon[ai->defender].condition2 & STATUS2_CONFUSION)
                moveScore += 2;
        }
        break;
    case MOVE_EFFECT_STATUS_LEECH_SEED:
        if ((ctx->battlemon[ai->defender].effect_of_moves & MOVE_EFFECT_FLAG_LEECH_SEED_ACTIVE) ||
            HasType(ctx, ai->defender, TYPE_GRASS) ||
            ai->defenderMon.ability == ABILITY_MAGIC_GUARD)
        {
            moveScore -= NEVER_USE_MOVE_20;
        }
        else
        {
            moveScore += 6;
            if (ai->playerCanOneShotMonWithAnyMove == FALSE && ai->defenderMon.percenthp > 20)
            {
                moveScore += 1;
                if (BattleRand(bsys) % 2) //50%
                    moveScore += 1;
            }
            if (ctx->battlemon[ai->defender].condition2 & STATUS2_ATTRACT || ctx->battlemon[ai->defender].condition2 & STATUS2_CONFUSION)
                moveScore += 1;
            if (ai->defenderMon.condition & (STATUS_POISON | STATUS_BURN | STATUS_PARALYSIS))
                moveScore += 1;
        }
        break;
    case MOVE_EFFECT_USE_RANDOM_LEARNED_MOVE_SLEEP:
    case MOVE_EFFECT_DAMAGE_WHILE_ASLEEP:
        if (ctx->battlemon[attacker].condition & STATUS_SLEEP)
        {
            moveScore += 6;
            if (ai->attackerLastUsedMove == MOVE_REST)
                moveScore += 10;
            if (BattleRand(bsys) % 2 == 0)
                moveScore += 1;
        }
        else
        {
            moveScore -= NEVER_USE_MOVE_20;
        }
        break;
    case MOVE_EFFECT_KO_MON_THAT_DEFEATED_USER:
        if (ai->monCanOneShotPlayerWithAnyMove && ai->attackerMovesFirst)
        {
            moveScore += 6;
            if (BattleRand(bsys) % 5 == 0)
                moveScore += 1;
        }
        else if (ai->defenderMovesFirst)
        {
            moveScore += 5;
            if (BattleRand(bsys) % 2)
                moveScore += 1;
        }
        break;
    case MOVE_EFFECT_CHANGE_TO_WATER_TYPE:
        if (HasType(ctx, ai->defender, TYPE_WATER))
            moveScore -= NEVER_USE_MOVE_20;
        else
            moveScore += 6;
        break;
    case MOVE_EFFECT_ATK_DOWN:
    case MOVE_EFFECT_ATK_DOWN_2:
    case MOVE_EFFECT_ATK_DOWN_3:
    case MOVE_EFFECT_ATK_DEF_DOWN:
        if (ai->defenderHasAtleastOnePhysicalMove)
        {
            moveScore += 5;
            if (ai->attackerMon.hasMoldBreaker ||
                (ai->defenderMon.ability != ABILITY_CLEAR_BODY && 
                 ai->defenderMon.ability != ABILITY_WHITE_SMOKE && 
                 ai->defenderMon.ability != ABILITY_CONTRARY &&
                 ai->defenderMon.ability != ABILITY_HYPER_CUTTER))
            {
                moveScore += 1;
            }
            if (ai->defenderMovesFirst)
                moveScore -= 1;
        }
        else
        {
            moveScore -= NEVER_USE_MOVE_20;
		}
        break;
    case MOVE_EFFECT_SP_ATK_DOWN:
    case MOVE_EFFECT_SP_ATK_DOWN_2:
    case MOVE_EFFECT_SP_ATK_DOWN_3:
    case MOVE_EFFECT_SP_ATK_DOWN_2_OPPOSITE_GENDER:
    case MOVE_EFFECT_ATK_SP_ATK_DOWN:
        if (ai->defenderHasAtleastOneSpecialMove)
        {
            moveScore += 5;
            if (ai->attackerMon.hasMoldBreaker ||
                (ai->defenderMon.ability != ABILITY_CLEAR_BODY &&
                    ai->defenderMon.ability != ABILITY_WHITE_SMOKE &&
                    ai->defenderMon.ability != ABILITY_CONTRARY))
            {
                moveScore += 1;
            }
            if (ai->defenderMovesFirst)
                moveScore -= 1;
        }
        else
        {
            moveScore -= NEVER_USE_MOVE_20;
        }
        break;
    case MOVE_EFFECT_DEF_DOWN:
    case MOVE_EFFECT_DEF_DOWN_2:
    case MOVE_EFFECT_DEF_DOWN_3:
        if (2 * ai->maxDamageReceived < ai->attackerMon.hp && 2 * ai->attackerRolledMaxDamage < ai->defenderMon.hp)
        {
            if (ai->attackerMon.hasMoldBreaker ||
                (ai->defenderMon.ability != ABILITY_CLEAR_BODY &&
                    ai->defenderMon.ability != ABILITY_WHITE_SMOKE &&
                    ai->defenderMon.ability != ABILITY_BIG_PECKS &&
                    ai->defenderMon.ability != ABILITY_CONTRARY))
            {
                moveScore += 7;
            }
        }
        break;
    case MOVE_EFFECT_SP_DEF_DOWN:
    case MOVE_EFFECT_SP_DEF_DOWN_2:
    case MOVE_EFFECT_SP_DEF_DOWN_3:
        if (2 * ai->maxDamageReceived < ai->attackerMon.hp && 2 * ai->attackerRolledMaxDamage < ai->defenderMon.hp)
        {
            if (ai->attackerMon.hasMoldBreaker ||
                (ai->defenderMon.ability != ABILITY_CLEAR_BODY &&
                    ai->defenderMon.ability != ABILITY_WHITE_SMOKE &&
                    ai->defenderMon.ability != ABILITY_CONTRARY))
            {
                moveScore += 7;
            }
        }
        break;    
    case MOVE_EFFECT_EVA_DOWN:
    case MOVE_EFFECT_EVA_DOWN_2:
    case MOVE_EFFECT_EVA_DOWN_3:
    case MOVE_EFFECT_ACC_DOWN:
    case MOVE_EFFECT_ACC_DOWN_2:
    case MOVE_EFFECT_ACC_DOWN_3:
        moveScore += 5;
        if (ai->attackerMon.hasMoldBreaker ||
            (ai->defenderMon.ability != ABILITY_CLEAR_BODY &&
                ai->defenderMon.ability != ABILITY_WHITE_SMOKE &&
                ai->defenderMon.ability != ABILITY_KEEN_EYE &&
                ai->defenderMon.ability != ABILITY_CONTRARY))
        {
            moveScore += 1;
        }
        if (ai->defenderMovesFirst)
            moveScore -= 1;
        break;
    case MOVE_EFFECT_VENOM_DRENCH:
        if (ai->defenderMon.condition & STATUS_POISON_ALL)
        {
            if ((ai->defenderHasAtleastOnePhysicalMove && ctx->battlemon[ai->defender].states[STAT_ATTACK] >= 6) || 
                (ai->defenderHasAtleastOneSpecialMove && ctx->battlemon[ai->defender].states[STAT_SPATK] >= 6))
            {
                moveScore += 5;
                if (ai->attackerMon.hasMoldBreaker ||
                    (ai->defenderMon.ability != ABILITY_CLEAR_BODY &&
                     ai->defenderMon.ability != ABILITY_WHITE_SMOKE &&
                     ai->defenderMon.ability != ABILITY_CONTRARY))
                {
                    moveScore += 1;
                }
                if (ai->defenderMovesFirst)
                    moveScore -= 1;
            }
            else
            {
                moveScore -= NEVER_USE_MOVE_20;
			}
        }
        else
            moveScore -= NEVER_USE_MOVE_20;
        break;
    default:
        break;
    }

    return moveScore;
}


BOOL LONG_CALL shouldRecover(struct BattleSystem* bsys, u32 attacker, u32 attackerMoveEffect, struct AIContext* ai)
{
    u32 recoverAmountPercent = 50;
    switch (attackerMoveEffect)
    {
    case MOVE_EFFECT_RECOVER_HEALTH_AND_SLEEP:
        recoverAmountPercent = 100;
        break;
    case MOVE_EFFECT_HEAL_HALF_MORE_IN_SUN:
        recoverAmountPercent = 67;
        break;
    default:
        break;
    }
    u32 recoverAmountHP = recoverAmountPercent * ai->attackerMon.maxhp / 100;
    if ((recoverAmountHP + ai->attackerMon.hp) > ai->attackerMon.maxhp)
        recoverAmountHP = ai->attackerMon.maxhp - ai->attackerMon.hp; // prevent overheal

    if (ai->attackerMon.condition & STATUS_BAD_POISON)
        return FALSE;
    if (ai->maxDamageReceived > recoverAmountHP)
        return FALSE;
    if (ai->attackerMovesFirst)
    {
        if (ai->playerCanOneShotMonWithAnyMove && ai->maxDamageReceived < (ai->attackerMon.hp + recoverAmountHP)) //cannot kill after heal
            return TRUE;
        if (!ai->playerCanOneShotMonWithAnyMove)
        {
            if (ai->attackerMon.percenthp < 40)
                return TRUE;
            else if (ai->attackerMon.percenthp < 66)
                return FALSE + (BattleRand(bsys) % 2);
        }
    }
    else
    {
        if (ai->attackerMon.percenthp < 50)
            return TRUE;
        else if (ai->attackerMon.percenthp < 70 && (BattleRand(bsys) % 4) != 0) //75%
            return TRUE;
    }

    return FALSE;
}


int LONG_CALL RecoveryScoring(struct BattleSystem *bsys, u32 attacker, int i, struct AIContext* ai)
{
    int moveScore = 0;
    struct BattleStruct* ctx = bsys->sp;
	BOOL isHealingMove = TRUE;

    ai->attackerMove = ctx->battlemon[attacker].move[i];
    ai->attackerMoveEffect = ctx->moveTbl[ai->attackerMove].effect;

    BOOL aiShouldRecover = shouldRecover(bsys, attacker, ai->attackerMoveEffect, ai);
    if (ai->attackerMon.percenthp >= 85)
		moveScore -= 6;

    switch (ai->attackerMoveEffect)
    {
        //case MOVE_EFFECT_HIT_STRENGTH_SAP
        case MOVE_EFFECT_HEAL_HALF_REMOVE_FLYING_TYPE:
        case MOVE_EFFECT_RESTORE_HALF_HP:
            if (aiShouldRecover)
                moveScore += 7;
            else
				moveScore += 5;
            break;
        case MOVE_EFFECT_HEAL_HALF_MORE_IN_SUN:
            if (aiShouldRecover && ctx->field_condition & WEATHER_SUNNY_ANY)
                moveScore += 7;
			else if (shouldRecover(bsys, attacker, MOVE_EFFECT_RESTORE_HALF_HP, ai))
                moveScore += 7;
            else
                moveScore += 5;
            break;
        case MOVE_EFFECT_RECOVER_HEALTH_AND_SLEEP:
            if (aiShouldRecover)
            {
                if (ai->attackerMon.item == ITEM_CHESTO_BERRY || ai->attackerMon.item == ITEM_LUM_BERRY ||
					ai->attackerMon.ability == ABILITY_EARLY_BIRD || ai->attackerMon.ability == ABILITY_SHED_SKIN ||
                    BattlerKnowsMove(bsys, attacker, MOVE_SLEEP_TALK, ai) || BattlerKnowsMove(bsys, attacker, MOVE_SNORE, ai) ||
                    (ai->attackerMon.ability == ABILITY_HYDRATION && (ctx->field_condition & WEATHER_RAIN_ANY)))
                {
                    moveScore += 8;
                }
                else
                    moveScore += 7;
            }
            else
                moveScore += 5;
            break;
        default:
            isHealingMove = FALSE;
            break;
    }

    if (!isHealingMove)
        moveScore = 0;
    else if (ai->attackerMon.percenthp == 100)
        moveScore = 0 - NEVER_USE_MOVE_20;

    return moveScore;
}




/*-------------------------------Helper Functions--------------------------------*/

BOOL LONG_CALL battlerHasSoundBasedMove(struct BattleSystem* bsys, u32 battler, struct AIContext* ai)
{
    BOOL hasMove = FALSE;
    struct BattleStruct* ctx = bsys->sp;
    for (int i = 0; i < CLIENT_MAX; ++i)
    {
        u32 move = ctx->battlemon[battler].move[i];
        if (IsMoveSoundBased(move))
        {
            hasMove = TRUE;
            break;
        }
    }
    return hasMove;
}

BOOL LONG_CALL battlerKnowsFlinchingMove(struct BattleSystem* bsys, u32 battler, struct AIContext* ai UNUSED)
{
    BOOL knowsMove = 0;
    struct BattleStruct* ctx = bsys->sp;
    for (int i = 0; i < CLIENT_MAX; ++i)
    {
        u32 move = ctx->battlemon[battler].move[i];
        switch (move)
        {
            case MOVE_AIR_SLASH:
            case MOVE_ASTONISH:
            case MOVE_BITE:
            case MOVE_BONE_CLUB:
            case MOVE_DARK_PULSE:
            case MOVE_DOUBLE_IRON_BASH:
            case MOVE_DRAGON_RUSH:
            case MOVE_EXTRASENSORY:
            case MOVE_FIERY_WRATH:
            case MOVE_FIRE_FANG:
            case MOVE_FLOATY_FALL:
            case MOVE_HEADBUTT:
            case MOVE_HEART_STAMP:
            case MOVE_HYPER_FANG:
            case MOVE_ICE_FANG:
            case MOVE_ICICLE_CRASH:
            case MOVE_IRON_HEAD:
            case MOVE_LOW_KICK:
            case MOVE_MOUNTAIN_GALE:
            case MOVE_NEEDLE_ARM:
            case MOVE_ROCK_SLIDE:
            case MOVE_ROLLING_KICK:
            case MOVE_STEAMROLLER:
            case MOVE_STOMP:
            case MOVE_THUNDER_FANG:
            case MOVE_TRIPLE_ARROWS:
            case MOVE_TWISTER:
            case MOVE_WATERFALL:
            case MOVE_ZEN_HEADBUTT:
            case MOVE_ZING_ZAP:
                knowsMove = TRUE;
                break;
            default:
                break;
        }
    }
    return knowsMove;
}

BOOL LONG_CALL battlerKnowsThawingMove(struct BattleSystem* bsys, u32 battler, struct AIContext* ai UNUSED)
{
    BOOL knowsMove = 0;
    struct BattleStruct* ctx = bsys->sp;
    for (int i = 0; i < CLIENT_MAX; ++i)
    {
        u32 move = ctx->battlemon[battler].move[i];
        switch (move)
        {
        case MOVE_FLAME_WHEEL:
        case MOVE_SACRED_FIRE:
        case MOVE_FLARE_BLITZ:
		case MOVE_FUSION_FLARE:
        case MOVE_SCALD:
        case MOVE_STEAM_ERUPTION:
        case MOVE_BURN_UP:
        case MOVE_PYRO_BALL:
        case MOVE_SCORCHING_SANDS:
        case MOVE_MATCHA_GOTCHA:
            knowsMove = TRUE;
            break;
        default:
            break;
        }
    }
    return knowsMove;
}


/*Returns true if user has a particular move in their 4 move slots.*/
BOOL LONG_CALL BattlerKnowsMove(struct BattleSystem *bsys, u32 battler, u32 move, struct AIContext* ai UNUSED)
{
    BOOL knowsMove = FALSE;
    struct BattleStruct *ctx = bsys->sp;
    for(int i = 0; i < 4; i++)
    {
        u32 battler_move_check = ctx->battlemon[battler].move[i];
        if (battler_move_check == move)
        {
            knowsMove = TRUE;
            break;
        }
    }
    return knowsMove;
}



/*Returns true if a given list has a certain value in it.*/
BOOL LONG_CALL IsInList(u32 value, const u16 list[], u16 ListLength) 
{
    BOOL output = FALSE;
    for (u16 i = 0; i < ListLength; i++) {
        if (value == list[i]) {
            output = TRUE;
            break;
        }
    }
    return output;
}

/*Returns true if the given battler has a move in a list.*/
BOOL LONG_CALL BattlerKnowsMoveInList(struct BattleSystem *bsys, u32 battler, const u16 MoveList[], u16 listLength, struct AIContext* ai UNUSED) {
    struct BattleStruct *ctx = bsys->sp;
    BOOL knowsMove = FALSE;
    for (int i = 0; i < 4; i++) {
        for (int listIndex = 0; listIndex < listLength; listIndex++){
            if (ctx->battlemon[battler].move[i] == MoveList[listIndex]) {
                knowsMove = TRUE;
                break;
            }
        }
    }
    return knowsMove;
}

/*Returns true if the battler moves first. 
This function particularly accounts for 4 battlers, not just 2.*/
BOOL LONG_CALL BattlerMovesFirstDoubles(struct BattleSystem *bsys, struct BattleStruct *ctx, int mainBattler, int flag, struct AIContext* ai UNUSED){
    BOOL movesFirst = TRUE;
    for (int otherBattler = 0; otherBattler < 4; otherBattler++){
        if(ctx->battlemon[otherBattler].hp != 0 && mainBattler != otherBattler){
            if(CalcSpeed(bsys, ctx, otherBattler, mainBattler, flag) != 1){
                return FALSE;
            }
        }
    }
    return movesFirst;
}


int LONG_CALL BattlerPositiveStatChangesSum(struct BattleSystem* bsys, u32 battler, struct AIContext* ai UNUSED) {
    struct BattleStruct* ctx = bsys->sp;
    int statSum = 0;

    for (int stat = 0; stat < STAT_MAX; stat++) {
        if (ctx->battlemon[battler].states[stat] > 6) {
            statSum += ctx->battlemon[battler].states[stat];
        }
    }
    return statSum;
}


BOOL LONG_CALL canMoveKillBattler(u32 move, u32 damage, u32 defenderHP, u32 defenderMaxHP, BOOL attackerHasMoldBreaker, u32 defenderAbility, u32 defenderItem)
{
    BOOL canOneShot = FALSE;
    BOOL isMoveMultihit = IsMultiHitMove(move);
    if (damage >= defenderHP)
    {
        if (defenderHP == defenderMaxHP)
        {
			if ((attackerHasMoldBreaker || isMoveMultihit || defenderAbility != ABILITY_STURDY) && (defenderItem != ITEM_FOCUS_SASH || isMoveMultihit))
                canOneShot = TRUE;
        }
        else
            canOneShot = TRUE;
    }
	return canOneShot;
}

BOOL monDiesFromResidualDamage(struct BattleStruct* ctx, u32 attacker, u32 attackerCondition, BOOL isSeeded)
{
    // TODO order with heal
    //https://pokemondb.net/pokebase/409424/at-the-end-of-each-turn-what-goes-first-healing-or-damaging
    BOOL diesFromResidual = FALSE;
    u32 hp = ctx->battlemon[attacker].hp;
    u32 maxHp = ctx->battlemon[attacker].maxhp;
    u32 ability = ctx->battlemon[attacker].ability;
    u32 item = ctx->battlemon[attacker].item;
    u32 damageReceived = 0;

    if (isSeeded)
    {
        damageReceived += maxHp / 8;
    }

    if (attackerCondition & STATUS_BURN)
    {
        damageReceived += maxHp / 16;
    }
    else if (attackerCondition & STATUS_POISON)
    {
        damageReceived += maxHp / 8;
    }
    else if (attackerCondition & STATUS_BAD_POISON)
    {
        if ((attackerCondition & STATUS_POISON_COUNT) != STATUS_POISON_COUNT) {
            attackerCondition += 1 << 8;
        }
		int toxicDamageTicks = ((attackerCondition & STATUS_POISON_COUNT) >> 8); //TODO: check this
        damageReceived += (toxicDamageTicks * maxHp / 16);
    }
    if (ctx->field_condition & WEATHER_SANDSTORM_ANY)
    {
        if (!HasType(ctx, attacker, TYPE_ROCK) && !HasType(ctx, attacker, TYPE_GROUND) && !HasType(ctx, attacker, TYPE_STEEL)
            && ability != ABILITY_SAND_FORCE && ability != ABILITY_SAND_RUSH && ability != ABILITY_SAND_VEIL
            && ability != ABILITY_OVERCOAT && ability != ABILITY_MAGIC_GUARD
            && item != ITEM_SAFETY_GOGGLES)
        {
            damageReceived += maxHp / 16;
        }
    }
    else if (ctx->field_condition & WEATHER_HAIL_ANY)
    {
        if (!HasType(ctx, attacker, TYPE_ICE)
            && ability != ABILITY_ICE_BODY && ability != ABILITY_SNOW_CLOAK && ability != ABILITY_SLUSH_RUSH
            && ability != ABILITY_OVERCOAT && ability != ABILITY_MAGIC_GUARD
            && item != ITEM_SAFETY_GOGGLES)
        {
            damageReceived += maxHp / 16;
        }
    }
    else if (ctx->field_condition & WEATHER_SUNNY_ANY)
    {
        if (ability == ABILITY_DRY_SKIN || ability == ABILITY_SOLAR_POWER)
            damageReceived += maxHp / 8;
    }

    if (damageReceived > hp)
		diesFromResidual = TRUE;
	return diesFromResidual;
}


void LONG_CALL SetupStateVariables(struct BattleSystem *bsys, u32 attacker, u32 defender, struct AIContext* ai){
    struct BattleStruct *ctx = bsys->sp;
    u8 critical = 0;
    u8 speedCalc;
    u32 effectivenessFlag = 0;

    FillDamageStructFromBattleMon(bsys, ctx, &ai->attackerMon, attacker);
    FillDamageStructFromBattleMon(bsys, ctx, &ai->defenderMon, defender);
	ai->isDoubleBattle = FALSE;
    if (BattleTypeGet(bsys) & (BATTLE_TYPE_MULTI | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TAG))
        ai->isDoubleBattle = TRUE;
    //if (ai->isDoubleBattle && ctx->battlemon[BATTLER_ALLY(defender)].hp)
    //    FillDamageStructFromBattleMon(bsys, ctx, &ai->defenderAlly, BATTLER_ALLY(defender));

    ai->attacker = attacker;
    ai->defender = defender;
    ai->attackerSide = BATTLER_IS_ENEMY(ai->attacker);
    ai->defenderSide = BATTLER_IS_ENEMY(ai->defender);

    ai->attackerLastUsedMove = ctx->waza_no_old[ai->attacker];
    ai->defenderLastUsedMove = ctx->waza_no_old[ai->defender];
    ai->defenderLastUsedMoveEffect = ctx->moveTbl[ai->defenderLastUsedMove].effect;
    ai->defenderTurnsOnField = ctx->total_turn - ctx->battlemon[ai->defender].moveeffect.fakeOutCount;
    ai->attackerTurnsOnField = ctx->total_turn - ctx->battlemon[attacker].moveeffect.fakeOutCount;

    ai->defenderMovesFirst = 0;
    ai->attackerMovesFirst = 0;
    ai->isSpeedTie = 0;
    ai->maxDamageReceived = 0;
    ai->attackerRolledMaxDamage = 0;

	ai->defenderKnowsThawingMove = battlerKnowsThawingMove(bsys, ai->defender, ai);

    ai->isPartnerGrounded = FALSE;
    if (ai->isDoubleBattle && ctx->battlemon[BATTLER_ALLY(attacker)].hp)
    {
        ai->isPartnerGrounded = IsClientGrounded(ctx, BATTLER_ALLY(attacker));
    }
    

    speedCalc = CalcSpeed(bsys, ctx, defender, attacker, CALCSPEED_FLAG_NO_PRIORITY); //checks actual turn order with field state considered
    //evaluates to 0 if ai->defender > attacker (false)
    //and 1 if ai->defender < attacker (true)
    //if speed tie, then 2.
    
    if(speedCalc == 0){
        ai->defenderMovesFirst = 1;
    }
    else {
        ai->attackerMovesFirst = 1;
        if(speedCalc == 2)
            ai->isSpeedTie = 1;

    }
    ai->attackerMon.speed = ctx->effectiveSpeed[attacker];
    ai->defenderMon.speed = ctx->effectiveSpeed[defender];

#ifdef BATLLE_DEBUG_OUTPUT
    debug_printf("SpeedCalc %d, defMovesFirst %d, attMovesFirst %d \n", speedCalc, ai->defenderMovesFirst, ai->attackerMovesFirst);
#endif // BATLLE_DEBUG_OUTPUT
    
    
	ai->isDefenderIncapacitated = FALSE;
    if ((ai->defenderMon.condition & STATUS_SLEEP)
        || ((ai->defenderMon.condition & STATUS_FREEZE) && !ai->defenderKnowsThawingMove)
        || (ai->defenderMon.condition2 & STATUS2_RECHARGE)
        || ST_CheckIfInTruant(ctx, defender))
    {
        ai->isDefenderIncapacitated = TRUE;
    }

    BOOL isDefenderImmuneToAnyStatus = FALSE;
    if ((ai->defenderMon.condition & STATUS_ALL)
        || (ai->defenderMon.ability == ABILITY_GOOD_AS_GOLD) || (ai->defenderMon.ability == ABILITY_COMATOSE) || (ai->defenderMon.ability == ABILITY_PURIFYING_SALT) 
        || (ai->defenderMon.ability == ABILITY_SHIELDS_DOWN && ai->defenderMon.percenthp > 50)
        || (ai->defenderMon.ability == ABILITY_LEAF_GUARD && ctx->field_condition & WEATHER_SUNNY_ANY)
        || (ai->defenderMon.ability == ABILITY_HYDRATION && ctx->field_condition & WEATHER_RAIN_ANY) 
        || (ctx->side_condition[ai->defenderSide] & SIDE_STATUS_SAFEGUARD))
    {
        isDefenderImmuneToAnyStatus = TRUE;
    }

    ai->defenderImmuneToPoison = FALSE;
    if (isDefenderImmuneToAnyStatus
        || ai->defenderMon.ability == ABILITY_MAGIC_GUARD || ai->defenderMon.ability == ABILITY_IMMUNITY || ai->defenderMon.ability == ABILITY_POISON_HEAL
        || (ai->defenderMon.isGrounded && ctx->terrainOverlay.type == MISTY_TERRAIN) 
        || ((HasType(ctx, ai->defender, TYPE_POISON) || HasType(ctx, ai->defender, TYPE_STEEL)) && ai->attackerMon.ability != ABILITY_CORROSION))
    {
        ai->defenderImmuneToPoison = TRUE;
    }

	ai->defenderImmuneToParalysis = FALSE;
    if (isDefenderImmuneToAnyStatus ||
        ai->defenderMon.ability == ABILITY_LIMBER ||
        (ai->defenderMon.isGrounded && ctx->terrainOverlay.type == MISTY_TERRAIN) ||
        HasType(ctx, ai->defender, TYPE_ELECTRIC))
    {
        ai->defenderImmuneToParalysis = TRUE;
    }

    ai->defenderImmuneToBurn = FALSE;
    if (isDefenderImmuneToAnyStatus
        || ai->defenderMon.ability == ABILITY_WATER_VEIL || ai->defenderMon.ability == ABILITY_THERMAL_EXCHANGE
        || ai->defenderMon.ability == ABILITY_MAGIC_GUARD || ai->defenderMon.ability == ABILITY_WATER_BUBBLE 
        || (ai->defenderMon.isGrounded && ctx->terrainOverlay.type == MISTY_TERRAIN) 
        || HasType(ctx, ai->defender, TYPE_FIRE))
    {
        ai->defenderImmuneToBurn = TRUE;
    }

    ai->defenderImmuneToSleep = FALSE;
    if (isDefenderImmuneToAnyStatus
        || ai->defenderMon.ability == ABILITY_VITAL_SPIRIT || ai->defenderMon.ability == ABILITY_INSOMNIA
        || (ai->defenderMon.isGrounded && (ctx->terrainOverlay.type == MISTY_TERRAIN || ctx->terrainOverlay.type == ELECTRIC_TERRAIN)))
    {
        ai->defenderImmuneToSleep = TRUE;
    }

    ai->partySizeAttacker = Battle_GetClientPartySize(bsys, attacker);
    ai->livingMembersAttacker = 0;
    ai->partySizeDefender = Battle_GetClientPartySize(bsys, ai->defender);
    ai->livingMembersDefender = 0;

    for (int i = 0; i < ai->partySizeAttacker; i++)
    {
        struct PartyPokemon * current = Battle_GetClientPartyMon(bsys, attacker, i);
        if(!(GetMonData(current, MON_DATA_HP, 0) == 0 ||
        GetMonData(current, MON_DATA_SPECIES_OR_EGG, 0) == 0||
        GetMonData(current, MON_DATA_SPECIES_OR_EGG, 0) == SPECIES_EGG))
        {

            ai->livingMembersAttacker++;
        }
    }


    for (int i = 0; i < ai->partySizeDefender; i++)
    {
        struct PartyPokemon * current = Battle_GetClientPartyMon(bsys, ai->defender, i);
        if(!(GetMonData(current, MON_DATA_HP, 0) == 0 ||
        GetMonData(current, MON_DATA_SPECIES_OR_EGG, 0) == 0||
        GetMonData(current, MON_DATA_SPECIES_OR_EGG, 0) == SPECIES_EGG))
        {
            ai->livingMembersDefender++;
        }
    }
    
    ai->attackerMovesKnown = GetBattlerLearnedMoveCount(bsys, ctx, attacker);

    int highestDamageMoveIndex = 0;
    ai->playerCanOneShotMonWithAnyMove = FALSE;
    for (int k = 0; k < GetBattlerLearnedMoveCount(bsys, ctx, ai->defender); k++)
    {
        struct AI_damage damages = { 0 };
        u32 defenderMoveno = ctx->battlemon[defender].move[k];
        struct BattleMove defenderMove = ctx->moveTbl[defenderMoveno];

        if (defenderMove.split != SPLIT_STATUS && defenderMove.power)
        {
            damages.damageRoll = BattleAI_CalcDamage(bsys, ctx, defenderMoveno, ctx->side_condition[BATTLER_IS_ENEMY(defender)], ctx->field_condition, defenderMove.power, defenderMove.type, critical, defender, attacker, &damages, &ai->defenderMon, &ai->attackerMon);

            damages.damageRoll = BattleAI_AdjustUnusualMoveDamage(ai->defenderMon.level, ai->defenderMon.hp, ai->attackerMon.hp, damages.damageRoll, defenderMove.effect, ai->defenderMon.ability, ai->defenderMon.item);
            for (int u = 0; u < 16; u++)
            {
                damages.damageRange[u] = BattleAI_AdjustUnusualMoveDamage(ai->defenderMon.level, ai->defenderMon.hp, ai->attackerMon.hp, damages.damageRange[u], defenderMove.effect, ai->defenderMon.ability, ai->defenderMon.item);
            }

            BOOL playerCanOneShotAiMon = canAttackerOneShotDefender(damages.damageRoll, defenderMove.split, defenderMoveno, &ai->defenderMon, &ai->attackerMon);
            if (playerCanOneShotAiMon)
            {
                ai->playerCanOneShotMonWithAnyMove = TRUE;
                ai->playerCanOneShotMonWithMove[k] = TRUE;
            }

            if (damages.damageRoll > ai->maxDamageReceived)
            {
                ai->maxDamageReceived = damages.damageRoll;
                highestDamageMoveIndex = k;
            }
            if (defenderMove.split == SPLIT_SPECIAL)
                ai->defenderHasAtleastOneSpecialMove = 1;
            else
                ai->defenderHasAtleastOnePhysicalMove = 1;
            debug_printf("Receiving from move %d: %d is [%d-%d], roll %d > att.HP %d\n", k, defenderMoveno, damages.damageRange[0], damages.damageRange[15], damages.damageRoll, ai->attackerMon.hp);
        }
    }

#ifdef BATLLE_DEBUG_OUTPUT
    debug_printf("Overall Max damage received from %i:%i is %d > %d att.HP\n", highestDamageMoveIndex, ctx->battlemon[ai->defender].move[highestDamageMoveIndex], ai->maxDamageReceived, ai->attackerMon.hp);
#endif // BATLLE_DEBUG_OUTPUT
 

    ai->monCanOneShotPlayerWithAnyMove = FALSE;
    for (int j = 0; j < ai->attackerMovesKnown; j++)
    {
        struct AI_damage damages = { 0 };
        u32 attackerMoveno = ctx->battlemon[attacker].move[j];
        struct BattleMove attackerMove = ctx->moveTbl[attackerMoveno];
        if (attackerMove.split == SPLIT_STATUS)
        {
            u8 movetype = GetAdjustedMoveTypeBasics(ctx, attackerMoveno, ai->attackerMon.ability, attackerMove.type);
            ai->effectivenessOnPlayer[j] = BattleAI_GetTypeEffectiveness(bsys, ctx, movetype, &effectivenessFlag, &ai->attackerMon, &ai->defenderMon);
        }
        else if (attackerMove.power)
        {
            damages.damageRoll = BattleAI_CalcDamage(bsys, ctx, attackerMoveno, ctx->side_condition[BATTLER_IS_ENEMY(attacker)], ctx->field_condition, attackerMove.power, attackerMove.type, critical, attacker, defender, &damages, &ai->attackerMon, &ai->defenderMon);
			ai->effectivenessOnPlayer[j] = damages.moveEffectiveness;

            damages.damageRoll = BattleAI_AdjustUnusualMoveDamage(ai->attackerMon.level, ai->attackerMon.hp, ai->defenderMon.hp, damages.damageRoll, attackerMove.effect, ai->attackerMon.ability, ai->attackerMon.item);
            for (int u = 0; u < 16; u++)
            {
                damages.damageRange[u] = BattleAI_AdjustUnusualMoveDamage(ai->attackerMon.level, ai->attackerMon.hp, ai->defenderMon.hp, damages.damageRange[u], attackerMove.effect, ai->attackerMon.ability, ai->attackerMon.item);
            }

            BOOL aiMonCanOneshotPlayer = canAttackerOneShotDefender(damages.damageRoll, attackerMove.split, attackerMoveno, &ai->attackerMon, &ai->defenderMon);

            ai->attackerRolledMoveDamages[j] = damages.damageRoll;
            if (aiMonCanOneshotPlayer)
            {
                ai->monCanOneShotPlayerWithAnyMove = TRUE;
                ai->monCanOneShotPlayerWithMove[j] = TRUE;
                ai->attackerRolledMoveDamages[j] = ai->defenderMon.hp; // cap killing move's damage at defender HP, so that all killing moves are treated equally as "highest damage"
            }
            debug_printf("Dealing with move %d: %d is [%d-%d], roll %d > def.HP %d\n", j, attackerMoveno, damages.damageRange[0], damages.damageRange[15], damages.damageRoll, ai->defenderMon.hp);

            if (ai->attackerRolledMoveDamages[j] > ai->attackerRolledMaxDamage)
                ai->attackerRolledMaxDamage = ai->attackerRolledMoveDamages[j];
        }
    }
}