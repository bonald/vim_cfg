from cdb_const import*

AGENT = None

# Tables from sub modules
IF_TABLE = None
IFX_TABLE = None
INTERFACE_TABLE = None

# Thread that retrieving interface statistics and flow statistics
def statsMonitor():
    global IF_TABLE, IFX_TABLE, INTERFACE_TABLE
    # wait until all sub modules are initialized
    while (IF_TABLE == None or IFX_TABLE == None or INTERFACE_TABLE == None):
        IF_TABLE = GLB_TABLE_DICT["iftable"]
        IFX_TABLE = GLB_TABLE_DICT["ifxtable"]
        INTERFACE_TABLE = GLB_TABLE_DICT["interface"]
        time.sleep(1)

    while(1):
        szmsg = execute_cmd('cdbctl show/cdb/sys/mem_summary')
        execute_cmd("cdbctl update/cdb/sys/update_if_stats/")
        time.sleep(1)
        
        all_stats_db = get_arr_simple("cdbctl read/cdb/sys/show_mib_stats")
        
       	GLB_INTERFACE_MUTEX.acquire()
        # retrieve all interface statistics
        for stats_db in all_stats_db:
            # print stats_db
            ifindex = long(stats_db.get('ifindex'))

            # IF_TABLE
            old_row = IF_TABLE.addRow([AGENT.Integer32(ifindex)])
            if old_row:
                if stats_db.get('oct_rx'):
                    if long(stats_db.get('oct_rx')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_IN_OCTECTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_IN_OCTECTS, AGENT.Counter32(long(stats_db.get('oct_rx'))))
                if stats_db.get('pkt_rx_uc'):
                    if long(stats_db.get('pkt_rx_uc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_IN_UCASTPKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_IN_UCASTPKTS, AGENT.Counter32(long(stats_db.get('pkt_rx_uc'))))
                if stats_db.get('pkt_rx_mc') and stats_db.get('pkt_rx_bc'):
                    
                    if long(stats_db.get('pkt_rx_mc'))+long(stats_db.get('pkt_rx_bc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_IN_NUCASTPKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_IN_NUCASTPKTS, AGENT.Counter32(long(stats_db.get('pkt_rx_mc'))+long(stats_db.get('pkt_rx_bc'))))
                if stats_db.get('pkt_rx_crc'):
                    
                    if long(stats_db.get('pkt_rx_crc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_IN_DISCARDS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_IN_DISCARDS, AGENT.Counter32(long(stats_db.get('pkt_rx_crc'))))        
                if stats_db.get('oct_tx'):
                    
                    if long(stats_db.get('oct_tx')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_OUT_OCTECTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_OUT_OCTECTS, AGENT.Counter32(long(stats_db.get('oct_tx'))))
                if stats_db.get('pkt_tx_uc'):
                    
                    if long(stats_db.get('pkt_tx_uc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_OUT_UCAST_PKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_OUT_UCAST_PKTS, AGENT.Counter32(long(stats_db.get('pkt_tx_uc'))))
                if stats_db.get('pkt_tx_mc') and stats_db.get('pkt_tx_bc'):
                    
                    if long(stats_db.get('pkt_tx_mc'))+long(stats_db.get('pkt_tx_bc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_OUT_NUCAST_PKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_OUT_NUCAST_PKTS, AGENT.Counter32(long(stats_db.get('pkt_tx_mc'))+long(stats_db.get('pkt_tx_bc'))))
                if stats_db.get('pkt_rx_bad'):
                    
                    if long(stats_db.get('pkt_rx_bad')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_IN_ERRORS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_IN_ERRORS,   AGENT.Counter32(long(stats_db.get('pkt_rx_bad'))))
                if stats_db.get('pkt_tx_err'):
                    
                    if long(stats_db.get('pkt_tx_err')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IF_TBL_MEM_OUT_ERRORS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IF_TBL_MEM_OUT_ERRORS,  AGENT.Counter32(long(stats_db.get('pkt_tx_err'))))

            # IFX_TABLE
            old_row = IFX_TABLE.addRow([AGENT.Integer32(ifindex)])
            if old_row:
                if stats_db.get('pkt_rx_mc'):
                    
                    if long(stats_db.get('pkt_rx_mc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IFX_TBL_MEM_IN_MULTICAST_PKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IFX_TBL_MEM_IN_MULTICAST_PKTS,     AGENT.Counter32(long(stats_db.get('pkt_rx_mc'))))
                if stats_db.get('pkt_rx_bc'):
                    
                    if long(stats_db.get('pkt_rx_bc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IFX_TBL_MEM_IN_BROADCAST_PKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IFX_TBL_MEM_IN_BROADCAST_PKTS,     AGENT.Counter32(long(stats_db.get('pkt_rx_bc'))))
                if stats_db.get('pkt_tx_mc'):
                    
                    if long(stats_db.get('pkt_tx_mc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IFX_TBL_MEM_OUT_MULTICAST_PKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IFX_TBL_MEM_OUT_MULTICAST_PKTS,    AGENT.Counter32(long(stats_db.get('pkt_tx_mc'))))
                if stats_db.get('pkt_tx_bc'):
                    
                    if long(stats_db.get('pkt_tx_bc')) > GLB_COUNTER32_MAX:
                        old_row.setRowCell(IFX_TBL_MEM_OUT_BROADCAST_PKTS, AGENT.Counter32(GLB_COUNTER32_MAX))
                    else:
                        old_row.setRowCell(IFX_TBL_MEM_OUT_BROADCAST_PKTS,    AGENT.Counter32(long(stats_db.get('pkt_tx_bc'))))
                if stats_db.get('oct_rx'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_IN_OCTETS,          AGENT.Counter64(long(stats_db.get('oct_rx'))))
                if stats_db.get('pkt_rx_uc'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_IN_UCAST_PKTS,      AGENT.Counter64(long(stats_db.get('pkt_rx_uc'))))
                if stats_db.get('pkt_rx_mc'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_IN_MULTICAST_PKTS,  AGENT.Counter64(long(stats_db.get('pkt_rx_mc'))))
                if stats_db.get('pkt_rx_bc'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_IN_BROADCAST_PKTS,  AGENT.Counter64(long(stats_db.get('pkt_rx_bc'))))
                if stats_db.get('oct_tx'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_OUT_OCTETS,         AGENT.Counter64(long(stats_db.get('oct_tx'))))
                if stats_db.get('pkt_tx_uc'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_OUT_UCAST_PKTS,     AGENT.Counter64(long(stats_db.get('pkt_tx_uc'))))
                if stats_db.get('pkt_tx_mc'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_OUT_MULTICAST_PKTS, AGENT.Counter64(long(stats_db.get('pkt_tx_mc'))))
                if stats_db.get('pkt_tx_bc'):
                    old_row.setRowCell(IFX_TBL_MEM_HC_OUT_BROADCAST_PKTS, AGENT.Counter64(long(stats_db.get('pkt_tx_bc'))))

            old_row = INTERFACE_TABLE.addRow([AGENT.Integer32(ifindex)])
            if old_row:
                if stats_db.get('oct_rx_rate'):
                    old_row.setRowCell(TBL_INTERFACE_5MIN_IN_BITS_RATE,  AGENT.Counter64(long(stats_db.get('oct_rx_rate'))))
                if stats_db.get('pkt_rx_rate'):
                    old_row.setRowCell(TBL_INTERFACE_5MIN_IN_PKTS_RATE,  AGENT.Counter64(long(stats_db.get('pkt_rx_rate'))))
                if stats_db.get('oct_tx_rate'):
                    old_row.setRowCell(TBL_INTERFACE_5MIN_OUT_BITS_RATE, AGENT.Counter64(long(stats_db.get('oct_tx_rate'))))
                if stats_db.get('pkt_tx_rate'):
                    old_row.setRowCell(TBL_INTERFACE_5MIN_OUT_PKTS_RATE, AGENT.Counter64(long(stats_db.get('pkt_tx_rate'))))

       	GLB_INTERFACE_MUTEX.release()
        time.sleep(4)

def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj

    stats_polling_thread = threading.Thread(target = statsMonitor, name = "statsMonitor")
    stats_polling_thread.daemon = True
    stats_polling_thread.start()
