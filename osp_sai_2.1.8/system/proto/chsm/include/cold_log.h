#ifndef _COLD_LOGGING_H_
#define _COLD_LOGGING_H_

#define COLD_PATH_NAME_LEN     63
#define COLD_MAX_KEY_SIZE      80
#define COLD_MAX_REC_SIZE      4096 /*data length plus header length are COLD_MAX_REC_SIZE */
#define COLD_MAX_REC_DATA_SIZE (COLD_MAX_REC_SIZE - COLD_MAX_KEY_SIZE)
#define STR_RETURN             "\r\n"
#define COLD_MAX_LINE_CHAR     255
#define COLD_CMD_EXE_CHAR      255
    
#define COLD_DEFAULT_MAX_ITEM_NUM 480   /* items in 24 hours */
#define COLD_DEFAULT_REC_DATA_SIZE 128

#define COLD_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define COLD_MIN(a,b)            (((a) < (b)) ? (a) : (b))

typedef enum {
    COLD_DB_CPU_SYS_INFO_TID = 1, 
    COLD_DB_MEM_SYS_INFO_TID = 2,
    COLD_DB_ENV_INFO_TID = 3,
    COLD_DB_GEN_INFO_TID = 4,
    COLD_DB_MAX_TABLE_ID = 10
} cold_db_table_id_t;

typedef enum {
	COLD_DB_SUCCESS= 0,
	COLD_DB_RECOVED= 1,
	COLD_DB_UPGRADED= 2,
	COLD_DB_NEW= 3,
	
	COLD_DB_NO_DB= -200,
	COLD_DB_NO_DIR,
	COLD_DB_REC_EXIST,
	COLD_DB_NO_MORE_RESOURCE,
	COLD_DB_REC_NOT_EXIST,
	COLD_DB_INVAILD_PARAM,
	COLD_DB_INVAILD_REC_LEN,
} cold_db_ret_code_t;

typedef enum {
    COLD_OF_COVER_REC = 1,    /*cover old record*/
    COLD_OF_STOP_REC = 2,      /*stop  record*/
    COLD_OF_RW_NEW_RECORD = 3,        /*delete all record and write a new record*/
} cold_db_of_hdl_flag_t;

typedef struct cold_db_tbl_header_s
{
    uint16 key_size;          
    uint16 data_size;	
    uint16 item_num; 
    uint16 cur_item_num; 
    uint16 max_item_num; 
    uint16 sysup_item_index; 
    uint8  handle_flag;              /*cold_db_of_hdl_flag_t*/    
    uint8  db_init_count;
    uint16 reserved;
} cold_db_tbl_header_t;

/***************************************************
Function:    odb_initTable
 init table during process initialization.
 setup the database environment for access.
if the table you specified exists, recover it;
 if no table:  ODB_NEW returned .
*****************************************************/
int32 cold_db_init_table(cold_db_table_id_t tableId,
                       cold_db_tbl_header_t *tbl_hdr,
                       const char* p_db_dir_name,
				       const char* p_db_name,
                       char* buffer,
                       uint16 buf_size,
                       uint16* numRecs );


/***************************************************
 Function:   odb_closeTable
*****************************************************/

int32 cold_db_close_table(cold_db_table_id_t tableId);

/***************************************************
 Function:   odb_removeTable
*****************************************************/

int32 cold_db_remove_table(cold_db_table_id_t tableId);


/*************************************************************
 Function: create a record to storage (append).
************************************************************/
int32 cold_db_add_record_to_storage(cold_db_table_id_t tableId,
                                          void *key_ptr,
                    		              uint16 key_len,
                                          void  *data_ptr,
                    		              uint16 data_len);

/*************************************************************
 Function: create  a record to table (append).
************************************************************/
int32 cold_db_add_record(cold_db_table_id_t tableId,
                                void* key_ptr,
                                uint16 key_len,
                                void* data_ptr,
                                uint16 data_len);

/*************************************************************
 Function: create or update a record matching key.
************************************************************/
int32 cold_db_save_record(cold_db_table_id_t tableId,
                                void* key_ptr,
                                uint16 key_len,
                                void* data_ptr,
                                uint16 data_len);

/*************************************************************
 Function: Records in memory are saved to persistent storage 
************************************************************/
int cold_db_fflush_to_storage(cold_db_table_id_t tableId);

/*************************************************************
/ Function: get the first record matching the key.
************************************************************/

int32 cold_db_get_record_first(cold_db_table_id_t tableId,
                                   void* key_ptr,
                                   uint16 key_len,
                                   void* recPtr,      // returned
                                   uint16 buffsize, // returned
                                   void* offset);


/*************************************************************
 Function: get the next record matching the key.
************************************************************/

int32 cold_db_get_record_next(cold_db_table_id_t tableId,
                                   void* key_ptr,
                                   uint16 key_len,
                                   void* recPtr,
                                   uint16 buffsize,
                                   void* offset);


/*************************************************************
 Function: get all records matching the key.

************************************************************/

int32 cold_db_get_records_by_key(cold_db_table_id_t tableId,
                                       void* key_ptr,
                                       uint16 key_len,
                                       uint16* num_rec_ptr);
int32 cold_db_get_records(cold_db_table_id_t tableId,
                            uint16* num_rec_ptr);


/****************************************************************
 Function: delete all records matching the key.
************************************************************/
int32 cold_db_delete_records(cold_db_table_id_t tableId,
                                 void* key_ptr,
                                 uint16 key_len,
                                 uint16* num_rec_ptr); 


char* get_logging_err_desc(int32 error);

#endif
