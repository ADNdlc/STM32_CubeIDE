#ifndef APPLICATION_HOME_RES_BURNER_H_
#define APPLICATION_HOME_RES_BURNER_H_

/**
 * @brief Run the resource burning process
 *
 * This will write compiled-in image data to LittleFS if enabled.
 * Should be called after flash_handler is initialized.
 */
void res_burner_run(void);

#endif /* APPLICATION_HOME_RES_BURNER_H_ */
