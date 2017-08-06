#ifndef PACING_H
#define PACING_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PACKETS_PER_BURST (2)
#define BURSTS_PER_SEC (5)
#define USER_RATE_MBITPSEC (38.81070039)
#define HCA_RATE_MBITPSEC (41.382)

#define DUMMY_SPOOF_MAC (0x44000000)
#define USE_MTU_PACKETS (0)

#define MBITPSEC_TO_BYTEPSEC(RATE) ((RATE) * 1000000 / 8)
#define PACKETS_PER_FRAME(RATE, PACKET_SIZE) \
	(MBITPSEC_TO_BYTEPSEC((RATE)) / (PACKET_SIZE) / (BURSTS_PER_SEC))

/* Headers: ETH+IP+UDP, 42 bytes */
#if USE_MTU_PACKETS == 0
#define PACKET_TOTAL_SIZE (1356)
#else
#if USE_MTU_PACKETS == 1
#define PACKET_TOTAL_SIZE (1514)
#endif
#endif

#define EXACT_USER_PACKET_PER_BURST (PACKETS_PER_FRAME(USER_RATE_MBITPSEC, PACKET_TOTAL_SIZE))
#define TOTAL_PACKET_PER_BURST (PACKETS_PER_FRAME(HCA_RATE_MBITPSEC, PACKET_TOTAL_SIZE))
#define DUMMY_RATIO ((TOTAL_PACKET_PER_BURST - EXACT_USER_PACKET_PER_BURST) / \
		EXACT_USER_PACKET_PER_BURST)




class pacing {
public:
	pacing() : m_counter_next_dummy(0.0) {};
	int paced_sendto(int fd, const void *buf, size_t nbytes, int flags, const struct sockaddr *to, socklen_t tolen);
private:
	float m_counter_next_dummy;
};
#endif // PACING_H
