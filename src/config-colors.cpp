#ifndef MOCHIGOTCHI_CONFIG_COLORS_H
#define MOCHIGOTCHI_CONFIG_COLORS_H

#include "WString.h"

/**
 * This Values should stay in an CPP-File, otherwise a lot of flash is used!
 * (even with inline... )
 * ca. 85 kB!!
 *
 */

// h:   3.233.117
// cpp: 3.146.225                           -85 kB

// loglevelstring[] h->cpp: 3.126.113       -20 kB

// config-logging: h->cpp:  2.795.477       -330 kB (!!)

// https://i.stack.imgur.com/9UVnC.png
const String ANSICOLOR_RESET =    "\033[0m";
const String ANSICOLOR_RED =      "[1;31m";
const String ANSICOLOR_GREEN =    "[1;32m";
const String ANSICOLOR_YELLOW =   "[1;33m";
const String ANSICOLOR_WHITE =    "\033[1;37m";
const String ANSICOLOR_CYAN =    "\033[1;36m";
const String ANSICOLOR_BLUE =     "\033[1;34m";
const String ANSICOLOR_BLACK =     "\033[1;30m";


const String ANSICOLOR_BRIGHT_MAGENTA =   "[1;95m";
const String ANSICOLOR_BRIGHT_YELLOW =   "[1;93m";
const String ANSICOLOR_BRIGHT_BLACK =    "[1;90m";
const String ANSICOLOR_BRIGHT_RED =    "[1;91m";

const String ANSICOLOR_BG_YELLOW_BRIGHT =   "[1;103m";
const String ANSICOLOR_BG_GREEN =   "[1;102m";
const String ANSICOLOR_BG_RED =   "[1;101m";
const String ANSICOLOR_BG_CYAN =   "[1;46m";


#endif //MOCHIGOTCHI_CONFIG_COLORS_H
