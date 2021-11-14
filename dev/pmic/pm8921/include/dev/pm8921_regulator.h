#ifndef _PM8921_REGULATOR_H_
#define _PM8921_REGULATOR_H_

struct spmi_regulator {
	const char *name;
	uint32_t base;
	const struct spmi_regulator_mapping *mapping;
};

bool regulator_is_enabled(struct spmi_regulator *vreg)
{
        return false;
}

int regulator_get_voltage(struct spmi_regulator *vreg)
{
        return 0;
}

const char *regulator_get_range_name(struct spmi_regulator *vreg)
{
        return NULL;
}

void regulators_init(struct spmi_regulator *vregs) {}

struct spmi_regulator* target_get_regulators()
{
        return NULL;
}

#endif
