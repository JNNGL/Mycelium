#pragma once

#include <string>

enum ChatMode {
    ENABLED = 0,
    COMMANDS_ONLY = 1,
    HIDDEN = 2,
};

enum MainHand {
    LEFT = 0,
    RIGHT = 1
};

union SkinParts {
    struct {
        unsigned char capeEnabled : 1;
        unsigned char jacketEnabled : 1;
        unsigned char leftSleeveEnabled : 1;
        unsigned char rightSleeveEnabled : 1;
        unsigned char leftPantsLegEnabled : 1;
        unsigned char rightPantsLegEnabled : 1;
        unsigned char hatEnabled : 1;
        unsigned char unused : 1;
    };

    unsigned char value;
};

struct ClientSettings {
    std::string locale;
    unsigned char viewDistance;
    ChatMode chatMode;
    bool chatColors;
    SkinParts displayedSkinParts;
    MainHand mainHand;
    bool enableTextFiltering;
    bool allowServerListings;
};