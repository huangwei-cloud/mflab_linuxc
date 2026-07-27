#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <math.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "app.h"
#include "../utils/utils.h"
#include "../hislip/hislip_svc.h"
#include "../axidma/libaxidma.h"
#include "../axispi/spi.h"
#include "../kfifo/kfifo.h"
#include "../axigpio/gpio.h"
#include "../clk/reg.h"
#include "../dac/dac3283.h"
#include "../adc/adc9684.h"
#include "../task/datatask.h"
#include "../pm/param.h"
#include "../utils/scpi-def.h"
#include "../ring/ringbuffer.h"
#include "../calib/calib_init.h"
#include "../sweep/sweep.h"
#include "../log/log.h"

#define PORT 6025
#define BUFFER_SIZE 1024


static ssize_t send_all(int fd, const void *buf, size_t len)
{
    const uint8_t *p = (const uint8_t*)buf;
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, p + sent, len - sent, 0);
        if (n > 0) {
            sent += (size_t)n;
        } else if (n < 0 && (errno == EINTR)) {
            continue;
        } else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            
            continue;
        } else {
            return -1;
        }
    }
    return (ssize_t)sent;
}

void tcp_data_server_thread(void *arg)
{
	int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_fd < 0) { perror("socket"); return NULL; }

	int yes = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(TCP_PORT);
	if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		perror("bind"); close(listen_fd); return NULL;
	}
	if (listen(listen_fd, 8) < 0) { perror("listen"); close(listen_fd); return NULL; }
	printf("TCP server listening on %d...\n", TCP_PORT);

	struct pollfd pfds[1 + MAX_CLIENTS];
	int client_fds[MAX_CLIENTS]; memset(client_fds, -1, sizeof(client_fds));
	int nfds;
	int map_idx[1 + MAX_CLIENTS];
	map_idx[0] = -1;


	pfds[0].fd = listen_fd;
	pfds[0].events = POLLIN;

	char send_buffer[FRAME_SIZE];
	size_t frame_len;
	uint32_t frame_id;
	FrameRingBuffer *frb_daq = NULL;
	get_rb_multiple_instance(&frb_daq);

	for (;;) {
		nfds = 1;

    	for (int k = 0; k < 1 + MAX_CLIENTS; ++k) map_idx[k] = -1;

		for (int i = 0; i < MAX_CLIENTS; ++i) {
			if (client_fds[i] >= 0) {
				pfds[nfds].fd = client_fds[i];
				pfds[nfds].events = POLLIN | POLLHUP | POLLERR;
				map_idx[nfds] = i;
				++nfds;
			}
		}


		int rc = poll(pfds, nfds, 1);
		if (rc < 0 && errno != EINTR) { perror("poll"); break; }

		if (pfds[0].revents & POLLIN) {
			struct sockaddr_in cli; socklen_t sl = sizeof(cli);
			int cfd = accept(listen_fd, (struct sockaddr*)&cli, &sl);
			if (cfd >= 0) {
				int one = 1; setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
				int placed = 0;
				for (int i = 0; i < MAX_CLIENTS; ++i) {
					if (client_fds[i] < 0) { client_fds[i] = cfd; placed = 1; break; }
				}

				if (!placed) { printf("client list full, closing\n"); close(cfd); }
				else printf("client connected: %s:%d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
			}
		}

		for (int pos = 1; pos < nfds; ++pos) {
			int i = map_idx[pos];
			if (i < 0) continue;
			if (pfds[pos].revents & (POLLHUP | POLLERR)) {
				close(client_fds[i]);
				printf("1 close client pos: %d fd: %d\r\n", pos, client_fds[i]);
				client_fds[i] = -1;
			}
		}

		int has_client = 0;
		for (int i = 0; i < MAX_CLIENTS; ++i) {
			if (client_fds[i] >= 0) {
				has_client = 1;
				break;
			}
		}
		if (!has_client) {
			usleep(20);
			continue;
		}

		for (int ch = CH_X1; ch < CH_MAX; ++ch) {
			if (frame_buffer_read(frb_daq + ch, (uint8_t*)send_buffer, &frame_len, &frame_id)) {
				TcpHdr hdr;
				hdr.magic		= htonl(0xABCD1234);
				hdr.ch			= htons((uint16_t)ch);
				hdr.reserved	= 0;
				hdr.frame_id	= htonl(frame_id);
				hdr.payload_len = htonl((uint32_t)frame_len);

				for (int i = 0; i < MAX_CLIENTS; ++i) {
					int fd = client_fds[i];
					if (fd < 0) continue;

					if (send_all(fd, &hdr, sizeof(hdr)) < 0 ||
						send_all(fd, send_buffer, frame_len) < 0) {
						close(fd);
						printf("2: close client = %d\r\n", client_fds[i]);
						client_fds[i] = -1;
					}
				}
			}
		}
		usleep(20);
	}

	for (int i = 0; i < MAX_CLIENTS; ++i) if (client_fds[i] >= 0) close(client_fds[i]);
	close(listen_fd);
	return NULL;

}

#if 1

void *udp_server_thread(void *arg)
{
	int ch;
    int server_socket;
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(client_addr);
    char send_buffer[FRAME_SIZE];
    size_t frame_len;
    uint32_t frame_id;
	UdpPkt udp_pkt;
	FrameRingBuffer *frb_daq = NULL;

	get_rb_multiple_instance(&frb_daq);

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        pthread_exit(NULL);
    }
    printf("UDP Server started on port %d...\n", PORT);

    char recv_buffer[16];
    int recv_len = recvfrom(server_socket, recv_buffer, sizeof(recv_buffer) - 1, 0,
                            (struct sockaddr *)&client_addr, &addr_len);
    if (recv_len < 0) {
        perror("recvfrom failed");
        close(server_socket);
        pthread_exit(NULL);
    }
	recv_buffer[recv_len] = '\0';
    printf("Received from %s:%d -> %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recv_buffer);

    while (1) {
		for(ch = CH_X1; ch < CH_MAX; ch++) {
			if (frame_buffer_read(frb_daq+ch, (uint8_t *)send_buffer, &frame_len, &frame_id)) {
	            int num_packets = (frame_len + UDP_PACKET_SIZE - 1) / UDP_PACKET_SIZE;

	            for (int i = 0; i < num_packets; ++i) {
	                size_t offset = i * UDP_PACKET_SIZE;
	                size_t pkt_len = (frame_len - offset >= UDP_PACKET_SIZE) ? UDP_PACKET_SIZE : (frame_len - offset);

	                udp_pkt.fid = htonl(frame_id);
	                udp_pkt.total = htons(num_packets);
	                udp_pkt.index = htons(i);
	                memcpy(udp_pkt.payload, send_buffer + offset, pkt_len);
					printf("pack:%d, %d\r\n", i, pkt_len);
					//printhex(udp_pkt.payload, 20, 20);


	                sendto(server_socket, &udp_pkt, sizeof(udp_pkt.fid) + sizeof(udp_pkt.total) + sizeof(udp_pkt.index) + pkt_len,
	                       0, (struct sockaddr *)&client_addr, addr_len);

					//usleep(10);
	            }
        	}
		}

        usleep(100); // 1ms
    }
}

#else
void *udp_server_thread(void *arg)
{
	int length;
	int server_socket;
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;
	socklen_t addr_len = sizeof(client_addr);
    char recv_buffer[BUFFER_SIZE];
	char send_buffer[BUFFER_SIZE];
	struct __kfifo *pfifo;

    get_kfifo_instance(&pfifo);

    // ���� UDP �׽���
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("socket creation failed");
        pthread_exit(NULL);
    }

    // �󶨵�ַ�Ͷ˿�
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        pthread_exit(NULL);
    }
	printf("UDP Server started on port %d...\n", PORT);

	// �������տͻ�������
    int recv_len = recvfrom(server_socket, recv_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
    if (recv_len < 0) {
        perror("recvfrom failed");
        close(server_socket);
        pthread_exit(NULL);
    }

	recv_buffer[recv_len] = '\0';  // ȷ���ַ�������
    printf("Received from %s:%d -> %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recv_buffer);

	while (1) {
        length = get_kfifo_validlen(pfifo);
		if(length >= BUFFER_SIZE) {
			__kfifo_out(pfifo, send_buffer, BUFFER_SIZE);
			sendto(server_socket, send_buffer, BUFFER_SIZE, 0,(struct sockaddr *)&client_addr, addr_len);
        	//printf("Sent to %s:%d -> %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), send_buffer);
        	//printf("Sent to %s:%d ->\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		}
		usleep(200);
    }
}
#endif

void *clk_thread(void *arg)
{
	uint32_t freq_l, freq_h;
	double current_freq, write_freq;
	double osc_ref_clock_hz;
	uint32_t osc_ext_source_cnt = 0;
	uint64_t read_freq_code, write_freq_code;
	uint32_t demod_ch;
	char freq_word_l_name[64];
	char freq_word_h_name[64];
	gLobalParam *glp = NULL;

	get_global_param(&glp);
	glp->g_dev_clk_source = -1;

	while(1) {

		reg_read_by_name("pll_freq_l", &freq_l);
		reg_read_by_name("pll_freq_h", &freq_h);

		osc_ref_clock_hz = zwdx_oscillator_ref_clock_hz();
		read_freq_code = ((uint64_t)freq_h << 32) | ((uint64_t)freq_l);
		current_freq = read_freq_code / pow(2,48) * osc_ref_clock_hz;

		for(demod_ch = 0; demod_ch < DEVICE_DEMOD_NUM_L500K; demod_ch++) {
			if(glp->g_demod_param[demod_ch].osc_source == EXT) {
				if (current_freq == glp->g_demod_param[demod_ch].osc_freq) {
					continue;
				}
				printf("CH%u->PLL-FREQ: %.8f, LPCALOSC-FREQ: %.8f\r\n",
					demod_ch + 1, current_freq, glp->g_demod_param[demod_ch].osc_freq);
				write_freq = current_freq * glp->g_demod_param[demod_ch].osc_harm;
				glp->g_demod_param[demod_ch].osc_freq = write_freq;
			} else if(glp->g_demod_param[demod_ch].osc_source == DOUBLE) {
				write_freq = fabs(current_freq - glp->g_demod_param[demod_ch].osc_freq) * glp->g_demod_param[demod_ch].osc_harm;
				printf("CH%u->PLL-FREQ: %.8f, LPCALOSC-FREQ: %.8f, DOUBLE-FREQ: %.8f\r\n",
					demod_ch + 1, current_freq, glp->g_demod_param[demod_ch].osc_freq, write_freq);
			} else {
				continue;
			}

			write_freq_code = (uint64_t)(write_freq * pow(2, 48) / osc_ref_clock_hz);
			snprintf(freq_word_l_name, sizeof(freq_word_l_name),
				"oscillators%u_freq_word_l", demod_ch + 1);
			snprintf(freq_word_h_name, sizeof(freq_word_h_name),
				"oscillators%u_freq_word_h", demod_ch + 1);

			reg_write_by_name(freq_word_l_name, (uint32_t)(write_freq_code & 0xFFFFFFFF));
			reg_write_by_name(freq_word_h_name, (uint32_t)((write_freq_code >> 32)& 0xFFFFFFFF));

			osc_ext_source_cnt += 1;
		}

		if(osc_ext_source_cnt != 0) {
			osc_ext_source_cnt = 0;
			/*start work*/
			reg_write_by_name("osc_work_all", 0);
			reg_write_by_name("osc_work_all", 1);

			if(glp->g_dev_trig_source == INT) { 
				send_soft_trig();
			}else if(glp->g_dev_trig_source == EXT){

			}
		}

		if(glp->g_dev_clk_source == INTERNAL_100M) {
			dev_clk_switch(INTERNAL_100M);
			glp->g_dev_clk_source = -1;
		}else if(glp->g_dev_clk_source == EXTERN_100M ) {
			dev_clk_switch(EXTERN_100M);
			glp->g_dev_clk_source = -1;
		}else if(glp->g_dev_clk_source == EXTERN_10M ) {
			dev_clk_switch(EXTERN_10M);
			glp->g_dev_clk_source = -1;
		}

		sleep(5);
	}

}

void init_fan_speed(double speed)
{
	uint32_t code = (uint32_t)(speed / 6850.0 * 1600.0);

	reg_write_by_name("fan_pwm_val", code);
}

void init()
{
	pthread_t hisliptid, udptid, bramtid, udpcmdid, extclkid, paramid, scanid;
	
	init_system_param();
	// init_calib_param();

	plog_init(
        P_LOG_LEVEL_DEBUG |
        P_LOG_LEVEL_INFO |
        P_LOG_LEVEL_WARNING |
        P_LOG_LEVEL_ERROR |
        P_LOG_LEVEL_FATAL);

	// kfifo_init();
	init_reg();
	init_scpi_commands();

	reset_gpio();
	init_mem();
	init_clk(INTERNAL_100M);
	init_adc9684();

	init_dma();

	init_fan_speed(2500.0);
	init_param();
	init_default_param();

	pthread_create(&udptid, NULL, tcp_data_server_thread, NULL);

	// pthread_create(&udpcmdid, NULL, udp_cmd_thread, NULL);

	pthread_create(&bramtid, NULL, data_server_thread, NULL);

	// pthread_create(&extclkid, NULL, clk_thread, NULL);

	//pthread_create(&paramid, NULL, param_thread, NULL);

	// pthread_create(&scanid, NULL, &scan_thread, NULL);

	pthread_create(&hisliptid, NULL, hislip_service_thread, NULL);

	// printf("init config finish...\r\n");


}

void dev_clk_switch(RefClk rc)
{
	reset_gpio();
	init_clk(rc);
	init_adc9684();
	init_dac3283();
	init_gpio();

}

void test_modules()
{
	test_axidma_module();
	//test_spidev();
}


