#include  "AQ_PlatformRoutines.h"
#include <unistd.h>
//#include <winsock.h>

void AQ_API_Wait
(
  uint32_t milliseconds,     /*!< The delay in milliseconds */
  AQ_API_Port* port          /*!< The PHY to use if delay reads are necessary*/
)
{
    usleep(milliseconds*1000);
}

