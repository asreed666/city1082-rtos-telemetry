#include "mbed.h"
#include "display.h"
#include "control.h"
#include "cy_pdl.h"
#include "cycfg_capsense.h"
#include "cycfg.h"
#include "cybsp.h"
#include "cyhal.h"


extern struct dataSet myData;


/*******************************************************************************
* Global constants
*******************************************************************************/
/* Number of touches on the slider */
#define SLIDER_NUM_TOUCH                        (1u)
#define LED_OFF                                 (1u)
#define LED_ON                                  (0u)

/* Defines periodicity of the CapSense scan and touch processing in
 * milliseconds.
 */
#define CAPSENSE_SCAN_PERIOD_MS                 (20ms)
#define EZI2C_INTERRUPT_PRIORITY                (3u) /* EZI2C interrupt priority must be
                                                      * higher than CapSense interrupt */
                                                    
/* Enable tuner functionality */
#define TUNER_ENABLE                            (1u)


/***************************************
* Function Prototypes
**************************************/
void RunCapSenseScan(void);
static void initialize_capsense_tuner(void);
void ProcessTouchStatus(void);
void CapSense_InterruptHandler(void);
void CapSenseEndOfScanCallback(cy_stc_active_scan_sns_t * ptrActiveScan);


/*******************************************************************************
* Interrupt configuration
*******************************************************************************/
const cy_stc_sysint_t CapSense_ISR_cfg =
{
    .intrSrc = CYBSP_CSD_IRQ,
    .intrPriority = 4u
};


/*******************************************************************************
* Global variables
*******************************************************************************/
DigitalOut ledStatus(LED5);   //(CYBSP_USER_LED);
Semaphore capsense_sem;
EventQueue queue;
cy_stc_scb_ezi2c_context_t EZI2C_context;
uint32_t prevBtn0Status = 0u; 
uint32_t prevBtn1Status = 0u;
uint32_t prevSliderPos = 0u;

/* SysPm callback params */
cy_stc_syspm_callback_params_t callbackParams = 
{
    .base       = CYBSP_CSD_HW,
    .context    = &cy_capsense_context
};

cy_stc_syspm_callback_t capsenseDeepSleepCb = 
{
    Cy_CapSense_DeepSleepCallback,  
    CY_SYSPM_DEEPSLEEP,
    (CY_SYSPM_SKIP_CHECK_FAIL | CY_SYSPM_SKIP_BEFORE_TRANSITION | CY_SYSPM_SKIP_AFTER_TRANSITION),
    &callbackParams,
    NULL, 
    NULL
};

cy_stc_scb_ezi2c_context_t ezi2c_context;
cyhal_ezi2c_t sEzI2C;
cyhal_ezi2c_slave_cfg_t sEzI2C_sub_cfg;
cyhal_ezi2c_cfg_t sEzI2C_cfg;


/*******************************************************************************
* Function Name: handle_error
********************************************************************************
* Summary:
* User defined error handling function
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void handle_error(void)
{
    /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}


/*******************************************************************************
* Function Name: main()
********************************************************************************
* Summary:
*   Main function that starts a thread for CapSense scan and enters a forever
*   wait state. 
*
*******************************************************************************/
void controlThread(void)
{
    cybsp_init();
    
    #if TUNER_ENABLE
    initialize_capsense_tuner();
    sleep_manager_lock_deep_sleep();
    #endif

    /* Initialize the CSD HW block to the default state. */
    cy_status status = Cy_CapSense_Init(&cy_capsense_context);

    if(CY_RET_SUCCESS != status)
    {
        printf("CapSense initialization failed. Status code: %lu\r\n", (unsigned long)status);
        while (true) {
            ThisThread::sleep_for(1000s);
        }
        
    }
    
    /* Initialize CapSense interrupt */
    Cy_SysInt_Init(&CapSense_ISR_cfg, &CapSense_InterruptHandler);
    NVIC_ClearPendingIRQ(CapSense_ISR_cfg.intrSrc);
    NVIC_EnableIRQ(CapSense_ISR_cfg.intrSrc);

    /* Initialize the CapSense firmware modules. */
    Cy_CapSense_Enable(&cy_capsense_context);
    Cy_SysPm_RegisterCallback(&capsenseDeepSleepCb);
    Cy_CapSense_RegisterCallback(CY_CAPSENSE_END_OF_SCAN_E, 
                                 CapSenseEndOfScanCallback, &cy_capsense_context);
    
    /* Create a thread to run CapSense scan periodically using an event queue
     * dispatcher.
     */
    Thread thread(osPriorityNormal, OS_STACK_SIZE, NULL, "CapSense Scan Thread");
    thread.start(callback(&queue, &EventQueue::dispatch_forever));
    queue.call_every(CAPSENSE_SCAN_PERIOD_MS, RunCapSenseScan);

    /* Initiate scan immediately since the first call of RunCapSenseScan()
     * happens CAPSENSE_SCAN_PERIOD_MS after the event queue dispatcher has
     * started. 
     */
    Cy_CapSense_ScanAllWidgets(&cy_capsense_context); 
    printf("Application has started. Touch any CapSense button or slider.\r\n");

    /* The EZI2C pins for the target CYW9P62S1_43012EVB_01 are P1[0] and P1[1].
     * The SCB associated with these pins is not deep sleep wake-up capable.
     * i.e. It cannot wake up the device from deep sleep. To enable
     * communication with the CapSense Tuner, deep sleep is locked for this
     * target. Remove sleep_manager_lock_deep_sleep() function to allow the
     * device to enter deep sleep.
     */

    #ifdef TARGET_CYW9P62S1_43012EVB_01
    sleep_manager_lock_deep_sleep();
    #endif

    while (true) {
        ThisThread::sleep_for(1000s);
    }

}


/*******************************************************************************
* Function Name: RunCapSenseScan()
********************************************************************************
* Summary:
*   This function starts the scan, and processes the touch status. It is
* periodically called by an event dispatcher. 
*
*******************************************************************************/
void RunCapSenseScan(void)
{
    Cy_CapSense_Wakeup(&cy_capsense_context);

    if (CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context))
    {
        /* Device locks the deep sleep untill the scan is completed. */
        Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
         
    }

    capsense_sem.acquire();
    Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);
    
    #if TUNER_ENABLE
    Cy_CapSense_RunTuner(&cy_capsense_context);
    #endif

    ProcessTouchStatus();
}


/*******************************************************************************
* Function Name: initialize_capsense_tuner
********************************************************************************
* Summary:
*  Initializes interface between Tuner GUI and PSoC 6 MCU.
*
*******************************************************************************/
static void initialize_capsense_tuner(void)
{
    cy_rslt_t result;
    /* Configure Capsense Tuner as EzI2C Slave */
    sEzI2C_sub_cfg.buf = (uint8 *)&cy_capsense_tuner;
    sEzI2C_sub_cfg.buf_rw_boundary = sizeof(cy_capsense_tuner);
    sEzI2C_sub_cfg.buf_size = sizeof(cy_capsense_tuner);
    sEzI2C_sub_cfg.slave_address = 8U;

    sEzI2C_cfg.data_rate = CYHAL_EZI2C_DATA_RATE_400KHZ;
    sEzI2C_cfg.enable_wake_from_sleep = true;
    sEzI2C_cfg.slave1_cfg = sEzI2C_sub_cfg;
    sEzI2C_cfg.sub_address_size = CYHAL_EZI2C_SUB_ADDR16_BITS;
    sEzI2C_cfg.two_addresses = false;
    result = cyhal_ezi2c_init(&sEzI2C, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL, &sEzI2C_cfg);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }

}


/*******************************************************************************
* Function Name: ProcessTouchStatus
********************************************************************************
*
* Summary:
*   Controls the LED status according to the status of CapSense widgets and
*   prints the status to serial terminal.
*
*******************************************************************************/
void ProcessTouchStatus(void)
{
    uint32_t currSliderPos;    
    uint32_t currBtn0Status = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON0_WDGT_ID, CY_CAPSENSE_BUTTON0_SNS0_ID, &cy_capsense_context);        
    uint32_t currBtn1Status = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON1_WDGT_ID, CY_CAPSENSE_BUTTON1_SNS0_ID, &cy_capsense_context);       
    cy_stc_capsense_touch_t *sldrTouch = Cy_CapSense_GetTouchInfo(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, &cy_capsense_context);
    
    if(currBtn0Status != prevBtn0Status)
    {
        printf("Button_0 status: %lu\r\n", (unsigned long)currBtn0Status);
        prevBtn0Status = currBtn0Status;
    }
    
    if(currBtn1Status != prevBtn1Status)
    {
        printf("Button_1 status: %lu\r\n", (unsigned long)currBtn1Status);
        prevBtn1Status = currBtn1Status;
    } 

    if (sldrTouch->numPosition == SLIDER_NUM_TOUCH)
    {       
        currSliderPos = sldrTouch->ptrPosition->x;

        if(currSliderPos != prevSliderPos)
        {
            printf("Slider position: %lu\r\n", (unsigned long)currSliderPos);
            prevSliderPos = currSliderPos;
        }
    }

    ledStatus = (currBtn0Status || currBtn1Status || (SLIDER_NUM_TOUCH == sldrTouch->numPosition)) ? LED_ON : LED_OFF;
}


/*******************************************************************************
* Function Name: CapSense_InterruptHandler()
********************************************************************************
* Summary:
*  Wrapper function for handling interrupts from CSD block.
*
*******************************************************************************/
void CapSense_InterruptHandler(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}


/*******************************************************************************
* Function Name: CapSenseEndOfScanCallback()
********************************************************************************
* Summary:
*  This function releases a semaphore to indicate end of a CapSense scan.
*
* Parameters:
*  cy_stc_active_scan_sns_t* : pointer to active sensor details.
*
*******************************************************************************/
void CapSenseEndOfScanCallback(cy_stc_active_scan_sns_t * ptrActiveScan)
{  
    capsense_sem.release();
}


/* [] END OF FILE */
