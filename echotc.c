#include <stdio.h>
#include <string.h>
#include <pico_tcp.h>
#include <time.h>
#include <pico_stack.h>
#include <pico_ipv4.h>
#include <pico_icmp4.h>
#include <pico_dev_tap.h>


#define SERVER_IP "192.168.11.137"
#define SERVER_PORT 1337  // Echo service port

void handle_data(struct pico_socket *s, uint8_t *data, uint16_t len) {
    // Handle received data (echo response)
    printf("Received: %.*s\n", len, data);

    // Close the socket after receiving the response
    pico_socket_close(s);
}

void handle_event(struct pico_socket *s, uint16_t event) {
    // Handle socket events
    if (event & PICO_SOCK_EV_RD) {
        // Data ready to be read
        uint8_t buffer[1024];
        int bytes_received = pico_socket_recv(s, buffer, sizeof(buffer));
        if (bytes_received > 0) {
            handle_data(s, buffer, (uint16_t)bytes_received);
        }
    } else if (event & PICO_SOCK_EV_ERR) {
        // Error event
        printf("Socket error occurred\n");
        pico_socket_close(s);
    }
}

char msgbuf[100000];

unsigned int get_size() {
    unsigned int size = 0;
    switch(rand() % 4) {
        case 0:
            size = rand() % 100;
            break;
        case 1:
            size = rand() % 1000;
            break;
        case 2:
            size = rand() % 200 + 1400;
            break;
        case 3:
            if (rand() % 2 == 0)
                size = rand() % 32767 + 5;
            else
                size = rand() % 100000;
    }

    return size;
}

int main(int argc, char **argv) {

    struct pico_ip4 ipaddr, netmask;
    struct pico_device* dev;

    char *iface = "tap0";

    int b = 0;

    if (argc >= 2) 
        iface = argv[1];

    // Initialize PicoTCP
    pico_stack_init();
    dev = pico_tap_create(iface);
    if (!dev)
        return -1;
    printf("dev: %p\n", dev);

    /* assign the IP address to the tap interface */
    pico_string_to_ipv4("192.168.11.133", &ipaddr.addr);
    pico_string_to_ipv4("255.255.255.0", &netmask.addr);
    pico_ipv4_link_add(dev, ipaddr, netmask);


// ----------------------------

    int conn_count = 1000;

    for (int cc = 0; cc < conn_count; cc++) {

        printf("creating new connection!!!\n");

        // Create a TCP socket
        struct pico_socket *tcp_socket = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_TCP, &handle_event);
        printf("here 5\n");
        if (!tcp_socket) {
            fprintf(stderr, "Error creating socket\n");
            return 1;
        }

        // Connect to the server
        struct pico_ip4 server_addr;
        pico_string_to_ipv4(SERVER_IP, &server_addr);
        if (pico_socket_connect(tcp_socket, &server_addr, htons(SERVER_PORT)) != 0) {
            fprintf(stderr, "Error connecting to the server\n");
            pico_socket_close(tcp_socket);
            return 1;
        }


        int jj = 0;
        int zerocnt = 0;

        memset(msgbuf, 'a', sizeof(msgbuf) - 1);

        for (jj = 0; jj < 10500 ; jj++) {
            // Send data (echo request)
            const char *message = "Hello, PicoTCP!";
            unsigned int msgsize = get_size();

            char m[100];
            char *ptr;
            sprintf(m, "%s(%d)", message, jj);

            if (rand() % 2) { 
                msgsize = strlen(m);
                ptr = m;
            } else {
                ptr = msgbuf;
            }

            if (pico_socket_send(tcp_socket, (uint8_t *)ptr, msgsize) != msgsize) {
                fprintf(stderr, "Error sending data\n");
 //               pico_socket_close(tcp_socket);
                b = 1;
                break;
    //            return 1;
            }
            pico_stack_tick();
            pico_stack_tick();
            pico_stack_tick();
            pico_stack_tick();
            usleep(100000);
            pico_stack_tick();
            pico_stack_tick();
            pico_stack_tick();
            pico_stack_tick();

            char buf[1024] = {};
            int rr =  pico_socket_recv(tcp_socket, buf, sizeof(buf) - 1);
            if (rr == -1) {
                fprintf(stderr, "error receiving data\n");
//                pico_socket_close(tcp_socket);
                b = 1;
                break;
    //            return 2;
            }

            pico_stack_tick();
            printf("received data(%d): %s\n", rr, buf);

            if(rr == 0) {
                zerocnt++;
                if (zerocnt >= 35) {
                    printf("ZERO TRIGGERED!!!\n");
//                    pico_socket_close(tcp_socket);
                    b = 1;
                    break;
                }
            } else {
                zerocnt = 0;
            }

        }
//        pico_socket_close(tcp_socket);
    }

    // Run the PicoTCP event loop
    while (1) {
        pico_stack_tick();
    }

    // Cleanup (this part is unreachable in this example)
//    pico_socket_close(tcp_socket);
    pico_stack_deinit();

    return 0;
}
