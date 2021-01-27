#ifndef FIRMWARE_H
#define FIRMWARE_H
#include <ion.h>
#include <escher.h>
#include <kandinsky.h>

#define FIRMWARE_NAME "Epsilon"
#define FIRMWARE_VERSION Ion::softwareVersion()
#define FIRMWARE_BRANCH "-"
#define FIRMWARE_BRANCH_COLOR KDColorWhite
#define FIRMWARE_MAIN_COLOR Palette::YellowDark

#define NEOFETCH_LOGO_WIDTH 11
#define NEOFETCH_LOGO_HEIGHT 12

static bool neofetch_logo[] = {
    false, false, false, true , true , true , true , true , false, false, false,
    false, false, true, true , true , true , true , true , true, false, false,
    false, true, true, true , false , false , false , true , true, true, false,
    false, true, true, false , false , false , false , false , true, true, false,
    false, true, true, true , false , false , false , false , false, false, false,
    false, false, true, true , true , true , false , false , false, false, false,
    false, false, true, true , true , true , false , false , false, false, false,
    false, true, true, true , false , false , false , false , false, false, false,
    false, true, true, false , false , false , false , false , true, true, false,
    false, true, true, true , false , false , false , true , true, true, false,
    false, false, true, true , true , true , true , true , true, false, false,
    false, false, false, true , true , true , true , true , false, false, false,
};

#endif