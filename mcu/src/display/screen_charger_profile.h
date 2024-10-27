#pragma once

#include "../charger_profile.h"
#include "screen_settings.h"

namespace screen::charger_profile {

var bool g_bCurrentProfile;

void Show(bool bCurrentProfile);

} // namespace screen::charger_profile
