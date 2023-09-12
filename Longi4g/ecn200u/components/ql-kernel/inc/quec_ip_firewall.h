#ifndef __QUEC_IP_FIREWALL_H_
#define __QUEC_IP_FIREWALL_H_

typedef enum
{
    QUEC_FIREWALL_ENABLE = 0,
    QUEC_TCP_BLACKLIST_CLEAN_TIME= 1,
    QUEC_TCP_RECORD_LIST_CLEAN_TIME,
    QUEC_ICMP_BLACKLIST_CLEAN_TIME,
    QUEC_ICMP_RECORD_LIST_CLEAN_TIME,
    QUEC_TCP_ACCESS_PERCENTAGE,
    QUEC_TCP_INPUT_SCALE = 6,
    QUEC_ICMP_INPUT_SCALE,
    QUEC_ICMP_INPUT_INTERVAL,
    QUEC_SAVE_FILE_LOG_TIME,

    QUEC_FIREWALL_CONFIG_MAX
}quec_firewall_config_type;

void quec_firewall_set_config(quec_firewall_config_type type, int value);
int quec_firewall_get_config(quec_firewall_config_type type);

#endif /*__QUEC_IP_FIREWALL_H_*/
