import re
file = open('ctc_app_warmboot_test_nh.c', 'w')
file_h = open('ctc_app_warmboot_test_nh.h', 'w')
file_ele = open('warmboot_test_nh.txt','r')
file_common_h = open('../../core/usw/include/sys/sys_usw_wb_nh.h','r')
file_line = file_ele.readlines()
file_line_common_h = file_common_h.readlines()
#.c
file.write('#if 0\n')
file.write('#include "../../app/ctc_app.h"\n')
file.write('#include "ctc_warmboot.h"\n')
file.write('#include "sys_usw_wb_nh.h"\n')
file.write('#include "../../app/usr/ctc_app_warmboot.h"\n')
file.write('#ifdef SDK_IN_USERMODE\n')
file.write('#include "sqlite3.h"\n')
file.write('sqlite3 *pFile = NULL;\n')
file.write('char *warmboot_db_path_test1 = "warmboot_test_1.db";\n')
file.write('char *warmboot_db_path_test2 = "warmboot_test_2.db";\n')
file.write('char *mode = NULL;\n')
file.write('\n')
file.write('int32 ctc_app_wb_test_load(uint8 load_flag, uint8 choice)\n')
file.write('{\n')
file.write('    int32 ret = 0;\n')
file.write('    char *file_path = NULL;\n')
file.write('    sqlite3 *pFile_tmp;\n')
file.write('    sqlite3 *pTo;\n')
file.write('    sqlite3 *pFrom;\n')
file.write('    sqlite3_backup *pBackup;\n')
file.write('\n')
file.write('    if (pFile == NULL)\n')
file.write('    {\n')
file.write('        return -1;\n')
file.write('    }\n')
file.write('\n')
file.write('    if (1 == choice)\n')
file.write('    {\n')
file.write('        file_path = warmboot_db_path_test1;\n')
file.write('    }\n')
file.write('    else if (2 == choice)\n')
file.write('    {\n')
file.write('        file_path = warmboot_db_path_test2;\n')
file.write('    }\n')
file.write('\n')
file.write('    ret = sqlite3_open(file_path, &pFile_tmp);\n')
file.write('    if ( ret != SQLITE_OK )\n')
file.write('    {\n')
file.write('        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Can\'t open database: %s, ret: %d\\n", sqlite3_errmsg(pFile), ret);\n')
file.write('        sqlite3_close(pFile_tmp);\n')
file.write('\n')
file.write('        return (ret > 0) ? -ret : ret;\n')
file.write('    }\n')
file.write('\n')
file.write('    if (load_flag == LOAD_FILE_TO_MEMORY)\n')
file.write('    {\n')
file.write('        pFrom = pFile_tmp;\n')
file.write('        pTo = pFile;\n')
file.write('    }\n')
file.write('    else if (load_flag == LOAD_MEMORY_TO_FILE)\n')
file.write('    {\n')
file.write('        pFrom = pFile;\n')
file.write('        pTo = pFile_tmp;\n')
file.write('    }\n')
file.write('    else\n')
file.write('    {\n')
file.write('        //CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "database close in ctc_app_wb_load 0\\n");\n')
file.write('        sqlite3_close(pFile_tmp);\n')
file.write('\n')
file.write('        return CTC_E_INVALID_PARAM;\n')
file.write('    }\n')
file.write('\n')
file.write('    pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");\n')
file.write('    if( pBackup ){\n')
file.write('        sqlite3_backup_step(pBackup, -1);\n')
file.write('        sqlite3_backup_finish(pBackup);\n')
file.write('    }\n')
file.write('    ret = sqlite3_errcode(pTo);\n')
file.write('\n')
file.write('    //CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "database close file db in ctc_app_wb_load 1\\n");\n')
file.write('    sqlite3_close(pFile_tmp);\n')
file.write('\n')
file.write('    return (ret > 0) ? -ret : ret;\n')
file.write('}\n')
file.write('\n')
file.write('int32 ctc_app_wb_test_init(uint8 choice)\n')
file.write('{\n')
file.write('    int32 ret = 0;\n')
file.write('    ret = sqlite3_open(":memory:", &pFile);\n')
file.write('    \n')
file.write('    if ( ret != SQLITE_OK )\n')
file.write('    {\n')
file.write('        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Can not open database: %s\\n", sqlite3_errmsg(pFile));\n')
file.write('        sqlite3_close(pFile);\n')
file.write('        return (ret > 0) ? -ret : ret;\n')
file.write('    }\n')
file.write('    return ret;\n')
file.write('}\n')
file.write('int32 ctc_app_wb_test_init_done(uint8 choice)\n')
file.write('{\n')
file.write('    if (pFile)\n')
file.write('    {\n')
file.write('        ctc_app_wb_test_load(LOAD_MEMORY_TO_FILE, choice);\n')
file.write('        \n')
file.write('        sqlite3_close(pFile);\n')
file.write('        pFile = NULL;\n')
file.write('    }\n')
file.write('    return 0;\n')
file.write('}\n')
#file.write('int32 ctc_app_wb_del_table(uint8 choice)\n')
#file.write('{\n')
#file.write('    int32 ret = 0;\n')
#file.write('    char table_name[128][128];\n')
#file.write('    char sql[256];\n')
#file.write('    uint16 loop = 0;\n')
#file.write('    uint16 table_cnt = 0;\n')
#file.write('    sqlite3_stmt *stmt = NULL;\n')
#file.write('\n')
#file.write('    sal_memset(&table_name, 0, sizeof(table_name));\n')
#file.write('    sal_memset(&sql, 0, sizeof(sql));\n')
#file.write('\n')
#file.write('    if (1 == choice)\n')
#file.write('    {\n')
#file.write('		ret = sqlite3_open_v2(warmboot_db_path_test1, &pFile, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);\n')
#file.write('    }\n')
#file.write('    else if (2 == choice)\n')
#file.write('    {\n')
#file.write('        ret = sqlite3_open_v2(warmboot_db_path_test2, &pFile, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);\n')
#file.write('    }\n')
#file.write('    if ( ret != SQLITE_OK )\n')
#file.write('    {\n')
#file.write('        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Can not open database: %s\\n", sqlite3_errmsg(pFile));\n')
#file.write('        sqlite3_close(pFile);\n')
#file.write('        return (ret > 0) ? -ret : ret;\n')
#file.write('    }\n')
#file.write('\n')
#file.write('    ret = sqlite3_prepare_v2(pFile, "select name from sqlite_master where type = \'table\';", -1, &stmt, NULL);\n')
#file.write('    if ( ret != SQLITE_OK )\n')
#file.write('    {\n')
#file.write('        CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "BEGIN TRANSACTION error! error code: %d, error message: %s\\n", ret, sqlite3_errmsg(pFile));\n')
#file.write('        goto done;\n')
#file.write('    }\n')
#file.write('\n')
#file.write('    while ( sqlite3_step(stmt) == SQLITE_ROW )\n')
#file.write('    {\n')
#file.write('        sal_memcpy(&table_name[table_cnt], sqlite3_column_text(stmt, 0), sizeof(table_name[table_cnt]));\n')
#file.write('        table_cnt++;\n')
#file.write('    }\n')
#file.write('    if (table_name != NULL)\n')
#file.write('    {\n')
#file.write('        for(loop = 0; loop < table_cnt; loop++)\n')
#file.write('        {\n')
#file.write('            sal_sprintf(sql, "DROP TABLE \'%s\'", table_name[loop]);\n')
#file.write('            ret = sqlite3_exec(pFile, sql, NULL, NULL, NULL);\n')
#file.write('            if ( ret != SQLITE_OK )\n')
#file.write('            {\n')
#file.write('                CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Create table_sort %s error ret: %d! error message: %s\\n", table_name[loop], ret, sqlite3_errmsg(pFile));\n')
#file.write('                goto done;\n')
#file.write('            }\n')
#file.write('        }\n')
#file.write('    }\n')
#file.write('    return ret;\n')
#file.write('done:\n')
#file.write('    if ( stmt )\n')
#file.write('    {\n')
#file.write('        sqlite3_finalize( stmt );\n')
#file.write('    }\n')
#file.write('    return ret;\n')
#file.write('}\n')
structure_name_bak = 0
module_name_bak = 0
structure_sync = []
field = [] 
structure_dic = {}
structure_dic_special = {}
module_dic = {}
#create dict
for line in file_line:
	line_split = line.split(':')
	structure_name = line_split[1]
	module_name = line_split[0]
	if (structure_name_bak != structure_name):#create structure&field dict
		structure_name_bak = structure_name	
		field = []
		field.append(line_split[2]) 
		structure_dic[structure_name] = field
		#if ('uint32%calc_key_len[0]\n' == line_split[2]): #create module&structure dict
		#	structure_sync = []
		#	structure_sync.append(line_split[1])
		#	module_dic[module_name] = structure_sync
	else:
		field.append(line_split[2]) #create structure&field dict
		structure_dic[structure_name] = field
		#if ('uint32%calc_key_len[0]\n' == line_split[2]): #create module&structure dict
		#	structure_sync.append(line_split[1])
		#	module_dic[module_name] = structure_sync
	if (module_name_bak == module_name): #create module&structure dict
		if ('uint32%calc_key_len[0]\n' == line_split[2]):
			structure_sync.append(line_split[1])
			module_dic[module_name] = structure_sync
	else:
		structure_sync = []
	module_name_bak = module_name
#create special dic about sys_wb_nh_info_com_t
structure_dic_special[2] = 'sys_wb_nh_info_brguc_t'
structure_dic_special[3] = 'sys_wb_nh_info_ipuc_t'
structure_dic_special[4] = 'sys_wb_nh_info_mpls_t'
structure_dic_special[5] = 'sys_wb_nh_info_ecmp_t'
structure_dic_special[6] = 'sys_wb_nh_info_special_t'
structure_dic_special[7] = 'sys_wb_nh_info_special_t'
structure_dic_special[8] = 'sys_wb_nh_info_special_t'
structure_dic_special[9] = 'sys_wb_nh_info_special_t'
structure_dic_special[10] = 'sys_wb_nh_info_special_t'
structure_dic_special[11] = 'sys_wb_nh_info_rspan_t'
structure_dic_special[12] = 'sys_wb_nh_info_ip_tunnel_t'
structure_dic_special[13] = 'sys_wb_nh_info_trill_t'
structure_dic_special[14] = 'sys_wb_nh_info_misc_t'
structure_dic_special[15] = 'sys_wb_nh_info_wlan_t'
max_count = 16
count = 2
count_special = 2
count_chr = ''
special_start = 1

#print structure_dic_special
#print structure_dic		
#print module_dic

#create .c code
for module in module_dic.keys():
	file.write('/****merge start****/\n')
	file.write('int32'+' '+'ctc_app_wb_test_'+module+'(uint8 lchip, uint8 choice)\n')
	file.write('{\n')
	for loop in range(len(module_dic[module])):
		structure_def = module_dic[module][loop]
		structure_def = re.sub('sys_', 'p_', structure_def)
		structure_def = re.sub('_t', '', structure_def)
		file.write('    '+module_dic[module][loop]+'*'+' '+structure_def+' '+'='+' '+'NULL;\n')
	loop = 0
	file.write('    ctc_wb_query_t wb_query;\n')
	file.write('    char sql[14000];\n')
	file.write('    char array_tmp[5120][256];\n')
	file.write('    uint32 array_tmp_num = 0;\n')
	file.write('    char array_single[32];\n')
	file.write('    int32 ret = 0;\n')
	file.write('    uint32 array_len = 0;\n')
	file.write('    uint32 array_raw = 0;\n')
	file.write('    uint32 array_column = 0;\n')
	file.write('    uint32 array_len_in_structure = 0;\n')
	file.write('    uint32 loop = 0;\n')
	file.write('    uint32 loop1 = 0;\n')
	file.write('    sqlite3_stmt *stmt1 = NULL;\n')
	file.write('    char* point1 = "\\\\\\\\";\n')
	file.write('    char* point2 = "||";\n')
	file.write('    uint32 loop_row = 0;\n')
	file.write('    uint32 loop_column = 0;\n')
	file.write('    uint32 loop_in_structurn = 0;\n')
	file.write('\n')
	file.write('    array_tmp_num = 0;\n')
	file.write('    array_len = 0;\n')
	file.write('    array_raw = 0;\n')
	file.write('    array_column = 0;\n')
	file.write('    array_len_in_structure = 0;\n')
	file.write('    loop = 0;\n')
	file.write('    loop_row = 0;\n')
	file.write('    loop_column = 0;\n')
	file.write('    loop_in_structurn = 0;\n')
	file.write('\n')
	file.write('\n')
	file.write('	sal_printf("ctc_app_wb_test_'+module+'\\n");\n')
	file.write('	/*init*/\n')
	file.write('    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));\n')
	file.write('    wb_query.buffer = mem_malloc(MEM_NEXTHOP_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	file.write('    sal_memset(&array_tmp, 0, sizeof(array_tmp));\n')
	file.write('    sal_memset(&array_single, 0, sizeof(array_single));\n')
	file.write('    sal_memset(&sql, 0, sizeof(char));\n')
	file.write('    if (NULL == wb_query.buffer)\n')
	file.write('    {\n')
	file.write('        ret = CTC_E_NO_MEMORY;\n')
	file.write('        goto error_1;\n')
	file.write('    }\n')
	file.write('    sal_memset(wb_query.buffer, 0, CTC_WB_DATA_BUFFER_LENGTH);\n')
	file.write('    CTC_ERROR_GOTO(ctc_app_wb_init(lchip,CTC_WB_STATUS_RELOADING), ret, error_1);\n')
	file.write('    CTC_ERROR_GOTO(ctc_app_wb_test_init(choice), ret, error_1);\n')
	file.write('\n')
	for loop in range(len(module_dic[module])):
		file.write('    /*query '+module_dic[module][loop]+'*/\n')
		subid = module_dic[module][loop]
		subid = re.sub(r'_t$', '', subid,0)
		subid = re.sub('sys_wb_'+module+'_', '', subid)
		if (module_dic[module][loop] == 'sys_wb_nh_vni_fid_t'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_VNI);\n')
		elif (module_dic[module][loop] == 'sys_wb_nh_l2edit_t'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_NEXTHOP, SYS_WB_APPID_NH_SUBID_INFO_COM);\n')
		else:
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_NEXTHOP, SYS_WB_APPID_'+module.upper()+'_SUBID_'+subid.upper()+');\n')
		file.write('    CTC_ERROR_GOTO(ctc_wb_query_entry(&wb_query), ret, error_1);\n')
		file.write('    for (loop1 = 0; loop1 < wb_query.valid_cnt; loop1 ++)\n')
		file.write('    {\n')
		structure_def = module_dic[module][loop]
		structure_def = re.sub('sys_', 'p_', structure_def)
		structure_def = re.sub('_t', '', structure_def)
		file.write('    	'+structure_def+' = ('+module_dic[module][loop]+' *)wb_query.buffer + loop1;\n')
		file.write('    	/*synup '+module_dic[module][loop]+'*/\n')
		file.write('    	if (0 == loop1)\n')
		file.write('        {\n')
		create_table = ''
		create_table_value = ''
		create_tabel_tmp = ''
		create_tabel_value_tmp = ''
		create_table_column = 0
		create_table_special = {}
		create_table_value_special = {}
		create_table_column_special = {}
		for create_table_loop in structure_dic[module_dic[module][loop]]:
			create_table_tmp = create_table_loop.split('%')
			#######special process about sys_wb_nh_info_com_t
			if ('sys_wb_nh_info_com_t' == module_dic[module][loop]):
				if (-1 != create_table_tmp[1].find('info')):		
					if ((-1 != create_table_tmp[1].find('[') and 'calc_key_len[0]\n' != create_table_tmp[1] and -1 != create_table_tmp[0].find('sys_'))):
						type = 'TEXT'
						create_table_tmp[1] = re.sub('\[.*\]', '', create_table_tmp[1])
						for field_sub in structure_dic[create_table_tmp[0]]:
							field_tmp = field_sub.split('%')
							field_tmp[1] = re.sub('\[.*\]', '', field_tmp[1])
							create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+' '+type+', '
							create_table_value = create_table_value+'?, '
							create_table_column = create_table_column + 1
					elif ((-1 != create_table_tmp[1].find('[') and 'calc_key_len[0]\n' != create_table_tmp[1] and -1 == create_table_tmp[0].find('sys_')) or ('mac_addr_t' == create_table_tmp[0])):
						type = 'TEXT'
						create_table_tmp[1] = re.sub('\[.*\]', '', create_table_tmp[1])
						create_table = create_table+create_table_tmp[1].replace('\n','')+' '+type+', '
						create_table_value = create_table_value+'?, '
						create_table_column = create_table_column + 1
					elif ('calc_key_len[0]\n' != create_table_tmp[1] and -1 == create_table_tmp[0].find('sys_')):
						type = 'INT'			
						create_table = create_table+create_table_tmp[1].replace('\n','')+' '+type+', '
						create_table_value = create_table_value+'?, '
						create_table_column = create_table_column + 1
					elif (-1 != create_table_tmp[0].find('sys_')):
						type = 'INT'
						for field_sub in structure_dic[create_table_tmp[0]]:
							field_tmp = field_sub.split('%')
							if (-1 != field_tmp[0].find('sys_')):
								for field_sub_sub in structure_dic[field_tmp[0]]:
									field_tmp_tmp = field_sub_sub.split('%')
									if (-1 != field_tmp_tmp[0].find('sys_')):
										for field_sub_sub in structure_dic[field_tmp_tmp[0]]:
											field_tmp_tmp_tmp = field_sub_sub.split('%')
											field_tmp_tmp_tmp[1] = re.sub('\[.*\]', '', field_tmp_tmp_tmp[1]) 
											if (create_table_column == 30 or create_table_column == 60 or create_table_column == 90 or create_table_column == 120 or create_table_column == 150 or create_table_column == 180 or create_table_column == 210 or create_table_column == 240 or create_table_column == 270):
												create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+'_'+field_tmp_tmp_tmp[1].replace('\n','')+' '+type+', \\\n' 
											else:
												create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+'_'+field_tmp_tmp_tmp[1].replace('\n','')+' '+type+', '
											create_table_value = create_table_value+'?, '
											create_table_column = create_table_column + 1
									else:
										if (create_table_column == 30 or create_table_column == 60 or create_table_column == 90 or create_table_column == 120 or create_table_column == 150 or create_table_column == 180 or create_table_column == 210 or create_table_column == 240 or create_table_column == 270):
											create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+' '+type+', \\\n' 
										else:
											create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+' '+type+', '
										create_table_value = create_table_value+'?, '
										create_table_column = create_table_column + 1
							else:
								if (-1 != field_tmp[1].find('[')):
									type = 'TEXT'
									field_tmp[1] = re.sub('\[.*\]', '', field_tmp[1])
								create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+' '+type+', '
								create_table_value = create_table_value+'?, '
								create_table_column = create_table_column + 1	
					create_table = re.sub(', $', '',create_table)
					create_table = re.sub('\.', '_',create_table)
					create_table_value = re.sub(', $', '',create_table_value)	
					create_table_special[create_table_tmp[0]] = create_table	
					create_table_value_special[create_table_tmp[0]] = create_table_value
					create_table_column_special[create_table_tmp[0]] = create_table_column
					if ((len(structure_dic[module_dic[module][loop]]) - 1) > structure_dic[module_dic[module][loop]].index(create_table_loop)):
						create_table = ''
						create_table_value = ''
						continue
					file.write('		switch (p_wb_nh_info_com->nh_type)\n')
					file.write('		{\n')
					while (count < max_count):
						count_chr = '%d'%count
						file.write('			case '+count_chr+':\n')							
						file.write('    			sal_sprintf(sql, "CREATE TABLE \'%s\'('+create_tabel_tmp+create_table_special[structure_dic_special[count]]+');", "'+module_dic[module][loop]+'");\n')
						create_table = ''
						#create_tabel_tmp = ''
						create_table_value = ''
						#create_tabel_value_tmp = ''
						file.write('				break;\n')
						count = count + 1
					count = 2
					file.write('			default:\n')
					file.write('				break;\n')
					file.write('				}\n')
					continue
				else:
					if ('calc_key_len[0]\n' != create_table_tmp[1] and -1 == create_table_tmp[0].find('sys_')):
						type = 'INT'			
						create_table = create_table+create_table_tmp[1].replace('\n','')+' '+type+', '
						create_table_value = create_table_value+'?, '
						create_table_column = create_table_column + 1
					elif (-1 != create_table_tmp[0].find('sys_')):
						type = 'INT'
						for field_sub in structure_dic[create_table_tmp[0]]:
							field_tmp = field_sub.split('%')			
							create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+' '+type+', '			
							create_table_value = create_table_value+'?, '
							create_table_column = create_table_column + 1	
						create_tabel_tmp = create_table
						create_tabel_value_tmp = create_table_value
						create_table = ''
						create_table_value = ''
					continue
			############normal process
			if ((-1 != create_table_tmp[1].find('[') and 'calc_key_len[0]\n' != create_table_tmp[1] and -1 != create_table_tmp[0].find('sys_'))):
				type = 'TEXT'
				create_table_tmp[1] = re.sub('\[.*\]', '', create_table_tmp[1])
				for field_sub in structure_dic[create_table_tmp[0]]:
					field_tmp = field_sub.split('%')
					field_tmp[1] = re.sub('\[.*\]', '', field_tmp[1])
					create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+' '+type+', '
					create_table_value = create_table_value+'?, '
					create_table_column = create_table_column + 1
			elif ((-1 != create_table_tmp[1].find('[') and 'calc_key_len[0]\n' != create_table_tmp[1] and -1 == create_table_tmp[0].find('sys_')) or ('mac_addr_t' == create_table_tmp[0])):
				type = 'TEXT'
				create_table_tmp[1] = re.sub('\[.*\]', '', create_table_tmp[1])
				create_table = create_table+create_table_tmp[1].replace('\n','')+' '+type+', '
				create_table_value = create_table_value+'?, '
				create_table_column = create_table_column + 1
			elif ('calc_key_len[0]\n' != create_table_tmp[1] and -1 == create_table_tmp[0].find('sys_')):
				type = 'INT'			
				create_table = create_table+create_table_tmp[1].replace('\n','')+' '+type+', '
				create_table_value = create_table_value+'?, '
				create_table_column = create_table_column + 1
			elif (-1 != create_table_tmp[0].find('sys_')):
				type = 'INT'
				for field_sub in structure_dic[create_table_tmp[0]]:
					field_tmp = field_sub.split('%')
					if (-1 != field_tmp[0].find('sys_')):
						for field_sub_sub in structure_dic[field_tmp[0]]:
							field_tmp_tmp = field_sub_sub.split('%')
							if (-1 != field_tmp_tmp[0].find('sys_')):
								for field_sub_sub in structure_dic[field_tmp_tmp[0]]:
									field_tmp_tmp_tmp = field_sub_sub.split('%')
									field_tmp_tmp_tmp[1] = re.sub('\[.*\]', '', field_tmp_tmp_tmp[1]) 
									if (create_table_column == 30 or create_table_column == 60 or create_table_column == 90 or create_table_column == 120 or create_table_column == 150 or create_table_column == 180 or create_table_column == 210 or create_table_column == 240 or create_table_column == 270):
										create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+'_'+field_tmp_tmp_tmp[1].replace('\n','')+' '+type+', \\\n' 
									else:
										create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+'_'+field_tmp_tmp_tmp[1].replace('\n','')+' '+type+', '
									create_table_value = create_table_value+'?, '
									create_table_column = create_table_column + 1
							else:
								if (create_table_column == 30 or create_table_column == 60 or create_table_column == 90 or create_table_column == 120 or create_table_column == 150 or create_table_column == 180 or create_table_column == 210 or create_table_column == 240 or create_table_column == 270):
									create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+' '+type+', \\\n' 
								else:
									create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+'_'+field_tmp_tmp[1].replace('\n','')+' '+type+', '
								create_table_value = create_table_value+'?, '
								create_table_column = create_table_column + 1
					else:
						if (-1 != field_tmp[1].find('[')):
							type = 'TEXT'
							field_tmp[1] = re.sub('\[.*\]', '', field_tmp[1])
						create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+' '+type+', '
						create_table_value = create_table_value+'?, '
						create_table_column = create_table_column + 1
		create_table = re.sub(', $', '',create_table)
		create_table = re.sub('\.', '_',create_table)
		create_table_value = re.sub(', $', '',create_table_value)
		#print create_table
		#print create_table_column
		if ('sys_wb_nh_info_com_t' != module_dic[module][loop]):
			file.write('    		sal_sprintf(sql, "CREATE TABLE \'%s\'('+create_table+');", "'+module_dic[module][loop]+'");\n')		
		file.write('    		ret = sqlite3_exec(pFile, sql, NULL, NULL, NULL);\n')
		file.write('    		if ( ret != SQLITE_OK )\n')
		file.write('    		{\n')
		file.write('        		CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Create table %s error ret: %d, error message: %s\\n", "'+module_dic[module][loop]+'", ret, sqlite3_errmsg(pFile));\n')
		file.write('        		goto error_1;\n')
		file.write('    		}\n')
		file.write('        }\n')
		file.write('\n')
		if ('sys_wb_nh_info_com_t' == module_dic[module][loop]):
			file.write('		switch (p_wb_nh_info_com->nh_type)\n')
			file.write('		{\n')
			while (count < max_count):
				count_chr = '%d'%count
				file.write('			case '+count_chr+':\n')					
				file.write('    			sal_sprintf(sql, "INSERT INTO \''+module_dic[module][loop]+'\' VALUES ('+create_tabel_value_tmp+create_table_value_special[structure_dic_special[count]]+');");\n')
				file.write('				break;\n')
				count = count + 1
			count = 2
			file.write('			default:\n')
			file.write('				break;\n')
			file.write('				}\n')
		else:
			file.write('    	sal_sprintf(sql, "INSERT INTO \''+module_dic[module][loop]+'\' VALUES ('+create_table_value+');");\n')
		file.write('    	sqlite3_prepare_v2(pFile, sql, -1, &stmt1, NULL);\n')
		loop_add_data = 0
		for field_sync in structure_dic[module_dic[module][loop]]:
			##############special sys_wb_nh_info_com_t
			if ('sys_wb_nh_info_com_t' == module_dic[module][loop]):
				if (-1 != field_sync.find('info')):
					if ((-1 != field_sync.find('sys_')) and (-1 == field_sync.find('sys_wb_nh_info_com_hdr_t'))):
						if (special_start == 1):
							file.write('		switch (p_wb_nh_info_com->nh_type)\n')
							file.write('		{\n')
							special_start = 0	
						loop_add_data = 10
						if (-1 != field_sync.find('sys_wb_nh_info_brguc_t')):
							file.write('		case 2:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_brguc_t')):
							file.write('		case 3:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_ipuc_t')):
							file.write('		case 4:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_mpls_t')):
							file.write('		case 5:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_ecmp_t')):
							file.write('		case 6:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_special_t')):
							file.write('		case 7:\n')
							file.write('		case 8:\n')
							file.write('		case 9:\n')
							file.write('		case 10:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_rspan_t')):
							file.write('		case 11:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_ip_tunnel_t')):
							file.write('		case 12:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_trill_t')):
							file.write('		case 13:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_misc_t')):
							file.write('		case 14:\n')
							file.write('\n')
						elif (-1 != field_sync.find('sys_wb_nh_info_wlan_t')):
							file.write('		case 15:\n')
							file.write('\n')
				elif (-1 == field_sync.find('info')):
					if ((-1 == field_sync.find('calc_key_len[0]\n')) and (-1 != field_sync.find('nh_type'))):
						loop_add_data = 0
					elif (-1 != field_sync.find('sys_wb_nh_info_com_hdr_t')):
						loop_add_data = 2
			filed_name = field_sync.split('%')
			if ((-1 != filed_name[1].find('[') and 'calc_key_len[0]\n' != filed_name[1] and -1 == filed_name[0].find('sys_')) or ( -1 != filed_name[0].find('mac_addr_t')) or 
				( -1 != filed_name[0].find('ipv6_addr_t')) or ( -1 != filed_name[0].find('ctc_port_bitmap_t'))): #normal array
				if ((-1 != filed_name[1].find(']['))):
					loop_add_data = loop_add_data + 1
					loop_add_data_chr = '%d'%loop_add_data
					array_num = re.findall('\[.*\]\[',filed_name[1])
					array = re.sub('\[.*\]', '', filed_name[1])
					file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0][0]);\n')
					file.write('		array_raw = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0]);\n')
					file.write('		array_column = array_len/array_raw;\n')
					file.write('    	while (loop_column < array_raw)\n')
					file.write('    	{\n')
					array_num = re.findall('\]\[.*\]',filed_name[1])
					file.write('    		while (loop_row < array_column)\n')
					file.write('    		{\n')
					if (filed_name[0] == 'uint64'):
						file.write('    			sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'[loop_column][loop_row]);\n')
					else:
						file.write('    			sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'[loop_column][loop_row]);\n')
					file.write('        		sal_strcat(array_tmp[array_tmp_num], array_single);\n')
					file.write('        		loop_row++;\n')
					file.write('    		}\n')
					file.write('        	loop_row = 0;\n')
					file.write('        	sal_strcat(array_tmp[array_tmp_num], point2);\n')
					file.write('        	loop_column++;\n')
					file.write('    	}\n')
					file.write('        loop_column = 0;\n')
					file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
					file.write('        array_tmp_num ++;\n')
					file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
					file.write('\n')
				else:
					loop_add_data = loop_add_data + 1
					loop_add_data_chr = '%d'%loop_add_data
					array = re.sub('\[.*\]', '', filed_name[1])
					file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0]);\n')
					file.write('    	while (loop < array_len)\n')
					file.write('    	{\n')
					if (filed_name[0] == 'uint64'):
						file.write('        	sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'[loop]);\n')
					else:
						file.write('        	sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'[loop]);\n')
					file.write('        	sal_strcat(array_tmp[array_tmp_num], array_single);\n')
					file.write('        	loop++;\n')
					file.write('    	}\n')
					file.write('    	loop = 0;\n')
					file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
					file.write('        array_tmp_num ++;\n')
					file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
					file.write('\n')		
			elif ('calc_key_len[0]\n' != filed_name[1] and -1 == filed_name[0].find('sys_')): #normal entry
				loop_add_data = loop_add_data + 1
				loop_add_data_chr = '%d'%loop_add_data
				file.write('		sqlite3_bind_int(stmt1, '+loop_add_data_chr+', '+structure_def+'->'+filed_name[1].replace('\n','')+');\n')	
				file.write('\n')
			elif ((-1 != filed_name[0].find('sys_')) and (-1 != filed_name[1].find('[')) and (-1 == filed_name[1].find(']['))): #structure array
				for field_sub in structure_dic[filed_name[0]]:
					field_tmp = field_sub.split('%')
					if (-1 == field_tmp[1].find('[') and -1 == field_tmp[0].find('ipv6_addr_t') and -1 == field_tmp[0].find('mac_addr_t')):
						loop_add_data = loop_add_data + 1
						loop_add_data_chr = '%d'%loop_add_data
						array = re.sub('\[.*\]', '', filed_name[1])
						file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0]);\n')
						file.write('    	while (loop < array_len)\n')
						file.write('    	{\n')
						if (filed_name[0] == 'uint64'):
							file.write('        	sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'[loop]'+field_tmp[1].replace('\n','')+');\n')
						else:
							file.write('        	sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'[loop].'+field_tmp[1].replace('\n','')+');\n')
						file.write('        	sal_strcat(array_tmp[array_tmp_num], array_single);\n')
						file.write('        	loop++;\n')
						file.write('    	}\n')
						file.write('    	loop = 0;\n')
						file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
						file.write('        array_tmp_num ++;\n')
						file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
						file.write('\n')
					else:
						loop_add_data = loop_add_data + 1
						loop_add_data_chr = '%d'%loop_add_data
						array = re.sub('\[.*\]', '', filed_name[1])
						array_structure_name = re.sub('\[.*\]', '', field_tmp[1])
						file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0]);\n')
						file.write('    	while (loop < array_len)\n')
						file.write('    	{\n')
						file.write('			array_len_in_structure = sizeof('+structure_def+'->'+array.replace('\n','')+'[loop].'+array_structure_name.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[loop].'+array_structure_name.replace('\n','')+'[0]);\n')
						file.write('    		while (loop_in_structurn < array_len_in_structure)\n')
						file.write('    		{\n')
						if (filed_name[0] == 'uint64'):
							file.write('        		sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'[loop].'+array_structure_name.replace('\n','')+'[loop_in_structurn]);\n')
						else:
							file.write('        		sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'[loop].'+array_structure_name.replace('\n','')+'[loop_in_structurn]);\n')
						file.write('        		sal_strcat(array_tmp[array_tmp_num], array_single);\n')
						file.write('        		loop_in_structurn++;\n')
						file.write('    		}\n')
						file.write('        	sal_strcat(array_tmp[array_tmp_num], point2);\n')
						file.write('        	loop_in_structurn = 0;\n')					
						file.write('        	loop++;\n')
						file.write('    	}\n')
						file.write('    	loop = 0;\n')
						file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
						file.write('        array_tmp_num ++;\n')
						file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
						file.write('\n')
			elif ((-1 != filed_name[0].find('sys_')) and (-1 != filed_name[1].find('[')) and (-1 != filed_name[1].find(']['))): #double dimensional structure array and array in that structure
				for field_sub in structure_dic[filed_name[0]]:
					array_num = ""
					field_tmp = field_sub.split('%')
					if ('calc_key_len[0]\n' == field_tmp[1]):
						continue
					if (-1 == field_tmp[1].find('[') and -1 == field_tmp[0].find('mac_addr_t')):
						loop_add_data = loop_add_data + 1
						loop_add_data_chr = '%d'%loop_add_data
						array_num = re.findall('\[.*\]\[',filed_name[1])
						array_structure_name = re.sub('\[.*\]', '', field_tmp[1])
						array = re.sub('\[.*\]', '', filed_name[1])
						file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0][0]);\n')
						file.write('		array_raw = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0]);\n')
						file.write('		array_column = array_len/array_raw;\n')
						file.write('    	while (loop_column < array_raw)\n')
						file.write('    	{\n')
						array_num = re.findall('\]\[.*\]',filed_name[1])
						file.write('    		while (loop_row < array_column)\n')
						file.write('    		{\n')
						if (filed_name[0] == 'uint64'):
							file.write('    			sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'[loop_column][loop_row].'+array_structure_name.replace('\n','')+');\n')
						else:
							file.write('    			sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'[loop_column][loop_row].'+array_structure_name.replace('\n','')+');\n')
						file.write('        		sal_strcat(array_tmp[array_tmp_num], array_single);\n')
						file.write('        		loop_row++;\n')
						file.write('    		}\n')
						file.write('        	loop_row = 0;\n')
						file.write('        	sal_strcat(array_tmp[array_tmp_num], point2);\n')
						file.write('        	loop_column++;\n')
						file.write('    	}\n')
						file.write('        loop_column = 0;\n')
						file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
						file.write('        array_tmp_num ++;\n')
						file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
						file.write('\n')
					else:
						loop_add_data = loop_add_data + 1
						loop_add_data_chr = '%d'%loop_add_data
						array_num = re.findall('\[.*\]\[',filed_name[1])
						array_structure_name = re.sub('\[.*\]', '', field_tmp[1])
						array = re.sub('\[.*\]', '', filed_name[1])
						file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0][0]);\n')
						file.write('		array_raw = sizeof('+structure_def+'->'+array.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'[0]);\n')
						file.write('		array_column = array_len/array_raw;\n')
						file.write('    	while (loop_column < array_raw)\n')
						file.write('    	{\n')
						array_num = re.findall('\]\[.*\]',filed_name[1])
						file.write('    		while (loop_row < array_column)\n')
						file.write('    		{\n')
						array_num = re.findall('\[.*\]',field_tmp[1])
						if (-1 == field_tmp[0].find('mac_addr_t')):
							file.write('    			while (loop < '+array_num[0].replace('[','').replace(']','')+')\n')
						else:
							file.write('    			while (loop < 6)\n')
						file.write('    			{\n')
						if (filed_name[0] == 'uint64'):
							file.write('    				sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'[loop_column][loop_row].'+array_structure_name.replace('\n','')+'[loop]);\n')
						else:
							file.write('    				sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'[loop_column][loop_row].'+array_structure_name.replace('\n','')+'[loop]);\n')
						file.write('        			sal_strcat(array_tmp[array_tmp_num], array_single);\n')
						file.write('        			loop++;\n')
						file.write('    			}\n')
						file.write('        		loop = 0;\n')
						file.write('        		sal_strcat(array_tmp[array_tmp_num], point1);\n')
						file.write('        		loop_row++;\n')
						file.write('    		}\n')
						file.write('        	loop_row = 0;\n')
						file.write('        	sal_strcat(array_tmp[array_tmp_num], point2);\n')
						file.write('        	loop_column++;\n')
						file.write('    	}\n')
						file.write('        loop_column = 0;\n')
						file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
						file.write('        array_tmp_num ++;\n')
						file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
						file.write('\n')
			elif (-1 != filed_name[0].find('sys_')): #array in structure
				for field_sub in structure_dic[filed_name[0]]:
					loop_add_data = loop_add_data + 1
					loop_add_data_chr = '%d'%loop_add_data
					field_tmp = field_sub.split('%')
					if ((( -1 != field_tmp[0].find('mac_addr_t')) or ( -1 != field_tmp[0].find('ipv6_addr_t')) or ( -1 != field_tmp[1].find(']'))) and ( -1 == field_tmp[0].find('sys_'))):
						array_structure_name = re.sub('\[.*\]', '', field_tmp[1])
						array = re.sub('\[.*\]', '', filed_name[1])
						file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'[0]);\n')
						file.write('    	while (loop < array_len)\n')
						file.write('    	{\n')
						if (filed_name[0] == 'uint64'):
							file.write('        	sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'[loop]);\n')
						else:
							file.write('        	sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'[loop]);\n')
						file.write('        	sal_strcat(array_tmp[array_tmp_num], array_single);\n')
						file.write('        	loop++;\n')
						file.write('    	}\n')
						file.write('        loop = 0;\n')
						file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
						file.write('        array_tmp_num ++;\n')
						file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
						file.write('\n')
					elif (( -1 != field_tmp[0].find('sys_'))):
						loop_add_data = loop_add_data - 1
						for field_sub_sub in structure_dic[field_tmp[0]]:
							loop_add_data = loop_add_data + 1
							loop_add_data_chr = '%d'%loop_add_data
							field_tmp_tmp = field_sub_sub.split('%')
							if ('calc_key_len[0]\n' == field_tmp_tmp[1]):
								continue
							if (( -1 != field_tmp_tmp[0].find('mac_addr_t')) or ( -1 != field_tmp_tmp[0].find('ipv6_addr_t')) or ( -1 != field_tmp_tmp[1].find(']'))):
								array_structure_name = re.sub('\[.*\]', '', field_tmp[1])
								array_structure_sub_name = re.sub('\[.*\]', '', field_tmp_tmp[1])
								array = re.sub('\[.*\]', '', filed_name[1])
								file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+'[0]);\n')
								file.write('    	while (loop < array_len)\n')
								file.write('    	{\n')
								if (filed_name[0] == 'uint64'):
									file.write('        	sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+'[loop]);\n')
								else:
									file.write('        	sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+'[loop]);\n')
								file.write('        	sal_strcat(array_tmp[array_tmp_num], array_single);\n')
								file.write('        	loop++;\n')
								file.write('    	}\n')
								file.write('        loop = 0;\n')
								file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
								file.write('        array_tmp_num ++;\n')
								file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
								file.write('\n')
							else:
								if (-1 != field_tmp_tmp[0].find('sys_')):
									loop_add_data = loop_add_data - 1
									for field_sub_sub_sub in structure_dic[field_tmp_tmp[0]]:
										loop_add_data = loop_add_data + 1
										loop_add_data_chr = '%d'%loop_add_data
										field_tmp_tmp_tmp = field_sub_sub_sub.split('%')
										if ('calc_key_len[0]\n' == field_tmp_tmp[1]):
											continue
										if ( -1 != field_tmp_tmp_tmp[0].find('ipv6_addr_t')):
											array_structure_name = re.sub('\[.*\]', '', field_tmp[1])
											array_structure_sub_name = re.sub('\[.*\]', '', field_tmp_tmp[1])
											array_structure_sub_sub_name = re.sub('\[.*\]', '', field_tmp_tmp_tmp[1])
											array = re.sub('\[.*\]', '', filed_name[1])
											file.write('		array_len = sizeof('+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+'.'+array_structure_sub_sub_name.replace('\n','')+')/sizeof('+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+'.'+array_structure_sub_sub_name.replace('\n','')+'[0]);\n')
											file.write('    	while (loop < array_len)\n')
											file.write('    	{\n')
											if (filed_name[0] == 'uint64'):
												file.write('        	sal_sprintf(array_single, "%"PRId64"/", '+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+'.'+array_structure_sub_sub_name.replace('\n','')+'[loop]);\n')
											else:
												file.write('        	sal_sprintf(array_single, "%d/", '+structure_def+'->'+array.replace('\n','')+'.'+array_structure_name.replace('\n','')+'.'+array_structure_sub_name.replace('\n','')+'.'+array_structure_sub_sub_name.replace('\n','')+'[loop]);\n')
											file.write('        	sal_strcat(array_tmp[array_tmp_num], array_single);\n')
											file.write('        	loop++;\n')
											file.write('    	}\n')
											file.write('        loop = 0;\n')
											file.write('    	sqlite3_bind_text(stmt1, '+loop_add_data_chr+', array_tmp[array_tmp_num], -1, NULL);\n')
											file.write('        array_tmp_num ++;\n')
											file.write('		sal_memset(&array_tmp[array_tmp_num], 0, sizeof(array_tmp[array_tmp_num]));\n')
											file.write('\n')
										else:
											file.write('		sqlite3_bind_int(stmt1, '+loop_add_data_chr+', '+structure_def+'->'+filed_name[1].replace('\n','')+'.'+field_tmp[1].replace('\n','')+'.'+field_tmp_tmp[1].replace('\n','')+'.'+field_tmp_tmp_tmp[1].replace('\n','')+');\n')
								else:
									file.write('		sqlite3_bind_int(stmt1, '+loop_add_data_chr+', '+structure_def+'->'+filed_name[1].replace('\n','')+'.'+field_tmp[1].replace('\n','')+'.'+field_tmp_tmp[1].replace('\n','')+');\n')
								file.write('\n')			
					else:
						file.write('		sqlite3_bind_int(stmt1, '+loop_add_data_chr+', '+structure_def+'->'+filed_name[1].replace('\n','')+'.'+field_tmp[1].replace('\n','')+');\n')
						file.write('\n')
			if ('sys_wb_nh_info_com_t' == module_dic[module][loop]):
					if (-1 != filed_name[1].find('info')):
						file.write('		break;\n')				
		if ('sys_wb_nh_info_com_t' == module_dic[module][loop]):
				file.write('		default:\n')
				file.write('			break;\n')
				file.write('		}\n')	
		file.write('		CTC_ERROR_GOTO(sqlite3_step(stmt1), ret, error_3);\n')	
		file.write('		sqlite3_reset(stmt1);\n')
		file.write('    	sqlite3_finalize( stmt1 );\n')
		file.write('    sal_memset(&sql, 0, sizeof(char));\n')
		file.write('    }\n')
	##file.write('    CTC_ERROR_GOTO(ctc_app_wb_test_init_done(choice), ret, error_2);\n')
	##file.write('    CTC_ERROR_GOTO(ctc_app_wb_init_done(lchip), ret, error_1);\n')
	file.write('    goto error_2;\n')
	file.write('    error_3:\n')
	file.write('    CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Sqlite3_step(add) error ret: %d, error message: %s\\n", ret, sqlite3_errmsg(pFile));\n')
	file.write('    error_2:\n')
	file.write('    error_1:\n')
	file.write('    ctc_app_wb_test_init_done(choice);\n')
	file.write('    ctc_app_wb_init_done(lchip);\n')
	file.write('    if (wb_query.key)\n')
	file.write('    {\n')
	file.write('        mem_free(wb_query.key);\n')
	file.write('    }\n')
	file.write('    if (wb_query.buffer)\n')
	file.write('    {\n')
	file.write('        mem_free(wb_query.buffer);\n')
	file.write('    }\n')
	file.write('    return ret;\n')
	file.write('}\n')
	file.write('/****merge end****/\n')
file.write('#endif\n')
file.write('#endif\n')
file.write('\n')
file.write('\n')
file.write('\n')
file.write('\n')		
file.close()
#.h
file_h.write('\n')
file_h.write('\n')
file_h.write('#if 0\n')
file_h.write('/**\n')
file_h.write(' @file ctc_app_warmboot_test.h\n')
file_h.write('\n')
file_h.write(' @date 2017-04-18\n')
file_h.write('\n')
file_h.write(' @version v1.0\n')
file_h.write('\n')
file_h.write(' The file defines warmboot test api\n')
file_h.write('*/\n')
file_h.write('#ifndef _CTC_APP_WARMBOOT_TEST_H_\n')
file_h.write('#define _CTC_APP_WARMBOOT_TEST_H_\n')
file_h.write('#ifdef __cplusplus\n')
file_h.write('extern "C" {\n')
file_h.write('#endif\n')
file_h.write('/****************************************************************\n')
file_h.write(' *\n')
file_h.write(' * Header Files\n')
file_h.write(' *\n')
file_h.write(' ***************************************************************/\n')
file_h.write('#include "sal.h"\n')
file_h.write('#include "sal_types.h"\n')
file_h.write('#include "ctc_const.h"\n')
file_h.write('#include "ctc_mix.h"\n')
file_h.write('#include "ctc_error.h"\n')
file_h.write('#include "ctc_macro.h"\n')
file_h.write('#include "ctc_debug.h"\n')
file_h.write('#include "ctc_stats.h"\n')
file_h.write('#include "ctc_warmboot.h"\n')
file_h.write('/****************************************************************\n')
file_h.write(' *\n')
file_h.write(' * Structures\n')
file_h.write(' *\n')
file_h.write(' ***************************************************************/\n')
file_h.write('\n')
start = 0
block = 0
for line in file_line_common_h:
	if (line == '/***start***/\n'):
		start = 1
	if (1 == start):
		#if ('enum sys_wb_appid_scl_subid_e\n' == line or 'enum sys_wb_appid_security_subid_e\n' == line or 'enum sys_wb_appid_acl_subid_e\n' == line):
		#	block = 1
		#if (1 == block):
		#	file_h.write('//'+line)
		#else:
			file_h.write(line)
		#if ('typedef struct sys_wb_scl_sw_entry_s sys_wb_scl_sw_entry_t;\n' == line or 'typedef struct sys_wb_learn_limit_trhold_node_s sys_wb_learn_limit_trhold_node_t;\n' == line
		#	or 'typedef struct sys_wb_acl_entry_s sys_wb_acl_entry_t;\n' == line):
		#	block = 0
	if (line == '/***end***/\n'):
		start = 0	
file_h.write('/**********************************************************************************\n')
file_h.write('                      Define API function interfaces\n')
file_h.write(' ***********************************************************************************/\n')
file_h.write('extern int32\n')
file_h.write('ctc_app_wb_test_init(uint8 choice);\n')
file_h.write('extern int32\n')
file_h.write('ctc_app_wb_test_init_done(uint8 choice);\n')
file_h.write('/****merge start****/\n')
for module in module_dic.keys():
	file_h.write('extern int32\n')
	file_h.write('ctc_app_wb_test_'+module+'(uint8 lchip, uint8 choice);\n')
file_h.write('/****merge end****/\n')
file_h.write('#ifdef __cplusplus\n')
file_h.write('}\n')
file_h.write('#endif\n')
file_h.write('\n')
file_h.write('#endif  /* _CTC_APP_WARMBOOT__TEST_H_*/\n')
file_h.write('\n')
file_h.write('\n')
file_h.write('#endif')
file_h.close()
file_ele.close()

		

