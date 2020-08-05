/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef _VENUS_HFI_H_
#define _VENUS_HFI_H_

#include "msm_vidc_internal.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_core.h"

#define VIDC_MAX_NAME_LENGTH 		64
#define VIDC_MAX_PC_SKIP_COUNT 		10
#define VIDC_MAX_SUBCACHES 			4
#define VIDC_MAX_SUBCACHE_SIZE 		52

enum vidc_resource_id {
	VIDC_RESOURCE_NONE,
	VIDC_RESOURCE_SYSCACHE,
	VIDC_UNUSED_RESOURCE = 0x10000000,
};

struct vidc_resource_hdr {
	enum vidc_resource_id resource_id;
	void *resource_handle;
};

struct vidc_buffer_addr_info {
	enum msm_vidc_buffer_type buffer_type;
	u32 buffer_size;
	u32 num_buffers;
	u32 align_device_addr;
	u32 extradata_addr;
	u32 extradata_size;
	u32 response_required;
};

struct hfi_resource_subcache_type {
	u32 size;
	u32 sc_id;
};

struct hfi_resource_syscache_info_type {
	u32 num_entries;
	struct hfi_resource_subcache_type rg_subcache_entries[1];
};

int venus_hfi_core_init(struct msm_vidc_core *core);
int venus_hfi_core_release(struct msm_vidc_core *core);
int venus_hfi_suspend(struct msm_vidc_core *core);
int venus_hfi_session_open(struct msm_vidc_core *core, struct msm_vidc_inst *inst);
void venus_hfi_work_handler(struct work_struct *work);
void venus_hfi_pm_work_handler(struct work_struct *work);

void __write_register(struct msm_vidc_core *core,
		u32 reg, u32 value);
int __read_register(struct msm_vidc_core *core, u32 reg);
void __disable_unprepare_clks(struct msm_vidc_core *core);
int __disable_regulators(struct msm_vidc_core *core);
int __unvote_buses(struct msm_vidc_core *core);
int __prepare_pc(struct msm_vidc_core *core);

int __reset_ahb2axi_bridge(struct msm_vidc_core *core);
int __clock_config_on_enable(struct msm_vidc_core *core);
int __interrupt_init(struct msm_vidc_core *core);
int __setup_ucregion_memmap(struct msm_vidc_core *core);
int __raise_interrupt(struct msm_vidc_core *core);
int __power_off(struct msm_vidc_core *core);

#endif // _VENUS_HFI_H_