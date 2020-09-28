#include <madCopyright.h>
/********************************************************************************
* testEEESample.c
*
* DESCRIPTION:
*       test API of XMDIO PCS functions
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "madSample.h"
  #include "stdio.h" 

/*
#define TEST_DEBUG
*/

extern void testMADDisplayStatus(MAD_STATUS status);

#define MAX_SAVED_ERROR    32
static int savedResult [MAX_SAVED_ERROR];
static char savedResultHead [MAX_SAVED_ERROR] [64];

MAD_STATUS testXmdioPcsStatus(MAD_DEV *dev)
{
  MAD_STATUS  retVal;
  MAD_LPORT   port;
  MAD_PCS_STATUS status;
  MAD_PCS_EEE_CAPABILITY capability;
  MAD_U16     data;
  MAD_32        i, count=0;
  MAD_U8	  ipg = 0xaa;
  MAD_BOOL    en = 1;

  MSG_PRINT(("\nCheck XMDIO PCS status\n"));

  for ( i=0; i<MAX_SAVED_ERROR; i++)
    savedResult [i]=0;


  for(port=0; port<dev->numOfPorts; port++)
  {
    /* XMDIO PCS status: */
    retVal = madPCSStatus(dev,port,&status);
    if (retVal != MAD_OK)
    {
      MSG_PRINT(("%x\t",(int)retVal));  
      sprintf(savedResultHead[count], "GetPcsStatus Failed %x: ", retVal);
      savedResult[count++]=retVal;
    }
    else
    {
      MSG_PRINT(("port%1i XMDIO PCS status:\n",(int)port));
      MSG_PRINT(("\ttxLpIdleReceived %i\n",(int)status.txLpIdleReceived));
      MSG_PRINT(("\trxLpIdleReceived %i\n",(int)status.rxLpIdleReceived));
      MSG_PRINT(("\ttxLpIdleIndication %i\n",(int)status.txLpIdleIndication));
      MSG_PRINT(("\trxLpIdleIndication %i\n",(int)status.rxLpIdleIndication));
      MSG_PRINT(("\treceivedLnkSt %i\n",(int)status.receivedLnkSt));
      MSG_PRINT(("\tlowPowerAbility %i\n",(int)status.lowPowerAbility));
    }

    /* XMDIO PCS EEE Capability: */
    retVal = madPCSEEECapability(dev,port,&capability);
    if (retVal != MAD_OK)
    {
      MSG_PRINT(("%x\t",(int)retVal));  
      sprintf(savedResultHead[count], "Get PCS EEE Capability Failed %x: ", retVal);
      savedResult[count++]=retVal;
    }
    else
    {
      MSG_PRINT(("port%1i XMDIO PCS EEE Capability:\n",(int)port));
      MSG_PRINT(("\tEEE10GBaseKR %i\n",(int)capability.EEE10GBaseKR));
      MSG_PRINT(("\tEEE10GBaseKX4 %i\n",(int)capability.EEE10GBaseKX4));
      MSG_PRINT(("\tEEE1000BaseKX %i\n",(int)capability.EEE1000BaseKX));
      MSG_PRINT(("\tEEE10GBaseT %i\n",(int)capability.EEE10GBaseT));
      MSG_PRINT(("\tEEE1000BaseT %i\n",(int)capability.EEE1000BaseT));
      MSG_PRINT(("\tEEE100BaseTX %i\n",(int)capability.EEE100BaseTX));
    }

    /* XMDIO PCS Wake Err Counter: */
    retVal = madGetPCSWakeErrCounter(dev,port,&data);
    if (retVal != MAD_OK)
    {
      MSG_PRINT(("%x\t",(int)retVal));  
      sprintf(savedResultHead[count], "Get PCS Wake Err Counter Failed %x: ", retVal);
      savedResult[count++]=retVal;
    }
    else
    {
      MSG_PRINT(("port%1i XMDIO PCS Wake Err Counter: %x\n",(int)port, data));
    }

    /* XMDIO PCS EEE Advertisement: */
    retVal = madPCSEEEAdvertisement(dev,port,&capability);
    if (retVal != MAD_OK)
    {
      MSG_PRINT(("%x\t",(int)retVal));  
      sprintf(savedResultHead[count], "Get PCS EEE Advertisement Failed %x: ", retVal);
      savedResult[count++]=retVal;
    }
    else
    {
      MSG_PRINT(("port%1i XMDIO PCS EEE Advertisement:\n",(int)port));
      MSG_PRINT(("\tEEE10GBaseKR %i\n",(int)capability.EEE10GBaseKR));
      MSG_PRINT(("\tEEE10GBaseKX4 %i\n",(int)capability.EEE10GBaseKX4));
      MSG_PRINT(("\tEEE1000BaseKX %i\n",(int)capability.EEE1000BaseKX));
      MSG_PRINT(("\tEEE10GBaseT %i\n",(int)capability.EEE10GBaseT));
      MSG_PRINT(("\tEEE1000BaseT %i\n",(int)capability.EEE1000BaseT));
      MSG_PRINT(("\tEEE100BaseTX %i\n",(int)capability.EEE100BaseTX));
    }

    /* XMDIO PCS EEE Link Partener Advertisement: */
    retVal = madPCSEEELnkPartnerAdvertisement(dev,port,&capability);
    if (retVal != MAD_OK)
    {
      MSG_PRINT(("%x\t",(int)retVal));  
      sprintf(savedResultHead[count], "Get PCS EEE Link Partener Advertisement Failed %x: ", retVal);
      savedResult[count++]=retVal;
    }
    else
    {
      MSG_PRINT(("port%1i XMDIO PCS EEE Link Partener Advertisement:\n",(int)port));
      MSG_PRINT(("\tEEE10GBaseKR %i\n",(int)capability.EEE10GBaseKR));
      MSG_PRINT(("\tEEE10GBaseKX4 %i\n",(int)capability.EEE10GBaseKX4));
      MSG_PRINT(("\tEEE1000BaseKX %i\n",(int)capability.EEE1000BaseKX));
      MSG_PRINT(("\tEEE10GBaseT %i\n",(int)capability.EEE10GBaseT));
      MSG_PRINT(("\tEEE1000BaseT %i\n",(int)capability.EEE1000BaseT));
      MSG_PRINT(("\tEEE100BaseTX %i\n",(int)capability.EEE100BaseTX));
    }

	/* Set EEE Buffer configuration. */
	retVal = madSetEEEBufferConfig(dev, port, ipg, en);
	if (retVal != MAD_OK)
	{
		MSG_PRINT(("%x\t", (int)retVal));
		sprintf(savedResultHead[count], " Set EEE Buffer configuration Failed %x: ", retVal);
		savedResult[count++] = retVal;
	}
	else
	{
		MSG_PRINT(("Set EEE Buffer configuration Success\n"));
	}

	/* Read EEE Buffer configuration. */
	retVal = madGetEEEBufferConfig(dev, port, &ipg, &en);
	if (retVal != MAD_OK)
	{
		MSG_PRINT(("%x\t",(int)retVal));  
		sprintf(savedResultHead[count], " Read EEE Buffer configuration Failed %x: ", retVal);
		savedResult[count++] = retVal;
	}
	else
	{
		MSG_PRINT(("\t IPG of Packets %i\n", (int)ipg));
		MSG_PRINT(("\t EEE Buffer Enable:  %i\n", (int)en));
	}

  }

  if (count)
#ifdef SHOW_DEBUG_INFO
    MSG_PRINT_DBG ("\n\nReturned error: \n"));
#else
    MSG_PRINT (("$$$ Show all returned error:\n"));
#endif
  for (i=0; i<count; i++)
  {
    MSG_PRINT (("%s", savedResultHead[i]));
#ifdef SHOW_DEBUG_INFO
    testMADDisplayStatus(savedResult[i]));
#endif
  }

  return MAD_OK;
}

