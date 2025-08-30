.nds
.thumb

.include "armips/include/animscriptcmd.s"

.include "asm/include/abilities.inc"
.include "asm/include/items.inc"
.include "asm/include/species.inc"
.include "asm/include/moves.inc"

.create "build/move/move_anim/0_923", 0

//Vile Energy
a010_923:
    loadparticlefromspa 0, 543
    waitparticle

    //Start
    repeatse 1923, 0, 2, 12 // Shadow Ball Absorbing Energy Sound
    addparticle 0, 5, 0
    wait 15    
    addparticle 0, 3, 0
    wait 45
    callfunction 74, 1, 1, "NaN", "NaN", "NaN", "NaN", "NaN", "NaN", "NaN", "NaN", "NaN" // Background Colour Tint To Black
    addparticle 0, 4, 0
    playsepan 2108, -117 // Shadow Force Initial Disappearing Sound
    wait 15
    callfunction 36, 5, 1, 0, 1, 6, 264, "NaN", "NaN", "NaN", "NaN", "NaN" // Shakes The Target Pokemon
    callfunction 34, 6, 8, 0, 1, 8296, 14, 0, "NaN", "NaN", "NaN", "NaN" // Shades The Pokemon
    wait 30
    callfunction 74, 1, 0, "NaN", "NaN", "NaN", "NaN", "NaN", "NaN", "NaN", "NaN", "NaN" // Remove Background Colour Tint
    waitstate
    waitparticle

    unloadparticle 0
    waitstate
    end


.close
