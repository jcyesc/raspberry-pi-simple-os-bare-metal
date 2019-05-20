
#ifndef THREADS_INIT_H_
#define THREADS_INIT_H_

/* Initializes the Operating System. The interruptions have to be disabled at entrance.
*
*
*  - Sets interruptions
*  - Sets the periodic timer
*  - Set the thread functionality
*
*  This function is called by the main() function defined in arm_asm/start.s file.
*/
void init();

#endif /* THREADS_INIT_H_ */
