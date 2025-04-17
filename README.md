> 🔌 **Lab5_146: Client–Server File Transfer**  
> A simple C-based client–server application for COEN 146L.  
> Uses sockets to send a data file (`src1.dat`) from `test_client` to `server`, demonstrating basic network I/O and file handling.  
> Fully tested with `test_client.c` and sample data.

---

# 📡 Lab5_146: Client–Server File Transfer

A C program demonstrating basic client–server communication by transferring a data file over sockets.

---

## 🚀 Features

- 📤 Client reads `src1.dat` and sends data to server  
- 📥 Server writes incoming data to `dst.dat`  
- 🧪 `test_client.c` simulates client behavior  
- ⚙️ Handles connection setup, data streaming, and clean shutdown  

---

## 🛠 Prerequisites

- GCC or any C compiler  
- Unix/Linux environment (POSIX sockets)  

---

## 🔧 Build

Compile both server and client:

```bash
gcc -o server server.c
gcc -o test_client test_client.c
