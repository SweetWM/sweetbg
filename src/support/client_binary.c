#include "support/client_binary.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __FreeBSD__
#include <sys/sysctl.h>
#include <sys/types.h>
#endif

static bool executable_path(char *out, size_t out_size) {
#ifdef __FreeBSD__
	int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
	size_t length = out_size;
	return sysctl(mib, 4, out, &length, NULL, 0) == 0 && length > 0 &&
	       length <= out_size && out[length - 1] == '\0';
#else
	// The kernel supplies this fixed link; validate length and add the NUL
	ssize_t length = readlink("/proc/self/exe", out, out_size - 1);
	if (length <= 0 || (size_t)length >= out_size - 1) {
		return false;
	}
	// NOLINTNEXTLINE(clang-analyzer-security.ArrayBound)
	out[length] = '\0';
	return true;
#endif
}

void sweetbg_client_binary(char *out, size_t out_size) {
	if (out_size == 0) {
		return;
	}
	if (executable_path(out, out_size)) {
		char *slash = strrchr(out, '/');
		if (slash != NULL) {
			size_t dir_len = (size_t)(slash - out) + 1;
			const char bin[] = "sweetbg";
			if (out_size >= sizeof(bin) &&
				dir_len <= out_size - sizeof(bin)) {
				memcpy(out + dir_len, bin, sizeof(bin));
				return;
			}
		}
	}
	snprintf(out, out_size, "sweetbg");
}
