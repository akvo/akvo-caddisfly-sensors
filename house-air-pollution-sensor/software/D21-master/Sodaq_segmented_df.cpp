/*
 * SQ_dataflash.cpp
 *
 *  Created on: Apr 23, 2014
 *      Author: Kees Bakker
 */


#include <limits.h>
#include <Arduino.h>
#include <Stream.h>
#include <SPI.h>

#include "Sodaq_wdt.h"

#include "Sodaq_segmented_df.h"

// Enable diag by uncommenting the following define
#define ENABLE_SEGMENTED_DF_DIAG 1

#if ENABLE_SEGMENTED_DF_DIAG
  #define myDiagPrint(...) do { if (diag_stream) diag_stream->print(__VA_ARGS__); } while (0)
  #define myDiagPrintLn(...) do { if (diag_stream) diag_stream->println(__VA_ARGS__); } while (0)
#else
  #define myDiagPrint(...)
  #define myDiagPrintLn(...)
#endif


//Dataflash commands
#define FlashPageRead           0xD2    // Main memory page read
#define StatusReg               0xD7    // Status register
#define ReadMfgID               0x9F    // Read Manufacturer and Device ID
#define PageErase               0x81    // Page erase
#define ReadSecReg              0x77    // Read Security Register

#define FlashToBuf1Transfer     0x53    // Main memory page to buffer 1 transfer
#define Buf1Read                0xD4    // Buffer 1 read
#define Buf1ToFlashWE           0x83    // Buffer 1 to main memory page program with built-in erase
#define Buf1Write               0x84    // Buffer 1 write

#define FlashToBuf2Transfer     0x55    // Main memory page to buffer 2 transfer
#define Buf2Read                0xD6    // Buffer 2 read
#define Buf2ToFlashWE           0x86    // Buffer 2 to main memory page program with built-in erase
#define Buf2Write               0x87    // Buffer 2 write

DFlashBuffer1   dfbuf1;
DFlashBuffer2   dfbuf2;

static Stream * diag_stream;

DF_AT45DB081D::DF_AT45DB081D()
{
  _nrPages = 4096;
  _pageAddrBits = 12;
  _pageSize = 264;
  _pageBits = 9;
}

DF_AT45DB161D::DF_AT45DB161D()
{
  _nrPages = 4096;
  _pageAddrBits = 12;
  _pageSize = 528;
  _pageBits = 10;
}

void DFlash::init(uint8_t csPin)
{
    // setup the slave select pin
    _csPin = csPin;

    // This is used when CS != SS
    pinMode(_csPin, OUTPUT);
    deactivate();
}

void DFlash::init(uint8_t misoPin, uint8_t mosiPin, uint8_t sckPin, uint8_t csPin)
{
    init(csPin);
}

void DFlash::setDiagStream(Stream * stream)
{
    diag_stream = stream;
}
void DFlash::setDiagStream(Stream & stream)
{
    diag_stream = &stream;
}

uint8_t DFlash::transmit(uint8_t data)
{
    // Call the standard SPI transfer method
    return SPI.transfer(data);
}

uint8_t DFlash::readStatus()
{
    unsigned char result;

    activate();
    result = transmit(StatusReg);
    result = transmit(0x00);
    deactivate();

    return result;
}

// Monitor the status register, wait until busy-flag is high
void DFlash::waitTillReady()
{
    while (!(readStatus() & 0x80)) {
        // WDT reset maybe??
    }
}

void DFlash::readID(uint8_t *data)
{
    activate();
    transmit(ReadMfgID);
    data[0] = transmit(0x00);
    data[1] = transmit(0x00);
    data[2] = transmit(0x00);
    data[3] = transmit(0x00);
    deactivate();
}

// Reads a number of bytes from one of the Dataflash security register
void DFlash::readSecurityReg(uint8_t *data, size_t size)
{
    activate();
    transmit(ReadSecReg);
    transmit(0x00);
    transmit(0x00);
    transmit(0x00);
    for (size_t i = 0; i < size; i++) {
        *data++ = transmit(0x00);
    }
    deactivate();
}

void DFlash::pageErase(uint16_t pageNr)
{
    //myDiagPrint(F("DFlash::pageErase - page ")); myDiagPrintLn(pageNr);
    activate();
    transmit(PageErase);
    setPageAddr(pageNr);
    deactivate();
    waitTillReady();
}

void DFlash::chipErase()
{
    activate();
    transmit(0xC7);
    transmit(0x94);
    transmit(0x80);
    transmit(0x9A);
    deactivate();
    waitTillReady();
}

void DFlash::deactivate()
{
    digitalWrite(_csPin, HIGH);
    SPI.endTransaction();
}
void DFlash::activate()
{
    SPI.beginTransaction(_settings);
    digitalWrite(_csPin, LOW);
}

void DFlash::setPageAddr(unsigned int pageNr)
{
    transmit(getPageAddrByte0(pageNr));
    transmit(getPageAddrByte1(pageNr));
    transmit(getPageAddrByte2(pageNr));
}

/*
 * From the AT45DB081D documentation (other variants are not really identical)
 *   "For the DataFlash standard page size (264-bytes), the opcode must be
 *    followed by three address bytes consist of three don’t care bits,
 *    12 page address bits (PA11 - PA0) that specify the page in the main
 *    memory to be written and nine don’t care bits."
 */
/*
 * From the AT45DB161B documentation
 *   "For the standard DataFlash page size (528 bytes), the opcode must be
 *    followed by three address bytes consist of 2 don’t care bits, 12 page
 *    address bits (PA11 - PA0) that specify the page in the main memory to
 *    be written and 10 don’t care bits."
 */
uint8_t DFlash::getPageAddrByte0(uint16_t pageNr)
{
    // More correct would be to use a 24 bits number
    // shift to the left by number of bits. But the uint16_t can be considered
    // as if it was already shifted by 8.
    return (pageNr << (_pageBits - 8)) >> 8;
}
uint8_t DFlash::getPageAddrByte1(uint16_t page)
{
    return page << (_pageBits - 8);
}
uint8_t DFlash::getPageAddrByte2(uint16_t page)
{
    return 0;
}

// Transfers a page from flash to Dataflash SRAM buffer
void DFlash::readPageToBuf(uint8_t cmd, uint16_t pageNr)
{
    activate();
    transmit(cmd);
    setPageAddr(pageNr);
    deactivate();
    waitTillReady();
}

// Transfers Dataflash SRAM buffer 1 to flash page
void DFlash::writeBufToPage(uint8_t cmd, uint16_t pageNr)
{
    activate();
    transmit(cmd);
    setPageAddr(pageNr);
    deactivate();
    waitTillReady();
}

// Reads one byte from one of the Dataflash internal SRAM buffer 1
uint8_t DFlash::readByte(uint8_t cmd, uint16_t addr)
{
    unsigned char data = 0;

    activate();
    transmit(cmd);
    transmit(0x00);               //don't care
    transmit((uint8_t) (addr >> 8));
    transmit((uint8_t) (addr));
    transmit(0x00);               //don't care
    data = transmit(0x00);        //read byte
    deactivate();

    return data;
}

// Reads a number of bytes from one of the Dataflash internal SRAM buffer 1
void DFlash::readStr(uint8_t cmd, uint16_t addr, uint8_t *data, size_t size)
{
    activate();
    transmit(cmd);
    transmit(0x00);               //don't care
    transmit((uint8_t) (addr >> 8));
    transmit((uint8_t) (addr));
    transmit(0x00);               //don't care
    for (size_t i = 0; i < size; i++) {
        *data++ = transmit(0x00);
    }
    deactivate();
}

// Writes one byte to one to the Dataflash internal SRAM buffer 1
void DFlash::writeByte(uint8_t cmd, uint16_t addr, uint8_t data)
{
    activate();
    transmit(cmd);
    transmit(0x00);               //don't care
    transmit((uint8_t) (addr >> 8));
    transmit((uint8_t) (addr));
    transmit(data);               //write data byte
    deactivate();
}

// Writes a number of bytes to one of the Dataflash internal SRAM buffer 1
void DFlash::writeStr(uint8_t cmd, uint16_t addr, uint8_t *data, size_t size)
{
    activate();
    transmit(cmd);
    transmit(0x00);               //don't care
    transmit((uint8_t) (addr >> 8));
    transmit((uint8_t) (addr));
    for (size_t i = 0; i < size; i++) {
        transmit(*data++);
    }
    deactivate();
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

static void print02x(uint8_t val)
{
    if (val < 0x10) {
        myDiagPrint('0');
    }
    myDiagPrint(val, HEX);
}

static void dumpBuffer(const uint8_t * buf, size_t size)
{
    while (size > 0) {
        size_t size1 = size >= 16 ? 16 : size;
        for (size_t j = 0; j < size1; j++) {
            print02x(*buf++);
        }
        myDiagPrintLn();
        size -= size1;
    }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

DFlashBuffer1::DFlashBuffer1()
{
    // _df
    _curPage = 0xffff;            // This means "unknown"
    _flashToBufCmd = FlashToBuf1Transfer;
    _bufToFlashCmd = Buf1ToFlashWE;
    _bufReadCmd = Buf1Read;
    _bufWriteCmd = Buf1Write;
}

DFlashBuffer2::DFlashBuffer2()
{
    // _df
    _curPage = 0xffff;            // This means "unknown"
    _flashToBufCmd = FlashToBuf2Transfer;
    _bufToFlashCmd = Buf2ToFlashWE;
    _bufReadCmd = Buf2Read;
    _bufWriteCmd = Buf2Write;
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

// Transfers a page from flash to Dataflash SRAM buffer
void DFlashBuffer::readPageToBuf(uint16_t pageNr, bool forced)
{
    if (forced || _curPage != pageNr) {
        //myDiagPrint(F("DFlashBuffer::readPageToBuf - page ")); myDiagPrintLn(pageNr);
        _df->readPageToBuf(_flashToBufCmd, pageNr);
        _curPage = pageNr;
        //dumpBuffer();
    }
}

// Transfers Dataflash SRAM buffer 1 to flash page
void DFlashBuffer::writeBufToPage(uint16_t pageNr)
{
    myDiagPrint(F("DFlashBuffer::writeBufToPage - page "));
    myDiagPrintLn(pageNr);
    _df->writeBufToPage(_bufToFlashCmd, pageNr);
}

// Reads one byte from one of the Dataflash internal SRAM buffer 1
uint8_t DFlashBuffer::readByte(uint16_t addr)
{
    return _df->readByte(_bufReadCmd, addr);
}

// Reads a number of bytes from one of the Dataflash internal SRAM buffer 1
void DFlashBuffer::readStr(uint16_t addr, uint8_t *data, size_t size)
{
    _df->readStr(_bufReadCmd, addr, data, size);
}

// Writes one byte to one to the Dataflash internal SRAM buffer 1
void DFlashBuffer::writeByte(uint16_t addr, uint8_t data)
{
    _df->writeByte(_bufWriteCmd, addr, data);
}

// Writes a number of bytes to one of the Dataflash internal SRAM buffer 1
void DFlashBuffer::writeStr(uint16_t addr, uint8_t *data, size_t size)
{
    _df->writeStr(_bufWriteCmd, addr, data, size);
}

void DFlashBuffer::dumpBuffer()
{
    uint8_t buf[16];
    for (uint16_t addr = 0; addr < pageSize(); addr += sizeof(buf)) {
        sodaq_wdt_reset();
        readStr(addr, buf, sizeof(buf));
        ::dumpBuffer(buf, sizeof(buf));
    }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void DFlashSegment::init(DFlash & df, enum DFlashBuffer::DFBufferKind kind, uint16_t startPage,
        uint16_t nrPages)
{
    //myDiagPrint(F("DFlashSegment::init - this ")); myDiagPrintLn((uint16_t)this, HEX);
    // Only simple validation. The caller is responsible.
    if (startPage + nrPages <= df.nrPages()) {
        _startPage = startPage;
        _nrPages = nrPages;
        switch (kind) {
        case DFlashBuffer::DFBuf1:
            _dfbuf = &dfbuf1;
            break;
        case DFlashBuffer::DFBuf2:
            _dfbuf = &dfbuf2;
            break;
        default:
            // FATAL
            return;
            break;
        }
        _dfbuf->setDf(df);
    } else {
        myDiagPrint(F("DFlashSegment::init - startPage "));
        myDiagPrintLn(startPage);
        myDiagPrint(F("DFlashSegment::init - nrPages "));myDiagPrintLn(nrPages);
    }
}

/*
 * Erase all pages in this segment
 */
void DFlashSegment::erase()
{
    for (uint16_t pageNr = 0; pageNr <  _nrPages; ++pageNr) {
        pageErase(pageNr);
    }
}

void DFlashSegment::pageErase(uint16_t pageNr)
{
    //myDiagPrint(F("DFlashSegment::pageErase - page ")); myDiagPrintLn(pageNr);
    if (_dfbuf && pageNr < _nrPages) {
        _dfbuf->pageErase(_startPage + pageNr);
    }
}

void DFlashSegment::readPageToBuf(uint16_t pageNr, bool forced)
{
    if (_dfbuf && pageNr < _nrPages) {
        _dfbuf->readPageToBuf(_startPage + pageNr, forced);
    }
}

void DFlashSegment::writeBufToPage(uint16_t pageNr)
{
    myDiagPrint(F("DFlashSegment::writeBufToPage - page "));
    myDiagPrintLn(pageNr);
    if (_dfbuf && pageNr < _nrPages) {
        _dfbuf->writeBufToPage(_startPage + pageNr);
    }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

void DFlashStream::init(DFlashSegment & segment)
{
    _dfs = &segment;
    init();
}

void DFlashStream::init()
{
    _readByteNr = 0;
    _readPageNr = 0;
    _writeByteNr = 0;
    _writePageNr = 0;

    uint32_t pos = findEnd(_writePageNr, _writeByteNr);
    if (pos == (uint32_t) -1) {
        // The stream segment is completely full.
        // Create an empty page at the start, and increment the read position
        nextWritePage();
    }

    // Startup by filling our (flash)buffer with the first page
    _dfs->readPageToBuf(_readPageNr, true);
}

void DFlashStream::erase()
{
    _dfs->erase();
    init();
}

/*
 * Return the number of available bytes
 *
 * This is trickier than you may think. We have a write position
 * and a read position. Subtracting gives us a 32 bit signed number.
 * Stream.available() is defined as a int returning function, so we
 * will do clipping to avoid overflow.
 *
 * Notice that AVR has 16 bit ints, and SAMD has 32 bit ints. In the
 * case of SAMD the overflow clipping won't work.
 */
int DFlashStream::available()
{
    uint32_t wpos = writePos();
    uint32_t rpos = readPos();

    int32_t nrBytes = wpos - rpos;
    if (wpos < rpos) {
        nrBytes += (uint32_t) _dfs->nrPages() * _dfs->pageSize();
    }
    if (nrBytes > INT_MAX) {
        return INT_MAX;
    }
    return nrBytes;
}

int DFlashStream::read()
{
    // Make sure the buffer is filled with the page data
    readPageToBuf(_readPageNr);

    // Read the next byte
    uint8_t b = _dfs->readByte(_readByteNr);

    // Advance the pointer, and read the next page if we're at the end of this one
    if (++_readByteNr >= _dfs->pageSize()) {
        sodaq_wdt_reset();
        _readByteNr = 0;
        _readPageNr = _dfs->nextPage(_readPageNr);
        _dfs->readPageToBuf(_readPageNr, true);
    }
    return b;
}

int DFlashStream::peek()
{
    return _dfs->readByte(_readByteNr);
}

void DFlashStream::flush()
{
    // Do nothing.
}

size_t DFlashStream::write(uint8_t b)
{
    // This is too much diag  myDiagPrint((char )b);

    // Make sure the buffer is filled with the page data
    readPageToBuf(_writePageNr);

    // Write the byte to the buffer
    _dfs->writeByte(_writeByteNr, b);
    _writePageDirty = true;

    // Advance the write pointer
    if (++_writeByteNr >= _dfs->pageSize()) {
        sodaq_wdt_reset();
        writeBufToPage(_writePageNr);
        _writeByteNr = 0;
        nextWritePage();
    }
    return 1;
}

uint32_t DFlashStream::readPos()
{
    return (uint32_t) _readPageNr * _dfs->pageSize() + _readByteNr;
}

void DFlashStream::setReadPos(uint32_t pos)
{
    _readPageNr = pos / _dfs->pageSize();
    _readByteNr = pos - _readPageNr * _dfs->pageSize();
}

uint32_t DFlashStream::writePos()
{
    return (uint32_t) _writePageNr * _dfs->pageSize() + _writeByteNr;
}

void DFlashStream::setWritePos(uint32_t pos)
{
    _writePageNr = pos / _dfs->pageSize();
    _writeByteNr = pos - _writePageNr * _dfs->pageSize();
}

void DFlashStream::readPageToBuf(uint16_t pageNr)
{
    if (pageNr != _writePageNr) {
        if (_writePageDirty) {
            // We must flush the write page to the dataflash
            writeBufToPage(_writePageNr);
        }
    }

    _dfs->readPageToBuf(pageNr);
}

void DFlashStream::writeBufToPage(uint16_t pageNr)
{
    _dfs->writeBufToPage(pageNr);
    _writePageDirty = false;
}

/*
 * Find the end of the stream data
 *
 * We assume that the data only contains ASCII, and either '\0' or 0xFF
 * is taken as the end.
 */
uint32_t DFlashStream::findEnd(uint16_t & pageNr, uint16_t & byteNr)
{
    uint32_t pos = 0;
    uint8_t b;
    for (pageNr = 0; pageNr < _dfs->nrPages(); ++pageNr) {
        _dfs->readPageToBuf(pageNr);
        for (byteNr = 0; byteNr < _dfs->pageSize(); ++byteNr) {
            b = _dfs->readByte(byteNr);
            if (b == 0 || b == 0xFF) {
                return pos;
            }
            ++pos;
        }
    }

    // The segment is full.
    // pageNr points at next page AFTER this segment
    byteNr = 0;

    return (uint32_t) -1;
}

/*
 * Increment the write
 */
void DFlashStream::nextWritePage()
{
    _writePageNr = _dfs->nextPage(_writePageNr);
    // Wipe out this page
    pageErase(_writePageNr);
    if (_writePageNr == _readPageNr) {
        // Bump the read page
        _readPageNr = _dfs->nextPage(_readPageNr);
        _readByteNr = 0;
    }
}

void DFlashStream::writeAvailableToStream(Stream * output)
{
    int c;
    while (available() > 0) {
        c = read();
        if (c == 0xff) {
            break;
        }
        output->print((char) c);
    }
}

void DFlashStream::writeAvailableToStream(Stream & output)
{
    writeAvailableToStream(&output);
}

void DFlashStream::writeToStream(Stream & output, uint32_t startPos)
{
    uint32_t curPos = readPos();
    setReadPos(startPos);
    writeAvailableToStream(output);
    setReadPos(curPos);
}
