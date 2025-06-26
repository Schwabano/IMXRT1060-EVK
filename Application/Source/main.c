/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This software is delivered "as is" and shows the usage of other software
* components. It is provided as an example software which is intended to be
* modified and extended according to particular requirements.
*
* TARA Systems hereby disclaims all warranties and conditions with regard to the
* software, including all implied warranties and conditions of merchantability
* and non-infringement of any third party IPR or other rights which may result
* from the use or the inability to use the software.
*
********************************************************************************
*
* DESCRIPTION:
*   This file implements the main.c module for running Embedded Wizard
*   generated GUI applications on a dedicated target with or without the usage
*   of an operating system.
*
*   Important: Please be aware that every Embedded Wizard GUI application
*   requires the execution in a single GUI task!
*   If you are working with an operating system and your software is using
*   several threads/tasks, please take care to access your GUI application
*   only within the context of your GUI thread/task. Use operating system
*   services to exchange data or events between the GUI thread/task and other
*   worker threads/tasks.
*
*   For more information concerning the integration of an Embedded Wizard
*   generated GUI application into your main application, please see
*   https://doc.embedded-wizard.de/main-loop
*
*******************************************************************************/

#include "ewmain.h"
#include "ewrte.h"
#include "ew_bsp_system.h"
#include "ew_bsp_console.h"
#include "ew_bsp_os.h"

#if EW_USE_OPERATING_SYSTEM == 1

static void GuiThread( const void* arg );


/*******************************************************************************
* FUNCTION:
*   main
*
* DESCRIPTION:
*   The main function for running Embedded Wizard generated GUI applications on
*   a dedicated target using the FreeRTOS operating system.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
int main( void )
{
  /* initialize system */
  EwBspSystemInit();

  /* initialize console interface for debug messages */
  EwBspConsoleInit();

  /* create thread that drives the Embedded Wizard GUI application... */
  EwBspOsThreadCreate( GuiThread, EW_BSP_OS_THREAD_PRIORITY_NORMAL, EW_GUI_THREAD_STACK_SIZE, 0 );

  /* ...and start scheduler */
  EwBspOsStartScheduler();

  /* restore console */
  EwBspConsoleDone();

  /* terminate the system */
  EwBspSystemDone();

  return 0;
}


/*******************************************************************************
* FUNCTION:
*   GuiThread
*
* DESCRIPTION:
*   The EwThread processes the Embeded Wizard application.
*
* ARGUMENTS:
*   arg - not used.
*
* RETURN VALUE:
*   None.
*
*******************************************************************************/
static void GuiThread( const void* arg )
{
  unsigned int stack;

  /* determine the stack area and provide it to the Runtime Environmnet */
  EwSetStackAddressArea( &stack, (void*)((unsigned int)&stack - EW_GUI_THREAD_STACK_SIZE ));

  /* initialize Embedded Wizard application */
  if ( EwInit() == 0 )
    return;

  EwPrintSystemInfo();

  /* process the Embedded Wizard main loop */
  while( EwProcess())
    ;

  /* de-initialize Embedded Wizard application */
  EwDone();
}

#else  /* EW_USE_OPERATING_SYSTEM == 0 */

/*******************************************************************************
* FUNCTION:
*   main
*
* DESCRIPTION:
*   The main function for running Embedded Wizard generated GUI applications on
*   a dedicated target without using any operating system (bare metal).
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   Zero if successful.
*
*******************************************************************************/
int main( void )
{
  /* initialize system */
  EwBspSystemInit();

  /* initialize console interface for debug messages */
  EwBspConsoleInit();

  /* initialize Embedded Wizard application */
  if ( EwInit() == 0 )
    return 1;

  EwPrintSystemInfo();

  /* process the Embedded Wizard main loop */
  while( EwProcess())
    ;

  /* de-initialize Embedded Wizard application */
  EwDone();

  /* restore console */
  EwBspConsoleDone();

  /* terminate the system */
  EwBspSystemDone();

  return 0;
}

#endif


/* msy, mli */
