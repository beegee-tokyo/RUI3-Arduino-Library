/**
 * @file rui3_at.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief A library for controlling RUI3 based RAKwireless modules with AT commands over UART from a host MCU
 * @version 0.1
 * @date 2024-04-28
 *
 * @copyright Copyright (c) 2024
 *
 * Build from of RAK811.h written by Leopold Wang
 * Updated to use RUI3 AT commands
 */
#ifndef _RUI3_H_
#define _RUI3_H_
#include "Arduino.h"

/** No response from WisDuo */
#define NO_RESPONSE 255

/** Get size of an array */
#define ARRAY_SIZE(array) (sizeof((array)) / sizeof((array[0])))

/** LoRaWAN class A */
#define CLASS_A 0
/** LoRaWAN class B */
#define CLASS_B 1
/** LoRaWAN class C */
#define CLASS_C 2

/** LoRaWAN mode */
#define LoRaWAN 1
/** LoRa P2P mode */
#define LoRaP2P 0

/** OTAA join mode */
#define OTAA 1
/** ABP join mode */
#define ABP 0

/** LoRaWAN egions */
#define EU433 0
#define CN470 1
#define RU864 2
#define IN865 3
#define EU868 4
#define US915 5
#define AU915 6
#define KR920 7
#define AS923 8
#define AS923_1 8
#define AS923_2 9
#define AS923_3 10
#define AS923_4 11
#define LA915 12

/** Confirmed packet mode */
#define CONF 1
/** Unconfirmed packet mode */
#define UNCONF 0

/** LPM level 1 */
#define LPM_LVL_1 1
/** LPM level 2 (only RAK3172)*/
#define LPM_LVL_2 2

/** LPM off */
#define LPM_OFF 0
/** LPM level 1 */
#define LPM_ON 1

#define MAX_CMD_LEN (32)
#define MAX_ARGUMENT 25

// Debug output set to 0 to disable app debug output
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

#if DEBUG_MODE > 0
#define MYLOG(tag, ...)                  \
	do                                   \
	{                                    \
		if (tag)                         \
			Serial.printf("[%s] ", tag); \
		Serial.printf(__VA_ARGS__);      \
		Serial.printf("\n");             \
		Serial.flush();                  \
	} while (0)
#else
#define MYLOG(...)
#endif

/** Structure for parsed received string */
typedef struct _stParam
{
	char *argv[MAX_ARGUMENT];
	int argc;
} stParam;

/** Structure for P2P settings */
typedef struct _p2p_settings
{
	uint32_t freq; // Frequency in Hz, 150000000-960000000 Hz
	uint16_t sf;   // Spreading factor 6 - 12
	uint16_t bw;   // Bandwidth 0=125kHz, 1=250kHz, 2=500kHz, 3=7.8kHz, 4=10.4kHz, 5=15.63kHz, 6=20.83kHz, 7=31.25kHz, 8=41.67kHz, 9=62.5kHz
	uint16_t cr;   // Coding Rate 0 = 4/5, 1 = 4/6, 2 = 4/7, 3 = 4/8
	uint16_t ppl;  // Preamble length 2-65535
	uint16_t txp;  // TX power 5 - 22
} p2p_settings;

// #define DEBUG_MODE

/**
 * @brief Class RUI3
 *
 * @example RUI3-AT-ABP.ino
 * @example RUI3-AT-OTAA.ino
 * @example RUI3-AT-P2P.ino
 */
class RUI3
{
public:
	/**
	 * @brief Construct a new RUI3 object
	 * A simplified constructor taking only a Stream ({Software/Hardware}Serial) object.
	 * The serial port should already be initialised when initialising this library.
	 * Serial is used for debug output: host MCU <==> debug console
	 *
	 * ```cpp
	 * RUI3(Stream &serial1, Stream &serial);
	 * ```
	 * @param serial1 Serial for communication with RUI3 module: WisDuo <==> host MCU
	 * @param serial Serial for debug: host MCU <==> debug console -- Defaults to Serial
	 *
	 * @par Usage
	 * @code
	 * RUI3 wisduo(Serial1, Serial);
	 * @endcode
	 */
	RUI3(Stream &serial1, Stream &serial = Serial);

	/**
	 * @brief Enable/Disable Low power mode
	 * See [AT+LPM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-lpm)
	 *
	 * ```cpp
	 * bool setLPM(int mode);
	 * ```
	 * @param mode 0 = LPM off, 1 = LPM on
	 * @return true Success
	 * @return false No response or error response
	 *
	 * @par Usage
	 * @code
	 * if (!wisduo.setLPM(LPM_ON)) // set device LPM on
	 * {
	 * 		Serial.println("Error setting LPM mode.");
	 * }
	 * @endcode
	 */
	bool setLPM(int mode);

	/**    
	 * @brief Get Low power mode status    
	 * See [AT+LPM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-lpm)
	 *    
	 * ```cpp    
	 * uint8_t getLPM(void);    
	 * ```    
	 * @return 0 - LPM off, 1 = LPM on, 255 no response from WisDuo
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getLPM();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("LPM %d = %s\r\n", q_result, q_result == 0 ? "off" : "on");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getLPM(void);

	/**    
	 * @brief Set the LPM level    
	 * See [AT+LPMLVL](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-lpmlvl)
	 *    
	 * ```cpp    
	 * bool setLPMLevel(int mode);    
	 * ```    
	 * @param mode 1 = STOP1 mode, 2 = STOP2 mode (lowest power consumption)    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.setLPMLevel(LPM_LVL_2)) // set device LPM level 2    
	 * {    
	 * 		Serial.println("Error setting LPM level.");    
	 * }    
	 * @endcode
	 */
	bool setLPMLevel(int mode);

	/**    
	 * @brief Get the current LPM level    
	 * See [AT+LPMLVL](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-lpmlvl)
	 *    
	 * ```cpp    
	 * uint8_t getLPMLevel(void);    
	 * ```    
	 * @return uint8_t 1 = LPM level 1, 2 = LPM level 2, 255 no response from WisDuo
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getLPMLevel();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("LPM level %d\r\n", q_result);    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getLPMLevel(void);

	/**    
	 * @brief Set the device to LoRaWAN mode or LoRa P2P mode    
	 * See [AT+NWM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-nwm)    
	 * When switching the device mode, the device will perform a reset
	 *    
	 * ```cpp    
	 * bool setWorkingMode(int mode);    
	 * ```    
	 * @param mode 0 = LoRa P2P, 1 = LoRaWAN mode    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.setWorkingMode(LoRaWAN)) // set WisNode work_mode to LoRaWAN.    
	 * {    
	 * 		Serial.println("set work_mode failed, please reset module.");    
	 * }    
	 * @endcode
	 */
	bool setWorkingMode(int mode);

	/**    
	 * @brief Get the Working Mode status    
	 * See [AT+NWM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-nwm)
	 *    
	 * ```cpp    
	 * uint8_t getWorkingMode(void);    
	 * ```    
	 * @return uint8_t 0 = LoRa P2P, 1 = LoRaWAN mode 255 = no response from WisDuo
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getWorkingMode();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("Mode %d = %s\r\n", q_result, q_result == 0 ? "P2P" : "LoRaWAN");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getWorkingMode(void);

	/**    
	 * @brief Set the Data Rate    
	 * See [AT+DR](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-dr)
	 *    
	 * ```cpp    
	 * bool setDataRate(int rate);    
	 * ```    
	 * @param rate datarate between 0 and 15 see [Data Rate by Region](https://docs.rakwireless.com/RUI3/Appendix/#data-rate-by-region) for available data rates per LoRaWAN region    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.setDataRate(3)) // set datarate 3    
	 * {    
	 * 		Serial.println("Set data rate failed.");    
	 * }    
	 * @endcode
	 */
	bool setDataRate(int rate);

	/**    
	 * @brief Get the data rate    
	 * See [AT+DR](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-dr)
	 *    
	 * ```cpp    
	 * int8_t getDataRate(void);    
	 * ```    
	 * @return uint8_t datarate 0 - 15, 255 = no response from WisDuo
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getDataRate();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("Datarate %d\r\n", q_result);    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getDataRate(void);

	/**    
	 * @brief Set the LoRaWAN device Class    
	 * See [AT+CLASS](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-class)
	 *    
	 * ```cpp    
	 * bool setClass(int classMode);    
	 * ```    
	 * @param classMode 0 -> Class A, 1 -> Class B, 2 -> Class C    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.setClass(CLASS_A)) // set Class A    
	 * {    
	 * 		Serial.println("Set class failed.");    
	 * }    
	 * @endcode
	 */
	bool setClass(int classMode);

	/**    
	 * @brief Get the class    
	 * See [AT+CLASS](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-class)
	 *    
	 * ```cpp    
	 * uint8_t getClass(void);    
	 * ```    
	 * @return uint8_t class 0 -> Class A, 1 -> Class B, 2 -> Class C, 255 = no response from WisDuo
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getClass();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("Class %d (0=A, 1=B, 2=C)\r\n", q_result);    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getClass(void);

	/**    
	 * @brief Set the LoRaWAN Region    
	 * See [AT+BAND](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-band)
	 *    
	 * ```cpp    
	 * bool setRegion(int region);    
	 * ```    
	 * @param region    
	 * 	- EU433 0    
	 * 	- CN470 1    
	 * 	- RU864 2    
	 * 	- IN865 3    
	 * 	- EU868 4    
	 * 	- US915 5    
	 * 	- AU915 6    
	 * 	- KR920 7    
	 * 	- AS923 8    
	 * 	- AS923_1 8    
	 * 	- AS923_2 9    
	 * 	- AS923_3 10    
	 * 	- AS923_4 11    
	 * 	- LA915 12    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.setRegion(AS923_3)) // set region AS923 variant 3    
	 * {    
	 * 		Serial.println("Set Region failed.");    
	 * }    
	 * @endcode
	 */
	bool setRegion(int region);

	/**    
	 * @brief Get the Region    
	 * See [AT+BAND](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-band)
	 *    
	 * ```cpp    
	 * uint8_t getRegion(void);    
	 * ```    
	 * @return uint8_t Region 0 to 12, 255 = no response from WisDuo
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getRegion();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("Region %d\r\n", q_result);    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getRegion(void);

	/**    
	 * @brief Set network join mode OTAA or ABP    
	 * See [AT+NJM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-njm)
	 *    
	 * ```cpp    
	 * bool setJoinMode(int mode);    
	 * ```    
	 * @param mode 0: join a network using over the air activation, 1: join a network using personalization    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.setJoinMode(OTAA)) // set OTAA network join mode    
	 * {    
	 * 		Serial.println("Set join mode failed.");    
	 * }    
	 * @endcode
	 */
	bool setJoinMode(int mode);

	/**    
	 * @brief Get the network join mode    
	 * See [AT+NJM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-njm)
	 *    
	 * ```cpp    
	 * uint8_t getJoinMode(void);    
	 * ```    
	 * @return uint8_t mode 0: join a network using over the air activation, 1: join a network using personalization, 255 = no response from WisDuo
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getJoinMode();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("Join mode %d = %s\r\n", q_result, q_result == OTAA ? "OTAA" : "ABP");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getJoinMode(void);

	/**    
	 * @brief Set package transmission mode    
	 * In confirmed mode, the LNS will acknowledge the packet, in unconfirmed mode is no acknowledgement    
	 * See [AT+CFM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-cfm)
	 *    
	 * ```cpp    
	 * bool setConfirmed(int type);    
	 * ```    
	 * @param type 0->unconfirm, 1->confirm    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.setConfirmed(CONF)) // set Confirmed transmission mode    
	 * {    
	 * 		Serial.println("Set confirmed mode failed.");    
	 * }    
	 * @endcode
	 */
	bool setConfirmed(int type);

	/**    
	 * @brief Get the Confirmed/Unconfirmed packet status    
	 * In confirmed mode, the LNS will acknowledge the packet, in unconfirmed mode is no acknowledgement    
	 * See [AT+CFM](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-cfm)
	 *    
	 * ```cpp    
	 * uint8_t getConfirmed(void);    
	 * ```    
	 * @return uint8_t type 0->unconfirm, 1->confirm
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t q_result = wisduo.getConfirmed();    
	 * if (q_result != NO_RESPONSE)    
	 * {    
	 * 		Serial.printf("Confirm mode %d = %s\r\n", q_result, q_result == CONF ? "confirmed" : "unconfirmed");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint8_t getConfirmed(void);

	/**    
	 * @brief Set LoRaWAN credentials for OTAA join mode    
	 * Before using this command, you must call setJoinMode(0)
	 *    
	 * ```cpp    
	 * bool initOTAA(String devEUI, String appEUI, String appKEY);    
	 * ```    
	 * @param devEUI device EUI as string see [AT+DEVEUI](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-deveui)    
	 * @param appEUI application EUI as string see [AT+APPEUI](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-appeui)    
	 * @param appKEY application key as string see [AT+APPKEY](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-appkey)    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * String DevEUI = "AC1F09FFFE000000";    
	 * String AppEUI = "AC1F09FFFE000000";    
	 * String AppKey = "EFADFF0000004829ACF71E1A6E000000";    
	 * if (!wisduo.initOTAA(DevEUI, AppEUI, AppKey))    
	 * {    
	 * 		Serial.println("Set OTAA credentials failed.");    
	 * }    
	 * @endcode
	 */
	bool initOTAA(String devEUI, String appEUI, String appKEY);

	/**    
	 * @brief Get the DevEUI    
	 * See [AT+DEVEUI](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-deveui)
	 *    
	 * ```cpp    
	 * bool getDevEUI(char *eui, uint16_t array_len);    
	 * ```    
	 * @param eui byte array for DevEUI    
	 * @param array_len length of byte array (8 at least)    
	 * @return true if DevEUI could be obtained    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * char eui_key[34];    
	 * if (wisduo.getDevEUI(eui_key, ARRAY_SIZE(eui_key)))    
	 * {    
	 * 		Serial.print("DevEUI: ");    
	 * 		for (int idx = 0; idx < 8; idx++)    
	 * 		{    
	 * 			Serial.printf("%02X", eui_key[idx]);    
	 * 		}    
	 * 		Serial.println("\r\n");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool getDevEUI(char *eui, uint16_t array_len);

	/**    
	 * @brief Get the App EUI    
	 * See [AT+APPEUI](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-appeui)
	 *    
	 * ```cpp    
	 * bool getAppEUI(char *eui, uint16_t array_len);    
	 * ```    
	 * @param eui byte array for AppEUI    
	 * @param array_len length of byte array (8 at least)    
	 * @return true if AppEUI could be obtained    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * char eui_key[34];    
	 * if (wisduo.getAppEUI(eui_key, ARRAY_SIZE(eui_key)))    
	 * {    
	 * 		Serial.print("AppEUI: ");    
	 * 		for (int idx = 0; idx < 8; idx++)    
	 * 		{    
	 * 			Serial.printf("%02X", eui_key[idx]);    
	 * 		}    
	 * 		Serial.println("\r\n");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool getAppEUI(char *eui, uint16_t array_len);

	/**    
	 * @brief Get the AppKey    
	 * See [AT+APPKEY](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-appkey)
	 *    
	 * ```cpp    
	 * bool getAppKey(char *key, uint16_t array_len);    
	 * ```    
	 * @param key byte array for AppKey    
	 * @param array_len length of byte array (16 at least)    
	 * @return true if AppKey could be obtained    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * char eui_key[34];    
	 * if (wisduo.getAppKey(eui_key, ARRAY_SIZE(eui_key)))    
	 * {    
	 * 		Serial.print("AppKey: ");    
	 * 		for (int idx = 0; idx < 16; idx++)    
	 * 		{    
	 * 			Serial.printf("%02X", eui_key[idx]);    
	 * 		}    
	 * 		Serial.println("\r\n");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool getAppKey(char *key, uint16_t array_len);

	/**    
	 * @brief Set credentials for ABP join mode    
	 * Before using this command, you must call setJoinMode(1)
	 *    
	 * ```cpp    
	 * bool initABP(String devADDR, String nwksKEY, String appsKEY);    
	 * ```    
	 * @param devADDR device address as HEX string see [AT+DEVADDR](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-devaddr)    
	 * @param nwksKEY network Session Key as a HEX string see [AT+NWKSKEY](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-nwkskey)    
	 * @param appsKEY application Session Key as a HEX string see [AT+APPSKEY](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-appskey)    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * String DevAddr = "01360085";    
	 * String AppSKey = "f55a71bcc94ec6498511007c64a06c02";    
	 * String NwSKey = "616a6b21d7fcb25012d62b38a5829725";    
	 * if (!wisduo.initABP(DevAddr, NwSKey, AppSKey))    
	 * {    
	 * 		Serial.println("Set ABP credentials failed.");    
	 * }    
	 * @endcode
	 */
	bool initABP(String devADDR, String nwksKEY, String appsKEY);

	/**    
	 * @brief Get the DevAddr    
	 * See [AT+DEVADDR](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-devaddr)
	 *    
	 * ```cpp    
	 * uint32_t getDevAddress(void);    
	 * ```    
	 * @return uint32_t device address, NO_RESPONSE if device address could not be obtained
	 *    
	 * @par Usage    
	 * @code    
	 * uint32_t dev_addr = wisduo.getDevAddress();    
	 * {    
	 * 	Serial.printf("Device Address: %08X\r\n", dev_addr);    
	 * }    
	 * else    
	 * {    
	 * 	Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	uint32_t getDevAddress(void);

	/**    
	 * @brief Get the AppsKey    
	 * See [AT+APPSKEY](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-appskey)
	 *    
	 * ```cpp    
	 * bool getAppsKey(char *key, uint16_t array_len);    
	 * ```    
	 * @param key byte array for AppsKey    
	 * @param array_len length of byte array (16 at least)    
	 * @return true if AppsKey could be obtained    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * char eui_key[34];    
	 * if (wisduo.getAppsKey(eui_key, ARRAY_SIZE(eui_key)))    
	 * {    
	 * 		Serial.print("AppsKey: ");    
	 * 		for (int idx = 0; idx < 16; idx++)    
	 * 		{    
	 * 			Serial.printf("%02X", eui_key[idx]);    
	 * 		}    
	 * 		Serial.println("\r\n");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool getAppsKey(char *key, uint16_t array_len);

	/**    
	 * @brief Get the NwsKey    
	 * See [AT+NWKSKEY](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-nwkskey)
	 *    
	 * ```cpp    
	 * bool getNwsKey(char *key, uint16_t array_len);    
	 * ```    
	 * @param key byte array for AppsKey    
	 * @param array_len length of byte array (16 at least)    
	 * @return true if NwsKey could be obtained    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * char eui_key[34];    
	 * if (wisduo.getNwsKey(eui_key, ARRAY_SIZE(eui_key)))    
	 * {    
	 * 		Serial.print("NwsKey: ");    
	 * 		for (int idx = 0; idx < 16; idx++)    
	 * 		{    
	 * 			Serial.printf("%02X", eui_key[idx]);    
	 * 		}    
	 * 		Serial.println("\r\n");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool getNwsKey(char *key, uint16_t array_len);

	/**    
	 * @brief Join the network    
	 * See [AT+JOIN](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-join)
	 *    
	 * ```cpp    
	 * bool joinLoRaNetwork(int timeout = 15);    
	 * ```    
	 * @param timeout in seconds    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.joinLoRaNetwork(60))    
	 * {    
	 * 		Serial.println("Join request error, please make sure settings are correct.");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.println("Network join requested");    
	 * }    
	 * @endcode
	 */
	bool joinLoRaNetwork(int timeout = 15);

	/**    
	 * @brief Get the firmware version number of the module.    
	 * See [AT+VER](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-ver)
	 *    
	 * ```cpp    
	 * bool getVersion(void);    
	 * ```    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * wisduo.getVersion(); // get RUI3 firmware version    
	 * wisduo.recvResponse(5000);    
	 * str_ptr = strstr(wisduo.ret, "=");    
	 * if (str_ptr != NULL)    
	 * {    
	 * 		Serial.printf("Ver: %s\r\n", str_ptr + 1);    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %s\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool getVersion(void);

	/**    
	 * @brief Get the Join Status    
	 * See [AT+NJS](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-njs)
	 *    
	 * ```cpp    
	 * bool getJoinStatus(void);    
	 * ```    
	 * @return true if device has joined LoRaWAN network    
	 * @return false if device has not joined LoRaWAN network
	 *    
	 * @par Usage    
	 * @code    
	 * if (!wisduo.getJoinStatus())    
	 * {    
	 * str_ptr = strstr(wisduo.ret, "=");    
	 * if (str_ptr != NULL)    
	 * {    
	 * 		Serial.println("Network not yet joined");    
	 * }    
	 * else    
	 * {    
	 * 		Serial.println("Network joined");    
	 * }    
	 * @endcode
	 */
	bool getJoinStatus(void);

	/**    
	 * @brief Get the current channel list settings    
	 * This feature works only in Regions US915, AU915 or CN470    
	 * See [AT+MASK](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-mask)
	 *    
	 * ```cpp    
	 * String getChannelList(void);    
	 * ```    
	 * @return String List of enabled channels
	 */
	String getChannelList(void);

	/**    
	 * @brief Send data in LoRaWAN mode    
	 * See [AT+SEND](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-send)
	 *    
	 * ```cpp    
	 * bool sendData(int port, char *datahex);    
	 * ```    
	 * @param port fPort number.(1-223)    
	 * @param datahex payload in byte array    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * char buffer[] = "0174016e06688c0767011a087327560902fd98";    
	 * if (wisduo.sendData(1, buffer))    
	 * {    
	 * 	// Wait for TX finished or error    
	 * 	if (wisduo.recvResponse(60000))    
	 * 	{    
	 * 		Serial.printf("TX success - RX: %s\r\n", wisduo.ret);    
	 * 	}    
	 * 	else    
	 * 	{    
	 * 		Serial.printf("TX failed: %s\r\n", wisduo.ret);    
	 * 	}    
	 * }    
	 * else    
	 * {    
	 * 	Serial.println("Error while trying to send a packet");    
	 * }    
	 * @endcode
	 */
	bool sendData(int port, char *datahex);

	/**    
	 * @brief Initialize LoRa P2P mode    
	 * See [AT+P2P](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-p2p)
	 *    
	 * P2P configuration is set in a structure:    
	 * @code    
	 * typedef struct _p2p_settings    
	 * {    
	 * 	uint32_t freq; // Frequency in Hz, 150000000-960000000 Hz    
	 * 	uint16_t sf;   // Spreading factor 6 - 12    
	 * 	uint16_t bw;   // Bandwidth 0=125kHz, 1=250kHz, 2=500kHz, 3=7.8kHz, 4=10.4kHz, 5=15.63kHz, 6=20.83kHz, 7=31.25kHz, 8=41.67kHz, 9=62.5kHz    
	 * 	uint16_t cr;   // Coding Rate 0 = 4/5, 1 = 4/6, 2 = 4/7, 3 = 4/8    
	 * 	uint16_t ppl;  // Preamble length 2-65535    
	 * 	uint16_t txp;  // TX power 5 - 22    
	 * } p2p_settings;    
	 * @endcode    
	 * ```cpp    
	 * bool initP2P(p2p_settings *p2p_settings);    
	 * ```    
	 * @param p2p_settings Pointer to the structure with the P2P settings    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * p2p_settings p2p_sett;    
	 * p2p_sett.freq = 916100000;	// Frequency 916100000 MHz    
	 * p2p_sett.sf = 7;				// Spreading factor 7    
	 * p2p_sett.bw = 0;				// Bandwidth 125kHz    
	 * p2p_sett.cr = 1;				// Coding rate 4/5    
	 * p2p_sett.ppl = 8;			// Preamble length 8    
	 * p2p_sett.txp = 22;			// TX power 22 dBi    
	 * if (!wisduo.initP2P(&p2p_sett))    
	 * {    
	 * 	Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * else    
	 * {    
	 * 	Serial.printf("P2P setup done\r\n");    
	 * }    
	 * @endcode
	 */
	bool initP2P(p2p_settings *p2p_settings);

	/**    
	 * @brief Get current P2P settings    
	 * See [AT+P2P](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-p2p)
	 *    
	 * P2P configuration is returned in structure:    
	 * @code    
	 * typedef struct _p2p_settings    
	 * {    
	 * 	uint32_t freq; // Frequency in Hz, 150000000-960000000 Hz    
	 * 	uint16_t sf;   // Spreading factor 6 - 12    
	 * 	uint16_t bw;   // Bandwidth 0=125kHz, 1=250kHz, 2=500kHz, 3=7.8kHz, 4=10.4kHz, 5=15.63kHz, 6=20.83kHz, 7=31.25kHz, 8=41.67kHz, 9=62.5kHz    
	 * 	uint16_t cr;   // Coding Rate 0 = 4/5, 1 = 4/6, 2 = 4/7, 3 = 4/8    
	 * 	uint16_t ppl;  // Preamble length 2-65535    
	 * 	uint16_t txp;  // TX power 5 - 22    
	 * } p2p_settings;    
	 * @endcode    
	 * @param p2p_settings pointer to settings for P2P configuration    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * p2p_settings p2p_sett;    
	 * if (wisduo.getP2P(&p2p_sett))    
	 * {    
	 * 	Serial.printf("Freq: %d\r\n", p2p_sett.freq);    
	 * 	Serial.printf("SF:   %d\r\n", p2p_sett.sf);    
	 * 	Serial.printf("BW:   %d\r\n", p2p_sett.bw);    
	 * 	Serial.printf("CR:   %d\r\n", p2p_sett.cr);    
	 * 	Serial.printf("PPL:  %d\r\n", p2p_sett.ppl);    
	 * 	Serial.printf("TXP:  %d\r\n", p2p_sett.txp);    
	 * 	}    
	 * 	else    
	 * 	{    
	 * 		Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * 	}    
	 * }    
	 * else    
	 * {    
	 * 	Serial.println("Error while trying to send a packet");    
	 * }    
	 * @endcode
	 */
	bool getP2P(p2p_settings *p2p_settings);

	/**    
	 * @brief Send a data packet over LoRa P2P    
	 * See [AT+PSEND](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-psend)
	 *    
	 * ```cpp    
	 * bool sendP2PData(char *datahex);    
	 * ```    
	 * @param datahex payload in byte array    
	 * @return true Success    
	 * @return false No response or error response    
	 *     
	 * @par Usage    
	 * @code    
	 * char buffer[] = "0174016e06688c0767011a087327560902fd98";    
	 * if (wisduo.sendData(1, buffer))    
	 * {    
	 * 	// Wait for TX finished or error    
	 * 	if (wisduo.recvResponse(60000))    
	 * 	{    
	 * 		Serial.printf("TX success - RX: %s\r\n", wisduo.ret);    
	 * 	}    
	 * 	else    
	 * 	{    
	 * 		Serial.printf("TX failed: %s\r\n", wisduo.ret);    
	 * 	}    
	 * }    
	 * else    
	 * {    
	 * 	Serial.println("Error while trying to send a packet");    
	 * }    
	 * @endcode
	 */
	bool sendP2PData(char *datahex);

	/**    
	 * @brief Enable or disable P2P Channel Activitity Detection    
	 * See [AT+CAD](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-cad)
	 *    
	 * @param enable true = CAD enabled, false = CAD disabled    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * if (wisduo.setP2PCAD(true))    
	 * {    
	 * 	Serial.printf("CAD enabled\r\n");    
	 * }    
	 * else    
	 * {    
	 * 	Serial.printf("Response: %d\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool setP2PCAD(bool enable);

	/**    
	 * @brief Get CAD status (enabled/disabled)
	 *    
	 * @return true CAD enabled    
	 * @return false CAD disabled
	 *    
	 * @par Usage    
	 * @code    
	 * if (wisduo.getP2PCAD())    
	 * {    
	 * 	Serial.printf("CAD enabled\r\n");    
	 * }    
	 * else    
	 * {    
	 * 	Serial.printf("CAD disabled\r\n");    
	 * }    
	 * @endcode
	 */
	bool getP2PCAD(void);

	/**    
	 * @brief Get response to an AT command from the device    
	 * The last response to an AT command is stored in _**`RUI3::ret`**_ for further parsing. See the example codes for detailed usage.
	 *    
	 * ```cpp    
	 * bool recvResponse(uint32_t timeout = 10000);    
	 * ```    
	 * @param timeout time to wait for a response    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * wisduo.getVersion(); // get RUI3 firmware version    
	 * wisduo.recvResponse(5000);    
	 * str_ptr = strstr(wisduo.ret, "=");    
	 * if (str_ptr != NULL)    
	 * {    
	 * 		Serial.printf("Ver: %s\r\n", str_ptr + 1);    
	 * }    
	 * else    
	 * {    
	 * 		Serial.printf("Response: %s\r\n", wisduo.ret);    
	 * }    
	 * @endcode
	 */
	bool recvResponse(uint32_t timeout = 10000);

	/**    
	 * @brief Get RX packet after LoRaWAN TX or LoRa P2P receive command    
	 * The last received RX packet is stored in _**`RUI3::ret`**_ for further parsing. See the example codes for detailed usage.         
	 * In case of LoRa P2P, `recvRX()` can be used to catch any received packet during the specified timeout.         
	 * In case of LoRaWAN Class A, the received data is in _**`RUI3::ret`**_ after the `recvResponse()` call.         
	 * LoRaWAN Class B and C RX is not implemented, but would work similar to LoRa P2P     
	 *    
	 * ```cpp    
	 * void recvRX(uint32_t timeout);    
	 * ```    
	 * @param timeout max wait time for the RX packet
	 *
	 *    
	 * @par Usage    
	 * #### LoRa P2P    
	 * @code    
	 * Serial.printf("Listen for 30 seconds\r\n");    
	 * wisduo.recvRX(30000);    
	 * if (strstr(wisduo.ret, "+EVT:RX") != NULL)    
	 * {    
	 * 	Serial.printf("RX!\r\n");    
	 * 	Serial.printf("%s\r\n", wisduo.ret);    
	 * }    
	 * else    
	 * {    
	 * 	Serial.println("No data received");    
	 * }    
	 * @endcode    
	 * #### LoRaWAN Class A    
	 * Received data is included in the response of the send command    
	 * @code    
	 * if (wisduo.sendData(1, buffer))    
	 * {    
	 * 	// Wait for TX finished or error    
	 * 	Serial.println("Wait for TX/RX result");    
	 * 	if (wisduo.recvResponse(60000))    
	 * 	{    
	 * 		Serial.printf("TX success - RX: %s\r\n", wisduo.ret); // Shows received packet if any    
	 * 	}    
	 * 	else    
	 * 	{    
	 * 		Serial.printf("TX failed: %s\r\n", wisduo.ret);    
	 * 	}    
	 * }    
	 * @endcode
	 */
	void recvRX(uint32_t timeout);

	/**    
	 * @brief Flush the RX buffer in case there is something left
	 *    
	 * ```cpp    
	 * void flushRX(uint32_t timeout = 5000);    
	 * ```    
	 * @param timeout time to wait for a response
	 */
	void flushRX(uint32_t timeout = 5000);

	/**
	 * @brief Set the module in sleep mode. not required if low power mode is enabled)
	 * When the module is in sleep mode, the host can send any character to wake it up.
	 * When the module is awakened, the event response will automatically return through the serial information.
	 * See [AT+SLEEP](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-sleep)
	 *
	 * ```cpp
	 * void sleep(int mode);
	 * ```
	 * @param mode 0 == sleep until new commands is received, > 1 sleep time in milliseconds
	 * @return true Success
	 * @return false No response or error response
	 *
	 * @par Usage
	 * @code
	 * wisduo.sleep(60000); // force sleep mode for 1 minute
	 * @endcode
	 */
	bool sleep(int mode);

	/**    
	 * @brief Reset the RUI3 module    
	 * Communication with the module will be lost    
	 * See [ATZ](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#atz)
	 *    
	 * ```cpp    
	 * void reset(void);    
	 * ```
	 *
	 *    
	 * @par Usage    
	 * @code    
	 * bool wait_for_response(uint32_t timeout)    
	 * {    
	 * 	uint32_t start_time = millis();    
	 * 	bool got_response = false;    
	 * 	while (1)    
	 * 	{    
	 * 		if (Serial1.available())    
	 * 		{    
	 * 			got_response = true;    
	 * 			break;    
	 * 		}    
	 * 		if ((millis() - start_time) > timeout)    
	 * 		{    
	 * 			break;    
	 * 		}    
	 * 	}    
	 * 	return got_response;    
	 * }
	 *    
	 * wisduo.reset(); // force sleep mode    
	 * if (wait_for_response(30000))    
	 * {    
	 * 	wisduo.recvResponse(5000);    
	 * 	Serial.printf("Response:\r\n>>>\r\n%s\r\n<<<\r\n", wisduo.ret);    
	 * }    
	 * else    
	 * {    
	 * 	Serial.println("WisDuo module didn't send any response in 30 seconds");    
	 * }    
	 * @endcode
	 */
	void reset(void);

	/**    
	 * @brief Change RUI3 communication baud rate    
	 * See [AT+BAUD](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual/#at-baud)
	 *    
	 * ```cpp    
	 * bool setUARTConfig(int Baud);    
	 * ```    
	 * @param Baud Serial baud rate.Supports baud rate: 9600  19200  38400  57600  115200  230400  460800  921600.    
	 * @return true Success    
	 * @return false No response or error response
	 */
	bool setUARTConfig(int Baud);

	/**    
	 * @brief Send a raw command to the RUI3 module.    
	 * See [RUI3 AT command manual](https://docs.rakwireless.com/RUI3/Serial-Operating-Modes/AT-Command-Manual)
	 *    
	 * ```cpp    
	 * bool sendRawCommand(char *command);    
	 * ```    
	 * @param command char array with any of the RUI3 AT commands    
	 * @return true Success    
	 * @return false No response or error response
	 *    
	 * @par Usage    
	 * @code    
	 * char buffer[128] = ;    
	 * sprintf(buffer, "AT\r\n"); // Want to send AT command    
	 * wisduo.sendRawCommand(buffer);    
	 * if (wait_for_response(30000))    
	 * {    
	 * 	wisduo.recvResponse(5000);    
	 * 	Serial.printf("Response:\r\n>>>\r\n%s\r\n<<<\r\n", wisduo.ret);    
	 * }    
	 * else    
	 * {    
	 * 	Serial.println("RAK3172 didn't send any response in 30 seconds");    
	 * }    
	 * @endcode
	 */
	bool sendRawCommand(char *command);

	/**    
	 * @brief Convert a byte array into a ASCII HEX string array
	 *    
	 * ```cpp    
	 * bool byteArrayToAscii(char *b_array, char *a_array, uint16_t b_array_len, uint16_t a_array_len);    
	 * ```    
	 * @param b_array byte array to be converted    
	 * @param a_array char array to be filled with the ASCII HEX values, MUST BE DOUBLE SIZE OF THE BYTE ARRAY    
	 * @param b_array_len length of the byte array    
	 * @param a_array_len length of the char array    
	 * @return true if buffer sizes do match    
	 * @return false if buffer sizes do not match
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t b_array[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};    
	 * char a_array[34];    
	 * if (wisduo.byteArrayToAscii((char *)b_array, a_array, ARRAY_SIZE(b_array), ARRAY_SIZE(a_array)))    
	 * {
	 *	Serial.printf("Result: %s\r\n", a_array);    
	 * }    
	 * 	Serial.printf("Result: %s\r\n", a_array);    
	 * {    
	 * 	Serial.println("byteArrayToAscii throw error");    
	 * }    
	 * @endcode
	 */
	bool byteArrayToAscii(char *b_array, char *a_array, uint16_t b_array_len, uint16_t a_array_len);

	/**    
	 * @brief Convert a ASCII HEX string array into a byte array
	 *    
	 * ```cpp    
	 * bool asciiArrayToByte(char *b_array, char *a_array, uint16_t b_array_len, uint16_t a_array_len);    
	 * ```    
	 * @param b_array byte array to be filled with values    
	 * @param a_array char array to be converted    
	 * @param b_array_len length of the byte array    
	 * @param a_array_len length of char array    
	 * @return true if length of char array is even    
	 * @return false if length of char array is odd or the buffer sizes do not match
	 *    
	 * @par Usage    
	 * @code    
	 * uint8_t c_array[16];    
	 * char d_array[34] = "000102030405060708090A0B0C0D0E0F";    
	 * if (wisduo.asciiArrayToByte((char *)c_array, d_array, ARRAY_SIZE(d_array), ARRAY_SIZE(c_array)))    
	 * {
	 *	Serial.printf("Result: %s\r\n", a_array);
	 *	for (int idx = 0; idx < 16; idx++)    
	 * 	{    
	 * 		Serial.printf("%02X,", c_array[idx]);    
	 * 	}    
	 * 	Serial.println("}");    
	 * }    
	 * else    
	 * {    
	 * 	Serial.println("asciiArrayToByte throw error");    
	 * }    
	 * @endcode
	 */
	bool asciiArrayToByte(char *b_array, char *a_array, uint16_t b_array_len, uint16_t a_array_len);

	/** @brief Char array with the last response from the WisDuo module */
	char ret[1024] = {0};

	stParam param;

private:
	Stream &_serial;

	Stream &_serial1;

	String _devADDR = "00112233";

	String _devEUI = "60C5A8FFFE000001";

	String _appEUI = "60C5A8FFFE000001";

	String _nwksKEY = "60C5A8FFFE00000160C5A8FFFE000001";

	String _appKEY = "60C5A8FFFE00000160C5A8FFFE000001";

	String _appsKEY = "60C5A8FFFE00000160C5A8FFFE000001";
};
#endif // _RUI3_H_