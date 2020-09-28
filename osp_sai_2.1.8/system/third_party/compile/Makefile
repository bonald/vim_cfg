targets = 
targets += i2c-tools

clean_targets = $(targets:%=clean_%)
install_targets = $(targets:%=install_%)

all : $(targets)
clean : $(clean_targets)
install : prepare $(install_targets)

.PHONY : all clean install
.PHONY : $(targets) $(clean_targets) $(install_targets)

prepare:
	@if [ -z $(INSTALL_DIR_BIN) ]; then \
		echo "error: INSTALL_DIR_BIN is empty!"; exit 1; \
	else mkdir -p $(INSTALL_DIR_BIN)/; fi

i2c_tools_version=i2c-tools-3.0.2
i2c-tools :
	cd $(i2c_tools_version)/ && make
	cd $(i2c_tools_version)/ && make -C eepromer/

clean_i2c-tools :
	cd $(i2c_tools_version)/ && make clean
	cd $(i2c_tools_version)/ && make -C eepromer/ clean

install_i2c-tools: i2c-tools
	cd $(i2c_tools_version)/ && cd tools && cp i2cdetect i2cdump i2cget i2cset $(INSTALL_DIR_BIN)/
	cd $(i2c_tools_version)/ && cd eepromer && cp eeprom $(INSTALL_DIR_BIN)/