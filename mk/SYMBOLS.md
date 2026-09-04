# Clean symbols — `mk`

DIRECTORY=mk
GENERATION=CLEAN_RECONSTRUCTION
COVERAGE=COMPLETE

This directory owns the Make-level build graph for the clean Issue #7 linked
executable. PS2SDK-provided Make variables and conventional GNU Make interfaces
are external build vocabulary and are not duplicated here as project symbols.

| Name | Kind | File | Owner | Scope | Description | Context |
|---|---|---|---|---|---|---|
| BUILD_DIR | variable | mk/issue7-clean.mk | Issue #7 clean build | build | Names the disposable root directory containing every clean Issue #7 linked-build artifact. | Issue #7 linked build |
| GEN_DIR | variable | mk/issue7-clean.mk | Issue #7 clean build | build | Names the build-local directory containing C sources generated from required PS2SDK IRX binaries. | embedded IRX generation |
| DEP_DIR | variable | mk/issue7-clean.mk | Issue #7 clean build | build | Names the build-local directory containing the verified copied qualified PS2IP archive. | dependency authority |
| PS2IP_LIB | variable | mk/issue7-clean.mk | Issue #7 clean build | build | Names the exact qualified PS2IP archive that must be linked into the clean Issue #7 executable. | dependency authority |
| all | target | mk/issue7-clean.mk | Issue #7 clean build | build | Defines the default clean-build goal by requiring the PS2SDK-linked Issue #7 executable. | Issue #7 linked build |
| $(BUILD_DIR) | target | mk/issue7-clean.mk | Issue #7 clean build | build | Creates the root output directory required by clean compilation and linking. | build directory preparation |
| $(GEN_DIR) | target | mk/issue7-clean.mk | Issue #7 clean build | build | Creates the directory that receives C sources generated from PS2SDK IRX binaries. | embedded IRX generation |
| $(DEP_DIR) | target | mk/issue7-clean.mk | Issue #7 clean build | build | Creates the directory in which the verified qualified PS2IP archive is supplied to Make. | dependency authority |
| $(PS2IP_LIB) | target | mk/issue7-clean.mk | Issue #7 clean build | build | Fails closed when the canonical wrapper has not supplied the verified qualified PS2IP archive. | dependency authority |
| $(BUILD_DIR)/main.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the clean executable entry translation unit into its linked-build object. | clean source compilation |
| $(BUILD_DIR)/app.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the clean application-orchestration translation unit and its declared subsystem interfaces. | clean source compilation |
| $(BUILD_DIR)/diagnostics.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the clean runtime diagnostics implementation into its linked-build object. | clean source compilation |
| $(BUILD_DIR)/diagnostics_identity.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the runtime-identity implementation containing the pristine stampable DUT identity blob. | deterministic DUT identity |
| $(BUILD_DIR)/rfb.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the clean RFB protocol implementation into its linked-build object. | clean RFB core |
| $(BUILD_DIR)/framebuffer.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the clean framebuffer implementation used by the RFB and display paths. | clean framebuffer |
| $(BUILD_DIR)/rfb_session.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the clean RFB-session state and framing implementation with its transport and framebuffer interfaces. | clean RFB session |
| $(BUILD_DIR)/display.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the clean display-domain implementation and framebuffer interface. | clean display |
| $(BUILD_DIR)/pad.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the direct-libpad controller boundary used by the clean executable. | controller input |
| $(BUILD_DIR)/ps2_system.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the PS2 system platform boundary that owns system and IOP initialization responsibilities. | PS2 platform |
| $(BUILD_DIR)/ps2_network.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the PS2 network platform boundary used by the clean RFB transport path. | PS2 networking |
| $(BUILD_DIR)/ps2_graphics.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the PS2 graphics platform boundary used by the clean display path. | PS2 graphics |
| $(GEN_DIR)/SIO2MAN_irx.c | target | mk/issue7-clean.mk | Issue #7 clean build | build | Converts the PS2SDK FreeSIO2 IRX binary into C data so SIO2MAN can be embedded in the ELF. | embedded IRX generation |
| $(GEN_DIR)/PADMAN_irx.c | target | mk/issue7-clean.mk | Issue #7 clean build | build | Converts the PS2SDK FreePad IRX binary into C data so PADMAN can be embedded in the ELF. | embedded IRX generation |
| $(GEN_DIR)/DEV9_irx.c | target | mk/issue7-clean.mk | Issue #7 clean build | build | Converts the PS2SDK DEV9 IRX binary into C data for inclusion in the clean network-enabled ELF. | embedded IRX generation |
| $(GEN_DIR)/NETMAN_irx.c | target | mk/issue7-clean.mk | Issue #7 clean build | build | Converts the PS2SDK NETMAN IRX binary into C data for inclusion in the clean network-enabled ELF. | embedded IRX generation |
| $(GEN_DIR)/SMAP_irx.c | target | mk/issue7-clean.mk | Issue #7 clean build | build | Converts the PS2SDK SMAP IRX binary into C data for inclusion in the clean Ethernet-enabled ELF. | embedded IRX generation |
| $(BUILD_DIR)/SIO2MAN_irx.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the generated SIO2MAN IRX data into an object linked into the clean executable. | embedded IRX linkage |
| $(BUILD_DIR)/PADMAN_irx.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the generated PADMAN IRX data into an object linked into the clean executable. | embedded IRX linkage |
| $(BUILD_DIR)/DEV9_irx.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the generated DEV9 IRX data into an object linked into the clean executable. | embedded IRX linkage |
| $(BUILD_DIR)/NETMAN_irx.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the generated NETMAN IRX data into an object linked into the clean executable. | embedded IRX linkage |
| $(BUILD_DIR)/SMAP_irx.o | target | mk/issue7-clean.mk | Issue #7 clean build | build | Compiles the generated SMAP IRX data into an object linked into the clean executable. | embedded IRX linkage |
| clean | target | mk/issue7-clean.mk | Issue #7 clean build | build | Removes the complete disposable Issue #7 linked-build directory and all generated build artifacts beneath it. | build hygiene |
