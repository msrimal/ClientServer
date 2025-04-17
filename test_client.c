#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 10

// Packet structure
typedef struct {
    int seq_ack;
    int len;
    int checksum;
    char data[BUFFER_SIZE];
} Packet;

// Function to calculate checksum
int calculate_checksum(Packet *packet) {
    int checksum = 0;
    char *ptr = (char *)packet;
    char *end = ptr + sizeof(Packet);
    while (ptr < end) {
        checksum ^= *ptr++;  // XOR every byte
    }
    return checksum;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    Packet packet;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        return 0;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP

    // Populate packet
    packet.seq_ack = 1;  // Example sequence number
    packet.len = 5;  // Example data length
    strncpy(packet.data, "Hello", packet.len);  // Example data
    packet.checksum = calculate_checksum(&packet);  // Calculate checksum

    printf("Sending packet with seq_ack: %d, checksum: %d\n", packet.seq_ack, packet.checksum);

    // Send packet to server
    if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, addr_len) < 0) {
        perror("Failed to send packet to server");
        return 0;
    }

    printf("Packet sent to server.\n");

    close(sockfd);
    return 0;
}
