#ifndef FIRMWARE_S_H
#define FIRMWARE_S_H
#include <sigmap.h>
#include <kandinsky.h>

#define FIRMWARE_NAME "Sigma"
#define FIRMWARE_VERSION SIGMA_VERSION
#define FIRMWARE_BRANCH SIGMA_BRANCH
#define FIRMWARE_BRANCH_COLOR SIGMA_BRANCH_COLOR
#define FIRMWARE_MAIN_COLOR Palette::SigmaPurple

#define NEOFETCH_LOGO_WIDTH 12
#define NEOFETCH_LOGO_HEIGHT 16

static bool neofetch_logo[] = {
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false,
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  false,  false,
    false, true,  true,  false, false, false, false, false, false, false, false, false,
    false, true,  true,  true, false, false, false, false, false, false, false, false,
    false, false,  true,  true, true, false, false, false, false, false, false, false,
    false, false,  false,  true, true, true, false, false, false, false, false, false,
    false, false,  false,  false, true, true, true, false, false, false, false, false,
    false, false,  false,  false, true, true, true, false, false, false, false, false,
    false, false,  false,  true, true, true, false, false, false, false, false, false,
    false, false,  true,  true, true, false, false, false, false, false, false, false,
    false, true,  true,  true, false, false, false, false, false, false, false, false,
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  false,  false,
    false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false,
};

#endif