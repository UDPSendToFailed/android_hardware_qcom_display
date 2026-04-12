/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation. nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DISPLAY_CONFIG_V1_H__
#define __DISPLAY_CONFIG_V1_H__

#include <vendor/display/config/1.0/IDisplayConfig.h>
#include <hidl/HidlSupport.h>
#include <config/device_interface.h>

namespace sdm {

using ::android::hardware::Return;
using ::android::hardware::hidl_vec;

/*
 * Adapter that exposes the V1_0::IDisplayConfig HIDL interface and delegates
 * to the same DisplayConfig::ConfigInterface backend that the V2_0 opcode-based
 * service uses.  This allows legacy vendor blobs (e.g. liboemcrypto) that link
 * vendor.display.config@1.0 to resolve the service at runtime.
 */
class DisplayConfigV1 : public vendor::display::config::V1_0::IDisplayConfig {
 public:
  static int RegisterService(DisplayConfig::ClientContext *ctx);

 private:
  DisplayConfigV1() = default;

  DisplayConfig::ConfigInterface *intf_ = nullptr;
  DisplayConfig::ClientContext *ctx_ = nullptr;
  std::shared_ptr<DisplayConfig::ConfigCallback> callback_;

  // Return<void> with callback — methods that return multiple values
  Return<void> isDisplayConnected(DisplayType dpy, isDisplayConnected_cb _hidl_cb) override;
  Return<void> getConfigCount(DisplayType dpy, getConfigCount_cb _hidl_cb) override;
  Return<void> getActiveConfig(DisplayType dpy, getActiveConfig_cb _hidl_cb) override;
  Return<void> getDisplayAttributes(uint32_t configIndex, DisplayType dpy,
                                    getDisplayAttributes_cb _hidl_cb) override;
  Return<void> getPanelBrightness(getPanelBrightness_cb _hidl_cb) override;
  Return<void> getHDRCapabilities(DisplayType dpy, getHDRCapabilities_cb _hidl_cb) override;
  Return<void> displayBWTransactionPending(displayBWTransactionPending_cb _hidl_cb) override;

  // Return<int32_t> — methods that return only an error code
  Return<int32_t> setSecondayDisplayStatus(DisplayType dpy,
                                           DisplayExternalStatus status) override;
  Return<int32_t> configureDynRefeshRate(DisplayDynRefreshRateOp op,
                                         uint32_t refreshRate) override;
  Return<int32_t> setActiveConfig(DisplayType dpy, uint32_t config) override;
  Return<int32_t> setPanelBrightness(uint32_t level) override;
  Return<int32_t> minHdcpEncryptionLevelChanged(DisplayType dpy,
                                                uint32_t min_enc_level) override;
  Return<int32_t> refreshScreen() override;
  Return<int32_t> controlPartialUpdate(DisplayType dpy, bool enable) override;
  Return<int32_t> toggleScreenUpdate(bool on) override;
  Return<int32_t> setIdleTimeout(uint32_t value) override;
  Return<int32_t> setCameraLaunchStatus(uint32_t on) override;

  // Enum conversion helpers
  static DisplayConfig::DisplayType ToDispType(DisplayType dpy);
  static DisplayConfig::ExternalStatus ToExtStatus(DisplayExternalStatus status);
  static DisplayConfig::DynRefreshRateOp ToDynOp(DisplayDynRefreshRateOp op);
  static DisplayPortType FromPortType(DisplayConfig::DisplayPortType type);
};

}  // namespace sdm

#endif  // __DISPLAY_CONFIG_V1_H__
