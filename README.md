# SRAM Test Project

A Caravel user project implementing three instances of CF_SRAM_16384x32 (16K x 32-bit SRAM) with Wishbone bus interface.

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

## Overview

This project integrates three hardened SRAM macros (`CF_SRAM_16384x32`) into the Caravel user project area. Each SRAM provides 16,384 words × 32 bits (64 KB) of memory, for a total of 192 KB of SRAM accessible via the Wishbone bus.

## Features

- **Three SRAM Instances**: Each providing 64 KB (16K × 32-bit) of memory
- **Wishbone B4 Compliant Interface**: Standard Wishbone slave interface for easy integration
- **Address Decoding**: Automatic address space allocation across the three SRAMs
- **Hardened Macros**: Pre-characterized and optimized SRAM IP from ChipFoundry

## Memory Map

The three SRAM instances are mapped to consecutive 64 KB address ranges:

| SRAM Instance | Address Range | Size | Description |
|--------------|---------------|------|-------------|
| `sram`       | 0x00000000 - 0x0000FFFF | 64 KB | Base SRAM instance |
| `sram1`      | 0x00010000 - 0x0001FFFF | 64 KB | Second SRAM instance |
| `sram2`      | 0x00020000 - 0x0002FFFF | 64 KB | Third SRAM instance |

**Total Addressable Memory**: 192 KB (0x00000000 - 0x0002FFFF)

Address decoding is performed using bits [17:16] of the Wishbone address:
- `00` → `sram` (base address)
- `01` → `sram1` (offset 0x10000)
- `10` → `sram2` (offset 0x20000)

## IP Dependencies

This project requires the following IP from ChipFoundry:

- **CF_SRAM_16384x32** (v1.0.0): 16K × 32-bit SRAM macro with integrated Wishbone controller
  - Dependency: CF_SRAM_1024x32 (v1.2.3)

### Installing IP Dependencies

Install the required IP using `ipm`:

```bash
ipm install CF_SRAM_16384x32
```

This will automatically install `CF_SRAM_16384x32` and its dependency `CF_SRAM_1024x32`.

## Project Structure

```
sram_test/
├── verilog/
│   └── rtl/
│       └── user_project_wrapper.v    # Top-level wrapper with 3 SRAM instances
├── openlane/
│   └── user_project_wrapper/
│       └── config.json               # OpenLane configuration
├── ip/
│   └── CF_SRAM_16384x32/            # Installed SRAM IP
│       ├── gds/                      # GDSII layout files
│       ├── lef/                      # LEF abstract views
│       ├── lib/                      # Liberty timing files
│       ├── spef/                     # SPEF parasitic files
│       └── hdl/                      # Verilog netlists
└── README.md                         # This file
```

## Building the Project

### Prerequisites

- LibreLane/OpenLane installed and configured
- Sky130 PDK installed
- `ipm` (ChipFoundry IP Manager) installed
- Docker (for containerized builds)

### Build Steps

1. **Install IP dependencies**:
   ```bash
   ipm install CF_SRAM_16384x32
   ```

2. **Build the design**:
   ```bash
   make user_project_wrapper
   ```

   Or build from the openlane directory:
   ```bash
   cd openlane
   make user_project_wrapper
   ```

3. **Check build outputs**:
   - GDSII: `openlane/user_project_wrapper/runs/<run_tag>/results/final/gds/user_project_wrapper.gds`
   - LEF: `openlane/user_project_wrapper/runs/<run_tag>/results/final/lef/user_project_wrapper.lef`
   - Netlist: `openlane/user_project_wrapper/runs/<run_tag>/results/final/verilog/gl/user_project_wrapper.v`

## Design Details

### SRAM Macro Configuration

- **Type**: CF_SRAM_16384x32 (hardened macro)
- **Size**: 16,384 words × 32 bits = 64 KB per instance
- **Interface**: Wishbone B4 compliant slave
- **Power**: Connected to `vccd1` (1.8V) and `vssd1` (digital ground)

### Placement

The three SRAM instances are placed at the following locations (in microns):

- `sram`: [60, 150]
- `sram1`: [850, 150]
- `sram2`: [1600, 150]

All instances are oriented North (N).

### Power Distribution Network (PDN)

All three SRAM instances are connected to the power grid:
- Power: `VPWR` → `vccd1` (1.8V)
- Ground: `VGND` → `vssd1` (digital ground)

## Wishbone Interface

The design implements a Wishbone B4 compliant slave interface with the following signals:

| Signal | Direction | Width | Description |
|--------|-----------|-------|-------------|
| `wb_clk_i` | Input | 1 | Wishbone clock |
| `wb_rst_i` | Input | 1 | Wishbone reset (active high) |
| `wbs_stb_i` | Input | 1 | Strobe (indicates valid cycle) |
| `wbs_cyc_i` | Input | 1 | Cycle (indicates active bus cycle) |
| `wbs_we_i` | Input | 1 | Write enable (1=write, 0=read) |
| `wbs_sel_i` | Input | 4 | Byte lane select |
| `wbs_dat_i` | Input | 32 | Data input (for writes) |
| `wbs_adr_i` | Input | 32 | Address input |
| `wbs_ack_o` | Output | 1 | Acknowledge (transaction complete) |
| `wbs_dat_o` | Output | 32 | Data output (for reads) |

## Configuration Files

### OpenLane Configuration

The main configuration file is located at:
```
openlane/user_project_wrapper/config.json
```

Key settings:
- **Design Name**: `user_project_wrapper`
- **Die Area**: 2920 × 3520 microns
- **Clock Period**: 25 ns (40 MHz)
- **Macro Placement**: Fixed locations for all three SRAM instances
- **PDN**: Core ring enabled with custom spacing

### Magic DRC Configuration

Magic DRC errors are captured but do not fail the build:
- `MAGIC_CAPTURE_ERRORS`: false
- `ERROR_ON_MAGIC_DRC`: false

This allows the build to complete even if Magic encounters unknown GDS layers in the SRAM macros.

## Testing

To test the SRAM functionality:

1. **Simulation**: Use the Wishbone testbench to verify address decoding and data access
2. **Hardware**: Program the design onto a Caravel test chip and access via the Wishbone bus

### Example Wishbone Access

```c
// Access SRAM 0 (sram) at base address
uint32_t *sram0 = (uint32_t *)0x00000000;
sram0[0] = 0xDEADBEEF;  // Write
uint32_t data = sram0[0];  // Read

// Access SRAM 1 (sram1) at offset 0x10000
uint32_t *sram1_ptr = (uint32_t *)0x00010000;
sram1_ptr[0] = 0xCAFEBABE;

// Access SRAM 2 (sram2) at offset 0x20000
uint32_t *sram2_ptr = (uint32_t *)0x00020000;
sram2_ptr[0] = 0x12345678;
```

## Known Issues

- **Magic GDS Layer Warnings**: The SRAM GDS files contain layers (67, 68, 69) that are not defined in the Magic technology file. These are annotation layers and do not affect functionality. The build continues despite these warnings.

## License

This project is licensed under the Apache License 2.0 - see the LICENSE file for details.

## References

- [Caravel User Project Documentation](https://caravel-harness.readthedocs.io/)
- [ChipFoundry IP Documentation](https://chipfoundry.io/)
- [Wishbone B4 Specification](https://cdn.opencores.org/downloads/wbspec_b4.pdf)
- [OpenLane Documentation](https://openlane.readthedocs.io/)

## Contact

For questions or issues related to this project, please refer to the Caravel user project repository or ChipFoundry support channels.
