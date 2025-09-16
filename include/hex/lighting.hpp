#pragma once

#include <math.h>

namespace hex {
    const float RADIANCE_BOUNDS_SIZE          = 100.0f; // Size of bounds relative to the camera
    const float RADIANCE_PROBES_BASE_DISTANCE = 1.0f;   // Doubles for each level
    const int   RADIANCE_PROBES_BASE_RAYS     = 4;      // Doubles for each level
    const int   RADIANCE_PROBES_LEVELS        = 3;
    const int   PIXEL_COUNT = pow(RADIANCE_BOUNDS_SIZE /  RADIANCE_PROBES_BASE_DISTANCE     , 3) *     RADIANCE_PROBES_BASE_RAYS +
                              pow(RADIANCE_BOUNDS_SIZE / (RADIANCE_PROBES_BASE_DISTANCE * 2), 3) * pow(RADIANCE_PROBES_BASE_RAYS, 2) +
                              pow(RADIANCE_BOUNDS_SIZE / (RADIANCE_PROBES_BASE_DISTANCE * 4), 3) * pow(RADIANCE_PROBES_BASE_RAYS, 3);
    const int  METADATA_COUNT = pow(RADIANCE_BOUNDS_SIZE /  RADIANCE_PROBES_BASE_DISTANCE     , 3) +
                                pow(RADIANCE_BOUNDS_SIZE / (RADIANCE_PROBES_BASE_DISTANCE * 2), 3) +
                                pow(RADIANCE_BOUNDS_SIZE / (RADIANCE_PROBES_BASE_DISTANCE * 4), 3);
    const int RADIANCE_TEXTURE_SIZE = ceil(sqrt(PIXEL_COUNT));
    const int METADATA_TEXTURE_SIZE = ceil(sqrt(METADATA_COUNT));
}