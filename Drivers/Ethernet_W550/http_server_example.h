/*
 * http_server_example.h
 *
 *  Created on: Nov 15, 2025
 *      Author: controllerstech
 */
#ifndef ETHERNET_HTTP_SERVER_EXAMPLE_H_
#define ETHERNET_HTTP_SERVER_EXAMPLE_H_

#include "httpServer/httpParser.h"
#include <stdint.h>

/* HTTP Server Functions */
void httpServer_setup(void);
void httpServerRun(void);

/* UDP Server Functions - Added by MJ */
void udpServer_setup(void);
void udpServerRun(void);

/* CGI Processors */
uint8_t predefined_get_cgi_processor(uint8_t * uri_name, uint8_t * buf, uint16_t * len);
uint8_t predefined_set_cgi_processor(uint8_t * uri_name, uint8_t * uri, uint8_t * buf, uint16_t * len);

/* Old handler (if still needed) */
// void http_dynamic_handler(st_http_request * p_http_request);

#endif /* ETHERNET_HTTP_SERVER_EXAMPLE_H_ */
