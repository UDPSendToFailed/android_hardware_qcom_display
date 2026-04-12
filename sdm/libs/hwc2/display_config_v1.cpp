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

#include <log/log.h>
#include "display_config_v1.h"

namespace sdm {

using DispType = DisplayConfig::DisplayType;

class DisplayConfigV1Callback : public DisplayConfig::ConfigCallback {
 public:
  ~DisplayConfigV1Callback() override = default;
};

// -- Enum conversion helpers ------------------------------------------------

DispType DisplayConfigV1::ToDispType(DisplayType dpy) {
  switch (dpy) {
    case DisplayType::DISPLAY_PRIMARY:  return DispType::kPrimary;
    case DisplayType::DISPLAY_EXTERNAL: return DispType::kExternal;
    case DisplayType::DISPLAY_VIRTUAL:  return DispType::kVirtual;
    default:                            return DispType::kInvalid;
  }
}

DisplayConfig::ExternalStatus DisplayConfigV1::ToExtStatus(DisplayExternalStatus status) {
  switch (status) {
    case DisplayExternalStatus::EXTERNAL_OFFLINE: return DisplayConfig::ExternalStatus::kOffline;
    case DisplayExternalStatus::EXTERNAL_ONLINE:  return DisplayConfig::ExternalStatus::kOnline;
    case DisplayExternalStatus::EXTERNAL_PAUSE:   return DisplayConfig::ExternalStatus::kPause;
    case DisplayExternalStatus::EXTERNAL_RESUME:  return DisplayConfig::ExternalStatus::kResume;
    default:                                      return DisplayConfig::ExternalStatus::kInvalid;
  }
}

DisplayConfig::DynRefreshRateOp DisplayConfigV1::ToDynOp(DisplayDynRefreshRateOp op) {
  switch (op) {
    case DisplayDynRefreshRateOp::DISABLE_METADATA_DYN_REFRESH_RATE:
      return DisplayConfig::DynRefreshRateOp::kDisableMetadata;
    case DisplayDynRefreshRateOp::ENABLE_METADATA_DYN_REFRESH_RATE:
      return DisplayConfig::DynRefreshRateOp::kEnableMetadata;
    case DisplayDynRefreshRateOp::SET_BINDER_DYN_REFRESH_RATE:
      return DisplayConfig::DynRefreshRateOp::kSetBinder;
    default:
      return DisplayConfig::DynRefreshRateOp::kInvalid;
  }
}

DisplayConfigV1::DisplayPortType DisplayConfigV1::FromPortType(
    DisplayConfig::DisplayPortType type) {
  switch (type) {
    case DisplayConfig::DisplayPortType::kDefault:    return DisplayPortType::DISPLAY_PORT_DEFAULT;
    case DisplayConfig::DisplayPortType::kDsi:        return DisplayPortType::DISPLAY_PORT_DSI;
    case DisplayConfig::DisplayPortType::kDtv:        return DisplayPortType::DISPLAY_PORT_DTV;
    case DisplayConfig::DisplayPortType::kWriteback:  return DisplayPortType::DISPLAY_PORT_WRITEBACK;
    case DisplayConfig::DisplayPortType::kLvds:       return DisplayPortType::DISPLAY_PORT_LVDS;
    case DisplayConfig::DisplayPortType::kEdp:        return DisplayPortType::DISPLAY_PORT_EDP;
    case DisplayConfig::DisplayPortType::kDp:         return DisplayPortType::DISPLAY_PORT_DP;
    default:                                          return DisplayPortType::INVALID;
  }
}

// -- Service registration ---------------------------------------------------

int DisplayConfigV1::RegisterService(DisplayConfig::ClientContext *ctx) {
  DisplayConfigV1 *svc = new DisplayConfigV1();
  if (!svc) {
    return -ENOMEM;
  }

  svc->ctx_ = ctx;
  svc->callback_ = std::make_shared<DisplayConfigV1Callback>();

  int error = ctx->RegisterClientContext(svc->callback_, &svc->intf_);
  if (error || !svc->intf_) {
    ALOGE("DisplayConfigV1: failed to obtain ConfigInterface (%d)", error);
    delete svc;
    return error ? error : -EINVAL;
  }

  android::status_t status =
      svc->vendor::display::config::V1_0::IDisplayConfig::registerAsService();
  if (status != android::OK) {
    ALOGE("DisplayConfigV1: registerAsService failed (%d)", status);
    ctx->UnRegisterClientContext(svc->intf_);
    delete svc;
    return -EINVAL;
  }

  ALOGI("DisplayConfigV1: registered vendor.display.config@1.0::IDisplayConfig/default");
  return 0;
}

// -- Return<void> methods (multiple return values via callback) --------------

Return<void> DisplayConfigV1::isDisplayConnected(DisplayType dpy,
                                                  isDisplayConnected_cb _hidl_cb) {
  bool connected = false;
  int32_t error = intf_->IsDisplayConnected(ToDispType(dpy), &connected);
  _hidl_cb(error, connected);
  return android::hardware::Void();
}

Return<void> DisplayConfigV1::getConfigCount(DisplayType dpy, getConfigCount_cb _hidl_cb) {
  uint32_t count = 0;
  int32_t error = intf_->GetConfigCount(ToDispType(dpy), &count);
  _hidl_cb(error, count);
  return android::hardware::Void();
}

Return<void> DisplayConfigV1::getActiveConfig(DisplayType dpy, getActiveConfig_cb _hidl_cb) {
  uint32_t config = 0;
  int32_t error = intf_->GetActiveConfig(ToDispType(dpy), &config);
  _hidl_cb(error, config);
  return android::hardware::Void();
}

Return<void> DisplayConfigV1::getDisplayAttributes(uint32_t configIndex, DisplayType dpy,
                                                    getDisplayAttributes_cb _hidl_cb) {
  DisplayConfig::Attributes attr = {};
  int32_t error = intf_->GetDisplayAttributes(configIndex, ToDispType(dpy), &attr);

  DisplayAttributes hidl_attr = {};
  hidl_attr.vsyncPeriod = attr.vsync_period;
  hidl_attr.xRes = attr.x_res;
  hidl_attr.yRes = attr.y_res;
  hidl_attr.xDpi = attr.x_dpi;
  hidl_attr.yDpi = attr.y_dpi;
  hidl_attr.panelType = FromPortType(attr.panel_type);
  hidl_attr.isYuv = attr.is_yuv;
  _hidl_cb(error, hidl_attr);
  return android::hardware::Void();
}

Return<void> DisplayConfigV1::getPanelBrightness(getPanelBrightness_cb _hidl_cb) {
  uint32_t level = 0;
  int32_t error = intf_->GetPanelBrightness(&level);
  _hidl_cb(error, level);
  return android::hardware::Void();
}

Return<void> DisplayConfigV1::getHDRCapabilities(DisplayType dpy,
                                                  getHDRCapabilities_cb _hidl_cb) {
  DisplayConfig::HDRCapsParams caps = {};
  int32_t error = intf_->GetHDRCapabilities(ToDispType(dpy), &caps);

  DisplayHDRCapabilities hidl_caps = {};
  hidl_caps.supportedHdrTypes.resize(caps.supported_hdr_types.size());
  for (size_t i = 0; i < caps.supported_hdr_types.size(); i++) {
    hidl_caps.supportedHdrTypes[i] = caps.supported_hdr_types[i];
  }
  hidl_caps.maxLuminance = caps.max_luminance;
  hidl_caps.maxAvgLuminance = caps.max_avg_luminance;
  hidl_caps.minLuminance = caps.min_luminance;
  _hidl_cb(error, hidl_caps);
  return android::hardware::Void();
}

Return<void> DisplayConfigV1::displayBWTransactionPending(
    displayBWTransactionPending_cb _hidl_cb) {
  bool status = false;
  int32_t error = intf_->DisplayBWTransactionPending(&status);
  _hidl_cb(error, status);
  return android::hardware::Void();
}

// -- Return<int32_t> methods (single error return) --------------------------

Return<int32_t> DisplayConfigV1::setSecondayDisplayStatus(DisplayType dpy,
                                                           DisplayExternalStatus status) {
  return intf_->SetDisplayStatus(ToDispType(dpy), ToExtStatus(status));
}

Return<int32_t> DisplayConfigV1::configureDynRefeshRate(DisplayDynRefreshRateOp op,
                                                         uint32_t refreshRate) {
  return intf_->ConfigureDynRefreshRate(ToDynOp(op), refreshRate);
}

Return<int32_t> DisplayConfigV1::setActiveConfig(DisplayType dpy, uint32_t config) {
  return intf_->SetActiveConfig(ToDispType(dpy), config);
}

Return<int32_t> DisplayConfigV1::setPanelBrightness(uint32_t level) {
  return intf_->SetPanelBrightness(level);
}

Return<int32_t> DisplayConfigV1::minHdcpEncryptionLevelChanged(DisplayType dpy,
                                                                uint32_t min_enc_level) {
  return intf_->MinHdcpEncryptionLevelChanged(ToDispType(dpy), min_enc_level);
}

Return<int32_t> DisplayConfigV1::refreshScreen() {
  return intf_->RefreshScreen();
}

Return<int32_t> DisplayConfigV1::controlPartialUpdate(DisplayType dpy, bool enable) {
  return intf_->ControlPartialUpdate(ToDispType(dpy), enable);
}

Return<int32_t> DisplayConfigV1::toggleScreenUpdate(bool on) {
  return intf_->ToggleScreenUpdate(on);
}

Return<int32_t> DisplayConfigV1::setIdleTimeout(uint32_t value) {
  return intf_->SetIdleTimeout(value);
}

Return<int32_t> DisplayConfigV1::setCameraLaunchStatus(uint32_t on) {
  return intf_->SetCameraLaunchStatus(on);
}

}  // namespace sdm
