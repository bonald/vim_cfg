/* **
 * Example of Running VeriPHY Cable Diagnostics Suite on an external CPU. 
 * Some of the Vitesse PHYs such as VSC8558, VSC8658 have internal triggers
 * to run VeriPHY on the internal V-Core of the part. 
 * Refer to the Device Data Sheet for triggering VeriPHY internally.
 * the file support VSC8538/58/8658
 * support VSC8512/8504  bug25277
 * Not support VSC8211.
 * **/

#include "sal_common.h"
#include "drv_specific.h"
#include "drv_debug.h"
#include "mdio.h"
#include "glb_phy_define.h"
#include "glb_hw_define.h"
#include "phy_veriphy.h"

static int32 _vtss8xx8_veriphy_phy_write(phy_handle_t* phy_handle, const uint32 reg, uint16 value);
void vtss8xx8_veriphy(void *p_data);
DRV_DEFINE_TASK_ENCAP(vtss8xx8_veriphy)

#define Vtss8xx8VeriphySmiWrite(phy, reg, val) _vtss8xx8_veriphy_phy_write(phy, reg, val)
 
#define Vtss8xx8VeriphyExtMiiWrite(phy, reg, val) Vtss8xx8VeriphySmiWrite(phy, 31, VTSS8XX8_PHY_PAGE_EXTENDED); Vtss8xx8VeriphySmiWrite(phy, reg, val)

static int32 _vtss8xx8_veriphy_phy_read (phy_handle_t* phy_handle,
                       const uint32    reg,
                       uint16* value)
{
    uint16 page;
    phyreg_param_t phyreg_para;

    /* Determine page */
    page = (reg>>5);

    if (page) {
        /*Page Selection register*/
        phyreg_para.dat.regaddr8 = 0x1F;
        VERIPHY_RC(phy_handle->reg_write(phy_handle, &phyreg_para, page));
    }
    phyreg_para.dat.regaddr8 = reg & 0x1F;
    VERIPHY_RC( phy_handle->reg_read( phy_handle, &phyreg_para, value ) );
    if (page) {
        /*Page Selection register*/
        phyreg_para.dat.regaddr8 = 0x1F;
        VERIPHY_RC(phy_handle->reg_write(phy_handle, &phyreg_para, 0));
    }

    return RESULT_OK;
}

static int32 _vtss8xx8_veriphy_phy_write(phy_handle_t* phy_handle,
                       const uint32    reg,
                       uint16 value)
{
    uint16 page;
    phyreg_param_t phyreg_para;

    /* Determine page */
    page = (reg>>5);

    if (page) {
        /*Page Selection register*/
        phyreg_para.dat.regaddr8 = 0x1F;
        VERIPHY_RC(phy_handle->reg_write(phy_handle, &phyreg_para, page));
    }
    phyreg_para.dat.regaddr8 = reg & 0x1F;
    VERIPHY_RC(phy_handle->reg_write(phy_handle, &phyreg_para, value));
    if (page) {
        /*Page Selection register*/
        phyreg_para.dat.regaddr8 = 0x1F;
        VERIPHY_RC(phy_handle->reg_write(phy_handle, &phyreg_para, 0));
    }

    return RESULT_OK;
}

static int32 _vtss8xx8_veriphy_phy_writemasked(phy_handle_t* phy_handle,
                             const uint32      reg,
                             const uint16         value,
                             const uint16         mask)
{
    uint16 regval;
    uint16 page;
    phyreg_param_t phyreg_para;

    /* Determine page */
    page = (reg>>5);

    if (page) {
        phyreg_para.dat.regaddr8 = 0x1F;
        VERIPHY_RC( phy_handle->reg_write(phy_handle, &phyreg_para, page) );
    }
    phyreg_para.dat.regaddr8 = reg & 0x1F;
    VERIPHY_RC( phy_handle->reg_read(phy_handle, &phyreg_para, &regval) );
    regval &= ~mask;
    regval |= value & mask;
    VERIPHY_RC( phy_handle->reg_write(phy_handle, &phyreg_para, regval) );
    if (page) {
        phyreg_para.dat.regaddr8 = 0x1F;
        VERIPHY_RC( phy_handle->reg_write(phy_handle, &phyreg_para, 0) );
    }

    return RESULT_OK;
}

static int32 _vtss8xx8_veriphy_phy_page_std(phy_handle_t* phy_handle)
{
    return _vtss8xx8_veriphy_phy_write(phy_handle, 31, VTSS8XX8_PHY_PAGE_STANDARD);
}

static int32 _vtss8xx8_veriphy_phy_page_ext(phy_handle_t* phy_handle)
{
    return _vtss8xx8_veriphy_phy_write(phy_handle, 31, VTSS8XX8_PHY_PAGE_EXTENDED);
}

static int32 _vtss8xx8_veriphy_phy_page_test(phy_handle_t* phy_handle)
{
    return _vtss8xx8_veriphy_phy_write(phy_handle, 31, VTSS8XX8_PHY_PAGE_TEST);
}

static int32 _vtss8xx8_veriphy_phy_page_tr(phy_handle_t* phy_handle)
{
    return _vtss8xx8_veriphy_phy_write(phy_handle, 31, VTSS8XX8_PHY_PAGE_TR);
}

static int32 _vtss8xx8_veriphy_SmiRead(phy_handle_t* phy_handle, int32 reg)
{
    uint16 rv;
    _vtss8xx8_veriphy_phy_read(phy_handle, reg, &rv);
    
    return rv;
}

/* Local functions */
static int32 _vtss8xx8_veriphy_ExtMiiReadBits(phy_handle_t* phy_handle, int8 reg, int8 msb, int8 lsb)
{
    int32 x;

    if(NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_ExtMiiReadBits: invalid handle");
        return RESULT_ERROR;
    }

    _vtss8xx8_veriphy_phy_page_ext(phy_handle);
    x = _vtss8xx8_veriphy_SmiRead(phy_handle, reg);
    if (msb < 15)
        x &= (1 << (msb + 1)) - 1;
    x = (int32)((uint32) x >> lsb);

    return x;
}

static int32 _vtss8xx8_veriphy_MiiReadBits(phy_handle_t* phy_handle, int8 reg, int8 msb, int8 lsb)
{
    int32 x;

    if(NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_MiiReadBits: invalid handle");
        return RESULT_ERROR;
    }

    _vtss8xx8_veriphy_phy_page_std(phy_handle);
    x = _vtss8xx8_veriphy_SmiRead(phy_handle, reg);
    if (msb < 15)
        x &= (1 << (msb + 1)) - 1;
    x = (int32)((uint32) x >> lsb);

    return x;
}

static void _vtss8xx8_veriphy_TpWriteBit(phy_handle_t* phy_handle, int8 TpReg, int8 bitNum, int8 value)
{
    int16 val;

    if(NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_TpWriteBit: invalid handle");
        return;
    }

    _vtss8xx8_veriphy_phy_page_test(phy_handle);
    val = _vtss8xx8_veriphy_SmiRead(phy_handle, TpReg);
    if (value)
        val = val | (1 << bitNum);
    else
        val = val & ~(1 << bitNum);
    _vtss8xx8_veriphy_phy_write(phy_handle, TpReg, val);
}

static int32 _vtss8xx8_veriphy_tr_raw_read(phy_handle_t* phy_handle, uint16 TrSubchanNodeAddr, uint32 *tr_raw_data)
{
    uint16 x;
    //phy_vtss_mtimer_t timer;
    uint32 base;

    if(NULL == tr_raw_data || NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_tr_raw_read: invalid ptr");
        return RESULT_ERROR;
    }
    /* Determine base address */
    base = 16;
    VERIPHY_RC(_vtss8xx8_veriphy_phy_write(phy_handle, base, (5 << 13) | TrSubchanNodeAddr));

    VERIPHY_RC(_vtss8xx8_veriphy_phy_read(phy_handle, base, &x));
    if(!(x & 0x8000))
    {
        sal_udelay(500000);
        VERIPHY_RC(_vtss8xx8_veriphy_phy_read(phy_handle, base, &x));
    }

    if(!(x & 0x8000))
    {
        return GLB_VCT_ERROR;
    }

    _vtss8xx8_veriphy_phy_read(phy_handle, base + 2, &x);  
    *tr_raw_data = (uint32) x << 16;
    _vtss8xx8_veriphy_phy_read(phy_handle, base + 1, &x);  
    *tr_raw_data |= x;
    
    return RESULT_OK;
}

static int32 _vtss8xx8_veriphy_tr_raw_write(phy_handle_t* phy_handle, uint16 ctrl_word, uint32 val)
{
    uint32 base;
    base = 16;

    if(NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_tr_raw_write: invalid handle");
        return RESULT_ERROR;
    }
    VERIPHY_RC(_vtss8xx8_veriphy_phy_write(phy_handle, base + 2, (uint16)(val >> 16)));    
    VERIPHY_RC(_vtss8xx8_veriphy_phy_write(phy_handle, base + 1, (uint16) val));   
    VERIPHY_RC(_vtss8xx8_veriphy_phy_write(phy_handle, base, (uint16)((4 << 13) | ctrl_word)));
    
    return RESULT_OK;
}

/*- get_anom_thresh: determines threshold as a function of tap and EC/NC */ 
static void _vtss8xx8_veriphy_get_anom_thresh(phy_veriphy_task_t *tsk, uint8 tap)
{
    int16 log2ThreshX256;
    int8 sh;

    if(NULL == tsk)
    {
        DRV_LOG_ERR("veriphy_get_anom_thresh: invalid ptr");
        return;
    }
    
    log2ThreshX256 = 542 - (8 * (int16)tap) + tsk->log2VGAx256;
    if (tsk->nc != 0)
        log2ThreshX256 -= 500;
    tsk->thresh[0] = 256 + (log2ThreshX256 & 255);
    sh = (int8)(log2ThreshX256 >> 8);
    if (sh >= 0) {
        tsk->thresh[0] <<= sh;
    } 
    else {
        tsk->thresh[0] >>= -sh;
        if (tsk->thresh[0] < 23)
            tsk->thresh[0] = 23;
    }

    if (tsk->flags & 1) {
        tsk->thresh[1]  = tsk->thresh[0] >> 1;    /*- anomaly = 1/2 open/short when link up   */ 
    } 
    else {
        uint8 idx;

        tsk->thresh[1] = 0;
        for (idx = 0; idx < 5; ++idx) {  /*- anomaly = 1/3 open/short when link down */
            tsk->thresh[1] = (tsk->thresh[1] + tsk->thresh[0] + 2) >> 2;
        }
    }
    if (tsk->thresh[1] < 15)
        tsk->thresh[1] = 15;

    /*- Limit anomaly threshold to 12 lsbs less than tap range for each EC & NC tap */
    if ((tsk->thresh[1] > 1012) && ((tsk->nc > 0) || (tap >= 32))) {
        tsk->thresh[1] = 1012;
    } 
    else if ((tsk->thresh[1] > 4084) && (tap >= 16)) {
        tsk->thresh[1] = 4084;
    } 
    else if (tsk->thresh[1] > 8180) {
        tsk->thresh[1] = 8180;
    }
	DRV_LOG_DEBUG(DRV_PHY,"get_anom_thresh, thresh[0]=%d, thresh[1]=%d\n", tsk->thresh[0], tsk->thresh[1]);
}

/*- readAvgECNCECVar: reads averaged echo/NEXT-canceller coefficients */
/*- numCoeffs must be less than or equal to 12 */
/*- rpt can be 1, 2, 3, 4, or 8! */ 
static int16  *_vtss8xx8_veriphy_readAvgECNCECVar(phy_handle_t* phy_handle, uint8 subchan_phy, uint8 first, uint8 rpt_numCoeffs)
{
    /* Not Mustang family */
    int8 i, preScale=0;
    int16 c;
    int8 j, numRpt;
    uint32 tr_raw_data;
    uint8 discrd_flg=0;
    phy_veriphy_task_t* tsk;

    if((NULL == (tsk = phy_handle->phy_info.phy_veriphy_tsk)) || NULL == phy_handle)
    {
        DRV_LOG_ERR("readAvgECNCECVar: invalid ptr");
        return NULL;
    }
    DRV_LOG_DEBUG(DRV_PHY,"readAvgECNCECVar(%c, %d #%d, rpt %d, discard=%d)\n", 
        ('A'+(subchan_phy >> 6)), first, rpt_numCoeffs & 15, ((rpt_numCoeffs >> 4) & 7) + 1, (rpt_numCoeffs & VERIPHY_DISCARD) ? 1 : 0);
    
    tsk->maxAbsCoeff = 0;
    numRpt = ((rpt_numCoeffs >> 4) & 7) + 1;
    
    if (rpt_numCoeffs & VERIPHY_DISCARD) {
        preScale = -1;        
        discrd_flg = 1;
        rpt_numCoeffs &= ~VERIPHY_DISCARD;
    }
    
    if (!discrd_flg) {
        /*- Normal read & store coefficients -*/
        preScale = (first < 16 && numRpt >= 8) ? 1 : 0;
        
        /*- Clear coefficients to be averaged -*/
        for (j = 0; j < 8; ++j)
            tsk->coeff[j] = 0;
    }    
    
    /*- Accumulate coefficients (pre-scaling when necessay for dynamic range) -*/
    for (i = 0; i < numRpt; ++i) {
        for (j = 0; j < (rpt_numCoeffs & 15); ++j) {
            _vtss8xx8_veriphy_tr_raw_read(phy_handle, ((uint16)(subchan_phy & 0xc0) << 5)
                        | ((uint16)(j + first) << 1), &tr_raw_data);
            if (tr_raw_data & 0x2000 )
                c = 0xc000 | (int16)tr_raw_data;
            else
                c = (int16)tr_raw_data;
            
            if (c > 0) {
                if (c > tsk->maxAbsCoeff)
                    tsk->maxAbsCoeff = c;
            } 
            else if (-c > tsk->maxAbsCoeff)
                tsk->maxAbsCoeff = -c;
            
            if (preScale >= 0)
                tsk->coeff[j] += c >> preScale;
        }
    }
    
    if (preScale >= 0) {
        if (numRpt == 3) {
            /*- Complete averaging by scaling coefficients -*/
            for (j = 0; j < (rpt_numCoeffs & 15); ++j) {
                c = tsk->coeff[j];
                for (i = 0; i < 4; ++i)
                    c = (c >> 2) + tsk->coeff[j];
                tsk->coeff[j] = (c + 2) >> 2;
            }
        } 
        else {
            /*- Determine averaging scale-factor accounting for pre-scale -*/
            for (i = 0, j = numRpt >> preScale; j > 1; ++i, j = j >> 1)
                ;
            
            /*- Complete averaging by scaling coefficients -*/
            for (j = 0; j < (rpt_numCoeffs & 15); ++j)
                tsk->coeff[j] = tsk->coeff[j] >> i;
        }
        DRV_LOG_DEBUG(DRV_PHY,"{");   
        for (j = 0; j < (rpt_numCoeffs & 15); ++j) {
            DRV_LOG_DEBUG(DRV_PHY," %d", tsk->coeff[j]);   
        }
        DRV_LOG_DEBUG(DRV_PHY," }, ");
    }
    DRV_LOG_DEBUG(DRV_PHY,"maxAbsCoeff = %d\n", tsk->maxAbsCoeff);

    /*- Return pointer to first coefficient that was read */
    return &tsk->coeff[0];
}

/*- checkValidity: checks the validity of results in case of far-end turn-on or near-end bug -*/ 
static uint8 _vtss8xx8_veriphy_checkValidity(phy_veriphy_task_t *tsk, phy_handle_t* phy_handle, int16 noiseLimit)
{
    //phy_vtss_mtimer_t timer;
    int8 timeout = 0;

    if(NULL == tsk || NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_checkValidity: invalid ptr");
        return 0;
    }
    
    if (tsk->flags & 1)
        return 1;

    _vtss8xx8_veriphy_phy_page_tr(phy_handle);

    /*- EcVarForceDelayVal = 232 - 72, EcVarForceDelay = 1 */         
    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0188/*0, 3, 4*/, (tsk->tr_raw0188 & 0xfffe00) | (160 << 1) | 1);
    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0184/*0, 3, 2*/, 0x02 << tsk->subchan);  /*- EcVar<subchan>ForceIdle = 1 */
    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0184/*0, 3, 2*/, 0); /*- EcVar<subchan>ForceIdle = 0 */
    
    sal_udelay(1000);
    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0188/*0, 3, 4*/, tsk->tr_raw0188);


    _vtss8xx8_veriphy_readAvgECNCECVar(phy_handle,tsk->subchan << 6, 72, 0xb8);
    if((tsk->maxAbsCoeff) < ((tsk->stat[(int32)tsk->subchan] == 0) ? 4 : 1))
    {
        sal_udelay(200000);
        _vtss8xx8_veriphy_readAvgECNCECVar(phy_handle,tsk->subchan << 6, 72, 0xb8);
    }

    if((tsk->maxAbsCoeff) < ((tsk->stat[(int32)tsk->subchan] == 0) ? 4 : 1))
    {
        timeout = 1;
    }

    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0184/*0, 3, 2*/, 0x02 << tsk->subchan);  /*- EcVar<subchan>ForceIdle = 1 */
    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0184/*0, 3, 2*/, 0);  /*- EcVar<subchan>ForceIdle = 0 */

    if (timeout) {
        if (tsk->stat[(int32)tsk->subchan] != 0) {   
            tsk->stat[(int32)tsk->subchan]      = 0;
            tsk->loc[(int32)tsk->subchan]       = 0;
            tsk->strength[(int32)tsk->subchan]  = 0;
			DRV_LOG_DEBUG(DRV_PHY,"Invalid Anomaly Found");
        }
        return 0;
    } 
    else if (tsk->maxAbsCoeff > noiseLimit) {
        if (tsk->stat[(int32)tsk->subchan] == 0) {   
            tsk->loc[(int32)tsk->subchan] = 255;     
            tsk->strength[(int32)tsk->subchan] = tsk->maxAbsCoeff;
			DRV_LOG_DEBUG(DRV_PHY,"Length Invalid??? Block off this channel contribution...");
        } 
        else {   /*- Anomaly found, but invalid! */                      
            tsk->stat[(int32)tsk->subchan]      = 0;
            tsk->loc[(int32)tsk->subchan]       = 0;
            tsk->strength[(int32)tsk->subchan]  = 0;
			DRV_LOG_DEBUG(DRV_PHY,"Invalid Anomaly");
        }
        return 0;
    } 
    else if (tsk->stat[(int32)tsk->subchan] == 0 && (tsk->maxAbsCoeff + 2) >= VERIPHY_ABS(tsk->strength[(int32)tsk->subchan]))  {
        tsk->thresh[(int32)tsk->subchan]   = (tsk->maxAbsCoeff + 2);
        tsk->loc[(int32)tsk->subchan]      = 0;
        tsk->strength[(int32)tsk->subchan] = 0;
		DRV_LOG_DEBUG(DRV_PHY," thresh[%d] = %d, loc[%d] =0", tsk->subchan, tsk->maxAbsCoeff+2, tsk->subchan);
        return 1;
    }
	else {
		return 10;
	}
    return 1;
}


/*- `: search for anomalous echo/cross-coupled pair within 8-tap range of EC/NC */ 
static void _vtss8xx8_veriphy_xc_search(phy_veriphy_task_t *tsk, phy_handle_t* phy_handle, uint8 locFirst, uint8 prefix)
{
    uint8 idx;
    int16 s;
    int16 *c;

    DRV_LOG_DEBUG(DRV_PHY,"xc_search: locFirst = %d, prefix = %d", locFirst, prefix);
    if(NULL == tsk || NULL == phy_handle)
    {
        DRV_LOG_ERR("xc_search: invalid ptr");
        return;
    }
    _vtss8xx8_veriphy_phy_page_tr(phy_handle);
    c = _vtss8xx8_veriphy_readAvgECNCECVar(phy_handle, tsk->subchan << 6, tsk->firstCoeff,
                         (3 << 4) | tsk->numCoeffs);    

    for (idx = (tsk->numCoeffs - 1), c += (tsk->numCoeffs - 1); (int8)idx >= 0; --idx, --c) {

        s = (int16)(32L*(int32)*c/(int32)tsk->thresh[1]);
        DRV_LOG_DEBUG(DRV_PHY,"xc_search: Strength = %d", s);
        
        if (VERIPHY_ABS(*c) > tsk->thresh[1]) {
            if ((VERIPHY_ABS(s) <= VERIPHY_ABS(tsk->strength[(int32)tsk->subchan]))) {
                if ((tsk->signFlip < 0) && (tsk->stat[(int32)tsk->subchan] < 4)
                    && (tsk->loc[(int32)tsk->subchan] <= (locFirst + idx + 3))) {
                    if ((*c > 0) && (tsk->strength[(int32)tsk->subchan] < 0)) {
                        tsk->stat[(int32)tsk->subchan] = 2;
                        tsk->signFlip = 2;
                        if ((locFirst + idx) < prefix) {
                            tsk->loc[(int32)tsk->subchan] = locFirst + idx;
                        }
                        DRV_LOG_DEBUG(DRV_PHY,"\txc_search: Open-->int16 flip @ tap %d, strength = %d\n", locFirst + idx, s);
                    } 
                    else if ((*c < 0) && (tsk->strength[(int32)tsk->subchan] > 0)) {
                        tsk->stat[(int32)tsk->subchan] = 1;
                        tsk->signFlip = 2;
                        if ((locFirst + idx) < prefix) {
                            tsk->loc[(int32)tsk->subchan] = locFirst + idx;
                        }
                        DRV_LOG_DEBUG(DRV_PHY,"\txc_search: Short-->open flip @ tap %d, strength = %d\n", locFirst + idx, s);
                    }
                }
            } 
            else if (((locFirst + idx) >= prefix) || /* Or, implies ((locFirst + idx) < prefix) && .. */
                      ((tsk->loc[(int32)tsk->subchan] <= (locFirst + idx + 3)) &&
                       (tsk->stat[(int32)tsk->subchan] > 0) && (tsk->stat[(int32)tsk->subchan] <= 4))) {
                /*- magnitude is largest seen so far */         
                if (*c < -tsk->thresh[0]) {
                    if ((tsk->loc[(int32)tsk->subchan] <= (locFirst + idx + 3)) &&
                        (tsk->strength[(int32)tsk->subchan] > 0) && 
                        (tsk->strength[(int32)tsk->subchan] > (-s >> 1))) {
                        tsk->signFlip = 2;
                    }
                    tsk->stat[(int32)tsk->subchan] = 1;
                   DRV_LOG_DEBUG(DRV_PHY,"\txc_search: Open  @ tap %d, strength = %d < -thresh = -%d\n", 
                    locFirst + idx, s, tsk->thresh[0]);
                } 
                else if (*c > tsk->thresh[0]) {
                    if ((tsk->loc[(int32)tsk->subchan] <= (locFirst + idx + 3)) &&
                        (tsk->strength[(int32)tsk->subchan] < 0) && 
						(-tsk->strength[(int32)tsk->subchan] > (s >> 1))) {
                        tsk->signFlip = 2;
                    }
                    tsk->stat[(int32)tsk->subchan] = 2;
                    DRV_LOG_DEBUG(DRV_PHY,"\txc_search: Short @ tap %d, strength = %d > +thresh = %d\n", 
                        locFirst + idx, s, tsk->thresh[0]);
                } 
                else {
                    tsk->stat[(int32)tsk->subchan] = 4;
                    DRV_LOG_DEBUG(DRV_PHY,"\txc_search: Anom. @ tap %d, strength = %d > thresh = %d\n", 
                        locFirst + idx, s, tsk->thresh[1]);
                }
                tsk->loc[(int32)tsk->subchan] = locFirst + idx;
                tsk->strength[(int32)tsk->subchan] = s;
            }
        }                        
        tsk->signFlip = tsk->signFlip - 1;
    }                            
}

static const int32 FFEinit4_7search[2][4] = {
    {  1,   3,  -3,  -1 },  /*- coeff-set for anomSearch */
    { 10,  30, -30, -10 }   /*- coeff-set for lengthSearch */
};

/*- FFEinit4_7: Initializes FFE taps 4-7 from selected table and taps 0-3 & 8-15 are cleared */ 
static void _vtss8xx8_veriphy_FFEinit4_7(phy_handle_t* phy_handle, int8 taps4_7select)
{
    int8 idx, max;

    if(NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_task_start: invalid handle or task");
        return;
    }
    max = 16;
    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0240/*0, 4, 0x20*/, 0x100);  /*- Set FfeWriteAllSubchans */

    for (idx = 0; idx < max; ++idx) {   /*- Clear FFE except for taps 4-7 */    
        _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0200 | (idx << 1)/*0, 4, idx*/, 0);
        if (idx == 3)
            idx = 7;
    }
    for (idx = 0; idx < 4; ++idx) {
        _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0200 | ((4+idx) << 1)/*0, 4, 4+idx*/, FFEinit4_7search[(int32)taps4_7select][(int32)idx] << 9);
    }

    _vtss8xx8_veriphy_tr_raw_write(phy_handle, 0x0240/*0, 4, 0x20*/, 0);    /*- Clear FfeWriteAllSubchans */   
} 

extern int32 vsc8512_patch_suspend(phy_handle_t* phy_handle, uint8 suspend);
extern int32 vsc8504_patch_suspend(phy_handle_t* phy_handle, uint8 suspend);

int32 vtss8xx8_veriphy_task_start(phy_handle_t* phy_handle, uint8 mode)
{
    CTC_TASK_GET_MASTER
    phy_veriphy_task_t  *tsk;
    uint16 reg23; 

    if((NULL == (tsk = phy_handle->phy_info.phy_veriphy_tsk)) || NULL == phy_handle)
    {
        tsk->ret = GLB_VCT_ERROR;
        DRV_LOG_ERR("veriphy_task_start: invalid handle or task");
        return GLB_VCT_ERROR;
    }

    if((phy_handle->phy_info.phy_device_type == PORT_PHY_VSC8x58)
        ||(phy_handle->phy_info.phy_device_type == PORT_PHY_VSC8211))
    {
        tsk->ret = GLB_VCT_WAIT;
        tsk->task_state = (uint8) ((vtss8xx8_veriphy_task_state_t) VTSS8XX8_VERIPHY_STATE_INIT_0);
        tsk->flags = mode << 6;
        DRV_LOG_DEBUG(DRV_PHY,"VeriPHY algorithm initialized, State=%d\n", tsk->task_state);
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phy_handle, 1);
        /* Save Reg23 - to be restored after VeriPHY completes */
        /* select copper */
        _vtss8xx8_veriphy_phy_read(phy_handle, 23, &reg23);
        tsk->saveMediaSelect = (reg23 >> 6) & 3;
        reg23 = (reg23 & 0xff3f) | 0x0080;
        _vtss8xx8_veriphy_phy_write(phy_handle, 23, reg23);
    }
    else if((phy_handle->phy_info.phy_device_type == PORT_PHY_VSC8512)
        ||(phy_handle->phy_info.phy_device_type == PORT_PHY_VSC8504))
    {
        //Bugzilla#6769 - Ports get stuck if running Verify at port that is disabled.
        if(phy_handle->phy_info.phy_manage_info.enable)
        {
            tsk->task_state = (uint8) ((vtss8xx8_veriphy_task_state_t) VTSS8XX8_VERIPHY_STATE_INIT_0);
            
            // Micro patch must be suspended while veriphy is running. 
            if(phy_handle->phy_info.phy_device_type == PORT_PHY_VSC8512)
            {
                vsc8512_patch_suspend(phy_handle, TRUE);
            }
            else
            {
                vsc8504_patch_suspend(phy_handle, TRUE);
            }
            
            // Starting Veriphy
            _vtss8xx8_veriphy_phy_page_ext(phy_handle);
            _vtss8xx8_veriphy_phy_writemasked(phy_handle, VTSS_8xx8_VERIPHY_CTRL_REG1, 0x8000, 0x8000);
            _vtss8xx8_veriphy_phy_page_std(phy_handle);
        }
        tsk->ret = GLB_VCT_WAIT;
        tsk->count = 0;
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phy_handle, 5000);
    }
    return GLB_VCT_OK;
}
/*Support VSC8512/8504. bug 25277*/
int32 vtss_qsgmii_phy_veriphy(phy_handle_t* phdl, phy_veriphy_task_t *tsk)
{
    CTC_TASK_GET_MASTER
    uint16 value; 
    int32 ret;
    
    tsk->count++;
    _vtss8xx8_veriphy_phy_page_ext(phdl);
    _vtss8xx8_veriphy_phy_read(phdl, VTSS_8xx8_VERIPHY_CTRL_REG1, &value);
    if(phdl->phy_info.phy_manage_info.enable==0)
    {
        tsk->loc[0] = 0;
        tsk->loc[1] = 0;
        tsk->loc[2] = 0;
        tsk->loc[3] = 0;
        tsk->stat[3] = GLB_PAIR_STATE_ABNORMAL_OPEN;
        tsk->stat[2] = GLB_PAIR_STATE_ABNORMAL_OPEN;
        tsk->stat[1] = GLB_PAIR_STATE_ABNORMAL_OPEN;
        tsk->stat[0] = GLB_PAIR_STATE_ABNORMAL_OPEN;
        tsk->flags = 1<< 1;// Signal Veriphy result valid            
        ret = GLB_VCT_OK;
    }
    else if((value&0x8000) == 0)
    {
        // Multiply by 3 because resolution is 3 meters // See datasheet 
        tsk->loc[0] = ((value & 0x3f)>>8)*3;
        tsk->loc[1] = (value & 0x3f)*3;
        _vtss8xx8_veriphy_phy_read(phdl, VTSS_8xx8_VERIPHY_CTRL_REG2, &value);
        tsk->loc[2] = ((value & 0x3f)>>8)*3;
        tsk->loc[3] = (value & 0x3f)*3;
        _vtss8xx8_veriphy_phy_read(phdl, VTSS_8xx8_VERIPHY_CTRL_REG3, &value);
        tsk->stat[3] = (value & 0xf);
        tsk->stat[2] = (value & 0xf0) >> 4;
        tsk->stat[1] = (value & 0xf00) >> 8;
        tsk->stat[0] = (value & 0xf000) >> 12;
        tsk->flags = (1<<1) | (phdl->phy_info.phy_stat_flag.link_up!=0?1:0);// Signal Veriphy result valid            
        // Ok veriphy done we can resume the micro patchning. 
        if(phdl->phy_info.phy_device_type == PORT_PHY_VSC8512)
        {
            vsc8512_patch_suspend(phdl, FALSE);
        }
        else
        {
            vsc8504_patch_suspend(phdl, FALSE);
        }
        ret = GLB_VCT_OK;
    }
    else
    {
        ret = GLB_VCT_WAIT;
        /*wait for 12*5=60 seconds*/
        if(tsk->count <=12)
        {
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5000);
        }
        else
        {
            ret = GLB_VCT_ERROR;
        }
    }
    _vtss8xx8_veriphy_phy_page_std(phdl);
    return ret;
}

void
vtss8xx8_veriphy(void *p_data)
{
    CTC_TASK_GET_MASTER
    uint8 idx = 0;
    uint32 tr_raw_data = 0;
    phy_handle_t* phdl = NULL;
    phy_veriphy_task_t *tsk = NULL;
    uint16 reg23; 
    int32 ret = 0;

    if (NULL == p_data)
    {
        DRV_LOG_ERR("veriphy: invalid ptr");
        ret = GLB_VCT_ERROR;
        goto end;
    }
    
    phdl = (phy_handle_t*)p_data;
    if((NULL == (tsk = phdl->phy_info.phy_veriphy_tsk)) || NULL == phdl)
    {
        DRV_LOG_ERR("veriphy: invalid task");
        ret = GLB_VCT_ERROR;
        goto end;
    }
    
    if((phdl->phy_info.phy_device_type == PORT_PHY_VSC8512)||(phdl->phy_info.phy_device_type == PORT_PHY_VSC8504))
    {
        ret = vtss_qsgmii_phy_veriphy(phdl, tsk);
        goto end;
    }
    
    DRV_LOG_DEBUG(DRV_PHY,"---> ENTER VeriPHY state is: 0x%02x\n", tsk->task_state);
    DRV_LOG_DEBUG(DRV_PHY,"---> statA[%d], statB[%d], statC[%d], statD[%d] ", 
        tsk->stat[0], tsk->stat[1], tsk->stat[2], tsk->stat[3]);
    
    /*- Handle cleanup for VeriPHY task abort */     
    if (tsk->task_state & VTSS8XX8_VERIPHY_STATE_DONEBIT) {
        switch (tsk->task_state & ~VTSS8XX8_VERIPHY_STATE_DONEBIT) {
            
        case VTSS8XX8_VERIPHY_STATE_IDLE :   /*- Nothing to abort from Idle state */
            break;

        default :
            /*- Restore PHY to normal (non-VeriPHY) operation */

            /*- Restore blip-searches to default:  EcVarTrainingTime = 244, */
            /*-            EcVarTrainingGain = 1, EcVarShowtimeGain = 3    */
             
            _vtss8xx8_veriphy_phy_page_tr(phdl);
            _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0190/*0, 3, 8*/, 0xf47);
             
            /*- If VeriPHY operated in link-down mode, ... */
            if ((tsk->flags & 1) == 0)
            {
                /*- Clear EnableECvarDelayForce/Val */                       
                _vtss8xx8_veriphy_phy_page_tr(phdl);                
                _vtss8xx8_veriphy_tr_raw_read(phdl, 0x0f86/*1, 15, 0x03*/, &tr_raw_data);    
                tr_raw_data &= ~0x300000;
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0f86/*1, 15, 0x03*/, tr_raw_data);
            }

            /*- Signal completion of VeriPHY operation for Luton/Spyder */
            _vtss8xx8_veriphy_TpWriteBit(phdl, 12, 0, 1);

            /*- Link-up mode VeriPHY now completed, read valid bit! */
            if (tsk->flags & 1) 
            {
                sal_udelay(10000);              /*- Wait for valid flag to complete */
                tsk->flags |= _vtss8xx8_veriphy_ExtMiiReadBits(phdl, 24, 14, 14) << 1; /*- Set valid bit */
            }
            else /*- Results always valid in link-down mode (avoids AMS problem) */ 
                tsk->flags |= 2;  /*- Set valid bit */

            /*- Re-enable RClk125 gating */             
            _vtss8xx8_veriphy_TpWriteBit(phdl, 8, 9, 0);

            /*- Switch page back to main page */
            _vtss8xx8_veriphy_phy_page_std(phdl);
        }   /* switch (tsk->task_state & ~VTSS8XX8_VERIPHY_STATE_DONEBIT) */
        {
            /* restore mediaselect */
            _vtss8xx8_veriphy_phy_read(phdl, 23, &reg23);
            reg23 = (reg23 & 0xff3f) | (tsk->saveMediaSelect << 6);
            _vtss8xx8_veriphy_phy_write(phdl, 23, reg23);        
        }
        /*- Return to idle state */
        tsk->task_state = VTSS8XX8_VERIPHY_STATE_IDLE;        
        goto end;
    }  /*- if ABORT */

    switch (tsk->task_state)
    {
    /*- VeriPHY task is idle. Check for pending VeriPHY request */     
    case VTSS8XX8_VERIPHY_STATE_IDLE :
        break;
     
    /*- Configure for VeriPHY operation, and if successful, run VeriPHY algo. */
    case VTSS8XX8_VERIPHY_STATE_INIT_0 :
        /*- Initialize globals for VeriPHY search */
        tsk->numCoeffs = 8;
        for (idx = 0; idx < 4; ++idx) {
            tsk->stat[idx]     = 0;
            tsk->loc[idx]      = 0;
            tsk->strength[idx] = 0;
        }

        DRV_LOG_DEBUG(DRV_PHY, "VeriPHY init - TpWriteBit(8, 9, 1)");
        _vtss8xx8_veriphy_TpWriteBit(phdl, 8, 9, 1); /*- Disable RClk125 gating */
  
        DRV_LOG_DEBUG(DRV_PHY, "VeriPHY init - ExtMiiWrite(24, 0x8000)");
        Vtss8xx8VeriphyExtMiiWrite(phdl, 24, 0x8000); /* start trigger vriphy */
             
         /*- T(TR_MOD_PORT,TR_LVL_CRIT,"VeriPHY delay 750ms"); */
        /*- Wait for 750 ms for locRcvrStatus fall to propagate to link-down */
        tsk->task_state = VTSS8XX8_VERIPHY_STATE_INIT_1;
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 750);

        break;

    /*- Continue configuration for VeriPHY operation (after 750 ms delay) */
    case VTSS8XX8_VERIPHY_STATE_INIT_1 :
        tsk->flags |= _vtss8xx8_veriphy_MiiReadBits(phdl, 17, 12, 12);
        DRV_LOG_DEBUG(DRV_PHY,"VeriPHY init - vphy_linkup = %d", tsk->flags & 1);
        if (_vtss8xx8_veriphy_ExtMiiReadBits(phdl, 24, 15, 15) == 0) {
            /*- Link was up, but has gone down after trigger */
            DRV_LOG_DEBUG(DRV_PHY,"VeriPHY Init failed: ExtMiiReadBits(24, 15, 15) == 0");
            tsk->task_state |= 0x80; /*- Abort task! */
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
            break;
        }

        if (tsk->flags & 1) {
            int8 vgaGain;
 
            /*- Speed up blip-searches:  EcVarTrainingTime = 56 (32 for VSC8201), */
            /*-   EcVarTrainingGain = 1, EcVarShowtimeGain = 3 */  
            _vtss8xx8_veriphy_phy_page_tr(phdl);
            _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0190/*0, 3, 8*/, 0x000387);

            /*- Read VGA state for all four subchannels, extract VGA gain for A */             
            _vtss8xx8_veriphy_tr_raw_read(phdl, 0x0ff0/*1, 15, 0x38*/, &tr_raw_data);
            vgaGain = (int8)(tr_raw_data >> 4) & 0x0f;

            if (tr_raw_data & 0x100) {
                vgaGain -= 16;
            }
            tsk->log2VGAx256 = 900 + (26 * (int32)vgaGain);

            DRV_LOG_DEBUG(DRV_PHY, "VeriPHY link-up anomalySearch");
            tsk->task_state = VTSS8XX8_VERIPHY_STATE_INIT_ANOMSEARCH_0;
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        } 
        else {
            tsk->thresh[0]   = 400; /*- N: Setup timeout after N*5 ms of LinkControl1000 asserted */
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 10);
            tsk->task_state  = VTSS8XX8_VERIPHY_STATE_INIT_LINKDOWN;
        }
        break;
     
    /*- Continue configuration for link-down VeriPHY */
    case VTSS8XX8_VERIPHY_STATE_INIT_LINKDOWN :
        /*- Wait for MrSpeedStatus[1:0] to become 2 (LinkControl1000 asserted) */
        if (_vtss8xx8_veriphy_MiiReadBits(phdl, 28, 4, 3) == 2) {
            /*- Initialize FFE for link-down operation */
            _vtss8xx8_veriphy_phy_page_tr(phdl);
            _vtss8xx8_veriphy_FFEinit4_7(phdl, VERIPHY_FFEinit4_7anomSearch);

            /*- Speed up blip-searches:  EcVarTrainingTime = 56 (32 for VSC8201), */
            /*-   EcVarTrainingGain = 0, EcVarShowtimeGain = 0 */
            _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0190/*0, 3, 8*/, 0x381);
            
            _vtss8xx8_veriphy_tr_raw_read(phdl, 0x0f86/*1, 15, 0x03*/, &tr_raw_data);     /*- Set EnableECvarDelayForce/Val */
            tr_raw_data |= 0x300000;
            _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0f86/*1, 15, 0x03*/, tr_raw_data);
            tsk->log2VGAx256 = 0;

            DRV_LOG_DEBUG(DRV_PHY, "VeriPHY link-down anomalySearch"); 
            tsk->task_state = VTSS8XX8_VERIPHY_STATE_INIT_ANOMSEARCH_0;
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        } 
        else if (--(tsk->thresh[0]) > 0) {
            /*- Sleep for 5ms before polling MrSpeedStatus again */
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        } 
        else {  /*- timed out waiting for MrSpeedStatus to indicate LinkControl1000 asserted! */
            tsk->task_state |= 0x80;  /*- Abort VeriPHY task */
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        }
        break;

    /*- Setup for start of anomaly search */
    case VTSS8XX8_VERIPHY_STATE_INIT_ANOMSEARCH_0 :
        tsk->nc = (tsk->flags & 0x80) ? 0 : 3;
        tsk->thresh[2] = 0;/*- Clear EC invalid count accumulator */  
    case VTSS8XX8_VERIPHY_STATE_INIT_ANOMSEARCH_1 :
        tsk->thresh[1] = 0;/*- Clear EC invalid count (previous value) */  
        tsk->thresh[0] = 0;  /*- Clear EC invalid count */
        /*- fall throuh into VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_0 state */

    /*- Search for anomalous pair-coupling, pair-shorts, anomalous termination */
    /*- impedance, open-circuits, or short-circuits on all four twisted-pairs. */
    case VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_0 :
        /*- EC invalid count prev = current */
        tsk->thresh[1] = tsk->thresh[0];

        _vtss8xx8_veriphy_phy_page_tr(phdl);
        _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 1);  /*- EcVarForceIdle = 1 */

        /*- EcVar[A-D]InputSel = nc */
        tsk->tr_raw0188 = (int32)tsk->nc * 0x0aa000;

        _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0188/*0, 3, 4*/, tsk->tr_raw0188);
        _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 0);  /*- EcVarForceIdle = 0 */
        DRV_LOG_DEBUG(DRV_PHY,"VeriPHY delay 500ms"); 
        
        /*- allow blip time to train to anomaly location */
        tsk->task_state = VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_1;
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 500);
        break;

    /*- Validate at start of anomaly search that all subchannels are active */
    case VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_1 :
        _vtss8xx8_veriphy_phy_page_tr(phdl);

        for (idx = 0; idx < 4; ++idx) {
            _vtss8xx8_veriphy_tr_raw_read(phdl, (idx << 11) | 0x01c0/*idx, 3, 0x20*/, &tr_raw_data);
            if (((tr_raw_data & 0xff0080) == 0xf00080)) {
                /*- EC invalid count current = prev count + 1 */
                tsk->thresh[0] = tsk->thresh[1] + 1;
                
                DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: %s, blip is ZERO at all delays!",
                    idx+'A', (tsk->nc>=2) ? ((tsk->nc > 2) ? "NC3":"NC2") : ((tsk->nc > 0) ? "NC1":"EC")); 
                /*- Clear any anomaly found under this errant condition */
                tsk->stat[idx]      = 0;
                tsk->loc[idx]       = 0;
                tsk->strength[idx]  = 0;
            }
        }

        /*- If any invalid EC blips have been found, ... */
        if (tsk->thresh[0] != 0)
        {
            /*- If the most recent test had no invalid EC blips, ... */
            if (tsk->thresh[0] == tsk->thresh[1])
            {
                /*- If the anomaly search is just starting (NC == 3), ... */
                if (tsk->nc == ((tsk->flags & 0x80) ? 0 : 3))
                {
                    /*- Accumulate invalid EC blip count for timeouts */
                    tsk->thresh[2] += tsk->thresh[0] - 1;

                    /*- Continue on w/the anomaly search */
                    /*- Allow blip time to train to anomaly location */
                    tsk->subchan     = 0;/*- Start search with subchannel A */                   
                    tsk->task_state  = VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_2;
                    CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 500);
                }
                else/*- in the middle of anomaly search, ... */  
                {                     
                    /*- Restart with NEXT canceller anomaly search */
                    tsk->nc = (tsk->flags & 0x80) ? 0 : 3;                     

                    /*- EC invalid count current = prev count + 1 */
                    tsk->thresh[0] = tsk->thresh[1] + 1;
                    DRV_LOG_DEBUG(DRV_PHY,"VeriPHY: After blip is ZERO at all delays, reset to NC3!");                      

                    /*- delay before restarting anomaly search on NC 3 */
                    /*- x 5 ms/tick = 500 ms delay */
                    tsk->task_state  = VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_0;
                    CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 500);
                }
            }
            /*- else the most recent test had an invalid EC blip, */
            /*-      if the accumulated EC blip counts < timeout limit, ... */
            else if ((tsk->thresh[2] + tsk->thresh[0]) < 10)
            {
                /*- Loop back to wait for all valid EC blips or future timeout */
                tsk->task_state = VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_0;
                CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
            }
            else /*- more than 10 accumulated invalid EC blips, ... */
            {
                tsk->task_state |= 0x80;  /*- Abort VeriPHY task */
                CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
            }
            if ((tsk->flags & 1) == 0)/*- If running in link-down mode, reinitialize FFE */  
                _vtss8xx8_veriphy_FFEinit4_7(phdl, VERIPHY_FFEinit4_7anomSearch);
        }
        else /*- no invalid EC blips have been found */
        {
            /*- Continue on w/the anomaly search */ 
            /*- Allow blip time to train to anomaly location */
            CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, (tsk->nc == 0) ? 200 : 5);
            tsk->subchan     = 0;/*- Start search with subchannel A */ 
            tsk->task_state  = VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_2;
        }
        break;

    /*- All subchannels are now known to be active, commence anomaly search */
    case VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_2 :
        /*- Save current status as previous status */
        tsk->stat[(int32)tsk->subchan] = (tsk->stat[(int32)tsk->subchan] << 4) | (tsk->stat[(int32)tsk->subchan] & 0x0f);
        if (tsk->nc != 0 || (tsk->stat[(int32)tsk->subchan] & 0xc0) != 0x80) {/*- Keep cross-pair shorts */
            tsk->signFlip = 0;
            _vtss8xx8_veriphy_phy_page_tr(phdl);
            _vtss8xx8_veriphy_tr_raw_read(phdl, (tsk->subchan << 11) | 0x1c0 /*- tsk->subchan, 3, 0x20*/, &tr_raw_data);

            if (tr_raw_data & 128) { /*- found anything? */
                /*- ecVarBestDelay signals something really found (maybe < thresh) */
                /*- ^^^^^^^^^^^^^^ ---------> vvvvvvvvvvvv */
                idx = ((tsk->nc > 0) ? 16 : 72) + (tr_raw_data >> 16);
                DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: %s, blip @ tap %d", 
                    tsk->subchan+'A', (tsk->nc>=2) ? ((tsk->nc > 2) ? "NC3":"NC2") : ((tsk->nc > 0) ? "NC1":"EC"), idx);
                DRV_LOG_DEBUG(DRV_PHY,"possible anomaly near tap %d\n", idx); 
                _vtss8xx8_veriphy_get_anom_thresh(tsk, idx + (tsk->numCoeffs >> 1));
                /* tsk->firstCoeff = (family == VTSS_PHY_FAMILY_MUSTANG ? 192 : 72); */
                tsk->firstCoeff = 72;
                
                _vtss8xx8_veriphy_xc_search(tsk, phdl, idx, 0);
                if (tsk->stat[(int32)tsk->subchan] > 0 && tsk->stat[(int32)tsk->subchan] <= 0x0f) {
                    _vtss8xx8_veriphy_checkValidity(tsk, phdl, VERIPHY_MAX_ABS_COEFF_ANOM_INVALID_NOISE);

                    /*- Update previous status to match current status after validity check */ 
                    tsk->stat[(int32)tsk->subchan] = (tsk->stat[(int32)tsk->subchan] << 4) | (tsk->stat[(int32)tsk->subchan] & 0x0f);
                }
            } else {
                DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: %s (no blip)", 
                    tsk->subchan+'A', (tsk->nc>=2) ? ((tsk->nc > 2) ? "NC3":"NC2") : ((tsk->nc > 0) ? "NC1":"EC")); 
            }

            tsk->firstCoeff = (tsk->nc > 0) ? ((tsk->nc << 4) + 72) : 64;
            tsk->task_state = VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_3;
        } 
        else if (tsk->subchan < 3) {  /*- Move on to next subchannel, if not done */
            (tsk->subchan)++; /*- Re-enter in the same state for next subchannel */  
        } 
        else if (tsk->nc > 0) {  /*- Completed sweep of all 4 subchannels, move to next NC or EC */
            (tsk->nc)--;
            tsk->task_state = VTSS8XX8_VERIPHY_STATE_INIT_ANOMSEARCH_1;
        } 
        else {  /*- Completed sweep of EC, exit anomaly search */
            tsk->task_state = VTSS8XX8_VERIPHY_STATE_INIT_CABLELEN;
            tsk->subchan = 0;
        }
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        break;

    /*- Continue anomaly search by sweeping through fixed EC/NC @ tsk->firstCoeff */
    case VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_3 : 
        /* if (family == VTSS_PHY_FAMILY_MUSTANG) { */ 
        if (tsk->firstCoeff < 80)  /*- if searching EC */
            idx = tsk->firstCoeff;
        else /*- searching NC1..3 */  
            idx = tsk->firstCoeff & 0x0f;

        _vtss8xx8_veriphy_get_anom_thresh(tsk, idx + (tsk->numCoeffs >> 1));
        _vtss8xx8_veriphy_xc_search(tsk, phdl, idx, (idx == 4) ? 6 : 0);
        if (tsk->stat[(int32)tsk->subchan] > 0 && tsk->stat[(int32)tsk->subchan] <= 0x0f) {
            _vtss8xx8_veriphy_checkValidity(tsk, phdl, VERIPHY_MAX_ABS_COEFF_ANOM_INVALID_NOISE);

            /*- Update previous status to match current status after validity check */
            tsk->stat[(int32)tsk->subchan] = (tsk->stat[(int32)tsk->subchan] << 4) | (tsk->stat[(int32)tsk->subchan] & 0x0f);
        }
        if (tsk->nc > 0) {
            if (idx > 0)
                tsk->firstCoeff -= tsk->numCoeffs;
            else
            {
                /*- Recode NC-anomalies as cross-pair anomalies at end-of-search */
                if ((tsk->stat[(int32)tsk->subchan] & 0x0f) > 0 && (tsk->stat[(int32)tsk->subchan] & 0x08) == 0) {
                    tsk->stat[(int32)tsk->subchan] = (tsk->stat[(int32)tsk->subchan] & 0xf4) | 8;
                    if (tsk->nc != tsk->subchan)
                        tsk->stat[(int32)tsk->subchan] += tsk->nc;
                }
            }
        } 
        else {/*- for EC, conditionally extend search down to 6th tap */
            if (idx > 8)
                tsk->firstCoeff -= tsk->numCoeffs;
            else if (idx == 8) {
                tsk->firstCoeff  = 4;
                tsk->numCoeffs = 4;
            } 
            else {
                tsk->numCoeffs = 8;  /*- Restore to default of 8 coeffs */
                idx = 0;/*- Walk to next state */  
            }
        }
        if (idx == 0) {
            if (tsk->subchan < 3) {  /*- Move on to next subchannel, if not done */
                (tsk->subchan)++;
                tsk->task_state = VTSS8XX8_VERIPHY_STATE_ANOMSEARCH_2;
            } 
            else if (tsk->nc > 0) {/*- Completed sweep of all 4 subchannels, move to next NC or EC */  
                (tsk->nc)--;
                tsk->task_state = VTSS8XX8_VERIPHY_STATE_INIT_ANOMSEARCH_1;
            } 
            else {  /*- Completed sweep of EC, exit anomaly search */
                tsk->task_state = VTSS8XX8_VERIPHY_STATE_INIT_CABLELEN;
                tsk->subchan = 0;
            }
        }
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        break;

    /*- Initialize getCableLength search */
    case VTSS8XX8_VERIPHY_STATE_INIT_CABLELEN :
        for (idx = 0; (tsk->flags & 0xc0) == 0 && idx < 4; ++idx)
        {
            tsk->stat[idx] &= 0x0f;
            if (tsk->stat[idx] == 0) {
                if ((tsk->flags & 0xc0) != 0)
                    tsk->loc[idx] = 255; /*- Set to unknown location, if cable length not measured */  
                else
                    tsk->flags |= 4;
            }
        }
        if ((tsk->flags & 0xc0) != 0 || (tsk->flags & 4) == 0)  /*- Anoamlies found on all four pairs */
            tsk->task_state = VTSS8XX8_VERIPHY_STATE_PAIRSWAP;
        else  /*- Initialize for measuring cable length */
        {
            _vtss8xx8_veriphy_phy_page_tr(phdl);
            DRV_LOG_DEBUG(DRV_PHY,"VeriPHY getCableLength"); 
            if ((tsk->flags & 1) == 0) {/*- If link-down, gain up FFE to make blip easier to spot! */ 
                _vtss8xx8_veriphy_FFEinit4_7(phdl, VERIPHY_FFEinit4_7lengthSearch);
               /*- Smooth EcVar training: EcVarTrainingTime = 75, EcVarShowtimeGain = 1 */
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x190/*0, 3, 8*/, 0x4b5);
            }

            /*- EcVarForceDelayVal = 232 - 72, EcVarForceDelay = 1 */
            _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0188/*0, 3, 4*/, ((232 - 72) << 1) | 1);
            _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 1);    /*- EcVarForceIdle = 1 */
            _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 0);    /*- EcVarForceIdle = 0 */

            idx = 0xff;
            for (tsk->subchan = 0; tsk->subchan < 4; ++(tsk->subchan))
            {
                if (tsk->stat[(int32)tsk->subchan] != 0)
                    tsk->thresh[(int32)tsk->subchan] = 0;
                else
                {
                    for (tsk->signFlip = 0, tsk->maxAbsCoeff = 0; tsk->maxAbsCoeff < ((tsk->flags & 1) ? 1 : 4) && tsk->signFlip < 100; ++(tsk->signFlip))
                    {
                        /*- ECVar Tap=0, Discard =1, Rpt = 8, numCoeffs=8 */
                        _vtss8xx8_veriphy_readAvgECNCECVar(phdl, tsk->subchan << 6, 72, 0xf8);
                        if (tsk->maxAbsCoeff < ((tsk->flags & 1) ? 1 : 4)) {
                            sal_udelay(2000);
                        }
                        else
                            tsk->thresh[(int32)tsk->subchan] = tsk->maxAbsCoeff;
                    }
                    if (tsk->signFlip > 1)
                    {
                        DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: maxAbsC(tsk->thresh) = %d took %d attempts!", 
                            tsk->subchan+'A', tsk->thresh[(int32)tsk->subchan], tsk->signFlip); 
                    }
                    if (tsk->thresh[(int32)tsk->subchan] < 14)
                        tsk->thresh[(int32)tsk->subchan] = 20;
                    else
                        tsk->thresh[(int32)tsk->subchan] += 6;
                    if (tsk->thresh[(int32)tsk->subchan] > VERIPHY_MAX_ABS_COEFF_LEN_INVALID_NOISE)
                    {
                        DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: maxAbsC(tsk->thresh) = %d > noise limit of %d!", 
                            tsk->subchan+'A', tsk->thresh[(int32)tsk->subchan], VERIPHY_MAX_ABS_COEFF_LEN_INVALID_NOISE); 
                        tsk->loc[(int32)tsk->subchan] = 255;
                        tsk->strength[(int32)tsk->subchan] = tsk->thresh[(int32)tsk->subchan];
                    }
                    else if (idx == 0xff)
                        idx = (tsk->subchan << 2) | tsk->subchan;
                    else if (tsk->thresh[(int32)tsk->subchan] > tsk->thresh[idx >> 2])
                        idx = tsk->subchan << 2 | (idx & 3);
                    if (idx != 0xff && tsk->thresh[(int32)tsk->subchan] < tsk->thresh[idx & 3])
                        idx = (idx & 0x0c) | tsk->subchan;
                }
            }
            /*- if max(tsk->thresh) >= 1.5*min(tsk->thresh), the max subchan may be unreliable */
            if (idx != 0xff && (tsk->thresh[idx >> 2] >= (tsk->thresh[idx & 3] + (tsk->thresh[idx & 3] >> 1))))
                tsk->flags = ((idx << 2) & 0x30) | 0x08 | (tsk->flags & 0xc7);
            else
                tsk->flags = (tsk->flags & 0xc7);

            DRV_LOG_DEBUG(DRV_PHY,"VeriPHY : tsk->thresh = { %d, %d, %d, %d }, unreliablePtr = %d", 
                tsk->thresh[0], tsk->thresh[1], tsk->thresh[2], tsk->thresh[3], (((tsk->flags >> 4) & 3) + (tsk->flags & 8))); 

            tsk->flags &= ~4;  /*- Clear done flag */
            tsk->signFlip = 0;

            tsk->tr_raw0188 = (160 << 1) | 1;
            tsk->firstCoeff = 72;

            tsk->task_state = VTSS8XX8_VERIPHY_STATE_GETCABLELEN_0;
        }
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        break;

    /*- start getCableLength search */     
    case VTSS8XX8_VERIPHY_STATE_GETCABLELEN_0 :
        if (((tsk->flags & 4) == 0) && ((tsk->firstCoeff & 0x7f) >= tsk->numCoeffs))
        {
            if (tsk->firstCoeff == 72) {
                idx = tsk->tr_raw0188 >> 1;
                if (idx > 0) {
                    /*- EcVarForceDelayVal = current delay, EcVarForceDelay = 1 */
                    idx -= tsk->numCoeffs;
                    tsk->tr_raw0188 = (idx << 1) | 1;
                    idx += 72;/*- Make idx indicate coefficient tap # */  
                } 
                else { /*- End blip scan; restore blip canceller to normal operation */
                    /*- EcVarForceDelayVal = 0, EcVarForceDelay = 0 */
                    tsk->tr_raw0188 = 0;
                    tsk->firstCoeff = 64;
                    
                    /*- Scale up threshold to avoid false triggers */
                    for (idx = 0; idx < 4; ++idx) {
                        tsk->thresh[idx] = tsk->thresh[idx] + (tsk->thresh[idx] >> 2);
                    }
                    idx = tsk->firstCoeff;  /*- Make idx indicate coefficient tap # */
                }
                
                _vtss8xx8_veriphy_phy_page_tr(phdl);
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0188/*0, 3, 4*/, tsk->tr_raw0188);
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 1);      /*- EcVarForceIdle = 1 */
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 0);      /*- EcVarForceIdle = 0 */
            } 
            else {
                tsk->firstCoeff -= tsk->numCoeffs;
                idx = tsk->firstCoeff;  /*- Make idx indicate coefficient tap # */
            }
  
           DRV_LOG_DEBUG(DRV_PHY,"probing for cable length at tap %d\n", idx);
            /*- delay done by numCoeffs taps to scan numCoeffs taps past 1st detection to refine length estimate */             
            if ((tsk->loc[0] > 0) && (tsk->loc[1] > 0) && (tsk->loc[2] > 0) && (tsk->loc[3] > 0))
                tsk->flags |= 4;  /*- Set done flag */
  
            for (tsk->subchan = 0; tsk->subchan < 4; ++(tsk->subchan)) {
                /*- only measure length on non-anomalous subchans */
                _vtss8xx8_veriphy_phy_page_tr(phdl);
                if (tsk->stat[(int32)tsk->subchan] == 0 && ((idx+12) > tsk->loc[(int32)tsk->subchan])) {
                    int16 *c = _vtss8xx8_veriphy_readAvgECNCECVar(phdl, tsk->subchan << 6,
                                                          tsk->firstCoeff, 0x20 | tsk->numCoeffs); 
                    uint8 cnt;
                    c   += tsk->numCoeffs;
                    idx += tsk->numCoeffs;
                    cnt  = tsk->numCoeffs;
                    do {
                        --idx;
                        --c;
                        if (((tsk->signFlip >> tsk->subchan) & 1) != 0) {
                            if (tsk->loc[(int32)tsk->subchan] == (idx + 1)) {
                                if ( (tsk->strength[(int32)tsk->subchan] < 0 && *c < tsk->strength[(int32)tsk->subchan]) ||
                                     (tsk->strength[(int32)tsk->subchan] > 0 && *c > tsk->strength[(int32)tsk->subchan]) ) {
                                    tsk->loc[(int32)tsk->subchan] = idx;
                                    tsk->strength[(int32)tsk->subchan] = *c;
                                    DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: ", (int32)(tsk->subchan+'A'));  
                                    DRV_LOG_DEBUG(DRV_PHY,"sign-move 2 tap %d, strength = %d\n", 
                                        (int32)tsk->loc[(int32)tsk->subchan], tsk->strength[(int32)tsk->subchan]); 
                                }
                            }
                        } 
                        else if (VERIPHY_ABS(*c) > tsk->thresh[(int32)tsk->subchan]) {
                            if ((tsk->strength[(int32)tsk->subchan] == 0) ||
                                ((tsk->loc[(int32)tsk->subchan] <= (idx + 3)) &&
                                 (((tsk->strength[(int32)tsk->subchan] > 0) &&
                                   (tsk->strength[(int32)tsk->subchan] <= *c))
                                  || ((tsk->strength[(int32)tsk->subchan] < 0) &&
                                      (tsk->strength[(int32)tsk->subchan] >= *c))
                                 ))) {
                                if (tsk->strength[(int32)tsk->subchan] == 0) {  /*- Test validity 1st detection only */
                                    tsk->strength[(int32)tsk->subchan] = *c;
                                    tsk->loc[(int32)tsk->subchan] = idx;
                                    _vtss8xx8_veriphy_checkValidity(tsk, phdl, VERIPHY_MAX_ABS_COEFF_LEN_INVALID_NOISE);
                                    if (tsk->strength[(int32)tsk->subchan] != 0) {
                                        DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: ", (int32)(tsk->subchan+'A'));
                                        DRV_LOG_DEBUG(DRV_PHY,"blip-det. @ tap %d, strength = %d\n", idx, *c);
                                    }
                                } 
                                else {
                                    tsk->strength[(int32)tsk->subchan] = *c;
                                    tsk->loc[(int32)tsk->subchan] = idx;
                                    DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: ", (int32)(tsk->subchan+'A'));
                                    DRV_LOG_DEBUG(DRV_PHY,"blip-move 2 tap %d, strength = %d\n", idx, *c);
                                }
                            } 
                            else if ((tsk->loc[(int32)tsk->subchan] <= (idx + 5)) &&
                                       (((tsk->strength[(int32)tsk->subchan] > 0) &&
                                         ((-tsk->strength[(int32)tsk->subchan] + (tsk->strength[(int32)tsk->subchan]>>3)) >= *c))
                                     || ((tsk->strength[(int32)tsk->subchan] < 0) &&
                                         ((-tsk->strength[(int32)tsk->subchan] + (tsk->strength[(int32)tsk->subchan]>>3)) <= *c))
                                       )) {
                                tsk->loc[(int32)tsk->subchan] = idx;
                                tsk->strength[(int32)tsk->subchan] = *c;
                                tsk->signFlip |= 1 << tsk->subchan;
                                DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: ", (int32)(tsk->subchan+'A'));
                                DRV_LOG_DEBUG(DRV_PHY,"sign-flip @ tap %d, strength = %d\n", 
                                    tsk->loc[(int32)tsk->subchan], tsk->strength[(int32)tsk->subchan]); 
                            }
                        }
                    } while (--cnt != 0);
                }
            }                    /*- end for ( tsk->subchan = 0; tsk->subchan < 4; ++(tsk->subchan) ) */
            if (tsk->flags & 4) {
                tsk->task_state = VTSS8XX8_VERIPHY_STATE_GETCABLELEN_1;
                _vtss8xx8_veriphy_phy_page_tr(phdl);
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0188/*0, 3, 4*/, 0);  /*- EcVarForceDelay = 0 */
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 1);  /*- EcVarForceIdle = 1 */
                _vtss8xx8_veriphy_tr_raw_write(phdl, 0x0184/*0, 3, 2*/, 0);  /*- EcVarForceIdle = 0 */ 
            }
        }
        else
            tsk->task_state = VTSS8XX8_VERIPHY_STATE_GETCABLELEN_1;
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        break;

    /*- getCableLength search state 2 */
    case VTSS8XX8_VERIPHY_STATE_GETCABLELEN_1 :
        /*- Length estimate to be the average of medians of computed loc's */
        {
        uint8 maxptr, minptr, max2ptr, min2ptr, endloc = 0;

        maxptr = minptr = 8;
        for (idx = 0; idx < 4; ++idx)
        {
            if (tsk->stat[idx] == 0 && tsk->loc[idx] < 255)
            {
                int16 s;

                _vtss8xx8_veriphy_get_anom_thresh(tsk, tsk->loc[idx]);
                if (tsk->loc[idx] <= 8)
                    s = 0;                   /*- Don't change blips on <2m loops anomalous */                   
                else if (tsk->flags & 1)     /*- if in link-up mode, ... */
                    s = (3*tsk->strength[idx] + 2) >> 2;   /*- 3/4  of strength link-up */
                else/*- else in link-down mode */  
                    s = (3*tsk->strength[idx] + 16) >> 5;  /*- 3/32 of strength link-down */
 
                if (s > tsk->thresh[0]) {
                    tsk->stat[idx] = 2;
                    DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: changing length (%d) to int16 (%d > %d)!\n", 
                        idx+'A', endloc, s, tsk->thresh[0]); 
                } 
                else if (s < -tsk->thresh[0]) {
                    tsk->stat[idx] = 1;
                    DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: changing length (%d) to open (%d < -%d)!\n", 
                        idx+'A', endloc, s, tsk->thresh[0]);
                } 
                else if (VERIPHY_ABS(s) > tsk->thresh[1]) {
                    tsk->stat[idx] = 4;
                    DRV_LOG_DEBUG(DRV_PHY,"VeriPHY %c: changing length (%d) to anom (VERIPHY_ABS(%d) > %d)!\n", 
                        idx+'A', endloc, s, tsk->thresh[1]); 
                } 
                else if (((tsk->signFlip >> idx) & 1) == 0) {/*- if no sign flip detected, */
                        tsk->loc[idx] = tsk->loc[idx] - 1;/*- reduce estimated tap by 1 */
                }
                DRV_LOG_DEBUG(DRV_PHY,"subchan %c, loc = %d\n", idx+'A', tsk->loc[idx]); 
 
                if (((tsk->flags & 8) == 0 || idx != ((tsk->flags >> 4) & 3)) && ((minptr >= 4) || (tsk->loc[idx] < tsk->loc[minptr])))
                    minptr = idx;
                if (((tsk->flags & 8) == 0 || idx != ((tsk->flags >> 4) & 3)) && ((maxptr >= 4) || (tsk->loc[idx] > tsk->loc[maxptr])))
                    maxptr = idx;
            }
        }
        if (minptr == 8) {
            /*- Only use unreliablePtr length as a last resort */
            if (tsk->flags & 8) {
                endloc = tsk->loc[(tsk->flags >> 4) & 3];
                DRV_LOG_DEBUG(DRV_PHY,"VeriPHY : unreliable endloc = %d is only available!\n", endloc);
            } 
            else {
                endloc = 255;
                DRV_LOG_DEBUG(DRV_PHY,"VeriPHY : No length (endloc = 255) is available!\n");
            }
        } 
        else {
            /*- Rehabilitate the unreliablePtr if result is reasonable before median search */
            if ((tsk->flags & 8) && tsk->loc[(tsk->flags >> 4) & 3] >= (tsk->loc[minptr] - 1)) {
                if (tsk->loc[(tsk->flags >> 4) & 3] == (tsk->loc[minptr] - 1)) {
                    minptr = (tsk->flags >> 4) & 3;
                    tsk->flags &= ~8;
                } 
                else if (tsk->loc[(tsk->flags >> 4) & 3] <= (tsk->loc[maxptr] + 1)) {
                    if (tsk->loc[(tsk->flags >> 4) & 3] == (tsk->loc[maxptr] + 1))
                        maxptr = (tsk->flags >> 4) & 3;
                    tsk->flags &= ~8;
                }
            }
 
            /*- Find the middle 2 results (may point to the same item, min, or max) */
            min2ptr = maxptr;
            max2ptr = minptr;
            for (idx = 0; idx < 4; ++idx) {
                if (tsk->stat[idx] == 0 && tsk->loc[idx] < 255 && ((tsk->flags & 8) == 0 || idx != ((tsk->flags >> 4) & 3))) {
                    if ((idx != minptr) && (tsk->loc[idx] < tsk->loc[min2ptr]))
                        min2ptr = idx;
                    if ((idx != maxptr) && (tsk->loc[idx] > tsk->loc[max2ptr]))
                        max2ptr = idx;
                }
            }
            endloc = (max2ptr == minptr) ? tsk->loc[maxptr] : (uint8)(((uint32)tsk->loc[min2ptr] + tsk->loc[max2ptr]) >> 1);
            DRV_LOG_DEBUG(DRV_PHY,"VeriPHY : max/minptr = %d/%d, max2/min2ptr = %d/%d, endloc = %d!\n", 
                maxptr, minptr, max2ptr, min2ptr, endloc);
        }
        for (idx = 0; idx < 4; ++idx) {
            if (tsk->stat[idx] == 0) {
                DRV_LOG_DEBUG(DRV_PHY,"VeriPHY loc[%d] = %d --> %d\n", idx, tsk->loc[idx], endloc); 
                tsk->loc[idx] = endloc;
            }
        }
        tsk->task_state = VTSS8XX8_VERIPHY_STATE_PAIRSWAP;
        }
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        break;

    /*- PairSwap & VeriPHY finish-up state */
    case VTSS8XX8_VERIPHY_STATE_PAIRSWAP :
        /*- Swap pairs as appropriate to represent pairs at connector */
        /*- (up to now, it was subchan) */
        DRV_LOG_DEBUG(DRV_PHY,"VeriPHY swapPairs");
        {
            int8 MDIX_CDpairSwap;
            uint8 ubtemp;
            int16 stemp;
 
            /*- Read MDI/MDIX, bit[1], and CD-pair-swap, bit [0] */
            MDIX_CDpairSwap = _vtss8xx8_veriphy_MiiReadBits(phdl, 28, 13, 12 );
            if (MDIX_CDpairSwap < 2) {
                /*- Swap pairs A & B */
                stemp            = tsk->strength[1];
                tsk->strength[1] = tsk->strength[0];
                tsk->strength[0] = stemp;
                ubtemp           = tsk->stat[1];
                tsk->stat[1]     = tsk->stat[0];
                tsk->stat[0]     = ubtemp;
                ubtemp           = tsk->loc[1];
                tsk->loc[1]      = tsk->loc[0];
                tsk->loc[0]      = ubtemp;
 
                /*- Recode cross-pair status for pairs A & B */
                for (idx = 0; idx < 4; ++idx) {
                    if ( (tsk->stat[idx] & 10) == 8 )
                        tsk->stat[idx] = tsk->stat[idx] ^ 1;
                }
            }
 
            if ((MDIX_CDpairSwap == 0) || (MDIX_CDpairSwap == 3)) {
                /*- Swap pairs C & D */
                stemp            = tsk->strength[3];
                tsk->strength[3] = tsk->strength[2];
                tsk->strength[2] = stemp;
                ubtemp           = tsk->stat[3];
                tsk->stat[3]     = tsk->stat[2];
                tsk->stat[2]     = ubtemp;
                ubtemp           = tsk->loc[3];
                tsk->loc[3]      = tsk->loc[2];
                tsk->loc[2]      = ubtemp;
 
                /*- Recode cross-pair status for pairs C & D */
                for (idx = 0; idx < 4; ++idx) {
                    if ( (tsk->stat[idx] & 10) == 10 )
                        tsk->stat[idx] = tsk->stat[idx] ^ 1;
                }
            }
        }
 
        /*- Convert tap numbers into length in meters for return to user */
        for (idx = 0; idx < 4; ++idx) {
            uint8 loc = tsk->loc[idx];
			DRV_LOG_DEBUG(DRV_PHY,"Tap Location - loc[%d] = %d\n", idx, tsk->loc[idx]); 
            if (loc <= 7)
                tsk->loc[idx] = 0;
            else if (loc < 255)
            {
                /*- Initial taps that are close to zero, differing FFE spreading effects, */
                /*- scale factor is slightly different in Mustang */
 
                if (tsk->flags & 1)
                    loc -= 7;
                else
                    loc -= 6;
                loc = loc - ((((loc + (loc >> 4)) >> 2) + 1) >> 1);
                tsk->loc[idx] = loc;
                if ((tsk->stat[idx] == 2) && (loc < 5)) {
                    tsk->stat[idx] = 4;
                }                
            }
        }
        tsk->task_state = VTSS8XX8_VERIPHY_STATE_FINISH;
        CTC_TASK_ADD_TIME_MSEC(vtss8xx8_veriphy, phdl, 5);
        break;
    default:
        break;
    }

	DRV_LOG_DEBUG(DRV_PHY,"<--- EXIT VeriPHY state is: 0x%02x\n", tsk->task_state);

    ret = (tsk->task_state == VTSS8XX8_VERIPHY_STATE_IDLE) ? GLB_VCT_OK : GLB_VCT_WAIT;    
    if (tsk->ret == GLB_VCT_OK && !(tsk->flags & (1<<1))) {
        /* Invalid result */
        tsk->ret = GLB_VCT_ERROR;
    }
end:
    tsk->ret = ret;    
    return;
}



int32 vtss8xx8_veriphy_get_result(phy_handle_t* phy_handle)
{
    phy_veriphy_task_t *tsk;
    uint32              i;    

    if((NULL == (tsk = phy_handle->phy_info.phy_veriphy_tsk)) || NULL == phy_handle)
    {
        DRV_LOG_ERR("veriphy_status: invalid handle or task");
        return GLB_VCT_ERROR;
    }
    if(tsk->ret == GLB_VCT_WAIT)
    {
        return GLB_VCT_WAIT;
    }
    tsk->result.link = (((tsk->ret == GLB_VCT_OK) && (tsk->flags & 1)) ? 1 : 0);
    for (i = 0; i < 4; i++) {
        tsk->result.status[i] = (tsk->ret == GLB_VCT_OK ? (tsk->stat[i] & 0x0f) : GLB_PAIR_STATE_ABNORMAL_OPEN);
        tsk->result.length[i] = (tsk->ret == GLB_VCT_OK ? tsk->loc[i] : 0);
    }
    
    return GLB_VCT_OK;
}

