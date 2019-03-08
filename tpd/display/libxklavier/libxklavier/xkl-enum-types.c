


#include <libxklavier/xklavier.h>

/* enumerations from "xkl_engine.h" */
GType
xkl_engine_state_change_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GROUP_CHANGED, "GROUP_CHANGED", "group-changed" },
      { INDICATORS_CHANGED, "INDICATORS_CHANGED", "indicators-changed" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("XklEngineStateChange", values);
  }
  return etype;
}
GType
xkl_engine_features_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GFlagsValue values[] = {
      { XKLF_CAN_TOGGLE_INDICATORS, "XKLF_CAN_TOGGLE_INDICATORS", "can-toggle-indicators" },
      { XKLF_CAN_OUTPUT_CONFIG_AS_ASCII, "XKLF_CAN_OUTPUT_CONFIG_AS_ASCII", "can-output-config-as-ascii" },
      { XKLF_CAN_OUTPUT_CONFIG_AS_BINARY, "XKLF_CAN_OUTPUT_CONFIG_AS_BINARY", "can-output-config-as-binary" },
      { XKLF_MULTIPLE_LAYOUTS_SUPPORTED, "XKLF_MULTIPLE_LAYOUTS_SUPPORTED", "multiple-layouts-supported" },
      { XKLF_REQUIRES_MANUAL_LAYOUT_MANAGEMENT, "XKLF_REQUIRES_MANUAL_LAYOUT_MANAGEMENT", "requires-manual-layout-management" },
      { XKLF_DEVICE_DISCOVERY, "XKLF_DEVICE_DISCOVERY", "device-discovery" },
      { 0, NULL, NULL }
    };
    etype = g_flags_register_static ("XklEngineFeatures", values);
  }
  return etype;
}
GType
xkl_engine_listen_modes_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { XKLL_MANAGE_WINDOW_STATES, "XKLL_MANAGE_WINDOW_STATES", "manage-window-states" },
      { XKLL_TRACK_KEYBOARD_STATE, "XKLL_TRACK_KEYBOARD_STATE", "track-keyboard-state" },
      { XKLL_MANAGE_LAYOUTS, "XKLL_MANAGE_LAYOUTS", "manage-layouts" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("XklEngineListenModes", values);
  }
  return etype;
}



