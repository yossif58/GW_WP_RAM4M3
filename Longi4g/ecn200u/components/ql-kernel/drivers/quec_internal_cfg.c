
/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
    
/*=================================================================

						EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN			  WHO		  WHAT, WHERE, WHY
------------	 -------	 -------------------------------------------------------------------------------

=================================================================*/


/*===========================================================================
 * include files
 ===========================================================================*/
#include "quec_internal_cfg.h"
#include "ql_adc.h"
#include "drv_adc.h"
#include "quec_cust_feature.h"

/*===========================================================================
 * Customer set Map
 ===========================================================================*/
/*****  WIFISCAN RSSI offset  *****/
int8_t ql_wifi_rssi_offset[] =
#if defined (CONFIG_QL_PROJECT_DEF_EC200U) || defined (CONFIG_QL_PROJECT_DEF_EC020U)
    {17, 18, 24, 24, 21};
#elif defined CONFIG_QL_PROJECT_DEF_EC600U_CN_LB
    {21, 20, 22, 21, 20};
#elif defined CONFIG_QL_PROJECT_DEF_EC600U_EU_AB
    {20, 23, 24, 24, 21};
#elif defined (CONFIG_QL_PROJECT_DEF_EG700U) || defined (CONFIG_QL_PROJECT_DEF_EG070U) \
    || defined (CONFIG_QL_PROJECT_DEF_EG500U)
    {15, 17, 19, 18, 17};
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_EU_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_EC_AB)
    {20, 23, 24, 24, 21};
#elif defined CONFIG_QL_PROJECT_DEF_EG915U_LA_AB
    {20, 23, 24, 24, 21};
#elif defined CONFIG_QL_PROJECT_DEF_EG915U_CN_AB
    {20, 23, 24, 24, 21};
#endif


/********************************************************************************
*                    ADC channel corresponds to the drv
*
*           EC200U             EG500U          EC600U          EG700U          EG915U
* ADC0      ADC_CHANNEL_2      ADC_CHANNEL_2   ADC_CHANNEL_2   ADC_CHANNEL_1   ADC_CHANNEL_1
* ADC1      ADC_CHANNEL_3      ADC_CHANNEL_3   ADC_CHANNEL_3   ADC_CHANNEL_2   ADC_CHANNEL_2
* ADC2      ADC_CHANNEL_4      ADC_CHANNEL_1   ADC_CHANNEL_4   ADC_CHANNEL_3   ADC_CHANNEL_NONE
* ADC3      ADC_CHANNEL_NONE   ADC_CHANNEL_4   ADC_CHANNEL_1   ADC_CHANNEL_4   ADC_CHANNEL_NONE
*********************************************************************************/
const ql_adc_channel_s ql_adc_channel_num =
   /*      ADC0              ADC1                ADC2                ADC3         */
#if defined (CONFIG_QL_PROJECT_DEF_EC200U) || defined (CONFIG_QL_PROJECT_DEF_EC020U)
    {  ADC_CHANNEL_2,     ADC_CHANNEL_3,     ADC_CHANNEL_4,     ADC_CHANNEL_NONE  };
#elif defined CONFIG_QL_PROJECT_DEF_EC600U
    {  ADC_CHANNEL_2,     ADC_CHANNEL_3,     ADC_CHANNEL_4,     ADC_CHANNEL_1     };
#elif defined CONFIG_QL_PROJECT_DEF_EG700U || defined CONFIG_QL_PROJECT_DEF_EG070U
    {  ADC_CHANNEL_1,     ADC_CHANNEL_2,     ADC_CHANNEL_3,     ADC_CHANNEL_4     };
#elif defined CONFIG_QL_PROJECT_DEF_EG500U
    {  ADC_CHANNEL_2,     ADC_CHANNEL_3,     ADC_CHANNEL_1,     ADC_CHANNEL_4     };
#elif defined CONFIG_QL_PROJECT_DEF_EG915U
    {  ADC_CHANNEL_1,     ADC_CHANNEL_2,     ADC_CHANNEL_NONE,  ADC_CHANNEL_NONE  };
#endif


/*****  GSM func select              ******/
/*****  1: have GSM  0: haven't GSM  ******/
const uint8_t ql_gsm_select =
#if defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_AB) || defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MB)\
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_TA) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AB) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AT)\
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC600U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB) || defined (CONFIG_QL_PROJECT_DEF_EG070U_EU_AB)\
    || defined (CONFIG_QL_PROJECT_DEF_EG500U_CN_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG915U_CN_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG915U_EU_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_EC_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB)
    {  1  };
#else/* LTE only */
    {  0  };
#endif

/*****  band size  *****/
const ql_band_size_s ql_band_size =
   /*  gsm_size    lte_size  */
#if defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC600U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB)
    {     4,          10     };
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_EU_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB)
    {     4,          7      };
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_EC_AB)
    {     4,          6      };
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB)
    {     4,          11     };
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AT)
    {     2,          3      };
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MB) || defined (CONFIG_QL_PROJECT_DEF_EG700U_CN_MB)
    {     2,          7      };
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MT)
    {     0,          7      };
#elif defined (CONFIG_QL_PROJECT_DEF_EG070U_EU_AB)
    {     4,          12      };
#else
    {     2,          9      };
#endif

/*****  !0: have LTE_EXT  0: haven't LTE_EXT  ******/
const uint8_t ql_lte_ext_size =
#if defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB)
    {  1  };
#else
    {  0  };
#endif

const quectel_band_info_s quectel_band_tbl_gsm[] =
{
#if defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC600U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG915U_EU_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_EC_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB) || defined (CONFIG_QL_PROJECT_DEF_EG070U_EU_AB)
    {QUEC_BAND_GSM_900, RDA_BAND_EGSM_900 | RDA_BAND_PGSM_900}, //8910DM support EGSM_900 and PGSM_900
    {QUEC_BAND_GSM_1800, RDA_BAND_GSM_1800},
    {QUEC_BAND_GSM_850, RDA_BAND_GSM_850},
    {QUEC_BAND_GSM_1900, RDA_BAND_GSM_1900}
#else
    {QUEC_BAND_GSM_900, RDA_BAND_EGSM_900 | RDA_BAND_PGSM_900}, //8910DM support EGSM_900 and PGSM_900
    {QUEC_BAND_GSM_1800, RDA_BAND_GSM_1800}
#endif
};

const quectel_band_info_s quectel_band_tbl_lte[] =
{
#if defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC600U_EU_AB)
    {QUEC_BAND_LTE_BAND1, RDA_BAND_LTE_BAND1},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND20, RDA_BAND_LTE_BAND20},
    {QUEC_BAND_LTE_BAND28, RDA_BAND_LTE_BAND28},
    {QUEC_BAND_LTE_BAND38, RDA_BAND_LTE_BAND38},
    {QUEC_BAND_LTE_BAND40, RDA_BAND_LTE_BAND40},
    {QUEC_BAND_LTE_BAND41, RDA_BAND_LTE_BAND41}
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_EU_AB)
    {QUEC_BAND_LTE_BAND1, RDA_BAND_LTE_BAND1},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND20, RDA_BAND_LTE_BAND20},
    {QUEC_BAND_LTE_BAND28, RDA_BAND_LTE_BAND28}
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_EC_AB)
    {QUEC_BAND_LTE_BAND1, RDA_BAND_LTE_BAND1},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND20, RDA_BAND_LTE_BAND20},
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB)
    {QUEC_BAND_LTE_BAND2, RDA_BAND_LTE_BAND2},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND4, RDA_BAND_LTE_BAND4},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND28, RDA_BAND_LTE_BAND28}
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB)
    {QUEC_BAND_LTE_BAND1, RDA_BAND_LTE_BAND1},
    {QUEC_BAND_LTE_BAND2, RDA_BAND_LTE_BAND2},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND4, RDA_BAND_LTE_BAND4},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND28, RDA_BAND_LTE_BAND28},
    {QUEC_BAND_LTE_BAND38, RDA_BAND_LTE_BAND38},
    {QUEC_BAND_LTE_BAND40, RDA_BAND_LTE_BAND40},
    {QUEC_BAND_LTE_BAND41, RDA_BAND_LTE_BAND41}
#elif defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB)
    {QUEC_BAND_LTE_BAND2, RDA_BAND_LTE_BAND2},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND4, RDA_BAND_LTE_BAND4},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND28, RDA_BAND_LTE_BAND28},
    {QUEC_BAND_LTE_BAND38, RDA_BAND_LTE_BAND38},
    {QUEC_BAND_LTE_BAND40, RDA_BAND_LTE_BAND40},
    {QUEC_BAND_LTE_BAND41, RDA_BAND_LTE_BAND41}
#elif defined (CONFIG_QL_PROJECT_DEF_EG070U_EU_AB)
    {QUEC_BAND_LTE_BAND1, RDA_BAND_LTE_BAND1},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND20, RDA_BAND_LTE_BAND20},
    {QUEC_BAND_LTE_BAND28, RDA_BAND_LTE_BAND28},
    {QUEC_BAND_LTE_BAND34, RDA_BAND_LTE_BAND34},
    {QUEC_BAND_LTE_BAND38, RDA_BAND_LTE_BAND38},
    {QUEC_BAND_LTE_BAND39, RDA_BAND_LTE_BAND39},
    {QUEC_BAND_LTE_BAND40, RDA_BAND_LTE_BAND40},
    {QUEC_BAND_LTE_BAND41, RDA_BAND_LTE_BAND41}
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AT)
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND7, RDA_BAND_LTE_BAND7},
    {QUEC_BAND_LTE_BAND40, RDA_BAND_LTE_BAND40}
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MB) || defined (CONFIG_QL_PROJECT_DEF_EG700U_CN_MB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MT)
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND34, RDA_BAND_LTE_BAND34},
    {QUEC_BAND_LTE_BAND38, RDA_BAND_LTE_BAND38},
    {QUEC_BAND_LTE_BAND39, RDA_BAND_LTE_BAND39},
    {QUEC_BAND_LTE_BAND40, RDA_BAND_LTE_BAND40},
    {QUEC_BAND_LTE_BAND41, RDA_BAND_LTE_BAND41}
#else
    {QUEC_BAND_LTE_BAND1, RDA_BAND_LTE_BAND1},
    {QUEC_BAND_LTE_BAND3, RDA_BAND_LTE_BAND3},
    {QUEC_BAND_LTE_BAND5, RDA_BAND_LTE_BAND5},
    {QUEC_BAND_LTE_BAND8, RDA_BAND_LTE_BAND8},
    {QUEC_BAND_LTE_BAND34, RDA_BAND_LTE_BAND34},
    {QUEC_BAND_LTE_BAND38, RDA_BAND_LTE_BAND38},
    {QUEC_BAND_LTE_BAND39, RDA_BAND_LTE_BAND39},
    {QUEC_BAND_LTE_BAND40, RDA_BAND_LTE_BAND40},
    {QUEC_BAND_LTE_BAND41, RDA_BAND_LTE_BAND41}
#endif
};

const quectel_band_info_s quectel_band_tbl_lte_ext[] =
{
#if defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB)
    {QUEC_BAND_LTE_BAND66, RDA_BAND_LTE_BAND66}
#endif
};

qrftest_label_type qrftest_labels[] =
{
#ifdef CONFIG_QUEC_PROJECT_FEATURE_RFTEST_AT
    /*   mode        name              band            num    tx_min  tx_max  rx_min  rx_max  */
    //GSM
#if defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC600U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG915U_EU_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_EC_AB) || defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB) || defined (CONFIG_QL_PROJECT_DEF_EG070U_EU_AB)
    {  QUEC_GSM,  "GSM900"    ,  RDA_BAND_EGSM_900  ,  900 ,  0    ,  1023 ,  0    ,  1023   }, //For EGSM 900, range: (0-124) and (975-1023)
    {  QUEC_GSM,  "GSM1800"   ,  RDA_BAND_GSM_1800  ,  1800,  512  ,  885  ,  512  ,  885    },
    {  QUEC_GSM,  "GSM1900"   ,  RDA_BAND_GSM_1900  ,  1900,  512  ,  810  ,  512  ,  810    },
    {  QUEC_GSM,  "GSM850"    ,  RDA_BAND_GSM_850   ,  850 ,  128  ,  251  ,  128  ,  251    },
#elif defined (CONFIG_QL_PROJECT_DEF_EC020U_CN_LB) ||  defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MT)
    /* no GSM */
#else
    {  QUEC_GSM,  "GSM900"    ,  RDA_BAND_EGSM_900  ,  900 ,  0    ,  1023 ,  0    ,  1023   }, //For EGSM 900, range: (0-124) and (975-1023)
    {  QUEC_GSM,  "GSM1800"   ,  RDA_BAND_GSM_1800  ,  1800,  512  ,  885  ,  512  ,  885    },
#endif
    //LTE
#if defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC600U_EU_AB)
    {  QUEC_LTE,  "LTE BAND1" ,  RDA_BAND_LTE_BAND1 ,  1   ,  18000,  18599,  0    ,  599    },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND20",  RDA_BAND_LTE_BAND20,  20  ,  24150,  24449,  6150 ,  6449   },
    {  QUEC_LTE,  "LTE BAND28",  RDA_BAND_LTE_BAND28,  28  ,  27210,  27659,  9210 ,  9659   },
    {  QUEC_LTE,  "LTE BAND38",  RDA_BAND_LTE_BAND38,  38  ,  37750,  38249,  37750,  38249  },
    {  QUEC_LTE,  "LTE BAND40",  RDA_BAND_LTE_BAND40,  40  ,  38650,  39649,  38650,  39649  },
    #if defined (CONFIG_QL_PROJECT_DEF_EC600U_EU_AB)
    {  QUEC_LTE,  "LTE BAND41",  RDA_BAND_LTE_BAND41,  41  ,  40040,  41439,  40040,  41439  },  //140M
    #else
    {  QUEC_LTE,  "LTE BAND41",  RDA_BAND_LTE_BAND41,  41  ,  39650,  41589,  39650,  41589  },  //194M
    #endif
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_EU_AB)
    {  QUEC_LTE,  "LTE BAND1" ,  RDA_BAND_LTE_BAND1 ,  1   ,  18000,  18599,  0    ,  599    },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND20",  RDA_BAND_LTE_BAND20,  20  ,  24150,  24449,  6150 ,  6449   },
    {  QUEC_LTE,  "LTE BAND28",  RDA_BAND_LTE_BAND28,  28  ,  27210,  27659,  9210 ,  9659   },
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_EC_AB)
    {  QUEC_LTE,  "LTE BAND1" ,  RDA_BAND_LTE_BAND1 ,  1   ,  18000,  18599,  0    ,  599    },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND20",  RDA_BAND_LTE_BAND20,  20  ,  24150,  24449,  6150 ,  6449   },
#elif defined (CONFIG_QL_PROJECT_DEF_EG915U_LA_AB)
    {  QUEC_LTE,  "LTE BAND2" ,  RDA_BAND_LTE_BAND2 ,  2   ,  18600,  19199,  600  ,  1199   },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND4" ,  RDA_BAND_LTE_BAND4 ,  4   ,  19950,  20399,  1950 ,  2399   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND28",  RDA_BAND_LTE_BAND28,  28  ,  27210,  27659,  9210 ,  9659   },
    {  QUEC_LTE,  "LTE BAND66",  RDA_BAND_LTE_BAND66,  66  ,  131972, 132671, 66436,  67135  },
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB)
    {  QUEC_LTE,  "LTE BAND1" ,  RDA_BAND_LTE_BAND1 ,  1   ,  18000,  18599,  0    ,  599    },
    {  QUEC_LTE,  "LTE BAND2" ,  RDA_BAND_LTE_BAND2 ,  2   ,  18600,  19199,  600  ,  1199   },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND4" ,  RDA_BAND_LTE_BAND4 ,  4   ,  19950,  20399,  1950 ,  2399   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND28",  RDA_BAND_LTE_BAND28,  28  ,  27210,  27659,  9210 ,  9659   },
    {  QUEC_LTE,  "LTE BAND38",  RDA_BAND_LTE_BAND38,  38  ,  37750,  38249,  37750,  38249  },
    {  QUEC_LTE,  "LTE BAND40",  RDA_BAND_LTE_BAND40,  40  ,  38650,  39649,  38650,  39649  },
    {  QUEC_LTE,  "LTE BAND41",  RDA_BAND_LTE_BAND41,  41  ,  39650,  41589,  39650,  41589  },  //194M
    {  QUEC_LTE,  "LTE BAND66",  RDA_BAND_LTE_BAND66,  66  ,  131972, 132671, 66436,  67135  },
#elif defined (CONFIG_QL_PROJECT_DEF_EG070U_LA_AB)
    {  QUEC_LTE,  "LTE BAND2" ,  RDA_BAND_LTE_BAND2 ,  2   ,  18600,  19199,  600  ,  1199   },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND4" ,  RDA_BAND_LTE_BAND4 ,  4   ,  19950,  20399,  1950 ,  2399   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND28",  RDA_BAND_LTE_BAND28,  28  ,  27210,  27659,  9210 ,  9659   },
    {  QUEC_LTE,  "LTE BAND38",  RDA_BAND_LTE_BAND38,  38  ,  37750,  38249,  37750,  38249  },
    {  QUEC_LTE,  "LTE BAND40",  RDA_BAND_LTE_BAND40,  40  ,  38650,  39649,  38650,  39649  },
    {  QUEC_LTE,  "LTE BAND41",  RDA_BAND_LTE_BAND41,  41  ,  40040,  41439,  40040,  41439  },  //140M
    {  QUEC_LTE,  "LTE BAND66",  RDA_BAND_LTE_BAND66,  66  ,  131972, 132671, 66436,  67135  },
#elif defined (CONFIG_QL_PROJECT_DEF_EG070U_EU_AB)
    {  QUEC_LTE,  "LTE BAND1" ,  RDA_BAND_LTE_BAND1 ,  1   ,  18000,  18599,  0    ,  599    },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND20",  RDA_BAND_LTE_BAND20,  20  ,  24150,  24449,  6150 ,  6449   },
    {  QUEC_LTE,  "LTE BAND28",  RDA_BAND_LTE_BAND28,  28  ,  27210,  27659,  9210 ,  9659   },
    {  QUEC_LTE,  "LTE BAND34",  RDA_BAND_LTE_BAND34,  34  ,  36200,  36349,  36200,  36349  },
    {  QUEC_LTE,  "LTE BAND38",  RDA_BAND_LTE_BAND38,  38  ,  37750,  38249,  37750,  38249  },
    {  QUEC_LTE,  "LTE BAND39",  RDA_BAND_LTE_BAND39,  39  ,  38250,  38649,  38250,  38649  },
    {  QUEC_LTE,  "LTE BAND40",  RDA_BAND_LTE_BAND40,  40  ,  38650,  39649,  38650,  39649  },
    {  QUEC_LTE,  "LTE BAND41",  RDA_BAND_LTE_BAND41,  41  ,  39650,  41589,  39650,  41589  },  //194M

#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AT)
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND7" ,  RDA_BAND_LTE_BAND7 ,  7   ,  20750,  21449,  2750 ,  3449   },
    {  QUEC_LTE,  "LTE BAND40",  RDA_BAND_LTE_BAND40,  40  ,  38650,  39649,  38650,  39649  }
#elif defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MB) || defined (CONFIG_QL_PROJECT_DEF_EG700U_CN_MB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_CN_MT)
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND34",  RDA_BAND_LTE_BAND34,  34  ,  36200,  36349,  36200,  36349  },
    {  QUEC_LTE,  "LTE BAND38",  RDA_BAND_LTE_BAND38,  38  ,  37750,  38249,  37750,  38249  },
    {  QUEC_LTE,  "LTE BAND39",  RDA_BAND_LTE_BAND39,  39  ,  38250,  38649,  38250,  38649  },
    {  QUEC_LTE,  "LTE BAND40",  RDA_BAND_LTE_BAND40,  40  ,  38650,  39649,  38650,  39649  },
    {  QUEC_LTE,  "LTE BAND41",  RDA_BAND_LTE_BAND41,  41  ,  40040,  41439,  40040,  41439  },  //140M
#else
    {  QUEC_LTE,  "LTE BAND1" ,  RDA_BAND_LTE_BAND1 ,  1   ,  18000,  18599,  0    ,  599    },
    {  QUEC_LTE,  "LTE BAND3" ,  RDA_BAND_LTE_BAND3 ,  3   ,  19200,  19949,  1200 ,  1949   },
    {  QUEC_LTE,  "LTE BAND5" ,  RDA_BAND_LTE_BAND5 ,  5   ,  20400,  20649,  2400 ,  2649   },
    {  QUEC_LTE,  "LTE BAND8" ,  RDA_BAND_LTE_BAND8 ,  8   ,  21450,  21799,  3450 ,  3799   },
    {  QUEC_LTE,  "LTE BAND34",  RDA_BAND_LTE_BAND34,  34  ,  36200,  36349,  36200,  36349  },
    {  QUEC_LTE,  "LTE BAND38",  RDA_BAND_LTE_BAND38,  38  ,  37750,  38249,  37750,  38249  },
    {  QUEC_LTE,  "LTE BAND39",  RDA_BAND_LTE_BAND39,  39  ,  38250,  38649,  38250,  38649  },
    {  QUEC_LTE,  "LTE BAND40",  RDA_BAND_LTE_BAND40,  40  ,  38650,  39649,  38650,  39649  },
    {  QUEC_LTE,  "LTE BAND41",  RDA_BAND_LTE_BAND41,  41  ,  40040,  41439,  40040,  41439  },  //140M
#endif
#endif  /* CONFIG_QUEC_PROJECT_FEATURE_RFTEST_AT */
};

/*****  band41 bandwidth select  *****/
const uint16_t ql_band41_bw_sel =
/*** 194M ***/
#if defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_EU_AB) \
    || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AA) || defined (CONFIG_QL_PROJECT_DEF_EC200U_AU_AB)
    {  0  };        // 0 means full bandwidth, RF set 0 in Nv when band41 is 194M.
/*** 140M ***/
#else
    {  140  };
#endif

/*****  pin set  *****/
volatile ql_pin_set_s ql_pin_set =
   /*         sleep_ind                      wakeup_in                       main_dtr                     ap_ready                       net_mode                        w_disable  */
#ifdef CONFIG_QL_PROJECT_DEF_EC200U
 #ifdef CONFIG_QUEC_PROJECT_FEATURE_QDSIM
    {   QUEC_PIN_CUST_FUNC_SIM1,      QUEC_PIN_CUST_FUNC_SIM1,       QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_SIM1,      QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_SIM1};
 #else
    {   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT};
 #endif
#elif defined CONFIG_QL_PROJECT_DEF_EC600U
 #ifdef __QUEC_OEM_VER_LD__
    {   QUEC_PIN_CUST_FUNC_NONE,      QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_NONE,      QUEC_PIN_CUST_FUNC_NONE,      QUEC_PIN_CUST_FUNC_NONE,       QUEC_PIN_CUST_FUNC_NONE};
 #else
  #ifdef CONFIG_QUEC_PROJECT_FEATURE_QDSIM
    {   QUEC_PIN_CUST_FUNC_SIM1,      QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_SIM1,       QUEC_PIN_CUST_FUNC_SIM1};
  #else
    {   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT};
  #endif
 #endif
#elif defined CONFIG_QL_PROJECT_DEF_EG700U || defined CONFIG_QL_PROJECT_DEF_EG070U
    {   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT};
#elif defined CONFIG_QL_PROJECT_DEF_EC020U
    {   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_NONE,      QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT};
#elif defined CONFIG_QL_PROJECT_DEF_EG500U
    {   QUEC_PIN_CUST_FUNC_NONE,      QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_NONE,      QUEC_PIN_CUST_FUNC_DEFAULT,    QUEC_PIN_CUST_FUNC_DEFAULT};
#elif defined CONFIG_QL_PROJECT_DEF_EG915U
    {   QUEC_PIN_CUST_FUNC_NONE,      QUEC_PIN_CUST_FUNC_NONE,       QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_DEFAULT,   QUEC_PIN_CUST_FUNC_NONE,       QUEC_PIN_CUST_FUNC_DEFAULT};
#endif


#ifndef CONFIG_QL_OPEN_EXPORT_PKG
/*****  AT+QPINEXT  *****/
/* up to now, this atcmd for 1.EG700U/500U sd_det 2.EC600U cam_lamp  */
/* in addition to specified_func, other time don't use this atcmd!!! */
QPINEXT_S _qpinext[] =
{  /*    pin_num           pin_func1                  pin_func2               */
#if defined CONFIG_QL_PROJECT_DEF_EC200U || defined CONFIG_QL_PROJECT_DEF_EC020U

#elif defined CONFIG_QL_PROJECT_DEF_EC600U
    {  LOCATE_LAMP_PIN,  PIN_EXT_FUNC_SPI_LCD_CS ,  PIN_EXT_FUNC_LOCATE_LAMP  }, // default: SPI_LCD_CS
    {  FILL_LIGHT_PIN ,  PIN_EXT_FUNC_SPI_LCD_SIO,  PIN_EXT_FUNC_FILL_LIGHT   }, // default: SPI_LCD_SIO
#elif defined CONFIG_QL_PROJECT_DEF_EG700U || defined CONFIG_QL_PROJECT_DEF_EG070U
    {  SD_DET_PIN     ,  PIN_EXT_FUNC_SD         ,  PIN_EXT_FUNC_AP_READY     }, // default: SD_DET
#elif defined CONFIG_QL_PROJECT_DEF_EG500U
    {  SD_DET_PIN     ,  PIN_EXT_FUNC_SD         ,  PIN_EXT_FUNC_MAIN_DSR     }, // default: SD_DET
#elif defined CONFIG_QL_PROJECT_DEF_EG915U

#endif
    {  PINEXT_NONE    ,  PINEXT_NONE             ,  PINEXT_NONE               },
};
#endif

#ifndef CONFIG_QL_OPEN_EXPORT_PKG
/* EC200U & EC600U & EG915U can support dual sim single standby, sim1 pin need set fun */
/* EG915U already set fun at quec_pin_cfg_init()                                       */
const quec_sim_pin_set_s quec_sim1_pin_set_fun[] =
{   /*    pin_num          pin_enable    */
#ifdef CONFIG_QL_PROJECT_DEF_EC200U
    {  QUEC_PIN_SLEEP_IND,    QUEC_FUNC_SIM_ENABLE_DET    }, // SIM2_DET
    {  QUEC_PIN_WAKEUP_IN,    QUEC_FUNC_SIM_ENABLE        }, // SIM2_CLK
    {  QUEC_PIN_AP_READY,     QUEC_FUNC_SIM_ENABLE        }, // SIM2_DATA
    {  QUEC_PIN_NET_MODE,     QUEC_FUNC_SIM_DISABLE       }, // NOT FUNC
    {  QUEC_PIN_W_DISABLE,    QUEC_FUNC_SIM_ENABLE        }, // SIM2_RST
#elif defined CONFIG_QL_PROJECT_DEF_EC600U
 #ifndef __QUEC_OEM_VER_LD__
    {  QUEC_PIN_SLEEP_IND,    QUEC_FUNC_SIM_ENABLE        }, // SIM2_CLK
    {  QUEC_PIN_WAKEUP_IN,    QUEC_FUNC_SIM_DISABLE       }, // NOT FUNC
    {  QUEC_PIN_AP_READY,     QUEC_FUNC_SIM_DISABLE       }, // NOT FUNC
    {  QUEC_PIN_NET_MODE,     QUEC_FUNC_SIM_ENABLE        }, // SIM2_DATA
    {  QUEC_PIN_W_DISABLE,    QUEC_FUNC_SIM_ENABLE        }, // SIM2_RST
 #endif
#endif
    {  PINEXT_NONE,           QUEC_FUNC_SIM_DISABLE       },
};

#endif

