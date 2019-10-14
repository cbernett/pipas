/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"

#include "nrf_delay.h"
#include "nrf_gpiote.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "BNO055.h"

#include "pipas.h"
#include "logger.h"

#define IS_SRVC_CHANGED_CHARACT_PRESENT 0                                           /**< Include the service_changed characteristic. If not enabled, the server's database cannot be changed for the lifetime of the device. */

#define CENTRAL_LINK_COUNT              0                                           /**<number of central links used by the application. When changing this number remember to adjust the RAM settings*/
#define PERIPHERAL_LINK_COUNT           1                                           /**<number of peripheral links used by the application. When changing this number remember to adjust the RAM settings*/

#define DEVICE_NAME                     "Nordic_UART"                               /**< Name of device. Will be included in the advertising data. */
#define NUS_SERVICE_UUID_TYPE           BLE_UUID_TYPE_VENDOR_BEGIN                  /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_ADV_INTERVAL                64                                          /**< The advertising interval (in units of 0.625 ms. This value corresponds to 40 ms). */
#define APP_ADV_TIMEOUT_IN_SECONDS      180                                         /**< The advertising timeout (in units of seconds). */

#define APP_TIMER_PRESCALER             0                          f                 /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(20, UNIT_1_25_MS)             /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(75, UNIT_1_25_MS)             /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define START_STRING                    "Start...\n"                                /**< The string that will be sent over the UART when the application starts. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

#define LED1 20
#define LED2 29

static ble_nus_t                        m_nus;                                      /**< Structure to identify the Nordic UART Service. */
static uint16_t                         m_conn_handle = BLE_CONN_HANDLE_INVALID;    /**< Handle of the current connection. */

static ble_uuid_t                       m_adv_uuids[] = {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};  /**< Universally unique service identifier. */

void run();
void setup();

#define TIMER            NRF_TIMER1
#define TIMER_IRQn       TIMER1_IRQn
#define TIMER_IRQHandler TIMER1_IRQHandler

static uint32_t timer_seconds;

logger_t* main_logger;

static void timer_start(void)
{
    // Reset the second variable
    timer_seconds = 0;
    
    // Ensure the timer uses 24-bit bitmode or higher
    TIMER->BITMODE = TIMER_BITMODE_BITMODE_24Bit << TIMER_BITMODE_BITMODE_Pos;
    
    // Set the prescaler to 4, for a timer interval of 1 us (16M / 2^4)
    TIMER->PRESCALER = 4;
    
    // Set the CC[0] register to hit after 1 second
    TIMER->CC[0] = 1000000;
    
    // Make sure the timer clears after reaching CC[0]
    TIMER->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    
    // Trigger the interrupt when reaching CC[0]
    TIMER->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    
    // Set a low IRQ priority and enable interrupts for the timer module
    NVIC_SetPriority(TIMER_IRQn, 7);
    NVIC_EnableIRQ(TIMER_IRQn);
    
    // Clear and start the timer
    TIMER->TASKS_CLEAR = 1;
    TIMER->TASKS_START = 1;
}

static uint32_t millis(void)
{
    // Store the current value of the timer in the CC[1] register, by triggering the capture task
    TIMER->TASKS_CAPTURE[1] = 1;
    
    // Combine the state of the second variable with the current timer state, and return the result
    return (timer_seconds * 1000) + (TIMER->CC[1] / 1000);
}

static uint64_t micros(void)
{
    // Store the current value of the timer in the CC[1] register, by triggering the capture task
    TIMER->TASKS_CAPTURE[1] = 1;
    
    // Combine the state of the second variable with the current timer state, and return the result
    return (uint64_t)timer_seconds * 1000000 + TIMER->CC[1];
}

// Timer interrupt handler
void TIMER_IRQHandler(void)
{
    if(TIMER->EVENTS_COMPARE[0])
    {
        TIMER->EVENTS_COMPARE[0] = 0;

        // Increment the second variable
        timer_seconds++;
    }
}

/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyse 
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of 
 *          the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
    
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *) DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_nus    Nordic UART Service structure.
 * @param[in] p_data   Data to be send to UART module.
 * @param[in] length   Length of the data.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_t * p_nus, uint8_t * p_data, uint16_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        while(app_uart_put(p_data[i]) != NRF_SUCCESS);
    }
    while(app_uart_put('\n') != NRF_SUCCESS);
}
/**@snippet [Handling the data received over BLE] */


/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    uint32_t       err_code;
    ble_nus_init_t nus_init;
    
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;
    
    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    uint32_t err_code;
    
    if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;
    
    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;
    
    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;
        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;
        default:
            break;
    }
}


/**@brief Function for the application's SoftDevice event handler.
 *
 * @param[in] p_ble_evt SoftDevice event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t                         err_code;
    
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
            
        case BLE_GAP_EVT_DISCONNECTED:
            err_code = bsp_indication_set(BSP_INDICATE_IDLE);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = BLE_CONN_HANDLE_INVALID;
            break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported
            err_code = sd_ble_gap_sec_params_reply(m_conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(m_conn_handle, NULL, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for dispatching a SoftDevice event to all modules with a SoftDevice 
 *        event handler.
 *
 * @details This function is called from the SoftDevice event interrupt handler after a 
 *          SoftDevice event has been received.
 *
 * @param[in] p_ble_evt  SoftDevice event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    ble_conn_params_on_ble_evt(p_ble_evt);
    ble_nus_on_ble_evt(&m_nus, p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    
}


/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    // Initialize SoftDevice.
    SOFTDEVICE_HANDLER_INIT(NRF_CLOCK_LFCLKSRC_XTAL_20_PPM, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);
        
    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);
    
    // Subscribe for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in]   event   Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}


/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to 
 *          a string. The string will be be sent over BLE when the last character received was a 
 *          'new line' i.e '\n' (hex 0x0D) or if the string has reached a length of 
 *          @ref NUS_MAX_DATA_LENGTH.
 */
/**@snippet [Handling the data received over UART] */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;
    uint32_t       err_code;

    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;

            if ((data_array[index - 1] == '*') || (index >= (BLE_NUS_MAX_DATA_LEN)))
            {
                err_code = ble_nus_string_send(&m_nus, data_array, index);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
                
                index = 0;
            }
            break;

        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}
/**@snippet [Handling the data received over UART] */


/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        RTS_PIN_NUMBER,
        CTS_PIN_NUMBER,
        APP_UART_FLOW_CONTROL_ENABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud38400
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */


/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t scanrsp;

    // Build advertising data struct to pass into @ref ble_advertising_init.
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance = false;
    advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    memset(&scanrsp, 0, sizeof(scanrsp));
    scanrsp.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    scanrsp.uuids_complete.p_uuids  = m_adv_uuids;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_fast_enabled  = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval = APP_ADV_INTERVAL;
    options.ble_adv_fast_timeout  = APP_ADV_TIMEOUT_IN_SECONDS;

    err_code = ble_advertising_init(&advdata, &scanrsp, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER), 
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for placing the application in low power state while waiting for events.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


//void GPIOTE_IRQHandler()
//{
//	nrf_gpio_pin_toggle(LED1);
//	NRF_GPIOTE->EVENTS_IN[0] = 0;
//}

void send_time()
{
	uint32_t current_time = millis();
	static uint8_t buffer[100];
	sprintf((char*)buffer, "Current Time:%d.\n", current_time);
	//ble_nus_string_send(&m_nus, buffer, 20);
}

void send_message()
{
	uint8_t data[1];
	data[0] = 100;
	static uint8_t buffer[200];
	memset(buffer, 0, 200);
	static double heading = 1;
	static double roll = 1;
	static double pitch = 1;
	BNO055_read_euler_angles(&heading, &roll, &pitch);
	sprintf((char*)buffer, "Heading: %f Roll: %f Pitch: %f\n", heading, roll, pitch);
	//ble_nus_string_send(&m_nus, buffer, 20);
	//ble_nus_string_send(&m_nus, buffer + 20, 18);
	//ble_nus_string_send(&m_nus, buffer + 38, 18);
}

void send_euler()
{
	static double heading = 1;
	static double roll = 1;
	static double pitch = 1;
	BNO055_read_euler_angles(&heading, &roll, &pitch);
	char h_buf[20];
	char r_buf[20];
	char p_buf[20];
	sprintf(h_buf, "H:%f\n", heading);
	sprintf(r_buf, "R:%f\n", roll);
	sprintf(p_buf, "P:%f\n", pitch);
	ble_nus_string_send(&m_nus, (uint8_t*)h_buf, 20);
	ble_nus_string_send(&m_nus, (uint8_t*)r_buf, 20);
	ble_nus_string_send(&m_nus, (uint8_t*)p_buf, 20);
}

void send_accel()
{
	static double x;
	static double y;
	static double z;
	BNO055_read_accel(&x, &y, &z);
	char x_buf[20];
	char y_buf[20];
	char z_buf[20];
	sprintf(x_buf, "X:%f\n", x);
	sprintf(y_buf, "Y:%f\n", y);
	sprintf(z_buf, "Z:%f\n", z);
	ble_nus_string_send(&m_nus, (uint8_t*)x_buf, 20);
	//ble_nus_string_send(&m_nus, (uint8_t*)y_buf, 20);
	//ble_nus_string_send(&m_nus, (uint8_t*)z_buf, 20);
}

/**@brief Application main function.
 */
int main(void)
{
    uint32_t err_code;
    bool erase_bonds;
    uint8_t  start_string[] = START_STRING;
	
    // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
    uart_init();
    buttons_leds_init(&erase_bonds);
    ble_stack_init();
    gap_params_init();
    services_init();
    advertising_init();
    conn_params_init();
	
		BNO055_init();
	
		timer_start();
    
		//nrf_gpio_cfg_output(LED1);
		//nrf_gpio_cfg_output(LED2);
		nrf_gpio_cfg_input(9, BUTTON_PULL);
		//nrf_gpiote_event_configure(0, 9, NRF_GPIOTE_POLARITY_HITOLO);
		//NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_IN0_Enabled;
		//NVIC_EnableIRQ(GPIOTE_IRQn);
		//nrf_gpio_pin_clear(LED1);
		//nrf_gpio_pin_clear(LED2);
	
    printf("%s",start_string);

    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
    
		//rf_gpio_pin_clear(LED1);
		//nrf_gpio_pin_clear(LED2);
		
		setup();
    // Enter main loop.
    for (;;)
    {
			send_accel();
			nrf_delay_ms(100);
			//send_euler();
			//nrf_delay_ms(100);
        //power_manage();
				//nrf_gpio_pin_clear(LED1);
				//nrf_gpio_pin_set(LED2);
				//nrf_delay_ms(100);
				//nrf_gpio_pin_set(LED1);
				//nrf_gpio_pin_clear(LED2);
				//nrf_delay_ms(100);
				//if(nrf_gpio_pin_read(9) == 0)
				//{
					//nrf_gpio_pin_set(LED2);
					//nrf_gpio_pin_clear(LED1);
					//send_message();
					//send_time();
				//} else {
					//nrf_gpio_pin_set(LED1);
					//nrf_gpio_pin_clear(LED2);
				//}
				//run();
    }
}


/** 
 * @}
 */

/*------- Variables: Button -------*/
int b_button_active = 0;
int b_long_press_active = 0;

long button_timer = 0;
long long_press_time = 1000;

int button = 9;
int button_read;

int b_write_flash = 0;
int b_bt_send = 0;
/*---------------------------------*/

/*------- Variables: SD -------*/
uint8_t sd_card_present = 0;
char* log_filename;
char* log_file_buffer;
char* backup_filename;
char* backup_file_buffer;
char* log_file;
char* backup_file;
/*-----------------------------*/

unsigned long last_send_time = 0;
unsigned long last_sample_time = 0;
#define SAMPLE_DELAY 100

/*------- Variables: smoothing -------*/
const int num_readings = 2;

float x_readings[num_readings];      // the readings from the input
int x_read_index = 0;              // the index of the current reading
float x_total = 0;                  // the running total
float x_average = 0;                // the average

float y_readings[num_readings];      // the readings from the input
int y_read_index = 0;              // the index of the current reading
float y_total = 0;                  // the running total
float y_average = 0;                // the average

float z_readings[num_readings];      // the readings from the input
int z_read_index = 0;              // the index of the current reading
float z_total = 0;                  // the running total
float z_average = 0;                // the average

uint8_t enable_sd_logging = ENABLE_SD_LOGGING;
uint8_t enable_serial_logging = ENABLE_UART_LOGGING;
uint8_t enable_time_log = ENABLE_TIME_LOG;
uint8_t enableCalculatedValues = ENABLE_CALCULATED_LOG;
uint8_t enableAccel = ENABLE_ACCEL_LOG;
uint8_t enableGyro = ENABLE_GYRO_LOG;
uint8_t enableCompass = ENABLE_MAG_LOG;
uint8_t enableQuat = ENABLE_QUAT_LOG;
uint8_t enableEuler = ENABLE_EULER_LOG;
uint8_t enableHeading = ENABLE_HEADING_LOG;


uint16_t accel_FSR = IMU_ACCEL_FSR;
uint16_t gyro_FSR = IMU_GYRO_FSR;
uint16_t fifo_rate = DMP_SAMPLE_RATE;


uint8_t init_sd()
{
	return 1;
}

void println(const char* s)
{
	
}

void button_condition() {
  if (button_read == 0) {

    if (b_button_active == 0) {
      b_button_active = true;
      button_timer = millis();
    }

    if ((millis() - button_timer > long_press_time) && (b_long_press_active == 0)) {
      b_long_press_active = 1;
      b_bt_send = 1;
    }


  } else {
    if (b_button_active == true) {
      if (b_long_press_active == 1) {
        b_long_press_active = 0;
      } else {
        b_write_flash = ! b_write_flash;
      }

      b_button_active = 1;


    }

  }

  nrf_delay_ms(15);
}

void setup() {
  // put your setup code here, to run once:
  //if ( !init_IMU() )
  //{
   // println("Error connecting to MPU-9250");
    //while (1) ;
  //}
	
  //if ( init_sd() )
  //{
  //  log_filename = "PIPAS.txt";
  //}

  //pinMode(HW_LED_PIN, OUTPUT);
	nrf_gpio_cfg_output(HW_LED_PIN);

  //SerialUSB.begin(SERIAL_BAUD_RATE);
  //Serial1.begin(SERIAL_BAUD_RATE);

  //pinMode(button, INPUT_PULLUP);
	//nrf_gpio_cfg_input(button, NRF_GPIO_PIN_PULLUP);

  //for (int i = 0; i < num_readings; i++) {
  //  x_readings[i] = 0;
  //  y_readings[i] = 0;
  //  z_readings[i] = 0;
  //}

  /*
	if (!SD.exists("/backup")) {
    SD.mkdir("/backup");
    logFile = SD.open("/backup/default.txt", FILE_WRITE);
    logFile.close();
  }
	*/
  
	//digitalWrite(HW_LED_PIN, HIGH);
  nrf_gpio_pin_set(HW_LED_PIN);
	nrf_delay_ms(100);
  nrf_gpio_pin_clear(HW_LED_PIN);
	nrf_delay_ms(100);
	nrf_gpio_pin_set(HW_LED_PIN);
	nrf_delay_ms(100);
  nrf_gpio_pin_clear(HW_LED_PIN);
	nrf_delay_ms(100);
	nrf_gpio_pin_set(HW_LED_PIN);
	nrf_delay_ms(100);
  nrf_gpio_pin_clear(HW_LED_PIN);
	nrf_delay_ms(100);

}

uint8_t b_log_file = 0;

char* next_log_file()
{
	return "logfile.txt";
}

char* flash_open(char* s)
{
	return "logfile.txt";
}

char* flash_close(char* s)
{
}

void flash_remove(char* s)
{
}

void flash_write(char* f, char* s)
{
}

void flash_println(char* f, char* s)
{
}

void uart_write(char* s)
{
}

void uart_println(char* s)
{
	char buffer[20];
	sprintf(buffer, "%s\n", s);
	ble_nus_string_send(&m_nus, (uint8_t*)s, sizeof(*s));
}

uint8_t log_file_available()
{
	return 1;
}

char* log_file_read() 
{
}

int imu_ax;
int imu_ay;
int imu_az;

float imu_roll;
float imu_pitch;
float imu_yaw;

void log_data()
{
  double accel_x; 
  double accel_y; 
  double accel_z; 
	BNO055_read_accel(&accel_x, &accel_y, &accel_z);
	
	double yaw;
	double pitch;
	double roll;
	BNO055_read_accel(&yaw, &roll, &pitch);

  x_total = x_total - x_readings[x_read_index];
  // read from the sensor:
  x_readings[x_read_index] = accel_x;
  // add the reading to the total:
  x_total = x_total + x_readings[x_read_index];
  // advance to the next position in the array:
  x_read_index = x_read_index + 1;

  // if we're at the end of the array...
  if (x_read_index >= num_readings) {
    // ...wrap around to the beginning:
    x_read_index = 0;
  }

  // calculate the average:
  x_average = x_total / num_readings;

  //----------Y DIRECTION----------

  // subtract the last reading:
  y_total = y_total - y_readings[y_read_index];
  // read from the sensor:
  y_readings[y_read_index] = accel_y;
  // add the reading to the total:
  y_total = y_total + y_readings[y_read_index];
  // advance to the next position in the array:
  y_read_index = y_read_index + 1;

  // if we're at the end of the array...
  if (y_read_index >= num_readings) {
    // ...wrap around to the beginning:
    y_read_index = 0;
  }

  // calculate the average:
  y_average = y_total / num_readings;


  //----------Z DIRECTION----------

  // subtract the last reading:
  z_total = z_total - z_readings[z_read_index];
  // read from the sensor:
  z_readings[z_read_index] = accel_z;
  // add the reading to the total:
  z_total = z_total + z_readings[z_read_index];
  // advance to the next position in the array:
  z_read_index = z_read_index + 1;

  // if we're at the end of the array...
  if (z_read_index >= num_readings) {
    // ...wrap around to the beginning:
    z_read_index = 0;
  }

  // calculate the average:
  z_average = z_total / num_readings;

  char imu_log[200];
	sprintf(imu_log, "%lu,%f,%f,%f,%f,%f,%f\n", millis() - last_send_time, yaw, pitch, roll, z_average, y_average, z_average);
	
	/*
  imuLog += String(millis() - lastSendTime) + ',';
  imuLog += String(imu.yaw) + ',' + String(imu.pitch) + ',' + String(imu.roll) + ',';
  imuLog += String(Xaverage) + ',' + String(Yaverage) + ',' + String(Zaverage);
  imuLog += "\n";
	*/
  last_send_time = millis();
	
  // If SD card logging is enabled & a card is plugged in
  //if ( sdCardPresent && WriteSD )
  //{
    //if (imuLog.length() + logFileBuffer.length() >= SD_LOG_WRITE_BUFFER_SIZE)
    //{
      //sdLogString(logFileBuffer); // Log SD buffer
      //logFileBuffer = ""; // Clear SD log buffer
    //}
    // Add new line to SD log buffer
    //logFileBuffer += imuLog;
  //}
	flash_write("main", imu_log);
}

void run() 
{

  button_read = nrf_gpio_pin_read(button);
	if(button_read)
	{
		uart_println("active");
	}
  button_condition();
  if (b_write_flash && !b_log_file) {
    flash_remove("PIPAS.txt");
    backup_filename = next_log_file();
    log_file = flash_open("PIPAS.txt");
    backup_file = flash_open("/backup.txt");

    flash_println("main", "PIPAS v2.1");
    flash_println("main", "Back-up File: ");  
		flash_println("main", "");
    flash_println("main", "Sensor Data");
    flash_println("main", "data list:");
    flash_println("main", "time\tyaw_z\tpitch_y\troll_x\tlinA_x\tlinA_y\tlinA_z\t");


    flash_println("backup", "PIPAS v2.1");
    flash_println("backup", "Back-up File: "); 
		flash_println("backup", "");
    flash_println("backup", "Sensor Data");
    flash_println("backup", "data list:");
    flash_println("backup", "time\tyaw_z\tpitch_y\troll_x\tlinA_x\tlinA_y\tlinA_z\t");
    flash_println("backup", "open File");
  }

  if (millis() - last_sample_time > SAMPLE_DELAY) {
    if ( imu_fifo_available() )
    {
      // Use dmpUpdateFifo to update the ax, gx, mx, etc. values
      if ( imu_dmp_update_fifo() == INV_SUCCESS)
      {
        imu_update(UPDATE_ACCEL);
        imu_compute_euler_angles();

        log_data();

        if (log_file && !b_write_flash) {
          flash_close("main");
          flash_close("backup");
          uart_println("close File");
        }

      }
      last_sample_time = millis();
    }
  }

  if (b_bt_send) {
    b_bt_send = false;
    nrf_gpio_pin_set(HW_LED_PIN);
    nrf_delay_ms(100);
    nrf_gpio_pin_clear(HW_LED_PIN);
    nrf_delay_ms(100);
    nrf_gpio_pin_set(HW_LED_PIN);
    nrf_delay_ms(100);
    nrf_gpio_pin_set(HW_LED_PIN);

    log_file = flash_open("PIPAS.txt");
    uart_write(">");
    while (log_file_available() > 0) {
      uart_write(log_file_read());
    }
    uart_write("<");
    flash_close("logfile");

    nrf_gpio_pin_set(HW_LED_PIN);
    nrf_delay_ms(100);
    nrf_gpio_pin_clear(HW_LED_PIN);

  }
	if(b_write_flash)
	{
		nrf_gpio_pin_set(HW_LED_PIN);
	} else {
		nrf_gpio_pin_clear(HW_LED_PIN);
	}
}
