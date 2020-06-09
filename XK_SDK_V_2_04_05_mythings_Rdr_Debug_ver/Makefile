#Directory
DESKTOPDIR	= ../Desktop
SRCDIR		= src
HDRDIR  	= src
OBJDIR		= obj
BINDIR		= bin
TOOLSDIR	= tools
RLBINDIR	= /usr/bin
USRBINDIR	= /usr/local/bin
XKDIR		= /boot/XandarKardian
LOGDIR		= /var/log/xk
SYSLOGDIR	= /var/log/xk/sys
FALLLOGDIR	= ~/log
SCRTDIR		= /boot/XandarKardian/script
DOWNLOADDIR	= tmp
DEPDIR		= dep
DATDIR		= data
XKETCDIR	= /etc/xksdk

#File
CONFIGFILE	= config.json
PARAMFILE	= param.json
SCRIPTFLE	= xk.sh
OTAAPP		= ota
CMDAPP		= s2r
UARTAPP		= uart_app
RESTARTAPP	= restartxksdk

PYBTNDIR	= py

CC	= gcc
TARGET	= xksdk
TARGET_DBG	= xksdk_debug
CFLAGS	= -w -Wall -Wextra -pedantic -g -O0 -I$(HDRDIR)
# CFLAGS	= -Wall -Wextra -pedantic -g -O0 -I$(HDRDIR)
LFLAGS	= -lpthread -lwiringPi -lrt -lm -lcrypto -lssl

SOURCES	:= $(wildcard $(SRCDIR)/*.c)
HEADERS := $(wildcard $(HDRDIR)/*.h)
OBJECTS	:= $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
DEPS	:= $(OBJECTS:$(OBJDIR)/%.o=$(DEPDIR)/%.d)

REMOVE	:= rm -rf
BUILD_PRINT = @echo -e "\e[1;34mBuilding $<\e[0m"

# Linking
$(BINDIR)/$(TARGET): $(OBJECTS) $(CMN_OBJ)
	mkdir -p $(BINDIR)
	$(CC) $(LFLAGS) -o $@ $(OBJECTS) $(CMN_OBJ)
	
	sudo killall -2 xksdk || true

	sudo cp $(BINDIR)/$(TARGET) $(RLBINDIR)
# @echo "Linking complete"
	@echo "================================================"
	@echo "- Linking complete $@ "
	@echo "================================================"

-include $(DEPS)

# Compilation
$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	mkdir -p $(DEPDIR)
	$(CC) -c $(CFLAGS) $< -o $@
	$(CC) -I$(HDRDIR) -MM -MT '$(OBJDIR)/$*.o' $(SRCDIR)/$*.c > $(DEPDIR)/$*.d
# @echo "Compiled $<"
	
	@echo "================================================"
	@echo "- Compiled $< "
	@echo "================================================"

# Generate file list for cscope
cscope.files: $(SOURCES) $(HEADERS)
	echo $(SOURCES) $(HEADERS) > cscope.files

# Generate cscope database
cscope.out: cscope.files
	cscope -q -R -b -i cscope.files

.PHONY: clean
clean:
	$(REMOVE) $(OBJECTS) $(OBJDIR) $(BINDIR) $(DEPDIR)
# @echo "Deleted $<"
	@echo "================================================"
	@echo "- Deleted $< "
	@echo "================================================"

.PHONY: remove
remove:
	$(REMOVE) $(BINDIR)/$(TARGET)
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(DEPS)
	$(REMOVE) cscope.*
# @echo "Deleted $<"
	@echo "================================================"
	@echo "- Deleted $< "
	@echo "================================================"

.PHONY: cscope
cscope: cscope.out

.PHONY: installpkg
installpkg: 
	sudo apt-get install -y bluez libbluetooth-dev pi-bluetooth
	sudo pip3 install rpi_ws281x adafruit-circuitpython-neopixel pybluez
	@echo "================================================"
	@echo "- Package Install complete"
	@echo "================================================"

.PHONY: py
py: 
	sudo mkdir -p $(LOGDIR)/$(PYBTNDIR)
	sudo cp -r $(PYBTNDIR)/* $(LOGDIR)/$(PYBTNDIR)/
	sudo chmod 777 $(LOGDIR)/$(PYBTNDIR)/*
	sudo cp -r $(LOGDIR)/$(PYBTNDIR)/*.desktop $(DESKTOPDIR)/
	@echo "================================================"
	@echo "- py setting complete"
	@echo "================================================"

.PHONY: install
install: 
	sudo mkdir -p $(LOGDIR)/$(PYBTNDIR)
	sudo cp -r $(PYBTNDIR)/* $(LOGDIR)/$(PYBTNDIR)/
	sudo mkdir -p $(XKDIR) 
	sudo mkdir -p $(XKDIR)/tools
	sudo mkdir -p $(LOGDIR) 
	sudo mkdir -p $(SYSLOGDIR) 
	sudo mkdir -p $(FALLLOGDIR) 
	sudo mkdir -p $(FALLLOGDIR)/data 
	sudo mkdir -p $(SCRTDIR) 
	sudo mkdir -p $(SCRTDIR)/$(DOWNLOADDIR)
	sudo mkdir -p $(XKETCDIR) 
	sudo touch $(XKETCDIR)/pre-pid
	sudo cp $(BINDIR)/$(TARGET) $(RLBINDIR)
	sudo cp $(CONFIGFILE) $(XKDIR)
	sudo cp $(PARAMFILE) $(XKDIR)
	sudo cp $(TOOLSDIR)/$(OTAAPP) $(XKDIR)/tools
	sudo chmod 777 $(TOOLSDIR)/$(CMDAPP)
	sudo chmod 777 $(TOOLSDIR)/$(UARTAPP)
	sudo chmod 777 $(TOOLSDIR)/$(RESTARTAPP)/$(RESTARTAPP)
	sudo cp $(TOOLSDIR)/$(CMDAPP) $(USRBINDIR)
	sudo cp $(TOOLSDIR)/$(UARTAPP) $(USRBINDIR)
	sudo cp $(TOOLSDIR)/$(RESTARTAPP)/$(RESTARTAPP) $(USRBINDIR)
	sudo cp $(SCRIPTFLE) $(SCRTDIR)
	sudo cp rc.local /etc/rc.local
	sudo cp $(BINDIR)/$(TARGET) $(RLBINDIR)
	sudo mkdir -p $(LOGDIR)/$(PYBTNDIR)
	sudo cp -r $(PYBTNDIR)/* $(LOGDIR)/$(PYBTNDIR)/
	sudo cp $(PYBTNDIR)/$(TARGET_DBG) $(RLBINDIR)/$(TARGET)
	@echo "================================================"
	@echo "- Install complete"
	@echo "================================================"

.PHONY: bin
bin: 
	sudo cp -r $(PYBTNDIR)/* $(LOGDIR)/$(PYBTNDIR)/
	sudo cp $(BINDIR)/$(TARGET) $(RLBINDIR)
	@echo "================================================"
	@echo "- Copy bin complete"
	@echo "================================================"

.PHONY: dbgbin
dbgbin: 
	sudo cp -r $(PYBTNDIR)/* $(LOGDIR)/$(PYBTNDIR)/
	sudo cp $(PYBTNDIR)/$(TARGET_DBG) $(RLBINDIR)/$(TARGET)
	@echo "================================================"
	@echo "- Copy bin complete"
	@echo "================================================"

.PHONY: config
config: 
	sudo cp $(CONFIGFILE) $(XKDIR)
	sudo cp $(PARAMFILE) $(XKDIR)
	@echo "================================================"
	@echo "- Copy config.json complete"
	@echo "================================================"

.PHONY: dirinstall
dirinstall: 
	sudo mkdir -p $(XKDIR) 
	sudo mkdir -p $(LOGDIR) 
	sudo mkdir -p $(SYSLOGDIR) 
	@echo "================================================"
	@echo "- Make dir complete"
	@echo "================================================"

.PHONY: kill
kill: 
	sudo killall -2 $(TARGET)
	@echo "================================================"
	@echo "- Kill complete"
	@echo "================================================"


.PHONY: all
all: $(BINDIR)/$(TARGET)


