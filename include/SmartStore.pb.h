/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.5-dev at Wed Jan 27 11:02:47 2016. */

#ifndef PB_SMARTSTORE_PB_H_INCLUDED
#define PB_SMARTSTORE_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _Dev {
    bool has_ieee;
    char ieee[17];
    bool has_nwk_addr;
    char nwk_addr[5];
    bool has_node_name;
    char node_name[31];
    bool has_online;
    int32_t online;
    bool has_node_status;
    int32_t node_status;
    pb_callback_t eps;
} Dev;

typedef struct _DevInfo {
    bool has_manufacture;
    char manufacture[21];
    bool has_model;
    char model[21];
    bool has_sn;
    char sn[17];
    bool has_mac;
    char mac[13];
    bool has_sw_ver;
    char sw_ver[9];
    bool has_hw_ver;
    char hw_ver[9];
} DevInfo;

typedef struct _EPS {
    bool has_profile_id;
    char profile_id[6];
    bool has_device_id;
    char device_id[6];
    bool has_zone_type;
    char zone_type[6];
    bool has_ep;
    char ep[3];
    bool has_ep_name;
    char ep_name[31];
    bool has_rid;
    int32_t rid;
    bool has_arm;
    int32_t arm;
} EPS;

typedef struct _SSMsg {
    int32_t msgtype;
    bool has_result;
    int32_t result;
    bool has_devinfo;
    DevInfo devinfo;
    bool has_name;
    char name[31];
    bool has_status;
    int32_t status;
    pb_callback_t devlist;
    bool has_nwk_addr;
    char nwk_addr[5];
    bool has_ep;
    char ep[3];
    bool has_on_off;
    int32_t on_off;
    bool has_dev;
    Dev dev;
    bool has_date_time;
    int32_t date_time;
    bool has_ep_name;
    char ep_name[31];
    bool has_arm;
    int32_t arm;
    bool has_w_mode;
    int32_t w_mode;
    bool has_ieee;
    char ieee[17];
    bool has_device_id;
    char device_id[6];
    bool has_zone_type;
    char zone_type[6];
} SSMsg;

/* Default values for struct fields */

/* Initializer values for message structs */
#define SSMsg_init_default                       {0, false, 0, false, DevInfo_init_default, false, "", false, 0, {{NULL}, NULL}, false, "", false, "", false, 0, false, Dev_init_default, false, 0, false, "", false, 0, false, 0, false, "", false, "", false, ""}
#define Dev_init_default                         {false, "", false, "", false, "", false, 0, false, 0, {{NULL}, NULL}}
#define DevInfo_init_default                     {false, "", false, "", false, "", false, "", false, "", false, ""}
#define EPS_init_default                         {false, "", false, "", false, "", false, "", false, "", false, 0, false, 0}
#define SSMsg_init_zero                          {0, false, 0, false, DevInfo_init_zero, false, "", false, 0, {{NULL}, NULL}, false, "", false, "", false, 0, false, Dev_init_zero, false, 0, false, "", false, 0, false, 0, false, "", false, "", false, ""}
#define Dev_init_zero                            {false, "", false, "", false, "", false, 0, false, 0, {{NULL}, NULL}}
#define DevInfo_init_zero                        {false, "", false, "", false, "", false, "", false, "", false, ""}
#define EPS_init_zero                            {false, "", false, "", false, "", false, "", false, "", false, 0, false, 0}

/* Field tags (for use in manual encoding/decoding) */
#define Dev_ieee_tag                             1
#define Dev_nwk_addr_tag                         2
#define Dev_node_name_tag                        3
#define Dev_online_tag                           4
#define Dev_node_status_tag                      5
#define Dev_eps_tag                              6
#define DevInfo_manufacture_tag                  1
#define DevInfo_model_tag                        2
#define DevInfo_sn_tag                           3
#define DevInfo_mac_tag                          4
#define DevInfo_sw_ver_tag                       5
#define DevInfo_hw_ver_tag                       6
#define EPS_profile_id_tag                       1
#define EPS_device_id_tag                        2
#define EPS_zone_type_tag                        3
#define EPS_ep_tag                               4
#define EPS_ep_name_tag                          5
#define EPS_rid_tag                              6
#define EPS_arm_tag                              7
#define SSMsg_msgtype_tag                        1
#define SSMsg_result_tag                         2
#define SSMsg_devinfo_tag                        3
#define SSMsg_name_tag                           4
#define SSMsg_status_tag                         5
#define SSMsg_devlist_tag                        6
#define SSMsg_nwk_addr_tag                       7
#define SSMsg_ep_tag                             8
#define SSMsg_on_off_tag                         9
#define SSMsg_dev_tag                            10
#define SSMsg_date_time_tag                      11
#define SSMsg_ep_name_tag                        12
#define SSMsg_arm_tag                            13
#define SSMsg_w_mode_tag                         14
#define SSMsg_ieee_tag                           15
#define SSMsg_device_id_tag                      16
#define SSMsg_zone_type_tag                      17

/* Struct field encoding specification for nanopb */
extern const pb_field_t SSMsg_fields[18];
extern const pb_field_t Dev_fields[7];
extern const pb_field_t DevInfo_fields[7];
extern const pb_field_t EPS_fields[8];

/* Maximum encoded size of messages (where known) */
#define DevInfo_size                             102
#define EPS_size                                 84

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define SMARTSTORE_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif