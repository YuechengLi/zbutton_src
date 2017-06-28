#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h> 

//#define CHIP_ADDR 0x20
#define I2C_DEV "/dev/i2c-0"//i2c_dev为i2c　adapter创建的别名
//读操作先发Slaveaddr_W+Regaddr_H+Regaddr_L 3个字节来告诉设备操作器件及两个byte参数
//然后发送Slaveaddr_R读数据
static int iic_read(int fd, char buff[], int addr, int count)
{
    int res;
    char sendbuffer1[2];
    sendbuffer1[0]=addr;
    write(fd,sendbuffer1,1);      
        res=read(fd,buff,count);
        printf("read %d byte at 0x%x\n", res, addr);
        return res;
}
//在写之前，在数据前加两个byte的参数，根据需要解析
static int iic_write(int fd, char buff[], int addr, int count)
{
        int res;
        int i,n;
        static char sendbuffer[100];
        memcpy(sendbuffer+2, buff, count);
        sendbuffer[0]=addr>>8;
    sendbuffer[1]=addr;
        res=write(fd,sendbuffer,count+2);
        printf("write %d byte at 0x%x\n", res, addr);
}
int main(void){
    int fd;
    int res;
    char ch;
    char buf[50];
    int regaddr,i,slaveaddr;
    fd = open(I2C_DEV, O_RDWR);// I2C_DEV /dev/i2c-0
        if(fd < 0){
                printf("####i2c test device open failed####/n");
                return (-1);
        }
    res = ioctl(fd,I2C_TENBIT,0);   //not 10bit
    res = ioctl(fd,I2C_SLAVE,0x68);    //设置I2C从设备地址[6:0]
        
	
    res=iic_read(fd,buf,0,7);
    printf("Time read from AS1801 is (yyyy-mm-dd-day-hh-mm-ss): \n",res);
    printf("20%x-%x-%x-%x-%x-%x-%x \n ",buf[6],buf[5],buf[4],buf[3],buf[2],buf[1],buf[0]);
    printf("\n");

    return 0;
}
