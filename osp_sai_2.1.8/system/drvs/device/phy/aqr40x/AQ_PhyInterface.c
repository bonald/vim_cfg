#include "AQ_PhyInterface.h"

unsigned int
AQ_API_MDIO_Write(  AQ_Port PHY_ID,  unsigned int MMD,
unsigned int address,  unsigned int data)
{
    mdio_bus_para_t para;
    int32 ret;
    phy_handle_t*phdl= get_phy_hdl(PHY_ID);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    para.xgphy_para.devno = MMD;
    para.xgphy_para.reg = address;
    para.xgphy_para.phy_addr = phdl->phy_info.phy_addr;
    para.xgphy_para.val = data;
    if(NULL == phdl->mdio_hdl || NULL == phdl->mdio_hdl->write)
    {
        return RESULT_ERROR;
    }
    ret = phdl->mdio_hdl->write(phdl->mdio_hdl, &para);
    if(ret)
    {
        return RESULT_ERROR;
    }
    return RESULT_OK;
}

unsigned int
AQ_API_MDIO_Read(  AQ_Port PHY_ID,  unsigned int MMD,
  unsigned int address)
{
    int32 ret;
    uint16 value;
    phy_handle_t* phdl;
    mdio_bus_para_t para;
    phdl= get_phy_hdl(PHY_ID);
    if(NULL == phdl)
    {
        return RESULT_ERROR;
    }
    para.xgphy_para.devno = MMD;
    para.xgphy_para.reg = address;
    para.xgphy_para.phy_addr = phdl->phy_info.phy_addr;
    if(NULL == phdl->mdio_hdl || NULL == phdl->mdio_hdl->read)
    {
        return RESULT_ERROR;
    }

    ret = phdl->mdio_hdl->read(phdl->mdio_hdl, &para);
    if(ret)
    {
        return RESULT_ERROR;
    }
    value = para.xgphy_para.val;
    return value;
}

#if 0
void AQ_API_MDIO_BlockWrite
(
  AQ_Port PHY_ID,
  unsigned int MMD,
  unsigned int address,
  unsigned int data
)
{
      
}


void AQ_API_MDIO_BlockRead
(
  AQ_Port PHY_ID,
  unsigned int MMD,
  unsigned int address
)
{

}


unsigned int * AQ_API_MDIO_BlockOperationExecute
(
  AQ_Port PHY_ID
)
{

}


unsigned int AQ_API_MDIO_MaxBlockOperations
(
)
{
}

#endif
