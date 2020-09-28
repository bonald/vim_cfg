#!/bin/bash

postfix="_t"
auto_tbl_file="$HOME/sdklink/sdk/dkits/goldengate/sdk_static_tbl.txt"
max_tbl_id="MaxTblId_t"

rm ./*.log
rm ./*.rst
rm $auto_tbl_file

grep -rn "Set.*(.*A," $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c > ./temp_tbl_grep1.log
grep -rn "Set.*(.*V," $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c > ./temp_tbl_grep2.log
grep -rnw "DRV_SET_FIELD_V" $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c > ./temp_tbl_grep3_1.log
grep -rnw "DRV_SET_FIELD_A" $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c > ./temp_tbl_grep3_2.log
grep -rnw "DRV_IOW_FIELD" $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c > ./temp_tbl_grep3_3.log
grep -rnw "DRV_IOW" $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c > ./temp_tbl_grep4.log

if [ -e ./temp_tbl_grep1.log ] && [ -s ./temp_tbl_grep1.log ]
then
	echo "Process grep SetTblName(A, ...) result."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl=$(echo $line | sed 's/\(\S*\)\(\s*\)\('Set'\)\(\w*\)\('\('\)\(.*\)/\4/g')

			if [ "${tbl:0:1}" \> "@" ] && [ "${tbl:0:1}" \< "[" ]
			then
				echo $tbl$postfix >> ./auto_extract.log
			fi
		fi
	done<./temp_tbl_grep1.log
else
    echo "grep SetTblName(A, ...) none result!"
fi

if [ -e ./temp_tbl_grep2.log ] && [ -s ./temp_tbl_grep2.log ]
then
	echo "Process grep SetTblName(V, ...) result."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl=$(echo $line | sed 's/\(\S*\)\(\s*\)\('Set'\)\(\w*\)\('\('\)\(.*\)/\4/g')

			if [ "${tbl:0:1}" \> "@" ] && [ "${tbl:0:1}" \< "[" ]
			then
				echo $tbl$postfix >> ./auto_extract.log
			fi
		fi
	done<./temp_tbl_grep2.log
else
    echo "grep SetTblName(V, ...) none result!"
fi

if [ -e ./temp_tbl_grep3_1.log ] && [ -s ./temp_tbl_grep3_1.log ]
then
	echo "Process grep DRV_SET_FIELD_V result."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl1=$(echo $line | sed 's/\(.*\)\('DRV_SET_FIELD_A'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\([^_]*\)\(.*\)/\4/g')
			tbl2=$(echo $line | sed 's/\(.*\)\('DRV_SET_FIELD_A'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\([^_]*\)\(.*\)/\6/g')

			if [ "${tbl1:0:1}" \> "@" ] && [ "${tbl1:0:1}" \< "[" ]
			then
				echo $tbl1 >> ./auto_extract.log
			else
				if [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
				then
					echo $tbl2$postfix >> ./auto_extract.log
				fi
			fi
		fi
	done<./temp_tbl_grep3_1.log
else
	echo "grep DRV_SET_FIELD_A none result!"
fi

if [ -e ./temp_tbl_grep3_2.log ] && [ -s ./temp_tbl_grep3_2.log ]
then
	echo "Process grep DRV_SET_FIELD_V result."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl1=$(echo $line | sed 's/\(.*\)\('DRV_SET_FIELD_V'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\([^_]*\)\(.*\)/\4/g')
			tbl2=$(echo $line | sed 's/\(.*\)\('DRV_SET_FIELD_V'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\([^_]*\)\(.*\)/\6/g')

			if [ "${tbl1:0:1}" \> "@" ] && [ "${tbl1:0:1}" \< "[" ]
			then
				echo $tbl1 >> ./auto_extract.log
			else
				if [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
				then
					echo $tbl2$postfix >> ./auto_extract.log
				fi
			fi
		fi
	done<./temp_tbl_grep3_2.log
else
	echo "grep DRV_SET_FIELD_V none result!"
fi

if [ -e ./temp_tbl_grep3_3.log ] && [ -s ./temp_tbl_grep3_3.log ]
then
	echo "Process grep DRV_IOW_FIELD result."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl1=$(echo $line | sed 's/\(.*\)\('DRV_IOW_FIELD'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\([^_]*\)\(.*\)/\4/g')
			tbl2=$(echo $line | sed 's/\(.*\)\('DRV_IOW_FIELD'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\([^_]*\)\(.*\)/\6/g')

			if [ "${tbl1:0:1}" \> "@" ] && [ "${tbl1:0:1}" \< "[" ]
			then
				echo $tbl1 >> ./auto_extract.log
			else
				if [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
				then
					echo $tbl2$postfix >> ./auto_extract.log
				fi
			fi
		fi
	done<./temp_tbl_grep3_3.log
else
	echo "grep DRV_IOW_FIELD none result!"
fi

if [ -e ./temp_tbl_grep4.log ] && [ -s ./temp_tbl_grep4.log ]
then
	echo "Process grep DRV_IOW result."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl1=$(echo $line | sed 's/\(.*\)\([ ]*'DRV_IOW'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\(.*\)/\4/g')
			tbl2=$(echo $line | sed 's/\(.*\)\([ ]*'DRV_IOW'[ ]*\)\('\('[ ]*\)\(\w*\)\([ ]*,[ ]*\)\([^_]*\)\(.*\)\('\)'.*\)/\6/g')
			tbl3=$(echo $line | sed 's/\(.*\)\([ ]*'DRV_IOW'[ ]*\)\(\W*[ ]*\)\(\w*\)\([ ]*'+'[ ]*\)\(\w*\W*\)\([ ]*,[ ]*\)\(.*\)/\4/g')

			if [ "${tbl1:0:1}" \> "@" ] && [ "${tbl1:0:1}" \< "[" ]
			then
				echo $tbl1 >> ./auto_extract.log
			else
				if [ "$tbl2" != "DRV" ] && [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
				then
					if [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
					then
						echo $tbl2$postfix >> ./auto_extract.log
					fi
				else
					if [ "${tbl3:0:1}" \> "@" ] && [ "${tbl3:0:1}" \< "[" ]
					then
						echo $tbl3 >> ./tbl_prefix.log
					else
						if [ "${tbl1:0:1}" == "/" ]
						then
							echo $tbl1 >> ./manual_process1.log
						else
							echo $tbl1 >> ./tbl_nickname.log
						fi
					fi
				fi
			fi
		fi
	done<./temp_tbl_grep4.log
else
	echo "grep DRV_IOW none result!"
fi

cat ./tbl_nickname.log | sort | uniq > tbl_nickname_su.log

if [ -e ./tbl_nickname_su.log ] && [ -s ./tbl_nickname_su.log ]
then
	echo "Process indirect tbl name."
	while read line1
	do
		if [[ ! -z $line1 ]]
		then
			grep -rnw "$line1[ ]*=[ ]*" $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c > ./grep_nickname.log
			while read line2
			do
				if [[ ! -z $line2 ]]
				then
					tbl1=$(echo $line2 | sed 's/\(.*\)\('$line1'\)\([ ]*=[ ]*\)\(\w*\)\('\;'\)\(.*\)/\4/g')
					tbl2=$(echo $line2 | sed 's/\(.*\)\('$line1'\)\([ ]*=[ ]*\)\(\w*\)\([ ]*'+'[ ]*\)\(.*\)/\4/g')
					if [ "${tbl1:0:1}" \> "@" ] && [ "${tbl1:0:1}" \< "[" ]
					then
						echo $tbl1 >> ./auto_extract.log
					else
						if [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
						then
							echo $tbl2 >> ./tbl_prefix.log
						else
							if [ "${tbl1:0:1}" == "/" ]
							then
								echo $tbl1 >> ./manual_process1.log
							fi
						fi
					fi
				fi
			done<./grep_nickname.log
		fi
	done<./tbl_nickname_su.log
else
	echo "None indirect tbl name."
fi

if [ -e ./manual_process1.log ] && [ -s ./manual_process1.log ]
then
	echo "Process condition ? TblName : TblName"
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl1=$(echo $line | sed 's/\(.*\)\('?'[ ]*\)\(\w*\)\([ ]*':'[ ]*\)\(\w*\)\([ ]*.*\)/\3/g')
			tbl2=$(echo $line | sed 's/\(.*\)\('?'[ ]*\)\(\w*\)\([ ]*':'[ ]*\)\(\w*\)\([ ]*.*\)/\5/g')

			if [ "${tbl1:0:1}" \> "@" ] && [ "${tbl1:0:1}" \< "[" ]
			then
				echo $tbl1 >> ./auto_extract.log
			fi

			if [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
			then
				echo $tbl2 >> ./auto_extract.log
			fi

			if [ "${tbl1:0:1}" == "/" ] && [ "${tbl2:0:1}" == "/" ]
			then
				echo $tbl1 >> ./manual_process2.log
			fi
		fi
	done<./manual_process1.log
else
	echo "condition ? TblName : TblName skipped"
fi

if [ -e ./manual_process2.log ] && [ -s ./manual_process2.log ]
then
	echo "Recognize DRV_IOW indirect tbl name."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl=$(echo $line | sed 's/\(.*\)\([ ]*'DRV_IOW'\)\('\('\)\(\w*-*>*\w*\)\(.*\)/\4/g')

			if [ "${tbl:0:1}" \> "\`" ] && [ "${tbl:0:1}" \< "{" ]
			then
				echo $tbl >> ./indirect_process1.log
			fi

			if [ "${tbl:0:1}" == "/" ]
			then
				echo $line >> ./manual_process3.log
			fi
		fi
	done<./manual_process2.log
else
    echo "Recognize DRV_IOW indirect tbl name skipped."
fi

cat ./indirect_process1.log | sort | uniq > ./indirect_process1_su.log

if [ -e ./indirect_process1_su.log ] && [ -s ./indirect_process1_su.log ]
then
	echo "grep DRV_IOW indirect tbl name's real tbl."
	while read line
	do
		if [[ ! -z $line ]]
		then
			grep -rn "$line.*=" $HOME/sdklink/sdk/core/goldengate --include=*.[ch] --exclude=sys_goldengate_register.c >> indirect_process2.log
		fi
	done<./indirect_process1_su.log
else
	echo "grep DRV_IOW indirect tbl name's real tbl skipped."
fi

if [ -e ./indirect_process2.log ] && [ -s ./indirect_process2.log ]
then
	echo "Process grep DRV_IOW indirect tbl name's real tbl name result."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl1=$(echo $line | sed 's/\(.*\)\([ ]*'='[ ]*\)\(\W*\)\(\w*_*t*\)\([ ]*,*;*[ ]*\)\(\w*_*t*\)\(.*\)/\4/g')
			tbl2=$(echo $line | sed 's/\(.*\)\([ ]*'='[ ]*\)\(\W*\)\(\w*_*t*\)\([ ]*,*;*[ ]*\)\(\w*_*t*\)\(.*\)/\6/g')

			if [ "${tbl1:0:1}" \> "@" ] && [ "${tbl1:0:1}" \< "[" ]
			then
				echo $tbl1 >> ./auto_extract.log
			fi

			if [ "${tbl2:0:1}" \> "@" ] && [ "${tbl2:0:1}" \< "[" ]
			then
				echo $tbl2 >> ./auto_extract.log
			fi
		fi
	done<./indirect_process2.log
else
	echo "Process grep DRV_IOW indirect tbl name's real tbl name result skipped."
fi

cat ./tbl_prefix.log | sort | uniq > ./tbl_prefix_su.log

if [ -e ./tbl_prefix_su.log ] && [ -s ./tbl_prefix_su.log ]
then
	echo "Recognize Xx0Yy1_t tbl from driver."
	while read line
	do
		if [[ ! -z $line ]]
		then
			word1=$(echo $line | sed 's/\([A-Za-z]*\)\([0-9]*\)\([A-Za-z]*\)\([0-9]*\)\(\w*\)/\1/g')
			word2=$(echo $line | sed 's/\([A-Za-z]*\)\([0-9]*\)\([A-Za-z]*\)\([0-9]*\)\(\w*\)/\3/g')
			if [ "$word1" != "" ] && [ "$word2" != "" ]
			then
				grep -rn "$word1[0-9]*$word2[0-9]*.*_t" $HOME/sdklink/sdk/driver/goldengate/include/drv_enum.h --include=*.[ch] --exclude=sys_goldengate_register.c >> prefix_expand.log
			else
				if [ "$word1" != "" ] && [ "$word2" == "" ]
				then
					grep -rn "$word1[0-9]*.*_t" $HOME/sdklink/sdk/driver/goldengate/include/drv_enum.h --include=*.[ch] --exclude=sys_goldengate_register.c >> prefix_expand.log
				fi
			fi
		fi
	done<./tbl_prefix_su.log
else
	echo "Recognize Xx0Yy1_t tbl from driver skipped."
fi

if [ -e ./prefix_expand.log ] && [ -s ./prefix_expand.log ]
then
	echo "Process grep Xx0Yy1_t tbl from driver."
	while read line
	do
		if [[ ! -z $line ]]
		then
			tbl=$(echo $line | sed 's/\(\S*\)\(\s*\)\(\w*_t\)\(\s\)\(.*\)/\3/g')
			if [ "${tbl:0:1}" \> "@" ] && [ "${tbl:0:1}" \< "[" ]
			then
				echo $tbl >> ./auto_extract.log
			fi
		fi
	done<./prefix_expand.log
else
	echo "Process grep Xx0Yy1_t tbl from driver skipped."
fi

cat ./auto_extract.log | sort | uniq > tbl.log
cat ./manual_process3.log | sort | uniq > manual_process.txt

echo "#include \"drv_lib.h\"" >> $auto_tbl_file
echo "#include \"ctc_cli.h\"" >> $auto_tbl_file
echo "#include \"ctc_dkit.h\"" >> $auto_tbl_file
echo "" >> $auto_tbl_file
echo "tbls_id_t auto_genernate_tbl[] =" >> $auto_tbl_file
echo "{" >> $auto_tbl_file

if [ -e tbl.log ]
then
	echo "Check auto generate tbl."
	while read line
	do
		if [[ ! -z $line ]]
		then
			str=$(grep -rnw "$line" $HOME/sdklink/sdk/driver/goldengate/include/drv_enum.h)
			if [ "$str" == "" ]
			then
				echo $line >> ./driver_mismatch_tbl.txt
			else
				dynamic=$(grep -rnw "$line" $HOME/sdklink/sdk/dkits/goldengate/ctc_goldengate_dkit_dump_dynamic_tbl.c)
				hashtbl=$(grep -rnw "$line" $HOME/sdklink/sdk/driver/goldengate/src/drv_hash_db.c)
				if [ "$dynamic" == "" ] && [ "$hashtbl" == "" ]
				then
					echo "    $line," >> $auto_tbl_file
				fi
			fi
		fi
	done<./tbl.log
else
	echo "Check auto generate tbl skipped."
fi

echo "    $max_tbl_id" >> $auto_tbl_file
echo "};" >> $auto_tbl_file

rm *.log
