BEGIN{
	module_bak = 0
}
{
	ignore = 0
}
/^struct +sys_.*_/{
	ignore = 1
    name=$0
    sub(/_s$/,"_t",name)
    sub(/struct/,"",name)
    sub(/ */,"",name)
    module=$0
    gsub(/ */,"",module)
    sub(/struct/,"",module)
    sub(/sys_wb_/,"",module)
    sub(/_.*$/,"",module)
	if (module_bak != module)
	{
		#print "@@@"module
		module_out = module
	}
	#print "###"name
	module_bak = module
}

/The file defines queue api/{
	ignore = 1
}
/typedef (struct|enum|union) sys_.*_/{
    ignore = 1
}
/^(struct|enum|union) sys_.*_/{
    ignore = 1
}
/^typedef void/{
    ignore = 1
}
/^.*{/{
    ignore=1
}
/#define/{
    ignore=1
}
/[ \t]*[a-z0-9]+ +rsv.*/{
	ignore=1
}
/^.*}/{
	ignore=1
}
/^[\t ]*\/\*.*\*\/[\t ]*$/{
	ignore=1
}
/^#include.*$/{
	ignore=1
}
/^#ifndef.*$/{
	ignore=1
}
/^#ifdef.*$/{
	ignore=1
}
/^#endif.*$/{
	ignore = 1
}

/^extern.*$/{
	ignore = 1
}
#/^[\t ]*[a-z_0-9]+[\t ]*\/\*.*\*\/[\t ]*$/&& !/[A-Z][A-Z_0-9].*\/\*.*/{
#	ignore=1
#}
#
#/\s\+\/\*/ && !/[A-Z][A-Z_0-9].*\/\*.*/{
#	ignore=1
#}

/^[\t ]+[A-Z_]+/{
	ignorn = 1
}

/calc_key_len/{
	ignore = 0
}

/ *union */{
	union = 1
	loop = 0
}

union == 1{
	if($0~/}/)
	{
		var = $0
		sub(/^.*} */,"",var)
		sub(/ *; *$/,"",var)
		union_name = var
		ignore = 0
	}
}

ignore == 0{
    if($0~/^[ \t]*[a-z_0-9].*/ )
	{
		var = $0
		class = $1
		sub(/\/\*.*/,"",var)
		sub(/=.*/,"",var)
		sub(/;.*/,"",var)
		sub(/,/,"",var)
		sub(/ +[a-z0-9_]+/,"",var)
		sub(/ *$/,"",var)
		sub(/^ */,"",var)
		#gsub(/\[.*\]/,"",var)
		sub(/:.*$/,"",var)
		if (union == 1)
		{
			var_temp[loop] = var
			class_temp[loop] = class
			loop++
		}
		else
		{
			print module_out":"name":"class"%"var
		}
	}else if (union == 1)
	{
		union = 0
		var = union_name
		for (loop1 = 0; loop1 < loop; loop1++)
		{
			print module_out":"name":"class_temp[loop1]"%"var"."var_temp[loop1]
		}
		loop = 0
	}
}
#ignore==0{
#   if (type == "struct")
#   {
#       if($0~/^[ \t]*[a-z_0-9].*\/\*\*< *.*/ )
#	    {
#		    var=$0
#		    sub(/\/\*.*/,"",var)
#		    sub(/=.*/,"",var)
#		    sub(/;.*/,"",var)
#		    sub(/,/,"",var)
#		    sub(/ +[a-z0-9_]+/,"",var)
#		    gsub(/ */,"",var)
#		    gsub(/\[.*\]/,"",var)
#
#		    if($0~/\/\*\*< *\[.*\]/)
#		    {
#                chipset=$0
#                sub(/.*\/\*+/,"",chipset)
#                sub(/\*\/.*/,"",chipset)
#                sub(/\].*/,"",chipset)
#                sub(/.*\[/,"",chipset)
#		    }else
#		    {
#			    chipset=""
#		    }
#           if($0~/\/\*+<.*/)
#		    {
#		         text=text""module":"type":"name":"var":"chipset"\n"
#		    }else 
#	        {
#	             text =text"\n"
#	        }
#	    }#else if($0~/^\s*[a-z_0-9= ]+\s*,\s*$/ || $0~/^[ \t]*[a-z_0-9]+ +[a-z_0-9]+\s*;\s*$/ || $0~/^[\t ]*[A-Z_0-9]+\s*,*\s*$/)
#	   {
#		    var=$0
#		    sub(/\/\*.*/,"",var)
#		    sub(/=.*/,"",var)
#		    sub(/;.*/,"",var)
#            sub(/,/,"",var)
#		    commet=""
#		    chipset=""
#            text=text""module":"type":"name":"var":"chipset"\n"
#	    }
#        else
#	    {
#	        text=text"\n"
#	    }
#    }else if (type == "enum")
#    {
#        if ($0~/^[ \t]*[A-Z_0-9].*\/\*\*< *.*/)
#        {
#            var=$0
#            sub(/\/\*.*/,"",var)
#            sub(/=.*/,"",var)
#            sub(/;.*/,"",var)
#            sub(/,/,"",var)
#            gsub(/ */,"",var)
#            gsub(/\[.*\]/,"",var)
#
#            if ($0~/\/\*\*< *\[.*\]/)
#            {
#                chipset=$0
#                sub(/.*\/\*+/,"",chipset)
#                sub(/\*\/.*/,"",chipset)
#                sub(/\].*/,"",chipset)
#                sub(/.*\[/,"",chipset)
#            }else
#            {
#                chipset=""
#            }
#            if ($0~/\/\*+<.*/)
#            {
#                text=text""module":"type":"name":"var":"chipset"\n"
#            }else
#            {
#                text =text"\n"
#            }
#        }
#    }else
#    {
#        text =text"\n"
#    }
#}
END{
   # print name
}
