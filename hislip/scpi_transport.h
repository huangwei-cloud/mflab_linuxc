#ifndef HISLIP_SCPI_TRANSPORT_H
#define HISLIP_SCPI_TRANSPORT_H

#include <stddef.h>

#include "../pm/param.h"

int scpi_transport_init(void);
int scpi_transport_input(const char *data, size_t len);
int scpi_transport_has_output(void);
size_t scpi_transport_output_size(void);
size_t scpi_transport_read(char *data, size_t max_len);
void apply_param_items_by_scpi(const ParamItem *items, int count);

#endif
