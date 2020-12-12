#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

int debug = 1;

#define panic(fmt, ...)                                     \
    do {                                                    \
        fprintf(stderr, "error: " fmt "\n", ##__VA_ARGS__); \
        exit(1);                                            \
    } while (0)

struct eth_hdr
{
    uint8_t dmac[6];
    uint8_t smac[6];
    uint16_t ethertype;
    uint8_t payload[];
} __attribute__((packed));


int run_cmd(const char *cmd, ...)
{
    constexpr size_t CMDBUFLEN = 256;
    va_list ap;
    char buf[CMDBUFLEN];
    va_start(ap, cmd);
    vsnprintf(buf, CMDBUFLEN, cmd, ap);
    va_end(ap);
    if (debug) {
        printf("EXEC: %s\n", buf);
    }
    return system(buf);
}

const char *tapaddr = "10.0.0.5";
const char *taproute = "10.0.0.0/24";

static int set_if_route(const char *dev, const char *cidr)
{
    return run_cmd("ip route add dev %s %s", dev, cidr);
}

static int set_if_address(const char *dev, const char *cidr)
{
    return run_cmd("ip address add dev %s local %s", dev, cidr);
}

static int set_if_up(const char *dev)
{
    return run_cmd("ip link set dev %s up", dev);
}

int tun_alloc_old(char *dev){
    char tunname[IFNAMSIZ];
    sprintf(tunname, "/dev/%s", dev);
    return open(tunname, O_RDWR);
}

int tun_alloc(char* dev)
{
    struct ifreq ifr;
    int fd;
    int err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        return tun_alloc_old(dev);
    }

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    if (dev && *dev) {
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    }

    if ((err = ioctl(fd, TUNSETIFF, (void*)&ifr)) < 0) {
        close(fd);
        return err;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;
}


int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    char dev[32] = { 0 };
    int tun_fd = tun_alloc(dev);
    if (tun_fd < 0) {
        panic("tun_alloc failed: (%d) %s", tun_fd, strerror(tun_fd));
    }

    printf("tun_alloc returned: %d\n", tun_fd);


    if (set_if_up(dev) != 0) {
        close(tun_fd);
        panic("when setting up if");
    }

    if (set_if_route(dev, taproute) != 0) {
        close(tun_fd);
        panic("when setting route for if");
    }

    if (set_if_address(dev, tapaddr) != 0) {
        close(tun_fd);
        panic("when setting addr for if");
    }

    close(tun_fd);
    return 0;
}
