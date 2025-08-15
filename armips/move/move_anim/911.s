.nds
.thumb

.include "armips/include/animscriptcmd.s"

.include "asm/include/abilities.inc"
.include "asm/include/items.inc"
.include "asm/include/species.inc"
.include "asm/include/moves.inc"

.create "build/move/move_anim/0_911", 0

a010_911:
	// Burning Bulwark Animation Sequence
	loadparticlefromspa 0, 541   // Load custom particle bank from 541.spa

	// Flame charge-up one
	addparticle 0, 2, 2          // Emitter 2: Midground flame surge going up
	wait 6                       // Test Flames Going Up

	// Flame charge-up two
	addparticle 0, 3, 3          // Emitter 3: Foreground steam puff
	wait 8                       // Test Flames Going Up

	// Flame charge-up three     // Emitter 4: Background flame surge going up
	addparticle 0, 4, 1			 // Test Flames Going Up
	wait 8

	// Shield protect
	addparticle 0, 1, 1          // Emitter 2: Background shield pulse
	waitparticle                 // Wait for all emitters to finish

	// Optional effects
	playsound 45                 // Flame burst or shield impact SFX
	quake 2, 4                   // Mild screen shake for impact

	// Cleanup
	unloadparticle 0
	waitstate
	end


.close