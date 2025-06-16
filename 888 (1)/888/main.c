#include "move.h"
#include <pthread.h>
#include<stdlib.h>
#include <stdio.h>

unsigned int shp,shp_next; //shape
static int dir; //控制移动方向
static int paused=0;
static int running=0;

struct block *bk,*bk_next; //类型方块结构体——》当前移动及下一个提示
struct block * move_check(struct ls_all *head,int dir);

void *auto_down(void *arg);
void  *time_out(void *arg);

int welcome(void){
	int x,y;

	//调用mplayer，后台播放视频，开始播放
	//-zoom -x 400 -y 320：画面大小400*320
	//-quiet：减少终端信息打印
	//-slave：指定mplayer运行在slave模式下。
	//-input file=/root/fifo：指定mplayer在/root/myfifo管道文件中读取命令
	system("mplayer -quiet -slave -input file=./myfifo  -zoom -x 800 -y 480 ./welcome.mp4 &");
	
	
	//打开管道文件
	int fd = open("./myfifo", O_RDWR);
	if(fd < 0)
	{
		perror("打开管道失败");
		return -1;
	}
	
	while(1)
	{	

		get_xy(&x, &y);
		//暂停
		if(x>=200 && x<=250 )
		{
			//发送命令实现
			write(fd, "pause\n", 6);
		}
		//快进
		else if(x>=250 && x<=300 )
		{
			write(fd, "seek -2\n", 8);
		}
		//快退
		else if(x>=350 && x<=400)
		{
			write(fd, "seek +2\n", 8);
			break;
		}
		//停止播放，退出
		else if(x>=600 && x<=800 && y < 100)
		{	

			system("killall -KILL mplayer");
			break;
		}
	}
	//关闭文件
	close(fd);
}

int end(void){
	system("mplayer -quiet -slave -input file=./myfifo  -zoom -x 800 -y 480 end.mp4 &");
	
	
	//打开管道文件
	int fd = open("myfifo", O_RDWR);
	if(fd < 0)
	{
		perror("打开管道失败");
		return -1;
	}
	
	//键盘控制视频播放器
	while(1)
	{	
		int x,y;

		get_xy(&x, &y);
		//暂停
		if(x>=200 && x<=250 )
		{
			//发送命令实现
			write(fd, "pause\n", 6);
		}
		//快进
		else if(x>=250 && x<=300 )
		{
			write(fd, "seek -2\n", 8);
		}
		//快退
		else if(x>=350 && x<=400)
		{
			write(fd, "seek +2\n", 8);
			break;
		}
		//停止播放，退出
		else if(x>=600 && x<=800 && y < 100)
		{	

			system("killall -KILL mplayer");
			break;
		}
	}
	
	//关闭文件
	close(fd);
	
}

int main(int argc,char *argv[]){
    int x,y;
    int grade = 0;//0 简单 1 普通 2 困难
    //先打开触摸屏
    ts_open();
    lcd_open();
    //welcome();
    bmp_show_mix(0,0,800,480,"./tetris_pic/start.bmp");
    while(1){
        get_xy(&x, &y);
        if(x >= 80 && x<= 280 && y >= 300 && y<= 440){
            bmp_show_mix(0,0,800,480,"./tetris_pic/start-1.bmp");
            printf("jiandan moshi\n");
            grade = 0;
            while(1){
                get_xy(&x, &y);
                if(x >= 340 * 1.25 && x<= 460* 1.25 && y >= 80* 1.25 && y<= 200* 1.25)
                {
                    break;
                }
            }
            break;
        }
        if(x >= 300* 1.25 && x<= 500* 1.25 && y >= 300* 1.25 && y<= 440* 1.25){
            bmp_show_mix(0,0,800,480,"./tetris_pic/start-2.bmp");
            grade = 1;
            printf("putong moshi\n");
            
            while(1){
                get_xy(&x, &y);
                if(x > 340* 1.25 && x<= 460* 1.25&& y >= 80* 1.25 && y<= 200* 1.25)
                {
                    break;
                }
            }
            break;
        }
        if(x > 520* 1.25 && x<= 720* 1.25 && y >= 300* 1.25 && y<= 440* 1.25){
            bmp_show_mix(0,0,800,480,"./tetris_pic/start-3.bmp");
            printf("kunnan moshi\n");
            grade = 2;
            while(1){
                get_xy(&x, &y);
                if(x > 340* 1.25 && x<= 460* 1.25&& y >= 80* 1.25 && y<= 200* 1.25)
                {
                    break;
                }
            }
            break;
        }
    }

    // 根据难度等级初始化速度
    if(grade == 0){
        speed = 0;
    }else if(grade == 1){
        speed = 1;
    }
    else if(grade == 2){
        speed = 2;
    }

    struct ls_all *head;
    //显示背景图片
    bmp_show_mix(0,0,800,480,"./tetris_pic/bck4.bmp");

    int rt;
    pthread_t idt, idr;
    //获取两种随机形状并初始化，得到初始化结构体
    srand((unsigned int) time(NULL));
    shp = ((unsigned int) rand()) % 7 + 1;
    shp_next = ((unsigned int) rand()) % 7 + 1;
    bk = bk_init(shp);
    bk_next = bk_init(shp_next);
    //初始化掉落方块结构体
    head = ls_init();
    //初始化分数
    score = 0;
    
    //显示移动方块 及 提示方块
    the_show(bk);
    the_show_next(bk_next);
    score_show(0); //显示成绩
    //创建控制方块移动线程
    pthread_create(&idt, NULL, auto_down, NULL);
    //时间更新线程，时间到且无操作自动更新dir为下落状态
    pthread_create(&idr, NULL, time_out, NULL);
    system("madplay ./tetris_pic/bgm.mp3 -r &");//音频
    while(1) {
    label :
    if(running == 1){
        // 等待线程结束
        pthread_cancel(idt);
        pthread_join(idt, NULL);
        pthread_cancel(idr);
        pthread_join(idr, NULL);

        bmp_show_mix(0, 0, 800, 480, "./tetris_pic/bck4.bmp");

        // 获取两个随机形状并初始化方块
        srand((unsigned int)time(NULL));
        shp = ((unsigned int)rand()) % 7 + 1;
        shp_next = ((unsigned int)rand()) % 7 + 1;

        bk = bk_init(shp);
        bk_next = bk_init(shp_next);

        // 初始化掉落方块的结构
        head = ls_init();

        // 初始化分数
        score = 0;
        // 根据难度等级重新初始化速度
        if(grade == 0){
            speed = 0;
        }else if(grade == 1){
            speed = 1;
        }
        else if(grade == 2){
            speed = 2;
        }

        // 显示当前移动的方块和下一个方块
        the_show(bk);
        the_show_next(bk_next);
        score_show(0); // 显示分数

        // 创建自动下落方块和超时的线程
        pthread_create(&idt, NULL, auto_down, NULL);
        pthread_create(&idr, NULL, time_out, NULL);
        running = !running;
    }
    if (!paused) {
        dir = -2; //the random  number except -1 & 0 & 1 & 2;
        while(dir == -2){
            usleep(1000);
        }
        if(dir== -1){ //变形
            change_type(bk);
            the_show_bck_type(bk);
        }
        else { //移动
            change_dir(bk->p,dir);
            the_show_bck_dir(bk->p,dir);
        }

        //移动检查是否越界及掉落到底部
        bk = move_check(head,dir);
        if(bk == NULL){
            running =1;
            goto label;
        }

        //显示方块形状
        the_show(bk);
    } else {
        //printf("游戏已暂停，请按空格键恢复...\n");
        while (1){
            if(paused==1);
            break;
        };
        //printf("游戏恢复...\n");
    }
}
}

//移动检查是否越界及掉落到底部
struct block * move_check(struct ls_all *head,int dir){
	
	int rt;
	
	//1.检查是否掉落底部
	if(ls_check(head,bk->p) == -1){
		
		//1.1 变形越界则恢复原来（再变4次）
		if(dir == -1){
			
			change_type(bk);
			change_type(bk);
			change_type(bk);
			
			//the_show(bk);
		}
		else{
			//1.2 移动越界 则恢复原来
			change_dir_off(bk->p,dir);
			
			if(dir == 0){ //1.2.1 向下移动越界则已经到底把它加入到链表
				the_show(bk);
				if(ls_updata(head,bk) == -1){ //加入链表，包括重新显示整个链表
					return NULL; //game over
				}
				shp_next = ((unsigned int)rand())%7+1;
				bk = bk_next; //有新的方块产生
				the_show_bck_next(bk_next->p);
				bk_next = bk_init(shp_next);//有新的下一个方块产生
				the_show_next(bk_next);

			}
			//1.2.2 左右移动越界 正常显示原来的
			else if(dir == 1||dir ==2){
				//the_show(bk);
			}

		}
		
		return bk;
	}
	
	
	
	//2. 越界检查
	rt = bound_check(bk->p);

	//2.1 掉落底部
	if(rt==0){
		change_dir_off(bk->p,0);
		the_show(bk);
		ls_updata(head,bk);
		shp_next = ((unsigned int)rand())%7+1;
		bk = bk_next;
		the_show_bck_next(bk_next->p);
		bk_next = bk_init(shp_next);
		the_show_next(bk_next);

	}
	
	//2.2 左右移动越界
	else if(rt == -1){
		while(bound_check(bk->p)== -1){
			change_dir_off(bk->p,1);
		}
	}
	else if(rt == -2){
		while(bound_check(bk->p)== -2){
			change_dir_off(bk->p,2);
		}
	}	
	
	return bk;
	
}
void  *auto_down(void *arg){
	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	int x,y;

	
	while(1){
		
		get_xy(&x,&y);
		
		if(x>422&&x<558&&y>383&&y<480){
			
			dir =1;

		}
		else if(x>536&&x<636&&y>448&&y<595){
			dir = 0;

		}

		else if(x>652&&x<798&&y>383&&y<480){
			dir = 2;

		}

		else if(x>536&&x<636&&y>218&&y<365){
			dir = -1;
		}

		else if(x>0 && x<110 * 1.25 && y>0 &&y<60 * 1.25){
            paused = !paused;
		}
        else if(x>166 * 1.25&&x<260 *1.25&&y>0&&y<60 * 1.25){
            running = 1;
        }
	}
}

void *time_out(void *arg){
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    
    // 定义三个等级的基础延迟时间（毫秒），增加差异
    const int base_delays[3] = {1000, 500, 200}; // 简单,普通,困难
    
    while(1){
        // 确保speed在有效范围内
        if(speed < 0 || speed > 2) speed = 0;
        
        // 根据等级获取基础延迟
        int delay = base_delays[speed];
        
        // 每得5分加速10%（最多加速50%）
        int acceleration = score / 5; 
        if(acceleration > 5) acceleration = 5; // 最大加速50%
        
        delay = delay * (10 - acceleration) / 10;
        
        // 确保最小延迟（防止过快）
        if(delay < 50) delay = 50;
        
        usleep(delay * 1000); // 转换为微秒
        dir = 0;
    }
}
void *play_music(void *arg){//音乐
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    system("madplay ./tetris_pic/bgm.mp3 -r &");
}









