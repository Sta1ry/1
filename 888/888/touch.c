#include"touch.h"

//获取点击坐标点--》获取一个触摸点后不等手指离开屏幕立即返回
int get_xy(int *x,int *y){
	
	int fd_touch = open("/dev/input/event0",O_RDONLY);
	if(fd_touch == -1){
		printf("open touch dev fail\n");
		return -1;
	}
	
	struct input_event ts;
	
	int i=0;
	while(1){
		
		read(fd_touch,&ts,sizeof(ts));
		
		// printf("type:%d  code:%d   value:%d\n",ts.type,ts.code,ts.value);
		
		if(ts.type == 3){
			if(ts.code == 0){
				*x = ts.value;
				i++;
			}
			else if(ts.code == 1){
				*y = ts.value;
				i++;
			}
		}

		if(i>=2){
			break;
		}
		printf("(%d , %d)\n",*x,*y);
	}
	
	close(fd_touch);
	
}
