
#ifndef __VERSION_H__
#define __VERSION_H__

#define TAP_VERSION         "3.0.3.r3" 
#define OF_VERSION          "2.1.7.55.1"
#define L2PLUS_VERSION      "0.0.0.15.it"
#define SAI_VERSION         "2.1.8"
#define OFDPA_VERSION         "1.0.0.0"

#if defined TAPPRODUCT
#define OSP_VERSION         TAP_VERSION
#elif defined OFPRODUCT
#define OSP_VERSION         OF_VERSION
#elif defined SAIPRODUCT
#define OSP_VERSION         SAI_VERSION
#elif defined OFDPAPRODUCT
#define OSP_VERSION         OFDPA_VERSION
#else
#define OSP_VERSION         L2PLUS_VERSION
#endif

#define CENTEC_VERSION      OSP_VERSION
#define DEBIAN_VERSION      "1.0.1"

#endif /* !__VERSION_H__ */
