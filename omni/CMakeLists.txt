# lwIP 构建入口（模块自带 glue）。
#
# 源文件清单与 modules/lwip/lwip.clayer.yml 完全一致（37 个：core/ipv4/api/netif），
# include 路径为 src/include 与 port/（arch/cc.h）。

add_library(omni_module_lwip OBJECT
    # core
    "${OMNI_MODULE_LWIP_SRC}/src/core/def.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/dns.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/inet_chksum.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/init.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ip.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/mem.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/memp.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/netif.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/pbuf.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/raw.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/stats.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/sys.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/tcp.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/tcp_in.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/tcp_out.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/timeouts.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/udp.c"
    # core/ipv4
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/autoip.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/dhcp.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/etharp.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/icmp.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/igmp.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/ip4.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/ip4_addr.c"
    "${OMNI_MODULE_LWIP_SRC}/src/core/ipv4/ip4_frag.c"
    # api
    "${OMNI_MODULE_LWIP_SRC}/src/api/api_lib.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/api_msg.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/err.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/if_api.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/netbuf.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/netdb.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/netifapi.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/sockets.c"
    "${OMNI_MODULE_LWIP_SRC}/src/api/tcpip.c"
    # netif
    "${OMNI_MODULE_LWIP_SRC}/src/netif/bridgeif.c"
    "${OMNI_MODULE_LWIP_SRC}/src/netif/bridgeif_fdb.c"
    "${OMNI_MODULE_LWIP_SRC}/src/netif/ethernet.c")

target_include_directories(omni_module_lwip PUBLIC
    "${OMNI_MODULE_LWIP_SRC}/src/include"
    "${OMNI_MODULE_LWIP_SRC}/port")
target_link_libraries(omni_module_lwip PUBLIC
    omni::framework omni::config omni::hal)

omni_apply_arch(omni_module_lwip)
omni_apply_warnings(omni_module_lwip)
