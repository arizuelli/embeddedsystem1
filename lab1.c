#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <signal.h> // for signal handling
#include <linux/i2c.h> // for struct i2c_msg

#define Device_Address 0x68 /*Device Address/Identifier for MPU6050*/

#define PWR_MGMT_1   0x6B
#define SMPLRT_DIV   0x19
#define CONFIG       0x1A
#define INT_ENABLE   0x38
#define ACCEL_XOUT_H 0x3B
#define ACCEL_YOUT_H 0x3D
#define ACCEL_ZOUT_H 0x3F

volatile int running = 1; // Flag to control the main loop

void wr_function(int fd, char *write_bytes, char *read_bytes, int w_len, int r_len){
    // Prepare message(s)
    // Structs needed for I2C messages
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2]; // Set size depending on number of messages in one transaction

    // Write message
    messages[0].addr = Device_Address;
    messages[0].flags = 0; // means write
    messages[0].len = w_len;
    messages[0].buf = write_bytes; // Pointer to the data bytes to be written

    // Read message
    messages[1].addr = Device_Address;
    messages[1].flags = I2C_M_RD;
    messages[1].len = r_len;
    messages[1].buf = read_bytes; // Pointer for reading the data

    // Build packet list
    packets.msgs = messages;
    packets.nmsgs = 2;

    ioctl(fd, I2C_RDWR, &packets);
}

void MPU6050_Init(int fd) { // prepare messages
    char write_buffer[2];
    char read_buffer[1];
    /*write_buffer[0] = SMPLRT_DIV;
    write_buffer[1] = 0x07;
    wr_function(fd, write_buffer, read_buffer, 2, 1);
*/
    write_buffer[0] = PWR_MGMT_1;
    write_buffer[1] = 0x00;
    wr_function(fd, write_buffer, read_buffer, 2, 1);
/*
    write_buffer[0] = CONFIG;
    write_buffer[1] = 0;
    wr_function(fd, write_buffer, read_buffer, 2, 1);

    write_buffer[0] = INT_ENABLE;
    write_buffer[1] = 0x01;
    wr_function(fd, write_buffer, read_buffer, 2, 1);
*/
}

short read_raw_data(int fd, int addr) {
    char read_buffer[2];
    char write_buffer[1];
    write_buffer[0] = addr;
    short raw;
    
    wr_function(fd, write_buffer, read_buffer, 1, 2); // Read data from register  
    raw=(read_buffer[0] << 8) | read_buffer[1];
    return raw;
}

void ms_delay(int val) {
    usleep(val * 1000); // Delay in milliseconds
}

void sigint_handler(int sig) {
    printf("Ctrl+C detected. Exiting...\n");
    running = 0; // Set running flag to false to exit the loop
}

int main() {
    printf("Welcome! This program reads data from the MPU6050 accelerometer sensor.\n");
    printf("Sensor initialization in progress...\n");

    // Open file and configure slave address
    char i2cFile[15];
    int dev = 1;
    int addr = 0x68; // MPU6050 I2C address
    sprintf(i2cFile, "/dev/i2c-%d", dev); // system file identifying the hw
    int fd = open(i2cFile, O_RDWR); // Obtain file descriptor for RW
    if (fd < 0) {
        perror("Failed to open the i2c bus");
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, addr) < 0) { // ioct to configure the comm. 
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }

    MPU6050_Init(fd);

    float Acc_x = 0, Acc_y = 0 , Acc_z = 0;
    float Ax = 0, Ay = 0, Az = 0;

    // Set up signal handler for Ctrl+C
    signal(SIGINT, sigint_handler);

    while (running) { // Continue while running flag is true
        // Read raw acceleration data
        Acc_x = read_raw_data(fd, ACCEL_XOUT_H);
        Acc_y = read_raw_data(fd, ACCEL_YOUT_H);
        Acc_z = read_raw_data(fd, ACCEL_ZOUT_H);

        // Divide raw value by sensitivity scale factor
        Ax = Acc_x / 16384.0;
        Ay = Acc_y / 16384.0;
        Az = Acc_z / 16384.0;

        printf("\n Ax=%.3f g\tAy=%.3f g\tAz=%.3f g\n", Ax, Ay, Az);
        ms_delay(3000); // Measure every 3 seconds
    }

    close(fd); // Close file descriptor
    return 0;
}
