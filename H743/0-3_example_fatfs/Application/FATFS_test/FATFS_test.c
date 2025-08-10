#include "FATFS_test.h"

#include <stdio.h>
#include "ff.h"
#include "fatfs.h"


void fatfs_test_exfuns(){
	printf(" ====================== exfuns_test ======================\r\n");
	mf_init();
	   //文件系统句柄名称, 驱动器号, 1:立即挂载 0:操作时挂载
	    FRESULT res = f_mount(&SDFatFS, "0:", 1);//挂载卷
	    if (res == FR_OK) {             //挂载成功
	        printf(" File system mount success !!!\r\n\r\n");
	    }
	    else if (res == FR_NO_FILESYSTEM) {  //没有文件系统
	        printf(" No file system\r\n");
	        printf(" Create file system...\r\n");
	        // 驱动器号, 系统类型, 簇大小(字节)
	        BYTE workbuffer[4 * BLOCKSIZE];	//格式化缓冲区 = 4*SD块大小
	        DWORD cluster_size = 0;		// 0:自动确认簇大小(256 = 128k)
	        // 驱动器号, 系统类型, 簇大小, 初始化缓冲空间, 缓冲空间大小
	        res = f_mkfs("0:", FM_EXFAT, cluster_size, workbuffer, (4*BLOCKSIZE));//创建文件系统
	        if (res == FR_OK) {
	            printf(" File system Create success !!!\r\n");
	        }
	        else {
	            printf(" File system Create fail code: %d\r\n", res);return;
	        }
	    }
	    else {
	        printf(" File system mount failed  code: %d\r\n", res);return;
	    }






	printf(" ====================== exfuns_end ======================\r\n");
}

void fatfs_test(void) {
    printf(" ====================== fatfs_test ======================\r\n");
    /* =================================== 驱动器挂载 ========================================= */
    //文件系统句柄名称, 驱动器号, 1:立即挂载 0:操作时挂载
    FRESULT res = f_mount(&SDFatFS, "0:", 1);//挂载卷
    if (res == FR_OK) {             //挂载成功
        printf(" File system mount success !!!\r\n\r\n");
    }
    else if (res == FR_NO_FILESYSTEM) {  //没有文件系统
        printf(" No file system\r\n");
        printf(" Create file system...\r\n");
        // 驱动器号, 系统类型, 簇大小(字节)
        BYTE workbuffer[4 * BLOCKSIZE];	//格式化缓冲区 = 4*SD块大小
        DWORD cluster_size = 0;		// 0:自动确认簇大小(256 = 128k)
        // 驱动器号, 系统类型, 簇大小, 初始化缓冲空间, 缓冲空间大小
        res = f_mkfs("0:", FM_EXFAT, cluster_size, workbuffer, (4*BLOCKSIZE));//创建文件系统
        if (res == FR_OK) {
            printf(" File system Create success !!!\r\n");
        }
        else {
            printf(" File system Create fail code: %d\r\n", res);return;
        }
    }
    else {
        printf(" File system mount failed  code: %d\r\n", res);return;
    }

    HAL_Delay(100);
    /* =================================== 获取系统信息 ========================================= */
    FATFS *fs;  //  返回的文件系统指针
    DWORD free_clust;//剩余簇的数量
    // 驱动器号, 返回值， 返回指向文件系统的指针
    res = f_getfree("0:", &free_clust, &fs);//获取空闲簇数量,最后返回一个指向此驱动器的文件系统指针
    if(res != FR_OK){
		printf(" f_getfree() error code: %d\r\n", res);
		return;
    }
    //获取成功
    printf(" *** FAT disk info ***\r\n");
    DWORD total_sector = (fs->n_fatent - 2)*fs->csize;	//计算总扇区数量,(簇的数量 = n_fatent-2) * 簇大小。 n_fatent:文件分配表中条目的总数量
    DWORD free_sector = free_clust * fs->csize;			//计算空闲扇区数量


#if _MAX_SS == _MIN_SS
    QWORD total_space = (total_sector >> 11);	//计算总空间(MB)
    QWORD free_space = (free_sector >> 11);		//计算剩余空间(MB)
#endif
#if _MAX_SS != _MIN_SS
    DWORD free_space = (DWORD)(((QWORD)free_sector * fs->ssize)/1024)/1024;
	DWORD total_space = (DWORD)(((QWORD)total_sector * fs->ssize)/1024)/1024;
#endif

    //打印系统信息
    printf(" FAT type = %d\r\n",fs->fs_type);
    printf("  FS_FAT12 = 1\r\n");
    printf("  FS_FAT16 = 2\r\n");
    printf("  FS_FAT32 = 3\r\n");
    printf("  FS_exFAT = 4\r\n\r\n");
#if _MAX_SS == _MIN_SS
    printf(" Sector size(bytes)= %d\r\n",_MIN_SS);		//一个扇区大小,
#endif
#if _MAX_SS != _MIN_SS
    printf(" Sector size(bytes)= %d\r\n",fs->ssize);		//一个扇区大小,
#endif
    printf(" Cluster size(sectors)= %u\r\n",fs->csize); //一个簇大小,

    printf(" Total Sector count = %lu\r\n",total_sector);	//扇区数量,
    printf(" Total Cluster count = %lu\r\n",fs->n_fatent-2);	//簇数量,


    printf(" Total space = %lu(MB) \r\n", total_space) ;	//总空间(MB),

    printf(" free Sector count = %lu\r\n",free_sector);	//剩余扇区数
    printf(" free Cluster count = %lu\r\n",free_clust); 	//剩余簇数

    printf(" free space = %lu(MB) \r\n", free_space);		//剩余空间(MB)



    /* =================================== 文件读写测试 ========================================= */
    printf(" ================== operational testing =================== \r\n");
    FIL file;			//文件对象
    char buffer[] = {"hello fatfs\n"};
    //文本文件写入
       	   	   //文件对象, 文件名称,  		操作模式(打开文件不存在则创建并把读写指针定位到尾端,可写)
    res = f_open(&file, "readme.txt",FA_OPEN_APPEND|FA_WRITE);//新建和打开文件用的是同一个函数,只是操作模式不同
    if(res == FR_OK){
    	f_puts(buffer, &file);
    	printf(" write to readme.txt OK!\r\n");
    }else{
    	printf(" write file ERR!!! code: %d\r\n", res);
    }
    f_close(&file);//关闭文件,写入存储介质

    //读文件,并打印
    uint8_t show_str[40];
    FIL file2;
    //打开文件使用相对路径, 此时需要看当前目录(CWD),
    res = f_open(&file2, "readme.txt", FA_READ);
    if(res == FR_OK){
    	//判断读写指针是否到达文件尾
    	for(uint8_t i = 1; (!f_eof(&file2)); i++){
    		//字符串缓冲区, 缓冲区大小, 文件
    		f_gets(show_str, 40, &file2);	//从文件获取字符串,遇到换行符停止
    		printf(" Line%d:%s \r\n", i, show_str);	//打印此行, 读写指针自动增加下次读取下一行
    	}
    }
    else if(res == FR_NO_FILE){
    	printf(" readme.txt does not exist!!!\r\n");
    }
    else{
    	printf(" read file ERR!!! code: %d\r\n", res);
    }
    f_close(&file2);

return;

    //二进制数据写入binary
    FIL file1;			//文件对象
    res = f_open(&file1, "value.txt", FA_CREATE_ALWAYS|FA_WRITE);//始终创建(覆盖),写
    uint8_t value = 0;
    UINT	bw;			//实际写入字节数
    if(res == FR_OK){
    	for(uint16_t i=0;  i < 100; i++, value++){
    			//写入文件, 写入值, 需要写入字节数, 实际写入字节数
    		f_write(&file1, &value, 100, &bw);
    	}
    	printf(" write to value.txt OK, write count: %d, real count: %d\r\n", 100, bw);

    }else{
    	printf(" write file1 ERR!!! code: %d\r\n", res);
    }
    f_close(&file1);//关闭文件,写入存储介质


    /* ========================================= 路径操作测试 ========================================= */
    DIR 	dir;		//返回的目录对象
    FILINFO dir_info;	//存储一个文件或目录的元数据信息

    res = f_opendir(&dir,"0:");
    if (res != FR_OK){
    	//f_closedir(&dir);//打开失败不能操作失败的路径
    	if(res == FR_NO_PATH){
    		printf(" PATH does not exist!!!\r\n");
    	}
    	printf(" open dir ERR!!! code: %d\r\n", res);
    }
    else {
        //创建目录
        f_mkdir("0:/file1");
        f_mkdir("0:/file2");

    	/* 打印此目录下的所有项目 */
    	printf("\r\n ==============================\r\nAll items :\r\n");
    	while(1){
    		res = f_readdir(&dir, &dir_info);			//按顺序读取目录项
    		if(res != FR_OK || dir_info.fname[0]== 0){	//读取失败或为空
    			break;
    		}
    		if(dir_info.fattrib & AM_DIR){	//判断"目录条目的文件属性"是否为路径
    			printf("DIR :&s\r\n", dir_info.fname);
    		}else{							//不是路径说明是文件
    			printf("FILE :%s\r\n", dir_info.fname);
    		}
    	}
    	printf(" ==============================\r\n");
    	f_closedir(&dir);//操作完要关闭路径对象
    }


    /* ========================================= 获取文件信息 ========================================= */
    FILINFO file_info;

    res = f_stat("readme.txt", &file_info);//获取文件信息,
    if (res == FR_OK){
    	printf(" File size(bytes) = %d\r\n", file_info.fsize);	//显示文件大小,如果此路径项是一个目录,则此成员为0

    	printf(" File attribute = 0x%x\r\n",file_info.fattrib);//文件属性,应该是0x20即归档文件

    	printf(" File name = %s\r\n", file_info.fname);	//文件名称

    	//修改时间
    	RTC_DateTypeDef sDate;
    	RTC_TimeTypeDef sTime;

    	sDate.Date = file_info.fdate & 0x001F;
    	sDate.Month = (file_info.fdate & 0x01E0) >> 5;
    	sDate.Year = 1980 + ((file_info.fdate & 0xFEE0)>>9) - 2000;//文件系统和RTC开始年份不同,需要换算,

    	sTime.Hours = (file_info.ftime & 0xF800) >> 11;
    	sTime.Minutes = (file_info.ftime & 0x07E0) >> 5;
    	sTime.Seconds = (file_info.ftime & 0x001F) << 1;

    	printf(" File Date = %04d-%02d-%02d\r\n", sDate.Year, sDate.Month, sDate.Date);
    	printf(" File Time = %02d:%02d:%02d\r\n", sTime.Hours, sTime.Minutes, sTime.Seconds);
    }
    else{
    	printf(" get file info ERR!!! code: %d\r\n", res);
    }


    printf(" ==================== fatfs_test cplt ====================\r\n");
}
