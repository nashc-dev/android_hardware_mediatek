/*
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vibrator-impl/Vibrator.h"

#include <android-base/logging.h>
#include <fstream>

namespace aidl {
namespace android {
namespace hardware {
namespace vibrator {

ndk::ScopedAStatus Vibrator::setNode(const std::string path, const int32_t value) {
    std::ofstream file(path);

    if (!file.is_open()) {
        LOG(ERROR) << "Failed to write " << value << " to " << path;
        return ndk::ScopedAStatus(AStatus_fromExceptionCode(EX_SERVICE_SPECIFIC));
    }

    file << value << std::endl;

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Vibrator::activate(const int32_t timeoutMs) {
    ndk::ScopedAStatus status;

    if (timeoutMs < 1)
        return off();

    status = setNode(kVibratorState, 1);
    if (!status.isOk())
        return status;

    status = setNode(kVibratorDuration, timeoutMs);
    if (!status.isOk())
        return status;

    status = setNode(kVibratorActivate, 1);
    if (!status.isOk())
        return status;

    return ndk::ScopedAStatus::ok();
}

}  // namespace vibrator
}  // namespace hardware
}  // namespace android
}  // namespace aidl
