    .cpu    cortex-m0
    .thumb

// Default Clock State is HIGH.
// CS is controlled by caller.

// Single Bit Cycle is:

// Set Write Bit
// Clock Low
// Get Read Bit
// Clock High

.func   TX_SPI_WRITE_BYTE
.global TC_SPI_WRITE_BYTE
TC_SPI_WRITE_BYTE:
        // Transfer a single byte over SPI 
        // to the Thermocouple.
        // Byte to send is in r0
  bx  lr
.endfunc

.func   TX_SPI_READ_BYTE
.global TC_SPI_READ_BYTE
TC_SPI_READ_BYTE:
        // Read a single byte over SPI 
        // from the Thermocouple.
  bx  lr
.endfunc

.func   TX_SPI_READ_WORD
.global TC_SPI_READ_WORD
TC_SPI_READ_WORD:
        // Read 32 bits over SPI from the Thermocouple.
  bx  lr
.endfunc