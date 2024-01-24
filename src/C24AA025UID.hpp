#pragma once
#include "types.h"
#include "Wire.h"
#include <Arduino.h>

/// @brief this implmentation of EEPROM driver interfaces with Microchip's C24AA025UID EEPROM over I2C
/// @brief datasheet available @ https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/20005202A.pdf

class C24AA025UID
{
public:
    C24AA025UID(TwoWire *pWire = &Wire) : m_pWire(pWire) {}

    bool init()
    {
        bool bRetVal = false;
        if (m_pWire == nullptr)
        {
            bRetVal = true;
        }
        else
        {
            m_pWire->begin();
            m_pWire->setClock(400000);

            TUInt32 u32Val;
            if (!read(CONST_SERIAL_ADDR_START, (TUInt8 *)&u32Val, sizeof(u32Val)))
            {
                bRetVal = (u32Val == 0);
                TUInt16 u16Val;

                if (read(CONST_MANUFACTURE_ADDR_START, (TUInt8 *)&u16Val, sizeof(u16Val)) || (CONST_DEV_ID != u16Val))
                {
                    bRetVal = true;
                }
            }
            else
            {
                bRetVal = true;
                // Serial.printf("read fail");
            }
        }
        return bRetVal;
    }

    bool write(TUInt32 u32Address, const TUInt8 *u8Data, TUInt32 len)
    {
        bool bRetVal = false;
        int writeLen = len;
        int indexInArray = 0;
        if (u32Address + len < CONST_END_OF_EEPROM_WRITE)
        {
            while (writeLen > 0)
            {
                int lenToWrite = CONST_MAX_WRITE_LEN;
                if (writeLen < lenToWrite)
                {
                    lenToWrite = writeLen;
                }
                bool bOpStatus = writePage(u32Address + indexInArray, u8Data + indexInArray, lenToWrite);
                bRetVal = bRetVal || bOpStatus;
                delay(CONST_WRITE_TIME_IN_MSEC); // wait write done
                writeLen -= lenToWrite;
                indexInArray += lenToWrite;
            }
        }
        else // no write outside of allowed area
        {
            bRetVal = true;
        }
        return bRetVal;
    }

    bool read(TUInt32 u32Address, TUInt8 *u8Data, TUInt32 len)
    {
        bool bRetVal = false;
        int readLen = len;
        int indexInArray = 0;
        if (u32Address + len < CONST_END_OF_EEPROM_READ)
        {
            while (readLen > 0)
            {
                int lenToRead = CONST_MAX_READ_LEN;
                if (readLen < lenToRead)
                {
                    lenToRead = readLen;
                }
                bool bOpStatus = readPage(u32Address + indexInArray, u8Data + indexInArray, lenToRead);
                indexInArray += lenToRead;
                readLen -= lenToRead;

                bRetVal = bRetVal || bOpStatus;
            }
        }
        else // no read outside of allowed length
        {
            bRetVal = true;
        }
        return bRetVal;
    } // end of read()

private:
    static constexpr TUInt32 CONST_MAX_READ_LEN = 8;
    static constexpr TUInt32 CONST_END_OF_EEPROM_READ = 0xFF;
    static constexpr TUInt32 CONST_MAX_WRITE_LEN = 8;
    static constexpr TUInt32 CONST_END_OF_EEPROM_WRITE = 0x7F;
    bool readPage(TUInt16 u16Address, TUInt8 *u8Data, TUInt32 len)
    {
        bool bRetVal = false;
        if (len > 8)
            len = 8;
        Serial.printf("read page %u bytes from address %u \r\n", len, (TUInt32)u16Address);
        writeAddressAndBuffer(u16Address);

        m_pWire->requestFrom(CONST_UI_ADDR, len);
        for (int i = 0; (i < 10) && (m_pWire->available() == 0); i++)
        {
            delayMicroseconds(20);
        }
        if (m_pWire->available() < (int)len)
        {
            bRetVal = true;
        }
        else
        {
            for (TUInt32 i = 0; i < len; i++)
            {
                u8Data[i] = m_pWire->read();
            }
        }
        return bRetVal;
    }

    bool writePage(TUInt16 u16Address, const TUInt8 *u8Data, TUInt32 len)
    {
        Serial.printf("writing %u bytes to address %u\r\n", (TUInt32)len, (TUInt32) u16Address);
        return writeAddressAndBuffer(u16Address, u8Data, len);
    }

    bool writeAddressAndBuffer(TUInt16 u16Address, const TUInt8 *u8Data = nullptr, TUInt32 len = 0)
    {
        bool bRetVal = false;
        m_pWire->beginTransmission(CONST_UI_ADDR);

        m_pWire->write((TUInt8)(u16Address & 0xff));
        // Serial.printf("address 0x%x written\r\n", (TUInt32)u16Address);
        for (TUInt8 i = 0; i < len; i++)
        {
            if (u8Data == nullptr)
            {
                break;
            }
            // Serial.printf("writing %c to 0x%X\r\n", (char)u8Data[i], (TUInt32)(u16Address + i));
            m_pWire->write(u8Data[i]);
        }
        m_pWire->endTransmission();
        return bRetVal;
    }

    bool readBuffer(TUInt8 *pBuff, TUInt32 len)
    {
        TUInt32 u32ReadLen = 0;
        Serial.printf("\r\n\r\n reading buffer len %u\r\n", len);

        m_pWire->requestFrom(CONST_UI_ADDR, len);
        for (int i = 0; (i < 10) && (u32ReadLen < len); i++)
        {
            if (m_pWire->available() > 0)
            {
                while (m_pWire->available() > 0)
                {
                    pBuff[u32ReadLen++] = m_pWire->read();
                    Serial.printf("Read %c\r\n", pBuff[u32ReadLen - 1]);
                }
            }
            else
            {
                delay(20);
            }
        } // end of loop
        return u32ReadLen != len;
    }

private:
    TwoWire *m_pWire = &Wire;
    static constexpr TUInt8 CONST_UI_ADDR = 0b1010000;
    static constexpr TUInt16 CONST_MANUFACTURE_ADDR_START = 0xFA;
    static constexpr TUInt16 CONST_SERIAL_ADDR_START = 0xFC;
    static constexpr TUInt32 CONST_DEV_ID = 0x2941;
    static constexpr TUInt32 CONST_WRITE_TIME_IN_MSEC = 5;
};

// no code after this line
