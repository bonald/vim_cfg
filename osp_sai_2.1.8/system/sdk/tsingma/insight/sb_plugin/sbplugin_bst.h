

#include "insight.h"
#include "bst.h"
#include "system.h"



/*********************************************************************
* @brief  Get snapshot of all thresholds configured
*
*
* @param[in]   asic                          unit
* @param[out]  thresholdSnapshot             threshold snapshot
* @param[out]  time                          time
*
* @retval   BVIEW_STATUS_FAILURE      Due to lock acquistion failure or
*                                     Not able to get asic type of this unit or
*                                     BST feature is not present or
*                                     BST south bound function has returned failure
*
* @retval   BVIEW_STATUS_SUCCESS      Snapshot get is successful
*
* @retval   BVIEW_STATUS_UNSUPPORTED  Snapshot get functionality is
*                                     not supported on this unit
*
*
* @notes    none
*
*
*********************************************************************/
INSIGHT_STATUS
sbapi_bst_threshold_get (int asic,
                         INSIGHT_BST_ASIC_SNAPSHOT_DATA_t *thresholdSnapshot,
                         INSIGHT_TIME_t * time);


/*********************************************************************
* @brief       Get BST snapshot
*
* @param[in]     asic                  Unit number
* @param[out]    snapshot              BST snapshot
* @param[out]    time                  Time
*
* @retval   BVIEW_STATUS_FAILURE      Due to lock acquistion failure or
*                                     Not able to get asic type of this unit or
*                                     BST feature is not present or
*                                     BST south bound function has returned failure
*
* @retval   BVIEW_STATUS_SUCCESS      BST snapshot get is successful
*
* @retval   BVIEW_STATUS_UNSUPPORTED  BST snapshot get functionality is
*                                     not supported on this unit
*
* @notes    none
*
*********************************************************************/

INSIGHT_STATUS
sbapi_bst_snapshot_get (int asic,
                        INSIGHT_BST_ASIC_SNAPSHOT_DATA_t * snapshot,
                        INSIGHT_TIME_t * time);


/*********************************************************************
* @brief  Get snapshot of max buffers allocated
*
*
* @param[in]   asic                          unit
* @param[out]  maxBufSnapshot                Max buffers snapshot
* @param[out]  time                          time
*
* @retval   BVIEW_STATUS_FAILURE      Due to lock acquistion failure or
*                                     Not able to get asic type of this unit or
*                                     system feature is not present or
*                                     System south bound function has returned failure
*
* @retval   BVIEW_STATUS_SUCCESS      Snapshot get is successful
*
* @retval   BVIEW_STATUS_UNSUPPORTED  Snapshot get functionality is
*                                     not supported on this unit
*
*
* @notes    none
*
*
*********************************************************************/
INSIGHT_STATUS
sbapi_system_max_buf_snapshot_get (int asic,
                                   INSIGHT_SYSTEM_ASIC_MAX_BUF_SNAPSHOT_DATA_t *data,
                                   INSIGHT_TIME_t * time);