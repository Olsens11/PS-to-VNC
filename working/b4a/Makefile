GSKIT ?= $(PS2DEV)/gsKit

EE_BIN = PS2VNC.ELF
EE_OBJS = ps2ip.o ps2vnc_config_text.o ps2vnc_identity.o ps2vnc_gsHires.o DEV9_irx.o NETMAN_irx.o SMAP_irx.o

EE_INCS = -I$(GSKIT)/include

PS2IP_LIB = /work/build/deps/libps2ip_mtu1458_wscale128.a
EE_LIBS = -L$(GSKIT)/lib -lgskit -ldmakit -lpad -lnetman $(PS2IP_LIB) -ldebug -lpatches -Wl,--wrap=sendto

all: $(EE_BIN)

$(EE_BIN): $(PS2IP_LIB)

$(PS2IP_LIB):
	@echo "Missing generated PS2IP dependency: $(PS2IP_LIB)" >&2
	@echo "Run ./scripts/build.sh from the repository root." >&2
	@false

clean:
	rm -f $(EE_BIN) $(EE_OBJS) DEV9_irx.c NETMAN_irx.c SMAP_irx.c

DEV9_irx.c: $(PS2SDK)/iop/irx/ps2dev9.irx
	bin2c $< DEV9_irx.c DEV9_irx

NETMAN_irx.c: $(PS2SDK)/iop/irx/netman.irx
	bin2c $< NETMAN_irx.c NETMAN_irx

SMAP_irx.c: $(PS2SDK)/iop/irx/smap.irx
	bin2c $< SMAP_irx.c SMAP_irx

run: $(EE_BIN)
	ps2client execee host:$(EE_BIN)

reset:
	ps2client reset

include $(PS2SDK)/samples/Makefile.pref


# Local gsKit HIRES override.
#
# Source lineage:
#   gsKit 43122eb96289167975b56caa45beb71eb8684fa2
#
# Compilation options preserve the exact tested gsKit HIRES code shape.
ps2vnc_gsHires.o: ps2vnc_gsHires.c
	mips64r5900el-ps2-elf-gcc \
		-D_EE \
		-DF_gsKit_hires_init_screen=1 \
		-G0 \
		-march=r5900 \
		-mhard-float \
		-msingle-float \
		-mno-llsc \
		-mno-shared \
		-mplt \
		-O2 \
		-g \
		-flto=auto \
		-fno-fat-lto-objects \
		-I$(GSKIT)/include \
		-I$(PS2SDK)/ee/include \
		-I$(PS2SDK)/common/include \
		-c $< -o $@

include $(PS2SDK)/samples/Makefile.eeglobal

# M3 coarse migration scaffold.
ps2ip.o: ps2vnc_macro_body.inc
