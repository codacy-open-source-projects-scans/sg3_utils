/* A utility program originally written for the Linux OS SCSI subsystem.
 *  Copyright (C) 2004-2023 D. Gilbert
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  This program outputs information provided by a SCSI REPORT SUPPORTED
 *  OPERATION CODES [0xa3/0xc] (RSOC) and REPORT SUPPORTED TASK MANAGEMENT
 *  FUNCTIONS [0xa3/0xd] (RSTMF) commands.
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "sg_lib.h"
#include "sg_cmds_basic.h"
#include "sg_cmds_extra.h"
#include "sg_unaligned.h"
#include "sg_pr2serr.h"
#include "sg_json_sg_lib.h"

#include "sg_pt.h"

static const char * version_str = "1.03 20231209";    /* spc6r11 */

#define MY_NAME "sg_opcodes"

#define SENSE_BUFF_LEN 64       /* Arbitrary, could be larger */
#define DEF_TIMEOUT_SECS 60

#define SG_MAINTENANCE_IN 0xa3
#define RSOC_SA     0xc
#define RSTMF_SA    0xd
#define RSOC_CMD_LEN 12
#define RSTMF_CMD_LEN 12
#define MX_ALLOC_LEN 8192

#define NAME_BUFF_SZ 128

#define RSOC_ALL_BYTES_CTDP_0 8
#define RSOC_ALL_BYTES_CTDP_1 20

#define SEAGATE_READ_UDS_DATA_CMD 0xf7  /* may start reporting vendor cmds */

static int peri_dtype = -1; /* ugly but not easy to pass to alpha compare */
static bool no_final_msg = false;

static const char * ac_pd_sn = "all_commands_parameter_data";
static const char * oc_pd_sn = "one_command_parameter_data";

static const char * rstmf_b_pd_sn =
    "reported_supported_task_management_functions_basic_parameter_data";
static const char * rstmf_e_pd_sn =
    "reported_supported_task_management_functions_extended_parameter_data";

static const struct option long_options[] = {
    {"alpha", no_argument, 0, 'a'},
    {"compact", no_argument, 0, 'c'},
    {"enumerate", no_argument, 0, 'e'},
    {"help", no_argument, 0, 'h'},
    {"hex", no_argument, 0, 'H'},
    {"inhex", required_argument, 0, 'i'},
    {"in", required_argument, 0, 'i'},
    {"json", optional_argument, 0, '^'},    /* short option is '-j' */
    {"js-file", required_argument, 0, 'J'},
    {"js_file", required_argument, 0, 'J'},
    {"mask", no_argument, 0, 'm'},
    {"mlu", no_argument, 0, 'M'},           /* added in spc5r20 */
    {"no-inquiry", no_argument, 0, 'n'},
    {"no_inquiry", no_argument, 0, 'n'},
    {"new", no_argument, 0, 'N'},
    {"opcode", required_argument, 0, 'o'},
    {"old", no_argument, 0, 'O'},
    {"pdt", required_argument, 0, 'p'},
    {"raw", no_argument, 0, 'r'},
    {"rctd", no_argument, 0, 'R'},
    {"repd", no_argument, 0, 'q'},
    {"rep-opts", required_argument, 0, 'Q'},
    {"rep_opts", required_argument, 0, 'Q'},
    {"sa", required_argument, 0, 's'},
    {"tmf", no_argument, 0, 't'},
    {"unsorted", no_argument, 0, 'u'},
    {"verbose", no_argument, 0, 'v'},
    {"version", no_argument, 0, 'V'},
    {0, 0, 0, 0},
};

struct opts_t {
    bool do_alpha;
    bool do_compact;
    bool do_json;
    bool do_enumerate;
    bool no_inquiry;
    bool do_mask;
    bool do_mlu;
    bool do_raw;
    bool do_rctd;       /* Return command timeout descriptor */
    bool do_repd;
    bool do_unsorted;
    bool do_taskman;
    bool opt_new;
    bool rep_opts_given;
    bool verbose_given;
    bool version_given;
    int rep_opts;
    int do_help;
    int do_hex;
    int opcode;
    int servact;
    int verbose;
    const char * device_name;
    const char * inhex_fn;
    const char * json_arg;
    const char * js_file;
    sgj_state json_st;
};


static void
usage()
{
    pr2serr("Usage:  sg_opcodes [--alpha] [--compact] [--enumerate] "
            "[--help] [--hex]\n"
            "                   [--inhex=FN] [--json[=JO]] [--js-file=JFN] "
            "[--mask]\n"
            "                   [--mlu] [--no-inquiry] [--opcode=OP[,SA]] "
            "[--pdt=DT]\n"
            "                   [--raw] [--rctd] [--repd] [--rep-opts=RO] "
            "[--sa=SA]\n"
            "                   [--tmf] [--unsorted] [--verbose] "
            "[--version]\n"
            "                   DEVICE\n"
            "  where:\n"
            "    --alpha|-a      output list of operation codes sorted "
            "alphabetically\n"
            "    --compact|-c    more compact output\n"
            "    --enumerate|-e    use '--opcode=' and '--pdt=' to look up "
            "name,\n"
            "                      ignore DEVICE\n"
            "    --help|-h       print usage message then exit\n"
            "    --hex|-H        output response in hex, use -HHH for "
            "hex\n"
            "                    suitable for later use of --inhex= "
            "option\n"
            "    --inhex=FN|-i FN    contents of file FN treated as hex "
            "and used\n"
            "                        instead of DEVICE which is ignored\n"
            "    --json[=JO]|-j[=JO]    output in JSON instead of plain "
            "text\n"
            "                           Use --json=? for JSON help\n"
            "    --js-file=JFN|-J JFN    JFN is a filename to which JSON "
            "output is\n"
            "                            written (def: stdout); truncates "
            "then writes\n"
            "    --mask|-m       show cdb usage data (a mask) when "
            "all listed\n"
            "    --mlu|-M        show MLU bit when all listed\n"
            "    --no-inquiry|-n    don't output INQUIRY information\n"
            "    --opcode=OP[,SA]|-o OP[,SA]    opcode (OP) and service "
            "action (SA)\n"
            "    --pdt=DT|-p DT    give peripheral device type for "
            "'--no-inquiry',\n"
            "                      '--enumerate' and '--inhex=FN'\n"
            "    --raw|-r        output response in binary to stdout unless "
            "--inhex=FN\n"
            "                    is given then FN is parsed as binary "
            "instead\n"
            "    --rctd|-R       set RCTD (return command timeout "
            "descriptor) bit\n"
            "    --repd|-q       set Report Extended Parameter Data bit, "
            "with --tmf\n"
            "    --rep-opts=RO|-Q RO    set Reporting Options field in cdb\n"
            "    --sa=SA|-s SA    service action in addition to opcode\n"
            "    --tmf|-t        output list of supported task management "
            "functions\n"
            "    --unsorted|-u    output list of operation codes as is\n"
            "                     (def: sort by opcode (then service "
            "action))\n"
            "    --verbose|-v    increase verbosity\n"
            "    --old|-O        use old interface (use as first option)\n"
            "    --version|-V    print version string then exit\n\n"
            "Performs a SCSI REPORT SUPPORTED OPERATION CODES or a REPORT "
            "SUPPORTED\nTASK MANAGEMENT FUNCTIONS command. All values are "
            "in decimal by default,\nprefix with '0x' or add a trailing 'h' "
            "for hex numbers.\n");
}

static void
usage_old()
{
    pr2serr("Usage:  sg_opcodes [-a] [-c] [-e] [-H] [-j] [-m] [-M] [-n] "
            "[-o=OP]\n"
            "                   [-p=DT] [-q] [-r] [-R] [-s=SA] [-t] [-u] "
            "[-v] [-V]\n"
            "                   DEVICE\n"
            "  where:\n"
            "    -a    output list of operation codes sorted "
            "alphabetically\n"
            "    -c    more compact output\n"
            "    -e    use '--opcode=' and '--pdt=' to look up name, "
            "ignore DEVICE\n"
            "    -H    print response in hex\n"
            "    -j    print response in JSON\n"
            "    -m    show cdb usage data (a mask) when all listed\n"
            "    -M    show MLU bit when all listed\n"
            "    -n    don't output INQUIRY information\n"
            "    -o=OP    first byte of command to query (in hex)\n"
            "    -p=DT    alternate source of pdt (normally obtained from "
            "inquiry)\n"
            "    -q    set REPD bit for tmf_s\n"
            "    -r    output response in binary to stdout\n"
            "    -R    set RCTD (return command timeout "
            "descriptor) bit\n"
            "    -s=SA    in addition to opcode (in hex)\n"
            "    -t    output list of supported task management functions\n"
            "    -u    output list of operation codes as is (unsorted)\n"
            "    -v    verbose\n"
            "    -V    output version string\n"
            "    -N|--new   use new interface\n"
            "    -?    output this usage message\n\n"
            "Performs a SCSI REPORT SUPPORTED OPERATION CODES (or a REPORT "
            "TASK MANAGEMENT\nFUNCTIONS) command\n");
}

static const char * const rsoc_s = "Report supported operation codes";

static int
do_rsoc(struct sg_pt_base * ptvp, bool rctd, int rep_opts, int rq_opcode,
        int rq_servact, void * resp, int mx_resp_len, int * act_resp_lenp,
        bool noisy, int verbose)
{
    int ret, res, sense_cat;
    uint8_t rsoc_cdb[RSOC_CMD_LEN] = {SG_MAINTENANCE_IN, RSOC_SA, 0,
                                              0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN] SG_C_CPP_ZERO_INIT;

    if (rctd)
        rsoc_cdb[2] |= 0x80;
    if (rep_opts)
        rsoc_cdb[2] |= (rep_opts & 0x7);
    if (rq_opcode > 0)
        rsoc_cdb[3] = (rq_opcode & 0xff);
    if (rq_servact > 0)
        sg_put_unaligned_be16((uint16_t)rq_servact, rsoc_cdb + 4);
    if (act_resp_lenp)
        *act_resp_lenp = 0;
    sg_put_unaligned_be32((uint32_t)mx_resp_len, rsoc_cdb + 6);

    if (verbose) {
        char b[128];

        pr2serr("    %s cdb: %s\n", rsoc_s,
                sg_get_command_str(rsoc_cdb, RSOC_CMD_LEN, false,
                                   sizeof(b), b));
    }
    clear_scsi_pt_obj(ptvp);
    set_scsi_pt_cdb(ptvp, rsoc_cdb, sizeof(rsoc_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, -1, DEF_TIMEOUT_SECS, verbose);
    ret = sg_cmds_process_resp(ptvp, rsoc_s, res, noisy, verbose, &sense_cat);
    if (-1 == ret) {
        if (get_scsi_pt_transport_err(ptvp))
            ret = SG_LIB_TRANSPORT_ERROR;
        else
            ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    } else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else {
        if (act_resp_lenp)
            *act_resp_lenp = ret;
        if ((verbose > 2) && (ret > 0)) {
            pr2serr("%s response:\n", rsoc_s);
            hex2stderr((const uint8_t *)resp, ret, 1);
        }
        ret = 0;
    }
    return ret;
}

static const char * const rstmf_s = "Report supported task management "
                                    "functions";

static int
do_rstmf(struct sg_pt_base * ptvp, bool repd, void * resp, int mx_resp_len,
         int * act_resp_lenp, bool noisy, int verbose)
{
    int ret, res, sense_cat;
    uint8_t rstmf_cdb[RSTMF_CMD_LEN] = {SG_MAINTENANCE_IN, RSTMF_SA,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sense_b[SENSE_BUFF_LEN] SG_C_CPP_ZERO_INIT;

    if (repd)
        rstmf_cdb[2] = 0x80;
    if (act_resp_lenp)
        *act_resp_lenp = 0;
    sg_put_unaligned_be32((uint32_t)mx_resp_len, rstmf_cdb + 6);

    if (verbose) {
        char b[128];

        pr2serr("    %s cdb: %s\n", rstmf_s,
                sg_get_command_str(rstmf_cdb, RSTMF_CMD_LEN, false,
                                   sizeof(b), b));
    }
    clear_scsi_pt_obj(ptvp);
    set_scsi_pt_cdb(ptvp, rstmf_cdb, sizeof(rstmf_cdb));
    set_scsi_pt_sense(ptvp, sense_b, sizeof(sense_b));
    set_scsi_pt_data_in(ptvp, (uint8_t *)resp, mx_resp_len);
    res = do_scsi_pt(ptvp, -1, DEF_TIMEOUT_SECS, verbose);
    ret = sg_cmds_process_resp(ptvp, rstmf_s, res, noisy, verbose,
                               &sense_cat);
    if (-1 == ret) {
        if (get_scsi_pt_transport_err(ptvp))
            ret = SG_LIB_TRANSPORT_ERROR;
        else
            ret = sg_convert_errno(get_scsi_pt_os_err(ptvp));
    } else if (-2 == ret) {
        switch (sense_cat) {
        case SG_LIB_CAT_RECOVERED:
        case SG_LIB_CAT_NO_SENSE:
            ret = 0;
            break;
        default:
            ret = sense_cat;
            break;
        }
    } else {
        if (act_resp_lenp)
            *act_resp_lenp = ret;
        if ((verbose > 2) && (ret > 0)) {
            pr2serr("%s response:\n", rstmf_s);
            hex2stderr((const uint8_t *)resp, ret, 1);
        }
        ret = 0;
    }
    return ret;
}

/* Handles short options after '-j' including a sequence of short options
 * that include one 'j' (for JSON). Want optional argument to '-j' to be
 * prefixed by '='. Return 0 for good, SG_LIB_SYNTAX_ERROR for syntax error
 * and SG_LIB_OK_FALSE for exit with no error. */
static int
chk_short_opts(const char sopt_ch, struct opts_t * op)
{
    /* only need to process short, non-argument options */
    switch (sopt_ch) {
    case 'a':
        op->do_alpha = true;
        break;
    case 'c':
        op->do_compact = true;
        break;
    case 'e':
        op->do_enumerate = true;
        break;
    case 'h':
    case '?':
        ++op->do_help;
        break;
    case 'H':
        ++op->do_hex;
        break;
    case 'j':
        break;  /* simply ignore second 'j' (e.g. '-jxj') */
    case 'm':
        op->do_mask = true;
        break;
    case 'M':
        op->do_mlu = true;
        break;
    case 'n':
        op->no_inquiry = true;
        break;
    case 'N':
        break;      /* ignore */
    case 'O':
        op->opt_new = false;
        return 0;
    case 'q':
        op->do_repd = true;
        break;
    case 'r':
        op->do_raw = true;
        break;
    case 'R':
        op->do_rctd = true;
        break;
    case 't':
        op->do_taskman = true;
        break;
    case 'u':
        op->do_unsorted = true;
        break;
    case 'v':
        op->verbose_given = true;
        ++op->verbose;
        break;
    case 'V':
        op->version_given = true;
        break;
    default:
        pr2serr("unrecognised option code %c [0x%x] ??\n", sopt_ch, sopt_ch);
        return SG_LIB_SYNTAX_ERROR;
    }
    return 0;
}

static int
new_parse_cmd_line(struct opts_t * op, int argc, char * argv[])
{
    int c, n;
    char * cp;
    char b[32];

    while (1) {
        int option_index = 0;

        c = getopt_long(argc, argv, "^acehHi:j::J:mMnNo:Op:qQ:rRs:tuvV",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'a':
            op->do_alpha = true;
            break;
        case 'c':
            op->do_compact = true;
            break;
        case 'e':
            op->do_enumerate = true;
            break;
        case 'h':
        case '?':
            ++op->do_help;
            break;
        case 'H':
            ++op->do_hex;
            break;
        case 'i':
            op->inhex_fn = optarg;
            break;
        case 'j':       /* for: -j[=JO] */
        case '^':       /* for: --json[=JO] */
            op->do_json = true;
            /* Now want '=' to precede all JSON optional arguments */
            if (optarg) {
                int k, q;

                if ('^' == c) {
                    op->json_arg = optarg;
                    break;
                } else if ('=' == *optarg) {
                    op->json_arg = optarg + 1;
                    break;
                }
                n = strlen(optarg);
                for (k = 0; k < n; ++k) {
                    q = chk_short_opts(*(optarg + k), op);
                    if (SG_LIB_SYNTAX_ERROR == q)
                        return SG_LIB_SYNTAX_ERROR;
                    if (SG_LIB_OK_FALSE == q)
                        return 0;
                }
            } else
                op->json_arg = NULL;
            break;
        case 'J':
            op->do_json = true;
            op->js_file = optarg;
            break;
        case 'm':
            op->do_mask = true;
            break;
        case 'M':
            op->do_mlu = true;
            break;
        case 'n':
            op->no_inquiry = true;
            break;
        case 'N':
            break;      /* ignore */
        case 'o':
            if (strlen(optarg) >= (sizeof(b) - 1)) {
                pr2serr("argument to '--opcode' too long\n");
                return SG_LIB_SYNTAX_ERROR;
            }
            cp = strchr(optarg, ',');
            if (cp) {
                memset(b, 0, sizeof(b));
                strncpy(b, optarg, cp - optarg);
                n = sg_get_num(b);
                if ((n < 0) || (n > 255)) {
                    pr2serr("bad OP argument to '--opcode'\n");
                    return SG_LIB_SYNTAX_ERROR;
                }
                op->opcode = n;
                n = sg_get_num(cp + 1);
                if ((n < 0) || (n > 0xffff)) {
                    pr2serr("bad SA argument to '--opcode'\n");
                    usage();
                    return SG_LIB_SYNTAX_ERROR;
                }
                op->servact = n;
            } else {
                n = sg_get_num(optarg);
                if ((n < 0) || (n > 255)) {
                    pr2serr("bad argument to '--opcode'\n");
                    usage();
                    return SG_LIB_SYNTAX_ERROR;
                }
                op->opcode = n;
            }
            break;
        case 'O':
            op->opt_new = false;
            return 0;
        case 'p':
            if (isdigit((uint8_t)optarg[0]))
                n = sg_get_num(optarg);
            else if ((2 == strlen(optarg)) && (0 == strcmp("-1", optarg)))
                n = -1;
            else
                n = sg_get_pdt_from_acronym(optarg);
            if ((n < -1) || (n > PDT_MAX)) {
                if (-3 == n)        /* user asked for enueration */
                    return SG_LIB_OK_FALSE;
                pr2serr("bad argument to '--pdt=DT', expect -1 to 31 or "
                        "acronym\n");
                return SG_LIB_SYNTAX_ERROR;
            }
            peri_dtype = n;
            break;
        case 'q':
            op->do_repd = true;
            break;
        case 'Q':
            n = sg_get_num(optarg);
            if ((n < 0) || (n > 7)) {
                pr2serr("--rep-opts=RO expects a value between 0 and 7\n");
                return SG_LIB_SYNTAX_ERROR;
            }
            op->rep_opts = n;
            op->rep_opts_given = true;
            break;
        case 'r':
            op->do_raw = true;
            break;
        case 'R':
            op->do_rctd = true;
            break;
        case 's':
            n = sg_get_num(optarg);
            if ((n < 0) || (n > 0xffff)) {
                pr2serr("bad argument to '--sa'\n");
                usage();
                return SG_LIB_SYNTAX_ERROR;
            }
            op->servact = n;
            break;
        case 't':
            op->do_taskman = true;
            break;
        case 'u':
            op->do_unsorted = true;
            break;
        case 'v':
            op->verbose_given = true;
            ++op->verbose;
            break;
        case 'V':
            op->version_given = true;
            break;
        default:
            pr2serr("unrecognised option code %c [0x%x]\n", c, c);
            if (op->do_help)
                break;
            usage();
            return SG_LIB_SYNTAX_ERROR;
        }
    }
    if (optind < argc) {
        if (NULL == op->device_name) {
            op->device_name = argv[optind];
            ++optind;
        }
        if (optind < argc) {
            for (; optind < argc; ++optind)
                pr2serr("Unexpected extra argument: %s\n", argv[optind]);
            usage();
            return SG_LIB_SYNTAX_ERROR;
        }
    }
    return 0;
}

/* Processes command line options according to old option format. Returns
 * 0 is ok, else SG_LIB_SYNTAX_ERROR or similar is returned. Newer
 * functionality is not available via these old options, better to use
 * new options. */
static int
old_parse_cmd_line(struct opts_t * op, int argc, char * argv[])
{
    bool jmp_out;
    int k, plen, n, num;
    const char * cp;

    for (k = 1; k < argc; ++k) {
        cp = argv[k];
        plen = strlen(cp);
        if (plen <= 0)
            continue;
        if ('-' == *cp) {
            for (--plen, ++cp, jmp_out = false; plen > 0; --plen, ++cp) {
                switch (*cp) {
                case 'a':
                    op->do_alpha = true;
                    break;
                case 'c':
                    op->do_compact = true;
                    break;
                case 'e':
                    op->do_enumerate = true;
                    break;
                case 'H':
                    ++op->do_hex;
                    break;
                case 'j':    /* don't accept argument with this old syntax */
                    op->do_json = true;
                    break;
                case 'm':
                    op->do_mask = true;
                    break;
                case 'M':
                    op->do_mlu = true;
                    break;
                case 'n':
                    op->no_inquiry = true;
                    break;
                case 'N':
                    op->opt_new = true;
                    return 0;
                case 'O':
                    break;
                case 'q':
                    op->do_repd = true;
                    break;
                case 'r':
                    op->do_raw = true;
                    break;
                case 'R':
                    op->do_rctd = true;
                    break;
                case 't':
                    op->do_taskman = true;
                    break;
                case 'u':
                    op->do_unsorted = true;
                    break;
                case 'v':
                    op->verbose_given = true;
                    ++op->verbose;
                    break;
                case 'V':
                    op->version_given = true;
                    break;
                case 'h':
                case '?':
                    ++op->do_help;
                    break;
                default:
                    jmp_out = true;
                    break;
                }
                if (jmp_out)
                    break;
            }
            if (plen <= 0)
                continue;
            if (0 == strncmp("i=", cp, 2))
                op->inhex_fn = cp + 2;
            else if (0 == strncmp("o=", cp, 2)) {
                num = sscanf(cp + 2, "%x", (unsigned int *)&n);
                if ((1 != num) || (n > 255)) {
                    pr2serr("Bad number after 'o=' option\n");
                    usage_old();
                    return SG_LIB_SYNTAX_ERROR;
                }
                op->opcode = n;
            } else if (0 == strncmp("p=", cp, 2)) {
                num = sscanf(cp + 2, "%d", &n);
                if ((1 != num) || (n > PDT_MAX) || (n < -1)) {
                    pr2serr("Bad number after 'p=' option, expect -1 to "
                            "31\n");
                    return SG_LIB_SYNTAX_ERROR;
                }
                peri_dtype = n;
            } else if (0 == strncmp("s=", cp, 2)) {
                num = sscanf(cp + 2, "%x", (unsigned int *)&n);
                if (1 != num) {
                    pr2serr("Bad number after 's=' option\n");
                    usage_old();
                    return SG_LIB_SYNTAX_ERROR;
                }
                op->servact = n;
            } else if (0 == strncmp("-old", cp, 4))
                ;
            else if (jmp_out) {
                pr2serr("Unrecognized option: %s\n", cp);
                usage_old();
                return SG_LIB_SYNTAX_ERROR;
            }
        } else if (NULL == op->device_name)
            op->device_name = cp;
        else {
            pr2serr("too many arguments, got: %s, not expecting: %s\n",
                    op->device_name, cp);
            usage_old();
            return SG_LIB_SYNTAX_ERROR;
        }
    }
    return 0;
}

static int
parse_cmd_line(struct opts_t * op, int argc, char * argv[])
{
    int res;
    char * cp;

    cp = getenv("SG3_UTILS_OLD_OPTS");
    if (cp) {
        op->opt_new = false;
        res = old_parse_cmd_line(op, argc, argv);
        if ((0 == res) && op->opt_new)
            res = new_parse_cmd_line(op, argc, argv);
    } else {
        op->opt_new = true;
        res = new_parse_cmd_line(op, argc, argv);
        if ((0 == res) && (! op->opt_new))
            res = old_parse_cmd_line(op, argc, argv);
    }
    return res;
}

static void
dStrRaw(const char * str, int len)
{
    int k;

    for (k = 0; k < len; ++k)
        printf("%c", str[k]);
}

/* returns -1 when left < right, 0 when left == right, else returns 1 */
static int
opcode_num_compare(const void * left, const void * right)
{
    int l_serv_act = 0;
    int r_serv_act = 0;
    int l_opc, r_opc;
    const uint8_t * ll = *(uint8_t **)left;
    const uint8_t * rr = *(uint8_t **)right;

    if (NULL == ll)
        return -1;
    if (NULL == rr)
        return -1;
    l_opc = ll[0];
    if (ll[5] & 1)
        l_serv_act = sg_get_unaligned_be16(ll + 2);
    r_opc = rr[0];
    if (rr[5] & 1)
        r_serv_act = sg_get_unaligned_be16(rr + 2);
    if (l_opc < r_opc)
        return -1;
    if (l_opc > r_opc)
        return 1;
    if (l_serv_act < r_serv_act)
        return -1;
    if (l_serv_act > r_serv_act)
        return 1;
    return 0;
}

/* returns -1 when left < right, 0 when left == right, else returns 1 */
static int
opcode_alpha_compare(const void * left, const void * right)
{
    const uint8_t * ll = *(uint8_t **)left;
    const uint8_t * rr = *(uint8_t **)right;
    int l_serv_act = 0;
    int r_serv_act = 0;
    char l_name_buff[NAME_BUFF_SZ];
    char r_name_buff[NAME_BUFF_SZ];
    int l_opc, r_opc;

    if (NULL == ll)
        return -1;
    if (NULL == rr)
        return -1;
    l_opc = ll[0];
    if (ll[5] & 1)
        l_serv_act = sg_get_unaligned_be16(ll + 2);
    l_name_buff[0] = '\0';
    sg_get_opcode_sa_name(l_opc, l_serv_act, peri_dtype,
                          NAME_BUFF_SZ, l_name_buff);
    r_opc = rr[0];
    if (rr[5] & 1)
        r_serv_act = sg_get_unaligned_be16(rr + 2);
    r_name_buff[0] = '\0';
    sg_get_opcode_sa_name(r_opc, r_serv_act, peri_dtype,
                          NAME_BUFF_SZ, r_name_buff);
    return strncmp(l_name_buff, r_name_buff, NAME_BUFF_SZ);
}

/* For decoding a RSOC command's "All_commands" parameter data */
static int
list_all_codes(uint8_t * rsoc_buff, int rsoc_len, sgj_opaque_p jop,
               struct opts_t * op, struct sg_pt_base * ptvp)
{
    bool sa_v, ctdp;
    int k, j, m, n, cd_len, serv_act, len, bump, act_len, opcode, res;
    uint8_t byt5;
    unsigned int timeout;
    uint8_t * bp;
    uint8_t ** sort_arr = NULL;
    sgj_state * jsp = &op->json_st;
    sgj_opaque_p jap = NULL;
    sgj_opaque_p jo2p = NULL;
    sgj_opaque_p jo3p = NULL;
    char name_buff[NAME_BUFF_SZ];
    char sa_buff[8];
    char b[192];
    const int blen = sizeof(b);

    cd_len = sg_get_unaligned_be32(rsoc_buff + 0);
    if (cd_len > (rsoc_len - 4)) {
        sgj_pr_hr(jsp, "sg_opcodes: command data length=%d, allocation=%d; "
                   "truncate\n", cd_len, rsoc_len - 4);
        cd_len = ((rsoc_len - 4) / 8) * 8;
    }
    if (0 == cd_len) {
        sgj_pr_hr(jsp, "sg_opcodes: no commands to display\n");
        return 0;
    }
    if (op->do_rctd) {  /* Return command timeout descriptor */
        if (op->do_compact) {
            sgj_pr_hr(jsp, "\nOpcode,sa  Nominal  Recommended  Name\n");
            sgj_pr_hr(jsp,   "  (hex)    timeout  timeout(sec)     \n");
            sgj_pr_hr(jsp, "-----------------------------------------------"
                       "---------\n");
        } else {
            sgj_pr_hr(jsp, "\nOpcode  Service    CDB   Nominal  Recommended  "
                      "Name\n");
            sgj_pr_hr(jsp,   "(hex)   action(h)  size  timeout  timeout(sec) "
                      "    \n");
            sgj_pr_hr(jsp, "-------------------------------------------------"
                      "---------------\n");
        }
    } else {            /* RCTD clear in cdb */
        if (op->do_compact) {
            sgj_pr_hr(jsp, "\nOpcode,sa  Name\n");
            sgj_pr_hr(jsp,   "  (hex)        \n");
            sgj_pr_hr(jsp, "---------------------------------------\n");
        } else if (op->do_mlu) {
            sgj_pr_hr(jsp, "\nOpcode  Service    CDB    MLU    Name\n");
            sgj_pr_hr(jsp,   "(hex)   action(h)  size              \n");
            sgj_pr_hr(jsp, "-------------------------------------------"
                      "----\n");
        } else {
            sgj_pr_hr(jsp, "\nOpcode  Service    CDB  RWCDLP,  Name\n");
            sgj_pr_hr(jsp,   "(hex)   action(h)  size   CDLP       \n");
            sgj_pr_hr(jsp, "-------------------------------------------"
                      "----\n");
        }
    }
    /* SPC-4 does _not_ require any ordering of opcodes in the response */
    if (! op->do_unsorted) {
        sort_arr = (uint8_t **)calloc(cd_len, sizeof(uint8_t *));
        if (NULL == sort_arr) {
            pr2serr("sg_opcodes: no memory to sort operation codes, "
                    "try '-u'\n");
            return sg_convert_errno(ENOMEM);
        }
        memset(sort_arr, 0, cd_len * sizeof(uint8_t *));
        bp = rsoc_buff + 4;
        for (k = 0, j = 0; k < cd_len; ++j, k += len, bp += len) {
            sort_arr[j] = bp;
            len = (bp[5] & 0x2) ? 20 : 8;
        }
        qsort(sort_arr, j, sizeof(uint8_t *),
              (op->do_alpha ? opcode_alpha_compare : opcode_num_compare));
    }

    jap = sgj_named_subarray_r(jsp, jop, "command_descriptors");
    for (k = 0, j = 0; k < cd_len; ++j, k += bump) {
        jo2p = sgj_new_unattached_object_r(jsp);

        if (op->do_unsorted)
            bp = rsoc_buff + 4 + k;
        else if (sort_arr)
            bp = sort_arr[j];
        else {
            pr2serr("%s: logic error\n", __func__);
            return sg_convert_errno(EDOM);
        }
        byt5 = bp[5];
        ctdp = !! (byt5 & 0x2);
        bump = ctdp ? RSOC_ALL_BYTES_CTDP_1 : RSOC_ALL_BYTES_CTDP_0;
        opcode = bp[0];
        sa_v = !! (byt5 & 1);    /* service action valid */
        serv_act = 0;
        name_buff[0] = '\0';
        if (sa_v) {
            serv_act = sg_get_unaligned_be16(bp + 2);
            sg_get_opcode_sa_name(opcode, serv_act, peri_dtype, NAME_BUFF_SZ,
                                  name_buff);
            if (op->do_compact)
                snprintf(sa_buff, sizeof(sa_buff), "%-4x", serv_act);
            else
                snprintf(sa_buff, sizeof(sa_buff), "%4x", serv_act);
        } else {
            sg_get_opcode_name(opcode, peri_dtype, NAME_BUFF_SZ, name_buff);
            memset(sa_buff, ' ', sizeof(sa_buff));
        }
        if (op->do_rctd) {
            if (ctdp) {
                /* don't show CDLP because it makes line too long */
                if (op->do_compact)
                    n = sg_scnpr(b, blen, " %.2x%c%.4s", opcode,
                                 (sa_v ? ',' : ' '), sa_buff);
                else
                    n = sg_scnpr(b, blen, " %.2x     %.4s       %3d", opcode,
                                 sa_buff, sg_get_unaligned_be16(bp + 6));
                timeout = sg_get_unaligned_be32(bp + 12);
                if (0 == timeout)
                    n += sg_scn3pr(b, blen, n, "         -");
                else
                    n += sg_scn3pr(b, blen, n, "  %8u", timeout);
                timeout = sg_get_unaligned_be32(bp + 16);
                if (0 == timeout)
                    sg_scn3pr(b, blen, n, "          -");
                else
                    sg_scn3pr(b, blen, n, "   %8u", timeout);
                sgj_pr_hr(jsp, "%s    %s\n", b, name_buff);
            } else                      /* CTDP clear */
                if (op->do_compact)
                    sgj_pr_hr(jsp, " %.2x%c%.4s                        %s\n",
                              opcode, (sa_v ? ',' : ' '), sa_buff, name_buff);
                else
                    sgj_pr_hr(jsp, " %.2x     %.4s       %3d                 "
                              "        %s\n", opcode, sa_buff,
                              sg_get_unaligned_be16(bp + 6), name_buff);
        } else {            /* RCTD clear in cdb */
            /* before version 0.69 treated RWCDLP (1 bit) and CDLP (2 bits),
             * as a 3 bit field, now break them out separately */
            int rwcdlp = (byt5 >> 2) & 0x3;
            int cdlp = !!(0x40 & byt5);

            if (op->do_compact)
                sgj_pr_hr(jsp, " %.2x%c%.4s   %s\n", bp[0],
                          (sa_v ? ',' : ' '), sa_buff, name_buff);
            else if (op->do_mlu)
                sgj_pr_hr(jsp, " %.2x     %.4s       %3d   %3d     %s\n",
                          bp[0], sa_buff, sg_get_unaligned_be16(bp + 6),
                          ((byt5 >> 4) & 0x3), name_buff);
            else
                sgj_pr_hr(jsp, " %.2x     %.4s       %3d    %d,%d    %s\n",
                          bp[0], sa_buff, sg_get_unaligned_be16(bp + 6),
                          rwcdlp, cdlp, name_buff);
        }
        if (jsp->pr_as_json) {
            snprintf(b, blen, "0x%x", opcode);
            sgj_js_nv_s(jsp, jo2p, "operation_code", b);
            if (sa_v) {
                snprintf(b, blen, "0x%x", serv_act);
                sgj_js_nv_s(jsp, jo2p, "service_action", b);
            }
            if (name_buff[0])
                sgj_js_nv_s(jsp, jo2p, "name", name_buff);
            sgj_js_nv_i(jsp, jo2p, "rwcdlp", (byt5 >> 6) & 0x1);
            sgj_js_nv_i(jsp, jo2p, "mlu", (byt5 >> 4) & 0x3);
            sgj_js_nv_i(jsp, jo2p, "cdlp", (byt5 >> 2) & 0x3);
            sgj_js_nv_i(jsp, jo2p, "ctdp", (int)ctdp);
            sgj_js_nv_i(jsp, jo2p, "servactv", (int)sa_v);
            sgj_js_nv_i(jsp, jo2p, "cdb_length",
                        sg_get_unaligned_be16(bp + 6));

            sgj_js_nv_o(jsp, jap, NULL /* implies an array add */, jo2p);
        }

        if (op->do_mask && ptvp) {
            int cdb_sz;
            uint8_t d[64];

            n = 0;
            memset(d, 0, sizeof(d));
            res = do_rsoc(ptvp, false, (sa_v ? 2 : 1), opcode, serv_act,
                          d, sizeof(d), &act_len, true, op->verbose);
            if (0 == res) {
                int nn;

                cdb_sz = sg_get_unaligned_be16(d + 2);
                cdb_sz = (cdb_sz < act_len) ? cdb_sz : act_len;
                if ((cdb_sz > 0) && (cdb_sz <= 80)) {
                    if (op->do_compact)
                        n += sg_scn3pr(b, blen, n, "             usage: ");
                    else
                        n += sg_scn3pr(b, blen, n, "        cdb usage: ");
                    nn = n;
                    for (m = 0; (m < cdb_sz) && ((4 + m) < (int)sizeof(d));
                         ++m)
                        n += sg_scn3pr(b, blen, n, "%.2x ", d[4 + m]);
                    sgj_pr_hr(jsp, "%s\n", b);
                    if (jsp->pr_as_json) {
                        int l;
                        char *b2p = b + nn;
                        jo3p = sgj_named_subobject_r(jsp, jo2p, oc_pd_sn);

                        l = strlen(b2p);
                        if ((l > 0) && (' ' == b2p[l - 1]))
                            b2p[l - 1] = '\0';
                        sgj_js_nv_i(jsp, jo3p, "cdb_size", cdb_sz);
                        sgj_js_nv_s(jsp, jo3p, "cdb_usage_data", b2p);
                    }
                }
            } else
                goto err_out;
        }
    }                   /* <<<<<< end of loop over all supported commands */
    res = 0;
err_out:
    if (sort_arr)
        free(sort_arr);
    return res;
}

static void
decode_cmd_timeout_desc(uint8_t * dp, int max_b_len, char * b,
                        struct opts_t * op)
{
    int len;
    unsigned int timeout;
    sgj_state * jsp = &op->json_st;

    if ((max_b_len < 2) || (NULL == dp))
        return;
    b[max_b_len - 1] = '\0';
    --max_b_len;
    len = sg_get_unaligned_be16(dp + 0);
    if (10 != len) {
        snprintf(b, max_b_len, "command timeout descriptor length %d "
                 "(expect 10)", len);
        return;
    }
    timeout = sg_get_unaligned_be32(dp + 4);
    if (0 == timeout)
        snprintf(b, max_b_len, "no nominal timeout, ");
    else
        snprintf(b, max_b_len, "nominal timeout: %u secs, ", timeout);
    if (jsp->pr_as_json) {
        sgj_js_nv_i(jsp, jsp->userp, "command_specific", dp[3]);
        sgj_js_nv_i(jsp, jsp->userp, "nominal_command_processing_timeout",
                    timeout);
    }
    len = strlen(b);
    max_b_len -= len;
    b += len;
    timeout = sg_get_unaligned_be32(dp + 8);
    if (0 == timeout)
        snprintf(b, max_b_len, "no recommended timeout");
    else
        snprintf(b, max_b_len, "recommended timeout: %u secs", timeout);
    if (jsp->pr_as_json)
        sgj_js_nv_i(jsp, jsp->userp, "recommended_command_timeout", timeout);
    return;
}

/* For decoding a RSOC command's "One_command" parameter data which includes
 * cdb usage data. */
static void
list_one(uint8_t * rsoc_buff, int cd_len, int rep_opts, sgj_opaque_p jop,
         struct opts_t * op)
{
    bool valid = false;
    int k, mlu, cdlp, rwcdlp, support, ctdp;
    int n;
    uint8_t * bp;
    const char * cp;
    const char * dlp;
    const char * mlu_p;
    sgj_state * jsp = &op->json_st;
    char name_buff[NAME_BUFF_SZ];
    char d[64];
    char b[192];
    const int blen = sizeof(b);


    n = sg_scnpr(b, blen, "\n  Opcode=0x%.2x", op->opcode);
    if (rep_opts > 1)
        sg_scn3pr(b, blen, n, "  Service_action=0x%.4x", op->servact);
    sgj_pr_hr(jsp, "%s\n", b);
    sg_get_opcode_sa_name(((op->opcode > 0) ? op->opcode : 0),
                          ((op->servact > 0) ? op->servact : 0),
                          peri_dtype, NAME_BUFF_SZ, name_buff);
    sgj_pr_hr(jsp, "  Command_name: %s\n", name_buff);
    ctdp = !!(0x80 & rsoc_buff[1]);
    support = rsoc_buff[1] & 7;
    switch(support) {
    case 0:
        cp = "not currently available";
        break;
    case 1:
        cp = "NOT supported";
        break;
    case 3:
        cp = "supported [conforming to SCSI standard]";
        valid = true;
        break;
    case 5:
        cp = "supported [in a vendor specific manner]";
        valid = true;
        break;
    default:
        snprintf(name_buff, NAME_BUFF_SZ, "support reserved [0x%x]",
                 rsoc_buff[1] & 7);
        cp = name_buff;
        break;
    }
    cdlp = 0x3 & (rsoc_buff[1] >> 3);
    rwcdlp = rsoc_buff[0] & 1;
    switch (cdlp) {
    case 0:
        if (rwcdlp)
            dlp = "Reserved [RWCDLP=1, CDLP=0]";
        else
            dlp = "No command duration limit mode page";
        break;
    case 1:
        if (rwcdlp)
            dlp = "Command duration limit T2A mode page";
        else
            dlp = "Command duration limit A mode page";
        break;
    case 2:
        if (rwcdlp)
            dlp = "Command duration limit T2B mode page";
        else
            dlp = "Command duration limit B mode page";
        break;
    default:
        dlp = "reserved [CDLP=3]";
        break;
    }
    sgj_pr_hr(jsp, "  Command is %s\n", cp);
    sgj_pr_hr(jsp, "  %s\n", dlp);
    mlu = 0x3 & (rsoc_buff[1] >> 5);
    switch (mlu) {
    case 0:
        mlu_p = "not reported";
        break;
    case 1:
        mlu_p = "affects only this logical unit";
        break;
    case 2:
        mlu_p = "affects more than 1, but not all LUs in this target";
        break;
    case 3:
        mlu_p = "affects all LUs in this target";
        break;
    default:
        snprintf(d, sizeof(d), "reserved [MLU=%d]", mlu);
        mlu_p = d;
        break;
    }
    sgj_pr_hr(jsp, "  Multiple Logical Units (MLU): %s\n", mlu_p);
    if (valid) {
        n = sg_scnpr(b, blen, "  Usage data: ");
        bp = rsoc_buff + 4;
        for (k = 0; k < cd_len; ++k)
            n += sg_scn3pr(b, blen, n, "%.2x ", bp[k]);
        sgj_pr_hr(jsp, "%s\n", b);
    }
    if (jsp->pr_as_json) {
        int l;

        snprintf(b, blen, "0x%x", op->opcode);
        sgj_js_nv_s(jsp, jop, "operation_code", b);
        if (op->rep_opts > 1) {
            snprintf(b, blen, "0x%x", op->servact);
            sgj_js_nv_s(jsp, jop, "service_action", b);
        }
        sgj_js_nv_i(jsp, jop, "rwcdlp", rwcdlp);
        sgj_js_nv_i(jsp, jop, "ctdp", ctdp);
        sgj_js_nv_i(jsp, jop, "mlu", mlu);
        sgj_js_nv_i(jsp, jop, "cdlp", cdlp);
        sgj_js_nv_i(jsp, jop, "support", support);
        sgj_js_nv_s(jsp, jop, "support_str", cp);
        sgj_js_nv_i(jsp, jop, "cdb_size", cd_len);
        n = 0;
        for (k = 0; k < cd_len; ++k)
            n += sg_scn3pr(b, blen, n, "%.2x ", rsoc_buff[k + 4]);
        l = strlen(b);
        if ((l > 0) && (' ' == b[l - 1]))
            b[l - 1] = '\0';
        sgj_js_nv_s(jsp, jop, "cdb_usage_data", b);
    }
    if (ctdp) {
        jsp->userp = sgj_named_subobject_r(jsp, jsp->basep,
                                           "command_timeouts_descriptor");
        bp = rsoc_buff + 4 + cd_len;
        decode_cmd_timeout_desc(bp, NAME_BUFF_SZ, name_buff, op);
        sgj_pr_hr(jsp, "  %s\n", name_buff);
    }
}


int
main(int argc, char * argv[])
{
    bool as_json;
    int cd_len, res, len, act_len, rq_len, in_len, vb;
    int sg_fd = -1;
    const char * cp;
    struct opts_t * op;
    const char * op_name;
    uint8_t * rsoc_buff = NULL;
    uint8_t * free_rsoc_buff = NULL;
    struct sg_pt_base * ptvp = NULL;
    sgj_state * jsp;
    sgj_opaque_p jop = NULL;
    sgj_opaque_p jo2p = NULL;
    char buff[48];
    char b[80];
    struct sg_simple_inquiry_resp inq_resp;
    struct opts_t opts;

    op = &opts;
    memset(op, 0, sizeof(opts));
    if (getenv("SG3_UTILS_INVOCATION"))
        sg_rep_invocation(MY_NAME, version_str, argc, argv, stderr);
    op->opcode = -1;
    op->servact = -1;
    op->rep_opts = -1;
    res = parse_cmd_line(op, argc, argv);
    if (res) {
        if (SG_LIB_OK_FALSE == res)
            return 0;
        else
            return res;
    }
    if (op->do_help) {
        if (op->opt_new)
            usage();
        else
            usage_old();
        return 0;
    }
    jsp = &op->json_st;
    if (op->do_json && (! op->do_enumerate)) {
        if (! sgj_init_state(jsp, op->json_arg)) {
            int bad_char = jsp->first_bad_char;
            char e[1500];

            if (bad_char) {
                pr2serr("bad argument to --json= option, unrecognized "
                        "character '%c'\n\n", bad_char);
            }
            sg_json_usage(0, e, sizeof(e));
            pr2serr("%s", e);
            return SG_LIB_SYNTAX_ERROR;
        }
        jop = sgj_start_r(MY_NAME, version_str, argc, argv, jsp);
    }
    as_json = jsp->pr_as_json;
#ifdef DEBUG
    pr2serr("In DEBUG mode, ");
    if (op->verbose_given && op->version_given) {
        pr2serr("but override: '-vV' given, zero verbose and continue\n");
        op->verbose_given = false;
        op->version_given = false;
        op->verbose = 0;
    } else if (! op->verbose_given) {
        pr2serr("set '-vv'\n");
        op->verbose = 2;
    } else
        pr2serr("keep verbose=%d\n", op->verbose);
#else
    if (op->verbose_given && op->version_given)
        pr2serr("Not in DEBUG mode, so '-vV' has no special action\n");
#endif
    if (op->version_given) {
        pr2serr("Version string: %s\n", version_str);
        goto fini;
    }
    vb = op->verbose;
    if (op->do_enumerate) {
        char name_buff[NAME_BUFF_SZ];

        if (op->do_taskman)
            printf("enumerate not supported with task management "
                   "functions\n");
        else {  /* SCSI command */
            if (op->opcode < 0)
                op->opcode = 0;
            if (op->servact < 0)
                op->servact = 0;
            if (peri_dtype < 0)
                peri_dtype = 0;
            printf("SCSI command:");
            if (vb)
                printf(" [opcode=0x%x, sa=0x%x, pdt=0x%x]\n", op->opcode,
                       op->servact, peri_dtype);
            else
                printf("\n");
            sg_get_opcode_sa_name(op->opcode, op->servact, peri_dtype,
                                  NAME_BUFF_SZ, name_buff);
            printf("  %s\n", name_buff);
        }
        goto fini;
    } else if (op->inhex_fn) {
        if (op->device_name) {
            if (! as_json)
                pr2serr("ignoring DEVICE, best to give DEVICE or "
                        "--inhex=FN, but not both\n");
            op->device_name = NULL;
        }
    } else if (NULL == op->device_name) {
        pr2serr("No DEVICE argument given\n\n");
        if (op->opt_new)
            usage();
        else
            usage_old();
        res = SG_LIB_SYNTAX_ERROR;
        goto err_out;
    }
    if ((-1 != op->servact) && (-1 == op->opcode)) {
        pr2serr("When '-s' is chosen, so must '-o' be chosen\n");
        if (op->opt_new)
            usage();
        else
            usage_old();
        res = SG_LIB_CONTRADICT;
        goto err_out;
    }
    if (op->do_unsorted && op->do_alpha)
        pr2serr("warning: unsorted ('-u') and alpha ('-a') options chosen, "
                "ignoring alpha\n");
    if (op->do_taskman && ((-1 != op->opcode) || op->do_alpha ||
        op->do_unsorted)) {
        pr2serr("warning: task management functions ('-t') chosen so alpha "
                "('-a'),\n          unsorted ('-u') and opcode ('-o') "
                "options ignored\n");
    }
    op_name = op->do_taskman ? "Report supported task management functions" :
              "Report supported operation codes";

    rsoc_buff = (uint8_t *)sg_memalign(MX_ALLOC_LEN, 0, &free_rsoc_buff,
                                       false);
    if (NULL == rsoc_buff) {
        pr2serr("Unable to allocate memory\n");
        res = sg_convert_errno(ENOMEM);
        no_final_msg = true;
        goto err_out;
    }

    if (! op->rep_opts_given) {
        if (op->opcode >= 0)
            op->rep_opts = ((op->servact >= 0) ? 2 : 1);
        else
            op->rep_opts = 0;
    } else if (op->opcode < 0)
        op->opcode = 0;

    if (op->inhex_fn) {
        if ((res = sg_f2hex_arr(op->inhex_fn, op->do_raw, false, rsoc_buff,
                                &in_len, MX_ALLOC_LEN))) {
            if (SG_LIB_LBA_OUT_OF_RANGE == res)
                pr2serr("decode buffer [%d] not large enough??\n",
                        MX_ALLOC_LEN);
            goto err_out;
        }
        if (op->verbose > 2)
            pr2serr("Read %d [0x%x] bytes of user supplied data\n",
                    in_len, in_len);
        if (op->do_raw)
            op->do_raw = false;    /* can interfere on decode */
        if (in_len < 4) {
            pr2serr("--inhex=%s only decoded %d bytes (needs 4 at "
                    "least)\n", op->inhex_fn, in_len);
            res = SG_LIB_SYNTAX_ERROR;
            goto err_out;
        }
        act_len = in_len;
        goto start_response;
    }
    if (op->opcode < 0) {
        /* Try to open read-only */
        if ((sg_fd = scsi_pt_open_device(op->device_name, true, vb)) < 0) {
            int err = -sg_fd;

            if (op->verbose)
                pr2serr("sg_opcodes: error opening file (ro): %s: %s\n",
                        op->device_name, safe_strerror(err));
#ifndef SG_LIB_WIN32
            if (ENOENT == err) {
                /* file or directory in the file's path doesn't exist, no
                 * point in retrying with read-write flag */
                res = sg_convert_errno(err);
                goto err_out;
            }
#endif
            goto open_rw;
        }
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, op->verbose);
        if (NULL == ptvp) {
            pr2serr("Out of memory (ro)\n");
            res = sg_convert_errno(ENOMEM);
            no_final_msg = true;
            goto err_out;
        }
        if (op->no_inquiry && (! op->do_taskman) && (peri_dtype < 0))
            pr2serr("--no-inquiry ignored because --pdt= not given\n");
        if (op->no_inquiry && (op->do_taskman || (peri_dtype >= 0)))
            ;
        else if (0 == sg_simple_inquiry_pt(ptvp, &inq_resp, true, vb)) {
            peri_dtype = inq_resp.peripheral_type;
            if (! (as_json || op->do_raw || op->no_inquiry ||
                   (op->do_hex > 2))) {
                printf("  %.8s  %.16s  %.4s\n", inq_resp.vendor,
                       inq_resp.product, inq_resp.revision);
                cp = sg_get_pdt_str(peri_dtype, sizeof(buff), buff);
                if (strlen(cp) > 0)
                    printf("  Peripheral device type: %s\n", cp);
                else
                    printf("  Peripheral device type: 0x%x\n", peri_dtype);
            }
        } else {
            pr2serr("sg_opcodes: %s doesn't respond to a SCSI INQUIRY\n",
                    op->device_name);
            res = SG_LIB_CAT_OTHER;
            no_final_msg = true;
            goto err_out;
        }
    }

open_rw:                /* if not already open */
    if (sg_fd < 0) {
        sg_fd = scsi_pt_open_device(op->device_name, false /* RW */, vb);
        if (sg_fd < 0) {
            pr2serr("sg_opcodes: error opening file (rw): %s: %s\n",
                    op->device_name, safe_strerror(-sg_fd));
            res = sg_convert_errno(-sg_fd);
            no_final_msg = true;
            goto err_out;
        }
        ptvp = construct_scsi_pt_obj_with_fd(sg_fd, op->verbose);
        if (NULL == ptvp) {
            pr2serr("Out of memory (rw)\n");
            res = sg_convert_errno(ENOMEM);
            no_final_msg = true;
            goto err_out;
        }
    }
    if (op->do_taskman) {
        rq_len = (op->do_repd ? 16 : 4);
        res = do_rstmf(ptvp, op->do_repd, rsoc_buff, rq_len, &act_len, true,
                       vb);
    } else {
        rq_len = MX_ALLOC_LEN;
        res = do_rsoc(ptvp, op->do_rctd, op->rep_opts, op->opcode,
                      op->servact, rsoc_buff, rq_len, &act_len, true, vb);
    }
    if (res) {
        sg_get_category_sense_str(res, sizeof(b), b, vb);
        pr2serr("%s: %s\n", op_name, b);
        no_final_msg = true;
        if ((0 == op->servact) && (op->opcode >= 0))
            pr2serr("    >> perhaps try again without a service action "
                    "[SA] of 0\n");
        goto err_out;
    }
    act_len = (rq_len < act_len) ? rq_len : act_len;

start_response:
    if (act_len < 4) {
        pr2serr("Actual length of response [%d] is too small\n", act_len);
        res = SG_LIB_CAT_OTHER;
        no_final_msg = true;
        goto err_out;
    }
    if (op->do_taskman) {
        if (op->do_raw) {
            dStrRaw((const char *)rsoc_buff, act_len);
            goto fini;
        }
        if (op->do_hex) {
            if (op->do_hex > 2)
                hex2stdout(rsoc_buff, act_len, -1);
            else  {
                printf("\nTask Management Functions supported by device:\n");
                if (2 == op->do_hex)
                    hex2stdout(rsoc_buff, act_len, 0);
                else
                    hex2stdout(rsoc_buff, act_len, 1);
            }
            goto fini;
        }
        if (jsp->pr_as_json) {
            if (op->do_repd)
                jo2p = sgj_named_subobject_r(jsp, jop, rstmf_e_pd_sn);
            else
                jo2p = sgj_named_subobject_r(jsp, jop, rstmf_b_pd_sn);
            sgj_js_nv_b(jsp, jo2p, "ats", rsoc_buff[0] & 0x80);
            sgj_js_nv_b(jsp, jo2p, "atss", rsoc_buff[0] & 0x40);
            sgj_js_nv_b(jsp, jo2p, "cacas", rsoc_buff[0] & 0x20);
            sgj_js_nv_b(jsp, jo2p, "ctss", rsoc_buff[0] & 0x10);
            sgj_js_nv_b(jsp, jo2p, "lurs", rsoc_buff[0] & 0x8);
            sgj_js_nv_b(jsp, jo2p, "qts", rsoc_buff[0] & 0x4);
            sgj_js_nv_b(jsp, jo2p, "trs", rsoc_buff[0] & 0x2);
            sgj_js_nv_b(jsp, jo2p, "ws", rsoc_buff[0] & 0x1);
            sgj_js_nv_b(jsp, jo2p, "qaes", rsoc_buff[1] & 0x4);
            sgj_js_nv_b(jsp, jo2p, "qtss", rsoc_buff[1] & 0x2);
            sgj_js_nv_b(jsp, jo2p, "itnrs", rsoc_buff[1] & 0x1);
            if (! jsp->pr_out_hr)
                goto fini;
        }
        sgj_pr_hr(jsp, "\nTask Management Functions supported by device:\n");
        if (rsoc_buff[0] & 0x80)
            sgj_pr_hr(jsp, "    Abort task\n");
        if (rsoc_buff[0] & 0x40)
            sgj_pr_hr(jsp, "    Abort task set\n");
        if (rsoc_buff[0] & 0x20)
            sgj_pr_hr(jsp, "    Clear ACA\n");
        if (rsoc_buff[0] & 0x10)
            sgj_pr_hr(jsp, "    Clear task set\n");
        if (rsoc_buff[0] & 0x8)
            sgj_pr_hr(jsp, "    Logical unit reset\n");
        if (rsoc_buff[0] & 0x4)
            sgj_pr_hr(jsp, "    Query task\n");
        if (rsoc_buff[0] & 0x2)
            sgj_pr_hr(jsp, "    Target reset (obsolete)\n");
        if (rsoc_buff[0] & 0x1)
            sgj_pr_hr(jsp, "    Wakeup (obsolete)\n");
        if (rsoc_buff[1] & 0x4)
            sgj_pr_hr(jsp, "    Query asynchronous event\n");
        if (rsoc_buff[1] & 0x2)
            sgj_pr_hr(jsp, "    Query task set\n");
        if (rsoc_buff[1] & 0x1)
            sgj_pr_hr(jsp, "    I_T nexus reset\n");
        if (op->do_repd) {
            if (rsoc_buff[3] < 0xc) {
                pr2serr("when REPD given, byte 3 of response should be >= "
                        "12\n");
                res = SG_LIB_CAT_OTHER;
                no_final_msg = true;
                goto err_out;
            } else
                sgj_pr_hr(jsp, "  Extended parameter data:\n");
            sgj_pr_hr(jsp, "    TMFTMOV=%d\n", !!(rsoc_buff[4] & 0x1));
            sgj_pr_hr(jsp, "    ATTS=%d\n", !!(rsoc_buff[6] & 0x80));
            sgj_pr_hr(jsp, "    ATSTS=%d\n", !!(rsoc_buff[6] & 0x40));
            sgj_pr_hr(jsp, "    CACATS=%d\n", !!(rsoc_buff[6] & 0x20));
            sgj_pr_hr(jsp, "    CTSTS=%d\n", !!(rsoc_buff[6] & 0x10));
            sgj_pr_hr(jsp, "    LURTS=%d\n", !!(rsoc_buff[6] & 0x8));
            sgj_pr_hr(jsp, "    QTTS=%d\n", !!(rsoc_buff[6] & 0x4));
            sgj_pr_hr(jsp, "    QAETS=%d\n", !!(rsoc_buff[7] & 0x4));
            sgj_pr_hr(jsp, "    QTSTS=%d\n", !!(rsoc_buff[7] & 0x2));
            sgj_pr_hr(jsp, "    ITNRTS=%d\n", !!(rsoc_buff[7] & 0x1));
            sgj_pr_hr(jsp, "    tmf long timeout: %u (100 ms units)\n",
                      sg_get_unaligned_be32(rsoc_buff + 8));
            sgj_pr_hr(jsp, "    tmf short timeout: %u (100 ms units)\n",
                      sg_get_unaligned_be32(rsoc_buff + 12));
        }
    } else if (0 == op->rep_opts) {  /* list all supported operation codes */
        len = sg_get_unaligned_be32(rsoc_buff + 0) + 4;
        len = (len < act_len) ? len : act_len;
        if (op->do_raw) {
            dStrRaw((const char *)rsoc_buff, len);
            goto fini;
        }
        if (op->do_hex) {
            if (op->do_hex > 2)
                hex2stdout(rsoc_buff, len, -1);
            else if (2 == op->do_hex)
                hex2stdout(rsoc_buff, len, 0);
            else
                hex2stdout(rsoc_buff, len, 1);
            goto fini;
        }
        jo2p = sgj_named_subobject_r(jsp, jop, ac_pd_sn);
        list_all_codes(rsoc_buff, len, jo2p, op, ptvp);
    } else {    /* asked about specific command */
        cd_len = sg_get_unaligned_be16(rsoc_buff + 2);
        len = cd_len + 4;
        len = (len < act_len) ? len : act_len;
        cd_len = (cd_len < act_len) ? cd_len : act_len;
        if (op->do_raw) {
            dStrRaw((const char *)rsoc_buff, len);
            goto fini;
        }
        if (op->do_hex) {
            if (op->do_hex > 2)
                hex2stdout(rsoc_buff, len, -1);
            else if (2 == op->do_hex)
                hex2stdout(rsoc_buff, len, 0);
            else
                hex2stdout(rsoc_buff, len, 1);
            goto fini;
        }
        jo2p = sgj_named_subobject_r(jsp, jop, oc_pd_sn);
        list_one(rsoc_buff, cd_len, op->rep_opts, jo2p, op);
    }
fini:
    res = 0;

err_out:
    if (free_rsoc_buff)
        free(free_rsoc_buff);
    if (! op->inhex_fn) {
        if (ptvp)
            destruct_scsi_pt_obj(ptvp);
        if (sg_fd >= 0)
            scsi_pt_close_device(sg_fd);
    }
    if ((0 == op->verbose) && (! no_final_msg)) {
        if (! sg_if_can2stderr("sg_opcodes failed: ", res))
            pr2serr("Some error occurred, try again with '-v' "
                    "or '-vv' for more information\n");
    }
    res = (res >= 0) ? res : SG_LIB_CAT_OTHER;
    if (as_json && (! op->do_enumerate)) {
        FILE * fp = stdout;

        if (op->js_file) {
            if ((1 != strlen(op->js_file)) || ('-' != op->js_file[0])) {
                fp = fopen(op->js_file, "w");   /* truncate if exists */
                if (NULL == fp) {
                    pr2serr("unable to open file: %s\n", op->js_file);
                    res = SG_LIB_FILE_ERROR;
                }
            }
            /* '--js-file=-' will send JSON output to stdout */
        }
        if (fp)
            sgj_js2file(jsp, NULL, res, fp);
        if (op->js_file && fp && (stdout != fp))
            fclose(fp);
        sgj_finish(jsp);
    }
    return res;
}
