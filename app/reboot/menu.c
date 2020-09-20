// SPDX-License-Identifier: GPL-2.0+
// © 2019 Mis012

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pm8x41.h>
#include <pm8x41_hw.h>
#include <kernel/thread.h>
#include <dev/fbcon.h>
#include <target.h>

#include "aboot/aboot.h"

#include "boot.h"
#include "config.h"

int num_of_boot_entries;

struct boot_entry *entry_list;

struct hardcoded_entry {
	char *title;
	void (*function)(void);
};

//FUGLY
#define TIMEOUT_TEXT "press volume down for boot menu"
#define TIMEOUT_TEXT_SCALE 2

extern uint32_t target_volume_down(); //used in non-FUGLY code as well; commented out there, will use this

extern struct global_config global_config;
extern bool FUGLY_boot_to_default_entry;

static void FUGLY_default_boot_function() 
{
	int i;
	int num_iters = global_config.timeout * 1000 / 100; // times 1000 - sec to msec; divided by 100 - see "lower cpu stress"

	if(global_config.timeout > 0) {
		fbcon_draw_text(20, 20, TIMEOUT_TEXT, TIMEOUT_TEXT_SCALE, 0xFF0000);

		for (i = 0; i < num_iters; i++) {
			if (target_volume_down()) {
				fbcon_draw_text(20, 20, TIMEOUT_TEXT, TIMEOUT_TEXT_SCALE, 0x000000);
				return; //continue to boot menu
			}
			thread_sleep(100); //lower cpu stress
		}
	} else {
		thread_sleep(100); //somehow this is needed to avoid freeze
	}

	boot_to_entry(global_config.default_entry);
	dprintf(CRITICAL, "ERROR: Booting default entry failed. Forcibly bringing up menu.\n");
}
// end of FUGLY
void boot_from_mmc(void);
void boot_recovery_from_mmc(void);
#define HARDCODED_ENTRY_COUNT 3
struct hardcoded_entry hardcoded_entry_list[HARDCODED_ENTRY_COUNT] = {
	{.title = "legacy boot from 'boot' partition", .function = boot_from_mmc},
	{.title = "legacy boot from 'recovery' partition", .function = boot_recovery_from_mmc},
	{.title = "power off", .function = shutdown_device}
};

#define BOOT_ENTRY_SCALE 2
#define ACTUAL_FONT_WIDTH (FONT_WIDTH * BOOT_ENTRY_SCALE)
#define ACTUAL_FONT_HEIGHT (FONT_HEIGHT * BOOT_ENTRY_SCALE)

int selected_option = 0;

static void draw_menu(void) {
	num_of_boot_entries = get_entry_count();

	uint32_t max_width = fbcon_get_width() - 1;
	uint32_t max_height = fbcon_get_height() - 1;

	uint32_t frame_width = max_width * 0.90;
	uint32_t frame_height = max_height * 0.90;

	uint32_t margin_x = (max_width - frame_width) / 2;
	uint32_t margin_y = (max_height - frame_height) / 2;

	uint32_t highlight_color;
	uint32_t font_color;

	fbcon_draw_rectangle(margin_x, margin_y, frame_width, frame_height, 0xFFFFFF);

	int i;
	for (i = 0; i < num_of_boot_entries; i++) {
		if(i == selected_option)
			highlight_color = 0xFF0000;
		else
			highlight_color = 0x000000;

		if((entry_list + i)->error)
			font_color = 0xFF0000;
		else
			font_color = 0xFFFFFF;

		fbcon_draw_filled_rectangle(margin_x + 8, (margin_y + 8) + i * (ACTUAL_FONT_HEIGHT + 4) - 1, frame_width - (2 * 8), 2 + ACTUAL_FONT_HEIGHT + 2, highlight_color);
		fbcon_draw_text(margin_x + 10, (margin_y + 10) + i * (ACTUAL_FONT_HEIGHT + 4), (entry_list + i)->title, BOOT_ENTRY_SCALE, font_color);
	}

	uint32_t separator_y = (margin_y + 8) + i * (ACTUAL_FONT_HEIGHT + 4) + 2;

	fbcon_draw_horizontal_line(margin_x + 8, (margin_x + 8) + (frame_width - (2 * 8)), separator_y, 0xFFFFFF);

	for (i = 0; i < HARDCODED_ENTRY_COUNT; i++) {

		if((i + num_of_boot_entries) == selected_option)
			highlight_color = 0xFF0000;
		else
			highlight_color = 0x000000;

		font_color = 0xFFFFFF;

		fbcon_draw_filled_rectangle(margin_x + 8, (separator_y + 3) + i * (ACTUAL_FONT_HEIGHT + 4), frame_width - (2 * 8), 2 + ACTUAL_FONT_HEIGHT + 2, highlight_color);
		fbcon_draw_text(margin_x + 10, (separator_y + 5) + i * (ACTUAL_FONT_HEIGHT + 4), hardcoded_entry_list[i].title, BOOT_ENTRY_SCALE, font_color);
	}

	fbcon_flush();

}

#define KEY_DETECT_FREQUENCY		50

extern int target_volume_up();
//extern uint32_t target_volume_down(); //declared up top because FUGLY

static bool handle_keys(void) {
	uint32_t volume_up_pressed = target_volume_up();
	uint32_t volume_down_pressed = target_volume_down();
	uint32_t power_button_pressed = pm8x41_get_pwrkey_is_pressed();

	//FUGLY
	if(FUGLY_boot_to_default_entry) {
		FUGLY_boot_to_default_entry = 0; //in case we interrupt the autoboot
		FUGLY_default_boot_function();
	}
	//end of FUGLY

	if(volume_up_pressed) {
		if(selected_option > 0)
			selected_option--;
		return 1;
	}
	
	if (volume_down_pressed) {
		if(selected_option < (num_of_boot_entries + HARDCODED_ENTRY_COUNT - 1))
		selected_option++;
		return 1;
	}

	if(power_button_pressed) {
		if(selected_option < num_of_boot_entries) {
			struct boot_entry *entry = entry_list + selected_option;
			boot_to_entry(entry);
		}
		else {
			hardcoded_entry_list[selected_option - num_of_boot_entries].function();
		}
	}

	return 0;
}

int menu_thread(void *arg) {
	entry_list = (struct boot_entry *)arg;

	fbcon_clear();
	if(!FUGLY_boot_to_default_entry)
		draw_menu();

	while(1) {
		if(handle_keys()) {
			draw_menu();
			thread_sleep(100); //improve precision
		}

		thread_sleep(KEY_DETECT_FREQUENCY);
	}
}

// hardcoded functions

extern unsigned int boot_into_recovery;

void boot_from_mmc(void) {
	boot_into_recovery = 0;
	boot_linux_from_mmc();
}

void boot_recovery_from_mmc(void) {
	boot_into_recovery = 1;
	boot_linux_from_mmc();
}
