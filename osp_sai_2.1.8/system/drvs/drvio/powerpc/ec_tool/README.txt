usage:
insmod ec_tool.ko dev=xx ec_limit=xx
rmmod ec_tool

dev      : mtd device number, for mtd1 is 1
ec_limit : the block info will be print out if the erase conter exceed ec_limit