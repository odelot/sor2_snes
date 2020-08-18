# path to snesdev root directory (for emulators, devkitsnes, libsnes)
export DEVKITSNES := /c/snesdev/

# path to devkitsnes root directory for compiler
export DEVKIT65XX := /c/snesdev/devkitsnes

#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKIT65XX)),)
$(error "Please set DEVKIT65XX in your environment. export DEVKIT65XX=<path to>devkit65XX")
endif

include $(DEVKIT65XX)/snes_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	$(shell basename $(CURDIR))
SOURCES		:=	.

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
CFLAGS	+=	$(INCLUDE) 

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:=	$(PVSNESLIB)
LIBOBJS +:=	
 
export OUTPUT	:=	$(CURDIR)/$(TARGET)
 
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.asm)))
 
export AUDIOFILES :=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.it)))

ifneq ($(AUDIOFILES),)
ifeq ($(wildcard soundbank.asm),)
	SFILES		:=	soundbank.asm $(SFILES)
endif
endif

#---------------------------------------------------------------------------------
export OFILES	:=	$(BINFILES:.bin=.obj) $(CFILES:.c=.obj) $(SFILES:.asm=.obj)
 
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD)

SMCONVFLAGS	:= -l -s -o soundbank -v -b 5

GTITLE 		:= -ht"$(TARGET)"
 
.PHONY: bitmaps all
 
#---------------------------------------------------------------------------------
all	:	bitmaps $(OUTPUT).sfc
	$(SNTOOLS) -hi! $(GTITLE) $(TARGET).sfc

clean:
	@echo clean ...
	@rm -f $(OFILES) $(TARGET).sfc $(TARGET).sym *.pic *.pal *.map *.bnk soundbank.asm soundbank.h


#---------------------------------------------------------------------------------
soundbank.asm : $(AUDIOFILES)
	@echo Compiling Soundbank ... 
	@$(SMCONV) $(SMCONVFLAGS) $^

#---------------------------------------------------------------------------------

blaze.pic: blaze.bmp
	@echo convert bitmap ... $(notdir $@)
	$(GFXCONV) -gs64 -pc16 -po16 -pe2 -n $<

galsia.pic: galsia.bmp
	@echo convert bitmap ... $(notdir $@)
	$(GFXCONV) -gs64 -pc16 -po16 -pe3 -n $<

signal.pic: signal.bmp
	@echo convert bitmap ... $(notdir $@)
	$(GFXCONV) -gs64 -pc16 -po16 -pe4 -n $<

frames.pic: frames.bmp
	@echo convert bitmap ... $(notdir $@)
	$(GFXCONV) -gs32 -pc16 -po16 -pe0 -n $<

caixa.pic: caixa.bmp
	@echo convert bitmap ... $(notdir $@)
	$(GFXCONV) -gs64 -pc16 -po16 -n $<

fase11BG1.pic: fase11BG1.bmp
	@echo convert bitmap ... $(notdir $@)
	$(GFXCONV) -pr -pc16 -n -gs8 -pe2 -fbmp -m -m32p $<

hud.pic: hud.bmp
	@echo convert bitmap ... $(notdir $@)
	
	$(GFXCONV) -pr -pc4 -n -gs8 -pe0 -fbmp -mp -mR!  $<

fase11BG2.pic: fase11BG2.bmp
	@echo convert bitmap ... $(notdir $<)
	$(GFXCONV) -pr -pc16 -n -gs8 -pe4 -fbmp -mp -m32p $<

soco.brr: soco.wav
	@echo convert wav file ... $(notdir $<)
	$(BRCONV) -e $< $@

dano.brr: dano.wav
	@echo convert wav file ... $(notdir $<)
	$(BRCONV) -e $< $@

bitmaps :  fase11BG1.pic soundbank.asm blaze.pic  fase11BG2.pic caixa.pic galsia.pic signal.pic hud.pic frames.pic soco.brr dano.brr

#---------------------------------------------------------------------------------
$(OUTPUT).sfc	: $(OFILES)
