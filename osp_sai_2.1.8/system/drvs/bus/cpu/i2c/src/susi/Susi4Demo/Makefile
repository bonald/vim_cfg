$(info ***** SUSI4.0 Application (Linux) *****)

SUSI_LINUX_DRV_PATH = ../Driver/

TARGET=susidemo4
LIB4_NAME = libSUSI-4.00
LIB_LINK_NAME = $(LIB4_NAME).so

LINKLIB = -Wl,-rpath,./ $(SUSI_LINUX_DRV_PATH)$(LIB_LINK_NAME)
CFLAGS = -D_LINUX -Wall -Werror -O2
DGFLAGS=-MMD -MP -MT $@ -MF $(dir $@)/$(*F).d

DEMO_OBJ = common.o wdog.o hwm.o gpio.o iic.o info.o smartfan.o smb.o storage.o thermalprotection.o vga.o main.o

include demo_env.mak

%.o : %.c
	$(CC) -c $(ARCHFLAG) -o $@ $< $(CFLAGS) $(DGFLAGS)

$(TARGET): $(DEMO_OBJ)
	rm -f $(TARGET)
	$(CC) $(ARCHFLAG) -o $@ $(DEMO_OBJ) $(LINKLIB)

clean:
	rm -f $(DEMO_OBJ) $(TARGET) $(DEMO_OBJ:.o=.d)

-include $(DEMO_OBJ:.o=.d)
