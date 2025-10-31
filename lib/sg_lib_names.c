/*
 * Copyright (c) 2022 Douglas Gilbert.
 * All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the BSD_LICENSE file.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#else
#define SG_SCSI_STRINGS 1
#endif

#include "sg_lib.h"
#include "sg_lib_data.h"
#include "sg_lib_names.h"

/* List of SPC, then SBC, the ZBC mode page names. Tape and other mode pages
 * are squeezed into this list as long as they don't conflict.
 * The value is: (mode_page << 8) | mode_subpage
 * Maintain the list in numerical order to allow binary search. */
const struct sg_lib_simple_value_name_t sg_lib_names_mode_arr[] = {
    {0x0000, "Unit Attention condition"},  /* common vendor specific page */
    {0x0100, "Read-Write error recovery"},      /* SBC */
    {0x0200, "Disconnect-Reconnect"},           /* SPC */
    {0x0300, "Format (obsolete)"},              /* SBC */
    {0x0400, "Rigid disk geometry (obsolete)"}, /* SBC */
    {0x0500, "Flexible disk (obsolete)"},       /* SBC */
    {0x0700, "Verify error recovery"},          /* SBC */
    {0x0800, "Caching"},                        /* SBC */
    {0x0900, "Peripheral device (obsolete)"},   /* SPC */
    {0x0a00, "Control"},                        /* SPC */
    {0x0a01, "Control extension"},              /* SPC */
    {0x0a02, "Application tag"},                /* SBC */
    {0x0a03, "Command duration limit A"},       /* SPC */
    {0x0a04, "Command duration limit B"},       /* SPC */
    {0x0a05, "IO Advice Hints Grouping"},       /* SBC */
    {0x0a06, "Background operation control"},   /* SBC */
    {0x0af0, "Control data protection"},        /* SSC */
    {0x0af1, "PATA control"},                   /* SAT */
    {0x0b00, "Medium Types Supported (obsolete)"},   /* SSC */
    {0x0c00, "Notch and partition (obsolete)"}, /* SBC */
    {0x0d00, "Power condition (obsolete), CD device parameters"},
    {0x0e00, "CD audio control"},               /* MMC */
    {0x0e01, "Target device"},                  /* ADC */
    {0x0e02, "DT device primary port"},         /* ADC */
    {0x0e03, "Logical unit"},                   /* ADC */
    {0x0e04, "Target device serial number"},    /* ADC */
    {0x0f00, "Data compression"},               /* SSC */
    {0x1000, "XOR control (obsolete, Device configuration"}, /* SBC,SSC */
    {0x1001, "Device configuration extension"}, /* SSC */
    {0x1100, "Medium partition (1)"},           /* SSC */
    {0x1400, "Enclosure services management"},  /* SES */
    {0x1800, "Protocol specific logical unit"}, /* transport */
    {0x1900, "Protocol specific port"},         /* transport */
    {0x1901, "Phy control and discovery"},      /* SPL */
    {0x1902, "Shared port control"},            /* SPL */
    {0x1903, "Enhanced phy control"},           /* SPL */
    {0x1904, "Out of band  management control"}, /* SPL */
    {0x1A00, "Power condition"},                /* SPC */
    {0x1A01, "Power consumption"},              /* SPC */
    {0x1Af1, "ATA Power condition"},            /* SPC */
    {0x1b00, "LUN mapping"},                    /* ADC */
    {0x1c00, "Information exceptions control"}, /* SPC */
    {0x1c01, "Background control"},             /* SBC */
    {0x1c02, "Logical block provisioning"},     /* SBC */
    {0x1c02, "Logical block provisioning"},     /* SBC */
    {0x1d00, "Medium configuration, CD/DVD timeout, "
             "element address assignments"},    /* SSC,MMC,SMC */
    {0x1e00, "Transport geometry assignments"}, /* SMC */
    {0x1f00, "Device capabilities"},            /* SMC */

    {-1, NULL},                                 /* sentinel */
};

/* Don't count sentinel when doing binary searches, etc */
const size_t sg_lib_names_mode_len =
                SG_ARRAY_SIZE(sg_lib_names_mode_arr) - 1;

/* List of SPC, then SBC, the ZBC VPD page names. Tape and other VPD pages
 * are squeezed into this list as long as they don't conflict.
 * For VPDs > 0 the value is: (vpd << 8) | vpd_number
 * Maintain the list in numerical order to allow binary search. */
const struct sg_lib_simple_value_name_t sg_lib_names_vpd_arr[] = {
    {0x00, "Supported VPD pages"},              /* SPC */
    {0x80, "Unit serial number"},               /* SPC */
    {0x81, "Implemented operating definition (obsolete)"}, /* SPC */
    {0x82, "ASCII implemented operating definition (obsolete)"}, /* SPC */
    {0x83, "Device identification"},            /* SPC */
    {0x84, "Software interface identification"}, /* SPC */
    {0x85, "Management network addresses"},     /* SPC */
    {0x86, "Extended INQUIRY data"},            /* SPC */
    {0x87, "Mode page policy"},                 /* SPC */
    {0x88, "SCSI ports"},                       /* SPC */
    {0x89, "ATA information"},                  /* SAT */
    {0x8a, "Power condition"},                  /* SPC */
    {0x8b, "Device constituents"},              /* SSC */
    {0x8c, "CFA profile information"},          /* SPC */
    {0x8d, "Power consumption"},                /* SPC */
    {0x8f, "Third party copy"},                 /* SPC */
    {0x90, "Protocol specific logical unit information"}, /* transport */
    {0x91, "Protocol specific port information"}, /* transport */
    {0x92, "SCSI feature sets"},                /* SPC,SBC */
    {0xb0, "Block limits"},                     /* SBC */
    {0xb1, "Block device characteristics"},     /* SBC */
    {0xb2, "Logical block provisioning"},       /* SBC */
    {0xb3, "Referrals"},                        /* SBC */
    {0xb4, "Supported Block Lengths and Protection Types"}, /* SBC */
    {0xb5, "Block device characteristics extension"}, /* SBC */
    {0xb6, "Zoned block device characteristics"}, /* ZBC */
    {0xb7, "Block limits extension"},           /* SBC */
    {0xb8, "Format presets"},                   /* SBC */
    {0xb9, "Concurrent positioning ranges"},    /* SBC */
    {0x01b0, "Sequential access Device Capabilities"}, /* SSC */
    {0x01b1, "Manufacturer-assigned serial number"}, /* SSC */
    {0x01b2, "TapeAlert supported flags"},      /* SSC */
    {0x01b3, "Automation device serial number"}, /* SSC */
    {0x01b4, "Data transfer device element address"}, /* SSC */
    {0x01b5, "Data transfer device element address"}, /* SSC */
    {0x11b0, "OSD information"},                /* OSD */
    {0x11b1, "Security token"},                 /* OSD */

    {-1, NULL},                                 /* sentinel */
};

/* Don't count sentinel when doing binary searches, etc */
const size_t sg_lib_names_vpd_len =
                SG_ARRAY_SIZE(sg_lib_names_vpd_arr) - 1;



/* table from SPC-5 revision 16 [sorted numerically (from Annex E.9)] */
/* Can also be obtained from : https://www.t10.org/lists/stds-num.txt
 * dated 20230814 . */

#ifdef SG_SCSI_STRINGS

const struct sg_lib_simple_value_name_t sg_lib_version_descriptor_arr[] = {
    {0x0, "Version Descriptor not supported or No standard identified"},
    {0x20, "SAM (no version claimed)"},
    {0x3b, "SAM T10/0994-D revision 18"},
    {0x3c, "SAM INCITS 270-1996"},
    {0x3d, "SAM ISO/IEC 14776-411"},
    {0x40, "SAM-2 (no version claimed)"},
    {0x54, "SAM-2 T10/1157-D revision 23"},
    {0x55, "SAM-2 T10/1157-D revision 24"},
    {0x5c, "SAM-2 INCITS 366-2003"},
    {0x5e, "SAM-2 ISO/IEC 14776-412"},
    {0x60, "SAM-3 (no version claimed)"},
    {0x62, "SAM-3 T10/1561-D revision 7"},
    {0x75, "SAM-3 T10/1561-D revision 13"},
    {0x76, "SAM-3 T10/1561-D revision 14"},
    {0x77, "SAM-3 INCITS 402-2005"},
    {0x79, "SAM-3 ISO/IEC 14776-413"},
    {0x80, "SAM-4 (no version claimed)"},
    {0x87, "SAM-4 T10/1683-D revision 13"},
    {0x8b, "SAM-4 T10/1683-D revision 14"},
    {0x90, "SAM-4 INCITS 447-2008"},
    {0x92, "SAM-4 ISO/IEC 14776-414"},
    {0xa0, "SAM-5 (no version claimed)"},
    {0xa2, "SAM-5 T10/2104-D revision 4"},
    {0xa4, "SAM-5 T10/2104-D revision 20"},
    {0xa6, "SAM-5 T10/2104-D revision 21"},
    {0xa8, "SAM-5 INCITS 515-2016"},
    {0xaa, "SAM-5 ISO/IEC 14776-415"},
    {0xc0, "SAM-6 (no version claimed)"},
    {0xc2, "SAM-6 INCITS 546-2021"},
    {0xd4, "SAM-6 BSR INCITS 546 revision 10"},
    {0x120, "SPC (no version claimed)"},
    {0x13b, "SPC T10/0995-D revision 11a"},
    {0x13c, "SPC INCITS 301-1997"},
    {0x140, "MMC (no version claimed)"},
    {0x15b, "MMC T10/1048-D revision 10a"},
    {0x15c, "MMC INCITS 304-1997"},
    {0x160, "SCC (no version claimed)"},
    {0x17b, "SCC T10/1047-D revision 06c"},
    {0x17c, "SCC INCITS 276-1997"},
    {0x180, "SBC (no version claimed)"},
    {0x19b, "SBC T10/0996-D revision 08c"},
    {0x19c, "SBC INCITS 306-1998"},
    {0x19e, "SBC ISO/IEC 14776-321"},
    {0x1a0, "SMC (no version claimed)"},
    {0x1bb, "SMC T10/0999-D revision 10a"},
    {0x1bc, "SMC INCITS 314-1998"},
    {0x1be, "SMC ISO/IEC 14776-351"},
    {0x1c0, "SES (no version claimed)"},
    {0x1db, "SES T10/1212-D revision 08b"},
    {0x1dc, "SES INCITS 305-1998"},
    {0x1dd, "SES T10/1212 revision 08b w/ Amendment INCITS 305/AM1-2000"},
    {0x1de, "SES INCITS 305-1998 w/ Amendment INCITS 305/AM1-2000"},
    {0x1e0, "SCC-2 (no version claimed)"},
    {0x1fb, "SCC-2 T10/1125-D revision 04"},
    {0x1fc, "SCC-2 INCITS 318-1998"},
    {0x200, "SSC (no version claimed)"},
    {0x201, "SSC T10/0997-D revision 17"},
    {0x207, "SSC T10/0997-D revision 22"},
    {0x21c, "SSC INCITS 335-2000"},
    {0x21e, "SSC ISO/IEC 14776-331"},
    {0x220, "RBC (no version claimed)"},
    {0x238, "RBC T10/1240-D revision 10a"},
    {0x23c, "RBC INCITS 330-2000"},
    {0x23e, "RBC ISO/IEC 14776-326"},
    {0x240, "MMC-2 (no version claimed)"},
    {0x255, "MMC-2 T10/1228-D revision 11"},
    {0x25b, "MMC-2 T10/1228-D revision 11a"},
    {0x25c, "MMC-2 INCITS 333-2000"},
    {0x260, "SPC-2 (no version claimed)"},
    {0x267, "SPC-2 T10/1236-D revision 12"},
    {0x269, "SPC-2 T10/1236-D revision 18"},
    {0x275, "SPC-2 T10/1236-D revision 19"},
    {0x276, "SPC-2 T10/1236-D revision 20"},
    {0x277, "SPC-2 INCITS 351-2001"},
    {0x278, "SPC-2 ISO/IEC 14776-452"},
    {0x280, "OCRW (no version claimed)"},
    {0x29e, "OCRW ISO/IEC 14776-381"},
    {0x2a0, "MMC-3 (no version claimed)"},
    {0x2b5, "MMC-3 T10/1363-D revision 9"},
    {0x2b6, "MMC-3 T10/1363-D revision 10g"},
    {0x2b8, "MMC-3 INCITS 360-2002"},
    {0x2e0, "SMC-2 (no version claimed)"},
    {0x2f5, "SMC-2 T10/1383-D revision 5"},
    {0x2fc, "SMC-2 T10/1383-D revision 6"},
    {0x2fd, "SMC-2 T10/1383-D revision 7"},
    {0x2fe, "SMC-2 INCITS 382-2004"},
    {0x300, "SPC-3 (no version claimed)"},
    {0x301, "SPC-3 T10/1416-D revision 7"},
    {0x307, "SPC-3 T10/1416-D revision 21"},
    {0x30f, "SPC-3 T10/1416-D revision 22"},
    {0x312, "SPC-3 T10/1416-D revision 23"},
    {0x314, "SPC-3 INCITS 408-2005"},
    {0x316, "SPC-3 ISO/IEC 14776-453"},
    {0x320, "SBC-2 (no version claimed)"},
    {0x322, "SBC-2 T10/1417-D revision 5a"},
    {0x324, "SBC-2 T10/1417-D revision 15"},
    {0x33b, "SBC-2 T10/1417-D revision 16"},
    {0x33d, "SBC-2 INCITS 405-2005"},
    {0x33e, "SBC-2 ISO/IEC 14776-322"},
    {0x340, "OSD (no version claimed)"},
    {0x341, "OSD T10/1355-D revision 0"},
    {0x342, "OSD T10/1355-D revision 7a"},
    {0x343, "OSD T10/1355-D revision 8"},
    {0x344, "OSD T10/1355-D revision 9"},
    {0x355, "OSD T10/1355-D revision 10"},
    {0x356, "OSD INCITS 400-2004"},
    {0x360, "SSC-2 (no version claimed)"},
    {0x374, "SSC-2 T10/1434-D revision 7"},
    {0x375, "SSC-2 T10/1434-D revision 9"},
    {0x37d, "SSC-2 INCITS 380-2003"},
    {0x37e, "SSC-2 ISO/IEC 14776-342"},
    {0x380, "BCC (no version claimed)"},
    {0x3a0, "MMC-4 (no version claimed)"},
    {0x3b0, "MMC-4 T10/1545-D revision 5"},     /* dropped in spc4r09 */
    {0x3b1, "MMC-4 T10/1545-D revision 5a"},
    {0x3bd, "MMC-4 T10/1545-D revision 3"},
    {0x3be, "MMC-4 T10/1545-D revision 3d"},
    {0x3bf, "MMC-4 INCITS 401-2005"},
    {0x3c0, "ADC (no version claimed)"},
    {0x3d5, "ADC T10/1558-D revision 6"},
    {0x3d6, "ADC T10/1558-D revision 7"},
    {0x3d7, "ADC INCITS 403-2005"},
    {0x3e0, "SES-2 (no version claimed)"},
    {0x3e1, "SES-2 T10/1559-D revision 16"},
    {0x3e7, "SES-2 T10/1559-D revision 19"},
    {0x3eb, "SES-2 T10/1559-D revision 20"},
    {0x3f0, "SES-2 INCITS 448-2008"},
    {0x3f2, "SES-2 ISO/IEC 14776-372"},
    {0x400, "SSC-3 (no version claimed)"},
    {0x403, "SSC-3 T10/1611-D revision 04a"},
    {0x407, "SSC-3 T10/1611-D revision 05"},
    {0x409, "SSC-3 INCITS 467-2011"},
    {0x40b, "SSC-3 ISO/IEC 14776-333"},
    {0x420, "MMC-5 (no version claimed)"},
    {0x42f, "MMC-5 T10/1675-D revision 03"},
    {0x431, "MMC-5 T10/1675-D revision 03b"},
    {0x432, "MMC-5 T10/1675-D revision 04"},
    {0x434, "MMC-5 INCITS 430-2007"},
    {0x440, "OSD-2 (no version claimed)"},
    {0x444, "OSD-2 T10/1729-D revision 4"},
    {0x446, "OSD-2 T10/1729-D revision 5"},
    {0x448, "OSD-2 INCITS 458-2011"},
    {0x460, "SPC-4 (no version claimed)"},
    {0x461, "SPC-4 T10/BSR INCITS 513 revision 16"},
    {0x462, "SPC-4 T10/BSR INCITS 513 revision 18"},
    {0x463, "SPC-4 T10/BSR INCITS 513 revision 23"},
    {0x466, "SPC-4 T10/BSR INCITS 513 revision 36"},
    {0x468, "SPC-4 T10/BSR INCITS 513 revision 37"},
    {0x469, "SPC-4 T10/BSR INCITS 513 revision 37a"},
    {0x46c, "SPC-4 INCITS 513-2015"},
    {0x46e, "SPC-4 ISO/IEC 14776-454"},
    {0x480, "SMC-3 (no version claimed)"},
    {0x482, "SMC-3 T10/1730-D revision 15"},
    {0x484, "SMC-3 T10/1730-D revision 16"},
    {0x486, "SMC-3 INCITS 484-2012"},
    {0x4a0, "ADC-2 (no version claimed)"},
    {0x4a7, "ADC-2 T10/1741-D revision 7"},
    {0x4aa, "ADC-2 T10/1741-D revision 8"},
    {0x4ac, "ADC-2 INCITS 441-2008"},
    {0x4c0, "SBC-3 (no version claimed)"},
    {0x4c3, "SBC-3 T10/BSR INCITS 514 revision 35"},
    {0x4c5, "SBC-3 T10/BSR INCITS 514 revision 36"},
    {0x4c8, "SBC-3 INCITS 514-2014"},
    {0x4ca, "SBC-3 ISO/IEC 14776-323"},
    {0x4e0, "MMC-6 (no version claimed)"},
    {0x4e3, "MMC-6 T10/1836-D revision 02b"},
    {0x4e5, "MMC-6 T10/1836-D revision 02g"},
    {0x4e6, "MMC-6 INCITS 468-2010"},
    {0x4e7, "MMC-6 INCITS 468-2010 + MMC-6/AM1 INCITS 468-2010/AM 1"},
    {0x500, "ADC-3 (no version claimed)"},
    {0x502, "ADC-3 T10/1895-D revision 04"},
    {0x504, "ADC-3 T10/1895-D revision 05"},
    {0x506, "ADC-3 T10/1895-D revision 05a"},
    {0x50a, "ADC-3 INCITS 497-2012"},
    {0x520, "SSC-4 (no version claimed)"},
    {0x523, "SSC-4 T10/BSR INCITS 516 revision 2"},
    {0x525, "SSC-4 T10/BSR INCITS 516 revision 3"},
    {0x527, "SSC-4 INCITS 516-2013"},
    {0x560, "OSD-3 (no version claimed)"},
    {0x580, "SES-3 (no version claimed)"},
    {0x582, "SES-3 T10/BSR INCITS 518 revision 13"},
    {0x584, "SES-3 T10/BSR INCITS 518 revision 14"},
    {0x591, "SES-3 INCITS 518-2017"},
    {0x5a0, "SSC-5 (no version claimed)"},
    {0x5a2, "SSC-5 BSR INCITS 503-2022"},
    {0x5ab, "SSC-5 BSR INCITS 503 revision 06"},
    {0x5af, "SSC-5 AM1 (no version claimed)"},
    {0x5c0, "SPC-5 (no version claimed)"},
    {0x5c2, "SPC-5 INCITS 502-2019"},
    {0x5cb, "SPC-5 BSR INCITS 502 revision 22"},
    {0x5e0, "SFSC (no version claimed)"},
    {0x5e3, "SFSC BSR INCITS 501 revision 01"},
    {0x5e5, "SFSC BSR INCITS 501 revision 02"},
    {0x5e8, "SFSC INCITS 501-2016"},
    {0x5ea, "SFSC ISO/IEC 14776-481"},
    {0x600, "SBC-4 (no version claimed)"},
    {0x602, "SBC-4 INCITS 506-2021"},
    {0x60f, "SBC-4 BSR INCITS 506 revision 20a"},
    {0x610, "SBC-4 BSR INCITS 506 revision 22"},
    {0x620, "ZBC (no version claimed)"},
    {0x622, "ZBC BSR INCITS 536 revision 02"},
    {0x624, "ZBC BSR INCITS 536 revision 05"},
    {0x628, "ZBC INCITS 536-2016"},
    {0x629, "ZBC AM1 INCITS 536-2016/AM1-2019"},
    {0x640, "ADC-4 (no version claimed)"},
    {0x64b, "ADC-4 BSR INCITS 541 revision 04"},
    {0x64c, "ADC-4 BSR INCITS 541 revision 05"},
    {0x660, "ZBC-2 (no version claimed)"},
    {0x662, "ZBC-2 INCITS 550-2023"},
    {0x66b, "ZBC-2 BSR INCITS 550 revision 13"},
    {0x680, "SES-4 (no version claimed)"},
    {0x682, "SES-4 INCITS 555-2020"},
    {0x68f, "SES-4 BSR INCITS 555 revision 03"},
    {0x690, "SES-4 BSR INCITS 555 revision 05"},
    {0x6a0, "ZBC-3 (no version claimed)"},
    {0x6c0, "SBC-5 (no version claimed)"},
    {0x6e0, "SPC-6 (no version claimed)"},
    {0x820, "SSA-TL2 (no version claimed)"},
    {0x83b, "SSA-TL2 T10.1/1147-D revision 05b"},
    {0x83c, "SSA-TL2 INCITS 308-1998"},
    {0x840, "SSA-TL1 (no version claimed)"},
    {0x85b, "SSA-TL1 T10.1/0989-D revision 10b"},
    {0x85c, "SSA-TL1 INCITS 295-1996"},
    {0x860, "SSA-S3P (no version claimed)"},
    {0x87b, "SSA-S3P T10.1/1051-D revision 05b"},
    {0x87c, "SSA-S3P INCITS 309-1998"},
    {0x880, "SSA-S2P (no version claimed)"},
    {0x89b, "SSA-S2P T10.1/1121-D revision 07b"},
    {0x89c, "SSA-S2P INCITS 294-1996"},
    {0x8a0, "SIP (no version claimed)"},
    {0x8bb, "SIP T10/0856-D revision 10"},
    {0x8bc, "SIP INCITS 292-1997"},
    {0x8c0, "FCP (no version claimed)"},
    {0x8db, "FCP T10/0993-D revision 12"},
    {0x8dc, "FCP INCITS 269-1996"},
    {0x8e0, "SBP-2 (no version claimed)"},
    {0x8fb, "SBP-2 T10/1155-D revision 04"},
    {0x8fc, "SBP-2 INCITS 325-1998"},
    {0x900, "FCP-2 (no version claimed)"},
    {0x901, "FCP-2 T10/1144-D revision 4"},
    {0x915, "FCP-2 T10/1144-D revision 7"},
    {0x916, "FCP-2 T10/1144-D revision 7a"},
    {0x917, "FCP-2 INCITS 350-2003"},
    {0x918, "FCP-2 T10/1144-D revision 8"},
    {0x91a, "FCP-2 ISO/IEC 14776-222"},
    {0x920, "SST (no version claimed)"},
    {0x935, "SST T10/1380-D revision 8b"},
    {0x940, "SRP (no version claimed)"},
    {0x954, "SRP T10/1415-D revision 10"},
    {0x955, "SRP T10/1415-D revision 16a"},
    {0x95c, "SRP INCITS 365-2002"},
    /* 0x961 to 0x97F  iSCSI (versions as described via RFC 7144) */
    /* the following (up to 0x67f) taken from that document */
    {0x960, "iSCSI (no version claimed)"},
    {0x961, "iSCSI RFC 7143 iSCSIProtocolLevel=1"},
    {0x962, "iSCSI RFC 7144 iSCSIProtocolLevel=2"},
    {0x963, "iSCSI RFC 7144 iSCSIProtocolLevel=3"},
    {0x964, "iSCSI RFC 7144 iSCSIProtocolLevel=4"},
    {0x965, "iSCSI RFC 7144 iSCSIProtocolLevel=5"},
    {0x966, "iSCSI RFC 7144 iSCSIProtocolLevel=6"},
    {0x967, "iSCSI RFC 7144 iSCSIProtocolLevel=7"},
    {0x968, "iSCSI RFC 7144 iSCSIProtocolLevel=8"},
    {0x969, "iSCSI RFC 7144 iSCSIProtocolLevel=9"},
    {0x96a, "iSCSI RFC 7144 iSCSIProtocolLevel=10"},
    {0x96b, "iSCSI RFC 7144 iSCSIProtocolLevel=11"},
    {0x96c, "iSCSI RFC 7144 iSCSIProtocolLevel=12"},
    {0x96d, "iSCSI RFC 7144 iSCSIProtocolLevel=13"},
    {0x96e, "iSCSI RFC 7144 iSCSIProtocolLevel=14"},
    {0x96f, "iSCSI RFC 7144 iSCSIProtocolLevel=15"},
    {0x970, "iSCSI RFC 7144 iSCSIProtocolLevel=16"},
    {0x971, "iSCSI RFC 7144 iSCSIProtocolLevel=17"},
    {0x972, "iSCSI RFC 7144 iSCSIProtocolLevel=18"},
    {0x973, "iSCSI RFC 7144 iSCSIProtocolLevel=19"},
    {0x974, "iSCSI RFC 7144 iSCSIProtocolLevel=20"},
    {0x975, "iSCSI RFC 7144 iSCSIProtocolLevel=21"},
    {0x976, "iSCSI RFC 7144 iSCSIProtocolLevel=22"},
    {0x977, "iSCSI RFC 7144 iSCSIProtocolLevel=23"},
    {0x978, "iSCSI RFC 7144 iSCSIProtocolLevel=24"},
    {0x979, "iSCSI RFC 7144 iSCSIProtocolLevel=25"},
    {0x97a, "iSCSI RFC 7144 iSCSIProtocolLevel=26"},
    {0x97b, "iSCSI RFC 7144 iSCSIProtocolLevel=27"},
    {0x97c, "iSCSI RFC 7144 iSCSIProtocolLevel=28"},
    {0x97d, "iSCSI RFC 7144 iSCSIProtocolLevel=29"},
    {0x97e, "iSCSI RFC 7144 iSCSIProtocolLevel=30"},
    {0x97f, "iSCSI RFC 7144 iSCSIProtocolLevel=31"},

    {0x980, "SBP-3 (no version claimed)"},
    {0x982, "SBP-3 T10/1467-D revision 1f"},
    {0x994, "SBP-3 T10/1467-D revision 3"},
    {0x99a, "SBP-3 T10/1467-D revision 4"},
    {0x99b, "SBP-3 T10/1467-D revision 5"},
    {0x99c, "SBP-3 INCITS 375-2004"},
    {0x9a0, "SRP-2 (no version claimed)"},
    {0x9bc, "SRP-2 INCITS 551-2019"},
    {0x9c0, "ADP (no version claimed)"},
    {0x9e0, "ADT (no version claimed)"},
    {0x9f9, "ADT T10/1557-D revision 11"},
    {0x9fa, "ADT T10/1557-D revision 14"},
    {0x9fd, "ADT INCITS 406-2005"},
    {0xa00, "FCP-3 (no version claimed)"},
    {0xa07, "FCP-3 T10/1560-D revision 3f"},
    {0xa0f, "FCP-3 T10/1560-D revision 4"},
    {0xa11, "FCP-3 INCITS 416-2006"},
    {0xa1c, "FCP-3 ISO/IEC 14776-223"},
    {0xa20, "ADT-2 (no version claimed)"},
    {0xa22, "ADT-2 T10/1742-D revision 06"},
    {0xa27, "ADT-2 T10/1742-D revision 08"},
    {0xa28, "ADT-2 T10/1742-D revision 09"},
    {0xa2b, "ADT-2 INCITS 472-2011"},
    {0xa40, "FCP-4 (no version claimed)"},
    {0xa42, "FCP-4 T10/1828-D revision 01"},
    {0xa44, "FCP-4 T10/1828-D revision 02"},
    {0xa45, "FCP-4 T10/1828-D revision 02b"},
    {0xa46, "FCP-4 INCITS 481-2011"},
    {0xa50, "FCP-4 ISO/IEC 14776-224"},
    {0xa52, "FCP-4 AM1 INCITS 481-2011/AM1-2018"},
    {0xa60, "ADT-3 (no version claimed)"},
    {0xa62, "ADT-3 INCITS 542-2022"},
    {0xa6b, "ADT-3 BSR INCITS 542 revision 03"},
    {0xa80, "FCP-5 (no version claimed)"},
    {0xa82, "FCP-5 INCITS 563-2023"},
    {0xa8b, "FCP-5 BSR INCITS 563 revision 04"},
    {0xaa0, "SPI (no version claimed)"},
    {0xab9, "SPI T10/0855-D revision 15a"},
    {0xaba, "SPI INCITS 253-1995"},
    {0xabb, "SPI T10/0855-D revision 15a with SPI Amnd revision 3a"},
    {0xabc, "SPI INCITS 253-1995 with SPI Amnd INCITS 253/AM1-1998"},
    {0xac0, "Fast-20 (no version claimed)"},
    {0xadb, "Fast-20 T10/1071 revision 06"},
    {0xadc, "Fast-20 INCITS 277-1996"},
    {0xae0, "SPI-2 (no version claimed)"},
    {0xafb, "SPI-2 T10/1142-D revision 20b"},
    {0xafc, "SPI-2 INCITS 302-1999"},
    {0xb00, "SPI-3 (no version claimed)"},
    {0xb18, "SPI-3 T10/1302-D revision 10"},
    {0xb19, "SPI-3 T10/1302-D revision 13a"},
    {0xb1a, "SPI-3 T10/1302-D revision 14"},
    {0xb1c, "SPI-3 INCITS 336-2000"},
    {0xb20, "EPI (no version claimed)"},
    {0xb3b, "EPI T10/1134 revision 16"},
    {0xb3c, "EPI INCITS TR-23 1999"},
    {0xb40, "SPI-4 (no version claimed)"},
    {0xb54, "SPI-4 T10/1365-D revision 7"},
    {0xb55, "SPI-4 T10/1365-D revision 9"},
    {0xb56, "SPI-4 INCITS 362-2002"},
    {0xb59, "SPI-4 T10/1365-D revision 10"},
    {0xb60, "SPI-5 (no version claimed)"},
    {0xb79, "SPI-5 T10/1525-D revision 3"},
    {0xb7a, "SPI-5 T10/1525-D revision 5"},
    {0xb7b, "SPI-5 T10/1525-D revision 6"},
    {0xb7c, "SPI-5 INCITS 367-2003"},
    {0xbe0, "SAS (no version claimed)"},
    {0xbe1, "SAS T10/1562-D revision 01"},
    {0xbf5, "SAS T10/1562-D revision 03"},
    {0xbfa, "SAS T10/1562-D revision 4"},
    {0xbfb, "SAS T10/1562-D revision 04"},
    {0xbfc, "SAS T10/1562-D revision 05"},
    {0xbfd, "SAS INCITS 376-2003"},
    {0xbfe, "SAS ISO/IEC 14776-150"},
    {0xc00, "SAS-1.1 (no version claimed)"},
    {0xc07, "SAS-1.1 T10/1601-D revision 9"},
    {0xc0f, "SAS-1.1 T10/1601-D revision 10"},
    {0xc11, "SAS-1.1 INCITS 417-2006"},
    {0xc12, "SAS-1.1 ISO/IEC 14776-151"},
    {0xc20, "SAS-2 (no version claimed)"},
    {0xc23, "SAS-2 T10/1760-D revision 14"},
    {0xc27, "SAS-2 T10/1760-D revision 15"},
    {0xc28, "SAS-2 T10/1760-D revision 16"},
    {0xc2a, "SAS-2 INCITS 457-2010"},
    {0xc40, "SAS-2.1 (no version claimed)"},
    {0xc48, "SAS-2.1 T10/2125-D revision 04"},
    {0xc4a, "SAS-2.1 T10/2125-D revision 06"},
    {0xc4b, "SAS-2.1 T10/2125-D revision 07"},
    {0xc4e, "SAS-2.1 INCITS 478-2011"},
    {0xc4f, "SAS-2.1 INCITS 478-2011 w/ Amnd 1 INCITS 478/AM1-2014"},
    {0xc52, "SAS-2.1 ISO/IEC 14776-153"},
    {0xc60, "SAS-3 (no version claimed)"},
    {0xc63, "SAS-3 T10/BSR INCITS 519 revision 05a"},
    {0xc65, "SAS-3 T10/BSR INCITS 519 revision 06"},
    {0xc68, "SAS-3 INCITS 519-2014"},
    {0xc6a, "SAS-3 ISO/IEC 14776-154"},
    {0xc80, "SAS-4 (no version claimed)"},
    {0xc82, "SAS-4 T10/BSR INCITS 534 revision 08a"},
    {0xc84, "SAS-4 T10/BSR INCITS 534 revision 09"},
    {0xc92, "SAS-4 INCITS 534-2019"},
    {0xca0, "SAS-4.1 (no version claimed)"},
    {0xca2, "SAS-4.1 INCITS 567-2023"},
    {0xcaf, "SAS-4.1 BSR INCITS 567 revision 03"},
    {0xcb0, "SAS-4.1 BSR INCITS 567 revision 04"},
    {0xd20, "FC-PH (no version claimed)"},
    {0xd3b, "FC-PH INCITS 230-1994"},
    {0xd3c, "FC-PH INCITS 230-1994 with Amnd 1 INCITS 230/AM1-1996"},
    {0xd40, "FC-AL (no version claimed)"},
    {0xd5c, "FC-AL INCITS 272-1996"},
    {0xd60, "FC-AL-2 (no version claimed)"},
    {0xd61, "FC-AL-2 T11/1133-D revision 7.0"},
    {0xd63, "FC-AL-2 INCITS 332-1999 with AM1-2003 & AM2-2006"},
    {0xd64, "FC-AL-2 INCITS 332-1999 with Amnd 2 AM2-2006"},
    {0xd65, "FC-AL-2 ISO/IEC 14165-122 with AM1 & AM2"},
    {0xd7c, "FC-AL-2 INCITS 332-1999"},
    {0xd7d, "FC-AL-2 INCITS 332-1999 with Amnd 1 AM1-2003"},
    {0xd80, "FC-PH-3 (no version claimed)"},
    {0xd9c, "FC-PH-3 INCITS 303-1998"},
    {0xda0, "FC-FS (no version claimed)"},
    {0xdb7, "FC-FS T11/1331-D revision 1.2"},
    {0xdb8, "FC-FS T11/1331-D revision 1.7"},
    {0xdbc, "FC-FS INCITS 373-2003"},
    {0xdbd, "FC-FS ISO/IEC 14165-251"},
    {0xdc0, "FC-PI (no version claimed)"},
    {0xddc, "FC-PI INCITS 352-2002"},
    {0xde0, "FC-PI-2 (no version claimed)"},
    {0xde2, "FC-PI-2 T11/1506-D revision 5.0"},
    {0xde4, "FC-PI-2 INCITS 404-2006"},
    {0xe00, "FC-FS-2 (no version claimed)"},
    {0xe02, "FC-FS-2 INCITS 242-2007"},
    {0xe03, "FC-FS-2 INCITS 242-2007 with AM1 INCITS 242/AM1-2007"},
    {0xe20, "FC-LS (no version claimed)"},
    {0xe21, "FC-LS T11/1620-D revision 1.62"},
    {0xe29, "FC-LS INCITS 433-2007"},
    {0xe40, "FC-SP (no version claimed)"},
    {0xe42, "FC-SP T11/1570-D revision 1.6"},
    {0xe45, "FC-SP INCITS 426-2007"},
    {0xe60, "FC-PI-3 (no version claimed)"},
    {0xe62, "FC-PI-3 T11/1625-D revision 2.0"},
    {0xe68, "FC-PI-3 T11/1625-D revision 2.1"},
    {0xe6a, "FC-PI-3 T11/1625-D revision 4.0"},
    {0xe6e, "FC-PI-3 INCITS 460-2011"},
    {0xe80, "FC-PI-4 (no version claimed)"},
    {0xe82, "FC-PI-4 T11/1647-D revision 8.0"},
    {0xe88, "FC-PI-4 INCITS 450-2009"},
    {0xea0, "FC 10GFC (no version claimed)"},
    {0xea2, "FC 10GFC INCITS 364-2003"},
    {0xea3, "FC 10GFC ISO/IEC 14165-116"},
    {0xea5, "FC 10GFC ISO/IEC 14165-116 with AM1"},
    {0xea6, "FC 10GFC INCITS 364-2003 with AM1 INCITS 364/AM1-2007"},
    {0xec0, "FC-SP-2 (no version claimed)"},
    {0xee0, "FC-FS-3 (no version claimed)"},
    {0xee2, "FC-FS-3 T11/1861-D revision 0.9"},
    {0xee7, "FC-FS-3 T11/1861-D revision 1.0"},
    {0xee9, "FC-FS-3 T11/1861-D revision 1.10"},
    {0xeeb, "FC-FS-3 INCITS 470-2011"},
    {0xf00, "FC-LS-2 (no version claimed)"},
    {0xf03, "FC-LS-2 T11/2103-D revision 2.11"},
    {0xf05, "FC-LS-2 T11/2103-D revision 2.21"},
    {0xf07, "FC-LS-2 INCITS 477-2011"},
    {0xf20, "FC-PI-5 (no version claimed)"},
    {0xf27, "FC-PI-5 T11/2118-D revision 2.00"},
    {0xf28, "FC-PI-5 T11/2118-D revision 3.00"},
    {0xf2a, "FC-PI-5 T11/2118-D revision 6.00"},
    {0xf2b, "FC-PI-5 T11/2118-D revision 6.10"},
    {0xf2e, "FC-PI-5 INCITS 479-2011"},
    {0xf40, "FC-PI-6 (no version claimed)"},
    {0xf60, "FC-FS-4 (no version claimed)"},
    {0xf80, "FC-LS-3 (no version claimed)"},
    {0x12a0, "FC-SCM (no version claimed)"},
    {0x12a3, "FC-SCM T11/1824DT revision 1.0"},
    {0x12a5, "FC-SCM T11/1824DT revision 1.1"},
    {0x12a7, "FC-SCM T11/1824DT revision 1.4"},
    {0x12aa, "FC-SCM INCITS TR-47 2012"},
    {0x12c0, "FC-DA-2 (no version claimed)"},
    {0x12c3, "FC-DA-2 T11/1870DT revision 1.04"},
    {0x12c5, "FC-DA-2 T11/1870DT revision 1.06"},
    {0x12c9, "FC-DA-2 INCITS TR-49 2012"},
    {0x12e0, "FC-DA (no version claimed)"},
    {0x12e2, "FC-DA T11/1513-DT revision 3.1"},
    {0x12e8, "FC-DA INCITS TR-36 2004"},
    {0x12e9, "FC-DA ISO/IEC 14165-341"},
    {0x1300, "FC-Tape (no version claimed)"},
    {0x1301, "FC-Tape T11/1315 revision 1.16"},
    {0x131b, "FC-Tape T11/1315 revision 1.17"},
    {0x131c, "FC-Tape INCITS TR-24 1999"},
    {0x1320, "FC-FLA (no version claimed)"},
    {0x133b, "FC-FLA T11/1235 revision 7"},
    {0x133c, "FC-FLA INCITS TR-20 1998"},
    {0x1340, "FC-PLDA (no version claimed)"},
    {0x135b, "FC-PLDA T11/1162 revision 2.1"},
    {0x135c, "FC-PLDA INCITS TR-19 1998"},
    {0x1360, "SSA-PH2 (no version claimed)"},
    {0x137b, "SSA-PH2 T10.1/1145-D revision 09c"},
    {0x137c, "SSA-PH2 INCITS 293-1996"},
    {0x1380, "SSA-PH3 (no version claimed)"},
    {0x139b, "SSA-PH3 T10.1/1146-D revision 05b"},
    {0x139c, "SSA-PH3 INCITS 307-1998"},
    {0x14a0, "IEEE 1394 (no version claimed)"},
    {0x14bd, "IEEE 1394-1995"},
    {0x14c0, "IEEE 1394a (no version claimed)"},
    {0x14e0, "IEEE 1394b (no version claimed)"},
    {0x15e0, "ATA/ATAPI-6 (no version claimed)"},
    {0x15fd, "ATA/ATAPI-6 INCITS 361-2002"},
    {0x1600, "ATA/ATAPI-7 (no version claimed)"},
    {0x1602, "ATA/ATAPI-7 T13/1532-D revision 3"},
    {0x161c, "ATA/ATAPI-7 INCITS 397-2005"},
    {0x161e, "ATA/ATAPI-7 ISO/IEC 24739"},
    {0x1620, "ATA/ATAPI-8 ATA8-AAM (no version claimed)"},
    {0x1621, "ATA/ATAPI-8 ATA8-APT Parallel transport (no version claimed)"},
    {0x1622, "ATA/ATAPI-8 ATA8-AST Serial transport (no version claimed)"},
    {0x1623, "ATA/ATAPI-8 ATA8-ACS ATA/ATAPI command set (no version "
             "claimed)"},
    {0x1628, "ATA/ATAPI-8 ATA8-AAM INCITS 451-2008"},
    {0x162a, "ATA/ATAPI-8 ATA8-ACS INCITS 452-2009 w/ Amendment 1"},
    {0x1630, "ATA/ATAPI-8 ATA8-ACS ATA/ATAPI Command Set ISO/IEC 17760-101"},
    {0x1728, "Universal Serial Bus Specification, Revision 1.1"},
    {0x1729, "Universal Serial Bus Specification, Revision 2.0"},
    {0x172a, "Universal Serial Bus 3.2 Specification Revision 1.0"},
    {0x172b, "Universal Serial Bus 4 Specification Version 1.0"},
    {0x172c, "Universal Serial Bus 4 Specification Version 2.0"},
    {0x1730, "USB Mass Storage Class Bulk-Only Transport, Revision 1.0"},
    {0x1740, "UAS (no version claimed)"},    /* USB attached SCSI */
    {0x1743, "UAS T10/2095-D revision 02"},
    {0x1747, "UAS T10/2095-D revision 04"},
    {0x1748, "UAS INCITS 471-2010"},
    {0x1749, "UAS ISO/IEC 14776-251"},
    {0x1761, "ACS-2 (no version claimed)"},
    {0x1762, "ACS-2 INCITS 482-2013"},
    {0x1765, "ACS-3 (no version claimed)"},
    {0x1766, "ACS-3 INCITS 522-2014"},
    {0x1767, "ACS-4 INCITS 529-2018"},
    {0x1768, "ACS-4 (no version claimed)"},
    {0x1769, "ACS-5 (no version claimed)"},
    {0x176a, "ACS-5 INCITS 558-2021"},
    {0x176e, "ACS-6 (no version claimed)"},
    {0x1778, "ACS-2 ISO/IEC 17760-102"},
    {0x1779, "ACS-3 ISO/IEC 17760-103"},
    {0x177b, "ACS-5 ISO/IEC 17760-105"},
    {0x1780, "UAS-2 (no version claimed)"},
    {0x17a3, "ZAC (no version claimed)"},
    {0x17a4, "ZAC INCITS 537-2016"},
    {0x17a5, "ZAC-2 INCITS 549-2022"},
    {0x17a6, "ZAC-2 (no version claimed)"},
    {0x17a7, "ZAC-3 (no version claimed)"},
    {0x17bc, "ZAC AM1 INCITS 537-2016/AM1-2019"},
    {0x17c0, "UAS-3 (no version claimed)"},
    {0x17c2, "UAS-3 INCITS 572-2021"},
    {0x17c5, "UAS-3 BSR INCITS 572 revision 05"},
    {0x1807, "UAS-3 ISO/IEC 14776-253"},
    {0x1ea0, "SAT (no version claimed)"},
    {0x1ea7, "SAT T10/1711-D revision 8"},
    {0x1eab, "SAT T10/1711-D revision 9"},
    {0x1ead, "SAT INCITS 431-2007"},
    {0x1ec0, "SAT-2 (no version claimed)"},
    {0x1ec4, "SAT-2 T10/1826-D revision 06"},
    {0x1ec8, "SAT-2 T10/1826-D revision 09"},
    {0x1eca, "SAT-2 INCITS 465-2010"},
    {0x1ee0, "SAT-3 (no version claimed)"},
    {0x1ee2, "SAT-3 T10/BSR INCITS 517 revision 4"},
    {0x1ee4, "SAT-3 T10/BSR INCITS 517 revision 7"},
    {0x1ee8, "SAT-3 INCITS 517-2015"},
    {0x1f00, "SAT-4 (no version claimed)"},
    {0x1f02, "SAT-4 T10/BSR INCITS 491 revision 5"},
    {0x1f04, "SAT-4 T10/BSR INCITS 491 revision 6"},
    {0x1f0c, "SAT-4 INCITS 491-2018"},
    {0x1f20, "SAT-5 (no version claimed)"},
    {0x1f25, "SAT-5 BSR INCITS 577 revision 10"},
    {0x1f40, "SAT-6 (no version claimed)"},
    {0x1f60, "SNT (no version claimed)"},
    {0x20a0, "SPL (no version claimed)"},
    {0x20a3, "SPL T10/2124-D revision 6a"},
    {0x20a5, "SPL T10/2124-D revision 7"},
    {0x20a7, "SPL INCITS 476-2011"},
    {0x20a8, "SPL INCITS 476-2011 + SPL AM1 INCITS 476/AM1 2012"},
    {0x20aa, "SPL ISO/IEC 14776-261"},
    {0x20c0, "SPL-2 (no version claimed)"},
    {0x20c2, "SPL-2 T10/BSR INCITS 505 revision 4"},
    {0x20c4, "SPL-2 T10/BSR INCITS 505 revision 5"},
    {0x20c8, "SPL-2 INCITS 505-2013"},
    {0x20c9, "SPL-2 ISO/IEC 14776-262"},
    {0x20e0, "SPL-3 (no version claimed)"},
    {0x20e4, "SPL-3 T10/BSR INCITS 492 revision 6"},
    {0x20e6, "SPL-3 T10/BSR INCITS 492 revision 7"},
    {0x20e8, "SPL-3 INCITS 492-2015"},
    {0x20e9, "SPL-3 ISO/IEC 14776-263"},
    {0x2100, "SPL-4 (no version claimed)"},
    {0x2102, "SPL-4 T10/BSR INCITS 538 revision 08a"},
    {0x2104, "SPL-4 T10/BSR INCITS 538 revision 10"},
    {0x2105, "SPL-4 T10/BSR INCITS 538 revision 11"},
    {0x2107, "SPL-4 T10/BSR INCITS 538 revision 13"},
    {0x2110, "SPL-4 INCITS 538-2018"},
    {0x2120, "SPL-5 (no version claimed)"},
    {0x2122, "SPL-5 INCITS 554-2023"},
    {0x212e, "SPL-5 BSR INCITS 554 revision 14"},
    {0x212f, "SPL-5 BSR INCITS 554 revision 15"},
    {0x21e0, "SOP (no version claimed)"},
    {0x21e4, "SOP T10/BSR INCITS 489 revision 4"},
    {0x21e6, "SOP T10/BSR INCITS 489 revision 5"},
    {0x21e8, "SOP INCITS 489-2014"},
    {0x2200, "PQI (no version claimed)"},
    {0x2204, "PQI T10/BSR INCITS 490 revision 6"},
    {0x2206, "PQI T10/BSR INCITS 490 revision 7"},
    {0x2208, "PQI INCITS 490-2014"},
    {0x2220, "SOP-2 (no draft published)"},
    {0x2240, "PQI-2 (no version claimed)"},
    {0x2242, "PQI-2 T10/BSR INCITS 507 revision 01"},
    {0x2244, "PQI-2 INCITS 507-2016"},
    {0x2460, "ADT-4 (no version claimed)"},
    {0x246b, "ADT-4 INCITS 541-2023"},
    {0xffc0, "IEEE 1667 (no version claimed)"},
    {0xffc1, "IEEE 1667-2006"},
    {0xffc2, "IEEE 1667-2009"},
    {0xffc3, "IEEE 1667-2015"},
    {0xffc4, "IEEE 1667-2018"},
    {0xffff, NULL},     /* sentinel, leave at end */
};

#else

const struct sg_lib_simple_value_name_t sg_lib_version_descriptor_arr[] = {
    {0xffff, NULL},     /* sentinel, leave at end */
};

#endif		/* SG_SCSI_STRINGS */

/* Don't count sentinel when doing binary searches, etc */
const size_t sg_lib_version_descriptor_len =
                SG_ARRAY_SIZE(sg_lib_version_descriptor_arr) - 1;
