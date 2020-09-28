#!/bin/sh
back_dir=/tmp/back_tmp
partition=$1
log_dev=/dev/null
system_dir=/mnt/flash
boot_dir=/mnt/flash/boot
copy_files_to_tmp()
{
	`check_partition_is_mounted $partition`
	if [ "$?" = "0" ];then
		echo "ubi filesystem partition $1 is not mounted" > $log_dev
		return 0
	fi	
		
	if [ "$#" -ne "1" ];then
		return 0;
	fi
	if [ -f "$1" ]; then
		cp -rf $1 $back_dir
		if [ "$?" -ne "0" ]; then
			echo "cache file $1 to $back_dir failed" > $log_dev
			`logger -p 4 -t format "cache file $1 to $back_dir failed"`
			return 1
		fi
	fi
	if [ -d "$1" ]; then
		cp -rf $1 $back_dir
		if [ "$?" -ne "0" ]; then
			echo "cache dir $1 to $back_dir failed" > $log_dev
			`logger -p 4 -t format "cache dir $1 to $back_dir failed"`
			return 1
		fi
	fi
}

resume_files()
{
	`check_partition_is_mounted $partition`
	if [ "$?" = "0" ];then
		echo "ubi filesystem partition $1 is not mounted" > $log_dev
		return 0
	fi	
	
	if [ "$partition" = "boot" ]; then
		dst_dir=$boot_dir
	elif [ "$partition" = "system" ];then
		dst_dir=$system_dir
	fi
	
	echo "resume files from $back_dir to $dst_dir" > $log_dev
	`logger -p 4 -t format "resume files from $back_dir to $dst_dir"`
	
	if [ ! -d $back_dir ]; then
		echo "cache files is empty!" > $log_dev
		return 1
	fi
	count=`ls -A $back_dir`
	if [ "$count" = "" ];then
		echo "cache files is empty!" > $log_dev
		return 0
	fi	
	cp $back_dir/* $dst_dir/ -rfd
	if [ "$?" = "0" ]; then
		sync
		rm -rf $back_dir
		return 0
	else
		echo "resume files fail!" > $log_dev
		`logger -p 4 -t format "resume files fail"`
		return 1
	fi
}

format_partition()
{
	board_series=`cat /tmp/ctcos_board_info | grep product | awk '{print $2}'`
	board_type=`cat /tmp/ctcos_board_info | grep board_type | awk '{print $2}'`
	board="unsupport"
	#e350
	if [ "$board_series" = "1" ]; then
		board="e350"
	fi
	#GB demo
	if [ "$board_series" = "c" ]; then
		board="e350"
	fi
	#e580
	if [ "$board_series" = "10" ]; then
		board="e580"
	fi
	#P580
	if [ "$board_series" = "11" ]; then
		board="e580"
	fi
	if [ "$board" = "e580" ];then
		if [ "$partition" = "boot" ]; then
			mtd=`cat /proc/mtd | grep "Boot Area\|NAND (RW) Backup\|NAND (RW) Boot" | awk '{printf $1}'`
			mtd_id=`expr substr $mtd 4 1`
			ubi_id=1
			total_size=`cat /proc/mtd | grep "Boot Area\|NAND (RW) Backup\|NAND (RW) Boot" | awk '{printf $2}'`
			if [ "$total_size" = "38000000" ]; then
				ubi_size=832MiB
			elif [ "$total_size" = "08000000" ]; then
				ubi_size=70MiB
			else
				return 1
			fi
		elif [ "$partition" = "system" ]; then
			mtd=`cat /proc/mtd | grep "System Area\|NAND (RW) System" | awk '{printf $1}'`
			mtd_id=`expr substr $mtd 4 1`
			ubi_id=0
			total_size=`cat /proc/mtd | grep "System Area\|NAND (RW) System" | awk '{printf $2}'`
			if [ "$total_size" = "10000000" ]; then
				ubi_size=200MiB
			else
				ubi_size=950MiB
			fi
		else
			return 1
		fi
	elif [ "$board" = "e350" ]; then
		if [ "$partition" = "boot" ]; then
			mtd=`cat /proc/mtd | grep "Boot Area\|NAND (RW) Backup\|NAND (RW) Boot" | awk '{printf $1}'`
			mtd_id=`expr substr $mtd 4 1`
			ubi_id=1
			ubi_size=70MiB
		elif [ "$partition" = "system" ]; then
			mtd=`cat /proc/mtd | grep "System Area\|NAND (RW) System" | awk '{printf $1}'`
			mtd_id=`expr substr $mtd 4 1`
			ubi_id=0
			ubi_size=1800MiB
		else
			return 1
		fi	
	else
		echo "this board type series do not support format flash online" > $log_dev
		`logger -p 4 -t format "this board type series do not support format flash online"`
		return 1
	fi
	ubidetach -p /dev/mtd$mtd_id > /dev/null
	ubiformat  /dev/mtd$mtd_id -y > /dev/null
	ubiattach /dev/ubi_ctrl -m $mtd_id > /dev/null
	ubimkvol  /dev/ubi$ubi_id -N $partition -s $ubi_size > /dev/null
	
	return 0
}
check_partition_is_mounted()
{
	if [ "$1" = "system" ]; then
		mounted=`mount | grep "flash" | grep "system"`
	elif [ "$1" = "boot" ]; then
		mounted=`mount | grep "flash" | grep "boot"`
	fi
	if [ -n "$mounted" ];then
		return 1
	else
		return 0
	fi
}
mount_ubifs()
{
	if [ "$1" = "boot" ]; then
		ubi_id=1
		dir=$boot_dir
		if [ ! -d $boot_dir ];then
			mkdir $boot_dir
		fi
	elif [ "$1" = "system" ]; then
		ubi_id=0
		dir=$system_dir
	fi
    
	if ! mount -t ubifs ubi$ubi_id:$1 $dir > /dev/null; then
		return 1
	else
		return 0
	fi
}
umount_partition()
{
	`check_partition_is_mounted $1`
	if [ "$?" = "0" ];then
		return 0
	fi	
	if [ "$1" = "boot" ]; then
		dir=$boot_dir
	elif [ "$1" = "system" ]; then
		dir=$system_dir
	fi	
	umount $dir
	if [ "$?" != "0" ]; then
		return 1
	else
		return 0
	fi
}
remount_ubifs_ro()
{
	if [ "$1" = "boot" ]; then
		ubi_id=1
		dir=$boot_dir
	elif [ "$1" = "system" ]; then
		ubi_id=0
		dir=$system_dir
	fi
    
	if ! mount -t ubifs -o remount -r ubi$ubi_id:$1 $dir > /dev/null; then
		return 1
	else
		return 0
	fi
}

check_partition_busy()
{
	if [ "$1" = "boot" ]; then
		dir=$boot_dir
	elif [ "$1" = "system" ]; then
		dir=$system_dir
	fi
	if [ ! -d "$dir" ];then
		return 0
	fi
	flash_use=`fuser $dir -m`

	i=1
	is_occupy=n
	pid=`echo $flash_use|awk '{print $1}'`
	while [ -n "$pid" ]
	do			
			thread=`ps aux | awk '{printf("%s(%s)\n"),$2,$7}'|grep $pid`
			echo "the process $thread still use the partition $partition" > $log_dev
			is_occupy=y
			i=$(( $i + 1 ))
			pid=`echo $flash_use|awk '{print $i}' i=$i`
	done

	if [ "$is_occupy" = "y" ];then
		return 1
	fi
	return 0
}
is_console=`cdbctl show/cdb/app/login | grep "*" | grep con | awk -F " " '{print $4}'`
is_vty=`cdbctl show/cdb/app/login | grep "*" | grep vty | awk -F " " '{print $3}'`
if [ -n "$is_console" ];then
	log_dev=/dev/console
	echo "you are a console user! console NO is $is_console" > $log_dev
fi
if [ -n "$is_vty" ];then
	log_dev=/dev/pts/$is_vty
	echo "you are a vty user! vty NO is $is_vty" > $log_dev
fi

echo "prepare to format partition $partition..." > $log_dev
`logger -p 4 -t format "Prepare to format partition $partition..."`
if [ "$partition" = "boot" ]; then
	while true; do
		echo -n "% Are you sure to have backup the file in partition boot? [yes/no] "
		read sure
		if [ "$sure" != "yes" ] && [ "$sure" != "y" ] ; then
			if [ "$sure" == "no" ] || [ "$sure" == "n" ] ; then
				echo "% format partition $partition stop! Please backup your file in partition boot manually" > $log_dev
				`logger -p 4 -t format "% format partition $partition stop! Please backup your file in partition boot manually!"`
				exit 1
			else
				echo "% Please input [yes/no]"
				continue
			fi
		fi
		break
	done
fi

user_vty_num=`cdbctl show/cdb/app/login | grep vty | wc -l`
user_local=`cdbctl show/cdb/app/login | grep vty | grep "local\|127.0.0.1" |wc -l`
let user_num=user_vty_num-user_local
if [ $user_num -ge 2 ]; then
	echo "% there is someone else login on system by telnet/ssh, please logout it!" > $log_dev
	`logger -p 4 -t format "% format partition $partition stop! Please logout the other vty-users!"`
	exit 1
fi

`check_partition_busy "boot"`
if [ "$?" -eq "1" ]; then
	echo "% you can do 'cdmnt' for release occupy of partition" > $log_dev
	`logger -p 4 -t format "% format partition $partition stop! Please release the resource or device!"`
	exit 1
fi
if [ "$partition" = "system" ];then
	`check_partition_busy "system"`
	if [ "$?" -eq "1" ]; then
		echo "% you can do 'cdmnt' for release occupy of partition" > $log_dev
		`logger -p 4 -t format "% format partition $partition stop! Please release the resource or device!"`
		exit 1
	fi
fi

#back up files
mkdir $back_dir
if [ "$partition" = "system" ]; then
	echo "cache files to $back_dir" > $log_dev
	`logger -p 4 -t format "cache files to $back_dir"`
	# back up some important files for recover them after
	`copy_files_to_tmp "/mnt/flash/init_group"`
	`copy_files_to_tmp "/mnt/flash/init_meter"`
	`copy_files_to_tmp "/mnt/flash/init_flow"`
	`copy_files_to_tmp "/mnt/flash/startup-config.conf"`
	`copy_files_to_tmp "/mnt/flash/post_init_hook.sh"`
	
	`copy_files_to_tmp "/mnt/flash/reboot-info"`
	`copy_files_to_tmp "/mnt/flash/syslog"`
	echo "cache files finished" > $log_dev
	`logger -p 4 -t format "cache files finished"`
elif [ "$partition" = "boot" ]; then
	echo "cache files to $back_dir" > $log_dev
	`logger -p 4 -t format "cache files to $back_dir"`
	`copy_files_to_tmp "/mnt/flash/boot/.factory-config.conf"`
	echo "cache files finished" > $log_dev
	`logger -p 4 -t format "cache files finished"`	
fi

#format flash
`umount_partition "boot"`
if [ "$?" -eq "1" ]; then
	echo "% umount partition boot failed " > $log_dev
	`logger -p 4 -t format "% format partition $partition stop! umount partition boot failed!"`
	if [ -d $back_dir ]; then
		rm -rf $back_dir
	fi
	exit 1
fi
if [ "$partition" = "system" ]; then
	`umount_partition "system"`
	if [ "$?" -eq "1" ]; then
		echo "% umount partition system failed " > $log_dev
		`logger -p 4 -t format "% format partition $partition stop! umount partition system failed!"`
		`mount_ubifs "boot"`
		`remount_ubifs_ro "boot"`
		if [ -d $back_dir ]; then
			rm -rf $back_dir
		fi
		exit 1
	fi
fi
`format_partition`
if [ "$?" -eq "1" ]; then
	`logger -p 4 -t format "% format partition $partition stop!"`
	if [ "$partition" = "system" ]; then
		`mount_ubifs "system"`
	fi
	`mount_ubifs "boot"`
	`remount_ubifs_ro "boot"`
	if [ -d $back_dir ]; then
		rm -rf $back_dir
	fi
	echo "% format partition $partition failed " > $log_dev
	exit 1
fi

if [ "$partition" = "system" ]; then
	`mount_ubifs "system"`
fi
`mount_ubifs "boot"`

if [ "$?" -eq "1" ]; then
	echo "% mount partition $partition failed " > $log_dev
	`logger -p 4 -t format "% format partition $partition stop! mount partition $partition failed!"`
	exit 1
fi

`resume_files`

`remount_ubifs_ro "boot"`
if [ "$?" -eq "1" ]; then
	echo "% remount partition boot read-only failed " > $log_dev
	`logger -p 4 -t format "% format partition $partition finished! But remount partition boot read-only failed!"`
	exit 1
fi
echo "format partition $partition finished!" > $log_dev
`logger -p 4 -t format "format partition $partition finished!"`
