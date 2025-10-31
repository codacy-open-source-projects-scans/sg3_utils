#ifndef SG_VPD_COMMON_H
#define SG_VPD_COMMON_H

/*
 * Copyright (c) 2022-2023 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

/* This is a common header file for the sg_inq and sg_vpd utilities */

#include <stdint.h>
#include <stdbool.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sg_lib.h"
#include "sg_json_sg_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SINQ_COMMON_RESP_LEN 36
#define SINQ_VER_DESC_RESP_LEN 74

/* standard VPD pages, in ascending page number order */
#define VPD_SUPPORTED_VPDS 0x0
#define VPD_UNIT_SERIAL_NUM 0x80
#define VPD_IMP_OP_DEF 0x81             /* obsolete in SPC-2 */
#define VPD_ASCII_OP_DEF 0x82           /* obsolete in SPC-2 */
#define VPD_DEVICE_ID 0x83
#define VPD_SOFTW_INF_ID 0x84
#define VPD_MAN_NET_ADDR 0x85
#define VPD_EXT_INQ 0x86                /* Extended Inquiry */
#define VPD_MODE_PG_POLICY 0x87
#define VPD_SCSI_PORTS 0x88
#define VPD_ATA_INFO 0x89
#define VPD_POWER_CONDITION 0x8a
#define VPD_DEVICE_CONSTITUENTS 0x8b
#define VPD_CFA_PROFILE_INFO 0x8c
#define VPD_POWER_CONSUMPTION  0x8d
#define VPD_3PARTY_COPY 0x8f            /* 3PC, XCOPY, SPC-5, SBC-4 */
#define VPD_PROTO_LU 0x90
#define VPD_PROTO_PORT 0x91
#define VPD_SCSI_FEATURE_SETS 0x92      /* spc5r11 */
#define VPD_BLOCK_LIMITS 0xb0           /* SBC-3 */
#define VPD_SA_DEV_CAP 0xb0             /* SSC-3 */
#define VPD_OSD_INFO 0xb0               /* OSD */
#define VPD_BLOCK_DEV_CHARS 0xb1        /* SBC-3 */
#define VPD_MAN_ASS_SN 0xb1             /* SSC-3, ADC-2 */
#define VPD_SECURITY_TOKEN 0xb1         /* OSD */
#define VPD_TA_SUPPORTED 0xb2           /* SSC-3 */
#define VPD_LB_PROVISIONING 0xb2        /* SBC-3 */
#define VPD_REFERRALS 0xb3              /* SBC-3 */
#define VPD_AUTOMATION_DEV_SN 0xb3      /* SSC-3 */
#define VPD_SUP_BLOCK_LENS 0xb4         /* sbc4r01 */
#define VPD_DTDE_ADDRESS 0xb4           /* SSC-4 */
#define VPD_BLOCK_DEV_C_EXTENS 0xb5     /* sbc4r02 */
#define VPD_LB_PROTECTION 0xb5          /* SSC-5 */
#define VPD_ZBC_DEV_CHARS 0xb6          /* zbc-r01b */
#define VPD_BLOCK_LIMITS_EXT 0xb7       /* sbc4r08 */
#define VPD_FORMAT_PRESETS 0xb8         /* sbc4r18 */
#define VPD_CON_POS_RANGE 0xb9          /* sbc5r01 */
#define VPD_CAP_PROD_ID 0xba            /* sbc5r04 */
#define VPD_NOPE_WANT_STD_INQ -2        /* request for standard inquiry */

/* vendor/product identifiers */
#define VPD_VP_SEAGATE 0
#define VPD_VP_RDAC 1
#define VPD_VP_EMC 2
#define VPD_VP_DDS 3
#define VPD_VP_HP3PAR 4
#define VPD_VP_IBM_LTO 5
#define VPD_VP_HP_LTO 6
#define VPD_VP_WDC_HITACHI 7
#define VPD_VP_NVME 8
#define VPD_VP_SG 9     /* this package/library as a vendor */


/* vendor VPD pages */
#define VPD_V_HIT_PG3 0x3
#define VPD_V_HP3PAR 0xc0
#define VPD_V_FIRM_SEA  0xc0
#define VPD_V_UPR_EMC  0xc0
#define VPD_V_HVER_RDAC  0xc0
#define VPD_V_FVER_DDS 0xc0
#define VPD_V_FVER_LTO 0xc0
#define VPD_V_DCRL_LTO 0xc0
#define VPD_V_DATC_SEA  0xc1
#define VPD_V_FVER_RDAC  0xc1
#define VPD_V_HVER_LTO 0xc1
#define VPD_V_DSN_LTO 0xc1
#define VPD_V_JUMP_SEA 0xc2
#define VPD_V_SVER_RDAC 0xc2
#define VPD_V_PCA_LTO 0xc2
#define VPD_V_DEV_BEH_SEA 0xc3
#define VPD_V_FEAT_RDAC 0xc3
#define VPD_V_MECH_LTO 0xc3
#define VPD_V_SUBS_RDAC 0xc4
#define VPD_V_HEAD_LTO 0xc4
#define VPD_V_ACI_LTO 0xc5
#define VPD_V_DUCD_LTO 0xc7
#define VPD_V_EDID_RDAC 0xc8
#define VPD_V_MPDS_LTO 0xc8
#define VPD_V_VAC_RDAC 0xc9
#define VPD_V_RVSI_RDAC 0xca
#define VPD_V_SAID_RDAC 0xd0
#define VPD_V_HIT_PG_D1 0xd1
#define VPD_V_HIT_PG_D2 0xd2

#ifndef SG_NVME_VPD_NICR
#define SG_NVME_VPD_NICR 0xde   /* NVME Identify Controller Response */
#endif

#define DEF_ALLOC_LEN 252
#define MX_ALLOC_LEN (0xc000 + 0x80)
#define DEF_PT_TIMEOUT  60       /* 60 seconds */


/* This structure holds the union of options available in sg_inq and sg_vpd */
struct opts_t {
    bool do_all;                /* sg_vpd */
    bool do_ata;                /* sg_inq */
    bool do_decode;             /* sg_inq */
    bool do_debug;              /* sg_inq + sg_vpd (hidden) */
    bool do_descriptors;        /* sg_inq + sg_vpd */
    bool do_enum;               /* sg_enum */
    bool do_export;             /* sg_inq */
    bool do_force;              /* sg_inq + sg_vpd */
    bool do_json;               /* sg_inq + sg_vpd */
    bool do_only; /* sg_inq: --only after stdinq: don't fetch VPD page 0x80 */
    bool do_quiet;              /* sg_inq (new) + sg_vpd */
    bool examine_given;         /* sg_vpd */
    bool page_given;            /* sg_inq + sg_vpd */
    bool possible_nvme;         /* sg_inq */
    bool protect_not_sure;      /* sg_vpd */
    bool verbose_given;         /* sg_inq + sg_vpd */
    bool version_given;         /* sg_inq + sg_vpd */
    bool do_vpd;                /* sg_inq */
    bool std_inq_a_valid;       /* sg_inq + sg_vpd */
#ifdef SG_SCSI_STRINGS
    bool opt_new;               /* sg_inq */
#endif
    int cns;                    /* sg_inq(nvme device) */
    int do_block;               /* sg_inq */
    int do_cmddt;               /* sg_inq */
    int do_help;                /* sg_inq */
    int do_hex;                 /* sg_inq + sg_vpd */
    int do_ident;               /* sg_vpd */
    int do_long;                /* sg_inq[int] + sg_vpd[bool] */
    int do_raw;                 /* sg_inq + sg_vpd */
    int do_vendor;              /* sg_inq */
    int examine;                /* sg_vpd */
    int maxlen;                 /* sg_inq[was: resp_len] + sg_vpd */
    int num_pages;              /* sg_inq */
    int page_pdt;               /* sg_inq */
    int vend_prod_num;          /* sg_vpd */
    int verbose;                /* sg_inq + sg_vpd */
    int vpd_pn;                 /* sg_vpd */
    const char * device_name;   /* sg_inq + sg_vpd */
    const char * page_str;      /* sg_inq + sg_vpd */
    const char * inhex_fn;      /* sg_inq + sg_vpd */
    const char * json_arg;      /* sg_inq + sg_vpd */
    const char * js_file;       /* sg_inq + sg_vpd */
    const char * sinq_inraw_fn; /* sg_inq + sg_vpd */
    const char * vend_prod_arg; /* sg_vpd */
    sgj_state json_st;
    uint8_t std_inq_a[SINQ_VER_DESC_RESP_LEN];
};

struct svpd_values_name_t {
    int value;       /* VPD page number */
    int subvalue;    /* to differentiate if value+pdt are not unique */
    int pdt;         /* peripheral device type id, -1 is the default */
                     /* (all or not applicable) value */
    const char * acron;
    const char * name;
};

struct svpd_vp_name_t {
    int vend_prod_num;       /* vendor/product identifier */
    const char * acron;
    const char * name;
};

typedef int (*recurse_vpd_decodep)(struct opts_t *, sgj_opaque_p jop, int off);


sgj_opaque_p sg_vpd_js_hdr(sgj_state * jsp, sgj_opaque_p jop,
                           const char * name, const uint8_t * vpd_hdrp);
void decode_man_net_vpd(const uint8_t * buff, int len, struct opts_t * op,
                        sgj_opaque_p jap);
void decode_x_inq_vpd(const uint8_t * b, int len, bool protect,
                      struct opts_t * op, sgj_opaque_p jop);
void decode_softw_inf_id(const uint8_t * buff, int len, struct opts_t * op,
                         sgj_opaque_p jap);
void decode_mode_policy_vpd(const uint8_t * buff, int len, struct opts_t * op,
                            sgj_opaque_p jap);
void decode_cga_profile_vpd(const uint8_t * buff, int len, struct opts_t * op,
                       sgj_opaque_p jap);
void decode_power_condition(const uint8_t * buff, int len, struct opts_t * op,
                            sgj_opaque_p jop);
int filter_json_dev_ids(uint8_t * buff, int len, int m_assoc,
                        struct opts_t * op, sgj_opaque_p jap);
void decode_ata_info_vpd(const uint8_t * buff, int len, struct opts_t * op,
                        sgj_opaque_p jop);
void decode_feature_sets_vpd(const uint8_t * buff, int len, struct opts_t * op,
                             sgj_opaque_p jap);
void decode_dev_constit_vpd(const uint8_t * buff, int len,
                            struct opts_t * op, sgj_opaque_p jap,
                            recurse_vpd_decodep fp);
sgj_opaque_p std_inq_decode_js(const uint8_t * b, int len,
                               struct opts_t * op, sgj_opaque_p jop);
void decode_power_consumption(const uint8_t * buff, int len,
                              struct opts_t * op, sgj_opaque_p jap);
void decode_block_limits_vpd(const uint8_t * buff, int len,
                             struct opts_t * op, sgj_opaque_p jop);
void decode_block_dev_ch_vpd(const uint8_t * buff, int len,
                             struct opts_t * op, sgj_opaque_p jop);
int decode_block_lb_prov_vpd(const uint8_t * buff, int len,
                             struct opts_t * op, sgj_opaque_p jop);
void decode_referrals_vpd(const uint8_t * buff, int len, struct opts_t * op,
                          sgj_opaque_p jop);
void decode_sup_block_lens_vpd(const uint8_t * buff, int len,
                               struct opts_t * op, sgj_opaque_p jap);
void decode_cap_prod_id_vpd(const uint8_t * buff, int len, struct opts_t * op,
                            sgj_opaque_p jap);
void decode_block_dev_char_ext_vpd(const uint8_t * buff, int len,
                                   struct opts_t * op, sgj_opaque_p jop);
void decode_zbdch_vpd(const uint8_t * buff, int len, struct opts_t * op,
                      sgj_opaque_p jop);
void decode_block_limits_ext_vpd(const uint8_t * buff, int len,
                                 struct opts_t * op, sgj_opaque_p jop);
void decode_format_presets_vpd(const uint8_t * buff, int len,
                               struct opts_t * op, sgj_opaque_p jap);
void decode_con_pos_range_vpd(const uint8_t * buff, int len,
                              struct opts_t * op, sgj_opaque_p jap);
void decode_3party_copy_vpd(const uint8_t * buff, int len, struct opts_t * op,
                            sgj_opaque_p jap);
void
decode_proto_lu_vpd(const uint8_t * buff, int len, struct opts_t * op,
                    sgj_opaque_p jap);
void
decode_proto_port_vpd(const uint8_t * buff, int len, struct opts_t * op,
                      sgj_opaque_p jap);
void
decode_lb_protection_vpd(const uint8_t * buff, int len, struct opts_t * op,
                         sgj_opaque_p jap);
void
decode_tapealert_supported_vpd(const uint8_t * buff, int len,
                               struct opts_t * op, sgj_opaque_p jop);
/* T10's SNT project has started [version descriptor: 1F60h] and this is
 * this package's suggestion for a SNT specific VPD page. It is modelled
 * on the ATA Info VPD page [0x89h] which has been stable in the SAT
 * series of standards. It is currently given a identifier in the vendor
 * VPD space: 0xde . If T10 accepts this idea then maybe it could be
 * given a VPD identifier of 0x99 (0x10 more than the ATA Info VPD page). */
void decode_snt_nvme_info_vpd(uint8_t * buff, int len, struct opts_t * op,
                              sgj_opaque_p jop);

/* Share some vendor specific VPD pages as well */
void
decode_upr_vpd_c0_emc(uint8_t * buff, int len, struct opts_t * op,
                      sgj_opaque_p jop);
void
decode_rdac_vpd_c2(uint8_t * buff, int len, struct opts_t * op,
                   sgj_opaque_p jop);
void
decode_rdac_vpd_c9(uint8_t * buff, int len, struct opts_t * op,
                   sgj_opaque_p jop);

const char * pqual_str(int pqual);
int no_ascii_4hex(const struct opts_t * op);

void svpd_enumerate_vendor(int vend_prod_num);
int svpd_count_vendor_vpds(int vpd_pn, int vend_prod_num);
int svpd_decode_vendor(struct sg_pt_base * ptvp, struct opts_t * op,
                       sgj_opaque_p jop, int off);
void sgjv_js_hex_long(sgj_state * jsp, sgj_opaque_p jop, const uint8_t * bp,
                      int len);
const struct svpd_values_name_t * svpd_find_vendor_by_acron(const char * ap);
int svpd_find_vp_num_by_acron(const char * vp_ap);
const struct svpd_values_name_t * svpd_find_vendor_by_num(int page_num,
                                                          int vend_prod_num);
int vpd_fetch_page(struct sg_pt_base * ptvp, uint8_t * rp, int page,
                   int mxlen, bool qt, int vb, int * rlenp);

void named_hhh_output(const char * pname, const uint8_t * buff, int len,
                      const struct opts_t * op);

extern uint8_t * rsp_buff;
extern const char * t10_vendor_id_hr;
extern const char * t10_vendor_id_js;
extern const char * product_id_hr;
extern const char * product_id_js;
extern const char * product_rev_lev_hr;
extern const char * product_rev_lev_js;
extern const char * vpd_pg_s;
extern const char * lts_s;

extern const char * svp_vpdp;
extern const char * usn_vpdp;
extern const char * di_vpdp;
extern const char * mna_vpdp;
extern const char * eid_vpdp;
extern const char * mpp_vpdp;
extern const char * sp_vpdp;
extern const char * ai_vpdp;
extern const char * pc_vpdp;
extern const char * dc_vpdp;
extern const char * cpi_vpdp;
extern const char * psm_vpdp;
extern const char * tpc_vpdp;
extern const char * pslu_vpdp;
extern const char * pspo_vpdp;
extern const char * sfs_vpdp;
extern const char * bl_vpdp;
extern const char * sad_vpdp;
extern const char * osdi_vpdp;
extern const char * bdc_vpdp;
extern const char * masn_vpdp;
extern const char * st_vpdp;
extern const char * lbpv_vpdp;
extern const char * tas_vpdp;
extern const char * ref_vpdp;
extern const char * adsn_vpdp;
extern const char * sbl_vpdp;
extern const char * dtde_vpdp;
extern const char * bdce_vpdp;
extern const char * lbpro_vpdp;
extern const char * zbdc_vpdp;
extern const char * ble_vpdp;
extern const char * fp_vpdp;
extern const char * cpr_vpdp;
extern const char * cap_vpdp;

extern const struct svpd_vp_name_t vp_arr[];
extern const struct svpd_values_name_t vendor_vpd_pg[];


#ifdef __cplusplus
}
#endif

#endif  /* end of SG_VPD_COMMON_H */
