/***********************************************************************
//
//  Copyright (c) 2008  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          5300 California Avenue 
//          Irvine, California 92617 
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
************************************************************************/
#ifndef _BCMPWRMNGTCFG_H
#define _BCMPWRMNGTCFG_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************
 * Constant Definitions
 ***************************************************************************/
#define PWRMNGT_ENABLE                         1
#define PWRMNGT_DISABLE                        0

/* MIPS CPU Valid Speeds */
#define PWRMNGT_MIPS_CPU_SPEED_MIN_VAL         1
#define PWRMNGT_MIPS_CPU_SPEED_MAX_VAL         8

/* Return status values. */
typedef enum PwrMngtStatus
{
    PWRMNGTSTS_SUCCESS = 0,
    PWRMNGTSTS_INIT_FAILED,
    PWRMNGTSTS_ERROR,
    PWRMNGTSTS_STATE_ERROR,
    PWRMNGTSTS_PARAMETER_ERROR,
    PWRMNGTSTS_ALLOC_ERROR,
    PWRMNGTSTS_NOT_SUPPORTED,
    PWRMNGTSTS_TIMEOUT,
} PWRMNGT_STATUS;

typedef unsigned int ui32;

/* Masks defined here are used in selecting the required parameter from Mgmt
 * application.
 */
#if defined(CHIP_6358)
#define PWRMNGT_CFG_MAX_PARAMS                   17
#define PWRMNGT_CFG_PARAM_ALL_MASK               0x0002FFFF
#define PWRMNGT_CFG_DEF_PARAM_MASK               0x0002FFFF  /* Parameters for which the defaults exist */
#elif defined(CHIP_6368)
#define PWRMNGT_CFG_MAX_PARAMS                   13
#define PWRMNGT_CFG_PARAM_ALL_MASK               0x00002FFF
#define PWRMNGT_CFG_DEF_PARAM_MASK               0x00002FFF  /* Parameters for which the defaults exist */
#endif

typedef struct _PwrMngtConfigParams {
#define PWRMNGT_CFG_PARAM_CPUSPEED_MASK            0x00000001
   ui32                  cpuspeed;
#define PWRMNGT_CFG_PARAM_CPU_R4K_WAIT_MASK        0x00000002
   ui32                  cpur4kwait;
#define PWRMNGT_CFG_PARAM_DRAM_AUTO_PWR_DOWN_MASK  0x00000004
   ui32                  dram;
#define PWRMNGT_CFG_PARAM_VCT_MASK                 0x00000008
   ui32                  vct;
#define PWRMNGT_CFG_PARAM_WIFI_MASK                0x00000010
   ui32                  wifi;
#define PWRMNGT_CFG_PARAM_LED_MASK                 0x00000020
   ui32                  led;
#define PWRMNGT_CFG_PARAM_USBHOST_MASK             0x00000040
   ui32                  usbhost;
#define PWRMNGT_CFG_PARAM_USBDEV_MASK              0x00000080
   ui32                  usbdev;
#define PWRMNGT_CFG_PARAM_DSL_MASK                 0x00000100
   ui32                  dsl;
#define PWRMNGT_CFG_PARAM_SPU_MASK                 0x00000200
   ui32                  spu;
#define PWRMNGT_CFG_PARAM_NAND_MASK                0x00000400
   ui32                  nand;

#if defined(CHIP_6358) || defined(CONFIG_BCM96358)
#define PWRMNGT_CFG_PARAM_6358MAC0_MASK            0x00000800
   ui32                  bcm6358MAC0En;
#define PWRMNGT_CFG_PARAM_6358MAC1_MASK            0x00001000
   ui32                  bcm6358MAC1En;
#define PWRMNGT_CFG_PARAM_6358EPHY_MASK            0x00002000
   ui32                  bcm6358EPHYEn;
#define PWRMNGT_CFG_PARAM_5325EPHY0_MASK           0x00004000
   ui32                  bcm5325ePHY0En;
#define PWRMNGT_CFG_PARAM_5325EPHY1_MASK           0x00008000
   ui32                  bcm5325ePHY1En;
#define PWRMNGT_CFG_PARAM_5325EPHY2_MASK           0x00010000
   ui32                  bcm5325ePHY2En;
#define PWRMNGT_CFG_PARAM_5325EPHY3_MASK           0x00020000
   ui32                  bcm5325ePHY3En;
#define PWRMNGT_CFG_PARAM_5325EPHY45_MASK          0x00040000
   ui32                  bcm5325ePHY45En;
#endif /* CHIP_6358 */

#if defined(CHIP_6368) || defined(CONFIG_BCM96368)
#define PWRMNGT_CFG_PARAM_6368ETHPHY0_MASK         0x00000800
   ui32                  bcm6368EthPHY0En;
#define PWRMNGT_CFG_PARAM_6368ETHPHY1_MASK         0x00001000
   ui32                  bcm6368EthPHY1En;
#define PWRMNGT_CFG_PARAM_6368ETHPHY2_MASK         0x00002000
   ui32                  bcm6368EthPHY2En;
#define PWRMNGT_CFG_PARAM_6368ETHPHY3_MASK         0x00004000
   ui32                  bcm6368EthPHY3En;
#endif /* CHIP_6368 */
} PWRMNGT_CONFIG_PARAMS, *PPWRMNGT_CONFIG_PARAMS ;

/***************************************************************************
 * Function Prototypes
 ***************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _BCMPWRMNGTCGG_H */
