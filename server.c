#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 10

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

// Server implementation
int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    Packet packet, ack;
    char *file_name = "dst.dat";

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        return 0;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 0;
    }
    printf("Socket successfully binded.\n");
    
    //open dst file
    int file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0664);
    
    if (file_fd < 0) {
        perror("Failed to open file");
        return 0;
    }

    printf("Opened dst file.\n");

    int expected_seq = 0;  // Initially expect sequence 0

    while (1) {
        // Receive packet from client
        printf("Waiting for client to send a packet...\n");
        
        Packet packet;

        int n = recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("Failed to receive packet");
            continue;
        }
        printf("Received packet with seq_ack: %d Checksum: %d\n", packet.header.seq_ack, packet.header.checksum);

        //calculate expected_checksum
        int expected_checksum = calculate_checksum(packet);
        //char *ptr = (char *)&packet;
       // char *end = ptr + sizeof(Packet) - sizeof(packet->checksum);
        //while(ptr < end)
       // expected_checksum ^= *ptr++;
        printf("Expected checksum: %d\n", expected_checksum);

        if(packet.header.checksum != expected_checksum || packet.header.seq_ack != expected_seq){
            perror("Checksum or seq number doesn't match.\n");
        }

        printf("Packets are a match!\n");



        //return 0;

        if (packet.header.seq_ack != expected_seq) {
            printf("Out of order packet. Expected %d, Received %d. Ignoring.\n", expected_seq, packet.header.seq_ack);
            // Send ACK for the last valid packet received (keep expected_seq unchanged)
            ack.header.seq_ack = expected_seq;  // Acknowledge the last expected seq number
            ack.header.checksum = calculate_checksum(ack);
            sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&client_addr, addr_len);
            continue;
        }

      // int calculated_checksum = calculate_checksum(&packet);
      //  if (calculated_checksum != packet.checksum) {
     //       printf("Invalid checksum. Ignoring packet.\n");
      //      continue;  // Ignore the packet if checksum is incorrect
       // }
        if(packet.header.len == 0){
            printf("Final packet received. Ending transmission.\n");
            break;
        }
        if (packet.header.len > 0) {
            // Write data to file
            if (write(file_fd, packet.data, packet.header.len) < 0) {
                perror("Failed to write data to file");
                continue;
            }
            printf("Data written to file.\n");
        }

        // Prepare ACK
        ack.header.seq_ack = packet.header.seq_ack;  // Send back the same sequence number
        ack.header.checksum = calculate_checksum(ack);

        // Send ACK back to the client
        sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&client_addr, addr_len);
        printf("ACK sent for seq_ack: %d\n", ack.header.seq_ack);
    }

    close(file_fd);
    close(sockfd);
}
