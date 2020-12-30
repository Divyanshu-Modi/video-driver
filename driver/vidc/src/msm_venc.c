// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <media/v4l2_vidc_extensions.h>
#include <media/msm_media_info.h>

#include "msm_venc.h"
#include "msm_vidc_core.h"
#include "msm_vidc_inst.h"
#include "msm_vidc_driver.h"
#include "msm_vidc_internal.h"
#include "msm_vidc_platform.h"
#include "msm_vidc_control.h"
#include "msm_vidc_debug.h"
#include "venus_hfi.h"
#include "hfi_packet.h"

u32 msm_venc_input_set_prop[] = {
	HFI_PROP_COLOR_FORMAT,
	HFI_PROP_RAW_RESOLUTION,
	HFI_PROP_LINEAR_ALIGNMENT_FACTOR,
	HFI_PROP_BUFFER_HOST_MAX_COUNT,
};

u32 msm_venc_output_set_prop[] = {
	HFI_PROP_BITSTREAM_RESOLUTION,
	HFI_PROP_CROP_OFFSETS,
	HFI_PROP_BUFFER_HOST_MAX_COUNT,
};

u32 msm_venc_input_subscribe_for_properties[] = {
	HFI_PROP_NO_OUTPUT,
};

u32 msm_venc_deliver_as_metadata[] = {
	HFI_PROP_BUFFER_TAG,
	HFI_PROP_SEI_MASTERING_DISPLAY_COLOUR,
	HFI_PROP_SEI_CONTENT_LIGHT_LEVEL,
	HFI_PROP_SEI_HDR10PLUS_USERDATA,
	HFI_PROP_EVA_STAT_INFO,
};

u32 msm_venc_subscribe_for_metadata[] = {
	HFI_PROP_BUFFER_TAG,
	HFI_PROP_METADATA_SEQ_HEADER_NAL,
	HFI_PROP_TIMESTAMP,
	HFI_PROP_LTR_MARK_USE_DETAILS,
	HFI_PROP_SUBFRAME_OUTPUT,
	HFI_PROP_ENC_QP_METADATA,
};

u32 msm_venc_output_subscribe_for_properties[] = {
	HFI_PROP_PICTURE_TYPE,
	HFI_PROP_BUFFER_MARK,
};

static int msm_venc_codec_change(struct msm_vidc_inst *inst, u32 v4l2_codec)
{
	int rc = 0;

	if (inst->codec && inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat == v4l2_codec)
		return 0;

	s_vpr_h(inst->sid, "%s: codec changed from %#x to %#x\n",
		__func__, inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat, v4l2_codec);

	inst->codec = v4l2_codec_to_driver(v4l2_codec, __func__);
	rc = msm_vidc_get_inst_capability(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_deinit(inst);
	if (rc)
		goto exit;

	rc = msm_vidc_ctrl_init(inst);
	if (rc)
		goto exit;

exit:
	return rc;
}

/* todo: add logs for each property once finalised */
static int msm_venc_set_colorformat(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 pixelformat;
	enum msm_vidc_colorformat_type colorformat;
	u32 hfi_colorformat;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	pixelformat = inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat;
	if (pixelformat != V4L2_PIX_FMT_VIDC_NV12C &&
	    pixelformat != V4L2_PIX_FMT_VIDC_TP10C) {
		s_vpr_e(inst->sid, "%s: invalid pixelformat %#x\n",
			__func__, pixelformat);
		return -EINVAL;
	}
	colorformat = v4l2_colorformat_to_driver(pixelformat, __func__);
	hfi_colorformat = get_hfi_colorformat(inst, colorformat);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_COLOR_FORMAT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32_ENUM,
			&hfi_colorformat,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

/* TODO: Enable when NV12 support is required */
static int msm_venc_set_linear_alignment_factor(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
/*	int rc = 0;
	u32 pixelformat;
	u32 alignment_factor[2];

	if (port != INPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	pixelformat = inst->fmts[INPUT_PORT].fmt.pix_mp.pixelformat;
	if (pixelformat == V4L2_PIX_FMT_VIDC_NV12C ||
	    pixelformat == V4L2_PIX_FMT_VIDC_TP10C ||
		pixelformat == V4L2_PIX_FMT_VIDC_ARGB32C) {
		s_vpr_e(inst->sid,
			"%s: not a linear color fmt, property is not set\n",
			__func__);
		return 0;
	}

	if (pixelformat == V4L2_PIX_FMT_ARGB32) {
		alignment_factor[0] =
		    (rgb_stride_alignment(pixelformat, __func__) << 16) |
		     rgb_scanline_alignment(pixelformat, __func__);
		alignment_factor[1] = 0;
	} else {
		alignment_factor[0] =
		    (y_stride_alignment(pixelformat, __func__) << 16) |
		     y_scanline_alignment(pixelformat, __func__);
		alignment_factor[1] =
		    (uv_stride_alignment(pixelformat, __func__) << 16) |
		     uv_scanline_alignment(pixelformat, __func__);
	}

	s_vpr_h(inst->sid, "%s: payload[0]: %u payload[1]: %u\n", __func__,
		alignment_factor[0], alignment_factor[1]);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_LINEAR_ALIGNMENT_FACTOR,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_64_PACKED,
			&alignment_factor,
			sizeof(u64));
	if (rc)
		return rc;
*/
	return 0;
}

static int msm_venc_set_raw_resolution(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 resolution;

	if (port != INPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	resolution = inst->fmts[port].fmt.pix_mp.width << 16 |
		inst->fmts[port].fmt.pix_mp.height;
	s_vpr_h(inst->sid, "%s: width: %d height: %d\n", __func__,
			inst->fmts[port].fmt.pix_mp.width,
			inst->fmts[port].fmt.pix_mp.height);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_RAW_RESOLUTION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&resolution,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_bitstream_resolution(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 resolution;

	if (port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	resolution = (inst->fmts[port].fmt.pix_mp.width << 16) |
		inst->fmts[port].fmt.pix_mp.height;
	s_vpr_h(inst->sid, "%s: width: %d height: %d\n", __func__,
			inst->fmts[port].fmt.pix_mp.width,
			inst->fmts[port].fmt.pix_mp.height);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BITSTREAM_RESOLUTION,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_32_PACKED,
			&resolution,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_crop_offsets(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 left_offset, top_offset, right_offset, bottom_offset;
	u64 crop;

	if (port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	left_offset = inst->crop.left;
	top_offset = inst->crop.top;
	right_offset = (inst->fmts[port].fmt.pix_mp.width -
		inst->crop.width);
	bottom_offset = (inst->fmts[port].fmt.pix_mp.height -
		inst->crop.height);

	crop = (u64)right_offset << 48 | (u64)bottom_offset << 32 |
		(u64)left_offset << 16 | top_offset;
	s_vpr_h(inst->sid, "%s: left_offset: %d top_offset: %d "
		"right_offset: %d bottom_offset: %d", __func__,
		left_offset, top_offset, right_offset, bottom_offset);

	rc = venus_hfi_session_property(inst,
			HFI_PROP_CROP_OFFSETS,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_64_PACKED,
			&crop,
			sizeof(u64));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_host_max_buf_count(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	u32 count = DEFAULT_MAX_HOST_BUF_COUNT;

	if (port != INPUT_PORT && port != OUTPUT_PORT) {
		s_vpr_e(inst->sid, "%s: invalid port %d\n", __func__, port);
		return -EINVAL;
	}

	s_vpr_h(inst->sid, "%s: count: %u port: %u\n", __func__, count, port);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_BUFFER_HOST_MAX_COUNT,
			HFI_HOST_FLAGS_NONE,
			get_hfi_port(inst, port),
			HFI_PAYLOAD_U32,
			&count,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_stage(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core = inst->core;

	rc = call_session_op(core, decide_work_mode, inst);
	if (rc) {
		s_vpr_e(inst->sid, "%s: decide_work_mode failed\n",
			__func__);
		return -EINVAL;
	}

	s_vpr_h(inst->sid, "%s: stage: %u\n", __func__, inst->stage);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_STAGE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_NONE,
			HFI_PAYLOAD_U32,
			&inst->stage,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_pipe(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core = inst->core;

	rc = call_session_op(core, decide_work_route, inst);
	if (rc) {
		s_vpr_e(inst->sid, "%s: decide_work_route failed\n",
			__func__);
		return -EINVAL;
	}

	s_vpr_h(inst->sid, "%s: pipe: %u\n", __func__, inst->pipe);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_PIPE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_NONE,
			HFI_PAYLOAD_U32,
			&inst->pipe,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_quality_mode(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst->quality_mode) {
		s_vpr_e(inst->sid, "%s: invalid mode: %u\n",
			__func__, inst->quality_mode);
		return -EINVAL;
	}

	s_vpr_h(inst->sid, "%s: quality_mode: %u\n", __func__, inst->quality_mode);
	rc = venus_hfi_session_property(inst,
			HFI_PROP_QUALITY_MODE,
			HFI_HOST_FLAGS_NONE,
			HFI_PORT_BITSTREAM,
			HFI_PAYLOAD_U32_ENUM,
			&inst->quality_mode,
			sizeof(u32));
	if (rc)
		return rc;
	return 0;
}

static int msm_venc_set_input_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_input_set_prop);
	     i++) {
		switch (msm_venc_input_set_prop[i]) {
		case HFI_PROP_COLOR_FORMAT:
			rc = msm_venc_set_colorformat(inst, INPUT_PORT);
			break;
		case HFI_PROP_RAW_RESOLUTION:
			rc = msm_venc_set_raw_resolution(inst, INPUT_PORT);
			break;
		case HFI_PROP_LINEAR_ALIGNMENT_FACTOR:
			rc = msm_venc_set_linear_alignment_factor(inst, INPUT_PORT);
			break;
		case HFI_PROP_BUFFER_HOST_MAX_COUNT:
			rc = msm_venc_set_host_max_buf_count(inst, INPUT_PORT);
			break;
		default:
			d_vpr_e("%s: unknown property %#x\n", __func__,
				msm_venc_input_set_prop[i]);
			rc = -EINVAL;
			break;
		}
	}

	return rc;
}

static int msm_venc_set_output_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;
	int i = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(msm_venc_output_set_prop);
	     i++) {
		switch (msm_venc_output_set_prop[i]) {
		case HFI_PROP_BITSTREAM_RESOLUTION:
			rc = msm_venc_set_bitstream_resolution(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_CROP_OFFSETS:
			rc = msm_venc_set_crop_offsets(inst, OUTPUT_PORT);
			break;
		case HFI_PROP_BUFFER_HOST_MAX_COUNT:
			rc = msm_venc_set_host_max_buf_count(inst, OUTPUT_PORT);
			break;
		default:
			d_vpr_e("%s: unknown property %#x\n", __func__,
				msm_venc_output_set_prop[i]);
			rc = -EINVAL;
			break;
		}
	}

	return rc;
}

static int msm_venc_set_internal_properties(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	//TODO: set HFI_PORT_NONE properties at master port streamon.
	rc = msm_venc_set_stage(inst);
	if (rc)
		return rc;

	rc = msm_venc_set_pipe(inst);
	if (rc)
		return rc;

	rc = msm_venc_set_quality_mode(inst);
	if (rc)
		return rc;

	return rc;
}

static int msm_venc_get_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	inst->buffers.arp.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_ARP) + 100000000;
	inst->buffers.bin.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_BIN) + 100000000;
	inst->buffers.comv.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_COMV) + 100000000;
	inst->buffers.non_comv.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_NON_COMV) + 100000000;
	inst->buffers.line.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_LINE) + 100000000;
	inst->buffers.dpb.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_DPB) + 100000000;
	//inst->buffers.vpss.size = call_session_op(core, buffer_size,
			//inst, MSM_VIDC_BUF_VPSS) + 100000000;
			//vpss is req - 100 mb

	/* inst->buffers.persist.size = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_PERSIST); */

	inst->buffers.arp.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_ARP);
	inst->buffers.bin.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_BIN);
	inst->buffers.comv.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_COMV);
	inst->buffers.non_comv.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_NON_COMV);
	inst->buffers.line.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_LINE);
	inst->buffers.dpb.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_DPB);
	/* inst->buffers.persist.min_count = call_session_op(core, min_count,
			inst, MSM_VIDC_BUF_PERSIST); */

	s_vpr_h(inst->sid, "internal buffer: min     size\n");
	s_vpr_h(inst->sid, "arp  buffer: %d      %d\n",
		inst->buffers.arp.min_count,
		inst->buffers.arp.size);
	s_vpr_h(inst->sid, "bin  buffer: %d      %d\n",
		inst->buffers.bin.min_count,
		inst->buffers.bin.size);
	s_vpr_h(inst->sid, "comv  buffer: %d      %d\n",
		inst->buffers.comv.min_count,
		inst->buffers.comv.size);
	s_vpr_h(inst->sid, "non_comv  buffer: %d      %d\n",
		inst->buffers.non_comv.min_count,
		inst->buffers.non_comv.size);
	s_vpr_h(inst->sid, "line buffer: %d      %d\n",
		inst->buffers.line.min_count,
		inst->buffers.line.size);
	s_vpr_h(inst->sid, "dpb buffer: %d      %d\n",
		inst->buffers.dpb.min_count,
		inst->buffers.dpb.size);
	/* s_vpr_h(inst->sid, "persist buffer: %d      %d\n",
		inst->buffers.persist.min_count,
		inst->buffers.persist.size); */

	return rc;
}

static int msm_venc_create_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_ARP);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;
	rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;
	/* rc = msm_vidc_create_internal_buffers(inst, MSM_VIDC_BUF_PERSIST);
	if (rc)
		return rc; */

	return 0;
}

static int msm_venc_queue_input_internal_buffers(struct msm_vidc_inst *inst)
{
	int rc = 0;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_ARP);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_BIN);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_NON_COMV);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_LINE);
	if (rc)
		return rc;
	rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_DPB);
	if (rc)
		return rc;
	// TODO: fw is not accepting persist buffer and returning session error.
	//rc = msm_vidc_queue_internal_buffers(inst, MSM_VIDC_BUF_PERSIST);
	if (rc)
		return rc;

	return 0;
}

static int msm_venc_property_subscription(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;
	u32 payload_size = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_PROPERTY;
	if (port == INPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(msm_venc_input_subscribe_for_properties); i++)
			payload[i + 1] = msm_venc_input_subscribe_for_properties[i];
		payload_size = (ARRAY_SIZE(msm_venc_input_subscribe_for_properties) + 1) *
				sizeof(u32);
	} else if (port == OUTPUT_PORT) {
		for (i = 0; i < ARRAY_SIZE(msm_venc_output_subscribe_for_properties); i++)
			payload[i + 1] = msm_venc_output_subscribe_for_properties[i];
		payload_size = (ARRAY_SIZE(msm_venc_output_subscribe_for_properties) + 1) *
				sizeof(u32);
	} else {
		s_vpr_e(inst->sid, "%s: invalid port: %d\n", __func__, port);
		return -EINVAL;
	}

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			payload_size);

	return rc;
}

static int msm_venc_metadata_delivery(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(msm_venc_deliver_as_metadata); i++)
		payload[i + 1] = msm_venc_deliver_as_metadata[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_DELIVERY_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_venc_deliver_as_metadata) + 1) *
			sizeof(u32));

	return rc;
}

static int msm_venc_metadata_subscription(struct msm_vidc_inst *inst,
	enum msm_vidc_port_type port)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 payload[32] = {0};
	u32 i;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;
	d_vpr_h("%s()\n", __func__);

	payload[0] = HFI_MODE_METADATA;
	for (i = 0; i < ARRAY_SIZE(msm_venc_subscribe_for_metadata); i++)
		payload[i + 1] = msm_venc_subscribe_for_metadata[i];

	rc = venus_hfi_session_command(inst,
			HFI_CMD_SUBSCRIBE_MODE,
			port,
			HFI_PAYLOAD_U32_ARRAY,
			&payload[0],
			(ARRAY_SIZE(msm_venc_subscribe_for_metadata) + 1) *
			sizeof(u32));

	return rc;
}

int msm_venc_streamoff_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_streamoff(inst, INPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_venc_streamon_input(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_input_meta_enabled(inst) &&
		!inst->vb2q[INPUT_META_PORT].streaming) {
		s_vpr_e(inst->sid,
			"%s: Meta port must be streamed on before data port\n",
			__func__);
		return -EINVAL;
	}

	//rc = msm_vidc_check_session_supported(inst);
	if (rc)
		goto error;
	//rc = msm_vidc_check_scaling_supported(inst);
	if (rc)
		goto error;

	rc = msm_venc_set_input_properties(inst);
	if (rc)
		goto error;

	/* Decide bse vpp delay after work mode */
	//msm_vidc_set_bse_vpp_delay(inst);

	rc = msm_venc_get_input_internal_buffers(inst);
	if (rc)
		goto error;
	/* check for memory after all buffers calculation */
	//rc = msm_vidc_check_memory_supported(inst);
	if (rc)
		goto error;

	//msm_vidc_update_dcvs(inst);
	//msm_vidc_update_batching(inst);
	//msm_vidc_scale_power(inst);

	rc = msm_venc_create_input_internal_buffers(inst);
	rc = 0; // TODO
	if (rc)
		goto error;
	rc = msm_venc_queue_input_internal_buffers(inst);
	rc = 0; // TODO
	if (rc)
		goto error;

	rc = msm_venc_property_subscription(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_venc_metadata_delivery(inst, INPUT_PORT);
	if (rc)
		return rc;

	rc = msm_vidc_session_streamon(inst, INPUT_PORT);
	if (rc)
		goto error;

	return 0;

error:
	s_vpr_e(inst->sid, "%s: failed\n", __func__);
	msm_venc_streamoff_input(inst);
	return rc;
}

int msm_venc_process_cmd(struct msm_vidc_inst *inst, u32 cmd)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (cmd == V4L2_ENC_CMD_STOP) {
		if (!msm_vidc_allow_stop(inst))
			return -EBUSY;
		rc = venus_hfi_session_command(inst,
				HFI_CMD_DRAIN,
				INPUT_PORT,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
		if (rc)
			return rc;
	} else if (cmd == V4L2_ENC_CMD_START) {
		if (!msm_vidc_allow_start(inst))
			return -EBUSY;
		rc = venus_hfi_session_command(inst,
				HFI_CMD_RESUME,
				INPUT_PORT,
				HFI_PAYLOAD_NONE,
				NULL,
				0);
		if (rc)
			return rc;
	} else {
		d_vpr_e("%s: unknown cmd %d\n", __func__, cmd);
		return -EINVAL;
	}
	return 0;
}

int msm_venc_streamoff_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	rc = msm_vidc_session_streamoff(inst, OUTPUT_PORT);
	if (rc)
		return rc;

	return 0;
}

int msm_venc_streamon_output(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	if (is_output_meta_enabled(inst) &&
		!inst->vb2q[OUTPUT_META_PORT].streaming) {
		s_vpr_e(inst->sid,
			"%s: Meta port must be streamed on before data port\n",
			__func__);
		return -EINVAL;
	}

	rc = msm_venc_set_output_properties(inst);
	if (rc)
		goto error;

	rc = msm_vidc_adjust_v4l2_properties(inst);
	if (rc)
		goto error;

	rc = msm_vidc_set_v4l2_properties(inst);
	if (rc)
		goto error;

	rc = msm_venc_set_internal_properties(inst);
	if (rc)
		goto error;

	rc = msm_venc_property_subscription(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	rc = msm_venc_metadata_subscription(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	rc = msm_vidc_session_streamon(inst, OUTPUT_PORT);
	if (rc)
		goto error;

	return 0;

error:
	s_vpr_e(inst->sid, "%s: failed\n", __func__);
	msm_venc_streamoff_output(inst);
	return rc;
}

int msm_venc_s_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *fmt;

	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->type == INPUT_MPLANE) {
		fmt = &inst->fmts[INPUT_PORT];
		fmt->type = INPUT_MPLANE;
		fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
		fmt->fmt.pix_mp.width = VENUS_Y_STRIDE(
			v4l2_colorformat_to_media(fmt->fmt.pix_mp.pixelformat,
			__func__),
			f->fmt.pix_mp.width);
		fmt->fmt.pix_mp.height = VENUS_Y_SCANLINES(
			v4l2_colorformat_to_media(fmt->fmt.pix_mp.pixelformat,
			__func__),
			f->fmt.pix_mp.height);

		fmt->fmt.pix_mp.num_planes = 1;
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline =
			fmt->fmt.pix_mp.width;
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_INPUT);
		inst->buffers.input.min_count = call_session_op(core,
			min_count, inst, MSM_VIDC_BUF_INPUT);
		inst->buffers.input.extra_count = call_session_op(core,
			extra_count, inst, MSM_VIDC_BUF_INPUT);
		if (inst->buffers.input.actual_count <
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count) {
			inst->buffers.input.actual_count =
				inst->buffers.input.min_count +
				inst->buffers.input.extra_count;
		}
		inst->buffers.input.size =
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage;

		//rc = msm_vidc_check_session_supported(inst);
		if (rc)
			goto err_invalid_fmt;
		//update_log_ctxt(inst->sid, inst->session_type,
		//	mplane->pixelformat);
		s_vpr_h(inst->sid,
			"%s: input: codec %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, f->fmt.pix_mp.pixelformat, f->fmt.pix_mp.width,
			f->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.input.min_count,
			inst->buffers.input.extra_count);

		//msm_vidc_update_dcvs(inst);
		//msm_vidc_update_batching(inst);

	} else if (f->type == INPUT_META_PLANE) {
		fmt = &inst->fmts[INPUT_META_PORT];
		fmt->type = INPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
		if (is_input_meta_enabled(inst)) {
			fmt->fmt.meta.buffersize = call_session_op(core,
				buffer_size, inst, MSM_VIDC_BUF_OUTPUT_META);
			inst->buffers.input_meta.min_count =
					inst->buffers.input.min_count;
			inst->buffers.input_meta.extra_count =
					inst->buffers.input.extra_count;
			inst->buffers.input_meta.actual_count =
					inst->buffers.input.actual_count;
			inst->buffers.input_meta.size = fmt->fmt.meta.buffersize;
		} else {
			fmt->fmt.meta.buffersize = 0;
			inst->buffers.input_meta.min_count = 0;
			inst->buffers.input_meta.extra_count = 0;
			inst->buffers.input_meta.actual_count = 0;
			inst->buffers.input_meta.size = 0;
		}
		s_vpr_h(inst->sid,
			"%s: input meta: size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.input_meta.min_count,
			inst->buffers.input_meta.extra_count);
	} else if (f->type == OUTPUT_MPLANE) {
		fmt = &inst->fmts[OUTPUT_PORT];
		if (fmt->fmt.pix_mp.pixelformat != f->fmt.pix_mp.pixelformat) {
			s_vpr_e(inst->sid,
				"%s: codec changed from %#x to %#x\n", __func__,
				fmt->fmt.pix_mp.pixelformat, f->fmt.pix_mp.pixelformat);
			rc = msm_venc_codec_change(inst, f->fmt.pix_mp.pixelformat);
			if (rc)
				goto err_invalid_fmt;
		}
		fmt->type = OUTPUT_MPLANE;
		if (f->fmt.pix_mp.pixelformat == V4L2_PIX_FMT_HEVC) {
			fmt->fmt.pix_mp.width = ALIGN(f->fmt.pix_mp.width,
					H265_BITSTREM_ALIGNMENT);
			fmt->fmt.pix_mp.height = ALIGN(f->fmt.pix_mp.height,
					H265_BITSTREM_ALIGNMENT);
		} else {
			fmt->fmt.pix_mp.width = ALIGN(f->fmt.pix_mp.width,
					DEFAULT_BITSTREM_ALIGNMENT);
			fmt->fmt.pix_mp.height = ALIGN(f->fmt.pix_mp.height,
					DEFAULT_BITSTREM_ALIGNMENT);
		}
		fmt->fmt.pix_mp.pixelformat = f->fmt.pix_mp.pixelformat;
		fmt->fmt.pix_mp.num_planes = 1;
		fmt->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
		fmt->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
			buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
		inst->buffers.output.min_count = call_session_op(core,
			min_count, inst, MSM_VIDC_BUF_OUTPUT);
		inst->buffers.output.extra_count = call_session_op(core,
			extra_count, inst, MSM_VIDC_BUF_OUTPUT);
		if (inst->buffers.output.actual_count <
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count) {
			inst->buffers.output.actual_count =
				inst->buffers.output.min_count +
				inst->buffers.output.extra_count;
		}
		inst->buffers.output.size =
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage;

		//rc = msm_vidc_check_session_supported(inst);
		if (rc)
			goto err_invalid_fmt;

		/* update crop dimensions */
		inst->crop.left = inst->crop.top = 0;
		inst->crop.width = f->fmt.pix_mp.width;
		inst->crop.height = f->fmt.pix_mp.height;

		//update_log_ctxt(inst->sid, inst->session_type,
		//	mplane->pixelformat);

		s_vpr_h(inst->sid,
			"%s: output: format %#x width %d height %d size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.pix_mp.pixelformat, fmt->fmt.pix_mp.width,
			fmt->fmt.pix_mp.height,
			fmt->fmt.pix_mp.plane_fmt[0].sizeimage,
			inst->buffers.output.min_count,
			inst->buffers.output.extra_count);
	} else if (f->type == OUTPUT_META_PLANE) {
		fmt = &inst->fmts[OUTPUT_META_PORT];
		fmt->type = OUTPUT_META_PLANE;
		fmt->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
		if (is_output_meta_enabled(inst)) {
			fmt->fmt.meta.buffersize = call_session_op(core,
				buffer_size, inst, MSM_VIDC_BUF_OUTPUT_META);
			inst->buffers.output_meta.min_count =
					inst->buffers.output.min_count;
			inst->buffers.output_meta.extra_count =
					inst->buffers.output.extra_count;
			inst->buffers.output_meta.actual_count =
					inst->buffers.output.actual_count;
			inst->buffers.output_meta.size = fmt->fmt.meta.buffersize;
		} else {
			fmt->fmt.meta.buffersize = 0;
			inst->buffers.output_meta.min_count = 0;
			inst->buffers.output_meta.extra_count = 0;
			inst->buffers.output_meta.actual_count = 0;
			inst->buffers.output_meta.size = 0;
		}
		s_vpr_h(inst->sid,
			"%s: output meta: size %d min_count %d extra_count %d\n",
			__func__, fmt->fmt.meta.buffersize,
			inst->buffers.output_meta.min_count,
			inst->buffers.output_meta.extra_count);
	} else {
		s_vpr_e(inst->sid, "%s: invalid type %d\n", __func__, f->type);
		goto err_invalid_fmt;
	}
	memcpy(f, fmt, sizeof(struct v4l2_format));

err_invalid_fmt:
	return rc;
}

int msm_venc_g_fmt(struct msm_vidc_inst *inst, struct v4l2_format *f)
{
	int rc = 0;
	int port;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}

	port = v4l2_type_to_driver_port(inst, f->type, __func__);
	if (port < 0)
		return -EINVAL;

	memcpy(f, &inst->fmts[port], sizeof(struct v4l2_format));

	return rc;
}

int msm_venc_enum_fmt(struct msm_vidc_inst *inst, struct v4l2_fmtdesc *f)
{
	int rc = 0;
	struct msm_vidc_core *core;
	u32 array[32] = {0};
	u32 i = 0, idx = 0;

	if (!inst || !inst->core || !inst->capabilities || !f) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	if (f->type == OUTPUT_MPLANE) {
		u32 codecs = core->capabilities[DEC_CODECS].value;

		while (codecs) {
			if (idx > 31)
				break;
			if (codecs & BIT(i)) {
				array[idx] = codecs & BIT(i);
				idx++;
			}
			i++;
			codecs >>= 1;
		}
		f->pixelformat = v4l2_codec_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		f->flags = V4L2_FMT_FLAG_COMPRESSED;
		strlcpy(f->description, "codec", sizeof(f->description));
	} else if (f->type == INPUT_MPLANE) {
		u32 formats = inst->capabilities->cap[PIX_FMTS].step_or_mask;

		while (formats) {
			if (idx > 31)
				break;
			if (formats & BIT(i)) {
				array[idx] = formats & BIT(i);
				idx++;
			}
			i++;
			formats >>= 1;
		}
		f->pixelformat = v4l2_colorformat_from_driver(array[f->index],
				__func__);
		if (!f->pixelformat)
			return -EINVAL;
		strlcpy(f->description, "colorformat", sizeof(f->description));
	} else if (f->type == INPUT_META_PLANE || f->type == OUTPUT_META_PLANE) {
		if (!f->index) {
			f->pixelformat = V4L2_META_FMT_VIDC;
			strlcpy(f->description, "metadata", sizeof(f->description));
		} else {
			return -EINVAL;
		}
	}
	memset(f->reserved, 0, sizeof(f->reserved));

	s_vpr_h(inst->sid, "%s: index %d, %s : %#x, flags %#x\n",
		__func__, f->index, f->description, f->pixelformat, f->flags);
	return rc;
}

int msm_venc_inst_init(struct msm_vidc_inst *inst)
{
	int rc = 0;
	struct msm_vidc_core *core;
	struct v4l2_format *f;

	d_vpr_h("%s()\n", __func__);
	if (!inst || !inst->core) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	core = inst->core;

	f = &inst->fmts[OUTPUT_PORT];
	f->type = OUTPUT_MPLANE;
	f->fmt.pix_mp.width = DEFAULT_WIDTH;
	f->fmt.pix_mp.height = DEFAULT_HEIGHT;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = 0;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_OUTPUT);
	inst->buffers.output.actual_count =
			inst->buffers.output.min_count +
			inst->buffers.output.extra_count;
	inst->buffers.output.size = f->fmt.pix_mp.plane_fmt[0].sizeimage;

	inst->crop.left = inst->crop.top = 0;
	inst->crop.width = f->fmt.pix_mp.width;
	inst->crop.height = f->fmt.pix_mp.height;

	f = &inst->fmts[OUTPUT_META_PORT];
	f->type = OUTPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_OUTPUT_META);
	inst->buffers.output_meta.min_count = inst->buffers.output.min_count;
	inst->buffers.output_meta.extra_count = inst->buffers.output.extra_count;
	inst->buffers.output_meta.actual_count = inst->buffers.output.actual_count;
	inst->buffers.output_meta.size = f->fmt.meta.buffersize;

	f = &inst->fmts[INPUT_PORT];
	f->type = INPUT_MPLANE;
	f->fmt.pix_mp.pixelformat = V4L2_PIX_FMT_VIDC_NV12C;
	f->fmt.pix_mp.width = VENUS_Y_STRIDE(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_WIDTH);
	f->fmt.pix_mp.height = VENUS_Y_SCANLINES(
		v4l2_colorformat_to_media(f->fmt.pix_mp.pixelformat, __func__),
		DEFAULT_HEIGHT);
	f->fmt.pix_mp.num_planes = 1;
	f->fmt.pix_mp.plane_fmt[0].bytesperline = f->fmt.pix_mp.width;
	f->fmt.pix_mp.plane_fmt[0].sizeimage = call_session_op(core,
		buffer_size, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.min_count = call_session_op(core,
		min_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.extra_count = call_session_op(core,
		extra_count, inst, MSM_VIDC_BUF_INPUT);
	inst->buffers.input.actual_count =
			inst->buffers.input.min_count +
			inst->buffers.input.extra_count;
	inst->buffers.input.size = f->fmt.pix_mp.plane_fmt[0].sizeimage;

	f = &inst->fmts[INPUT_META_PORT];
	f->type = INPUT_META_PLANE;
	f->fmt.meta.dataformat = V4L2_META_FMT_VIDC;
	f->fmt.meta.buffersize = call_session_op(core, buffer_size,
			inst, MSM_VIDC_BUF_INPUT_META);
	inst->buffers.input_meta.min_count = inst->buffers.input.min_count;
	inst->buffers.input_meta.extra_count = inst->buffers.input.extra_count;
	inst->buffers.input_meta.actual_count = inst->buffers.input.actual_count;
	inst->buffers.input_meta.size = f->fmt.meta.buffersize;

	inst->prop.frame_rate = DEFAULT_FPS << 16;
	inst->prop.operating_rate = DEFAULT_FPS << 16;
	inst->stage = MSM_VIDC_STAGE_2;
	inst->pipe = MSM_VIDC_PIPE_4;
	inst->quality_mode = MSM_VIDC_MAX_QUALITY_MODE;

	rc = msm_venc_codec_change(inst,
			inst->fmts[OUTPUT_PORT].fmt.pix_mp.pixelformat);

	return rc;
}

int msm_venc_inst_deinit(struct msm_vidc_inst *inst)
{
	int rc = 0;

	if (!inst) {
		d_vpr_e("%s: invalid params\n", __func__);
		return -EINVAL;
	}
	rc = msm_vidc_ctrl_deinit(inst);

	return rc;
}
