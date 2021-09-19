// SPDX-License-Identifier: GPL-2.0-later
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or later
 * as published by the Free Software Foundation.
 *
 * Author: Andrey Skvortsov <Andrej.Skvortzov@gmail.com>
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kprobes.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>

#define DRIVER_VERSION	"1.0"
#define DRIVER_AUTHOR	"Andrey Skvortsov <andrej.skvortzov@gmail.com>"
#define DRIVER_DESC	"Livepatch regulatory domain for Atheros cards to support 5G AP mode"

static int kprobe_pre_handler(struct kprobe *kp, struct pt_regs *reg);

static struct kprobe ath_regd_init_kprobe = {
	.symbol_name = "ath_regd_init",
	.pre_handler = kprobe_pre_handler,
};


/* copied from ath.h */
struct ath_regulatory {
	char alpha2[2];
	enum nl80211_dfs_regions region;
	u16 country_code;
	u16 max_power_level;
	u16 current_rd;
	int16_t power_limit;
	struct reg_dmn_pair_mapping *regpair;
};

/* from regd.h */
#define COUNTRY_ERD_FLAG        0x8000
#define CTRY_UNITED_STATES      840

static int country = CTRY_UNITED_STATES;
module_param(country, int, 0644);

/*
 * probe following function:
 *
 * int
 * ath_regd_init(struct ath_regulatory *reg,
 *		    struct wiphy *wiphy,
 *		    void (*reg_notifier)(struct wiphy *wiphy,
 *					 struct regulatory_request *request))
 */

static int kprobe_pre_handler(struct kprobe *kp, struct pt_regs *regs)
{
	struct ath_regulatory *reg;

	reg = (struct ath_regulatory *)regs_get_kernel_argument(regs, 0);
	reg->current_rd = country | COUNTRY_ERD_FLAG;
	pr_info("set country_rd to %d\n", reg->current_rd);
	return 0;
}

static int __init kmodule_init(void)
{
	if (register_kprobe(&ath_regd_init_kprobe)) {
		pr_err("Can't register probe\n");
		return -EFAULT;
	}
	pr_err("Loaded regdm patcher. Reload dependent kernel modules\n");
	return 0;
}

static void __exit kmodule_exit(void)
{
	pr_err("unloaded\n");
	unregister_kprobe(&ath_regd_init_kprobe);
}

module_init(kmodule_init);
module_exit(kmodule_exit);


MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
