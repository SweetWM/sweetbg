#include "support/termination.h"

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <unistd.h>

#ifdef __FreeBSD__
#include <fcntl.h>
#include <sys/event.h>
#include <sys/types.h>
#else
#include <sys/signalfd.h>
#endif

#ifdef __FreeBSD__
static int signal_queue(void) {
	int saved_errno;
	int fd = kqueue();
	if (fd < 0) {
		return -1;
	}
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) < 0) {
		goto fail;
	}
	struct kevent events[2] = {0};
	EV_SET(&events[0], SIGINT, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
	EV_SET(&events[1], SIGTERM, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
	if (kevent(fd, events, 2, NULL, 0, NULL) < 0) {
		goto fail;
	}
	// Signals blocked before registration must also wake the queue
	sigset_t pending;
	if (sigpending(&pending) < 0) {
		goto fail;
	}
	if ((sigismember(&pending, SIGINT) == 1 ||
		    sigismember(&pending, SIGTERM) == 1) &&
		kill(getpid(), SIGTERM) < 0) {
		goto fail;
	}
	return fd;

fail:
	saved_errno = errno;
	close(fd);
	errno = saved_errno;
	return -1;
}
#endif

int sweetbg_termination_fd(void) {
	struct sigaction chld = {.sa_handler = SIG_IGN};
	sigemptyset(&chld.sa_mask);
	if (sigaction(SIGCHLD, &chld, NULL) < 0) {
		return -1;
	}
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);
	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		return -1;
	}
#ifdef __FreeBSD__
	return signal_queue();
#else
	return signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
#endif
}
