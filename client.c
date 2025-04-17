#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 10
#define TIMEOUT 1
#define MAX_RETRIES 3

// Packet structure
typedef struct {
    int seq_ack;
    int len;
    int checksum;
} Header;

typedef struct {
    Header header;
    char data[BUFFER_SIZE];
    
}Packet;

// Function to calculate checksum
int calculate_checksum(Packet packet) {
    packet.header.checksum = 0;
    int checksum = 0;
    char *ptr = (char *)&packet;
    char *end = ptr + sizeof(Header) + packet.header.len;
    while (ptr < end)
    checksum ^= *ptr++;
    return checksum;
}

// Client implementation
int main() {
    int sockfd, seq = 0, retries;
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    Packet packet, ack;
    char *file_name = "src1.dat";

    // Open source file
    int src_fd = open(file_name, O_RDONLY);
    if (src_fd < 0) {
        perror("Source file failed to open.\n");
        return 0;
    }
    
    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        return 0;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Successfully created socket and opened source file.\n");

    while (1) {
        retries = 0;
        packet.header.seq_ack = seq;
        packet.header.len = read(src_fd, packet.data, BUFFER_SIZE);

        packet.header.checksum = calculate_checksum(packet);
        printf("Calculated checksum : %d\n", packet.header.checksum);
        
/*
        // **Introduce checksum error randomly for testing**
        if (rand() % 10 < 2) { // 20% probability of error
            packet.header.checksum ^= (rand() % 255);  // Modify checksum randomly
            printf("Sending packet with FAULTY checksum!\n");
        }
*/
        // Send packet
        if(sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, addr_len) < 0){
            perror("Failed to send packet to server.\n");
        }

        do {
            // Wait for ACK with timeout
            struct timeval tv = {TIMEOUT, 0};
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

            if (rv > 0) {
                recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&server_addr, &addr_len);
                if (ack.header.seq_ack == seq && ack.header.checksum == calculate_checksum(ack)) {
                    seq = 1 - seq; // Toggle sequence number
                    break;
                }
            }

            printf("Timeout or invalid ACK received. Retrying...\n");

        } while (++retries < MAX_RETRIES);

        if (packet.header.len < BUFFER_SIZE) break;
    }

    // Send final empty packet to indicate completion
    memset(&packet, 0, sizeof(packet));
    packet.header.checksum = calculate_checksum(packet);

    int final_attempts = 0;
    while (final_attempts < 3) {
        sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr, addr_len);
        struct timeval tv = {TIMEOUT, 0};
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        int rv = select(sockfd + 1, &readfds, NULL, NULL, &tv);

       if (rv > 0) {
            recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&server_addr, &addr_len);
            if (ack.header.seq_ack == seq && ack.header.checksum == calculate_checksum(ack)) {
                break;
            }
        }

        final_attempts++;
    }

    close(src_fd);
    close(sockfd);
}