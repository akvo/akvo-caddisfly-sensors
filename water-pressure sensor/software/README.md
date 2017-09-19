EXTENDED FUSE: 0xED 
    (1110 1101): 21kHz ULP, Brown-out detection sampled when sleeping and enabled when idle or active

HIGH FUSE: 0xD5
    (1101 0101): Brown-out detection at 2.7v Preserve EEPROM enable, SPI enabled, WDTON disabled, no debugwire, no reset disable

LOW FUSE: 0xC2 
	(1110 0010): Internal 8MHz, no division. no clock out, full SUT

Or depending on bit 5 (Reserved)

LOW FUSE: 0xE2
	(1110 0010): Internal 8MHz, no division. no clock out, full SUT

LOCK BITS: 0xFF (no locking)
