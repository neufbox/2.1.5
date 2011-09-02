/*
<:copyright-broadcom

 Copyright (c) 2005 Broadcom Corporation
 All Rights Reserved
 No portions of this material may be reproduced in any form without the
 written permission of:
          Broadcom Corporation
          16215 Alton Parkway
          Irvine, California 92619
 All information contained in this document is Broadcom Corporation
 company private, proprietary, and trade secret.

:>
*/
/***************************************************************************
 * File Name  : dsp_mod_size.h
 *
 * Description: This file contains two defines that represent the core and
 * init sizes of the DSP module. These values are only used during a voice 
 * build and are updated during the build procedure.
 ***************************************************************************/

#if !defined(_DSP_MOD_SIZE_H)
#define _DSP_MOD_SIZE_H

/*
 * Defines representing the memory chunks allocated for the DSP module. These defines will be set during
 * the build process.
 */
#define DSP_CORE_SIZE 797872
#define DSP_INIT_SIZE 0

#endif /* _DSP_MOD_SIZE_H */

