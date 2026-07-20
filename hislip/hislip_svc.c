#include "hislip_svc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <hislip/server.h>

#include "scpi_transport.h"

#define HISLIP_SUBADDRESS "hislip0"
#define HISLIP_MESSAGE_SIZE_MAX (1024 * 1024)

static hs_server_config_t g_hislip_config;
static hs_server_t g_hislip_server;
static hs_subaddress_callbacks_t g_hislip_callbacks;
static int g_hislip_initialized = 0;

static int hislip_message_sync(int socket, uint32_t message_id, void *buffer, int length, int timeout)
{
    char *response = NULL;
    size_t response_size;
    size_t response_len = 0;
    int ret;

    if (length > 0 && buffer != NULL) {
        if (scpi_transport_input((const char *)buffer, (size_t)length) != 0) {
            return hs_server_send_response(socket, message_id, NULL, 0, timeout);
        }
    }

    response_size = scpi_transport_output_size();
    if (response_size > 0) {
        response = (char *)malloc(response_size);
        if (response == NULL) {
            return hs_server_send_response(socket, message_id, NULL, 0, timeout);
        }

        response_len = scpi_transport_read(response, response_size);
    }

    ret = hs_server_send_response(socket,
                                  message_id,
                                  response_len > 0 ? response : NULL,
                                  (int)response_len,
                                  timeout);

    free(response);
    return ret;
}

static int hislip_message_async(int socket, uint32_t message_id, void *buffer, int length, int timeout)
{
    (void)buffer;
    (void)length;
    return hs_server_send_response(socket, message_id, NULL, 0, timeout);
}

int start_hislip_service(void)
{
    if (g_hislip_initialized) {
        return 0;
    }

    if (scpi_transport_init() != 0) {
        fprintf(stderr, "SCPI transport init failed\r\n");
        return -1;
    }

    hs_server_config_init(&g_hislip_config);
    g_hislip_config.connections_max = 4;
    g_hislip_config.message_size_max = HISLIP_MESSAGE_SIZE_MAX;
    g_hislip_config.message_timeout = 5000;

    if (hs_server_init(&g_hislip_server, &g_hislip_config) != 0) {
        fprintf(stderr, "HiSLIP server init failed\r\n");
        return -1;
    }

    g_hislip_callbacks.message_sync = hislip_message_sync;
    g_hislip_callbacks.message_async = hislip_message_async;

    if (hs_server_register_subaddress(&g_hislip_server, HISLIP_SUBADDRESS, &g_hislip_callbacks) != 0) {
        fprintf(stderr, "HiSLIP subaddress register failed\r\n");
        return -1;
    }

    g_hislip_initialized = 1;
    return 0;
}

void *hislip_service_thread(void *arg)
{
    (void)arg;

    if (start_hislip_service() != 0) {
        return NULL;
    }

    hs_server_run(&g_hislip_server);
    return NULL;
}
