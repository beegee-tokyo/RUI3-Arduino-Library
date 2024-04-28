/**
 * @file RUI3-P2P.ino
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief P2P example code
 * @version 0.1
 * @date 2024-04-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <Arduino.h>
#ifdef NRF_52
#include <Adafruit_TinyUSB.h>
#endif

#include <rui3_at.h> // Click to install library: http://librarymanager/All#RUI3-Arduino-Library

RUI3 wisduo(Serial1, Serial);

/** P2P Frequency */
#define P2P_FREQ 916100000
#define P2P_SF 7
#define P2P_BW 0
#define P2P_CR 1
#define P2P_PRLEN 8
#define P2P_PWR 22

char buffer[] = "0174016e06688c0767011a087327560902fd98";
char com_buff[64];

volatile bool continuous_loop = true;
volatile bool breakout_flag = false;

uint32_t send_counter = 1;
uint32_t rx_counter = 0;

char *str_ptr = NULL;

/** For testing the byte array to ASCII hex array */
uint8_t b_array[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
char a_array[34];
/** For testing the ASCII hex array to byte array */
uint8_t c_array[16];
char d_array[34] = "000102030405060708090A0B0C0D0E0F";

void setup()
{
	pinMode(LED_GREEN, OUTPUT);
	digitalWrite(LED_GREEN, HIGH);
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, LOW);

	Serial.begin(115200);
	Serial1.begin(115200);

	time_t serial_timeout = millis();
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			break;
		}
	}
	digitalWrite(LED_GREEN, LOW);
}

void loop()
{
	if (continuous_loop)
	{
		digitalWrite(WB_IO2, HIGH);

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
		Serial.println("Get LPM");
		uint8_t q_result = wisduo.getLPM();
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

		Serial.println("===========================================");
		Serial.println("Get CAD status");
		if (wisduo.getP2PCAD())
		{

			Serial.printf("CAD active\r\n");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
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
		Serial.println("Get P2P parameters");
		if (wisduo.getP2P(&p2p_sett))
		{
			Serial.printf("Freq: %d\r\n", p2p_sett.freq);
			Serial.printf("SF:   %d\r\n", p2p_sett.sf);
			Serial.printf("BW:   %d\r\n", p2p_sett.bw);
			Serial.printf("CR:   %d\r\n", p2p_sett.cr);
			Serial.printf("PPL:  %d\r\n", p2p_sett.ppl);
			Serial.printf("TXP:  %d\r\n", p2p_sett.txp);
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		// Enable P2P CAD
		Serial.println("===========================================");
		Serial.println("Enable CAD");
		if (wisduo.setP2PCAD(true))
		{
			Serial.printf("CAD enabled\r\n");
		}
		else
		{
			Serial.printf("Response: %d\r\n", wisduo.ret);
		}

		// Set to permanent RX with TX enabled
		Serial.println("===========================================");
		Serial.println("Enable continuous RX wit TX enabled");
		sprintf(com_buff, "AT+PRECV=65533\r\n"); // Want to send AT command
		wisduo.sendRawCommand(com_buff);
		wisduo.recvResponse(5000);
		Serial.printf("Response:\r\n>>>\r\n%s\r\n<<<\r\n", wisduo.ret);

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
	}

	// Check if any packet has arrived for 30 seconds
	Serial.println("===========================================");
	Serial.printf("Listen for 30 seconds\r\n");
	wisduo.recvRX(30000);
	if (strstr(wisduo.ret, "+EVT:RX") != NULL)
	{
		Serial.printf("RX!\r\n");
		Serial.printf("%s\r\n", wisduo.ret);
		rx_counter++;
	}

	Serial.println("===========================================");
	Serial.printf("Start send packet %ld received: %ld\r\n", send_counter, rx_counter);

	// Send a packet
	if (wisduo.sendP2PData(buffer))
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
}
