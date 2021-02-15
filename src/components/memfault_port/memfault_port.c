//! @file components/memfault_port/memfault_port.c

#include "memfault/components.h"

#include <stdio.h>

#include "memfault/esp8266_port/core.h"

sMfltHttpClientConfig g_mflt_http_client_config = {
  .api_key = "YOUR_PROJECT_API_KEY",
};

static char s_fw_version[32];

void memfault_platform_get_device_info(sMemfaultDeviceInfo *info) {
  if (s_fw_version[0] == 0) {
    // initialize version
    char build_id[7];
    memfault_build_id_get_string(build_id, sizeof(build_id));
    snprintf(s_fw_version, sizeof(s_fw_version), "1.0.0+%s", build_id);
  }

  // platform specific version information. For more details
  // see https://mflt.io/version-nomenclature
  *info = (sMemfaultDeviceInfo) {
    .device_serial = "DEMOSERIAL",
    .software_type = "app-fw",
    .software_version = s_fw_version,
    .hardware_version = "dvt",
  };
}

int memfault_platform_boot(void) {
  memfault_esp_port_boot();
  return 0;
}

