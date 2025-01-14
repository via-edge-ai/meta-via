/*
gcc fw-update.c -o fw-update
./fw-update /dev/ttyUSB0 TRK-03-M_STM32_FW_vx.x.x.bin
*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h> 
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>

#define CRC_POLY_16 0xA001
#define CRC_START_16 0x0000

/* flush working on PC, not working on MTK */
//#define DO_FLUSH 

unsigned short g_crc16_table[256];

void crc16_init(void){
    unsigned short i, j, crc, c;
    for(i=0; i<256; i++){
        crc = 0;
        c = i;
        for(j=0; j<8; j++){
            if(((crc ^ c) & 0x0001) != 0){
                crc = (crc >> 1) ^ CRC_POLY_16;
            }else{
                crc = crc >> 1;
            }
            c = c >> 1;
        }
        g_crc16_table[i] = crc;
    }
}

unsigned short crc16_calculate(unsigned char *buf, unsigned int len){
    unsigned short crc, tmp, short_c;
    unsigned int i;
    
    if(buf == NULL){
        return 0;
    }
    crc = CRC_START_16;
    for(i=0; i<len; i++){
        short_c = 0x00ff & (unsigned short)buf[i];
        tmp = crc ^ short_c;
        crc = (crc >> 8) ^ g_crc16_table[tmp & 0xff];
    }
    return crc;
}

unsigned short crc16_calculate_cont(unsigned short prev_crc, unsigned char *buf, unsigned int len){
    unsigned short crc, tmp, short_c;
    unsigned int i;
    
    if(buf == NULL){
        return 0;
    }
    crc = prev_crc;
    for(i=0; i<len; i++){
        short_c = 0x00ff & (unsigned short)buf[i];
        tmp = crc ^ short_c;
        crc = (crc >> 8) ^ g_crc16_table[tmp & 0xff];
    }
    return crc;
}

unsigned char int_to_hex(unsigned char d){
    if(d > 0xF){
        return '0';
    }
    if(d >= 0xA){
        return 'A' + d - 10;
    }
    return '0' + d;
}

unsigned char hex_to_int(unsigned char c){
    if(c >= '0' && c <= '9'){
        return c - '0';
    }
    if(c >= 'A' && c <= 'F'){
        return c - 'A' + 10;
    }
    return 0;
}

void put_hex16_to_buf(unsigned short d, unsigned char *buf){
    buf[0] = int_to_hex((d >> 12) & 0xF);
    buf[1] = int_to_hex((d >> 8) & 0xF);
    buf[2] = int_to_hex((d >> 4) & 0xF);
    buf[3] = int_to_hex(d & 0xF);
}

int uart_read(int fd, unsigned char *values, int len){
    int i, n;
    struct timeval twait;
    fd_set fds;
    
    i = 0;
    while(i < len){
        unsigned char c;
        
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        twait.tv_sec = 10;
        twait.tv_usec = 0;
        n = select(fd+1, &fds, NULL, NULL, &twait);
        if(n < 0){
            return -1;
        }else if(n == 0){
            return i;
        }
        if(!FD_ISSET(fd, &fds)){
            values[i] = 0;
            break;
        }
        n = read(fd, &c, 1);
        if(n != 1){
            return -2;
        }
        if(c >= ' '){
            values[i++] = c;
            
        }else if(c == '\n'){
            // handle new line char at start of message
            if(i == 0){
                continue;
            }
            values[i] = 0;
            break;
        }
    }
    return i;
}

int send_cmd(int fd, unsigned char *cmd, int cmdlen, unsigned char *retbuf, int retlen){
    int i, n;
    
    n = write(fd, cmd, cmdlen);
    if(n != cmdlen){
        printf("write %d but %d writen\n", cmdlen, n);
        return -1;
    }
    
    n = uart_read(fd, retbuf, retlen);
    if(n <= 0){
        printf("read %d byte\n", n);
        retbuf[0] = 0;
        return n;
    }
    // 0 terminated string
    retbuf[n] = 0;
    return n;
}

int uart_read_fast(int fd, unsigned char *values, int len){
    int i, n;
    struct timeval twait;
    fd_set fds;
    
    i = 0;
    while(i < len){
        unsigned char c;
        
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        twait.tv_sec = 0;
        twait.tv_usec = 500000;
        n = select(fd+1, &fds, NULL, NULL, &twait);
        if(n < 0){
            return -1;
        }else if(n == 0){
            return i;
        }
        if(!FD_ISSET(fd, &fds)){
            values[i] = 0;
            break;
        }
        n = read(fd, &c, 1);
        if(n != 1){
            return -2;
        }
        values[i++] = c;
    }
    values[i] = 0;
    return i;
}

int send_cmd_fast(int fd, unsigned char *cmd, int cmdlen, unsigned char *retbuf, int retlen){
    int i, n;
    
    n = write(fd, cmd, cmdlen);
    if(n != cmdlen){
        return -1;
    }
    
    n = uart_read_fast(fd, retbuf, retlen);
    if(n <= 0){
        retbuf[0] = 0;
        return n;
    }
    // 0 terminated string
    retbuf[n] = 0;
    return n;
}

int uart_init(char *path)
{
    int n, fd;
    struct termios uartopt;

    printf("open uart...\n");
    fd = open(path, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        printf("cannot open %s\n", path);
        return 2;
    }
    /* Cancel the O_NDELAY flag. */
    n = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, n & ~O_NDELAY);
    
    printf("setup uart...\n");
    //tcgetattr(fd, &uartopt);
    //bzero(&uartopt, sizeof(uartopt));
    memset(&uartopt, 0, sizeof(uartopt));
    uartopt.c_cflag = CBAUD | CS8 | CLOCAL | CREAD;
    /* Set into raw, no echo mode */
    uartopt.c_iflag = IGNBRK | IGNPAR | ICRNL;
    uartopt.c_oflag = 0;
    uartopt.c_lflag = 0;
    uartopt.c_cc[VMIN] = 1;
    uartopt.c_cc[VTIME] = 1;
    cfsetispeed(&uartopt, B115200);  
    cfsetospeed(&uartopt, B115200);  

    if(tcsetattr(fd, TCSANOW, &uartopt) != 0){  
        printf("cannot set para for %s\n", path);
        close(fd);
        return -1;
    }
    return fd;
}

int do_rtc(int fd, int argc, char *argv[]){
    unsigned char buf[30];
    unsigned char ret_buf[30];
    char *rtc_r;
    int count = 0, week;

    if(!strcmp(argv[1], "r") ||
       !strcmp(argv[1], "w")) {
        // rtc read
        if (!strcmp(argv[1], "r")) {
            printf("rtc read ...\n");
            if(argc != 2){
                printf("Arg wrong: rtc <r/w> <time>\n");
                return 1;
            }
            buf[0] = 'g';
            buf[1] = '_';
            buf[2] = 'd';
            buf[3] = 't';
            buf[4] = 0xD;
            buf[5] = 0xA;
            if(send_cmd(fd, buf, 6, ret_buf, 30) <= 0){
                return 3;
            }
            if(strncmp((char *)ret_buf, "$g_dt", 5) != 0){
                printf("Failed, rtc returns %s\n", ret_buf);
                return 4;
            }else{
                //printf("rtc read buf %s\n", ret_buf);
                printf("rtc: 20");
                rtc_r = strtok((char *)(ret_buf + 6), ",");
                while( rtc_r!=NULL ) {
                     if (count != 3) {
                        if (strlen(rtc_r) == 1)
                          printf("0%s", rtc_r);
                        else
                          printf("%s", rtc_r);
                     } else {
                        week = atoi(rtc_r);
                     }
                     rtc_r = strtok(NULL, ",");
                     count ++;
                }
                switch(week) {
                    case 1:
                      printf(" Mon");
                    break;
                    case 2:
                      printf(" Tue");
                    break;
                    case 3:
                      printf(" Wed");
                    break;
                    case 4:
                      printf(" Thurs");
                    break;
                    case 5:
                      printf(" Fri");
                    break;
                    case 6:
                      printf(" Sat");
                    break;
                    case 7:
                      printf(" Sun");
                    break;
                    default:
                      printf(" error!!");
                }
                printf("\n");
            }
        } else {  // rtc write
            printf("rtc write ...\n");
            if(argc != 3 || strlen(argv[2]) > 20 || strlen(argv[2]) < 14){
                printf("Arg wrong: time format <YY,MM,DD,DoW,HH,mm,ss>\n");
                return 1;
            }
            //printf("data:%s\n", argv[2]);
            sprintf((char *)buf, "s_dt %s\n%c", argv[2], 0xd);
            //printf("buf:%s\n", buf);
            if(send_cmd(fd, buf, 30, ret_buf, 30) <= 0){
                return 3;
            }
            //printf("ret_buf:%s\n", ret_buf);
            if(strncmp((char *)ret_buf, "$s_dt", 5) != 0){
                printf("Failed, rtc returns %s\n", ret_buf);
                return 4;
            }
        }
    }else{
        printf("Arg wrong: rtc <r/w> <time>\n");
        return 2;
    }
    return 0;
}

int do_update(int fd, int argc, char *argv[])
{
    int ifd, i, ret, func_ofs, n;
    unsigned char buf[100];
    unsigned short block, crc;
    unsigned int fsize;

    printf("open %s ...\n", argv[1]);
    /* rw, create if not exist, clean and new from length 0, mode = rw */
    ifd = open(argv[1], 0);
    if(ifd <= 0){
        printf("cannot open rom\n");
        close(ifd);
        close(fd);
        return 4;
    }

    /* kick */
    i = 5;
    for(i=0; i<10; i++){
        buf[0] = 'A';
        buf[1] = 'T';
        buf[2] = '$';
        buf[3] = 'U';
        buf[4] = 'P';
        buf[5] = 'F';
        buf[6] = 'W';
        buf[7] = '=';
        buf[8] = '1';
        buf[9] = 0xD;
        buf[10] = 0xA;
        ret = send_cmd_fast(fd, buf, 11, buf, 100);
        if(ret == 0){
            continue;
        }
        if(ret < 0){
            close(ifd);
            close(fd);
            return 5;
        }
        if((buf[0] == '$') && (buf[1] == 'R') && (buf[2] == 'E') && 
           (buf[3] == 'A') && (buf[4] == 'D') && (buf[5] == 'Y')){
            printf("Get Ready ! ($Ready)\n");
            usleep(500000);
            break;
        
        }else if((buf[0] == '$') && (buf[1] == 'O') && (buf[2] == 'K')){
            printf("Get Ready ! ($OK)\n");
            usleep(500000);
            break;
        
        }else{
            printf("Receive: %s\n", buf);
        }
    }

    n = 0;
    while(n < 3){
        if(uart_read_fast(fd, buf, 100) <= 0){
            n++;
        }
    }

    crc16_init();
    block = 0;
    fsize = 0;
    /* put data */
    while(1){
        unsigned char fbuf[256];
        unsigned char cmdbuf[512];
        unsigned char *ptr;
        int nread, cmdlen;
        
        nread = read(ifd, fbuf, 128);
        if(nread <= 0){
            break;
        }

        crc = crc16_calculate(fbuf, nread);

        cmdbuf[0] = 'A';
        cmdbuf[1] = 'T';
        cmdbuf[2] = '$';
        cmdbuf[3] = '0';
        cmdbuf[4] = '1';
        put_hex16_to_buf(crc, &cmdbuf[5]);
        put_hex16_to_buf(nread, &cmdbuf[9]);
        put_hex16_to_buf(block, &cmdbuf[13]);
        ptr = &cmdbuf[17];
        cmdlen = 17;
        
        for(i=0; i<nread; i++){
            ptr[0] = int_to_hex(fbuf[i] >> 4);
            ptr[1] = int_to_hex(fbuf[i] & 0xF);
            ptr += 2;
            cmdlen += 2;
        }

        ptr[0] = 0xD;
        ptr[1] = 0xA;
        cmdlen += 2;
        
        ret = send_cmd(fd, cmdbuf, cmdlen, fbuf, 256);
        if(ret <= 0){
            printf("read %d byte\n", ret);
            close(ifd);
            close(fd);
            return 6;
        }
        if(!((fbuf[0] == '$') && (fbuf[1] == 'O') && (fbuf[2] == 'K'))){
            printf("write block %d failed : %s\n\n", block, fbuf);
            close(ifd);
            close(fd);
            return 7;
        }
        printf("write block %d ok : %s\n", block, fbuf);
        block++;
        fsize += nread;
    }
    printf("sent file size = %u\n", fsize);

    /* calculate all file crc */
    crc = CRC_START_16;
    fsize = 0;
    // reset position pointer
    if(lseek(ifd, 0, SEEK_SET) != 0){
        printf("cannot reset ptr!\n");
        close(ifd);
        close(fd);
        return 8;
    }
    while(1){
        unsigned char fbuf[128];
        unsigned char *ptr;
        int nread, cmdlen;
        
        nread = read(ifd, fbuf, 128);
        if(nread <= 0){
            break;
        }
        
        fsize += nread;
        crc = crc16_calculate_cont(crc, fbuf, nread);
    }
    printf("file size = %u, crc = %04x\n", fsize, crc);

    buf[0] = 'A';
    buf[1] = 'T';
    buf[2] = '$';
    buf[3] = '0';
    buf[4] = '2';
    put_hex16_to_buf(crc, &buf[5]);
    put_hex16_to_buf((fsize >> 16) & 0xFFFF, &buf[9]);
    put_hex16_to_buf((fsize & 0xFFFF), &buf[13]);
    buf[17] = 0xD;
    buf[18] = 0xA;
    
    ret = send_cmd(fd, buf, 18, buf, 100);
    if(ret <= 0){
        printf("read %d byte\n", ret);
        close(ifd);
        close(fd);
        return 9;
    }
    if(!((buf[0] == '$') && (buf[1] == 'O') && (buf[2] == 'K'))){
        printf("copy & verify failed : %s\n\n", buf);
        close(ifd);
        close(fd);
        return 10;
    }
    printf("copy & verify ok : %s\n", buf);

    buf[0] = 'A';
    buf[1] = 'T';
    buf[2] = '$';
    buf[3] = 'U';
    buf[4] = 'P';
    buf[5] = 'F';
    buf[6] = 'W';
    buf[7] = '=';
    buf[8] = '0';
    buf[9] = 0xD;
    buf[10] = 0xA;
    ret = send_cmd(fd, buf, 11, buf, 100);
    if(ret <= 0){
        printf("read %d byte\n", ret);
        close(ifd);
        close(fd);
        return 11;
    }
    if(!((buf[0] == '$') && (buf[1] == 'O') && (buf[2] == 'K'))){
        printf("exit failed : %s\n\n", buf);
        close(ifd);
        close(fd);
        return 12;
    }
    printf("exit ok : %s\n", buf);
    return 0;
}


int do_info(int fd){
    unsigned char buf[20];
    int ret, i;
    /* print version */
    buf[0] = 'v';
    buf[1] = 'e';
    buf[2] = 'r';
    buf[3] = 's';
    buf[4] = 'i';
    buf[5] = 'o';
    buf[6] = 'n';
    buf[7] = 0xD;
    buf[8] = 0xA;
    /* kick */
    i = 5;
    for(i=0; i<10; i++) {
        ret = send_cmd_fast(fd, buf, 9, buf, 20);
        if(ret == 0){
            continue;
        }
        if(ret < 0){
            close(fd);
            return 5;
        }
        //printf("get %s\n", buf);
        if((buf[0] == '$') && (buf[1] == 'v') && (buf[2] == 'e') && 
           (buf[3] == 'r') && (buf[4] == 's') && (buf[5] == 'i') && 
           (buf[6] == 'o') && (buf[7] == 'n')){
            printf("%s", buf+1);
            usleep(500000);
            break;
        }
    }
    return 0;
}

int do_gpo(int fd, int argc, char *argv[]){
    char buf[20];
    char cmp_buf[20];

    if(argc != 3){
        printf("Arg wrong: gpo pin-num on-off\n");
        return 1;
    }
    if(!strcmp(argv[1], "1") ||
       !strcmp(argv[1], "2") ||
       !strcmp(argv[1], "3")) {
        sprintf(buf, "gpo %c,%c\n%c", argv[1][0], argv[2][0] == '0' ? '0' : '1', 0xd);
        if(send_cmd(fd, (unsigned char *)buf, 9, (unsigned char *)cmp_buf, 20) <= 0){
            return 3;
        }
        if(strncmp(buf, &cmp_buf[1], 7) != 0){
            printf("Failed, gpo returns %s\n", cmp_buf);
            return 4;
        }else{
            printf("gpo: ok\nGPO %c Trigger %c\n", argv[1][0], argv[2][0] == '0' ? '0' : '1');
        }
    }else{
        printf("Arg wrong: no such pin (%s) should be 1~3\n", argv[1]);
        return 2;
    }
    return 0;
}

int do_gpi(int fd, int argc, char *argv[]){
    int value;
    char buf[20];

    if(argc != 2){
        printf("Arg wrong: gpi pin-num\n");
        return 1;
    }
    if(!strcmp(argv[1], "1") ||
       !strcmp(argv[1], "2") ||
       !strcmp(argv[1], "3")) {
        sprintf(buf, "gpi %c\n%c", argv[1][0], 0xd);
        if(send_cmd(fd, (unsigned char *)buf, 7, (unsigned char *)buf, 20) <= 0){
            return 3;
        }
        if(strncmp(buf, "$gpi", 4) != 0){
            printf("Failed, gpi returns %s\n", buf);
            return 4;
        }else{
            sscanf(&buf[7], "%x", &value);
            printf("gpi %c returns %d\n", argv[1][0], value);
            return value;
        }
    }else{
        printf("Arg wrong: no such pin (%s) should be 1~3\n", argv[1]);
        return 2;
    }
    return 0;
}

int do_adc(int fd, int argc, char *argv[]){
    char buf[20];
    int adc;
    if(argc != 2){
        printf("Arg wrong: adc <pin-num>\n");
        return 1;
    }
    if(!strcmp(argv[1], "1") ||
       !strcmp(argv[1], "2")) {
       sprintf(buf, "adc %c\n%c", argv[1][0], 0xd);

       if(send_cmd(fd, (unsigned char *)buf, 7, (unsigned char *)buf, 20) <= 0){
           return 3;
       }
       if(!strncmp(buf, "$adc fail", 9)){
           printf("adc returns %s\n", buf);
           return 2;
       }
       sscanf(&buf[7], "%x", &adc);
       printf("Pin %c, adc: %d\n", buf[5], adc);
    } else {
        printf("Arg wrong: no such pin (%s) should be 1~2\n", argv[1]);
        return 2;
    }
    return 0;
}

int do_wdt(int fd){
    unsigned char buf[20];
    int ret, i;
    /* print wdt */
    buf[0] = 'g';
    buf[1] = '_';
    buf[2] = 'w';
    buf[3] = 'd';
    buf[4] = 't';
    buf[5] = 0xD;
    buf[6] = 0xA;
    /* kick */
    i = 5;
    for(i=0; i<10; i++) {
        ret = send_cmd_fast(fd, buf, 7, buf, 20);
        if(ret == 0){
            continue;
        }
        if(ret < 0){
            close(fd);
            return 5;
        }
        //printf("get %s\n", buf);
        if((buf[0] == '$') && (buf[1] == 'g') && (buf[2] == '_') &&
           (buf[3] == 'w') && (buf[4] == 'd') && (buf[5] == 't')){
            if (buf[7] == '1')
                printf("WDT enable !!\n");
            else
                printf("WDT disable !!\n");
            usleep(500000);
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[]){
    int fd, ret, func_ofs;
    char *func;

    if(argc < 3){
        printf("Usage: %s </dev/ttySX> <func> [arg1] [arg2] ...\n", argv[0]);
        printf("version: 1.0.0\n");
        printf("func -\n");
        printf("      info                 <Show firmware version>\n");
        printf("      wdt                  <Show WDT status>\n");
        printf("      update path          <Update MCU firmware>\n");
        printf("      rtc r,w time         <Read/Write MCU RTC>\n");
        printf("      gpo number on/off    <On/Off MCU GPO>\n");
        printf("      gpi number           <Set MCU GPI>\n");
        printf("      adc number           <Read MCU adc>\n");
        return 1;
    }
    fd = uart_init(argv[1]);
    if(fd < 0){
        return 2;
    }

    ret = 1;
    func_ofs = 2;
    func = argv[func_ofs];
    if(strcmp(func, "rtc") == 0){
        ret = do_rtc(fd, (argc - func_ofs), &argv[func_ofs]);
    } else if(strcmp(func, "info") == 0){
        ret = do_info(fd);
    } else if(strcmp(func, "update") == 0){
        ret = do_update(fd, (argc - func_ofs), &argv[func_ofs]);
    } else if(strcmp(func, "gpi") == 0){
        ret = do_gpi(fd, (argc - func_ofs), &argv[func_ofs]);
    }else if(strcmp(func, "gpo") == 0){
        ret = do_gpo(fd, (argc - func_ofs), &argv[func_ofs]);
    }else if(strcmp(func, "adc") == 0){
        ret = do_adc(fd, (argc - func_ofs), &argv[func_ofs]);
    }else if(strcmp(func, "wdt") == 0){
       ret = do_wdt(fd);
    }

    close(fd);
    return ret;
}