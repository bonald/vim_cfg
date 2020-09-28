from cdb_const import*

AGENT = None

# Tables from sub modules
IF_TABLE = None
IFX_TABLE = None
INTERFACE_TABLE = None

def _stats_line_to_obj_simple(line):
    obj = []
    
    for i in line.split('/'):
        if i:
            obj.append(i)
    
    return obj 

def _stats_get_arr_simple(lines):
    arr = []
    for line in lines:
        tmp = _stats_line_to_obj_simple(line)
        if tmp:
            arr.append(tmp)
    return arr

def stats_get_arr_simple(cmd1):
    """send cmd and convert response to dict array, parse single layer
    """
    res = cmd(cmd1)
    arr = _stats_get_arr_simple(res)
    return arr

# Thread that retrieving interface statistics and flow statistics
def statsMonitor():
    global IF_TABLE, IFX_TABLE, INTERFACE_TABLE, FLOW_IF_STATS_TABLE
    # wait until all sub modules are initialized
    while (IF_TABLE == None or IFX_TABLE == None or INTERFACE_TABLE == None or FLOW_IF_STATS_TABLE == None or FLOW_IF_EGRESS_STATS_TABLE == None):
        IF_TABLE = GLB_TABLE_DICT["iftable"]
        IFX_TABLE = GLB_TABLE_DICT["ifxtable"]
        INTERFACE_TABLE = GLB_TABLE_DICT["interface"]
        FLOW_IF_STATS_TABLE = GLB_TABLE_DICT["flow_stats"]
        FLOW_IF_EGRESS_STATS_TABLE = GLB_TABLE_DICT["flow_egress_stats"]
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
                if stats_db.get('pkt_rx_crc'):
                    old_row.setRowCell(IF_TBL_MEM_IN_DISCARDS, AGENT.Counter32(long(stats_db.get('pkt_rx_crc'))))
                if stats_db.get('pkt_rx_bad'):
                    old_row.setRowCell(IF_TBL_MEM_IN_ERRORS,   AGENT.Counter32(long(stats_db.get('pkt_rx_bad'))))
                if stats_db.get('pkt_tx_err'):
                    old_row.setRowCell(IF_TBL_MEM_OUT_ERRORS,  AGENT.Counter32(long(stats_db.get('pkt_tx_err'))))

            old_row = IFX_TABLE.addRow([AGENT.Integer32(ifindex)])
            if old_row:
                if stats_db.get('pkt_rx_mc'):
                    old_row.setRowCell(IFX_TBL_MEM_IN_MULTICAST_PKTS,     AGENT.Counter32(long(stats_db.get('pkt_rx_mc'))))
                if stats_db.get('pkt_rx_bc'):
                    old_row.setRowCell(IFX_TBL_MEM_IN_BROADCAST_PKTS,     AGENT.Counter32(long(stats_db.get('pkt_rx_bc'))))
                if stats_db.get('pkt_tx_mc'):
                    old_row.setRowCell(IFX_TBL_MEM_OUT_MULTICAST_PKTS,    AGENT.Counter32(long(stats_db.get('pkt_tx_mc'))))
                if stats_db.get('pkt_tx_bc'):
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

        time.sleep(2)
        
        igs_flow_stats_db = stats_get_arr_simple("cdbctl read/cdb/tap/show_tap_counter/ingress")

        GLB_INTERFACE_MUTEX.acquire()
        GLB_FLOW_STATS_MUTEX.acquire()

        for stats_db in igs_flow_stats_db:
            if (6 != len(stats_db)):
            	continue
            old_row = FLOW_IF_STATS_TABLE.addRow([AGENT.Integer32(int(stats_db[1])),
                                                  AGENT.DisplayString(stats_db[2]),
                                                  AGENT.Integer32(int(stats_db[3]))])
            if old_row:
                old_row.setRowCell(FLOW_IF_STATS_TBL_MEM_MATCH_PKTS,  AGENT.Counter64(long(stats_db[5])))
                old_row.setRowCell(FLOW_IF_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(long(stats_db[4])))
            #print stats_db

        GLB_FLOW_STATS_MUTEX.release()
        GLB_INTERFACE_MUTEX.release()

        time.sleep(2)

        igs_flow_stats_db = stats_get_arr_simple("cdbctl read/cdb/tap/show_tap_counter/egress")

        GLB_INTERFACE_MUTEX.acquire()
        GLB_FLOW_STATS_MUTEX.acquire()

        for stats_db in igs_flow_stats_db:
            if (5 != len(stats_db)):
            	continue
            old_row = FLOW_IF_EGRESS_STATS_TABLE.addRow([AGENT.Integer32(int(stats_db[1])),
                                                  AGENT.Integer32(int(stats_db[2]))])
            if old_row:
                old_row.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_PKTS,  AGENT.Counter64(long(stats_db[4])))
                old_row.setRowCell(FLOW_IF_EGRESS_STATS_TBL_MEM_MATCH_BYTES, AGENT.Counter64(long(stats_db[3])))
            #print stats_db

        GLB_FLOW_STATS_MUTEX.release()
        GLB_INTERFACE_MUTEX.release()
        
        time.sleep(1)


def init(agent_obj, subscriber):
    global AGENT
    AGENT = agent_obj

    stats_polling_thread = threading.Thread(target = statsMonitor, name = "statsMonitor")
    stats_polling_thread.daemon = True
    stats_polling_thread.start()
