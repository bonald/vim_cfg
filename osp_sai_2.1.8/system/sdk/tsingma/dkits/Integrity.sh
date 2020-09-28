#!/bin/bash

chip=$1
cvt_file=$2
out_file=target_$cvt_file
tbl_file=./tbl.auto
fld_file=./fld.auto

if [ "$1" == "--help" ]
then
    echo " usage:"
    echo " sh Integrity.sh chip_name integrity_check_file_name"
    exit 0
fi

src_file=$(find ../driver/$chip/include/drv_ds.h)

if [ -z "$chip" ] || [ -z "$cvt_file" ] || [ ! -e "$cvt_file" ] || [ -z "$src_file ] || [ ! -e "$src_file ]
then
    echo "Please input: chip\(goldengate, duet2...\), tbl, field, integrity check file specifically."
    exit 0
fi

if [ -e $tbl_file ]
then
    rm $tbl_file
fi

if [ -e $fld_file ]
then
    rm $fld_file
fi

if [ -e $out_file ]
then
    rm $out_file
fi

fldid=0
tblid=0

while read line
do
	if [[ ! -z $line ]]
	then
		str=$(echo $line | sed 's/\([ ]*\)\(\w*\)\(.*\)/\2/g')

        if [ "$str" == "DRV_DEF_D" ]
        then
            tbl=$str
            fldid=0
            tbl=$(echo $line | sed 's/\([ ]*DRV_DEF_D(\w*,[ ]*\w*,[ ]*\)\(\w*\)\(.*\)/\2/g')
            echo "    $tbl"_t" = $tblid""," >> $tbl_file
            let tblid=tblid+1
	    elif [ "$str" == "DRV_DEF_F" ]
	    then
            fld=$(echo $line | sed 's/\([ ]*DRV_DEF_F(\w*,[ ]*\w*,[ ]*\w*,[ ]*\)\(\w*\)\(.*\)/\2/g')
            echo "    $tbl"_"$fld""_f = $fldid""," >> $fld_file
            let fldid=fldid+1
		fi
	fi
done<$src_file

prefix="#@"

while read line
do
    if [[ ! -z $line ]] && [ "${line:0:1}" == "[" ]
    then
        key1=$(echo $line | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*'='\s*\)\([0-9]*\)/\2 \4 \6 \8/g')
        tbl_name1=$(echo $key1 | awk '{ print $1 }')
        tbl_idx1=$(echo $key1 | awk '{ print $2 }')
        field_name1=$(echo $key1 | awk '{ print $3 }')
        field_val1=$(echo $key1 | awk '{ print $4 }')
        equal=$(echo $line | sed 's/\(.*\']'\)\(\s*\)\('='\)\(\s*\)\(\'['.*\)/\3/g')
        operand=$(echo $line | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*\)\('\{'\s*'op'\s*'+'\s*'\}'\)\(.*\)/\8/g')

        if [ "${tbl_name1:0:1}" != "[" ] && [ "${equal:0:1}" != "=" ] && [ "$operand" != "{op +}" ]
        then
            str1=$(grep -w "$tbl_name1"_t $tbl_file)
            tblid1=$(echo $str1 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
            str2=$(grep -w "$tbl_name1"_"$field_name1"_f $fld_file)
            fldid1=$(echo $str2 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
            echo $prefix$line >> $out_file
            echo "["$tblid1"."$tbl_idx1"."$fldid1"] = "$field_val1 >> $out_file
            echo "" >> $out_file
        elif  [ "${equal:0:1}" == "=" ] &&  [ "$operand" != "{op +}" ]
        then
            left_str=$(echo $line | sed 's/\(.*\']'\)\(\s*\)\('='\)\(\s*\)\(\'['.*\)/\1/g')
            key2=$(echo $left_str | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*\)/\2 \4 \6/g')
            tbl_name2=$(echo $key2 | awk '{ print $1 }')
            tbl_idx2=$(echo $key2 | awk '{ print $2 }')
            field_name2=$(echo $key2 | awk '{ print $3 }')

            str3=$(grep -w "$tbl_name2"_t $tbl_file)
            tblid2=$(echo $str3 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
            str4=$(grep -w "$tbl_name2"_"$field_name2"_f $fld_file)
            fldid2=$(echo $str4 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')

            right_str=$(echo $line | sed 's/\(.*\']'\)\(\s*\)\('='\)\(\s*\)\(\'['.*\)/\5/g')
            key3=$(echo $right_str | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*\)/\2 \4 \6/g')
            tbl_name3=$(echo $key3 | awk '{ print $1 }')
            tbl_idx3=$(echo $key3 | awk '{ print $2 }')
            field_name3=$(echo $key3 | awk '{ print $3 }')
            str5=$(grep -w "$tbl_name3"_t $tbl_file)
            tblid3=$(echo $str5 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
            str6=$(grep -w "$tbl_name3"_"$field_name3"_f $fld_file)
            fldid3=$(echo $str6 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
            echo $prefix$line >> $out_file
            echo "["$tblid2"."$tbl_idx2"."$fldid2"] = ["$tblid3"."$tbl_idx3"."$fldid3"]" >> $out_file
            echo "" >> $out_file
        elif [ "$operand" = "{op +}" ]
        then
            multi_operand_str=""
            remain=$line
            while [ "$operand" == "{op +}" ]
            do
                key4=$(echo $remain | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*\)\('\{'\s*'op'\s*'+'\s*'\}'\)\(.*\)/\2 \4 \6/g')
                tbl_name4=$(echo $key4 | awk '{ print $1 }')
                tbl_idx4=$(echo $key4 | awk '{ print $2 }')
                field_name4=$(echo $key4 | awk '{ print $3 }')
                str7=$(grep -w "$tbl_name4"_t $tbl_file)
                tblid4=$(echo $str7 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
                str8=$(grep -w "$tbl_name4"_"$field_name4"_f $fld_file)
                fldid4=$(echo $str8 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
                multi_operand_str=$multi_operand_str"["$tblid4"."$tbl_idx4"."$fldid4"]"" {op +} "
                remain=$(echo $remain | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*\)\('\{'\s*'op'\s*'+'\s*'\}'\)\(.*\)/\9/g')
                operand=$(echo $remain | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*\)\('\{'\s*'op'\s*'+'\s*'\}'\)\(.*\)/\8/g')
            done

            key5=$(echo $remain | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*'='\s*\)\([0-9]*\)/\2 \4 \6/g')
            tbl_name5=$(echo $key5 | awk '{ print $1 }')
            tbl_idx5=$(echo $key5 | awk '{ print $2 }')
            field_name5=$(echo $key5 | awk '{ print $3 }')

            str9=$(grep -w "$tbl_name5"_t $tbl_file)
            tblid5=$(echo $str9 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
            str10=$(grep -w "$tbl_name5"_"$field_name5"_f $fld_file)
            fldid5=$(echo $str10 | sed 's/\(\s*\w*\s*\)\(\s*'='\s*\)\([0-9]*\)\(.*\)/\3/g')
            total_val=$(echo $remain | sed 's/\(\s*\'['\s*\)\([A-Za-z0-9_]*\)\(\s*'.'\s*\)\([0-9]*\)\(\s*'.'\s*\)\([A-Za-z0-9_]*\)\(\s*\']'\s*'='\s*\)\([0-9]*\)/\8/g')
            multi_operand_str=$multi_operand_str"["$tblid5"."$tbl_idx5"."$fldid5"]"" = "$total_val
            echo $prefix$line >> $out_file
            echo $multi_operand_str >> $out_file
            echo "" >> $out_file
        else
            echo $line >> $out_file
        fi

    elif [[ ! -z $line ]] && [ "${line:0:1}" == "#" ]
    then
        echo $line >> $out_file
    elif [[ ! -z $line ]]
    then
        echo "Fail to parser line:\""$line"\""
    fi
done<$cvt_file
