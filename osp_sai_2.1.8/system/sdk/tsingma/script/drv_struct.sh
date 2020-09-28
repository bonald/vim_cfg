cd ../driver/goldengate/include/

cat drv_enum.h | sed -n -e '/^#define \w\+_BYTES\s\+[0-9]\+$/,/^$/p' | sed -e '/^$/d;s/^#define \(\w\+\)_BYTES\s\+[0-9]\+$/\1/' | awk '{print "typedef struct {uint32 m["$0"_BYTES/4];} "tolower($0)"_m;"}' > drv_struct.h
