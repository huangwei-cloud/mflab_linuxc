#include "scpi_transport.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../pm/param.h"
#include "../utils/scpi-def.h"
#include "units.h"

#define SCPI_OUTPUT_BUFFER_SIZE (1024 * 1024)

static char g_scpi_output_buffer[SCPI_OUTPUT_BUFFER_SIZE];
static size_t g_scpi_output_wr_idx = 0;
static size_t g_scpi_output_rd_idx = 0;
static int g_scpi_initialized = 0;
static pthread_mutex_t g_scpi_lock = PTHREAD_MUTEX_INITIALIZER;

size_t SCPI_Write(scpi_t *context, const char *data, size_t len)
{
    size_t write_len = len;

    if (STB_MAV & SCPI_RegGet(context, SCPI_REG_STB)) {
        SCPI_RegClearBits(context, SCPI_REG_STB, STB_MAV);
        SCPI_ErrorPush(context, SCPI_ERROR_QUERY_INTERRUPTED);
        g_scpi_output_wr_idx = 0;
        g_scpi_output_rd_idx = 0;
    }

    if ((g_scpi_output_wr_idx + write_len) > (SCPI_OUTPUT_BUFFER_SIZE - 1)) {
        write_len = (SCPI_OUTPUT_BUFFER_SIZE - 1) - g_scpi_output_wr_idx;
    }

    if (write_len > 0) {
        memcpy(&g_scpi_output_buffer[g_scpi_output_wr_idx], data, write_len);
        g_scpi_output_wr_idx += write_len;
    }

    g_scpi_output_buffer[g_scpi_output_wr_idx] = '\0';
    return write_len;
}

scpi_result_t SCPI_Flush(scpi_t *context)
{
    SCPI_RegSetBits(context, SCPI_REG_STB, STB_MAV);
    g_scpi_output_rd_idx = 0;
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t *context, int_fast16_t err)
{
    (void)context;
    fprintf(stderr, "**ERROR: %d, \"%s\"\r\n", (int16_t)err, SCPI_ErrorTranslate(err));
    return 0;
}

scpi_result_t SCPI_Control(scpi_t *context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val)
{
    (void)context;

    if (SCPI_CTRL_SRQ == ctrl) {
        fprintf(stderr, "**SRQ: 0x%X (%d)\r\n", val, val);
    } else {
        fprintf(stderr, "**CTRL %02x: 0x%X (%d)\r\n", ctrl, val, val);
    }

    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t *context)
{
    (void)context;
    fprintf(stderr, "**Reset\r\n");
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t *context)
{
    (void)context;
    return SCPI_RES_ERR;
}

int scpi_transport_init(void)
{
    SystemConfig *psc = NULL;
    static char version_ext[STR_SMALL_LEN * 2];

    pthread_mutex_lock(&g_scpi_lock);

    if (g_scpi_initialized) {
        pthread_mutex_unlock(&g_scpi_lock);
        return 0;
    }

    scpi_context.user_context = NULL;
    get_system_param(&psc);
    snprintf(version_ext, sizeof(version_ext), "%s,%s", psc->version, psc->mac);

    SCPI_Init(&scpi_context,
              scpi_commands,
              &scpi_interface,
              scpi_units_def,
              psc->vendors, psc->model, psc->serial, version_ext,
              scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
              scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

    g_scpi_output_wr_idx = 0;
    g_scpi_output_rd_idx = 0;
    g_scpi_initialized = 1;

    pthread_mutex_unlock(&g_scpi_lock);
    return 0;
}

int scpi_transport_input(const char *data, size_t len)
{
    if (data == NULL || len == 0) {
        return 0;
    }

    if (!g_scpi_initialized && scpi_transport_init() != 0) {
        return -1;
    }

    pthread_mutex_lock(&g_scpi_lock);
    SCPI_Input(&scpi_context, data, len);
    pthread_mutex_unlock(&g_scpi_lock);

    return 0;
}

void apply_param_items_by_scpi(const ParamItem *items, int count)
{
    if (items == NULL || count <= 0) {
        return;
    }

    /* Skip system parameters. They are not hardware configuration commands. */
    for (int i = 20; i < count; ++i) {
        const ParamItem *item = &items[i];
        char buffer[1024] = {0};
        int offset = snprintf(buffer, sizeof(buffer), "%s ", item->scpi_cmd);

        for (int j = 0; j < item->param_count && offset < (int)sizeof(buffer) - 1; ++j) {
            char temp[256] = {0};

            switch (item->values[j].type) {
                case PARAM_INT32:
                    snprintf(temp, sizeof(temp), "%d", item->values[j].value.i32);
                    break;
                case PARAM_UINT32:
                    snprintf(temp, sizeof(temp), "%u", item->values[j].value.u32);
                    break;
                case PARAM_INT64:
                    snprintf(temp, sizeof(temp), "%lld", item->values[j].value.i64);
                    break;
                case PARAM_UINT64:
                    snprintf(temp, sizeof(temp), "%llu", item->values[j].value.u64);
                    break;
                case PARAM_DOUBLE:
                    snprintf(temp, sizeof(temp), "%.10g", item->values[j].value.d);
                    break;
                case PARAM_STRING:
                    snprintf(temp, sizeof(temp), "%s", item->values[j].value.str);
                    break;
            }

            if (j != 0) {
                offset += snprintf(buffer + offset, sizeof(buffer) - offset, ",");
            }
            offset += snprintf(buffer + offset, sizeof(buffer) - offset, "%s", temp);
        }

        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");

        if (scpi_transport_input(buffer, strlen(buffer)) != 0) {
            fprintf(stderr, "SCPI command failed: %s\n", buffer);
        }
    }
}

int scpi_transport_has_output(void)
{
    int has_output;

    pthread_mutex_lock(&g_scpi_lock);
    has_output = ((STB_MAV & SCPI_RegGet(&scpi_context, SCPI_REG_STB)) != 0);
    pthread_mutex_unlock(&g_scpi_lock);

    return has_output;
}

size_t scpi_transport_output_size(void)
{
    size_t size = 0;

    pthread_mutex_lock(&g_scpi_lock);
    if (STB_MAV & SCPI_RegGet(&scpi_context, SCPI_REG_STB)) {
        size = g_scpi_output_wr_idx - g_scpi_output_rd_idx;
    }
    pthread_mutex_unlock(&g_scpi_lock);

    return size;
}

size_t scpi_transport_read(char *data, size_t max_len)
{
    size_t in_buffer;
    size_t to_copy;

    if (data == NULL || max_len == 0) {
        return 0;
    }

    pthread_mutex_lock(&g_scpi_lock);

    if (!(STB_MAV & SCPI_RegGet(&scpi_context, SCPI_REG_STB))) {
        pthread_mutex_unlock(&g_scpi_lock);
        return 0;
    }

    in_buffer = g_scpi_output_wr_idx - g_scpi_output_rd_idx;
    to_copy = (max_len > in_buffer) ? in_buffer : max_len;

    memcpy(data, &g_scpi_output_buffer[g_scpi_output_rd_idx], to_copy);
    g_scpi_output_rd_idx += to_copy;

    if (g_scpi_output_rd_idx == g_scpi_output_wr_idx) {
        SCPI_RegClearBits(&scpi_context, SCPI_REG_STB, STB_MAV);
        g_scpi_output_wr_idx = 0;
        g_scpi_output_rd_idx = 0;
    }

    pthread_mutex_unlock(&g_scpi_lock);
    return to_copy;
}
