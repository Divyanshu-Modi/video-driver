/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#ifndef __VENUS_HFI_RESPONSE_H__
#define __VENUS_HFI_RESPONSE_H__

int handle_response(struct msm_vidc_core *core,
	void *response);
int validate_packet(u8 *response_pkt, u8 *core_resp_pkt,
	u32 core_resp_pkt_size, const char *func);
bool is_valid_port(struct msm_vidc_inst *inst, u32 port,
	const char *func);
bool is_valid_hfi_buffer_type(struct msm_vidc_inst *inst,
	u32 buffer_type, const char *func);
void handle_session_input_psc_work_handler(struct work_struct *work);
int handle_session_input_psc(struct msm_vidc_inst *inst,
		struct input_psc_work *psc_work);

#endif // __VENUS_HFI_RESPONSE_H__
