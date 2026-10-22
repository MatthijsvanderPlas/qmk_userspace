// Copyright 2026 Matthijs van der Plas
// SPDX-License-Identifier: GPL-2.0-or-later

// Reports the active layer over raw HID to a host that subscribes (the Elora cheatsheet page).
// Silent unless subscribed, so the Vial app sharing this channel never receives stray reports.

#include QMK_KEYBOARD_H
#include "raw_hid.h"
#include "via.h"

#define LAYER_REPORT_ID 0xF1
#define LAYER_REPORT_SUBSCRIBE 0x01
#define LAYER_REPORT_CHANGED 0x02
#define LAYER_REPORT_TTL_MS 10000 // the host re-subscribes every few seconds
#define LAYER_REPORT_SIZE 32

static bool     subscribed    = false;
static uint32_t subscribed_at = 0;

static bool is_subscribed(void) {
    return subscribed && timer_elapsed32(subscribed_at) < LAYER_REPORT_TTL_MS;
}

void raw_hid_receive_kb(uint8_t *data, uint8_t length) {
    if (length >= 3 && data[0] == LAYER_REPORT_ID && data[1] == LAYER_REPORT_SUBSCRIBE) {
        subscribed    = true;
        subscribed_at = timer_read32();
        data[2]       = get_highest_layer(layer_state); // via.c sends this buffer back as the reply
        return;
    }
    data[0] = id_unhandled;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    if (is_keyboard_master() && is_subscribed()) {
        uint8_t report[LAYER_REPORT_SIZE] = {LAYER_REPORT_ID, LAYER_REPORT_CHANGED, get_highest_layer(state)};
        raw_hid_send(report, sizeof(report));
    }
    return state;
}
