/*
 * Dataflash_Utils.h
 *
 * based on: SQ_DataflashUtils.h
 *  Created on: Mar 29, 2014
 *      Author: Kees Bakker
 */

#ifndef DATAFLASH_UTILS_H_
#define DATAFLASH_UTILS_H_

#include <SPI.h>
#include "Sodaq_segmented_df.h"

struct PageHeader_t
{
    uint32_t      ts;
    uint32_t      version;
    char          magic[20];
};
typedef struct PageHeader_t PageHeader_t;

typedef bool(*boolFuncPtr)(uint8_t* rec);

#define NR_RECORDS_PER_PAGE(RECORD_SIZE)  ((DF_PAGE_SIZE - sizeof(PageHeader_t)) / RECORD_SIZE))

class DataflashUtils
{
public:
  DataflashUtils();
  void init(uint32_t ts, DFlashSegment & dataSeg, const char* headerMagic,
      uint32_t dataVersion);

  bool isEmpty(uint16_t recSize);
  size_t numOfRecords(uint16_t recSize);

  void addCurPageRecord(uint8_t *rec, uint16_t recSize, uint32_t ts);
  void eraseUploadPage(uint32_t ts);
  int getCurPage();
  int getNextPage(int page);
  int getUploadPage();
  void newCurPage(uint32_t ts);
  bool isValidUploadPage(int page);
  bool readPageNthRecord(int page, uint8_t nth, uint8_t *rec, uint16_t recSize);
  void reset(uint32_t ts);

  void dumpPage(int page);
  void readAllPages();

private:
  void erasePage(int page);
  void findCurAndUploadPage(uint32_t randomNum);
  uint32_t getPageTS(int page);
  void initNewPage(int page, uint32_t ts);
  bool isValidHeader(PageHeader_t *hdr);
  void readPage(int page, uint8_t *buffer, size_t size);
  bool readPageHeader(int page, PageHeader_t *hdr);
  void restoreCurPage();

  DFlashSegment * _dataSeg;
    
  char _headerMagic[20];
  uint32_t _dataVersion;

  int _curPage;
  uint16_t _curByte;            // Byte number in the current page (_curPage)
  int _uploadPage;
};

//extern DataflashUtils dflashUtils;

#endif /* DATAFLASH_UTILS_H_ */
