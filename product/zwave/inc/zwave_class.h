#ifndef __ZWAVE_CLASS_H_
#define	__ZWAVE_CLASS_H_


int zwave_class_version_get(int id, char class);

/* powerlevel */
int zwave_cc1_powerlevel_get();
int zwave_cc1_powerlevel_rpt(char *param, int size,  char *power_level);
int zwave_cc1_powerlevel_set(char power_level);
int zwave_cc1_powerlevel_test_node_get();
int zwave_cc1_powerlevel_test_node_rpt(char *param, int size, char *test_node_id, char *test_result, short *test_frame_count);
int zwave_cc1_powerlevel_test_node_set(char test_node_id, char power_level, short test_frame_count);

/* switch bianry */
int zwave_cc1_switch_binary_get();
int zwave_cc1_switch_binary_rpt(char *param, int size, char *value);
int zwave_cc1_switch_binary_set(char value);

/* zwaveplus info */
int zwave_cc1_zwaveplus_info_get();
int zwave_cc1_zwaveplus_info_rpt(char *param, int size, char *zplus_version, char *role_type, char *node_type);

int zwave_cc2_zwaveplus_info_get();
int zwave_cc2_zwaveplus_info_rpt(char *param, int size, char *zplus_version, char *role_type, char *node_type, 
																short *ins_icon_type, short *usr_icon_type);

/* association */
int zwave_cc1_association_get(char group_id);
int zwave_cc1_association_groupings_get();
int zwave_cc1_association_groupings_rpt(char *param, int size, char *supported_groups);
int zwave_cc1_association_rmv(char group_id, char *node_ids, int node_num);
int zwave_cc1_association_rpt(char *param, int size, char *group_id, char *max_nodes_supported, char *reports_to_follow, char *node_ids, int *node_num);

int zwave_cc2_association_get(char group_id);
int zwave_cc2_association_groupings_get();
int zwave_cc2_association_groupings_rpt(char *param, int size, char *supported_groups);
int zwave_cc2_association_rmv(char group_id, char *node_ids, int node_num);
int zwave_cc2_association_rpt(char *param, int size, char *group_id, char *max_nodes_supported, char *reports_to_follow, char *node_ids, int *node_num);
int zwave_cc2_association_set(char group_id, char *node_ids, int node_num);
int zwave_cc2_association_specific_group_get();
int zwave_cc2_association_specific_group_rpt(char *group);

/* version */
int zwave_cc1_version_class_get(char class);
int zwave_cc1_version_class_rpt(char *param, int size,char *class, char *version);
int zwave_cc1_version_get();
int zwave_cc1_version_rpt(char *param, int size,char *zlib_type, char *zproto_version, char *zproto_sub_type, char *app_vesion, char *app_sub_version);


int zwave_cc2_version_class_get(char class);
int zwave_cc2_version_class_rpt(char *param, int size,char *class, char *version);
int zwave_cc2_version_get();
//int zwave_cc2_version_rpt(char *param, int size,char *zlib_type, char *zproto_version, char *zproto_sub_type, char *firmware_0_version, char *firmware_0_sub_version,
//													char *hw_version, char *num_firmware_targets, stFirmwareVersion_t *firmware_versions, int firmware_version_cnt);


/* manufacturer */
int zwave_cc1_manufacturer_specific_get();
int zwave_cc1_manufacturer_specific_rpt(char *param, int size,short *manufacturer_id, short *product_type_id, short *product_id);

int zwave_cc2_manufacturer_specific_get();
int zwave_cc2_manufacturer_specific_rpt(char *param, int size,short *manufacturer_id, short *product_type_id, short *product_id);
int zwave_cc2_device_specific_get(char properties1);
int zwave_cc2_device_specific_rpt(char *param, int size,char *properties1, int *device_id_data_len, char *device_id_data_fmt, char *device_id, int *device_id_len);

/* device reset */
int zwave_cc1_device_reset_locally_notifaction(char *rst_notify);

/* battery init */
int zwave_cc1_battery_get();
int zwave_cc1_battery_rpt(char *param, int size, char *battery_level);

/* wakeup */
int zwave_cc1_wake_up_internal_get();
int zwave_cc1_wake_up_internal_rpt(char *param, int size, int *seconds, char *node_id);
int zwave_cc1_wake_up_internal_set(int seconds, char node_id);
int zwave_cc1_wake_up_no_more_information_set();
int zwave_cc1_wake_up_notification_rpt(char *param, int size);

int zwave_cc2_wake_up_internal_capabilities_get();
int zwave_cc2_wake_up_internal_capabilities_rpt(char *param, int size, int *min_interval, int *max_interval, int *def_interval, int *step_interval);
int zwave_cc2_wake_up_internal_get();
int zwave_cc2_wake_up_internal_rpt(char *param, int size, int *seconds, char *node_id);
int zwave_cc2_wake_up_internal_set(int seconds, char node_id);
int zwave_cc2_wake_up_no_more_information_set();
int zwave_cc2_wake_up_notification_rpt(char *param, int size);

/* notify alarm */
int zwave_cc1_alarm_get(char alarm_type);
int zwave_cc1_alarm_rpt(char *param, int size, char *alarm_type, char *alarm_level);

int zwave_cc2_alarm_get(char ararm_type, char zwave_alarm_type);
int zwave_cc2_alarm_rpt(char *param, int size, char *alarm_type, char *alarm_level, char *node_id, char *zwave_alarm_status, char *zwave_alarm_type, 
											 char *zwave_alarm_event, char event_params_num, char *event_params);
int zwave_cc2_alarm_set(char zwave_alarm_type, char zwave_alarm_satus);
int zwave_cc2_alarm_type_supported_get();
int zwave_cc2_alarm_type_supported_rpt(char *param, int size, char *num_bitmask,  char *vl_alarm, char *bitmasks);

int zwave_cc3_notification_get(char v1_alarm_type, char notification_type, char event);
int zwave_cc3_notification_rpt(char *param, int size, char *v1_alarm_type, char *v1_alarm_level, char *node_id, 
															char *notification_status, char *notification_type, char *event, char *event_param_len, char *has_seq, char *event_param, char *seq);
int zwave_cc3_notification_set(char notification_type, char notification_status);
int zwave_cc3_nofification_supported_get();
int zwave_cc3_nofification_supported_rpt(char *param, int size, char *num_bitmask, char *vl_alarm, char *bitmasks);
int zwave_cc3_event_supported_get(char notification_type);
int zwave_cc3_event_supported_rpt(char *param, int size, char *notification_type, char *num_bitmask, char *bitmasks);


int zwave_cc4_notification_get(char v1_alarm_type, char notification_type, char event);
int zwave_cc4_notification_rpt(char *param, int size, char *v1_alarm_type, char *v1_alarm_level, char *node_id, 
															char *notification_status, char *notification_type, char *event, char *event_param_len, char *has_seq, char *event_param, char *seq);
int zwave_cc4_notification_set(char notification_type, char notification_status);
int zwave_cc4_nofification_supported_get();
int zwave_cc4_nofification_supported_rpt(char *param, int size, char *num_bitmask, char *vl_alarm, char *bitmasks);
int zwave_cc4_event_supported_get(char notification_type);
int zwave_cc4_event_supported_rpt(char *param, int size, char *notification_type, char *num_bitmask, char *bitmasks);

/* switch multi */

/* switch all */
int zwave_cc1_switch_all_get();
int zwave_cc1_switch_all_off();
int zwave_cc1_switch_all_on();
int zwave_cc1_switch_all_rpt(char *param, int size, char *mode);
int zwave_cc1_switch_all_set(char mode);

/* protection */

/* configure */

#endif

