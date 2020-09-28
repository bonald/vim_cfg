print() {
    echo "the module list is below:"
	echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "-----------  Basic L2  -----------"
	echo "1    port"                      
	echo "2    vlan"                      
	echo "3    scl"                       
	echo "4    fdb"                       
	echo "5    register"                  
	echo "6    chip"                      
	echo "7    ftm"                       
	echo "8    qos"                       
	echo "9    nexthop"                   
	echo "10   parser"                    
	echo "11   datapath"                  
	echo "12   linkagg"                   
	echo "13   mirror"                    
	echo "14   security"                  
	echo "15   pdu"                       
	echo "16   learning aging"            
	echo "17   acl"                       
	echo "18   stp"                       
	echo "19   dma"                       
	echo "20   interrupt"                 
	echo "21   packet"                    
	echo "22   internal port"             
	echo "23   l3if"                      
	echo "24   stats"                     
	echo "25   aps"                       
    echo "-----------  Basic L3  -----------"
	echo "26   ipuc"                      
	echo "27   ipmc" 
	echo "-----------   Others   -----------"
	echo "28   mpls"
	echo "29   overlay"
	echo "30   dot1ae"
	echo "31   wlan"
	echo "32   bpe"
	echo "33   oam"
	echo "34   npm"
	echo "35   monitor"
	echo "36   efd"
	echo "37   ipfix"
	echo "38   fcoe"
	echo "39   trill"
	echo "40   stacking"
	echo "41   ptp"
	echo "42   sync"
	echo "-----------   Chip   -------------"	
	echo "43   goldengate"
	echo "44   greatbelt"
	echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "please input the number,such as:1 2 3 or 1-2 3 or 1-3"
} 
process() {
	SS=""
	for str in $index
	do
		FALSE=`expr index $str "-"`
		if [ $FALSE != "0" ]
			then
				let "len=$FALSE-1"
				x1=${str:0:$len}
				x2=${str:$FALSE}
				var=$x1
				while [ $var -le "$x2" ]
				do
					SS="$SS $var"
					((var++))
				done
		else
				SS="$SS $str"
		fi
	done
	SS=${SS:1}
}
cd ..
Module=( zero port vlan scl fdb register chip ftm qos nexthop parser datapath linkagg mirror security pdu learning_aging acl stp dma interrupt packet internal_port l3if stats aps ipuc ipmc mpls overlay dot1ae wlan bpe oam npm monitor efd ipfix fcoe trill stacking ptp sync goldengate greatbelt)
Module_sum=${#Module[@]}

print

if [ "$2" ] && [ "$2" = "-a" ] ;then #All?
	index="1-$Module_sum"
else
	read index
fi

if [ -z "$index" ]
then
	exit
fi

process

echo "your choose is: $SS"

for name in $SS
do
    module_name=${Module[name]}"*"
    if [ $module_name = "port*" ]
    then
        rm `echo "*usw*$module_name" | xargs find . -name | sed -n -e '/svn/d' -e '/bak/d' -e '/nexthop/d' -e '/dkits/d' -e '/internal/d' -e '/build/d;p'`
    elif [ $module_name = "nexthop*" ]
    then
        rm `echo "*usw*$module_name" | xargs find . -name | sed -n -e '/svn/d' -e '/bak/d' -e '/dkits/d' -e '/build/d;p'`
    elif [ $module_name = "goldengate*" ]
    then
        rm `echo "*goldengate*" | xargs find . -name | sed -n -e '/bak/d' -e '/svn/d' -e '/build/d;p'`
    elif [ $module_name = "greatbelt*" ]
    then
        rm `echo "*greatbelt*" | xargs find . -name | sed -n -e '/bak/d' -e '/svn/d' -e '/build/d;p'`
	elif [ $module_name = "ipuc*" ]
	then
		rm `echo "*sys_duet2_calpm*" | xargs find . -name | sed -n  -e '/nexthop/d' -e '/bak/d' -e '/svn/d' -e '/build/d;p'`	
		rm `echo "*sys_duet2_ipuc_tcam*" | xargs find . -name | sed -n  -e '/nexthop/d' -e '/bak/d' -e '/svn/d' -e '/build/d;p'`
		rm `echo "*usw*ip_tunnel*" | xargs find . -name | sed -n  -e '/nexthop/d' -e '/bak/d' -e '/svn/d' -e '/build/d;p'`
		rm `echo "*usw*rpf*" | xargs find . -name | sed -n  -e '/nexthop/d' -e '/bak/d' -e '/svn/d' -e '/build/d;p'`
		rm `echo "*usw*$module_name" | xargs find . -name | sed -n  -e '/nexthop/d' -e '/bak/d' -e '/svn/d' -e '/build/d;p'`
    else
        rm `echo "*usw*$module_name" | xargs find . -name | sed -n -e '/svn/d' -e '/bak/d' -e '/nexthop/d' -e '/dkits/d' -e '/build/d;p'`
    fi
done

