#include "shop_data.h"

static const ShopEntry bandit_forest_first_village[] = {
    { ITEM_HERB, 25 },
    { ITEM_TONIC, 50 },
    { ITEM_ETHER, 80 },
    { ITEM_LEATHER_ARMOR, 150 },
    { ITEM_IRON_SWORD, 280 },
    { ITEM_DRAGON_SCALE, 8000 }
};

const ShopDef shop_defs[] = {
    { "Shady Man in the Woods", bandit_forest_first_village, sizeof(bandit_forest_first_village) / sizeof(bandit_forest_first_village[0]) },
};