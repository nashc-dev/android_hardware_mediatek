/*
 * Copyright (C) 2020 The Android Open Source Project
 * Copyright (C) 2023 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <dlfcn.h>

#include "Power.h"

#include <android-base/logging.h>

#ifdef TAP_TO_WAKE_NODE
#include <android-base/file.h>
#endif

#define POWERHAL_LIB_NAME "libpowerhal.so"
#define LIBPOWERHAL_INIT "libpowerhal_Init"
#define LIBPOWERHAL_CUSLOCKHINT "libpowerhal_CusLockHint"
#define LIBPOWERHAL_LOCKREL "libpowerhal_LockRel"
#define LIBPOWERHAL_USERSCNDISABLEALL "libpowerhal_UserScnDisableAll"
#define LIBPOWERHAL_USERSCNRESTOREALL "libpowerhal_UserScnRestoreAll"

namespace aidl {
namespace android {
namespace hardware {
namespace power {
namespace impl {
namespace mediatek {

const std::vector<Boost> BOOST_RANGE{ndk::enum_range<Boost>().begin(),
                                     ndk::enum_range<Boost>().end()};
const std::vector<Mode> MODE_RANGE{ndk::enum_range<Mode>().begin(), ndk::enum_range<Mode>().end()};

Power::Power()
{
    powerLibHandle = dlopen(POWERHAL_LIB_NAME, RTLD_LAZY);
    if (!powerLibHandle) {
        LOG(ERROR) << "Could not dlopen " << POWERHAL_LIB_NAME;
        abort();
    }

    libpowerhal_Init =
        reinterpret_cast<libpowerhal_Init_handle>(dlsym(powerLibHandle, LIBPOWERHAL_INIT));

    if (!libpowerhal_Init) {
        LOG(ERROR) << "Could not locate symbol " << LIBPOWERHAL_INIT;
        abort();
    }

    libpowerhal_CusLockHint =
        reinterpret_cast<libpowerhal_CusLockHint_handle>(dlsym(powerLibHandle, LIBPOWERHAL_CUSLOCKHINT));

    if (!libpowerhal_CusLockHint) {
        LOG(ERROR) << "Could not locate symbol " << LIBPOWERHAL_CUSLOCKHINT;
        abort();
    }

    libpowerhal_LockRel =
        reinterpret_cast<libpowerhal_LockRel_handle>(dlsym(powerLibHandle, LIBPOWERHAL_LOCKREL));

    if (!libpowerhal_LockRel) {
        LOG(ERROR) << "Could not locate symbol " << LIBPOWERHAL_LOCKREL;
        abort();
    }

    libpowerhal_UserScnDisableAll =
         reinterpret_cast<libpowerhal_UserScnDisableAll_handle>(dlsym(powerLibHandle, LIBPOWERHAL_USERSCNDISABLEALL));

    if (!libpowerhal_UserScnDisableAll) {
        LOG(ERROR) << "Could not locate symbol " << LIBPOWERHAL_USERSCNDISABLEALL;
        abort();
    }

    libpowerhal_UserScnRestoreAll =
        reinterpret_cast<libpowerhal_UserScnRestoreAll_handle>(dlsym(powerLibHandle, LIBPOWERHAL_USERSCNRESTOREALL));

    if (!libpowerhal_UserScnRestoreAll) {
        LOG(ERROR) << "Could not locate symbol " << LIBPOWERHAL_USERSCNRESTOREALL;
        abort();
    }

    lowPowerEnabled = 0;
    libpowerhal_Init(1);
}

Power::~Power()
{

}

ndk::ScopedAStatus Power::setMode(Mode type, bool enabled)
{
    LOG(VERBOSE) << "Power setMode: " << static_cast<int32_t>(type) << " to: " << enabled;

    switch (type) {
#ifdef TAP_TO_WAKE_NODE
        case Mode::DOUBLE_TAP_TO_WAKE:
        {
            ::android::base::WriteStringToFile(enabled ? "1" : "0", TAP_TO_WAKE_NODE, true);
            break;
        }
#endif
        case Mode::LAUNCH:
        {
            if (perfHandle != 0) {
                libpowerhal_LockRel(perfHandle);
                perfHandle = 0;
            }

            if (lowPowerEnabled)
                break;

            if (enabled)
                perfHandle = libpowerhal_CusLockHint(11, 30000, getpid());

            break;
        }
        case Mode::INTERACTIVE:
        {
            if (enabled)
                /* Device now in interactive state,
                   restore all currently held hints. */
                libpowerhal_UserScnRestoreAll();
            else
                /* Device entering non interactive state,
                   disable all hints to save power. */
                libpowerhal_UserScnDisableAll();
            break;
        }
        case Mode::LOW_POWER:
            lowPowerEnabled = enabled;
            break;
        default:
            break;
    }
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isModeSupported(Mode type, bool* _aidl_return)
{
    LOG(INFO) << "Power isModeSupported: " << static_cast<int32_t>(type);
    *_aidl_return = type >= MODE_RANGE.front() && type <= MODE_RANGE.back();

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::setBoost(Boost type, int32_t durationMs)
{
    if (lowPowerEnabled)
        return ndk::ScopedAStatus::ok();

    int32_t intType = static_cast<int32_t>(type);

    if (type == Boost::INTERACTION && durationMs < 1)
        durationMs = 80;

    LOG(VERBOSE) << "Power setBoost: " << intType
                 << ", duration: " << durationMs;

    libpowerhal_CusLockHint(intType, durationMs, getpid());
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::isBoostSupported(Boost type, bool* _aidl_return)
{
    LOG(INFO) << "Power isBoostSupported: " << static_cast<int32_t>(type);
    *_aidl_return = type >= BOOST_RANGE.front() && type <= BOOST_RANGE.back();

    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Power::createHintSession(int32_t, int32_t, const std::vector<int32_t>&, int64_t,
                                            std::shared_ptr<IPowerHintSession>* _aidl_return)
{
    *_aidl_return = nullptr;
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

ndk::ScopedAStatus Power::getHintSessionPreferredRate(int64_t* outNanoseconds)
{
    *outNanoseconds = -1;
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}

}  // namespace mediatek
}  // namespace impl
}  // namespace power
}  // namespace hardware
}  // namespace android
}  // namespace aidl
