#!/usr/bin/tclsh
set src_file "ctc_api.txt"
set fp_src [open $src_file r+]

set dst_file_api_h "ctc_api.h"
set fp_dst_api_h [open $dst_file_api_h w+]

set dst_file_apis_h "ctcs_api.h"
set fp_dst_apis_h [open $dst_file_apis_h w+]

set dst_file_api_c "ctc_api.c"
set fp_dst_api_c [open $dst_file_api_c w+]

set dst_file_apis_c "ctcs_api.c"
set fp_dst_apis_c [open $dst_file_apis_c w+]

set all_chip [string trim [lindex $argv 0]]
set module [string trim [lindex $argv 1]]

set file_temp [open "file_temp.txt" r+]
seek $file_temp 0 start
set buffer_temp [read $file_temp]
set delet_file_H 0


foreach chipname $all_chip {
    set dst_file_chip_api_c($chipname) "$chipname\_api.c"
    set fp_dst_chip_api_c($chipname) [open $dst_file_chip_api_c($chipname) w+]
}

set api_cnt 0


#########################################################################
#
#write  ctc_api.h 's header part
#########################################################################
puts $fp_dst_api_h "/**********************************************************"
puts $fp_dst_api_h " * ctc_api.h"
puts $fp_dst_api_h " * Date:"
puts $fp_dst_api_h " * Author: auto generate from include file"
puts $fp_dst_api_h " **********************************************************/"
puts $fp_dst_api_h "#ifndef _CTC_API_H"
puts $fp_dst_api_h "#define _CTC_API_H"
puts $fp_dst_api_h "#ifdef __cplusplus"
puts $fp_dst_api_h "extern \"C\" {"
puts $fp_dst_api_h "#endif"
puts $fp_dst_api_h "/**********************************************************"
puts $fp_dst_api_h " * "
puts $fp_dst_api_h " * Header file"
puts $fp_dst_api_h " * "
puts $fp_dst_api_h " **********************************************************/"
puts $fp_dst_api_h "#include \"common/../../sal/include/sal_types.h\""
puts $fp_dst_api_h "#include \"common/../../dal/include/dal.h\""
puts $fp_dst_api_h "#include \"common/include/ctc_error.h\""
puts $fp_dst_api_h "#include \"common/include/ctc_const.h\""
if { $chipname == "duet2" } {
	puts $fp_dst_api_h "#include \"common/include/ctc_field.h\""
}

foreach file $module {
	if {[file exists "../../core/common/include/ctc_$file\.h"]} {
		puts $fp_dst_api_h "#include \"common/include/ctc_$file\.h\""
	}
}
puts $fp_dst_api_h "#include \"app/ctc_app_vlan_port_api.h\""
puts $fp_dst_api_h "#include \"app/ctc_app_index.h\""
puts $fp_dst_api_h "#include \"init/ctc_init.h\""

puts $fp_dst_api_h "/**********************************************************"
puts $fp_dst_api_h " * "
puts $fp_dst_api_h " * Defines and macros"
puts $fp_dst_api_h " * "
puts $fp_dst_api_h " **********************************************************/"
puts $fp_dst_api_h "struct ctc_api_s"
puts $fp_dst_api_h "{"

#########################################################################
#
#write  ctc_api.h 's header part
#########################################################################
puts $fp_dst_apis_h "/**********************************************************"
puts $fp_dst_apis_h " * ctcs_api.h"
puts $fp_dst_apis_h " * Date:"
puts $fp_dst_apis_h " * Author: auto generate from include file"
puts $fp_dst_apis_h " **********************************************************/"
puts $fp_dst_apis_h "#ifndef _CTC_APIS_H"
puts $fp_dst_apis_h "#define _CTC_APIS_H"
puts $fp_dst_apis_h "#ifdef __cplusplus"
puts $fp_dst_apis_h "extern \"C\" {"
puts $fp_dst_apis_h "#endif"
puts $fp_dst_apis_h "/**********************************************************"
puts $fp_dst_apis_h " * "
puts $fp_dst_apis_h " * Header file"
puts $fp_dst_apis_h " * "
puts $fp_dst_apis_h " **********************************************************/"
puts $fp_dst_apis_h "#include \"common/../../sal/include/sal_types.h\""
puts $fp_dst_apis_h "#include \"common/../../dal/include/dal.h\""
puts $fp_dst_apis_h "#include \"common/include/ctc_error.h\""
puts $fp_dst_apis_h "#include \"common/include/ctc_const.h\""
if {$chipname == "usw"} {
	puts $fp_dst_apis_h "#include \"common/include/ctc_field.h\""
}
foreach file $module {
	if {[file exists "../../core/common/include/ctc_$file\.h"]} {
		puts $fp_dst_apis_h "#include \"common/include/ctc_$file\.h\""
	}
}
puts $fp_dst_apis_h "#include \"app/ctc_app_vlan_port_api.h\""
puts $fp_dst_apis_h "#include \"app/ctc_app_index.h\""
puts $fp_dst_apis_h "#include \"init/ctc_init.h\""

puts $fp_dst_apis_h "/**********************************************************"
puts $fp_dst_apis_h " * "
puts $fp_dst_apis_h " * Defines and macros"
puts $fp_dst_apis_h " * "
puts $fp_dst_apis_h " **********************************************************/"
puts $fp_dst_apis_h "struct ctcs_api_s"
puts $fp_dst_apis_h "{"

###########################################################################
#
#write ctc_api.c's header part
###########################################################################
puts $fp_dst_api_c "/**********************************************************"
puts $fp_dst_api_c " * ctc_api.c"
puts $fp_dst_api_c " * Date:"
puts $fp_dst_api_c " * Author: auto generate from include file"
puts $fp_dst_api_c " **********************************************************/"
puts $fp_dst_api_c "/**********************************************************"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " * Header file"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " **********************************************************/"
puts $fp_dst_api_c "#include \"ctc_api.h\""
puts $fp_dst_api_c "#include \"ctcs_api.h\""
puts $fp_dst_api_c "#include \"dal.h\""
puts $fp_dst_api_c "/**********************************************************"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " * Defines and macros"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " **********************************************************/"
puts $fp_dst_api_c "/**********************************************************"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " * Global and Declaration"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " **********************************************************/"
puts $fp_dst_api_c "ctcs_api_t *ctc_api = NULL;"
puts $fp_dst_api_c "uint8 ctc_chip_type = 0;;"
foreach chipname $all_chip {
    puts $fp_dst_api_c "#ifdef [string toupper $chipname]"
    puts $fp_dst_api_c "extern ctcs_api_t ctc_$chipname\_api;"
    puts $fp_dst_api_c "#endif"
}
puts $fp_dst_api_c "/**********************************************************"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " * Functions"
puts $fp_dst_api_c " * "
puts $fp_dst_api_c " **********************************************************/"
puts $fp_dst_api_c "int32 ctc_install_api()"
puts $fp_dst_api_c "\{"
puts $fp_dst_api_c "    uint32 dev_id = 0;"
puts $fp_dst_api_c "    dal_get_chip_dev_id(0, &dev_id);"
puts $fp_dst_api_c "    switch (dev_id)"
puts $fp_dst_api_c "    \{"
foreach chipname $all_chip {

if { $chipname == "usw"} {
	puts $fp_dst_api_c "#ifdef [string toupper $chipname]"
    puts $fp_dst_api_c "        case DAL_[string toupper "duet2"]_DEVICE_ID:"
    puts $fp_dst_api_c "            ctc_api = &ctc_$chipname\_api;"
    puts $fp_dst_api_c "            ctc_chip_type = CTC_CHIP_DUET2;"
    puts $fp_dst_api_c "            break;"

    puts $fp_dst_api_c "        case DAL_[string toupper "tsingma"]_DEVICE_ID:"
    puts $fp_dst_api_c "            ctc_api = &ctc_$chipname\_api;"
    puts $fp_dst_api_c "            ctc_chip_type = CTC_CHIP_TSINGMA;"
    puts $fp_dst_api_c "            break;"
    puts $fp_dst_api_c "#endif"
} else {
    puts $fp_dst_api_c "#ifdef [string toupper $chipname]"
    puts $fp_dst_api_c "        case DAL_[string toupper $chipname]_DEVICE_ID:"
    if {$chipname == "goldengate"} {
        puts $fp_dst_api_c "        case DAL_[string toupper $chipname]_DEVICE_ID1:"
    }
    puts $fp_dst_api_c "            ctc_api = &ctc_$chipname\_api;"
    puts $fp_dst_api_c "            ctc_chip_type = CTC_CHIP_[string toupper $chipname];"
    puts $fp_dst_api_c "            break;"
    puts $fp_dst_api_c "#endif"
}
}
puts $fp_dst_api_c "        default:"
puts $fp_dst_api_c "            return CTC_E_INVALID_PARAM;"
puts $fp_dst_api_c "            break;"
puts $fp_dst_api_c "    \}"
puts $fp_dst_api_c "    return CTC_E_NONE;"
puts $fp_dst_api_c "\} "

puts $fp_dst_api_c "int32 ctc_uninstall_api()
\{
    ctc_api = NULL;
    return CTC_E_NONE;
\}
char* ctc_get_chip_name()
\{
    char* chip_name\[\] = \{NULL, \"Greatbelt\", \"Goldengate\", \"Duet2\", \"TsingMa\"\};
    return chip_name\[ctc_chip_type\];
\}
\CTC_EXPORT_SYMBOL(ctc_get_chip_name);
uint8 ctc_get_chip_type()
\{
    return ctc_chip_type;
\}
\CTC_EXPORT_SYMBOL(ctc_get_chip_type);"

###########################################################################
#
#write ctcs_api.c's header part
###########################################################################
puts $fp_dst_apis_c "/**********************************************************"
puts $fp_dst_apis_c " * ctcs_api.c"
puts $fp_dst_apis_c " * Date:"
puts $fp_dst_apis_c " * Author: auto generate from include file"
puts $fp_dst_apis_c " **********************************************************/"
puts $fp_dst_apis_c "/**********************************************************"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " * Header file"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " **********************************************************/"
puts $fp_dst_apis_c "#include \"ctc_api.h\""
puts $fp_dst_apis_c "#include \"ctcs_api.h\""
puts $fp_dst_apis_c "#include \"dal.h\""
puts $fp_dst_apis_c "/**********************************************************"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " * Defines and macros"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " **********************************************************/"
puts $fp_dst_apis_c "/**********************************************************"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " * Global and Declaration"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " **********************************************************/"
puts $fp_dst_apis_c "ctcs_api_t *ctcs_api\[CTC_MAX_LOCAL_CHIP_NUM\] = \{NULL\};"
puts $fp_dst_apis_c "uint8 ctcs_chip_type\[CTC_MAX_LOCAL_CHIP_NUM\] = \{0\};"
puts $fp_dst_apis_c "uint8 g_ctcs_api_en = 0;"
foreach chipname $all_chip {
    puts $fp_dst_apis_c "#ifdef [string toupper $chipname]"
    puts $fp_dst_apis_c "extern ctcs_api_t ctc_$chipname\_api;"
    puts $fp_dst_apis_c "#endif"
}
puts $fp_dst_apis_c "/**********************************************************"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " * Functions"
puts $fp_dst_apis_c " * "
puts $fp_dst_apis_c " **********************************************************/"
puts $fp_dst_apis_c "int32 ctcs_install_api(uint8 lchip)"
puts $fp_dst_apis_c "\{"
puts $fp_dst_apis_c "    uint32 dev_id = 0;"
puts $fp_dst_apis_c "    dal_get_chip_dev_id(lchip, &dev_id);"
puts $fp_dst_apis_c "    switch (dev_id)"
puts $fp_dst_apis_c "    \{"
foreach chipname $all_chip {

	if { $chipname == "usw"} {
	puts $fp_dst_apis_c "#ifdef [string toupper $chipname]"
    puts $fp_dst_apis_c "        case DAL_[string toupper "duet2"]_DEVICE_ID:"
    puts $fp_dst_apis_c "            ctcs_api\[lchip\] = &ctc_$chipname\_api;"
    puts $fp_dst_apis_c "            ctcs_chip_type\[lchip\] = CTC_CHIP_DUET2;"
    puts $fp_dst_apis_c "            break;"

    puts $fp_dst_apis_c "        case DAL_[string toupper "tsingma"]_DEVICE_ID:"
    puts $fp_dst_apis_c "            ctcs_api\[lchip\] = &ctc_$chipname\_api;"
    puts $fp_dst_apis_c "            ctcs_chip_type\[lchip\] = CTC_CHIP_TSINGMA;"
    puts $fp_dst_apis_c "            break;"
} else {
    puts $fp_dst_apis_c "#ifdef [string toupper $chipname]"
    puts $fp_dst_apis_c "        case DAL_[string toupper $chipname]_DEVICE_ID:"
    if {$chipname == "goldengate"} {
        puts $fp_dst_apis_c "        case DAL_[string toupper $chipname]_DEVICE_ID1:"
    }
    puts $fp_dst_apis_c "            ctcs_api\[lchip\] = &ctc_$chipname\_api;"
    puts $fp_dst_apis_c "            ctcs_chip_type\[lchip\] = CTC_CHIP_[string toupper $chipname];"
    puts $fp_dst_apis_c "            break;"
}
    puts $fp_dst_apis_c "#endif"
}
puts $fp_dst_apis_c "        default:"
puts $fp_dst_apis_c "            return CTC_E_INVALID_PARAM;"
puts $fp_dst_apis_c "            break;"
puts $fp_dst_apis_c "    \}"
puts $fp_dst_apis_c "    g_ctcs_api_en = 1;"
puts $fp_dst_apis_c "    return CTC_E_NONE;"
puts $fp_dst_apis_c "\}"
puts $fp_dst_apis_c "int32 ctcs_uninstall_api(uint8 lchip)"
puts $fp_dst_apis_c "\{"
puts $fp_dst_apis_c "    ctcs_api\[lchip\] = NULL;"
puts $fp_dst_apis_c "    return CTC_E_NONE;"
puts $fp_dst_apis_c "\}"
puts $fp_dst_apis_c "char* ctcs_get_chip_name(uint8 lchip)"
puts $fp_dst_apis_c "\{"
puts $fp_dst_apis_c "    char* chip_name\[\] = \{NULL, \"Greatbelt\", \"Godengate\", \"Duet2\", \"TsingMa\"\};"
puts $fp_dst_apis_c "    return chip_name\[ctcs_chip_type\[lchip\]\];"
puts $fp_dst_apis_c "\}"
puts $fp_dst_apis_c "CTC_EXPORT_SYMBOL(ctcs_get_chip_name);"
puts $fp_dst_apis_c "uint8 ctcs_get_chip_type(uint8 lchip)"
puts $fp_dst_apis_c "\{"
puts $fp_dst_apis_c "    return ctcs_chip_type\[lchip\];"
puts $fp_dst_apis_c "\}"
puts $fp_dst_apis_c "CTC_EXPORT_SYMBOL(ctcs_get_chip_type);"
#############################################################################
#
#($chipname)_api.c's header part
#############################################################################
foreach chipname $all_chip {
    puts $fp_dst_chip_api_c($chipname) "/**********************************************************"
    puts $fp_dst_chip_api_c($chipname) " * $chipname\_api.c"
    puts $fp_dst_chip_api_c($chipname) " * Date:"
    puts $fp_dst_chip_api_c($chipname) " * Author: auto generate from include file"
    puts $fp_dst_chip_api_c($chipname) " **********************************************************/"
    puts $fp_dst_chip_api_c($chipname) "/**********************************************************"
    puts $fp_dst_chip_api_c($chipname) " * "
    puts $fp_dst_chip_api_c($chipname) " * Header file"
    puts $fp_dst_chip_api_c($chipname) " * "
    puts $fp_dst_chip_api_c($chipname) " **********************************************************/"
    puts $fp_dst_chip_api_c($chipname) "#ifdef [string toupper $chipname]"
    puts $fp_dst_chip_api_c($chipname) "#include \"ctcs_api.h\""
    foreach file $module {
		foreach module_temp $buffer_temp {
			if { $module_temp == $file} {				
				set delet_file_H 1
			}
		}
		if { $delet_file_H == 0} {
			if {[file exists "../../core/$chipname/include/ctc/ctc_$chipname\_$file\.h"]} {
					puts $fp_dst_chip_api_c($chipname) "#include \"ctc_$chipname\_$file\.h\""
			}
		}
		set delet_file_H 0
    }
    puts $fp_dst_chip_api_c($chipname) "/**********************************************************"
    puts $fp_dst_chip_api_c($chipname) " * "
    puts $fp_dst_chip_api_c($chipname) " * Global and Declaration"
    puts $fp_dst_chip_api_c($chipname) " * "
    puts $fp_dst_chip_api_c($chipname) " **********************************************************/"
    puts $fp_dst_chip_api_c($chipname) "ctcs_api_t ctc_$chipname\_api ="
    puts $fp_dst_chip_api_c($chipname) "\{"
}

################################################################################
#
#Body part
###############################################################################
while {[gets $fp_src line] >= 0} {
    if {[string first "#" $line] == 0} {
		puts $fp_dst_api_h "    /*$line*/"
		puts $fp_dst_apis_h "    /*$line*/"
		puts $fp_dst_api_c "/*$line*/"
		puts $fp_dst_apis_c "/*$line*/"
        foreach chipname $all_chip {
		    puts $fp_dst_chip_api_c($chipname) "    /*$line*/"
        }
        continue
    }
    regsub -all {\s+} $line " " line
    set api_list [split $line ";"]

    set api [lindex $api_list 0]
    set ctc_api($api_cnt) $api
    set divider [string first "(" $api]
    set function [string trim [string range $api 0 [expr $divider - 1]]]
    set parameter [string trim [string range $api  $divider end]]

    #### APIs function and paramter
    set apis_function [string replace $function 0 2 "ctcs"]
    set apis_parameter $parameter

	if { [string equal $parameter "(void)"] == 1} {
		set para_type "(void)"
		set para_name "()"
	} elseif {[string equal $parameter "()"] == 1} {
		set para_type "(void)"
		set para_name "()"
	} else {
		set para_list [split $parameter ","]
		set para_num [llength $para_list]
		append para_name ""

    for {set i 0} {$i < $para_num} {incr i} {
			append para_type [lindex [split [string trim [lindex $para_list $i]] ] 0]
			if {$i != [expr $para_num -1]} {
				append para_type ", "
			}
			join $para_type
			set pre_name [lindex [split [string trim [lindex $para_list $i]] ] 1]
			regsub -all {\*} $pre_name "" pre_name
			append para_name $pre_name
			if {$i != [expr $para_num - 1]} {
				append para_name ", "
			}
		}
		append para_type "\)"
	}

         

        set temp $para_type

        set DefChip 1
        ###judge the paramter have lchip_id
        if {[regexp  "lchip_id," $apis_parameter]} {
            #puts "apis_parameter:${apis_parameter}\n"
            set DefChip 0
        }

        if { $DefChip == "1" } {
            if {[regexp "\\(uint8 lchip\\)" $parameter ]} {
			#puts "apis_parameter:${parameter}\n"
                regsub  {uint8} $temp "" temp
                regsub  {uint8 lchip} ${apis_parameter} "" apis_parameter
            } else {
                regsub  {uint8, } $temp "" temp
                regsub  {uint8 lchip, } ${apis_parameter} "" apis_parameter
            }
         
        }
        regsub  {lchip_id} $temp "lchip" temp
        regsub  {lchip_id} $para_type "lchip" para_type
        

	puts $fp_dst_api_h "    int32(*$function)$temp;"
	puts $fp_dst_apis_h "    int32(*${apis_function})$para_type;"

   	#####ctc_api.c's body part##
	puts $fp_dst_api_c "int32 $function${apis_parameter}"
	puts $fp_dst_api_c "\{"
        if { $DefChip == "1" } {
        	puts $fp_dst_api_c "    uint8 lchip = 0;"
        }
	puts $fp_dst_api_c "    CTC_PTR_VALID_CHECK(ctc_api);"
	if {[regexp {_init} $function] || [regexp {ctc_set_gchip_id} $function] || [regexp {ctc_ftm_mem_alloc} $function] || [regexp {ctc_chip_get_property} $function] || [regexp {ctc_chip_set_property} $function] || [regexp {ctc_set_chip_global_cfg} $function] || [regexp {ctc_global_ctl_set} $function]} {
		puts $fp_dst_api_c "    CTC_API_INIT_ERROR_RETURN(ctc_api->${apis_function}, $para_name;"
	} else {
		puts $fp_dst_api_c "    CTC_API_ERROR_RETURN(ctc_api->${apis_function}, $para_name;"
	}
	puts $fp_dst_api_c "\}"
	puts $fp_dst_api_c "CTC_EXPORT_SYMBOL($function);"
        puts $fp_dst_api_c " "

   	#####ctcs_api.c's body part##
    regsub  {lchip_id} $parameter "lchip" parameter
    regsub  {lchip_id} $para_name "lchip" para_name
	puts $fp_dst_apis_c "int32 ${apis_function}$parameter"
	puts $fp_dst_apis_c "\{"
	puts $fp_dst_apis_c "    CTC_PTR_VALID_CHECK(ctcs_api\[lchip\]);"
	if {[regexp {_init} $apis_function] || [regexp {ctcs_set_gchip_id} $apis_function] || [regexp {ctcs_ftm_mem_alloc} $apis_function] || [regexp {ctcs_chip_get_property} $apis_function] || [regexp {ctcs_chip_set_property} $apis_function] || [regexp {ctcs_set_chip_global_cfg} $apis_function] || [regexp {ctcs_global_ctl_set} $apis_function]} {
		puts $fp_dst_apis_c "    CTC_API_INIT_ERROR_RETURN(ctcs_api\[lchip\]->${apis_function}, $para_name;"
	} else {
		puts $fp_dst_apis_c "    CTC_API_ERROR_RETURN(ctcs_api\[lchip\]->${apis_function}, $para_name;"
	}
	puts $fp_dst_apis_c "\}"
	puts $fp_dst_apis_c "CTC_EXPORT_SYMBOL(${apis_function});"
        puts $fp_dst_apis_c " "

   	#####$chipname.c's body part##
    foreach chipname $all_chip {
        regsub ctc_ $function "ctc_$chipname\_" chip_func
        puts $fp_dst_chip_api_c($chipname) "    $chip_func,"
    }

	incr api_cnt
	unset para_type
	unset para_name
}
######################################################################
#
#ctc_api.h 's bottom part
#######################################################################
puts $fp_dst_api_h "};"
puts $fp_dst_api_h "typedef struct ctc_api_s ctc_api_t;"
puts $fp_dst_api_h "/**********************************************************"
puts $fp_dst_api_h " * "
puts $fp_dst_api_h " * Functions"
puts $fp_dst_api_h " * "
puts $fp_dst_api_h " **********************************************************/"
puts $fp_dst_api_h "extern int32 ctc_install_api();"
puts $fp_dst_api_h "extern int32 ctc_uninstall_api();"
puts $fp_dst_api_h "extern char* ctc_get_chip_name();"
puts $fp_dst_api_h "extern uint8 ctc_get_chip_type();"
for {set i 0} {$i < $api_cnt} {incr i} {
    set temp $ctc_api($i)
    set DefChip 1

    ###judge the paramter have lchip_id
	if {[regexp  "lchip_id," $temp]} {
        set DefChip 0
    }

    if { $DefChip == "1" } {

        if {[regexp "\\(uint8 lchip\\)" $temp]} {
            regsub  {uint8 lchip} $temp "" temp
        } else {
            regsub  {uint8 lchip, } $temp "" temp
        }
   
    }
    regsub  {lchip_id} $temp "lchip" temp
    puts $fp_dst_api_h "extern int32 $temp;"
}
puts $fp_dst_api_h " "

puts $fp_dst_api_h "#ifdef __cplusplus"
puts $fp_dst_api_h "}"
puts $fp_dst_api_h "#endif"

puts $fp_dst_api_h "#endif"
close $fp_dst_api_h

######################################################################
#
#ctcs_api.h 's bottom part
#######################################################################
puts $fp_dst_apis_h "};"
puts $fp_dst_apis_h "typedef struct ctcs_api_s ctcs_api_t;"
puts $fp_dst_apis_h "extern uint8 g_ctcs_api_en;"
puts $fp_dst_apis_h "/**********************************************************"
puts $fp_dst_apis_h " * "
puts $fp_dst_apis_h " * Functions"
puts $fp_dst_apis_h " * "
puts $fp_dst_apis_h " **********************************************************/"
puts $fp_dst_apis_h "extern int32 ctcs_install_api(uint8 lchip);"
puts $fp_dst_apis_h "extern int32 ctcs_uninstall_api(uint8 lchip);"
puts $fp_dst_apis_h "extern char* ctcs_get_chip_name(uint8 lchip);"
puts $fp_dst_apis_h "extern uint8 ctcs_get_chip_type(uint8 lchip);"
for {set i 0} {$i < $api_cnt} {incr i} {
    regsub  {lchip_id} $ctc_api($i) "lchip" ctc_api($i)
	puts $fp_dst_apis_h "extern int32 [string replace $ctc_api($i) 0 2 "ctcs"];"
}
puts $fp_dst_apis_h " "

puts $fp_dst_apis_h "#ifdef __cplusplus"
puts $fp_dst_apis_h "}"
puts $fp_dst_apis_h "#endif"

puts $fp_dst_apis_h "#endif"
close $fp_dst_apis_h
######################################################################
#
#ctc_api.c 's bottom part
#######################################################################
puts $fp_dst_api_c " "
puts $fp_dst_api_c " "
close $fp_dst_api_c

#ctcs_api.c 's bottom part
#######################################################################
puts $fp_dst_apis_c " "
puts $fp_dst_apis_c " "
close $fp_dst_apis_c

###################################################################
#
# $(chip_name)_api.c
####################################################################
foreach chipname $all_chip {
    puts $fp_dst_chip_api_c($chipname) "\};"
    puts $fp_dst_chip_api_c($chipname) "#endif"
    puts $fp_dst_chip_api_c($chipname) " "
    puts $fp_dst_chip_api_c($chipname) " "
    close $fp_dst_chip_api_c($chipname)
}

close $fp_src
