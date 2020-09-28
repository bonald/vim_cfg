ifeq ($(ARCH), mips)
mychecksum : sendmsg.c 
	mipsel-linux-gcc-4.4.7 -g -Wall  -o sendmsg sendmsg.c
endif
ifeq ($(ARCH), arm64)
mychecksum : sendmsg.c 
	aarch64-linux-gnu-gcc -g -Wall  -o sendmsg sendmsg.c
	if [ ! -d $(BLD_DIR)/bin ]; then mkdir -p $(BLD_DIR)/bin/; fi
	cp sendmsg $(BLD_DIR)/bin/
endif
clean :
	rm  sendmsg 

