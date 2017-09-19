/*
 * Sodaq_segmented_df.h
 *
 *  Created on: Apr 23, 2014
 *      Author: Kees Bakker
 *
 * This module provides a general purpose interface to utilize
 * the dataflash of the SODAQ boards.
 *
 * The software is done in several layers (classes).
 *  - DFlash, class to deal with the dataflash chip
 *  - DFlashBuffer, class to read and write from the dataflash through on of its buffers
 *  - DFlashSegment, class to use a portion of the dataflash (a segment)
 *  - DFlashStream, class to read and write to a Stream (Arduino) into a segment
 *
 * For the actual dataflash chip (8Mb, 16Mb, etc) there is a derived class from DFlash
 *
 * For the two buffers there is a DFlashBuffer1 and a DFlashBuffer2 class, derived from DFlashBuffer.
 *
 * The DFlashStream is a derived class from Arduino Stream, so print, println, etc can be used.
 */

#ifndef SODAQ_SEGMENTED_DF_H_
#define SODAQ_SEGMENTED_DF_H_

#include <Stream.h>
#include <SPI.h>

class DFlashBuffer;
class DFlash
{
    friend class DFlashBuffer;
public:
    // TODO Why is init() virtual?
    virtual void init(uint8_t misoPin, uint8_t mosiPin, uint8_t sckPin, uint8_t csPin) __attribute__((deprecated("Use: void init(uint8_t csPin)")));
    virtual void init(uint8_t csPin);
    virtual ~DFlash() {}

    void setDiagStream(Stream * stream);
    void setDiagStream(Stream & stream);

    virtual uint16_t nrPages() const { return _nrPages; }
    virtual uint16_t pageSize() const { return _pageSize; }
    void readID(uint8_t *data);
    void readSecurityReg(uint8_t *data, size_t size);

    uint8_t readByte(uint8_t cmd, uint16_t addr);
    void readStr(uint8_t cmd, uint16_t addr, uint8_t *data, size_t size);
    void writeByte(uint8_t cmd, uint16_t addr, uint8_t data);
    void writeStr(uint8_t cmd, uint16_t addr, uint8_t *data, size_t size);

    void readPageToBuf(uint8_t cmd, uint16_t pageNr);
    void writeBufToPage(uint8_t cmd, uint16_t pageNr);

    void pageErase(uint16_t pageNr);
    void chipErase();

    void dumpBuffer(uint8_t * buf, size_t size);

private:
  uint8_t readStatus();
  void waitTillReady();
  uint8_t transmit(uint8_t data);
  void activate();
  void deactivate();
  void setPageAddr(unsigned int PageAdr);
  uint8_t getPageAddrByte0(uint16_t pageNr);
  uint8_t getPageAddrByte1(uint16_t pageNr);
  uint8_t getPageAddrByte2(uint16_t pageNr);

protected:
  uint8_t       _csPin;
  size_t        _nrPages;
  size_t        _pageAddrBits;          // number of bits to address a page
  size_t        _pageSize;              // number of bytes in a page
  size_t        _pageBits;              // number of bits to address a byte in a page
  SPISettings   _settings;
};

class DF_AT45DB081D : public DFlash
{
public:
  DF_AT45DB081D();
};
class DF_AT45DB161D : public DFlash
{
public:
  DF_AT45DB161D();
};

class DFlashBuffer
{
public:
  enum DFBufferKind {
    DFBuf1,
    DFBuf2,
  };
  virtual ~DFlashBuffer() {}
  virtual uint16_t nrPages() const { return _df->_nrPages; }
  virtual uint16_t pageSize() const { return _df->_pageSize; }
  virtual uint16_t curPage() const { return _curPage; }

  uint8_t readByte(uint16_t addr);
  void readStr(uint16_t addr, uint8_t *data, size_t size);
  void writeByte(uint16_t addr, uint8_t data);
  void writeStr(uint16_t addr, uint8_t *data, size_t size);

  void readPageToBuf(uint16_t pageNr, bool forced=false);
  void writeBufToPage(uint16_t pageNr);

  void pageErase(uint16_t pageNr) { if (_df) _df->pageErase(pageNr); }

  void dumpBuffer();

  void setDf(DFlash & df) { _df = &df; }

protected:
  DFlash *      _df;
  uint16_t      _curPage;
  uint8_t       _flashToBufCmd;
  uint8_t       _bufToFlashCmd;
  uint8_t       _bufReadCmd;
  uint8_t       _bufWriteCmd;
};
class DFlashBuffer1 : public DFlashBuffer
{
public:
  DFlashBuffer1();
};
class DFlashBuffer2 : public DFlashBuffer
{
public:
  DFlashBuffer2();
};

class DFlashSegment
{
public:
  void init(DFlash & df, enum DFlashBuffer::DFBufferKind kind, uint16_t startPage, uint16_t nrPages);
  uint16_t nrPages() const { return _nrPages; }
  uint16_t pageSize() const { return _dfbuf->pageSize(); }

  uint8_t readByte(uint16_t addr) { return _dfbuf->readByte(addr); }
  void readStr(uint16_t addr, uint8_t *data, size_t size) { _dfbuf->readStr(addr, data, size); }
  void writeByte(uint16_t addr, uint8_t data) { _dfbuf->writeByte(addr, data); }
  void writeStr(uint16_t addr, uint8_t *data, size_t size) { _dfbuf->writeStr(addr, data, size); }

  void readPageToBuf(uint16_t pageNr, bool forced=false);
  void writeBufToPage(uint16_t pageNr);

  void erase();
  void pageErase(uint16_t pageNr);

  // page number is relative to segment start
  uint16_t nextPage(uint16_t page) { ++page; if (page >= _nrPages) page = 0; return page; }
private:
  DFlashBuffer * _dfbuf;
  uint16_t      _startPage;
  uint16_t      _nrPages;
};

class DFlashStream : public Stream
{
public:
  virtual ~DFlashStream() {}
  void init(DFlashSegment & segment);

  void erase();

  uint32_t readPos();
  void setReadPos(uint32_t pos);
  uint32_t writePos();
  void setWritePos(uint32_t pos);
#if 0
  // For testing
  uint16_t getReadByteNr() { return _readByteNr; }
  uint16_t getReadPageNr() { return _readPageNr; }
  uint16_t getWriteByteNr() { return _writeByteNr; }
  uint16_t getWritePageNr() { return _writePageNr; }
#endif

  void writeAvailableToStream(Stream & output);
  void writeAvailableToStream(Stream * output);
  void writeToStream(Stream & output, uint32_t startPos=0);

  // virtual functions in Stream
  int available();
  int read();
  int peek();
  void flush();
  // virtual functions in Print
  size_t write(uint8_t);

private:
  void init();

  uint32_t findEnd(uint16_t & pageNr, uint16_t & byteNr);
  void nextWritePage();
  void pageErase(uint16_t pageNr) { if (_dfs) _dfs->pageErase(pageNr); }

  void readPageToBuf(uint16_t pageNr);
  void writeBufToPage(uint16_t pageNr);

  DFlashSegment *       _dfs;
  uint16_t              _readByteNr;
  uint16_t              _readPageNr;
  uint16_t              _writeByteNr;
  uint16_t              _writePageNr;
  bool                  _writePageDirty;
};


#endif /* SODAQ_SEGMENTED_DF_H_ */
