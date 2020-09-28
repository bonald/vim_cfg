import re
file = open('../../core/app/ctc_app_warmboot_test.c', 'w')
file_h = open('ctc_app_warmboot_test.h', 'w')
file_nh = open('ctc_app_warmboot_test_nh.c', 'r')
file_nh_h = open('ctc_app_warmboot_test_nh.h', 'r')
file_ele = open('warmboot_test.txt','r')
file_common_h = open('../../core/usw/include/sys/sys_usw_wb_common.h','r')
file_line = file_ele.readlines()
file_line_common_h = file_common_h.readlines()
file_line_nh = file_nh.readlines()
file_line_nh_h = file_nh_h.readlines()
#.c
file.write('#include "../../app/ctc_app.h"\n')
file.write('#include "ctc_warmboot.h"\n')
file.write('#include "sys_usw_wb_common.h"\n')
file.write('#include "sys_usw_wb_nh.h"\n')
file.write('#include "../../app/usr/ctc_app_warmboot.h"\n')
file.write('#ifdef SDK_IN_USERMODE\n')
file.write('#include "../../app/usr/sqlite3.h"\n')
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
		
#print structure_dic		
#print module_dic

#create .c code
for module in module_dic.keys():
	file.write('int32'+' '+'ctc_app_wb_test_'+module+'(uint8 lchip, uint8 choice)\n')
	file.write('{\n')
	for loop in range(len(module_dic[module])):
		structure_def = module_dic[module][loop]
		structure_def = re.sub('sys_', 'p_', structure_def)
		structure_def = re.sub('_t', '', structure_def)
		file.write('    '+module_dic[module][loop]+'*'+' '+structure_def+' '+'='+' '+'NULL;\n')
	loop = 0
	file.write('    ctc_wb_query_t wb_query;\n')
	file.write('    char sql[1024];\n')
	file.write('    char array_tmp[1024][4096];\n')
	file.write('    uint8 array_tmp_num = 0;\n')
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
	file.write('    point1 = "\\\\\\\\";\n')
	file.write('    point2 = "||";\n')
	file.write('\n')
	file.write('\n')
	file.write('	sal_printf("ctc_app_wb_test_'+module+'\\n");\n')
	file.write('	/*init*/\n')
	file.write('    sal_memset(&wb_query, 0, sizeof(ctc_wb_query_t));\n')
	if (module == 'l2'):
		file.write('    wb_query.buffer = mem_malloc(MEM_FDB_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'overlay'):
		file.write('    wb_query.buffer = mem_malloc(MEM_OVERLAY_TUNNEL_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'ofb'):
		file.write('    wb_query.buffer = mem_malloc(MEM_OPF_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'stacking'):
		file.write('    wb_query.buffer = mem_malloc(MEM_STK_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'ip'):
		file.write('    wb_query.buffer = mem_malloc(MEM_IPUC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'syncether'):
		file.write('    wb_query.buffer = mem_malloc(MEM_SYNC_ETHER_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'chip'):
		file.write('    wb_query.buffer = mem_malloc(MEM_SYSTEM_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'qos'):
		file.write('    wb_query.buffer = mem_malloc(MEM_QUEUE_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'security'):
		file.write('    wb_query.buffer = mem_malloc(MEM_STMCTL_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'calpm'):
		file.write('    wb_query.buffer = mem_malloc(MEM_IPUC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'register'):
		file.write('    wb_query.buffer = mem_malloc(MEM_SYSTEM_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'interport'):
		file.write('    wb_query.buffer = mem_malloc(MEM_PORT_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	elif (module == 'nalpm'):
		file.write('    wb_query.buffer = mem_malloc(MEM_IPUC_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
	else:
		file.write('    wb_query.buffer = mem_malloc(MEM_'+module.upper()+'_MODULE,  CTC_WB_DATA_BUFFER_LENGTH);\n')
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
		subid = re.sub(r'_t$', '', subid)
		subid = re.sub('sys_wb_'+module+'_', '', subid)
		if (module == 'ip'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_IP_TUNNEL, SYS_WB_APPID_'+module.upper()+'_TUNNEL_SUBID_'+subid.upper().replace('TUNNEL_','')+');\n')
		elif (module == 'syncether'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_SYNC_ETHER, SYS_WB_APPID_'+module.upper()+'_SUBID_'+subid.upper()+');\n')
		elif (module == 'calpm'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_ALPM_MASTER);\n')
		elif (module == 'interport'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_PORT, SYS_WB_APPID_IPUC_SUBID_ALPM_MASTER);\n')
		elif (module == 'nalpm'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_IPUC, SYS_WB_APPID_IPUC_SUBID_INFO1);\n')
		elif (module_dic[module][loop] == 'sys_wb_qos_logicsrc_port_t'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_LOGIC_SRC_PORT);\n')
		elif (module_dic[module][loop] == 'sys_wb_qos_destport_t'):
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_QOS, SYS_WB_APPID_QOS_SUBID_QUEUE_DESTPORT);\n')
		else:
			file.write('	CTC_WB_INIT_QUERY_T((&wb_query), '+module_dic[module][loop]+', CTC_FEATURE_'+module.upper()+', SYS_WB_APPID_'+module.upper()+'_SUBID_'+subid.upper()+');\n')
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
		create_table_column = 0
		for create_table_loop in structure_dic[module_dic[module][loop]]:
			create_table_tmp = create_table_loop.split('%')
			if ((-1 != create_table_tmp[1].find('[') and 'calc_key_len[0]\n' != create_table_tmp[1] and -1 != create_table_tmp[0].find('sys_'))):
				type = 'TEXT'
				create_table_tmp[1] = re.sub('\[.*\]', '', create_table_tmp[1])
				for field_sub in structure_dic[create_table_tmp[0]]:
					field_tmp = field_sub.split('%')
					field_tmp[1] = re.sub('\[.*\]', '', field_tmp[1])
					create_table = create_table+create_table_tmp[1].replace('\n','')+'_'+field_tmp[1].replace('\n','')+' '+type+', '
					create_table_value = create_table_value+'?, '
					create_table_column = create_table_column + 1
			elif ((-1 != create_table_tmp[1].find('[') and 'calc_key_len[0]\n' != create_table_tmp[1] and -1 == create_table_tmp[0].find('sys_')) or ('mac_addr_t\n' == create_table_tmp[1])):
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
			elif (-1 != create_table_tmp[0].find('sys_') and 'sys_wb_ptp_device_type_t' != create_table_tmp[0]):
				type = 'INT'
				for field_sub in structure_dic[create_table_tmp[0]]:
					field_tmp = field_sub.split('%')
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
		file.write('    		sal_sprintf(sql, "CREATE TABLE \'%s\'('+create_table+');", "'+module_dic[module][loop]+'");\n')
		file.write('    		ret = sqlite3_exec(pFile, sql, NULL, NULL, NULL);\n')
		file.write('    		if ( ret != SQLITE_OK )\n')
		file.write('    		{\n')
		file.write('        		CTC_APP_DBG_OUT(CTC_DEBUG_LEVEL_DUMP, "Create table %s error ret: %d, error message: %s\\n", "'+module_dic[module][loop]+'", ret, sqlite3_errmsg(pFile));\n')
		file.write('        		goto error_1;\n')
		file.write('    		}\n')
		file.write('        }\n')
		file.write('\n')
		file.write('    	sal_sprintf(sql, "INSERT INTO \''+module_dic[module][loop]+'\' VALUES ('+create_table_value+');");\n')
		file.write('    	sqlite3_prepare_v2(pFile, sql, -1, &stmt1, NULL);\n')
		loop_add_data = 0
		for field_sync in structure_dic[module_dic[module][loop]]:
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
			elif (('calc_key_len[0]\n' != filed_name[1] and -1 == filed_name[0].find('sys_')) or 'sys_wb_ptp_device_type_t' == filed_name[0]): #normal entry
				loop_add_data = loop_add_data + 1
				loop_add_data_chr = '%d'%loop_add_data
				file.write('		sqlite3_bind_int(stmt1, '+loop_add_data_chr+', '+structure_def+'->'+filed_name[1].replace('\n','')+');\n')	
				file.write('\n')
			elif ((-1 != filed_name[0].find('sys_')) and (-1 != filed_name[1].find('[')) and (-1 == filed_name[1].find(']['))): #structure array
				for field_sub in structure_dic[filed_name[0]]:
					field_tmp = field_sub.split('%')
					if (-1 == field_tmp[1].find('[')):
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
					if (-1 == field_tmp[1].find('[')):
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
						file.write('    			while (loop < '+array_num[0].replace('[','').replace(']','')+')\n')
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
					if (( -1 != field_tmp[0].find('mac_addr_t')) or ( -1 != field_tmp[0].find('ipv6_addr_t')) or ( -1 != field_tmp[1].find(']'))):
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
					else:
						file.write('		sqlite3_bind_int(stmt1, '+loop_add_data_chr+', '+structure_def+'->'+filed_name[1].replace('\n','')+'.'+field_tmp[1].replace('\n','')+');\n')
						file.write('\n')
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
start = 0
for line in file_line_nh:
	if (line == '/****merge start****/\n'):
		start = 1
	if (1 == start):
		file.write(line)
	if (line == '/****merge end****/\n'):
		start = 0	
file.write('#endif\n')
file.write('\n')
file.write('\n')
file.write('\n')
file.write('\n')	
	
file.close()
#.h
file_h.write('\n')
file_h.write('\n')
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
		file_h.write(line)
	if (line == '/***end***/\n'):
		start = 0	
for line in file_line_nh_h:
	if (line == '/***start***/\n'):
		start = 1
	if (1 == start):
		file_h.write(line)
	if (line == '/***end***/\n'):
		start = 0	
file_h.write('/**********************************************************************************\n')
file_h.write('                      Define API function interfaces\n')
file_h.write(' ***********************************************************************************/\n')
file_h.write('extern int32\n')
file_h.write('ctc_app_wb_test_init(uint8 choice);\n')
file_h.write('extern int32\n')
file_h.write('ctc_app_wb_test_init_done(uint8 choice);\n')
for module in module_dic.keys():
	file_h.write('extern int32\n')
	file_h.write('ctc_app_wb_test_'+module+'(uint8 lchip, uint8 choice);\n')
for line in file_line_nh_h:
	if (line == '/****merge start****/\n'):
		start = 1
	if (1 == start):
		file_h.write(line)
	if (line == '/****merge end****/\n'):
		start = 0	
file_h.write('#ifdef __cplusplus\n')
file_h.write('}\n')
file_h.write('#endif\n')
file_h.write('\n')
file_h.write('#endif  /* _CTC_APP_WARMBOOT__TEST_H_*/\n')
file_h.write('\n')
file_h.write('\n')
file_h.close()
file_ele.close()

		

