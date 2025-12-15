/*
 * http_server_example.c
 *
 *  Created on: Dec 14, 2025
 *      Author: MJ - UDP Data Gateway (WITH CHAT HISTORY)
 */

#include "main.h"
#include "wizchip_conf.h"
#include "socket.h"
#include "httpServer/httpServer.h"
#include "httpServer/httpParser.h"
#include "httpServer/httpUtil.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define HTTP_MAX_SOCK 2
#define UDP_SOCKET 2
#define UDP_PORT 5000

// ===== TAMBAHAN: Message History =====
#define MAX_MESSAGES 20
#define MAX_MESSAGE_LEN 128

typedef struct {
    uint8_t message[MAX_MESSAGE_LEN];
    uint8_t source_ip[4];
    uint16_t source_port;
    uint32_t timestamp;
    uint8_t is_sent;  // 0 = received from device, 1 = sent from web
} message_t;

message_t message_history[MAX_MESSAGES];
uint8_t message_count = 0;
uint8_t message_index = 0;  // Circular buffer index
// ===== END TAMBAHAN =====

uint8_t sockList[] = {0, 1};

uint8_t rx_buf[1024];
uint8_t tx_buf[1024];

// UDP Data Storage
uint8_t udp_rx_buffer[512];
uint16_t udp_data_len = 0;
uint8_t remote_ip[4] = {0};
uint16_t remote_port = 0;
uint32_t last_receive_time = 0;

// Debug tracking
uint8_t last_send_status[128] = "Not sent yet";
uint32_t last_send_time = 0;
int32_t last_send_result = 0;

// Function to get current tick
extern uint32_t HAL_GetTick(void);

// ===== TAMBAHAN: Function to add message to history =====
void add_message_to_history(const char* msg, uint8_t* ip, uint16_t port, uint8_t is_sent)
{
    message_t* m = &message_history[message_index];

    // Copy message
    strncpy((char*)m->message, msg, MAX_MESSAGE_LEN - 1);
    m->message[MAX_MESSAGE_LEN - 1] = '\0';

    // Copy IP and port
    memcpy(m->source_ip, ip, 4);
    m->source_port = port;

    // Timestamp
    m->timestamp = HAL_GetTick();
    m->is_sent = is_sent;

    // Increment circular buffer
    message_index = (message_index + 1) % MAX_MESSAGES;
    if(message_count < MAX_MESSAGES) message_count++;
}
// ===== END TAMBAHAN =====


const char index_page[] =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta charset='UTF-8'>"
"<title>W5500 UDP Gateway</title>"
"<style>"
"body{font-family:Arial;background:#eef;padding:20px;}"
".card{background:#fff;padding:20px;border-radius:10px;margin:15px 0;box-shadow:0 0 10px #aaa;}"
"button{padding:10px 20px;margin:5px;background:#4CAF50;color:#fff;border:none;cursor:pointer;border-radius:5px;}"
"button:hover{background:#45a049;}"
"input[type='text']{padding:8px;width:300px;border:1px solid #ddd;border-radius:4px;}"
".status{padding:10px;background:#e8f5e9;border-radius:5px;margin:10px 0;}"
".data-display{background:#f5f5f5;padding:15px;border-radius:5px;font-family:monospace;min-height:100px;max-height:400px;overflow-y:auto;}"
".error{background:#ffebee;color:#c62828;}"
".success{background:#e8f5e9;color:#2e7d32;}"

/* TAMBAHAN: Chat message styles */
".chat-message{margin:8px 0;padding:8px;border-radius:8px;clear:both;}"
".msg-received{background:#e3f2fd;border-left:4px solid #2196F3;}"
".msg-sent{background:#e8f5e9;border-left:4px solid #4CAF50;}"
".msg-header{font-size:11px;color:#666;margin-bottom:4px;}"
".msg-text{font-size:13px;color:#333;}"

"</style>"
"</head>"
"<body>"

"<h2>W5500 UDP Data Gateway</h2>"

/*-------------------- UDP Data Display ------------------------*/
"<div class='card'>"
"<h3>Received UDP Data</h3>"
"<div class='status'>"
"<strong>Source:</strong> <span id='sourceIP'>-</span><br>"
"<strong>Last Update:</strong> <span id='lastUpdate'>Never</span>"
"</div>"
"<div class='data-display' id='udpData'>Waiting for data...</div>"
"</div>"

"<script>"
"var updateTimer = null;"
"var lastMsgCount = 0;"

/* MODIFIKASI: Update function sekarang load chat messages */
"function updateUDPData(){"
" fetch('udp_data.cgi')"
" .then(r=>{if(!r.ok) throw new Error('HTTP '+r.status); return r.json();})"
" .then(data=>{"
" if(data.has_data){"
" document.getElementById('sourceIP').innerHTML = data.source_ip + ':' + data.source_port;"
" document.getElementById('lastUpdate').innerHTML = data.age + 's ago';"
" }"
" })"
" .catch(e=>console.log('Fetch error:',e));"
" "
" /* Load chat messages */"
" fetch('udp_messages.cgi')"
" .then(r=>r.json())"
" .then(data=>{"
" if(data.count > 0){"
" var html = '';"
" for(var i=0; i<data.messages.length; i++){"
" var msg = data.messages[i];"
" var msgClass = msg.is_sent == 0 ? 'msg-received' : 'msg-sent';"
" var direction = msg.is_sent == 0 ? 'FROM' : 'TO';"
" html += '<div class=\"chat-message '+msgClass+'\">';"
" html += '<div class=\"msg-header\">'+direction+' '+msg.source+' • '+msg.time+'</div>';"
" html += '<div class=\"msg-text\">'+msg.text+'</div>';"
" html += '</div>';"
" }"
" document.getElementById('udpData').innerHTML = html;"
" if(data.count != lastMsgCount){"
" document.getElementById('udpData').scrollTop = document.getElementById('udpData').scrollHeight;"
" lastMsgCount = data.count;"
" }"
" }else{"
" document.getElementById('udpData').innerHTML = 'Waiting for data...';"
" }"
" })"
" .catch(e=>console.log('Messages fetch error:',e));"
"}"

"updateTimer = setInterval(updateUDPData, 2000);"
"updateUDPData();"
"window.addEventListener('beforeunload',function(){clearInterval(updateTimer);});"
"</script>"

/*-------------------- UDP Send Command ------------------------*/
"<div class='card'>"
"<h3>Send UDP Command</h3>"
"<input type='text' id='cmdInput' placeholder='Enter command to send...' onkeypress='if(event.key===\"Enter\")sendUDP()'>"
"<button onclick='sendUDP()'>Send</button>"
"<div id='sendStatus' style='margin-top:10px;'></div>"
"</div>"

/*-------------------- Debug Info ------------------------*/
"<div class='card'>"
"<h3>Debug Information</h3>"
"<div id='debugInfo' style='font-family:monospace;background:#f5f5f5;padding:10px;border-radius:5px;'>"
"Loading..."
"</div>"
"</div>"

"<script>"
"function sendUDP(){"
" var cmd = document.getElementById('cmdInput').value;"
" if(!cmd){"
" alert('Please enter a command');"
" return;"
" }"
" document.getElementById('sendStatus').innerHTML='<strong>Sending...</strong>';"
" fetch('udp_send.cgi',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},"
" body:'cmd='+encodeURIComponent(cmd)})"
" .then(r=>r.text())"
" .then(t=>{"
" var statusDiv = document.getElementById('sendStatus');"
" statusDiv.innerHTML='<strong>Status:</strong> '+t;"
" if(t.indexOf('SUCCESS')>=0){"
" statusDiv.className='status success';"
" document.getElementById('cmdInput').value='';"
" updateUDPData();"  /* Refresh messages */
" }else if(t.indexOf('ERROR')>=0 || t.indexOf('FAILED')>=0){"
" statusDiv.className='status error';"
" }else{"
" statusDiv.className='status';"
" }"
" updateDebug();"
" })"
" .catch(e=>{"
" document.getElementById('sendStatus').innerHTML='<strong>Error:</strong> '+e;"
" });"
"}"
"</script>"

"<script>"
"function updateDebug(){"
" fetch('udp_debug.cgi')"
" .then(r=>r.json())"
" .then(data=>{"
" var html = '<strong>Target IP:</strong> ' + data.target_ip + '<br>';"
" html += '<strong>Target Port:</strong> ' + data.target_port + '<br>';"
" html += '<strong>Last Send Status:</strong> ' + data.last_send_status + '<br>';"
" html += '<strong>Last Send Time:</strong> ' + data.last_send_time + '<br>';"
" html += '<strong>Socket Status:</strong> ' + data.socket_status;"
" document.getElementById('debugInfo').innerHTML = html;"
" });"
"}"
"setInterval(updateDebug, 2000);"
"updateDebug();"
"</script>"

"</body>"
"</html>";


// HTTP Server Setup
void httpServer_setup(void)
{
    httpServer_init(tx_buf, rx_buf, HTTP_MAX_SOCK, sockList);
    reg_httpServer_cbfunc(NVIC_SystemReset, NULL);

    /* Register Static pages */
    reg_httpServer_webContent((uint8_t*)"index.html", (uint8_t*)index_page);
}

// UDP Server Setup
void udpServer_setup(void)
{
    // Set retry time and count for ARP
    setRCR(10);  // Retry count: 10 times
    setRTR(2000); // Retry timeout: 200ms (unit is 100us, so 2000 = 200ms)

    uint8_t ret = socket(UDP_SOCKET, Sn_MR_UDP, UDP_PORT, 0);
    if(ret != UDP_SOCKET)
    {
        // Socket creation failed - could log via UART here
    }
}

// UDP Server Run - Call this in main loop
void udpServerRun(void)
{
    uint16_t len;

    switch(getSn_SR(UDP_SOCKET))
    {
        case SOCK_UDP:
            len = getSn_RX_RSR(UDP_SOCKET);
            if(len > 0)
            {
                if(len > sizeof(udp_rx_buffer))
                    len = sizeof(udp_rx_buffer);

                // Receive UDP data
                udp_data_len = recvfrom(UDP_SOCKET, udp_rx_buffer, len, remote_ip, &remote_port);

                if(udp_data_len > 0)
                {
                    // Null-terminate for safety
                    if(udp_data_len < sizeof(udp_rx_buffer))
                        udp_rx_buffer[udp_data_len] = '\0';

                    last_receive_time = HAL_GetTick();

                    // ===== TAMBAHAN: Save to message history =====
                    add_message_to_history((char*)udp_rx_buffer, remote_ip, remote_port, 0);
                    // ===== END TAMBAHAN =====

                    // Optional: Debug via UART
                    /*
                    char debug[100];
                    sprintf(debug, "UDP RX: %s from %d.%d.%d.%d:%d\r\n",
                            udp_rx_buffer, remote_ip[0], remote_ip[1],
                            remote_ip[2], remote_ip[3], remote_port);
                    HAL_UART_Transmit(&huart1, (uint8_t*)debug, strlen(debug), 100);
                    */
                }
            }
            break;

        case SOCK_CLOSED:
            // Reopen socket
            socket(UDP_SOCKET, Sn_MR_UDP, UDP_PORT, 0);
            break;

        default:
            // Socket in unknown state, try to reinit
            close(UDP_SOCKET);
            socket(UDP_SOCKET, Sn_MR_UDP, UDP_PORT, 0);
            break;
    }
}

// HTTP Server Run
void httpServerRun(void)
{
    for (int sn = 0; sn < HTTP_MAX_SOCK; sn++)
    {
        httpServer_run(sn);
    }
}

// CGI Handler for GET requests
uint8_t predefined_get_cgi_processor(uint8_t * uri_name, uint8_t * buf, uint16_t * len)
{
    if (strstr((char*)uri_name, "udp_data.cgi"))
    {
        uint32_t current_time = HAL_GetTick();
        uint32_t age_seconds = (current_time - last_receive_time) / 1000;

        if(udp_data_len > 0 && last_receive_time > 0)
        {
            // Has data - return JSON with data
            int written = sprintf((char*)buf,
                "{\"has_data\":true,"
                "\"message\":\"%s\","
                "\"source_ip\":\"%d.%d.%d.%d\","
                "\"source_port\":%d,"
                "\"age\":%lu}",
                udp_rx_buffer,
                remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3],
                remote_port,
                age_seconds);

            *len = written;
        }
        else
        {
            // No data yet
            sprintf((char*)buf, "{\"has_data\":false}");
            *len = strlen((char*)buf);
        }

        return 1;
    }

    // ===== TAMBAHAN: Messages history endpoint =====
    if (strstr((char*)uri_name, "udp_messages.cgi"))
    {
        int written = sprintf((char*)buf, "{\"count\":%d,\"messages\":[", message_count);

        if(message_count > 0)
        {
            uint8_t start_idx = (message_count < MAX_MESSAGES) ? 0 : message_index;

            for(uint8_t i = 0; i < message_count; i++)
            {
                uint8_t idx = (start_idx + i) % MAX_MESSAGES;
                message_t* m = &message_history[idx];

                uint32_t age = (HAL_GetTick() - m->timestamp) / 1000;

                // Escape quotes in message
                char escaped[MAX_MESSAGE_LEN * 2];
                int j = 0;
                for(int k = 0; m->message[k] != '\0' && j < sizeof(escaped) - 2; k++)
                {
                    if(m->message[k] == '"' || m->message[k] == '\\')
                        escaped[j++] = '\\';
                    escaped[j++] = m->message[k];
                }
                escaped[j] = '\0';

                written += sprintf((char*)buf + written,
                    "%s{\"text\":\"%s\",\"source\":\"%d.%d.%d.%d:%d\",\"time\":\"%lus ago\",\"is_sent\":%d}",
                    (i > 0) ? "," : "",
                    escaped,
                    m->source_ip[0], m->source_ip[1], m->source_ip[2], m->source_ip[3],
                    m->source_port,
                    age,
                    m->is_sent);
            }
        }

        written += sprintf((char*)buf + written, "]}");
        *len = written;

        return 1;
    }
    // ===== END TAMBAHAN =====

    // Debug endpoint
    if (strstr((char*)uri_name, "udp_debug.cgi"))
    {
        uint8_t sock_status = getSn_SR(UDP_SOCKET);
        const char* sock_status_str;

        switch(sock_status)
        {
            case SOCK_UDP: sock_status_str = "UDP_OPEN"; break;
            case SOCK_CLOSED: sock_status_str = "CLOSED"; break;
            case SOCK_INIT: sock_status_str = "INIT"; break;
            default: sock_status_str = "UNKNOWN"; break;
        }

        uint32_t send_age = (HAL_GetTick() - last_send_time) / 1000;

        sprintf((char*)buf,
                "{\"target_ip\":\"%d.%d.%d.%d\","
                "\"target_port\":%d,"
                "\"last_send_status\":\"%s\","
                "\"last_send_time\":\"%lu sec ago\","
                "\"socket_status\":\"%s\"}",
                remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3],
                remote_port,
                last_send_status,
                last_send_time > 0 ? send_age : 0,
                sock_status_str);

        *len = strlen((char*)buf);
        return 1;
    }

    return 0;
}

// CGI Handler for POST requests
uint8_t predefined_set_cgi_processor(uint8_t * uri_name, uint8_t * uri, uint8_t * buf, uint16_t * len)
{
    if(strcmp((char*)uri_name, "udp_send.cgi") == 0)
    {
        // Extract POST data: "cmd=your_command_here"
        char* cmd_start = strstr((char*)uri, "cmd=");

        if(cmd_start && remote_ip[0] != 0) // Check if we have a valid remote IP
        {
            cmd_start += 4; // skip "cmd="

            // URL decode (simple version - handles %20 for spaces)
            char decoded[256];
            int j = 0;
            for(int i = 0; cmd_start[i] != '\0' && cmd_start[i] != '&' && j < 255; i++)
            {
                if(cmd_start[i] == '%' && cmd_start[i+1] && cmd_start[i+2])
                {
                    // Simple hex decode
                    char hex[3] = {cmd_start[i+1], cmd_start[i+2], 0};
                    decoded[j++] = (char)strtol(hex, NULL, 16);
                    i += 2;
                }
                else if(cmd_start[i] == '+')
                {
                    decoded[j++] = ' ';
                }
                else
                {
                    decoded[j++] = cmd_start[i];
                }
            }
            decoded[j] = '\0';

            // Send immediately
            uint8_t sock_status = getSn_SR(UDP_SOCKET);

            if(sock_status == SOCK_UDP)
            {
                // Socket is ready, send now!
                last_send_result = sendto(UDP_SOCKET,
                                         (uint8_t*)decoded,
                                         strlen(decoded),
                                         remote_ip,
                                         remote_port);

                last_send_time = HAL_GetTick();

                if(last_send_result > 0)
                {
                    // Success
                    sprintf((char*)last_send_status,
                           "OK: Sent %d bytes",
                           last_send_result);

                    // ===== TAMBAHAN: Save sent message to history =====
                    add_message_to_history(decoded, remote_ip, remote_port, 1);
                    // ===== END TAMBAHAN =====

                    *len = sprintf((char*)buf,
                                  "SUCCESS: Sent '%s' (%d bytes) to %d.%d.%d.%d:%d",
                                  decoded,
                                  last_send_result,
                                  remote_ip[0], remote_ip[1], remote_ip[2], remote_ip[3],
                                  remote_port);
                }
                else
                {
                    // Failed
                    sprintf((char*)last_send_status,
                           "FAILED: sendto returned %d",
                           last_send_result);

                    *len = sprintf((char*)buf,
                                  "FAILED: sendto() returned %d. Check network connection.",
                                  last_send_result);
                }
            }
            else
            {
                // Socket not ready
                sprintf((char*)last_send_status,
                       "FAILED: Socket status=%d",
                       sock_status);

                *len = sprintf((char*)buf,
                              "ERROR: UDP socket not ready (status=%d). Reinitializing...",
                              sock_status);

                // Try to reopen socket
                close(UDP_SOCKET);
                socket(UDP_SOCKET, Sn_MR_UDP, UDP_PORT, 0);
            }
        }
        else if(remote_ip[0] == 0)
        {
            *len = sprintf((char*)buf,
                   "ERROR: No device connected yet. Wait for device to send data to port 5000 first!");
        }
        else
        {
            *len = sprintf((char*)buf, "ERROR: Invalid command format");
        }

        return 1;
    }

    return 0;
}
