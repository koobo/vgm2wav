# Cross-compile to Amiga

VBCC ?= /opt/amiga
GCC ?= $(VBCC)/bin/m68k-amigaos-gcc
GPP ?= $(VBCC)/bin/m68k-amigaos-g++
STRIP ?= $(VBCC)/bin/m68k-amigaos-strip
OBJCOPY ?= $(VBCC)/bin/m68k-amigaos-objcopy
CFLAGS :=  -m68060 -mhard-float -O2 -noixemul -fomit-frame-pointer -mno-bitfield -Ilibgme -DBLARGG_BIG_ENDIAN -DBLIP_BUFFER_FAST

TARGET := vgm2wav

SRC := app/main.c \
	app/wave_writer.c \
	libgme/gme/Ay_Apu.cpp \
	libgme/gme/Ay_Cpu.cpp \
	libgme/gme/Ay_Emu.cpp \
	libgme/gme/Blip_Buffer.cpp \
	libgme/gme/Classic_Emu.cpp \
	libgme/gme/Data_Reader.cpp \
	libgme/gme/Dual_Resampler.cpp \
	libgme/gme/Effects_Buffer.cpp \
	libgme/gme/Fir_Resampler.cpp \
	libgme/gme/Gb_Apu.cpp \
	libgme/gme/Gb_Cpu.cpp \
	libgme/gme/Gb_Oscs.cpp \
	libgme/gme/Gbs_Emu.cpp \
	libgme/gme/Gme_File.cpp \
	libgme/gme/gme.cpp \
	libgme/gme/Gym_Emu.cpp \
	libgme/gme/Hes_Apu.cpp \
	libgme/gme/Hes_Cpu.cpp \
	libgme/gme/Hes_Emu.cpp \
	libgme/gme/Kss_Cpu.cpp \
	libgme/gme/Kss_Emu.cpp \
	libgme/gme/Kss_Scc_Apu.cpp \
	libgme/gme/M3u_Playlist.cpp \
	libgme/gme/Multi_Buffer.cpp \
	libgme/gme/Music_Emu.cpp \
	libgme/gme/Nes_Apu.cpp \
	libgme/gme/Nes_Cpu.cpp \
	libgme/gme/Nes_Fme7_Apu.cpp \
	libgme/gme/Nes_Namco_Apu.cpp \
	libgme/gme/Nes_Oscs.cpp \
	libgme/gme/Nes_Vrc6_Apu.cpp \
	libgme/gme/Nsf_Emu.cpp \
	libgme/gme/Nsfe_Emu.cpp \
	libgme/gme/Sap_Apu.cpp \
	libgme/gme/Sap_Cpu.cpp \
	libgme/gme/Sap_Emu.cpp \
	libgme/gme/Sms_Apu.cpp \
	libgme/gme/Snes_Spc.cpp \
	libgme/gme/Spc_Cpu.cpp \
	libgme/gme/Spc_Dsp.cpp \
	libgme/gme/Spc_Emu.cpp \
	libgme/gme/Spc_Filter.cpp \
	libgme/gme/Vgm_Emu_Impl.cpp \
	libgme/gme/Vgm_Emu.cpp \
 	libgme/gme/Ym2612_Emu.cpp 

all: $(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).sym mo_ym2413_emu.zip

mo_ym2413_emu.zip: 
	wget https://www.slack.net/~ant/libs/mo_ym2413_emu.zip

libgme/gme/Ym2413_Emu.cpp: mo_ym2413_emu.zip
	cd libgme/gme && unzip -jo ../../mo_ym2413_emu.zip

$(TARGET).sym: $(SRC) libgme/gme/Ym2413_Emu.cpp
	$(GPP) $(CFLAGS) -Wl,-Map,$(TARGET).map,--cref $^ -o $@

$(TARGET): $(TARGET).sym
	$(STRIP) $^ -o $@

 
