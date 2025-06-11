#include "move.h"
#include <pthread.h>

unsigned int shp,shp_next; //shape
static int dir; //控制移动方向
static int paused=0;///暂停
static int running=0;

struct block *bk,*bk_next; //类型方块结构体——》当前移动及下一个提示
struct block * move_check(struct ls_all *head,int dir);

void *auto_down(void *arg);
void  *time_out(void *arg);


int main(int argc,char *argv[]){

    int x,y;
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
        //初始化分数和速度
        score = 0;
        speed = 0;
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
            pthread_cancel(idt);
            pthread_cancel(idr);
            bmp_show_mix(0, 0, 800, 480, "./tetris_pic/bck4.bmp");

            // 获取两个随机形状并初始化方块
            srand((unsigned int)time(NULL));
            shp = ((unsigned int)rand()) % 7 + 1;
            shp_next = ((unsigned int)rand()) % 7 + 1;

            bk = bk_init(shp);
            bk_next = bk_init(shp_next);

            // 初始化掉落方块的结构
            head = ls_init();

            // 初始化分数和速度
            score = 0;
            speed = 0;

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

		else if(x>0&&x<80&&y>0&&y<80){
            paused = !paused;
		}
        else if(x>250&&x<340&&y>0&&y<80){
            running = 1;
        }
	}
}
void  *time_out(void *arg){
	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

	
	while(1){
		
		usleep((400-speed*5)*1000);
		
		dir =0;
		
	}
	
	
}
void *play_music(void *arg){//音乐
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    system("madplay ./tetris_pic/bgm.mp3 -r &");
}









