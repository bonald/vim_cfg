#!/usr/bin/tclsh

set chipname [lindex $argv 0]
set feature_mode [lindex $argv 1]

set dest_file "ctc_$chipname\_api.txt"
set file_temp "file_temp.txt"
set fp_dest [open $dest_file w+]
set module_file_temp [open $file_temp w+]
set filelist [glob ../../core/$chipname/include/ctc/*.h]
set ::module_list [list start]
set match1 0
set result_temp 0
foreach includefile $filelist {
	set number1 9
	set number2 9
	set sourceFileId [open $includefile r]

#	puts "/*auto generated from $includefile*/"
	set module [file tail $includefile]
	regsub "ctc_\[a-z0-9\]\+_" $module "" module
	regsub "\\.h" $module "" module

	puts $fp_dest "##$module##"

	while {[gets $sourceFileId line] >= 0 } {
    	set line [string trim $line]
		
		#set result1 [regexp {.*(FEATURE_MODE) *\) *== *([0-9]) *|| *\((FEATURE_MODE) *== *(0-9).*} $line match match1 match2 match3 match4]
		set result_temp1 [regexp {.*FEATURE_MODE.*} $line match1]
		if { $result_temp1 == 1 } {
			set result_temp2 [regexp {2} $match1 number1]
			set result_temp3 [regexp {0} $match1 number2]			
			if { $result_temp3 == 1 && $result_temp2 != 1 && "l2l3" == $feature_mode} { # 当配置的参数为2的时候，删除#if (FEATURE_MODE == 0)
				puts $module_file_temp $module
				break
			} elseif { $result_temp3 == 1 && "l2" == $feature_mode } { #当配置的参数为1的时候，删除#if ((FEATURE_MODE == 2) || (FEATURE_MODE == 0))和#if (FEATURE_MODE == 0)
				puts $module_file_temp $module
				break
			}
		}
        #set is_function [regexp {^extern +.*} $line]
        if [regexp {^extern +.*} $line] {
            if [regexp {\{} $line] {
                set is_function 0
            } else {
                set is_function 1
            }
        } else {
            set is_function 0
        }

    	if { $is_function == 1 } {
			set complete [regexp {.*\).*;.*} $line]
			if { $complete == 0} {
        		while {[gets $sourceFileId next_line] >=0 } {
                    set next_line [string trim $next_line]
            		set complete [regexp {.*\).*} $next_line]
            		append line " "
					append line $next_line
            		if { $complete == 1 } {
                		break
        			}
        		}
			}

			set line [string trim $line]
			regsub -all {\s*\;} $line "" function
            regsub -all {\s+\,} $function "," function
			set function [string replace $function 0 12]
            regsub "_$chipname" $function "" API_function

			append API_function ";"
			puts $fp_dest $API_function
		}
    }
	close $sourceFileId
}

puts $fp_dest "##end of file##"
close $fp_dest
return $module_list
