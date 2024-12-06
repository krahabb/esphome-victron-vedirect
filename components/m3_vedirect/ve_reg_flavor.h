/*
    'Flavor' framework:
    Our macro generated code uses conditional 'flavors' to detect which symbols to define in order
    to reduce (or enlarge) code generations to types/data which we are actually interested in.
    This set of macros leverages the 'grammar' defined in "ve_reg_macro.h" to achieve its behavior.

    This works by defining some symbols for the pre-processors which corresponds to
    VEDirect devices classes (more or less). In fact, some registers/text records are only relevant
    to specific device types so we'll use these conditionals to actually include/exclude symbols from
    our generated code.
    For example, if we're building for a 'classic' Blue Solar MPPT charger we should define 'VEDIRECT_FLAVOR_MPPT_BS'
    in our compilation environment in order to get all (and only) the structs needed for the BS MPPT charger.

    Some symbols relate to more general classes of devices so if using the aforementioned 'VEDIRECT_FLAVOR_MPPT_BS'
    this in turn will enable 'VEDIRECT_FLAVOR_MPPT' (symbols common to all of the MPPT chargers) and also
    'VEDIRECT_FLAVOR_CHG'.

    'VEDIRECT_FLAVOR_ALL' shortcut to define all flavors at once
    'VEDIRECT_FLAVOR_ANY' refers to registers which are available in every device (or the vast majority)

    This file is also being used to generate mock symbols for flavors ('generator.py').
*/

#pragma once
#include "ve_reg_macro.h"

#if 0
// debug/testing preset environment: remove on release
//#define VEDIRECT_FLAVOR_ALL
//#define VEDIRECT_FLAVOR_MPPT_RS
//#define VEDIRECT_FLAVOR_INV_PHNX
//#define VEDIRECT_FLAVOR_BMV
//#define VEDIRECT_FLAVOR_BMV71
#endif

#ifdef VEDIRECT_FLAVOR_ALL  // defines all flavors in one shot
#define VEDIRECT_FLAVOR_MPPT_BS
#define VEDIRECT_FLAVOR_MPPT_RS
#define VEDIRECT_FLAVOR_INV_PHNX
#define VEDIRECT_FLAVOR_CHG_PHNX
#define VEDIRECT_FLAVOR_BMV60
#define VEDIRECT_FLAVOR_BMV70
#define VEDIRECT_FLAVOR_BMV71
#endif

// Conditionals always active for characteristcs that belong to all device flavors
// or to a majority of different groups (so that they're likely in any device)
// #define VEDIRECT_FLAVORS_ANY  // This is pretty useless and is just for symmetry with DEF_ANY_Y
// #define DEF_ANY Y  // this must be fixed at Y in order to always activate features using the special flavor 'ANY'

// Inverter flavors
#ifdef VEDIRECT_FLAVOR_INV_PHNX  // Phoenix inverters
#define VEDIRECT_FLAVOR_INV
#define DEF_INV_PHNX Y
#else
#define DEF_INV_PHNX N
#endif

// Charger flavors
#ifdef VEDIRECT_FLAVOR_CHG_PHNX  // Phoenix chargers
#define VEDIRECT_FLAVOR_CHG
#define DEF_CHG_PHNX Y
#else
#define DEF_CHG_PHNX N
#endif

// MPPT charger flavors
#ifdef VEDIRECT_FLAVOR_MPPT_BS  // BlueSolar MPPT
#define VEDIRECT_FLAVOR_MPPT
#define DEF_MPPT_BS Y
#else
#define DEF_MPPT_BS N
#endif

#ifdef VEDIRECT_FLAVOR_MPPT_RS  // MPPT RS
#define VEDIRECT_FLAVOR_MPPT
#define DEF_MPPT_RS Y
#else
#define DEF_MPPT_RS N
#endif

// Battery Monitor flavors
#ifdef VEDIRECT_FLAVOR_BMV60
#define VEDIRECT_FLAVOR_BMV
#define DEF_BMV60 Y
#else
#define DEF_BMV60 N
#endif

#ifdef VEDIRECT_FLAVOR_BMV70
#define VEDIRECT_FLAVOR_BMV
#define DEF_BMV70 Y
#else
#define DEF_BMV70 N
#endif

#ifdef VEDIRECT_FLAVOR_BMV71
#define VEDIRECT_FLAVOR_BMV
#define DEF_BMV71 Y
#else
#define DEF_BMV71 N
#endif

// define some 'groups' (flavors common to more specific 'flavors' previously defined)
#ifdef VEDIRECT_FLAVOR_MPPT  // any MPPT charger flavor
#define VEDIRECT_FLAVOR_CHG
#define DEF_MPPT Y
#else
#define DEF_MPPT N
#endif

#ifdef VEDIRECT_FLAVOR_BMV  // any BMV flavor
#define VEDIRECT_FLAVOR_ANY
#define DEF_BMV Y
#else
#define DEF_BMV N
#endif

#ifdef VEDIRECT_FLAVOR_CHG  // any charger flavor
#define VEDIRECT_FLAVOR_ANY
#define DEF_CHG Y
#else
#define DEF_CHG N
#endif

#ifdef VEDIRECT_FLAVOR_INV  // any inverter flavor
#define VEDIRECT_FLAVOR_ANY
#define DEF_INV Y
#else
#define DEF_INV N
#endif

#ifdef VEDIRECT_FLAVOR_ANY  // any flavor
#define DEF_ANY Y
#else
#define DEF_ANY N
#endif