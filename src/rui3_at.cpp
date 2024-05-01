/**
 * @file rui3_at.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Commands for LoRa P2P and LoRaWAN
 * @version 0.1
 * @date 2024-04-28
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <Arduino.h>
#include "rui3_at.h"

extern "C"
{
#include "string.h"
#include "stdlib.h"
}

char command[1024] = {0};

/*
  @param serial Needs to be an already opened Stream ({Software/Hardware}Serial) to write to and read from.
*/
RUI3::RUI3(Stream &serial1, Stream &serial) : _serial1(serial1), _serial(serial)
{
	/// \todo test if no need to set the timeouts
	// _serial1.setTimeout(5000);
	// _serial.setTimeout(5000);
}

bool RUI3::getVersion()
{
	snprintf(command, 1024, "at+ver=?\r\n");
	return sendRawCommand(command);
}

bool RUI3::getJoinStatus(void)
{
	snprintf(command, 1024, "at+njs=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "AT+NJS=1") != NULL)
	{
		return true;
	}
	return false;
}

String RUI3::getChannelList()
{
	snprintf(command, 1024, "at+mask=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	String result = String(ret);
	result.trim();
	return result;
}

bool RUI3::setDataRate(int rate)
{
	if ((rate < 0) || (rate > 15))
	{
		return false;
	}
	snprintf(command, 1024, "at+dr=%d\r\n", rate);
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint8_t RUI3::getDataRate(void)
{
	snprintf(command, 1024, "at+dr=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		uint8_t _dr = strtol(str_ptr + 1, NULL, 10);
		if ((_dr >= 0) && (_dr <= 15))
		{
			return _dr;
		}
	}
	return NO_RESPONSE;
}

bool RUI3::setClass(int classMode)
{
	switch (classMode)
	{
	case 0:
		snprintf(command, 1024, "at+class=a\r\n");
		break;
	case 1:
		snprintf(command, 1024, "at+class=b\r\n");
		break;
	case 2:
		snprintf(command, 1024, "at+class=c\r\n");
		break;
	default:
		_serial.println("Parameter error");
		return false;
		break;
	}
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint8_t RUI3::getClass(void)
{
	snprintf(command, 1024, "at+class=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		switch (str_ptr[1])
		{
		case 'A':
			return 0;
			break;
		case 'B':
			return 1;
			break;
		case 'C':
			return 2;
			break;
		default:
			return NO_RESPONSE;
			break;
		}
	}
	return NO_RESPONSE;
}

bool RUI3::setRegion(int region)
{
	if (region > 12)
	{
		_serial.println("Parameter error");
		return false;
	}
#if defined DEBUG_MODE
	String REGION;
	switch (region)
	{
	case 0:
		REGION = "EU433";
		break;
	case 1:
		REGION = "CN470";
		break;
	case 2:
		REGION = "RU864";
		break;
	case 3:
		REGION = "IN865";
		break;
	case 4:
		REGION = "EU868";
		break;
	case 5:
		REGION = "US915";
		break;
	case 6:
		REGION = "AU915";
		break;
	case 7:
		REGION = "KR920";
		break;
	case 8:
		REGION = "AS923-1";
		break;
	case 9:
		REGION = "AS923-2";
		break;
	case 10:
		REGION = "AS923-3";
		break;
	case 11:
		REGION = "AS923-4";
		break;
	case 12:
		REGION = "LA915";
		break;
	}
	_serial.println("Requested work region: " + REGION);
#endif
	snprintf(command, 1024, "at+band=%d\r\n", region);
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint8_t RUI3::getRegion(void)
{
	snprintf(command, 1024, "at+band=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		uint8_t _region = strtol(str_ptr + 1, NULL, 10);
		if ((_region >= 0) && (_region <= 12))
		{
			return _region;
		}
	}
	return NO_RESPONSE;
}

void RUI3::sleep(int mode)
{
	if (mode < 0)
	{
		_serial.println("Parameter error");
		return;
	}
	if (mode == 0)
	{
		snprintf(command, 1024, "at+sleep\r\n");
	}
	else
	{
		snprintf(command, 1024, "at+sleep=%d\r\n", mode);
	}
	sendRawCommand(command);
}

bool RUI3::setLPM(int mode)
{
	if (mode > 1)
	{
		_serial.println("Parameter error");
		return false;
	}
	snprintf(command, 1024, "at+lpm=%d\r\n", mode);
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if ((strstr(ret, "Initialization OK") != NULL) || (strstr(ret, "OK") != NULL))
	{
		return true;
	}

	return false;
}

uint8_t RUI3::getLPM(void)
{
	snprintf(command, 1024, "at+lpm=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "AT+LPM=1") != NULL)
	{
		return LPM_ON;
	}
	else if (strstr(ret, "AT+LPM=0") != NULL)
	{
		return LPM_OFF;
	}
	return NO_RESPONSE;
}

bool RUI3::setLPMLevel(int mode)
{
	if ((mode > 2) || mode == 0)
	{
		_serial.println("Parameter error");
		return false;
	}
	snprintf(command, 1024, "at+lpmlvl=%d\r\n", mode);
	sendRawCommand(command);

	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if ((strstr(ret, "Initialization OK") != NULL) || (strstr(ret, "OK") != NULL))
	{
		return true;
	}

	return false;
}

uint8_t RUI3::getLPMLevel(void)
{
	snprintf(command, 1024, "at+lpmlvl=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "AT+LPMLVL=1") != NULL)
	{
		return LPM_LVL_1;
	}
	else if (strstr(ret, "AT+LPMLVL=2") != NULL)
	{
		return LPM_LVL_1;
	}
	return NO_RESPONSE;
}

void RUI3::reset(void)
{
	snprintf(command, 1024, "atz\r\n");
	sendRawCommand(command);
}

bool RUI3::setWorkingMode(int mode)
{
	switch (mode)
	{
	case 0:
		snprintf(command, 1024, "at+nwm=0\r\n");
		sendRawCommand(command);
		break;
	case 1:
		snprintf(command, 1024, "at+nwm=1\r\n");
		sendRawCommand(command);
		break;
	default:
		return false;
	}

	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if ((strstr(ret, "Initialization OK") != NULL) || (strstr(ret, "OK") != NULL))
	{
		return true;
	}

	return false;
}

uint8_t RUI3::getWorkingMode(void)
{
	snprintf(command, 1024, "at+nwm=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "AT+NWM=1") != NULL)
	{
		return LoRaWAN;
	}
	return LoRaP2P;
}

bool RUI3::setJoinMode(int mode)
{
	switch (mode)
	{
	case 0:
		snprintf(command, 1024, "at+njm=0\r\n");
		sendRawCommand(command);
		break;
	case 1:
		snprintf(command, 1024, "at+njm=1\r\n");
		sendRawCommand(command);
		break;
	default:
		Serial.println("Wrong mode");
		return false;
	}
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif

	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint8_t RUI3::getJoinMode(void)
{
	snprintf(command, 1024, "at+njm=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "AT+NJM=1") != NULL)
	{
		return OTAA;
	}
	return ABP;
}

bool RUI3::joinLoRaNetwork(int timeout)
{
	snprintf(command, 1024, "at+join\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif

	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}

	return false;
}

bool RUI3::initOTAA(String devEUI, String appEUI, String appKEY)
{
	if (devEUI.length() == 16)
	{
		_devEUI = devEUI;
	}
	else
	{
		_serial.println("The parameter devEUI is set incorrectly!");
		return false;
	}
	if (appEUI.length() == 16)
	{
		_appEUI = appEUI;
	}
	else
	{
		_serial.println("The parameter appEUI is set incorrectly!");
		return false;
	}
	if (appKEY.length() == 32)
	{
		_appKEY = appKEY;
	}
	else
	{
		_serial.println("The parameter appKEY is set incorrectly!");
		return false;
	}

	snprintf(command, 1024, "at+deveui=%s\r\n", _devEUI.c_str());
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		snprintf(command, 1024, "at+appeui=%s\r\n", _devEUI.c_str());
		sendRawCommand(command);

		recvResponse();
#if defined DEBUG_MODE
		_serial.printf(">> %s\r\n", ret);
#endif
		if (strstr(ret, "OK") != NULL)
		{
			snprintf(command, 1024, "at+appkey=%s\r\n", _appKEY.c_str());
			sendRawCommand(command);

			recvResponse();
#if defined DEBUG_MODE
			_serial.printf(">> %s\r\n", ret);
#endif
			if (strstr(ret, "OK") != NULL)
			{
				return true;
			}
		}
	}

	return false;
}

bool RUI3::getDevEUI(char *eui, uint16_t array_len)
{
	if (array_len < 8)
	{
		return false;
	}
	snprintf(command, 1024, "at+deveui=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		asciiArrayToByte(eui, str_ptr + 1, 8, 16);
		return true;
	}
	return false;
}

bool RUI3::getAppEUI(char *eui, uint16_t array_len)
{
	if (array_len < 8)
	{
		return false;
	}
	snprintf(command, 1024, "at+appeui=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		asciiArrayToByte(eui, str_ptr + 1, 8, 16);
		return true;
	}
	return false;
}

bool RUI3::getAppKey(char *key, uint16_t array_len)
{
	if (array_len < 16)
	{
		return false;
	}
	snprintf(command, 1024, "at+appkey=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		return asciiArrayToByte(key, str_ptr + 1, 16, 34);
	}
	return false;
}

bool RUI3::initABP(String devADDR, String nwksKEY, String appsKEY)
{
	if (devADDR.length() == 8)
	{
		_devADDR = devADDR;
	}
	else
	{
		_serial.println("The parameter devADDR is set incorrectly!");
		return false;
	}
	if (nwksKEY.length() == 32)
	{
		_nwksKEY = nwksKEY;
	}
	else
	{
		_serial.println("The parameter nwksKEY is set incorrectly!");
		return false;
	}
	if (appsKEY.length() == 32)
	{
		_appsKEY = appsKEY;
	}
	else
	{
		_serial.println("The parameter appsKEY is set incorrectly!");
		return false;
	}
	snprintf(command, 1024, "at+devaddr=%s\r\n", _devADDR.c_str());
	sendRawCommand(command);
	recvResponse();
#ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		snprintf(command, 1024, "at+nwkskey=%s\r\n", _nwksKEY.c_str());
		sendRawCommand(command);
		recvResponse();
#if defined DEBUG_MODE
		_serial.printf(">> %s\r\n", ret);
#endif
		if (strstr(ret, "OK") != NULL)
		{
			snprintf(command, 1024, "at+appskey=%s\r\n", _appsKEY.c_str());
			sendRawCommand(command);
			recvResponse();
#if defined DEBUG_MODE
			_serial.printf(">> %s\r\n", ret);
#endif
			if (strstr(ret, "OK") != NULL)
			{
				return true;
			}
		}
	}
	return false;
}

uint32_t RUI3::getDevAddress(void)
{
	snprintf(command, 1024, "at+devaddr=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		uint32_t _addr = strtol(str_ptr + 1, NULL, 16);
		return _addr;
	}
	return NO_RESPONSE;
}

bool RUI3::getAppsKey(char *key, uint16_t array_len)
{
	if (array_len < 16)
	{
		return false;
	}
	snprintf(command, 1024, "at+appskey=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		return asciiArrayToByte(key, str_ptr + 1, 16, 32);
	}
	return false;
}

bool RUI3::getNwsKey(char *key, uint16_t array_len)
{
	if (array_len < 16)
	{
		return false;
	}
	snprintf(command, 1024, "at+nwkskey=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *str_ptr = strstr(ret, "=");
	if (str_ptr != NULL)
	{
		return asciiArrayToByte(key, str_ptr + 1, 16, 32);
	}
	return false;
}

bool RUI3::setConfirmed(int type)
{
	switch (type)
	{
	case 0:
		snprintf(command, 1024, "at+cfm=0\r\n");
		sendRawCommand(command);
		break;
	case 1:
		snprintf(command, 1024, "at+cfm=1\r\n");
		sendRawCommand(command);

		break;
	default:
		return false;
	}
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

uint8_t RUI3::getConfirmed(void)
{
	snprintf(command, 1024, "at+cfm=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "AT+CFM=1") != NULL)
	{
		return CONF;
	}
	return UNCONF;
}

bool RUI3::sendData(int port, char *datahex)
{
	snprintf(command, 1024, "at+send=%d:%s\r\n", port, datahex);
	sendRawCommand(command);

	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RUI3::recvResponse(uint32_t timeout)
{
	ret[0] = 0x00;
	uint16_t ret_index = 0;
	bool rx_ok = false;
	time_t start_listen = millis();
	while ((millis() - start_listen) < timeout)
	{
		if (_serial1.available())
		{
			rx_ok = true;
			ret[ret_index] = _serial1.read();
			if ((ret[ret_index] != '\r') || (ret[ret_index] != '\n'))
			{
				ret_index++;
			}
			ret[ret_index] = 0x00;
		}

		if ((strstr(ret, "+EVT:TX_DONE") != NULL) || (strstr(ret, "+EVT:SEND_CONFIRMED_OK") != NULL))
		{
			// LoRaWAN, wait for RX
			recvRX(5000);
			return true;
		}
		if (strstr(ret, "+EVT:TXP2P DONE") != NULL)
		{
			return true;
		}
		if ((strstr(ret, "OK") != NULL))
		{
			return true;
		}

		if ((strstr(ret, "AT_COMMAND_NOT_FOUND") != NULL) || (strstr(ret, "AT_PARAM_ERROR") != NULL) ||
			(strstr(ret, "SEND_CONFIRMED_FAILED") != NULL) || (strstr(ret, "AT_NO_NETWORK_JOINED") != NULL))
		{
			return false;
		}
		delay(20);
	}
#ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif

	if (!rx_ok)
	{
		snprintf(ret, 1024, "NO_RESPONSE");
	}
	return false;
}

void RUI3::recvRX(uint32_t timeout)
{
	ret[0] = 0x00;
	uint16_t ret_index = 0;
	bool rx_ok = false;
	volatile bool wait_eol = false;
	time_t start_listen = millis();
	bool cont_while = true;
	char rx_byte;

	while (cont_while)
	{
		if (_serial1.available())
		{
			rx_ok = true;
			rx_byte = (char)_serial1.read();
			ret[ret_index] = (char)rx_byte;
			if (wait_eol)
			{
				if ((ret[ret_index] == '\r') || (ret[ret_index] == '\n'))
				{
					// EOL found, break reception
					ret_index++;
					ret[ret_index] = 0x00;
					cont_while = false;
				}
			}

			ret_index++;
			if (ret_index > 1024)
			{
#ifdef DEBUG_MODE
				_serial.printf("Buffer overflow\r\n");
#endif
				cont_while = false;
			}
			ret[ret_index] = 0x00;

			// Check if we got an RX event
			if ((strstr(ret, "+EVT:RX") != NULL) && !wait_eol)
			{
				// RX detected, wait for next \r\n
#ifdef DEBUG_MODE
				_serial.printf("RX found\r\n");
#endif
				wait_eol = true;
			}

			if (strstr(ret, "+EVT:RXP2P_RECEIVE_TIMEOUT") != NULL)
			{
				// P2P RX timeout
#ifdef DEBUG_MODE
				_serial.printf("P2P RX timeout\r\n");
#endif
				cont_while = false;
			}
			delay(20);
		}

		delay(20);

		if (wait_eol)
		{
			if ((millis() - start_listen) > 120000)
			{
				snprintf(ret, 1024, "FAILED_RX");
				cont_while = false;
				break;
			}
		}
		else
		{
			if ((millis() - start_listen) > timeout)
			{
				snprintf(ret, 1024, "NO_RX");
				cont_while = false;
				break;
			}
		}
	}
#ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif

	if (!rx_ok)
	{
		snprintf(ret, 1024, "NO_RX");
	}
	return;
}

void RUI3::flushRX(uint32_t timeout)
{
	ret[0] = 0x00;
	uint16_t ret_index = 0;
	time_t start_listen = millis();
	while ((millis() - start_listen) < timeout)
	{
		if (_serial1.available())
		{
			ret[ret_index] = _serial1.read();
			if ((ret[ret_index] != '\r') || (ret[ret_index] != '\n'))
			{
				ret_index++;
			}
			ret[ret_index] = 0x00;
		}

		if ((strstr(ret, "+EVT:TX_DONE") != NULL) || (strstr(ret, "+EVT:SEND_CONFIRMED_OK") != NULL))
		{
			return;
		}
		if ((strstr(ret, "OK") != NULL))
		{
			return;
		}

		if ((strstr(ret, "AT_COMMAND_NOT_FOUND") != NULL) || (strstr(ret, "AT_PARAM_ERROR") != NULL))
		{
			return;
		}
		delay(20);
	}
	return;
}

bool RUI3::initP2P(p2p_settings *p2p_settings)
{
	snprintf(command, 1024, "at+p2p=%ld:%d:%d:%d:%d:%d\r\n", p2p_settings->freq, p2p_settings->sf, p2p_settings->bw, p2p_settings->cr, p2p_settings->ppl, p2p_settings->txp);
#ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", command);
#endif
	sendRawCommand(command);
	recvResponse();
#ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RUI3::getP2P(p2p_settings *p2p_settings)
{
	// AT+P2P=916100000:7:0:1:8:22

	snprintf(command, 1024, "at+p2p=?\r\n");
	sendRawCommand(command);
	recvResponse();
#ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	char *data_buff = strstr(ret, "AT+P2P=");
	if (data_buff != NULL)
	{
		char *param;
		param = strtok(ret + 7, ":");

		if (param != NULL)
		{
			/* check frequency */
			p2p_settings->freq = strtol(param, NULL, 0);
			/* check SF */
			param = strtok(NULL, ":");
			if (param != NULL)
			{
				p2p_settings->sf = strtol(param, NULL, 0);
				// Check Bandwidth
				param = strtok(NULL, ":");
				if (param != NULL)
				{
					p2p_settings->bw = strtol(param, NULL, 0);
					// Check CR
					param = strtok(NULL, ":");
					if (param != NULL)
					{
						p2p_settings->cr = strtol(param, NULL, 0);
						// Check Preamble length
						param = strtok(NULL, ":");
						if (param != NULL)
						{
							p2p_settings->ppl = strtol(param, NULL, 0);
							// Check TX power
							param = strtok(NULL, ":");
							if (param != NULL)
							{
								p2p_settings->txp = strtol(param, NULL, 0);
							}
						}
					}
				}
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool RUI3::sendP2PData(char *datahex)
{
	snprintf(command, 1024, "at+psend=%s\r\n", datahex);
	// #ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", command);
	// #endif
	sendRawCommand(command);
	recvResponse();
	// #ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
	// #endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RUI3::setP2PCAD(bool enable)
{
	if (enable)
	{
		snprintf(command, 1024, "at+cad=1\r\n");
	}
	else
	{
		snprintf(command, 1024, "at+cad=0\r\n");
	}
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "OK") != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RUI3::getP2PCAD(void)
{
	snprintf(command, 1024, "at+cad=?\r\n");
	sendRawCommand(command);
	recvResponse();
#if defined DEBUG_MODE
	_serial.printf(">> %s\r\n", ret);
#endif
	if (strstr(ret, "AT+CAD=1") != NULL)
	{
		return true;
	}
	return false;
}

bool RUI3::setUARTConfig(int Baud)
{
	snprintf(command, 1024, "at+baud=%d\r\n", Baud);
	sendRawCommand(command);

	return true;
}

bool RUI3::sendRawCommand(char *cmd)
{
	// Flush out the buffer first
	_serial1.print("\r\n");
	flushRX(1000);

#ifdef DEBUG_MODE
	_serial.printf(">> %s\r\n", cmd);
#endif
	_serial1.print(cmd);
	_serial1.flush();
	delay(50);
	return true;
}

bool RUI3::byteArrayToAscii(char *b_array, char *a_array, uint16_t b_array_len, uint16_t a_array_len)
{
	if (a_array_len < (b_array_len * 2) - 1)
	{
		Serial.printf("a_array_size %ld b_array_size %ld\r\n", a_array_len, b_array_len);
		return false;
	}
	for (int index = 0; index < b_array_len; index++)
	{
		sprintf(&a_array[index * 2], "%02X", b_array[index]);
	}
	return true;
}

bool RUI3::asciiArrayToByte(char *b_array, char *a_array, uint16_t b_array_len, uint16_t a_array_len)
{
	if (a_array_len % 2 != 0)
	{
		return false;
	}

	if (b_array_len < (a_array_len / 2) - 1)
	{
		Serial.printf("a_array_size %ld b_array_size %ld\r\n", a_array_len, b_array_len);
		return false;
	}

	char doublet[3];
	for (int index = 0; index < a_array_len; index++)
	{
		doublet[0] = a_array[index * 2];
		doublet[1] = a_array[(index * 2) + 1];
		doublet[2] = 0x00;
		b_array[index] = strtol(doublet, NULL, 16);
		b_array[index + 1] = 0x00;
	}
	return true;
}
