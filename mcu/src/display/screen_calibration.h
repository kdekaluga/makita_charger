#pragma once

#include "../data.h"

namespace screen::calibration {

// Voltage and current settings x1000 for the calibration mode
var uint16_t g_voltageX1000;
var uint16_t g_currentX1000;

void Show();

} // namespace screen::calibration
