// SPDX-FileCopyrightText: 2025 ChipFoundry
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// SPDX-License-Identifier: Apache-2.0

#include <firmware_apis.h>

// SRAM address ranges (64KB each = 16384 words * 4 bytes)
#define SRAM0_BASE 0x00000000  // sram instance
#define SRAM1_BASE 0x00010000  // sram1 instance  
#define SRAM2_BASE 0x00020000  // sram2 instance
#define SRAM_SIZE  0x10000     // 64KB per SRAM

// Test patterns
#define TEST_PATTERN_1 0xDEADBEEF
#define TEST_PATTERN_2 0xCAFEBABE
#define TEST_PATTERN_3 0x12345678
#define TEST_PATTERN_4 0xABCDEF00

void main(){
    // Enable management gpio as output to use as indicator for finishing configuration  
    ManagmentGpio_outputEnable();
    ManagmentGpio_write(0);
    enableHkSpi(0); // disable housekeeping spi
    
    // Configure all gpios as user out for monitoring
    GPIOs_configureAll(GPIO_MODE_USER_STD_OUT_MONITORED);
    GPIOs_loadConfigs(); // load the configuration 
    
    // Enable Wishbone interface to user project
    User_enableIF(); // necessary when reading or writing between wishbone and user project
    
    ManagmentGpio_write(1); // configuration finished 
    
    // Test SRAM 0 (sram) - Base address 0x00000000
    // Write to beginning of SRAM 0
    USER_writeWord(SRAM0_BASE, TEST_PATTERN_1);
    
    // Write to end of SRAM 0 (last word: 0x0000FFFC)
    USER_writeWord(SRAM0_BASE + SRAM_SIZE - 4, TEST_PATTERN_2);
    
    // Test SRAM 1 (sram1) - Base address 0x00010000
    // Write to beginning of SRAM 1
    USER_writeWord(SRAM1_BASE, TEST_PATTERN_3);
    
    // Write to end of SRAM 1 (last word: 0x0001FFFC)
    USER_writeWord(SRAM1_BASE + SRAM_SIZE - 4, TEST_PATTERN_4);
    
    // Test SRAM 2 (sram2) - Base address 0x00020000
    // Write to beginning of SRAM 2
    USER_writeWord(SRAM2_BASE, TEST_PATTERN_4);
    
    // Write to end of SRAM 2 (last word: 0x0002FFFC)
    USER_writeWord(SRAM2_BASE + SRAM_SIZE - 4, TEST_PATTERN_1);
    
    // Read back and verify all locations
    unsigned int read_data;
    
    // Read SRAM 0 beginning
    read_data = USER_readWord(SRAM0_BASE);
    if (read_data != TEST_PATTERN_1) {
        ManagmentGpio_write(0xAA); // Error indicator
        return;
    }
    
    // Read SRAM 0 end
    read_data = USER_readWord(SRAM0_BASE + SRAM_SIZE - 4);
    if (read_data != TEST_PATTERN_2) {
        ManagmentGpio_write(0xBB); // Error indicator
        return;
    }
    
    // Read SRAM 1 beginning
    read_data = USER_readWord(SRAM1_BASE);
    if (read_data != TEST_PATTERN_3) {
        ManagmentGpio_write(0xCC); // Error indicator
        return;
    }
    
    // Read SRAM 1 end
    read_data = USER_readWord(SRAM1_BASE + SRAM_SIZE - 4);
    if (read_data != TEST_PATTERN_4) {
        ManagmentGpio_write(0xDD); // Error indicator
        return;
    }
    
    // Read SRAM 2 beginning
    read_data = USER_readWord(SRAM2_BASE);
    if (read_data != TEST_PATTERN_4) {
        ManagmentGpio_write(0xEE); // Error indicator
        return;
    }
    
    // Read SRAM 2 end
    read_data = USER_readWord(SRAM2_BASE + SRAM_SIZE - 4);
    if (read_data != TEST_PATTERN_1) {
        ManagmentGpio_write(0xFF); // Error indicator
        return;
    }
    
    // All tests passed - set success indicator
    ManagmentGpio_write(0xAB); // Success indicator (matches pattern from other tests)
    
    return;
}
