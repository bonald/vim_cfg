#include "proto.h"
#include "cold.h"
#include "cold_log.h"

#define COLD_MAX_TBL_HDR_LEN 64

typedef struct cold_db_buf_mng_s
{
    char* buffer;  
    uint16 buf_size;
    uint16 reserved;
} cold_db_buf_mng_t;

typedef struct cold_db_tbl_s
{
    FILE* stream;
    cold_db_tbl_header_t tbl_hdr;
    cold_db_buf_mng_t buf_mng;
    char achFileName[COLD_PATH_NAME_LEN + 1];
} cold_db_tbl_t;

static cold_db_tbl_t g_db_tbl[COLD_DB_MAX_TABLE_ID];
static char g_rec_str[COLD_MAX_REC_SIZE + 1] = {0};

char*
get_logging_err_desc(int error)
{ 
    static char error_desc[COLD_MAX_LINE_CHAR + 1] = {0};
    
    switch(error)
    {
        case COLD_DB_NO_DB  :
            return "No DB is found";  
        case COLD_DB_REC_EXIST:
            return "Record exists on the DB";
        case COLD_DB_NO_MORE_RESOURCE:
            return "Have not enough resource";
        case COLD_DB_REC_NOT_EXIST:
            return "No record is found";
        case COLD_DB_INVAILD_PARAM :
            return "Invalid parameter";
        case COLD_DB_INVAILD_REC_LEN:
            return "Invalid record length"; 
        default:
            sal_snprintf(error_desc, COLD_MAX_LINE_CHAR, "Error code:%d", error);
            return error_desc;
    }
}

char* 
code_format_tbl_hdr(cold_db_tbl_header_t* tbl_hdr)
{   
    static char str_record[COLD_MAX_TBL_HDR_LEN + 1]  = {0} ;
    if (!tbl_hdr)
    {
        return "";
    }               

    sal_snprintf(str_record, COLD_MAX_TBL_HDR_LEN, "%.5d%.5d%.5d%.5d%.5d%.5d%.3d%.3d",
            tbl_hdr->key_size, tbl_hdr->data_size, tbl_hdr->item_num,
            tbl_hdr->cur_item_num, tbl_hdr->max_item_num, tbl_hdr->sysup_item_index,
            tbl_hdr->handle_flag, tbl_hdr->db_init_count);

    sal_memcpy(&str_record[COLD_MAX_TBL_HDR_LEN - 2], STR_RETURN, 2);

    return str_record;
}

int 
cold_parser_tbl_hdr(char* tbl_hdr_desc, cold_db_tbl_header_t* tbl_hdr)
{
    int tmp1 = 0;
    int tmp2 = 0;
    int tmp3 = 0;
    int tmp4 = 0;
    int tmp5 = 0;
    int tmp6 = 0;
    int tmp7 = 0;
    int tmp8 = 0;

    if (!tbl_hdr_desc || !tbl_hdr)
    {
        return COLD_DB_INVAILD_PARAM ;
    }    

    sal_sscanf(tbl_hdr_desc, "%5d%5d%5d%5d%5d%5d%3d%3d",
           &tmp1, &tmp2, &tmp3, &tmp4, &tmp5, &tmp6, &tmp7, &tmp8);

    tbl_hdr->key_size         = (uint16)tmp1 ;          
    tbl_hdr->data_size        = (uint16)tmp2 ;        
    tbl_hdr->item_num         = (uint16)tmp3 ;        
    tbl_hdr->cur_item_num     = (uint16)tmp4 ;        
    tbl_hdr->max_item_num     = (uint16)tmp5 ;        
    tbl_hdr->sysup_item_index = (uint16)tmp6 ;        
    tbl_hdr->handle_flag      = (uint8)tmp7 ;          
    tbl_hdr->db_init_count    = (uint8)tmp8 ;
    
    return COLD_DB_SUCCESS;
}

/***************************************************
Function:    odb_initTable
 init table during process initialization.
 setup the database environment for access.
if the table you specified exists, recover it;
 if no table:  ODB_NEW returned .
*****************************************************/
int 
cold_db_init_table(cold_db_table_id_t tableId,
                       cold_db_tbl_header_t* tbl_hdr,
                       const char* p_db_dir_name,
                       const char* p_db_name,
                       char* buffer,
                       uint16 buf_size,
                       uint16* num_recs )
{
    char achFileName[COLD_PATH_NAME_LEN + 1] = {0};  
    cold_db_tbl_header_t db_tbl_hdr; 
    FILE* stream = NULL; 
    int ret = COLD_DB_SUCCESS;
    char str_tbl_hdr[COLD_MAX_TBL_HDR_LEN+1]  = {0};

    if(!tbl_hdr || !num_recs)
    {
        return COLD_DB_INVAILD_PARAM;
    }
    
    if (tableId >= COLD_DB_MAX_TABLE_ID)
    {
        return COLD_DB_NO_DB;
    }

    if ((tbl_hdr->key_size + tbl_hdr->data_size ) > COLD_MAX_REC_SIZE)
    {
        return COLD_DB_INVAILD_REC_LEN;
    }

    *num_recs = 0;
    sal_mkdir(p_db_dir_name , S_IRWXU | S_IRWXG | S_IRWXO) ;
   
    sal_memset(&db_tbl_hdr, 0, sizeof(cold_db_tbl_header_t));
    sal_snprintf(achFileName, COLD_PATH_NAME_LEN, "%s/%s", p_db_dir_name, p_db_name);
  
    stream = sal_fopen( achFileName, "r+" );
    if ( !stream )
    {
        ret = COLD_DB_NO_DB;
        stream = sal_fopen( achFileName, "w+" );
        if (stream)
        {
            chmod(achFileName, S_IRUSR | S_IWUSR | S_IXUSR | S_IXGRP | S_IXOTH);
            tbl_hdr->cur_item_num = 0;
            tbl_hdr->item_num = 0;
            tbl_hdr->sysup_item_index = 0;
            tbl_hdr->db_init_count      = 0;
            sal_memcpy(&g_db_tbl[tableId].tbl_hdr, tbl_hdr, sizeof(cold_db_tbl_header_t));
            sal_fwrite(code_format_tbl_hdr(tbl_hdr), COLD_MAX_TBL_HDR_LEN, 1, stream );  
            ret = COLD_DB_NEW  ; 
        }
      
    }
    else
    {
        chmod(achFileName, S_IRUSR | S_IWUSR | S_IXUSR | S_IXGRP | S_IXOTH);
        setbuffer(stream,buffer,buf_size);
        rewind(stream);
        if (sal_feof(stream))
        {
            tbl_hdr->cur_item_num = 0;
            tbl_hdr->item_num = 0;
            tbl_hdr->sysup_item_index = 0;
            tbl_hdr->db_init_count      = 0;
            sal_memcpy(&g_db_tbl[tableId].tbl_hdr, tbl_hdr, sizeof(cold_db_tbl_header_t));
            sal_fwrite(code_format_tbl_hdr(tbl_hdr), COLD_MAX_TBL_HDR_LEN, 1, stream );  
            ret = COLD_DB_NEW  ;  
          
        }
        else
        {
            sal_fread( str_tbl_hdr,  COLD_MAX_TBL_HDR_LEN, 1, stream );  
            cold_parser_tbl_hdr(str_tbl_hdr, &db_tbl_hdr);
           
            if (db_tbl_hdr.key_size != tbl_hdr->key_size
                || db_tbl_hdr.data_size != tbl_hdr->data_size
                || db_tbl_hdr.max_item_num != tbl_hdr->max_item_num)
            {
                ret = COLD_DB_INVAILD_REC_LEN;
                sal_fclose(stream);  
                
                stream = sal_fopen( achFileName, "w+" );          
                if (stream)
                { 
                    chmod(achFileName, S_IRUSR | S_IWUSR | S_IXUSR | S_IXGRP | S_IXOTH);
                    tbl_hdr->cur_item_num = 0;
                    tbl_hdr->item_num = 0;  
                    tbl_hdr->sysup_item_index = 0;
                    tbl_hdr->db_init_count      = db_tbl_hdr.db_init_count;
                    sal_memcpy(&g_db_tbl[tableId].tbl_hdr, tbl_hdr, sizeof(cold_db_tbl_header_t));
                    sal_fwrite(code_format_tbl_hdr(tbl_hdr), COLD_MAX_TBL_HDR_LEN, 1, stream );  
                    ret = COLD_DB_NEW; 
                }
            }
            else
            {
                sal_memcpy(&g_db_tbl[tableId].tbl_hdr, &db_tbl_hdr, sizeof(cold_db_tbl_header_t));
                sal_memcpy(tbl_hdr, &db_tbl_hdr, sizeof(cold_db_tbl_header_t));                 
            }
        }
    }
    
    if (ret >= 0)
    {
        g_db_tbl[tableId].tbl_hdr.db_init_count++;
        g_db_tbl[tableId].stream = stream; 
        g_db_tbl[tableId].buf_mng.buffer = buffer;
        g_db_tbl[tableId].buf_mng.buf_size = buf_size;    
        sal_strcpy(g_db_tbl[tableId].achFileName,achFileName);
        *num_recs = g_db_tbl[tableId].tbl_hdr.item_num;
        sal_fflush(stream);
   }
    return ret;

}

/***************************************************
 Function:   odb_removeTable
*****************************************************/
int 
cold_db_remove_table(cold_db_table_id_t tableId)
{
    FILE* stream = NULL; 
    uint32 offset;  
    uint8  *pData = NULL;
    cold_db_tbl_header_t *tbl_hdr;
    uint16 item_size;
    
   if (tableId >= COLD_DB_MAX_TABLE_ID  )
    {
        return COLD_DB_NO_DB;
    }
    stream = g_db_tbl[tableId].stream;
     
    if (!stream)
    {
        return COLD_DB_REC_NOT_EXIST;
    }

    tbl_hdr = &g_db_tbl[tableId].tbl_hdr;
  
    if (!tbl_hdr->item_num)
    {
        return COLD_DB_SUCCESS;
    }
    
    item_size = tbl_hdr->key_size + tbl_hdr->data_size;
    pData = (uint8 *)XCALLOC(MEM_COLD_LOGGING_BUF, item_size * sizeof(uint8));

    if (!pData)
    {
        return COLD_DB_INVAILD_PARAM;
    }
    offset = sizeof( cold_db_tbl_header_t);
    sal_fseek(stream, offset, SEEK_SET);

    sal_memset(pData, 0, item_size * sizeof(uint8));
    sal_fwrite(pData, item_size, tbl_hdr->item_num, stream);

    if (pData)
    {
        XFREE(MEM_COLD_LOGGING_BUF, pData);
        pData = NULL;
    }
    rewind(stream); 
    return COLD_DB_SUCCESS;
}

/*************************************************************
 Function: create a record to table (append).

************************************************************/
static int 
_cold_db_add_record(cold_db_table_id_t tableId,
                            void* key_ptr,
                            uint16 key_len,
                            void* recordPtr,
                            uint16 recordLen,
                            FILE* stream)
{
    uint16 item_size = 0;
    uint16 input_item_size = 0;
    uint32 offset = 0;
    cold_db_tbl_header_t* tbl_hdr = NULL;

    if (tableId >= COLD_DB_MAX_TABLE_ID  )
    {
        return COLD_DB_NO_DB;
    }
    tbl_hdr     = &g_db_tbl[tableId].tbl_hdr;
    stream      = g_db_tbl[tableId].stream;

    if (!stream)
    {
       return COLD_DB_REC_NOT_EXIST;
    }
    
    input_item_size = key_len + recordLen;   
    if( tbl_hdr->key_size != key_len || input_item_size > COLD_MAX_REC_SIZE  )
    { 
      return COLD_DB_INVAILD_REC_LEN;
    }

     
    if(tbl_hdr->item_num >= tbl_hdr->max_item_num)
    {
        switch(tbl_hdr->handle_flag)
        {
            case COLD_OF_COVER_REC:  
                if (tbl_hdr->cur_item_num == tbl_hdr->item_num)
                {
                    tbl_hdr->cur_item_num = 0;
                }
                tbl_hdr->item_num--;
                break;
            case COLD_OF_RW_NEW_RECORD:
                if (stream)
                {
                    sal_fclose(stream);            
                }
                stream = sal_fopen( g_db_tbl[tableId].achFileName, "w+");
                if (stream)
                {
                    g_db_tbl[tableId].stream  = stream;
                    tbl_hdr->cur_item_num = 0;
                    tbl_hdr->item_num = 0;         
                    sal_memcpy(&g_db_tbl[tableId].tbl_hdr, tbl_hdr, sizeof(cold_db_tbl_header_t));
                    rewind(stream);
                    sal_fwrite(code_format_tbl_hdr(tbl_hdr), COLD_MAX_TBL_HDR_LEN, 1, stream ); 
                }
                else
                {
                   g_db_tbl[tableId].stream = NULL;
                   return COLD_DB_NO_DB;
                }
                break;             
            case COLD_OF_STOP_REC:
            default:
                return COLD_DB_SUCCESS;
        }
    }
    
    item_size = tbl_hdr->key_size + tbl_hdr->data_size;  
    offset = COLD_MAX_TBL_HDR_LEN + item_size * tbl_hdr->cur_item_num;
    sal_fseek(stream, offset, SEEK_SET);     
    sal_memset(g_rec_str,0,sizeof(g_rec_str));
    sal_memcpy(&g_rec_str[0],key_ptr,key_len);
    sal_memcpy(&g_rec_str[key_len],recordPtr,recordLen);
    /*modify by hezhcihuan for fixing bug 6513 2008/08/21*/
    sal_fwrite(g_rec_str, item_size, 1, stream);
    rewind(stream);
  
    tbl_hdr->cur_item_num ++;
    tbl_hdr->item_num++; 
    sal_fwrite(code_format_tbl_hdr(tbl_hdr), COLD_MAX_TBL_HDR_LEN, 1, stream); 
    
    return COLD_DB_SUCCESS;    
    
}

int 
cold_db_add_record(cold_db_table_id_t tableId,
                            void* key_ptr,
                            uint16 key_len,
                            void* recordPtr,
                            uint16 recordLen)
{
    FILE *stream = NULL;
    return _cold_db_add_record(tableId, key_ptr, key_len, recordPtr, recordLen, stream);
}

/*************************************************************
 Function: create a record to storage (append).
************************************************************/
int 
cold_db_add_record_to_storage(cold_db_table_id_t tableId,
                                       void* key_ptr,
                                       uint16 key_len,
                                       void* recordPtr,
                                       uint16 recordLen)
{
    FILE* stream = NULL;
    int ret;
    
    ret = _cold_db_add_record(tableId, key_ptr, key_len, recordPtr, recordLen, stream);
    sal_fflush(stream);
    
    return ret; 
}

/*************************************************************
 Function: create or update a record matching key.
************************************************************/
int cold_db_save_record(
                            cold_db_table_id_t tableId,	
                            void* key_ptr,
                            uint16 key_len,
                            void* recordPtr,
                            uint16 recordLen)
{
    uint16 item_size;
    uint16 input_item_size;
    uint32 offset;
    char str_record[COLD_MAX_REC_SIZE]  = {0};
    cold_db_tbl_header_t *tbl_hdr;
    FILE* stream = NULL;
    uint16 wloop;
   
    if (tableId >= COLD_DB_MAX_TABLE_ID)
    {
        return COLD_DB_NO_DB;
    }
    tbl_hdr = &g_db_tbl[tableId].tbl_hdr;
    stream = g_db_tbl[tableId].stream;
     
    if (!stream)
    {
        return COLD_DB_REC_NOT_EXIST;
    }

    input_item_size = key_len + recordLen;   
    if( tbl_hdr->key_size != key_len || input_item_size > COLD_MAX_REC_SIZE  )
    { 
        return COLD_DB_INVAILD_REC_LEN;
    }

    item_size = tbl_hdr->key_size + tbl_hdr->data_size;  
   
    for(wloop = 0; wloop < tbl_hdr->item_num ;wloop++ )
    {
      sal_fread( str_record, item_size, 1, stream );
      if(!sal_memcmp(str_record,key_ptr,key_len))
      {
         offset =  COLD_MAX_TBL_HDR_LEN + item_size * wloop;
         sal_fseek(stream, offset, SEEK_SET);
         sal_fwrite( key_ptr, key_len, 1, stream);
         sal_fwrite( recordPtr, recordLen, 1, stream);     
         return COLD_DB_SUCCESS;
      }
    }
    
    if (tbl_hdr->item_num >= tbl_hdr->max_item_num)
    {
        switch(tbl_hdr->handle_flag)
        {
            case COLD_OF_COVER_REC: 
                if(tbl_hdr->cur_item_num == tbl_hdr->item_num )
                {
                    tbl_hdr->cur_item_num = 0;
                }
                tbl_hdr->item_num--;
                break;
            case COLD_OF_RW_NEW_RECORD:
                if (stream)
                {
                    sal_fclose(stream);            
                }
           
                stream = sal_fopen(  g_db_tbl[tableId].achFileName, "w+" );
                if (stream)
                {
                    g_db_tbl[tableId].stream  = stream;
                    tbl_hdr->cur_item_num = 0;
                    tbl_hdr->item_num = 0;         
                    sal_memcpy(&g_db_tbl[tableId].tbl_hdr, tbl_hdr, sizeof(cold_db_tbl_header_t));

                    rewind(stream);
                    sal_fwrite(code_format_tbl_hdr(tbl_hdr), COLD_MAX_TBL_HDR_LEN, 1, stream ); 
                }
                else
                {
                    g_db_tbl[tableId].stream = NULL;
                    return COLD_DB_NO_DB;
                }
                break;             
            case COLD_OF_STOP_REC:
            default:
                return COLD_DB_SUCCESS;
        }
    }

    offset = COLD_MAX_TBL_HDR_LEN + item_size * tbl_hdr->cur_item_num;
    sal_fseek(stream, offset, SEEK_SET);     

    sal_memset(g_rec_str,0,sizeof(g_rec_str));
    sal_memcpy(&g_rec_str[0],key_ptr,key_len);
    sal_memcpy(&g_rec_str[key_len],recordPtr,recordLen);            
    sal_fwrite( g_rec_str, item_size, 1, stream); /*modify by hezhcihuan for fixing bug 6513 2008/08/21*/

    rewind(stream);
    tbl_hdr->cur_item_num ++;
    tbl_hdr->item_num++;
    sal_fwrite(code_format_tbl_hdr(tbl_hdr), COLD_MAX_TBL_HDR_LEN, 1, stream ); 
    
    return COLD_DB_SUCCESS; 
}

/*************************************************************
 Function: Records in memory are saved to persistent storage 
************************************************************/
int cold_db_fflush_to_storage(cold_db_table_id_t tableId)
{
    FILE* stream = NULL; 
    if (tableId >= COLD_DB_MAX_TABLE_ID  )
    {
        return COLD_DB_NO_DB;
    }
     
    stream = g_db_tbl[tableId].stream;
     
    if (!stream)
    {
        return COLD_DB_REC_NOT_EXIST;
    }
    sal_fflush(stream);
    
    return COLD_DB_SUCCESS;
}
/*************************************************************
/ Function: get the first record matching the key.
************************************************************/
static int 
_cold_db_get_record(
                            cold_db_table_id_t tableId,
                            void* key_ptr,
                            uint16 key_len,
                            void* data_ptr,
                            uint16 buffsize, 
                            void* offset,
                            bool is_first)
{
    uint16 item_size;  
    uint32 dwoffset;
    uint16 wloop;
    char str_record[COLD_MAX_REC_SIZE] = {0};
    FILE* stream = NULL;
    cold_db_tbl_header_t *tbl_hdr;

    if (tableId >= COLD_DB_MAX_TABLE_ID  )
    {
        return COLD_DB_NO_DB;
    }
    tbl_hdr = &g_db_tbl[tableId].tbl_hdr;
    stream = g_db_tbl[tableId].stream;
     
    if (!stream)
    {
        return COLD_DB_REC_NOT_EXIST;
    }
  
    if(key_len != tbl_hdr->key_size)
    { 
        return COLD_DB_INVAILD_REC_LEN;
    }

    item_size = tbl_hdr->key_size + tbl_hdr->data_size;
    dwoffset = *(uint32 *)offset;
    
    if(dwoffset == 0 || dwoffset< COLD_MAX_TBL_HDR_LEN)
    {
        dwoffset = COLD_MAX_TBL_HDR_LEN;
    }

    if (item_size > buffsize)
    {
        return COLD_DB_INVAILD_PARAM;
    }

    if(is_first)
    {
        sal_fseek(stream, dwoffset, SEEK_SET);
    }
  
    for(wloop = 0; wloop < tbl_hdr->item_num ;wloop++ )
    {
        sal_fread( str_record, item_size, 1, stream );
        if(!sal_memcmp(str_record,key_ptr,key_len))
        {
            sal_memcpy(data_ptr,&str_record[key_len],tbl_hdr->data_size);
            *(uint32 *)offset = sal_ftell(stream);      
            return COLD_DB_SUCCESS;
        }
    }
          
    return COLD_DB_REC_NOT_EXIST;
}



int 
cold_db_get_record_first(
                                cold_db_table_id_t tableId,
                                void* key_ptr,
                                uint16 key_len,
                                void* data_ptr,
                                uint16 buffsize, 
                                void *offset)
{
    return _cold_db_get_record(tableId, key_ptr, key_len, data_ptr, 
                                buffsize, offset, TRUE);
}

/*************************************************************
 Function: get the next record matching the key.
************************************************************/

int 
cold_db_get_record_next(
                               cold_db_table_id_t tableId,
                               void* key_ptr,
                               uint16 key_len,
                               void* data_ptr,
                               uint16 buffsize,
                               void* offset )
{
    return _cold_db_get_record(tableId, key_ptr, key_len, data_ptr, 
                                buffsize, offset, FALSE);
}


/*************************************************************
 Function: get all records matching the key.

************************************************************/

int 
cold_db_get_records_by_key(
                                    cold_db_table_id_t tableId,
                                    void* key_ptr,
                                    uint16 key_len,                
                                    uint16* num_recs)
{

    uint16 item_size;  
    uint32 offset;
    uint16 wloop;
    char str_record[COLD_MAX_REC_SIZE]  =  { 0 };
    FILE* stream = NULL;
    cold_db_tbl_header_t *tbl_hdr;

    if (tableId >= COLD_DB_MAX_TABLE_ID  )
    {
        return COLD_DB_NO_DB;
    }
    tbl_hdr = &g_db_tbl[tableId].tbl_hdr;
    stream = g_db_tbl[tableId].stream;
     
    if (!stream)
    {
        return COLD_DB_REC_NOT_EXIST;
    }  
  
    if(key_len != tbl_hdr->key_size)
    { 
        return COLD_DB_INVAILD_REC_LEN;
    }
    
    item_size = tbl_hdr->key_size + tbl_hdr->data_size;
    
    offset = COLD_MAX_TBL_HDR_LEN;
    sal_fseek(stream, offset, SEEK_SET);
    *num_recs = 0;

    for (wloop = 0; wloop < tbl_hdr->item_num ; wloop++ )
    {
        sal_fread( str_record, item_size, 1, stream );
        if (!sal_memcmp(str_record, key_ptr, key_len))
        {
            (*num_recs)++;
        }
    }
    
    return COLD_DB_SUCCESS;
}

int 
cold_db_get_records(cold_db_table_id_t tableId, uint16 *num_recs) 
{
    cold_db_tbl_header_t *tbl_hdr;   
 
    if (tableId >= COLD_DB_MAX_TABLE_ID)
    {
        return COLD_DB_NO_DB;
    }
    tbl_hdr = &g_db_tbl[tableId].tbl_hdr; 
    *num_recs = tbl_hdr->item_num;
    
    return COLD_DB_SUCCESS;     
}
/****************************************************************
 Function: delete all records matching the key.
************************************************************/
int 
cold_db_delete_records(
                         cold_db_table_id_t tableId,
                         void* key_ptr,
                         uint16 key_len,
                         uint16* num_recs)
{
    uint16 item_size;  
    uint32 offset;
    uint16 wloop;
    char str_record[COLD_MAX_REC_SIZE]  =  { 0 };
    FILE* stream = NULL;
    cold_db_tbl_header_t* tbl_hdr;
    
    if (tableId >= COLD_DB_MAX_TABLE_ID  )
    {
        return COLD_DB_NO_DB;
    }
    tbl_hdr = &g_db_tbl[tableId].tbl_hdr;
    stream = g_db_tbl[tableId].stream;
     
    if (!stream)
    {
        return COLD_DB_REC_NOT_EXIST;
    }

       
    if ( tbl_hdr->key_size != key_len  )
    { 
        return COLD_DB_REC_NOT_EXIST;
    }
    item_size = tbl_hdr->key_size + tbl_hdr->data_size;
    
    offset = COLD_MAX_TBL_HDR_LEN;
    sal_fseek(stream, offset, SEEK_SET);
    *num_recs = 0;
    for (wloop = 0; wloop < tbl_hdr->item_num ; wloop++ )
    {
        sal_fread( str_record, item_size, 1, stream );
        if (!sal_memcmp(str_record, key_ptr, key_len))
        {
            (*num_recs)++;
        }
    }
    
    return COLD_DB_SUCCESS;
}

