/*
 * Based on UploadPages.h by Gabriel Notman
 */

#ifndef UPLOADPAGES_H_
#define UPLOADPAGES_H_

#include <Arduino.h>
#include <stdint.h>
#include "SumDataRecord.h"

int16_t uploadPages(DataflashUtils& dflashUtils, DataRecord& record);
int16_t uploadSumRecords(DataflashUtils& dflashUtils, SumDataRecord& flashRecord);

#endif /* UPLOADPAGES_H_ */

