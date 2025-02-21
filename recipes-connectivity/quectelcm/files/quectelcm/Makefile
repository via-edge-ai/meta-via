
TARGET := quectelcm

APL_DIR := .

QL_CM_SRC=$(APL_DIR)/QmiWwanCM.c $(APL_DIR)/GobiNetCM.c $(APL_DIR)/main.c $(APL_DIR)/MPQMUX.c $(APL_DIR)/QMIThread.c $(APL_DIR)/util.c $(APL_DIR)/qmap_bridge_mode.c $(APL_DIR)/mbim-cm.c $(APL_DIR)/device.c
ifeq (1,1)
QL_CM_DHCP=$(APL_DIR)/udhcpc.c
else
LIBMNL=$(APL_DIR)/libmnl/ifutils.c $(APL_DIR)/libmnl/attr.c $(APL_DIR)/libmnl/callback.c $(APL_DIR)/libmnl/nlmsg.c $(APL_DIR)/libmnl/socket.c
DHCP=$(APL_DIR)/libmnl/dhcp/dhcpclient.c $(APL_DIR)/libmnl/dhcp/dhcpmsg.c $(APL_DIR)/libmnl/dhcp/packet.c
QL_CM_DHCP=$(APL_DIR)/udhcpc_netlink.c
QL_CM_DHCP+=${LIBMNL}
endif

CFLAGS += -Wall -I$(APL_DIR)
#CFLAGS += -D_DEBUG

.PHONY: all clean

all:    $(TARGET)
#$(TARGET): qmi-proxy
#	$(CC) -Wall -s ${QL_CM_SRC} ${QL_CM_DHCP} -o quectelcm -lpthread -ldl
$(TARGET): qmi-proxy
	$(CC) -Wall -g -DCM_DEBUG -DNO_CM_DHCP ${QL_CM_SRC} ${QL_CM_DHCP} -o quectelcm -lpthread -ldl

debug: clean
	$(CC) -Wall -g -DCM_DEBUG -DNO_CM_DHCP ${QL_CM_SRC} ${QL_CM_DHCP} -o quectelcm -lpthread -ldl

qmi-proxy:
	$(CC) -Wall -s quectel-qmi-proxy.c  -o quectel-qmi-proxy -lpthread -ldl

clean:
	rm -rf $(TARGET) $(APL_DIR)/*~
	rm -rf quectel-qmi-proxy
