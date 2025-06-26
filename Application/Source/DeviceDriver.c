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
*   This file implements an interface between an Embedded Wizard generated UI
*   application and a certain device. Please consider this file only as template
*   that is intended to show the binding between an UI application and the
*   underlying system (e.g. middleware, BSP, hardware driver, protocol, ...).
*
*   This device driver is the counterpart to a device class implemented within
*   your Embedded Wizard project.
*
*   Feel free to adapt this file according your needs!
*
*   The following code demonstrates the access to the board LED, a simulated
*   ADC value and the hardware button. Furthemore, the console interface is used
*   to print a string.
*
*   The Embedded Wizard GUI example 'DeviceIntegration' shows the usage of this
*   device driver.
*
*   This file assumes to be the counterpart of the device class 'DeviceClass'
*   within the unit 'Application'.
*
*******************************************************************************/

/*
   Include all necessary files to access the real device and/or to get access
   to the required operating system calls.
*/
#include "ewrte.h"
#include "ew_bsp_inout.h"
#include "ew_bsp_os.h"

/*
   Include the generated header file to access the device class, for example to
   access the class 'DeviceClass' from the unit 'Application' include the
   generated file 'Application.h'.
*/
#include "Application.h"

#if EW_USE_OPERATING_SYSTEM == 1

  static volatile XThreadHandle WorkerThread = 0;

#endif

static volatile int DeviceInitialized = 0;

/*
   In order to ensure that the example code of this module is only compiled
   and linked to the example 'DeviceIntegration', we check for generated defines.
*/
#if ( defined _ApplicationDeviceClass__TriggerHardButtonEvent_ && defined _ApplicationDeviceClass__UpdateAdcValue_ )
  #define DEVICE_INTEGRATION_EXAMPLE
#endif

#ifdef DEVICE_INTEGRATION_EXAMPLE

/*******************************************************************************
* FUNCTION:
*   TriggerHardButtonEventProc
*
* DESCRIPTION:
*   This function is used to trigger a system event in the GUI application by
*   calling the method TriggerHardButtonEvent of the device class.
*   Please note, that this function has to be called in the context of the main
*   GUI thread/task. Use EwInvoke() to ensure the invocation of this function
*   in the right context.
*
*******************************************************************************/
static void TriggerHardButtonEventProc( const void* aData )
{
  ApplicationDeviceClass device = EwGetAutoObject( &ApplicationDevice, ApplicationDeviceClass );
  ApplicationDeviceClass__TriggerHardButtonEvent( device );
}


/*******************************************************************************
* FUNCTION:
*   UpdateAdcValueProc
*
* DESCRIPTION:
*   This function is used to inform the GUI application about a new ADC value by
*   calling the method UpdateAdcValue of the device class.
*   Please note, that this function has to be called in the context of the main
*   GUI thread/task. Use EwInvoke() to ensure the invocation of this function
*   in the right context.
*
*******************************************************************************/
static void UpdateAdcValueProc( const void* aData )
{
  int adcValue = *((int*)aData);

  /* only in case that the device driver is still initialized and the worker
     thread is still running, the data should be provided to the device class
     - otherwise, a new autoobject will be created and a new worker thread
     started... */
  if ( DeviceInitialized )
  {
    ApplicationDeviceClass device = EwGetAutoObject( &ApplicationDevice, ApplicationDeviceClass );
    ApplicationDeviceClass__UpdateAdcValue( device, adcValue );
  }
}


/*******************************************************************************
* FUNCTION:
*   HardButtonIsrCallback
*
* DESCRIPTION:
*   Callback function for the hardware button. This function is called every time
*   the hardware button is pressed or released.
*   Please note, that this function is called from the interrupt service routine!
*
*******************************************************************************/
void HardButtonIsrCallback( int aButtonPressed )
{
  /*
     Important note: This function is called from an interrupt handler and not
     in the context of the main GUI thread/task. NEVER make a direct function
     call to a method of the driver class or any other generated code
     from an interrupt handler or any other thread/task.
     EwInvoke() or EwInvokeCopy() have to be used to schedule the invocation of
     the desired method in the context of the GUI thread/task.
  */
  if ( aButtonPressed )
    EwInvoke( TriggerHardButtonEventProc, 0 );
}


#if EW_USE_OPERATING_SYSTEM == 1

/*******************************************************************************
* FUNCTION:
*   AdcWorkerThread
*
* DESCRIPTION:
*   In case of an operating system, a separate thread/task is used to query data
*   from the ADC. The function AdcWorkerThread implements this thread/task.
*   The thread is started when the device driver is initialized and it will be
*   terminated when the device driver is de-initialized.
*
*******************************************************************************/
static void AdcWorkerThread( const void* arg )
{
  int simulatedAdcValue = 0;

  while ( DeviceInitialized )
  {
    /* in order to simplify the example, the ADC value is just simulated - it
       could be read from a real ADC by using corresponding BSP functions... */
    simulatedAdcValue = ( simulatedAdcValue + 1 ) % 1024;

    /*
       Important note: This function is a separate thread/task and not executed
       in the context of the main GUI thread/task. NEVER make a direct function
       call to a method of the driver class or any other generated code
       from an interrupt handler or any other thread/task.
       EwInvoke() or EwInvokeCopy() have to be used to schedule the invocation of
       the desired method in the context of the GUI thread/task.
    */
    EwInvokeCopy( UpdateAdcValueProc, &simulatedAdcValue, sizeof( int ));

    /* sleep for a certain period... */
    EwBspOsDelay( 20 );
  }

  /* terminate the worker thread */
  WorkerThread = 0;
  EwBspOsThreadDestroy( EwBspOsThreadGetHandle());
}

#endif

/*******************************************************************************
* FUNCTION:
*   DeviceIntegrationExample_Init
*
* DESCRIPTION:
*   This is the initialization function used for the example 'DeviceIntegration'.
*   It initializes the hardware that should be accessed from the GUI. If the
*   GUI application is running in an operating system, a separate worker-thread
*   will be created to showcase the data processing as separate thread.
*
*******************************************************************************/
void DeviceIntegrationExample_Init( void )
{
  /* check for initialization */
  if ( DeviceInitialized )
    return;

  /* configure LED */
  EwBspInOutInitLed();

  /* configure interrupt for hardware button */
  EwBspInOutInitButton( HardButtonIsrCallback );

  DeviceInitialized = 1;

#if EW_USE_OPERATING_SYSTEM == 1

  /* create and start the worker thread to process ADC data */
  WorkerThread = EwBspOsThreadCreate( AdcWorkerThread, EW_BSP_OS_THREAD_PRIORITY_NORMAL, 1024, 0 );

#endif
}


/*******************************************************************************
* FUNCTION:
*   DeviceIntegrationExample_Done
*
* DESCRIPTION:
*   This is the de-initialization function of the example 'DeviceIntegration'.
*   It de-initializes the hardware that was accessed from the GUI. If the
*   GUI application is running in an operating system, the separate worker-thread
*   will be terminated.
*
*******************************************************************************/
void DeviceIntegrationExample_Done( void )
{
  DeviceInitialized = 0;

#if EW_USE_OPERATING_SYSTEM == 1

  /* wait until the worker thread is really terminated */
  while ( WorkerThread )
    EwBspOsDelay( 1 );

#endif
}


/*******************************************************************************
* FUNCTION:
*   DeviceIntegrationExample_SetLedStatus
*
* DESCRIPTION:
*   This is a sample for a function called from the device class, when a
*   property has changed. As a result, the corresponding value of the real
*   device should be changed.
*   In this implementation the LED is switched on or off.
*
*******************************************************************************/
void DeviceIntegrationExample_SetLedStatus( XInt32 aValue )
{
  /*
     Please note, that in case of an operating system this function is called
     within the context of the main GUI thread.
     If you want to execute a certain activity in a separate thread/task,
     you should send a message via OS message queue and transmit the new value.
  */

  /* here we are accessing directly a BSP function to switch on/off the LED */
  if ( aValue )
    EwBspInOutLedOn();
  else
    EwBspInOutLedOff();
}


/*******************************************************************************
* FUNCTION:
*   DeviceIntegrationExample_PrintMessage
*
* DESCRIPTION:
*   This is a sample for a function that is called directly from a 'Command'
*   method of the device class. As a result, the corresponding action should
*   happen.
*   In this implementation the given message is printed via the console interface.
*
*******************************************************************************/
void DeviceIntegrationExample_PrintMessage( XString aText )
{
  /* just print the given 16bit string... */
  EwPrint( "The string is: %S\n", aText );
}

#endif /* DEVICE_INTEGRATION_EXAMPLE */


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_Initialize
*
* DESCRIPTION:
*   The function DeviceDriver_Initialize() initializes the module and prepares all
*   necessary things to access or communicate with the real device.
*   The function has to be called from your main module, before the initialization
*   of your GUI application.
*
*   IMPORTANT NOTE: This global initialization function exists for compatibility
*   reasons. The initialization of dedicated hardware drivers or middleware APIs
*   should be done in separate functions called by the Init constructor of the
*   device class.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void DeviceDriver_Initialize( void )
{
  /*
     You can implement here the necessary code to initialize your particular
     hardware, to open needed devices, to open communication channels, etc.
  */
}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_Deinitialize
*
* DESCRIPTION:
*   The function DeviceDriver_Deinitialize() deinitializes the module and
*   finalizes the access or communication with the real device.
*   The function has to be called from your main module, after the GUI
*   application will be deinitialized.
*
*   IMPORTANT NOTE: This global de-initialization function exists for
*   compatibility reasons. It is recommend to use the Done destructor of the
*   device class.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void DeviceDriver_Deinitialize( void )
{
  /*
     You can implement here the necessary code to deinitialize your particular
     hardware, to close used devices, to close communication channels, etc.
  */
}


/*******************************************************************************
* FUNCTION:
*   DeviceDriver_ProcessData
*
* DESCRIPTION:
*   The function DeviceDriver_ProcessData() is called from the main UI loop, in
*   order to process data and events from your particular device.
*   This function is responsible to update properties within the device class
*   if the corresponding state or value of the real device has changed.
*   This function is also responsible to trigger system events if necessary.
*
*   IMPORTANT NOTE: This data processing function can be used to process data
*   periodically in the context of the main GUI thread/task or in case there
*   is no operating system at all (bare metal).
*   In case you want to feed the GUI application with data and events from a
*   foreign thread/task or an interrupt service routine it will be much easier
*   to use the functions EwInvoke() and EwInvokeCopy().
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   The function returns a non-zero value if a property has changed or if a
*   system event was triggered. If nothing happens, the function returns 0.
*
*******************************************************************************/
int DeviceDriver_ProcessData( void )
{
  int needUpdate = 0;

  /*
     Get the data you want to provide to the GUI application.
     Please note, that this function is called within the context of the main
     GUI thread/task.
  */

  #ifdef DEVICE_INTEGRATION_EXAMPLE

    /* the following code is used only in case of bare metal when there is no
       separate thread/task running */
    #if EW_USE_OPERATING_SYSTEM == 0

      static int simulatedAdcValue = 0;

      /* in order to simplify the example, the ADC value is just simulated - it
         could be read from a real ADC by using corresponding BSP functions... */
      simulatedAdcValue = ( simulatedAdcValue + 1 ) % 1024;

      /* due to the fact that this code is executed only in case there is no
        operating system used (bare metal), there is no need to use EwInvoke()
        here - so the update function can be called directly */
      UpdateAdcValueProc( &simulatedAdcValue );

      needUpdate = 1;

    #endif

  #endif /* DEVICE_INTEGRATION_EXAMPLE */

  /*
     Return a value != 0 if there is at least one property changed or if a
     system event was triggered. The return value is used by the main loop, to
     decide whether the GUI application has changed or not.
  */
  return needUpdate;
}


/* msy */
