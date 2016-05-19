/** vi: tw=128 ts=3 sw=3 et
@file dog_hal_8974_mpss.c
@brief This file contains the API details for the Dog Services, API 2.0
*/
/*=============================================================================
NOTE: The @brief description above does not appear in the PDF.
The tms_mainpage.dox file contains the group/module descriptions that
are displayed in the output PDF generated using Doxygen and LaTeX. To
edit or update any of the group/module text in the PDF, edit the
tms_mainpage.dox file or contact Tech Pubs.
===============================================================================*/
/*=============================================================================
Copyright (c) 2014 Qualcomm Technologies Incorporated.
All rights reserved.
Qualcomm Confidential and Proprietary
=============================================================================*/
/*=============================================================================
Edit History
$Header: //components/rel/core.adsp/2.6.1/debugtools/dog/src/dog_hal_8974_mpss.c#1 $
$DateTime: 2014/10/16 12:45:40 $
$Change: 6781644 $
$Author: pwbldsvc $
===============================================================================*/

/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

FUNCTIONS
  HAL_dogEnable
  HAL_dogDisable
  HAL_dogKick
  HAL_dogAutokick
  HAL_dogEnableBark
  HAL_dogDisableBark
  HAL_dogRegisterBark
  HAL_dogIsAutokicking

*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

#include "comdef.h"             /* Definitions for byte, word, etc.     */
#include "HALhwio.h"

#include "dog_hal.h"
#include "dog_hal_8974_mpss.h"

#include "msmhwiobase.h"

#include "DDIInterruptController.h"
#include "DalDevice.h"
#include "DALDeviceId.h"
#include "DALStdErr.h"

#include "err.h"

#define DOG_ISR                           68

/* sclk = 32MHz, 0.03125ms period, 1ms = 32sclk */

#define MS_TO_SCLK(ms)  ((ms)<<5) /* x32 */

#define DOG_BARK_ENABLE_TIMEOUT_SLCK      MS_TO_SCLK(250)
#define DOG_BARK_DISABLE_TIMEOUT_SLCK     0x3FFF            /* DISABLE == 512ms */
#define DOG_NMI_ENABLE_TIMEOUT_SLCK       MS_TO_SCLK(390)
#define DOG_BITE_ENABLE_TIMEOUT_SLCK      MS_TO_SCLK(400)
#define DOG_BITE_ENABLE_TIMEOUT_NOW_SLCK  0x0001            /* BITE "NOW" */

/*===========================================================================

FUNCTION HAL_dogBarkISR

DESCRIPTION
   This function is called when the watchdog bark interrupt occurs.
===========================================================================*/

void HAL_dogBarkISR(uint32 callback_param)
{
   static unsigned long count;

   do
   {
      count++;

      // RUNTIME POLICY CHECK

      // 1. frequency @2 seconds, collect current idle pcycles
      // 2. compare idle with last, remains same, IDLE_COMPARE_COUNT++
      // 3. compare idle with last, remains same, IDLE_COMPARE_COUNT(3) < counter, break do/while

      if (0 == (count % 8))
      {
      }

      // RUNTIME POLICY CHECK

      // 1. frequency @10 seconds, perform observers check of rcevt(RCINIT_RCEVT_INITGROUPS)
      // 2. observers check not set, OBSERVER_CHECK_COUNT++
      // 3. observers check not set, OBSERVER_CHECK_COUNT(3) < counter, break do/while

      if (0 == (count % 40))
      {
      }

      // RUNTIME POLICY CHECK: LAST

      // 1. Other Policy Checks Complete, HAL_dogKick

      HAL_dogKick();

   } while (0);

}

volatile boolean dog_intentional_timeout_flag = FALSE;

/** =====================================================================
 * Function:
 *     HAL_dogEnable
 *
 * Description:
 *     This function enables the hardware dog
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     none
 * =====================================================================  */
void HAL_dogEnable(void)
{
   // Set NMI Timeout
   HWIO_QDSP6SS_WDOG_NMI_TIME_OUT(DOG_NMI_ENABLE_TIMEOUT_SLCK);

   // Set Bite Timeout
   HWIO_QDSP6SS_WDOG_BITE_TIME_OUT(DOG_BITE_ENABLE_TIMEOUT_SLCK);

   // Enable, NMI + Bite
   HWIO_QDSP6SS_WDOG_CTL_OUTM(HWIO_QDSP6SS_WDOG_CTL_ENABLE_BMSK, 1 << HWIO_QDSP6SS_WDOG_CTL_ENABLE_SHFT);
   HWIO_QDSP6SS_WDOG_CTL_OUTM(HWIO_QDSP6SS_WDOG_CTL_WDOG_TO_NMI_EN_BMSK, 1 << HWIO_QDSP6SS_WDOG_CTL_WDOG_TO_NMI_EN_SHFT);
}

/** =====================================================================
 * Function:
 *     HAL_dogDisable
 *
 * Description:
 *     This function disables the hardware dog
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     none
 * =====================================================================  */
void HAL_dogDisable(void)
{
   if (!dog_intentional_timeout_flag)
   {
      HWIO_QDSP6SS_WDOG_CTL_OUTM(HWIO_QDSP6SS_WDOG_CTL_ENABLE_BMSK, 0 << HWIO_QDSP6SS_WDOG_CTL_ENABLE_SHFT);
   }
}

/** =====================================================================
 * Function:
 *     HAL_dogKick
 *
 * Description:
 *     This function kicks the hardware dog
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     none
 * =====================================================================  */
void HAL_dogKick(void)
{
   if (!dog_intentional_timeout_flag)
   {
      HWIO_QDSP6SS_WDOG_RESET_OUT(1 << HWIO_QDSP6SS_WDOG_RESET_WDOG_RESET_SHFT);
   }
}

/** =====================================================================
 * Function:
 *     HAL_dogAutokick
 *
 * Description:
 *     This function turns on dog autokick
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     none
 * =====================================================================  */
void HAL_dogAutokick(void)
{
   if (!dog_intentional_timeout_flag)
   {
      HWIO_QDSP6SS_WDOG_CTL_OUTM(HWIO_QDSP6SS_WDOG_CTL_ENABLE_BMSK, 0 << HWIO_QDSP6SS_WDOG_CTL_ENABLE_SHFT);
   }
}

/** =====================================================================
 * Function:
 *     HAL_dogEnableBark
 *
 * Description:
 *       Enable the watchdog bark timer.
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     none
 * =====================================================================  */
void HAL_dogEnableBark(void)
{
   HWIO_QDSP6SS_WDOG_BARK_TIME_OUT(DOG_BARK_ENABLE_TIMEOUT_SLCK);
}

/** =====================================================================
 * Function:
 *     HAL_dogDisableBark
 *
 * Description:
 *       Enable the watchdog bark timer.
 *
 * Parameters:
 *     none
 *
 * Returns:
 *     none
 * =====================================================================  */
void HAL_dogDisableBark(void)
{
   HWIO_QDSP6SS_WDOG_BARK_TIME_OUT(DOG_BARK_DISABLE_TIMEOUT_SLCK);
}

/** =====================================================================
 * Function:
 *     HAL_dogRegisterBark
 *
 * Description:
 *     Register an ISR for the watchdog bark interrupt.
 *
 * Parameters:
 *     none
 *
 *
 * Returns:
 *     none
 * =====================================================================  */
void HAL_dogRegisterBark(void)
{
   static DalDeviceHandle* hInthandle;

   if (DAL_SUCCESS != DAL_DeviceAttach(DALDEVICEID_INTERRUPTCONTROLLER, &hInthandle) || NULL == hInthandle)
   {
      ERR_FATAL("Dog cannot register with DAL", 0, 0, 0);
   }

   DalInterruptController_RegisterISR(hInthandle, DOG_ISR, (DALIRQ)HAL_dogBarkISR, 0, DALINTRCTRL_ENABLE_RISING_EDGE_TRIGGER);
}

/** =====================================================================
* Function:
*     HAL_dogIsAutokicking
*
* Description:
*     Check if dog is autokicking
*
* Parameters:
*     uint32 status pointer
*
* Returns:
*     pointer set to 1 if autokicking, 0 otherwise
* =====================================================================  */
void HAL_dogIsAutokicking(uint32* status)
{
   if (NULL != status)
   {
      if (0 != (HWIO_QDSP6SS_WDOG_CTL_INM(HWIO_QDSP6SS_WDOG_CTL_ENABLE_BMSK) & HWIO_QDSP6SS_WDOG_CTL_ENABLE_BMSK))
      {
         *status = 1;
      }
      else
      {
         *status = 0;
      }
   }
}

/** =====================================================================
* Function:
*     HAL_dogForceBite
*
* Description:
*       Force the watchdog to bite
*
* Parameters:
*     none
*
* Returns:
*     none
* =====================================================================  */
void HAL_dogForceBite(void)
{
   // SET MINIMUM TIMEOUT
   HWIO_QDSP6SS_WDOG_BITE_TIME_OUT(DOG_BITE_ENABLE_TIMEOUT_NOW_SLCK);

   // ENABLE
   HWIO_QDSP6SS_WDOG_CTL_OUTM(HWIO_QDSP6SS_WDOG_CTL_ENABLE_BMSK, 1 << HWIO_QDSP6SS_WDOG_CTL_ENABLE_SHFT);
}

void HAL_dogInit(void)
{
}