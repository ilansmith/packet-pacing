#include <sys/types.h>
#include <sys/socket.h>

#include "pacing.h"

int pacing::paced_sendto(int fd, const void *buf, size_t nbytes, int flags,
		const struct sockaddr *to, socklen_t tolen)
{
	int ret;

	ret = sendto(fd, buf, nbytes, flags, to, tolen);
	if (ret < 0) {
		return ret;
	}

	m_counter_next_dummy += DUMMY_RATIO;
	while (m_counter_next_dummy > 1.0) {
		m_counter_next_dummy -= 1.0;
		if (sendto(fd, buf, nbytes, DUMMY_SPOOF_MAC, to, tolen) < 0) {
			return -1;
		}
	}

	return ret;
}

