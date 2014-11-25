
#------------------------ Olimex Options -----------------------
OLIMEX_JTAG_DEFAULT_DEV = /dev/olimex
OLIMEX_JTAG_ALTERNATIVE = /dev/ttyUSB0

TEST = test
OLIMEX_JTAG_DEV ?= $(shell $(TEST) -c $(OLIMEX_JTAG_DEFAULT_DEV) && echo $(OLIMEX_JTAG_DEFAULT_DEV) || echo $(OLIMEX_JTAG_ALTERNATIVE) )

#============================================================================

# List of HADCON types
HADCON_TYPES ?= 1 2 
HADCON_MAKEFILE ?= Makefile_HadconX
HADCON_TARGET_BASE ?= api
HADCON_TARGET_SUFFIX ?= _hadcon_
HADCON_VERSION_DEFAULT ?= 2

ifndef HADCON_VERSION
HADCON_VERSION=$(HADCON_VERSION_DEFAULT)
endif 

# Default target.
all: all$(HADCON_TARGET_SUFFIX)$(HADCON_VERSION)
	
#all_hadcon_%: 
all$(HADCON_TARGET_SUFFIX)%: touch
	@HADCON_VERSION=$(*F) TARGET=$(HADCON_TARGET_BASE)_hadcon$(*F) $(MAKE) -f $(HADCON_MAKEFILE) $(subst $(HADCON_TARGET_SUFFIX)$(*F),,$@) 

both: $(foreach hadcon, $(HADCON_TYPES), all$(HADCON_TARGET_SUFFIX)$(hadcon))
	
help:
	@echo -e '\n  Available (reasonable) targets: \n\t(automatically determined)\n' 
	@targets=$$( $(MAKE) --print-data-base --dry-run | grep '^[[:alpha:]_-]*:.*$$' | grep -v '^Makefile.*:$$' | cut -d ":" -f 1 | sort | uniq); \
	list=""; \
	for item in $$( echo $${targets}); \
	do \
		echo $$list | grep -q -w $$item; \
		if [ 1 -eq "$$?" ]; \
		then \
		list="$$list $$item"; \
		fi; \
	done; \
	for item in $$( echo $${list} ); \
	do \
		echo -e "\t$$item"; \
	done; \
    echo 
    
# Display compiler version information.
gccversion : 
	@$(MAKE) -f $(HADCON_MAKEFILE) $@

#Set fuses - initial use of device
set_fuses: $(foreach hadcon, $(HADCON_VERSION), set_fuses$(HADCON_TARGET_SUFFIX)$(hadcon))
	
set_fuses_hadcon_%:
	@HADCON_VERSION=$(*F) TARGET=$(HADCON_TARGET_BASE)_hadcon$(*F) $(MAKE) -f $(HADCON_MAKEFILE) $(subst $(HADCON_TARGET_SUFFIX)$(*F),,$@)

# Program the device.  
program: $(foreach hadcon, $(HADCON_VERSION), program$(HADCON_TARGET_SUFFIX)$(hadcon))
	
program_hadcon_%:
#make sure api_version.c gets newest compile date
	@if [ -c "$(OLIMEX_JTAG_DEV)" ]; \
	then \
	    [ -f "api_version.c" ] && touch api_version.c \
		echo 'HADCON_VERSION=$(*F) TARGET=$(HADCON_TARGET_BASE)_hadcon$(*F) $(MAKE) -f $(HADCON_MAKEFILE) $(subst $(HADCON_TARGET_SUFFIX)$(*F),,$@)'; \
		HADCON_VERSION=$(*F) TARGET=$(HADCON_TARGET_BASE)_hadcon$(*F) $(MAKE) -f $(HADCON_MAKEFILE) $(subst $(HADCON_TARGET_SUFFIX)$(*F),,$@); \
	else \
		echo "\"$(OLIMEX_JTAG_DEV)\" is not a valid olimex jtag device"; \
	fi;  

	
## Generate avr-gdb config/init file which does the following:
##     define the reset signal, load the target file, connect to target, and set 
##     a breakpoint at main().
#gdb-config: 
#	@$(REMOVE) $(GDBINIT_FILE)
#	@echo define reset >> $(GDBINIT_FILE)
#	@echo SIGNAL SIGHUP >> $(GDBINIT_FILE)
#	@echo end >> $(GDBINIT_FILE)
#	@echo file $(TARGET).elf >> $(GDBINIT_FILE)
#	@echo target remote $(DEBUG_HOST):$(DEBUG_PORT)  >> $(GDBINIT_FILE)
#ifeq ($(DEBUG_BACKEND),simulavr)
#	@echo load  >> $(GDBINIT_FILE)
#endif
#	@echo break main >> $(GDBINIT_FILE)
#
#debug: gdb-config $(TARGET).elf
#ifeq ($(DEBUG_BACKEND), avarice)
#	@echo Starting AVaRICE - Press enter when "waiting to connect" message displays.
#	-killall -q avr-gdbtui avarice
#	sleep 1
#	-killall -q -9 avr-gdbtui avarice
#	sleep 1
#	#MKII
#	#avarice -2 -c 0,1,0,4 --jtag $(JTAG_DEV) -B 1000000 --erase --program --file \
#	$(TARGET).elf $(DEBUG_HOST):$(DEBUG_PORT) &
#	# Olimex
#	@echo avarice  -c 0,1,0,4/8 --jtag $(OLIMEX_JTAG_DEV) -B 1000000 --erase --program --file \
#	avarice  -c 0,1,0,4 --jtag $(OLIMEX_JTAG_DEV) -B 1000000 --erase --program --file $(TARGET).elf $(DEBUG_HOST):$(DEBUG_PORT) || \ 
#	avarice  -c 0,1,0,8 --jtag $(OLIMEX_JTAG_DEV) -B 1000000 --erase --program --file$(TARGET).elf $(DEBUG_HOST):$(DEBUG_PORT) 
#	@sleep 3
#
#else
#	@$(WINSHELL) /c start simulavr --gdbserver --device $(MCU) --clock-freq \
#	$(DEBUG_MFREQ) --port $(DEBUG_PORT)
#endif
#	sleep 12
#	avr-gdbtui -x gdb_env $(TARGET).elf
#	#@$(DEBUG_UI) -r localhost:4242 $(TARGET).elf




# Convert ELF to COFF for use in debugging / simulating in AVR Studio or VMLAB.
COFFCONVERT = $(OBJCOPY) --debugging
COFFCONVERT += --change-section-address .data-0x800000
COFFCONVERT += --change-section-address .bss-0x800000
COFFCONVERT += --change-section-address .noinit-0x800000
COFFCONVERT += --change-section-address .eeprom-0x810000

coff: $(foreach hadcon, $(HADCON_TYPES), coff$(HADCON_TARGET_SUFFIX)$(hadcon))
	
coff_hadcon_%:
	@HADCON_VERSION=$(*F) TARGET=$(HADCON_TARGET_BASE)_hadcon$(*F) $(MAKE) -f $(HADCON_MAKEFILE) $(subst $(HADCON_TARGET_SUFFIX)$(*F),,$@)

extcoff: $(foreach hadcon, $(HADCON_TYPES), extcoff$(HADCON_TARGET_SUFFIX)$(hadcon))
	
extcoff_hadcon_%:
	@HADCON_VERSION=$(*F) TARGET=$(HADCON_TARGET_BASE)_hadcon$(*F) $(MAKE) -f $(HADCON_MAKEFILE) $(subst $(HADCON_TARGET_SUFFIX)$(*F),,$@)

# Target: clean project.
clean: $(foreach hadcon, $(HADCON_TYPES), clean$(HADCON_TARGET_SUFFIX)$(hadcon))
	
clean_hadcon_%:
	@HADCON_VERSION=$(*F) TARGET=$(HADCON_TARGET_BASE)_hadcon$(*F) $(MAKE) -f $(HADCON_MAKEFILE) $(subst $(HADCON_TARGET_SUFFIX)$(*F),,$@)

# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex eep lss sym coff extcoff \
clean clean_list program debug gdb-config \
set_fuses help

# User added targets (PZ)
debug_makefile:
	DEBUG=yes $(MAKE) -n
touch: touch.sh
	@$$PWD/touch.sh