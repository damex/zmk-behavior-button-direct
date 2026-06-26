# Copyright 2026 Roman Kuzmitskii (@damex)
# SPDX-License-Identifier: MIT

if(CONFIG_DT_HAS_ZMK_BEHAVIOR_BUTTON_DIRECT_ENABLED AND
   (NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL))
  zephyr_library()
  zephyr_library_compile_options(-Wconversion -Wsign-conversion)
  zephyr_library_sources(src/behavior_button_direct.c)
endif()
