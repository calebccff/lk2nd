#include <stdlib.h>
#include <debug.h>
#include <target.h>
#include <dev/keys.h>
#if PLATFORM_MSM8960
#include <dev/pm8921.h>
#else
#include <pm8x41.h>
#endif
#include <platform/gpio.h>
#include <platform/timer.h>
#include <lk2nd.h>

/* if qcom does that... */
extern int target_volume_up_old();
extern uint32_t target_volume_down_old();

static struct lk2nd_keymap *keymap = NULL;

void target_init_keys(void)
{
	int i = 0;
	struct pm8x41_gpio pm_gpio = {
		.direction = PM_GPIO_DIR_IN,
		.function  = 0,
		.vin_sel   = 2,
	};
	keymap = lk2nd_dev.keymap;

	while (keymap && keymap[i].key) {
		switch (keymap[i].type) {
			case KEY_RESIN:
				break;
			case KEY_GPIO:
				gpio_tlmm_config(keymap[i].gpio, 0, GPIO_INPUT, keymap[i].pull, GPIO_2MA, GPIO_ENABLE);
				break;
			case KEY_PM_GPIO:
				pm_gpio.pull = keymap[i].pull;
				pm8x41_gpio_config(keymap[i].gpio, &pm_gpio);
				break;
		}
		i++;
	}

	/* Wait for the gpio config to take effect - debounce time */
	udelay(10000);
}

int target_key_pressed(int key)
{
	int i = 0;
	uint8_t tmp = 0, st = 0;
	bool found = false;
	while (keymap && keymap[i].key) {
		if (keymap[i].key == key) {
			found = true;
			switch (keymap[i].type) {
				case KEY_RESIN:
#if PLATFORM_MSM8960
					dprintf(INFO, "key %d is resin, stubbed on pm921\n", key);
					st = 0;
#else
					st = pm8x41_resin_status();
#endif
					
					break;
				case KEY_GPIO:
					st = (gpio_status(keymap[i].gpio) == keymap[i].active);
					break;
				case KEY_PM_GPIO:
#if PLATFORM_MSM8960
					pm8921_gpio_get(keymap[i].gpio, &tmp);
#else
					pm8x41_gpio_get(keymap[i].gpio, &tmp);
#endif
					
					st = (tmp == keymap[i].active);
					break;
			}
			if (st)
				return 1;
		}
		i++;
	}

	if (found)
		return 0;

	/* fallback to default handlers only if the keycode wasn't set in the DT */
#if !PLATFORM_MSM8960
	switch (key) {
		case KEY_VOLUMEDOWN:
			return target_volume_down_old();
		case KEY_VOLUMEUP:
			return target_volume_up_old();
	}
#endif
	return 0;
}

int target_volume_up() {
	return target_key_pressed(KEY_VOLUMEUP);
}

int target_volume_down() {
	return target_key_pressed(KEY_VOLUMEDOWN);
}

void target_keystatus()
{
	keys_init();
	target_init_keys();

	if (target_volume_down())
		keys_post_event(KEY_VOLUMEDOWN, 1);

	if (target_volume_up())
		keys_post_event(KEY_VOLUMEUP, 1);

	if (target_key_pressed(KEY_HOME))
		keys_post_event(KEY_HOME, 1);
}


