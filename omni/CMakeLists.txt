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

# 线程模式（NO_SYS==0）需要 RTOS 移植层。sys_arch.h 按 CONFIG_* 选择 RTOS，
# 这里按同一个符号把 port/<rtos>/sys_arch.c 编进来，并链接 RTOS 模块，
# 让 cmsis_os2.h / FreeRTOS.h 和 RTOS 实现一起传播给本模块。
# NO_SYS==1 的 raw API 工程不开这些符号，不产生任何额外源文件。
if(CONFIG_CMSIS_RTOS_V2)
    target_sources(omni_module_lwip PRIVATE
        "${OMNI_MODULE_LWIP_SRC}/port/cmsis_rtos/sys_arch.c")
    # cmsis_rtx / cmsis_freertos 都在 lwip 之前注册，目标此时已存在。
    foreach(_lwip_rtos omni_module_cmsis_rtx omni_module_cmsis_freertos)
        if(TARGET ${_lwip_rtos})
            target_link_libraries(omni_module_lwip PUBLIC ${_lwip_rtos})
        endif()
    endforeach()
elseif(CONFIG_FREERTOS)
    target_sources(omni_module_lwip PRIVATE
        "${OMNI_MODULE_LWIP_SRC}/port/freertos/sys_arch.c")
    if(TARGET omni_module_freertos)
        target_link_libraries(omni_module_lwip PUBLIC omni_module_freertos)
    endif()
endif()

omni_apply_arch(omni_module_lwip)
omni_apply_warnings(omni_module_lwip)
