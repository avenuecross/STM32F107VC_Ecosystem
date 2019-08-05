#ifndef __DRIVER_COMMON_H
#define __DRIVER_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define ARM_DRIVER_OK                  0 ///< Operation succeeded 
#define ARM_DRIVER_ERROR              -1 ///< Unspecified error
#define ARM_DRIVER_ERROR_BUSY         -2 ///< Driver is busy
#define ARM_DRIVER_ERROR_TIMEOUT      -3 ///< Timeout occurred
#define ARM_DRIVER_ERROR_UNSUPPORTED  -4 ///< Operation not supported
#define ARM_DRIVER_ERROR_PARAMETER    -5 ///< Parameter error
#define ARM_DRIVER_ERROR_SPECIFIC     -6 ///< Start of driver specific errors 

#endif
