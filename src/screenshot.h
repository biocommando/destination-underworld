#pragma once

#define SCREENSHOT_ACT_INIT 1
#define SCREENSHOT_ACT_CAPTURE 2
#define SCREENSHOT_ACT_DUMP_TO_DISK 3
#define SCREENSHOT_ACT_DESTROY 4

/*
 * Keeps a circular array of screenshots. Use SCREENSHOT_ACT_CAPTURE to capture
 * the current contents on the screen to the array. Use SCREENSHOT_ACT_DUMP_TO_DISK to
 * save the array contents each to individual files.
 * 
 * Call with SCREENSHOT_ACT_INIT before calling other actions and SCREENSHOT_ACT_DESTROY in the end.
 */
void screenshot(int action);