# SPDX-FileCopyrightText: 2025 ChipFoundry
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# SPDX-License-Identifier: Apache-2.0

"""
Cocotb testbench for verifying three CF_SRAM_16384x32 instances.

This test verifies:
- Address decoding across three SRAM blocks
- Write and read operations at beginning and end of each block
- Data integrity across all three SRAM instances
"""

from caravel_cocotb.caravel_interfaces import test_configure
from caravel_cocotb.caravel_interfaces import report_test
from caravel_cocotb.caravel_interfaces import Caravel_env
import cocotb
from cocotb.triggers import ClockCycles, RisingEdge, FallingEdge
from cocotb.binary import BinaryValue

# SRAM address ranges (64KB each = 16384 words * 4 bytes)
SRAM0_BASE = 0x00000000  # sram instance
SRAM1_BASE = 0x00010000  # sram1 instance  
SRAM2_BASE = 0x00020000  # sram2 instance
SRAM_SIZE = 0x10000      # 64KB per SRAM

# Test patterns
TEST_PATTERN_1 = 0xDEADBEEF
TEST_PATTERN_2 = 0xCAFEBABE
TEST_PATTERN_3 = 0x12345678
TEST_PATTERN_4 = 0xABCDEF00

@cocotb.test()
@report_test
async def sram_test(dut):
    """
    Test three SRAM instances by writing and reading at beginning and end of each block.
    
    The firmware performs all Wishbone operations and sets management GPIO to indicate:
    - 0xAB: All tests passed
    - 0xAA-0xFF: Error codes for different failure points
    """
    caravelEnv = await test_configure(dut, timeout_cycles=50000)
    
    cocotb.log.info("[TEST] Starting SRAM integration test")
    
    # Release CSB to enable Wishbone interface
    await caravelEnv.release_csb()
    
    # Wait for configuration to finish (GPIO = 1)
    await caravelEnv.wait_mgmt_gpio(1)
    cocotb.log.info("[TEST] Configuration finished, firmware starting SRAM tests")
    
    # Wait for firmware to complete tests and set result indicator
    # Success indicator is 0xAB (matches pattern from other tests)
    await caravelEnv.wait_mgmt_gpio(0xAB, timeout_cycles=40000)
    
    # Check the final GPIO value
    mgmt_gpio_val = caravelEnv.monitor_mgmt_gpio()
    cocotb.log.info(f"[TEST] Management GPIO value: 0x{mgmt_gpio_val:02X}")
    
    if mgmt_gpio_val == 0xAB:
        cocotb.log.info("[TEST] All SRAM tests passed successfully!")
        cocotb.log.info("[TEST] Verified:")
        cocotb.log.info("  - SRAM 0 (sram): Write/read at 0x00000000 and 0x0000FFFC")
        cocotb.log.info("  - SRAM 1 (sram1): Write/read at 0x00010000 and 0x0001FFFC")
        cocotb.log.info("  - SRAM 2 (sram2): Write/read at 0x00020000 and 0x0002FFFC")
    else:
        error_codes = {
            0xAA: "SRAM 0 begin read mismatch",
            0xBB: "SRAM 0 end read mismatch",
            0xCC: "SRAM 1 begin read mismatch",
            0xDD: "SRAM 1 end read mismatch",
            0xEE: "SRAM 2 begin read mismatch",
            0xFF: "SRAM 2 end read mismatch"
        }
        error_msg = error_codes.get(mgmt_gpio_val, f"Unknown error code: 0x{mgmt_gpio_val:02X}")
        cocotb.log.error(f"[TEST] SRAM test failed: {error_msg}")
        raise cocotb.result.TestFailure(f"SRAM test failed with error code 0x{mgmt_gpio_val:02X}: {error_msg}")
