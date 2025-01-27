#pragma once
/*
 * Gets a random number using a random number generator that is always initialized with the same state.
 * The idea is that all the playthroughs get the same dice rolls so (at least in theory) the
 * player can't blame bad luck at rng for not beating the levels. Another benefit is that it's
 * possible to play back pre-recorded "demo" files.
 */
int pr_get_random();

/*
 * Reset the random number generator to its initial state.
 */
void pr_reset_random();
