/*
 * sys_factory.h
 *
 *  Created on: Dec 9, 2025
 *      Author: Antigravity
 */

#ifndef FACTORY_SYS_FACTORY_H_
#define FACTORY_SYS_FACTORY_H_

#include "sys.h"

/**
 * @brief Get the platform specific system operations
 * @return Pointer to SysCoreOps
 */
const SysCoreOps *SysFactory_GetOps(void);

#endif /* FACTORY_SYS_FACTORY_H_ */
