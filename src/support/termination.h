#ifndef SWEETBG_TERMINATION_H
#define SWEETBG_TERMINATION_H

// Caller closes the fd; readability means SIGINT or SIGTERM was received
int sweetbg_termination_fd(void);

#endif
