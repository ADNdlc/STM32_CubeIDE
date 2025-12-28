#ifndef APPLICATION_SYS_INIT_H_
#define APPLICATION_SYS_INIT_H_

/**
 * @brief Initialize all foundational system services
 *
 * Includes Flash, FileSystem, Network, and System State.
 * This should be called before UI initialization and resource burning.
 *
 * @return 0 on success, non-zero on error
 */
int sys_services_init(void);

#endif /* APPLICATION_SYS_INIT_H_ */
