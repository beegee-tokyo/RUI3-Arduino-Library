/**
 * @file RUI3-AT-OTAA.ino
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief OTAA example code
 * @version 0.1
 * @date 2024-04-13
 *
 * @copyright Copyright (c) 2024
 *
 */
// #define LOOP_THROUGH

#include <Arduino.h>
#ifdef NRF_52
#include <Adafruit_TinyUSB.h>
#endif

#include <rui3_at.h> // Click to install library: http://librarymanager/All#RUI3-Arduino-Library

RUI3 wisduo(Serial1);

/** Device Address --- REPLACE WITH YOUR OWN DEVICE ADDRESS */
String DevAddr = "01360085";
/** Application Session Key --- REPLACE WITH YOUR OWN APPLICATION SESSION KEY */
String AppSKey = "f55a71bcc94ec6498511007c64a06c02";
/** Network Session Key --- REPLACE WITH YOUR OWN NETWORK SESSION KEY */
String NwSKey = "616a6b21d7fcb25012d62b38a5829725";

char buffer[] = "0174016e06688c0767011a087327560902fd98";

volatile bool continuous_loop = true;
volatile bool breakout_flag = false;

uint32_t send_counter = 1;
uint32_t fail_counter = 0;

char *str_ptr = NULL;

/** For getting the EUI's and Keys */
char eui_key[34] = {0};
/** For testing the byte array to ASCII hex array */
uint8_t b_array[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
char a_array[34];
/** For testing the ASCII hex array to byte array */
uint8_t c_array[16];
char d_array[34] = "000102030405060708090A0B0C0D0E0F";

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);

#ifdef USE_WB_IO2
	// Only required for WisBlock RAK11200 and RAK4631
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, LOW);
#endif

	Serial.begin(115200);
	Serial1.begin(115200);

	// Serial.setTimeout(5000);
	// Serial1.setTimeout(30000);

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
}

void loop()
{
	if (continuous_loop)
	{
#ifdef USE_WB_IO2
		// Only required for WisBlock RAK11200 and RAK4631
		digitalWrite(WB_IO2, HIGH);
#endif
		Serial.println("===========================================");
		Serial.println("Starting loop-through - exit with 'ESC' key");
		Serial.println("===========================================");

		char rx_char;
		while (!breakout_flag)
		{
			if (Serial.available())
			{
				rx_char = (char)Serial.read();
				if (rx_char == 0x1b)
				{
					breakout_flag = true;
					continuous_loop = false;
				}
				else
				{
					Serial1.write(rx_char);
				}
			}
			if (!breakout_flag)
			{
				if (Serial1.available())
				{
					Serial.write((char)Serial1.read());
				}
			}
		}

		// Clear Serial1 RX buffer
		Serial.flush();
		Serial1.flush();
		Serial.println("\r\nClear Serial1 RX buffer.");
		wisduo.flushRX();

		// Test byte to ascii
		Serial.println("===========================================");
		Serial.println("Convert byte array to ASCII array");
		if (wisduo.byteArrayToAscii((char *)b_array, a_array, ARRAY_SIZE(b_array), ARRAY_SIZE(a_array)))
		{
			Serial.printf("Result: %s\r\n", a_array);
		}
		else
		{
			Serial.println("byteArrayToAscii throw error");
		}

		// Test ascii to byte array
		Serial.println("===========================================");
		Serial.println("Convert ASCII array to byte array");
		if (wisduo.asciiArrayToByte((char *)c_array, d_array, ARRAY_SIZE(d_array), ARRAY_SIZE(c_array)))
		{
			Serial.print("Result: {");
			for (int idx = 0; idx < 16; idx++)
			{
				Serial.printf("%02X,", c_array[idx]);
			}
			Serial.println("}");
		}
		else
		{
			Serial.println("asciiArrayToByte throw error");
		}

		Serial.println("===========================================");
		Serial.println("Get version");
		wisduo.getVersion(); // get RUI3 firmware version
		wisduo.recvResponse(5000);
		str_ptr = strstr(wisduo.ret, "=");
		if (str_ptr != NULL)
		{
			Serial.printf("Ver: %s\r\n", str_ptr + 1);
		}
		else
		{
			Serial.printf("Response: %s\r\n", wisduo.ret);
		}

		// Check current work mode
		if (wisduo.getWorkingMode() == LoRaWAN)
		{
			Serial.println("LORAWAN mode set already");
		}
		else
		{
			Serial.println("Try to set work mode.");
			if (!wisduo.setWorkingMode(LoRaWAN)) // set WisNode work_mode to LoRaWAN.
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
		Serial.println("Get LoRaWAN class");
		uint8_t q_result = wisduo.getClass();
		if (q_result != NO_RESPONSE)
		{
			Serial.printf("Class %d (0=A, 1=B, 2=C)\r\n", q_result);
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get LoRaWAN region");
		q_result = wisduo.getRegion();
		if (q_result != NO_RESPONSE)
		{

			Serial.printf("Region %d\r\n", q_result);
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get DR");
		q_result = wisduo.getDataRate();
		if (q_result != NO_RESPONSE)
		{

			Serial.printf("Datarate %d\r\n", q_result);
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get confirmed/unconfirmed mode");
		q_result = wisduo.getConfirmed();
		if (q_result != NO_RESPONSE)
		{

			Serial.printf("Mode %d = %s\r\n", q_result, q_result == CONF ? "Confirmed" : "Unconfirmed");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get LPM");
		q_result = wisduo.getLPM();
		if (q_result != NO_RESPONSE)
		{

			Serial.printf("LPM %d = %s\r\n", q_result, q_result == 0 ? "off" : "on");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get LPM level");
		q_result = wisduo.getLPMLevel();
		if (q_result != NO_RESPONSE)
		{

			Serial.printf("LPM level %d\r\n", q_result);
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		wisduo.setJoinMode(ABP);

		Serial.println("===========================================");
		Serial.println("Get Join mode");
		q_result = wisduo.getJoinMode();
		if (q_result != NO_RESPONSE)
		{

			Serial.printf("Join mode %d = %s\r\n", q_result, q_result == OTAA ? "OTAA" : "ABP");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		wisduo.setJoinMode(OTAA);

		Serial.println("===========================================");
		Serial.println("Get DevEUI");
		if (wisduo.getDevEUI(eui_key, ARRAY_SIZE(eui_key)))
		{
			Serial.print("DevEUI: ");
			for (int idx = 0; idx < 8; idx++)
			{
				Serial.printf("%02X", eui_key[idx]);
			}
			Serial.println("\r\n");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get AppEUI");
		memset(eui_key, 0, 34);
		if (wisduo.getAppEUI(eui_key, ARRAY_SIZE(eui_key)))
		{
			Serial.print("AppEUI: ");
			for (int idx = 0; idx < 8; idx++)
			{
				Serial.printf("%02X", eui_key[idx]);
			}
			Serial.println("\r\n");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get AppKey");
		memset(eui_key, 0, 34);
		if (wisduo.getAppKey(eui_key, ARRAY_SIZE(eui_key)))
		{
			Serial.print("AppKey: ");
			for (int idx = 0; idx < 16; idx++)
			{
				Serial.printf("%02X", eui_key[idx]);
			}
			Serial.println("\r\n");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		wisduo.setJoinMode(ABP);

		Serial.println("===========================================");
		Serial.println("Get Device Address");
		uint32_t dev_addr = wisduo.getDevAddress();
		if (dev_addr != NO_RESPONSE)
		{
			Serial.printf("Device Address: %08X\r\n", dev_addr);

			// Use device DeviceAddress Key to join
			char dev_addr_array[4];
			dev_addr_array[0] = (char)(dev_addr >> 24);
			dev_addr_array[1] = (char)(dev_addr >> 16);
			dev_addr_array[2] = (char)(dev_addr >> 8);
			dev_addr_array[3] = (char)dev_addr;

			if (!wisduo.byteArrayToAscii(dev_addr_array, buffer, ARRAY_SIZE(dev_addr_array), ARRAY_SIZE(buffer)))
			{
				Serial.print("Error converting NW Session Key to string");
			}
			else
			{
				DevAddr = String(buffer);
				Serial.print("Device Address as String: ");
				Serial.println(DevAddr);
			}
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get App Session Key");
		memset(eui_key, 0, 34);
		if (wisduo.getAppsKey(eui_key, ARRAY_SIZE(eui_key)))
		{
			Serial.print("AppSKey: ");
			for (int idx = 0; idx < 16; idx++)
			{
				Serial.printf("%02X", eui_key[idx]);
			}
			Serial.println("\r\n");

			// Use device App Session Key to join
			if (!wisduo.byteArrayToAscii(eui_key, buffer, 16, 32))
			{
				Serial.print("Error converting App Session Key to string");
			}
			else
			{
				AppSKey = String(buffer);
				Serial.print("App Session Key as String: ");
				Serial.println(AppSKey);
			}
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		Serial.println("===========================================");
		Serial.println("Get Network Session Key");
		memset(eui_key, 0, 34);
		if (wisduo.getNwsKey(eui_key, ARRAY_SIZE(eui_key)))
		{
			Serial.print("NwSKey: ");
			for (int idx = 0; idx < 16; idx++)
			{
				Serial.printf("%02X", eui_key[idx]);
			}
			Serial.println("\r\n");

			// Use device NW Session Key to join
			if (!wisduo.byteArrayToAscii(eui_key, buffer, 16, 32))
			{
				Serial.print("Error converting NW SessionKey to string");
			}
			else
			{
				NwSKey = String(buffer);
				Serial.print("NW Session Key as String: ");
				Serial.println(NwSKey);
			}
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		bool init_success = true;

		// Check current join mode
		if (wisduo.getJoinMode() == ABP)
		{
			Serial.println("ABP mode set already");
		}
		Serial.println("===========================================");
		Serial.println("Set Join Mode");
		if (wisduo.setJoinMode(ABP)) // set join_mode:OTAA
		{
			Serial.println("===========================================");
			Serial.println("Set LoRaWAN region");
			if (wisduo.setRegion(AS923_3)) // set region AS923/3
			{
				Serial.println("===========================================");
				Serial.println("Set LoRaWAN credentials");
				if (wisduo.initABP(DevAddr, NwSKey, AppSKey))
				{
					Serial.println("RUI3 init OK!");
				}
				else
				{
					init_success = false;
				}
			}
			else
			{
				Serial.println("at+band=10 failed");
				init_success = false;
			}
		}
		else
		{
			Serial.println("at+njm=1 failed");
			init_success = false;
		}

		if (!init_success) // init LoRaWAN
		{
			Serial.println("Init error, please reset module.");
			Serial.flush();
			while (1)
			{
				delay(10000);
			};
		}

		Serial.println("===========================================");
		Serial.println("Start Join request");
		if (!wisduo.joinLoRaNetwork(60))
		{
			Serial.println("Join error, please make sure credentials are correct.");
			while (1)
				;
		}
		else
		{
			Serial.println("Network join requested");
		}

		time_t start_wait = millis();
		bool join_success = false;

		Serial.println("===========================================");
		Serial.println("Wait for join");
		uint8_t retry_join = 0;
		while (1)
		{
			if (!wisduo.getJoinStatus())
			{
				Serial.println("Network not yet joined");
				delay(5000);
			}
			else
			{
				Serial.println("Network joined");
				join_success = true;
				break;
			}
			if ((millis() - start_wait) > 30000)
			{
				if (retry_join < 8)
				{
					retry_join++;
					Serial.printf("No join success, retry %d\r\n", retry_join);
					wisduo.joinLoRaNetwork();
				}
				else
				{
					Serial.println("No join success for 8 retries");
					break;
				}
			}
		}

		if (!join_success)
		{
			Serial.println("Join failed, check your credentials");
			while (1)
			{
				delay(5000);
				Serial.print(".");
			}
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

		Serial.println("===========================================");
		Serial.println("Set Datarate");
		if (!wisduo.setDataRate(3)) // set LoRa data rate
		{
			Serial.println("Error setting DR.");
		}

		// Confirmed/unconfirmed packet mode set after join success!
		Serial.println("===========================================");
		Serial.println("Set confirmed packet mode");
		if (!wisduo.setConfirmed(CONF)) // set LoRa data send package type: UNCONF ->unconfirm,  CONF ->confirm
		{
			Serial.println("Error sending packet type.");
		}
	}

	Serial.println("===========================================");
	Serial.printf("Start send packet %ld failed: %ld\r\n", send_counter, fail_counter);
	if (wisduo.sendData(1, buffer))
	{
		// Wait for TX finished or error
		Serial.println("Wait for TX/RX result");
		if (wisduo.recvResponse(60000))
		{
			Serial.printf("TX success - RX: %s\r\n", wisduo.ret);
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
		fail_counter++;
	}
	delay(1000);
}
