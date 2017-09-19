/*
 * Dataflash_Utils.cpp
 *
 * based on: SQ_DataflashUtils.cpp
 *  Created on: Mar 29, 2014
 *      Author: Kees Bakker
 */

#include <Arduino.h>

#include "DiagPrint.h"
#include "Sodaq_segmented_df.h"
#include "Sodaq_wdt.h"

#include "DataflashUtils.h"

#define ENABLE_DFUTILS_DIAG            1

#define DEBUG_PREFIX String("[DFU]")

#if ENABLE_DFUTILS_DIAG
  #define myDiagPrint(...) diagPrint(__VA_ARGS__)
  #define myDiagPrintLn(...) diagPrintLn(__VA_ARGS__)
#else
  #define myDiagPrint(...)
  #define myDiagPrintLn(...)
#endif

/* 
* \brief Sets member variables to default or NULL values
*/
DataflashUtils::DataflashUtils()
{
  _dataSeg = 0;
  _headerMagic[0] = 0;
  _dataVersion = 0;
  _curPage = -1;
  _curByte = 0;
  _uploadPage = -1;
}

/*
* \brief Initialize the dataflash, find curPage and uploadPage
*/
void DataflashUtils::init(uint32_t ts, DFlashSegment & dataSeg,
    const char* headerMagic, uint32_t dataVersion)
{
  _dataSeg = &dataSeg;

  memset(_headerMagic, 0, sizeof(_headerMagic));
  strncpy(_headerMagic, headerMagic, sizeof(_headerMagic));
  _dataVersion = dataVersion;

  // We need the current timestamp as a pseudo random value
  findCurAndUploadPage(ts);
  
  myDiagPrint(DEBUG_PREFIX + "UploadPage:"); myDiagPrintLn(_uploadPage);
  myDiagPrint(DEBUG_PREFIX + "CurPage:"); myDiagPrintLn(_curPage);

  if (_uploadPage >= 0 && _uploadPage == _curPage) {
    // Data flash is totally filled up. Forget about oldest upload page
    _uploadPage = getNextPage(_uploadPage);
    myDiagPrint(DEBUG_PREFIX + " > UploadPage:"); myDiagPrintLn(_uploadPage);
    // No need to verify validity
  }
  
  initNewPage(_curPage, ts);
}

/*
* \brief Returns true if one or more records are stored
*/
bool DataflashUtils::isEmpty(uint16_t recSize)
{
  bool empty = true;

  uint8_t buff[recSize];

  int page = getUploadPage();

  while ((isValidUploadPage(page)) && (empty)){
    sodaq_wdt_reset();

    if (readPageNthRecord(page, 0, buff, sizeof(buff))) {
      myDiagPrintLn(DEBUG_PREFIX + String("Found at least one record on page: ") + page);
      empty = false;
    }
    page = getNextPage(page);
  }

  return empty;
}

/*
* \brief Returns the number of records stored
*/

size_t DataflashUtils::numOfRecords(uint16_t recSize)
{
  size_t count = 0;

  uint8_t buff[recSize];

  int page = getUploadPage();

  while (isValidUploadPage(page)) {
    sodaq_wdt_reset();

    uint16_t recNum = 0;
    while (readPageNthRecord(page, recNum, buff, sizeof(buff))) {
      recNum++;
      count++;
    }
    myDiagPrintLn(DEBUG_PREFIX + String("Page: ") + page + " has: " + recNum + " records");
    page = getNextPage(page);
  }

  return count;
}

/*
* \brief Add one record to the current page
*
* First it checks if there is enough space in the current
* flash page. If not the page is "closed" and a new page
* is prepared for the addition.
*/
void DataflashUtils::addCurPageRecord(uint8_t *rec, uint16_t recSize, uint32_t ts)
{
  myDiagPrint(DEBUG_PREFIX + "AddCurPageRecord:"); myDiagPrintLn(_curPage);
  myDiagPrint(DEBUG_PREFIX + "CurByte:"); myDiagPrintLn(_curByte);

  // Is there enough room for a new record in the current page?
  if (_curPage < 0 || _curByte > (int)(_dataSeg->pageSize() - recSize)) {
    // No, so start using the next page
    myDiagPrint(DEBUG_PREFIX + "AddCurPageRecord:"); myDiagPrintLn(_curPage);
#if ENABLE_DFUTILS_DIAG
    dumpPage(_curPage);
#endif
    newCurPage(ts);
  }
    
  // In case another instance is using the buffer
  restoreCurPage();

  // Write the record to the page
  _dataSeg->writeStr(_curByte, rec, recSize);
  _curByte += recSize;

  // Write flash internal buffer to the actual flash memory
  // The reason is that we don't want to loose the info if the software crashes
  _dataSeg->writeBufToPage(_curPage);

  if (_uploadPage < 0) {
    // Remember this as the first page to upload
    _uploadPage = _curPage;
  }
}

/*
* \brief Erases the current upload page
*/
void DataflashUtils::eraseUploadPage(uint32_t ts)
{
  if (isValidUploadPage(_uploadPage)) {
    erasePage(_uploadPage);
    int nextPage = getNextPage(_uploadPage);

    if (isValidUploadPage(nextPage)) {
      _uploadPage = nextPage;
    }
    else {
      _curPage = nextPage;
      initNewPage(_curPage, ts);
      _uploadPage = -1;
    }      
  }        
}

/*
* \brief Returns the current page index
*/
int DataflashUtils::getCurPage()
{
  return _curPage;
}

/*
* \brief Returns the next valid index to 'page'
*/
int DataflashUtils::getNextPage(int page)
{
  return _dataSeg->nextPage(page);
}

/*
* \brief Returns the upload page index
*/
int DataflashUtils::getUploadPage()
{
  return _uploadPage;
}

/*
* \brief Is this a valid page to upload
*
* Note. This invalidates the "internal buffer" of the Data Flash
*/
bool DataflashUtils::isValidUploadPage(int page)
{
  PageHeader_t hdr;
  readPageHeader(page, &hdr);

  return isValidHeader(&hdr);
}

/*
* \brief Read one record from the page
*/
bool DataflashUtils::readPageNthRecord(int page, uint8_t nth, uint8_t *rec, uint16_t recSize)
{
  size_t byte_offset = sizeof(PageHeader_t) + nth * recSize;
  if ((byte_offset + recSize) > _dataSeg->pageSize()) {
    // The record is crossing page boundary
    memset(rec, 0xFF, recSize);
    return false;
  }

  _dataSeg->readPageToBuf(page);
  uint8_t *buffer = (uint8_t *)rec;
  uint16_t remSize = recSize;
  while (remSize > 0) {
    int size1 = remSize >= 16 ? 16 : remSize;
    _dataSeg->readStr(byte_offset, buffer, size1);
    byte_offset += size1;
    buffer += size1;
    remSize -= size1;
  }

#if ENABLE_DFUTILS_DIAG
  myDiagPrintLn(DEBUG_PREFIX + "[" + page + "," + nth + "]");
  print02X(DEBUG_PREFIX, rec, recSize);
  myDiagPrintLn();
#endif

  // Record is valid if any of the bytes != 0xFF
  for (int i = 0; i < recSize; ++i) {
    if (rec[i] != 0xFF) {
      return true;
    }
  }

  return false;
}

/*
* \brief Erases this partition's pages and resets
* the current and upload page references.
*/
void DataflashUtils::reset(uint32_t ts)
{
  for (int i = 0; i < _dataSeg->nrPages(); ++i) {
    _dataSeg->pageErase(i);
  }
  
  // There is no upload page
  _uploadPage = -1;

  // Pick a "random" page to start
  _curPage = (ts % _dataSeg->nrPages());
  initNewPage(_curPage, ts);
}

/*
* \brief Dump the contents of a data flash page
*
* This function is just meant for diagnostics.
*/
void DataflashUtils::dumpPage(int page)
{
  if (page < 0) {
    return;
  }        

  myDiagPrint(DEBUG_PREFIX + "Page "); myDiagPrintLn(page);
  _dataSeg->readPageToBuf(page);
  uint8_t buffer[16];
  for (uint16_t i = 0; i < _dataSeg->pageSize(); i += sizeof(buffer)) {
    size_t nr = sizeof(buffer);
    if ((i + nr) > _dataSeg->pageSize()) {
      nr = _dataSeg->pageSize() - i;
    }
    _dataSeg->readStr(i, buffer, nr);

    print02X(DEBUG_PREFIX, buffer, nr);
    myDiagPrintLn();
  }
}

/*
* \brief This function reads all pages in the data flash and measures how long
* it takes.
*
* This function is just meant for diagnostics.
*/
void DataflashUtils::readAllPages()
{
#if ENABLE_DFUTILS_DIAG
  uint32_t start = millis();
#endif

  for (int page = 0; page < _dataSeg->nrPages(); ++page) {
    PageHeader_t hdr;
    readPage(page, (uint8_t*)&hdr, sizeof(hdr));
  }

#if ENABLE_DFUTILS_DIAG
  uint32_t elapse = millis() - start;
  myDiagPrint(DEBUG_PREFIX + "Nr secs to read all pages: "); myDiagPrintLn(elapse / 1000);
#endif
}

/*
* \brief Erase and Reset the current page
*/
void DataflashUtils::erasePage(int page)
{
  myDiagPrint(DEBUG_PREFIX + "ErasePage:"); myDiagPrintLn(page);
  
  if (page < 0) {
    return;
  }
  _dataSeg->pageErase(page);
}

/*
* \brief Search for curPage and uploadPage in the data flash
*
* Search through the whole date flash and try to find the best page for
* curPage and for the uploadPage.
*/
void DataflashUtils::findCurAndUploadPage(uint32_t randomNum)
{
#if ENABLE_DFUTILS_DIAG
  uint32_t start = millis();
#endif

  _curPage = -1;
  _uploadPage = -1;
  uint32_t uploadTs = 0;
  PageHeader_t hdr;

  // First round, search for upload page
  for (int page = 0; page < _dataSeg->nrPages(); ++page) {
    readPage(page, (uint8_t*)&hdr, sizeof(hdr));

    if (isValidHeader(&hdr)) {
      myDiagPrint(DEBUG_PREFIX + "Valid page: "); myDiagPrintLn(page);
      if (_uploadPage < 0) {
        _uploadPage = page;
        uploadTs = hdr.ts;
        } else {
        // Make sure we remember the oldest upload record
        if (hdr.ts < uploadTs) {
          _uploadPage = page;
          uploadTs = hdr.ts;
        }
      }
    }
  }
  
  if (_uploadPage >= 0) {
    // Starting from upload page, look for the next free spot.
    // TODO Verify this logic
    int page = _uploadPage;
    for (int nr = 0; nr < _dataSeg->nrPages(); ++nr, page = getNextPage(page)) {
      readPage(page, (uint8_t*)&hdr, sizeof(hdr));
      if (!isValidHeader(&hdr)) {
        _curPage = page;
        break;
      }
    }
    if (_curPage < 0) {
      // None of the pages is empty.
      // Use the page of the oldest upload, the caller will take care of this
      _curPage = _uploadPage;
    }
  } else {
    // No upload page found.
    _uploadPage = -1;

    // Start at a random place
    _curPage = (randomNum % _dataSeg->nrPages());
  }

#if ENABLE_DFUTILS_DIAG
  uint32_t elapse = millis() - start;
  myDiagPrint(DEBUG_PREFIX + "Found uploadPage in (ms) "); myDiagPrintLn(elapse);
#endif
}

/*
* \brief Returns the page's timestamp
*/
uint32_t DataflashUtils::getPageTS(int page)
{
  if (page < 0) {
    return -1;
  }
  PageHeader_t hdr;
  readPage(page, (uint8_t*)&hdr, sizeof(hdr));
  return hdr.ts;
}

/*
* \brief Initialises a new page
*/
void DataflashUtils::initNewPage(int page, uint32_t ts)
{
  myDiagPrint(DEBUG_PREFIX + "InitNewPage:"); myDiagPrintLn(page);

  _curByte = 0;
  // Write new header
  PageHeader_t hdr;
  hdr.ts = ts;
  strncpy(hdr.magic, _headerMagic, sizeof(hdr.magic));
  hdr.version = _dataVersion;

  _dataSeg->writeStr(_curByte, (uint8_t *)&hdr, sizeof(hdr));
  _curByte += sizeof(hdr);

  for (int b = _curByte; b < _dataSeg->pageSize(); ++b) {
    _dataSeg->writeByte(b, 0xff);
  }

  // Write flash internal buffer to the actual flash memory
  // The reason is that we don't want to loose the info if the software crashes
  _dataSeg->writeBufToPage(_curPage);
}

/*
* \brief Is this a valid page header
*/
bool DataflashUtils::isValidHeader(PageHeader_t *hdr)
{
  if (strncmp(hdr->magic, _headerMagic, sizeof(_headerMagic)) != 0) {
    return false;
  }
  if (hdr->version != _dataVersion) {
    return false;
  }
  // The timestamp should be OK too. How can it be bad?
  return true;
}

/*
* \brief Set curPage to the next page and go to the next
*
* This function also takes care of making sure we always have
* an empty page between curPage and uploadPage.
* If fact uploadPage must be at least 2 ahead because the
* next page for curPage is going to be filled in shortly after
* this.
*/
void DataflashUtils::newCurPage(uint32_t ts)
{
  _curPage = getNextPage(_curPage);
  myDiagPrint(DEBUG_PREFIX + "> New curPage:"); myDiagPrintLn(_curPage);
  int pageAfterCurPage = getNextPage(_curPage);
  
  if (isValidUploadPage(pageAfterCurPage)) {
    // This triggers when the flash is completely full. It will
    // be very rare, but still...

    if (_uploadPage == pageAfterCurPage) {
      // Shift uploadPage
      _uploadPage = getNextPage(_uploadPage);
      if (!isValidUploadPage(_uploadPage)) {
        // Not likely, but better be safe.
        _uploadPage = -1;
      }
      myDiagPrint(DEBUG_PREFIX + " >> New uploadPage:"); myDiagPrintLn(_uploadPage);
    }

    myDiagPrint(DEBUG_PREFIX + " >> Erase page after:"); myDiagPrintLn(pageAfterCurPage);
    erasePage(pageAfterCurPage);
  }

  initNewPage(_curPage, ts);
}

/*
* \brief Read a whole page into a buffer
*
* Buffer_Read_Str returns corrupted data if we're trying
* to read the whole page at once. Here we do it in chunks
* of 16 bytes.
*/
void DataflashUtils::readPage(int page, uint8_t *buffer, size_t size)
{
  memset(buffer, 0, size);
  if (page < 0) {
    return;
  }

  //myDiagPrint(DEBUG_PREFIX + "readPage - page: "); myDiagPrintLn(page);
  _dataSeg->readPageToBuf(page);
  // Read in chunks of max 16 bytes
  int byte_offset = 0;
  while (size > 0) {
    int size1 = size >= 16 ? 16 : size;
    _dataSeg->readStr(byte_offset, buffer, size1);
    byte_offset += size1;
    buffer += size1;
    size -= size1;
  }
}

/*
* \brief Read the page header
*/
bool DataflashUtils::readPageHeader(int page, PageHeader_t *hdr)
{
  size_t byte_offset = 0;
  size_t size = sizeof(PageHeader_t);

  _dataSeg->readPageToBuf(page);
  uint8_t *buffer = (uint8_t *)hdr;
  while (size > 0) {
    int size1 = size >= 16 ? 16 : size;
    _dataSeg->readStr(byte_offset, buffer, size1);
    byte_offset += size1;
    buffer += size1;
    size -= size1;
  }
  return isValidHeader(hdr);
}

/*
* \brief Reads the current page back into the SRAM buffer
*/
void DataflashUtils::restoreCurPage()
{
  _dataSeg->readPageToBuf(_curPage);
}

//DataflashUtils dflashUtils;
