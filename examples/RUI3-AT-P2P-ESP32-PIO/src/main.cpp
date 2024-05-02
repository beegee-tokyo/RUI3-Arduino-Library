/**
 * \@file main.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Low power example code for P2P communication between ESP32 and RAK3172
 * @version 0.1
 * @date 2024-04-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <Arduino.h>
#include <Ticker.h>
#include <WiFi.h>

#include <rui3_at.h> // Click to install library: http://librarymanager/All#RUI3-Arduino-Library

/** Wake up events, more events can be defined in app.h */
#define NO_EVENT 0
#define STATUS 0b0000000000000001
#define N_STATUS 0b1111111111111110
#define AT_CMD 0b0000000000000010
#define N_AT_CMD 0b1111111111111101

/** Communication instance for RAK3172 */
RUI3 wisduo(Serial1, Serial);

/** Semaphore used by events to wake up loop task */
SemaphoreHandle_t g_task_sem = NULL;

/** Flag for the event type */
volatile uint16_t g_task_event_type = NO_EVENT;

/** Timer to wakeup task frequently and send message */
Ticker g_task_wakeup_timer;

/** Flag for xSemaphoreGiveFromISR*/
static BaseType_t xHigherPriorityTaskWoken = pdFALSE;

/** Periodic wake up time to send P2P status in milli seconds */
uint32_t send_repeat_time = 30000;

/** P2P Frequency */
#define P2P_FREQ 916100000
#define P2P_SF 7
#define P2P_BW 0
#define P2P_CR 1
#define P2P_PRLEN 8
#define P2P_PWR 22

/** Buffer for outgoing AT commands */
char com_buff[64];
/** For statistics - number of send packets */
uint32_t send_counter = 1;
/** For statistics - number of received packets */
uint32_t rx_counter = 0;
/** Dummy LoRa P2P packet for testing*/
char tx_buffer[] = "0174016e06688c0767011a087327560902fd98";

/** Flag to enable RX callback after setup is finished */
volatile bool loop_active = false;

/**
 * @brief Callback when data from RAK3172 arrived
 */
void usb_rx_cb(void)
{
	// Handle only if setup is finished
	if (loop_active)
	{
		g_task_event_type |= AT_CMD;
		if (g_task_sem != NULL)
		{
			xSemaphoreGiveFromISR(g_task_sem, &xHigherPriorityTaskWoken);
		}
	}
}

/**
 * @brief Timer event that wakes up the loop task frequently
 *
 * @param unused
 */
void periodic_wakeup(void)
{
	// Switch on LED to show we are awake
	digitalWrite(LED_BUILTIN, HIGH);
	g_task_event_type |= STATUS;
	if (g_task_sem != NULL)
	{
		xSemaphoreGiveFromISR(g_task_sem, &xHigherPriorityTaskWoken);
	}
}

/**
 * @brief Arduino setup function. Called once after power-up or reset
 *
 */
void setup()
{
	WiFi.disconnect();
	WiFi.mode(WIFI_OFF);

	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

	// Only for WisBlock
	// pinMode(WB_IO2, OUTPUT);
	// digitalWrite(WB_IO2, HIGH);

	Serial.begin(115200);
	Serial1.begin(115200);

	// Wait for Serial to be available
	time_t serial_timeout = millis();
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		}
		else
		{
			break;
		}
	}
	digitalWrite(LED_BUILTIN, LOW);

	// Create the task event semaphore
	g_task_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(g_task_sem);
	// Take semaphore, so the loop is paused
	xSemaphoreTake(g_task_sem, 10);

	// Setup RAK3172 for LoRa P2P
	// Check current work mode
	Serial.println("===========================================");
	Serial.println("Set LoRa P2P mode");
	if (wisduo.getWorkingMode() == LoRaP2P)
	{
		Serial.println("LoRa P2P mode set already");
	}
	else
	{
		Serial.println("Try to set work mode.");
		if (!wisduo.setWorkingMode(LoRaP2P)) // set WisNode work_mode to LoRa P2P.
		{
			Serial.println("set work_mode failed, please reset module.");
			while (1)
			{
				delay(1000);
			};
		}
		// Module might reset after changing network mode
		// Flush RX buffer
		wisduo.recvResponse(5000);
	}

	Serial.println("===========================================");
	Serial.println("Setup P2P parameters");
	p2p_settings p2p_sett;
	p2p_sett.freq = P2P_FREQ;
	p2p_sett.sf = P2P_SF;
	p2p_sett.bw = P2P_BW;
	p2p_sett.cr = P2P_CR;
	p2p_sett.ppl = P2P_PRLEN;
	p2p_sett.txp = P2P_PWR;
	if (!wisduo.initP2P(&p2p_sett))
	{
		Serial.printf("Response: %d\r\n", wisduo.ret);
	}
	else
	{
		Serial.printf("P2P setup done\r\n");
	}

	Serial.println("===========================================");
	Serial.println("Enable continuous RX with TX enabled");
	sprintf(com_buff, "AT+PRECV=65533\r\n"); // Want to send AT command
	wisduo.sendRawCommand(com_buff);
	wisduo.recvResponse(5000);
	if (strstr(wisduo.ret, "OK") != NULL)
	{
		Serial.printf("P2P RX setup\r\n");
	}
	else
	{
		Serial.printf("Response:\r\n>>>\r\n%s\r\n<<<\r\n", wisduo.ret);
	}
	Serial.println("===========================================");
	Serial.println("Set LPM");
	if (!wisduo.setLPM(LPM_ON)) // set device LPM on
	{
		Serial.println("Error setting LPM mode.");
	}

	Serial.println("===========================================");
	Serial.println("Set LPM level");
	if (!wisduo.setLPMLevel(LPM_LVL_2)) // set device LPM level 2
	{
		Serial.println("Error setting LPM level.");
	}

	g_task_wakeup_timer.attach_ms(send_repeat_time, periodic_wakeup);

	// Register UART RX callback ==> Device will wakeup when RAK3172 sends a response.
	Serial1.onReceive(usb_rx_cb);

	Serial.println("===========================================");
	Serial.println("Start Loop");
	Serial.flush();
	// Take semaphore, so the loop is paused
	xSemaphoreTake(g_task_sem, 10);
	loop_active = true;
}

/**
 * @brief Arduino loop task. Called in a loop from the FreeRTOS task handler
 *	Sleeps until the g_task_sem is made available by either an RX from the RAK3172 UART or a timer
 */
void loop()
{
	// Wait until semaphore is released (FreeRTOS)
	xSemaphoreTake(g_task_sem, portMAX_DELAY);
	// Switch on green LED to show we are awake
	digitalWrite(LED_BUILTIN, HIGH);
	do
	{
		// Serial1 input event
		if ((g_task_event_type & AT_CMD) == AT_CMD)
		{
			g_task_event_type &= N_AT_CMD;
			// Check what arrived on Serial1
			wisduo.recvRX(60000);
			if (strstr(wisduo.ret, "+EVT:RX") != NULL)
			{
				// Switch on blue LED to show we are receiving
				digitalWrite(LED_BUILTIN, HIGH);
				int rx_rssi = 0;
				int rx_snr = 0;
				uint8_t rx_data[512] = {0};
				uint16_t rx_data_len = 0;

				Serial.printf("RX!\r\n");
				// Parse received RX data
				// +EVT:RXP2P:-112:1:1234
				char *data_buff = strtok(wisduo.ret, ":"); // +EVT
				if (data_buff != NULL)
				{
					data_buff = strtok(NULL, ":"); // RXP2P
					if (data_buff != NULL)
					{
						data_buff = strtok(NULL, ":"); // RSSI
						if (data_buff != NULL)
						{
							// get RSSI
							rx_rssi = strtol(data_buff, NULL, 0);

							// Get next value
							data_buff = strtok(NULL, ":"); // SNR
							if (data_buff != NULL)
							{
								// get SNR
								rx_snr = strtol(data_buff, NULL, 0);
							}

							// Get next value
							data_buff = strtok(NULL, ":"); // data
							if (data_buff != NULL)
							{
								// get packet data length
								while (data_buff[rx_data_len] != '\r')
								{
									rx_data_len++;
								}
								// Serial.printf("RX data length = %d\r\n", rx_data_len);
								wisduo.asciiArrayToByte((char *)rx_data, data_buff, 512, rx_data_len);

								// Print out RX packet info
								Serial.println("===========================================");
								Serial.printf("RSSI: %d\r\n", rx_rssi);
								Serial.printf("SNR:  %d\r\n", rx_snr);
								Serial.printf("DATA: ");
								for (int idx = 0; idx < rx_data_len / 2; idx++)
								{
									Serial.printf("%02X", rx_data[idx]);
								}
								Serial.println("");
								rx_counter++;
							}
						}
					}
				}
				else
				{
					Serial.println("Received from WisDuo:");
					Serial.printf("%s\r\n", wisduo.ret);
				}
				// Switch off blue LED to show we are finished parsing RX data
				digitalWrite(LED_BUILTIN, LOW);
			}
			else
			{
				Serial.println("Received from WisDuo:");
				Serial.printf("%s\r\n", wisduo.ret);
			}
			g_task_event_type &= N_AT_CMD;
		}
		// Periodic wake up
		if ((g_task_event_type & STATUS) == STATUS)
		{
			g_task_event_type &= N_STATUS;
			// Do some stuff and send a P2P packet
			Serial.println("===========================================");
			Serial.printf("Start send packet %ld received: %ld\r\n", send_counter, rx_counter);

			// Send a packet
			if (wisduo.sendP2PData(tx_buffer))
			{
				// Wait for TX finished or error
				Serial.println("Wait for TX result");
				if (wisduo.recvResponse(60000))
				{
					Serial.printf("TX success\r\n");
					send_counter++;
				}
				else
				{
					Serial.printf("TX failed: %s\r\n", wisduo.ret);
				}
			}
			else
			{
				Serial.println("Error while trying to send a packet");
				Serial.printf("Response: %s\r\n", wisduo.ret);
			}

			sprintf(com_buff, "AT+PRECV=0\r\n"); // Want to send AT command
			wisduo.sendRawCommand(com_buff);
			wisduo.recvResponse(5000);
			if (strstr(wisduo.ret, "OK") != NULL)
			{
				Serial.printf("P2P RX stopped\r\n");
			}
			else
			{
				Serial.printf("Response:\r\n>>>\r\n%s\r\n<<<\r\n", wisduo.ret);
			}
			sprintf(com_buff, "AT+PRECV=65533\r\n"); // Want to send AT command
			wisduo.sendRawCommand(com_buff);
			wisduo.recvResponse(5000);
			if (strstr(wisduo.ret, "OK") != NULL)
			{
				Serial.printf("P2P RX started\r\n");
			}
			else
			{
				Serial.printf("Response:\r\n>>>\r\n%s\r\n<<<\r\n", wisduo.ret);
			}

			// Clear eventual events coming from the UART callback
			g_task_event_type = NO_EVENT;
		}


		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
	} while (g_task_event_type != NO_EVENT);

	// Switch off green LED to show we are sleeping
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
}
