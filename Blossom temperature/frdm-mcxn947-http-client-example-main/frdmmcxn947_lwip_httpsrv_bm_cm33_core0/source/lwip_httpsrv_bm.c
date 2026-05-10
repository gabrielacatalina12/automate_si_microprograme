
/*
 * ============================================================================
 *  🌸 Fairy Bloom IoT Temperature Client 🌸
 *  FRDM-MCXN947 + lwIP + ASP.NET Server
 * ============================================================================
 */

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/timeouts.h"
#include "lwip/init.h"
#include "lwip/dhcp.h"

#include "netif/ethernet.h"
#include "ethernetif.h"

#include "board.h"
#include "app.h"

#include "fsl_phy.h"
#include "fsl_silicon_id.h"

#include "temperature.h"
#include "http_client.h"

#include <stdio.h>
#include <string.h>

/* ============================================================================
 *  Network Configuration
 * ========================================================================== */

#ifndef EXAMPLE_NETIF_INIT_FN
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif

#define SERVER_IP_1     192
#define SERVER_IP_2     168
#define SERVER_IP_3     1
#define SERVER_IP_4     206

#define SERVER_PORT     5290
#define SERVER_ENDPOINT "/api/Data"

/* ============================================================================
 *  Variables
 * ========================================================================== */

static phy_handle_t phyHandle;

extern volatile uint32_t g_tempInt;
extern volatile uint32_t g_tempFrac;

/* ============================================================================
 *  Function Prototypes
 * ========================================================================== */

static void http_client_result(
    void *arg,
    int status_code,
    const char *body,
    u16_t body_len);

static void print_ipv6_addresses(struct netif *netif);

static void netif_ipv6_callback(struct netif *cb_netif);

static void send_temperature(void);

/* ============================================================================
 *  SysTick
 * ========================================================================== */

void SysTick_Handler(void)
{
    time_isr();
}

/* ============================================================================
 *  HTTP Response Callback
 * ========================================================================== */

static void http_client_result(
    void *arg,
    int status_code,
    const char *body,
    u16_t body_len)
{
    (void)arg;

    PRINTF("\r\n");
    PRINTF("========================================\r\n");
    PRINTF("   HTTP POST RESPONSE \r\n");
    PRINTF("========================================\r\n");

    PRINTF(" Status Code : %d\r\n", status_code);

    if (body != NULL && body_len > 0)
    {
        PRINTF(" Server Reply: %.*s\r\n",
               (int)body_len,
               body);
    }

    PRINTF("========================================\r\n");
}

/* ============================================================================
 *  Send Temperature to ASP.NET Server
 * ========================================================================== */

static void send_temperature(void)
{
    static char json[128];

    snprintf(
        json,
        sizeof(json),
        "{\"deviceId\":\"FRDM_MCXN947\",\"temperature\":%u.%u}",
        g_tempInt,
        g_tempFrac);

    PRINTF("\r\n");
    PRINTF(" Sending JSON: %s\r\n", json);

    ip_addr_t server_ip;

    IP_ADDR4(
        &server_ip,
        SERVER_IP_1,
        SERVER_IP_2,
        SERVER_IP_3,
        SERVER_IP_4);

    http_client_post(
        &server_ip,
        SERVER_PORT,
        SERVER_ENDPOINT,
        "application/json",
        json,
        (u16_t)strlen(json),
        http_client_result,
        NULL);
}

/* ============================================================================
 *  IPv6 Utilities
 * ========================================================================== */

static void print_ipv6_addresses(struct netif *netif)
{
    for (int i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
    {
        const char *str_ip = "-";

        if (ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
        {
            str_ip = ip6addr_ntoa(netif_ip6_addr(netif, i));
        }

        PRINTF(" IPv6 Address %d : %s\r\n", i, str_ip);
    }
}

static void netif_ipv6_callback(struct netif *cb_netif)
{
    PRINTF("\r\n");
    PRINTF(" IPv6 Address Updated\r\n");

    print_ipv6_addresses(cb_netif);

    PRINTF("\r\n");
}

/* ============================================================================
 *  Main Application
 * ========================================================================== */

int main(void)
{
    struct netif netif;

    ip4_addr_t netif_ipaddr;
    ip4_addr_t netif_netmask;
    ip4_addr_t netif_gw;

    ethernetif_config_t enet_config =
    {
        .phyHandle   = &phyHandle,
        .phyAddr     = EXAMPLE_PHY_ADDRESS,
        .phyOps      = EXAMPLE_PHY_OPS,
        .phyResource = EXAMPLE_PHY_RESOURCE,
    };

    /* ----------------------------------------------------------------------
     *  Hardware Init
     * -------------------------------------------------------------------- */

    BOARD_InitHardware();

    time_init();

    temperature_init();

    /* ----------------------------------------------------------------------
     *  Ethernet Init
     * -------------------------------------------------------------------- */

    (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);

    enet_config.srcClockHz = EXAMPLE_CLOCK_FREQ;

    IP4_ADDR(&netif_ipaddr,  0, 0, 0, 0);
    IP4_ADDR(&netif_netmask, 0, 0, 0, 0);
    IP4_ADDR(&netif_gw,      0, 0, 0, 0);

    lwip_init();

    netif_add(
        &netif,
        &netif_ipaddr,
        &netif_netmask,
        &netif_gw,
        &enet_config,
        EXAMPLE_NETIF_INIT_FN,
        ethernet_input);

    netif_set_default(&netif);

    netif_set_up(&netif);

    netif_create_ip6_linklocal_address(&netif, 1);

    /* ----------------------------------------------------------------------
     *  Wait For Ethernet Link
     * -------------------------------------------------------------------- */

    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        PRINTF(
            " Ethernet cable not connected...\r\n");
    }

    /* ----------------------------------------------------------------------
     *  DHCP
     * -------------------------------------------------------------------- */

    dhcp_start(&netif);

    PRINTF("\r\n");
    PRINTF(" Waiting for DHCP address...\r\n");

    while (dhcp_supplied_address(&netif) == 0)
    {
        ethernetif_input(&netif);

        sys_check_timeouts();
    }

    set_ipv6_valid_state_cb(netif_ipv6_callback);

    /* ----------------------------------------------------------------------
     *  Network Info
     * -------------------------------------------------------------------- */

    PRINTF(" IPv4 Address : %s\r\n",
           ip4addr_ntoa(netif_ip4_addr(&netif)));

    PRINTF(" Subnet Mask  : %s\r\n",
           ip4addr_ntoa(netif_ip4_netmask(&netif)));

    PRINTF(" Gateway      : %s\r\n",
           ip4addr_ntoa(netif_ip4_gw(&netif)));

    PRINTF("====================================================\r\n");

    /* ----------------------------------------------------------------------
     *  Main Loop
     * -------------------------------------------------------------------- */

    static uint32_t last_send_ms = 0;

    while (1)
    {
        ethernetif_input(&netif);

        sys_check_timeouts();

        uint32_t now = sys_now();

        if ((now - last_send_ms) >= 5000U)
        {
            last_send_ms = now;

            temperature_read();

            PRINTF(
                " Current Temperature: %u.%u C\r\n",
                g_tempInt,
                g_tempFrac);

            send_temperature();
        }
    }
}

