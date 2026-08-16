#include "constants/battle_constants.h"
.include "battle_commands.inc"

.data

_000:
    // Call the custom E4 command to set the tailwind counter (assigns ninety nine turns via the C injection)
    SetTailwindCounter BATTLER_CATEGORY_ENEMY

    // Set up the Tailwind move ID fo the animation and message engine
    UpdateVar OPCODE_SET, BSCRIPT_VAR_MSG_MOVE_TEMP, MOVE_TAILWIND

    // Play the standard Tailwind move animation on the enemy side
    PlayMoveAnimation BATTLER_CATEGORY_MSG_TEMP
    Wait 

    // Reset battle status variables so the nex moves act normally
    UpdateVar OPCODE_SET, BSCRIPT_VAR_MOVE_EFFECT_CHANCE, 0
    UpdateVar OPCODE_FLAG_OFF, BSCRIPT_VAR_BATTLE_STATUS, BATTLE_STATUS_MOVE_ANIMATIONS_OFF
    
    // Print the Tailwind blew from behind the enemy team! message
    // (Using message index targeting the ENEMY side category)
    PrintMessage 1230, TAG_NONE_SIDE, BATTLER_CATEGORY_ENEMY
    Wait 
    WaitButtonABTime 30
    End
