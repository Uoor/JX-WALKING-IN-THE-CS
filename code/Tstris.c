/***************************** SOURCE FILE INFORMATION *****************************

	TARGET ENVIRONMENT	:: Linux
	CREATED				:: 2014.10.20
	LAST UPDATED		:: 2014.10.22
	MAIL				::905595245@qq.com
gcc Tstris.c -lpthread -o xxx
************************ END OF SOURCE FILE INFORMATION ***************************/
/**********************************************************************************
OPERATE:['a' ��] ['d' ��] ['s' ��] ['w' ��ת] [others ��]
************************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <termio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>

/*************************************************
��
**************************************************/
/*����Ĵ�С*/
#define WIDTH	10
#define HIGHT	20
/*����ı߽�*/
#define WIN_X1 1
#define WIN_X2 (WIN_X1+WIDTH+1)
#define WIN_Y1 1
#define WIN_Y2 (WIN_Y1+HIGHT+1)


/*����ĳ�ʼλ��*/
#define START_X	6
#define START_Y	3
#define SHAPE	'@'

/*������ٶ�*/
#define MAX_TIME	800000/*0.8��*/
#define MIN_TIME	0
#define MAX_LEVEL	10

/*������ƶ�����*/
#define DOWN	1
#define UP		0
#define LEFT	3
#define RIGHT	2

#define OVER	"Game Over!!!"
#define PIPE_NAME	"./pipe"
/*************************************************
�ṹ��
**************************************************/
typedef struct
{
    int ZBody[4][2];/*�ĸ�����:x y*/
    int flag;/*��״ 0:�� 1���� 2����Z 3:��Z  4����L 5:��L 6:�� */
    int direction;/*����  0:��1:��2:��3:��  �� Z��˵�ǳ�ʼ����*/
} ZzBody;
/*********************************************************
ȫ�ֱ���
***********************************************************/
int giScore = 0;
int giLevel = 0;	/*����*/
int giNewBlock = 0;/*�Ƿ�ʼһ���µķ���*/
int giMoveErr = 0;/*�Ƿ�ײǽ*/
int block_screen[WIN_Y2][WIN_X2] = {0};/*������*/
int giHight = 0;
int giPipeFd = -1;

/*************************************************
����
*************************************************
static void ZInitBlock(ZzBody *CurrBody,int flag)��
static void settty(int iFlag);
static void ZDrawOneNode(int y,int x,int iFlag);
static void ZInitScreen();
static void ZRestoreScreen();
static void ZDrawScope();
static void zDrawBlock(ZzBody block,int iflag);
static void zNextBlock(ZzBody *block,int Dir);
static void DrawScreen(int flag);
static void JudgeNext(ZzBody block);
static int zJudgeNew(ZzBody block,int dir);
static int JudgeScreen();
static int ZGetDir();

*/




/***************************************************
*****************************************************/
/*��ʼ������ �����Ƿ���ı���*/
static void ZInitBlock(ZzBody *CurrBody,int flag/*0����ʾ��һ�γ��ַ��� 1������ı���*/)
{
    int i,Zj,Lj;
    int plus,plus1;
    if(flag == 0)//0����һ�β������� 1������ĳ�����ֱ仯
    {
        srand(time(NULL));
        memset(CurrBody,0,sizeof(ZzBody));
        //��ʼ����һ������
        CurrBody->ZBody[0][0] = START_X;
        CurrBody->ZBody[0][1] = START_Y;
        //��ʼ����״
        CurrBody->flag = rand()%7;
    }


    /*��ʼ���������������*/

    switch(CurrBody->flag)
    {
    case 0:/*����*/
        if(!flag)
            CurrBody->direction = rand()%2;//��ʼ������
        for(i=1; i<4; i++)
        {
            CurrBody->ZBody[i][(~CurrBody->direction)&1] = CurrBody->ZBody[0][(~CurrBody->direction)&1];
            CurrBody->ZBody[i][CurrBody->direction] = CurrBody->ZBody[0][CurrBody->direction]+i;
        }
        /*����Ϊ0�Ǻ��ŵ� 1�����ŵ� */
        break;
    case 1:/*������*/
        CurrBody->direction = 0;
        for(i=1; i<4; i++)
        {
            CurrBody->ZBody[i][0] = CurrBody->ZBody[0][0]+(1&i);
            CurrBody->ZBody[i][1] = CurrBody->ZBody[0][1]+((i&2)>>1);
        }
        break;
    case 2://��Z
        if(!flag)
            CurrBody->direction = rand()%2;//��ʼ������0:��1������

        if(CurrBody->direction)
        {
            CurrBody->ZBody[0][0]++;
            plus = -1;
        }
        else
        {
            CurrBody->ZBody[0][0]--;
            plus = 1;
        }
        for(i=1; i<4; i++)
        {
            if(i&2)
                Lj = 1;
            else
                Lj = 0;
            if(((i&2)>>1) ^ (i&1))
                Zj = 1;
            else
                Zj = 2;

            CurrBody->ZBody[i][CurrBody->direction] = CurrBody->ZBody[0][CurrBody->direction]+Zj;
            CurrBody->ZBody[i][(~CurrBody->direction)&1] = CurrBody->ZBody[0][(~CurrBody->direction)&1]+Lj*plus;
        }
        break;
    case 3://�� Z
        if(!flag)
            CurrBody->direction = rand()%2;//��ʼ������0:��1������
        if(CurrBody->direction)
        {
            CurrBody->ZBody[0][0]--;
            plus = 1;
        }
        else
        {
            CurrBody->ZBody[0][0]++;
            plus = -1;
        }
        for(i=1; i<4; i++)
        {
            if(i&2)
                Lj = 1;
            else
                Lj = 0;
            if(((i&2)>>1) ^ (i&1))
                Zj = 1;
            else
                Zj = 2;
            CurrBody->ZBody[i][CurrBody->direction] = CurrBody->ZBody[0][CurrBody->direction]+Zj*plus;
            CurrBody->ZBody[i][(~CurrBody->direction)&1] = CurrBody->ZBody[0][(~CurrBody->direction)&1]+Lj;
        }
        break;

    case 4://��L
        if(!flag)
            CurrBody->direction = rand()%4;
        Zj = CurrBody->direction;
        if(((Zj&2)>>1)^(Zj&1)) /*0 3��ʾ����  1 2 ��ʾ�Ӻ�*/
        {
            plus = 1;
        }
        else
        {
            plus = -1;
        }
        for(i=1; i<4; i++)
        {
            if(i&2) /*1 ���� 2 3 �Ӻ� */
            {
                Lj = 1;
            }
            else
            {
                Lj = -1;
            }
            CurrBody->ZBody[i][(CurrBody->direction&1)] = CurrBody->ZBody[0][(CurrBody->direction&1)]+Lj;
            CurrBody->ZBody[i][(~(CurrBody->direction&1))&1] = CurrBody->ZBody[0][(~(CurrBody->direction&1))&1]+((~(i&1))&1)*plus;
        }
        if(Zj == 2 || Zj == 3)
        {
            CurrBody->ZBody[2][Zj&1] -= 2;
        }
        break;
    case 5://��L
        if(!flag)
            CurrBody->direction = rand()%4;
        Zj = CurrBody->direction;
        if(((Zj&2)>>1)^(Zj&1)) /*0 3��ʾ����  1 2 ��ʾ�Ӻ�*/
        {
            plus = 1;
        }
        else
        {
            plus = -1;
        }
        for(i=1; i<4; i++)
        {
            if(((i&2)>>1) ^ (i&1)) /*1 2 ���� 3 �Ӻ�*/
            {
                Lj = -1;
            }
            else
            {
                Lj = 1;
            }
            CurrBody->ZBody[i][(CurrBody->direction&1)] = CurrBody->ZBody[0][(CurrBody->direction&1)]+Lj;
            CurrBody->ZBody[i][(~(CurrBody->direction&1))&1] = CurrBody->ZBody[0][(~(CurrBody->direction&1))&1]+((~(i&1))&1)*plus;
        }
        if(Zj == 2 || Zj == 3)
        {
            CurrBody->ZBody[2][Zj&1] += 2;
        }
        break;
    case 6:/*��ʮ����*/
        if(!flag)
            CurrBody->direction = rand()%4;
        Zj = CurrBody->direction;
        if(((Zj&2)>>1)^(Zj&1)) /*0 3��ʾ����  1 2 ��ʾ�Ӻ�*/
        {
            plus = 1;
        }
        else
        {
            plus = -1;
        }
        for(i=1; i<4; i++)
        {
            CurrBody->ZBody[i][(CurrBody->direction&1)] = CurrBody->ZBody[0][(CurrBody->direction&1)]+(i-2);
            CurrBody->ZBody[i][(~(CurrBody->direction&1))&1] = CurrBody->ZBody[0][(~(CurrBody->direction&1))&1]+((~(i&1))&1)*plus;
        }
        break;
    default:
        break;

    }

}
static void settty(int iFlag)/*�����Ƿ�����ֱ�Ӵ�ӡ�������*/
{
    int fd;
    struct termio stTerm;

    if((fd = open(ttyname(1),O_RDWR))==-1)        return;
    if(iFlag == 1)
    {
        ioctl(fd,TCGETA,&stTerm);
        stTerm.c_lflag &= ~ICANON;
        stTerm.c_lflag &= ~ECHO;
        stTerm.c_cc[4] = 1;
        stTerm.c_cc[5] = 0;
        stTerm.c_iflag &= ~ISTRIP;
        stTerm.c_cflag |= CS8;
        stTerm.c_cflag &= ~PARENB;
        ioctl(fd,TCSETA,&stTerm);
    }
    else
    {
        ioctl(fd,TCGETA,&stTerm);
        stTerm.c_lflag |= ICANON;
        stTerm.c_lflag |= ECHO;
        stTerm.c_cc[4] = 4;
        stTerm.c_cc[5] = 5;
        stTerm.c_iflag &= ~ISTRIP;
        stTerm.c_cflag |= CS8;
        stTerm.c_cflag &= ~PARENB;
        ioctl(fd,TCSETA,&stTerm);
    }
    close(fd);
}
static void ZDrawOneNode(int y,int x,int iFlag)/*��ӡһ��Node*/
{
        if(iFlag == 1)
                printf("\033[%dm\033[47;%dm\033[%d;%dH%c",
                        iFlag,30/*black*/,y,x,SHAPE);
        else
                printf("\033[%dm\033[47;%dm\033[%d;%dH%c",
                        iFlag,37/*white*/,y,x,' ');

        fflush(stdout);
}


static void ZInitScreen()/*��ʼ����Ļ*/
{
    settty(1);
    printf("\033[?25l\033[2J");
}

static void ZRestoreScreen()/*��ԭ��Ļ*/
{
    printf("\033[24;1H\033[1m\033[40;34m\033[?25h");
    settty(0);
}

void ZDrawScope()/*���ϱ߿�*/
{
    int i,j;

    for(j=WIN_Y1; j<=WIN_Y2; j+=WIN_Y2-WIN_Y1)
    {
        printf("\033[%d;%dH+",j,WIN_X1);
        for(i=WIN_X1+1; i<WIN_X2; i++)
            printf("-");
        printf("+");
    }
    for(i=WIN_Y1+1; i<WIN_Y2; i++)
        printf("\033[%d;%dH|%*c|\n",i,WIN_X1,WIN_X2-WIN_X1-1,' ');
}

static int ZGetDir()/*��ȡ����*/
{
    fd_set fd;
    struct timeval tv;
    int dw;
    char buf;
    int len,iFlag;
    int maxfd;
    char ch[2] = {0};

    FD_ZERO(&fd);
    FD_SET(0,&fd);
    FD_SET(giPipeFd,&fd);

    maxfd = giPipeFd+1;

    tv.tv_sec = 2;/*mod 2014-1029*/
    tv.tv_usec = 0;/*MAX_TIME - (MAX_TIME-MIN_TIME)/MAX_LEVEL*giLevel;/*�����ٶ�*/

    dw = select(maxfd,&fd,NULL,NULL,&tv);
    if(dw == 0)
    {
        return DOWN;
    }
    if(FD_ISSET(giPipeFd,&fd))
    {
        read(giPipeFd,ch,sizeof(ch));
        return DOWN;
    }
    if(FD_ISSET(0,&fd))
    {
        len = read(0,&buf,1);
        if(len == 1)
        {
            switch(buf)
            {
            case 'w':
                return UP;//UP
            case 's':
                return DOWN;//DOWN
            case 'a':
                return LEFT;//RIGHT
            case 'd':
                return RIGHT;//LEFT

            default  :
                return(DOWN);
            }
        }
    }
}

static void zDrawBlock(ZzBody block,int iflag)/*��ʾһ������*/
{
    int i;
    for(i=0; i<4; i++)
    {
        ZDrawOneNode(block.ZBody[i][1],block.ZBody[i][0],iflag);
    }
}
static void zNextBlock(ZzBody *block,int Dir)/*�ƶ�������κ�ķ��������*/
{
    int i;
    int x = 0;
    do
    {
        switch(Dir)
        {
        /*������ת*/
        case UP:
            if(block->flag == 1)
            {
			break;
            }
            else if(block->flag == 4 || block->flag ==5 || block->flag == 6)
            {
                block->direction = (++block->direction)%4;/*���ַ���*/
            }
            else
            {
                block->direction = (++block->direction)%2;/*ֻ�����ַ���*/
            }
            ZInitBlock(block,1);
            break;
        /*��������*/
        case DOWN:

            for(i=0; i<4; i++)
            {
                block->ZBody[i][1] += 1;
            }
            break;
        /*���������ƶ�*/
        case LEFT:
            x -= 2;
        case RIGHT:
            x += 1;
            for(i=0; i<4; i++)
            {
                block->ZBody[i][0] += x;
            }
            break;
        default:
            break;
        }
    }
    while(0);
}
/*
int zJudgeBlock(ZzBody block)/*�����������
/*1������� �����µķ��� 2��������������� �����м俨ס�� �޷������ƶ� 3:������ ��Ϸ����
{

}*/
static void DrawScreen(int flag)/*������ ȫ�����(0) ����ȫ����ʾ(1)*/
{
    int x,y;
    for(y=WIN_Y2-2; y>(WIN_Y2-giHight-2); y--)
    {
        for(x = WIN_X1; x<WIN_X2-1; x++)
        {
            if(block_screen[y][x] == 1)/*��ʾ��������λ�� ���з����*/
                ZDrawOneNode(y+1,x+1,flag);
        }
    }
}
static void JudgeScreen()/*�鿴�Ƿ��з������������ģ�����еĻ���������*/
{
    int i,x,y;
    int len = 0;
    int sum = 0,score = 0;
    for(y=WIN_Y2-2; y>(WIN_Y2-giHight-2); y--) /*������������*/
    {

        len = 0;
        for(x = WIN_X1; x<WIN_X2-1; x++)
        {
            if(0 == block_screen[y][x])
                break;
	     len++;
        }
        if(len == WIDTH)/*�Ѻ�������ϸ���*/
        {
            DrawScreen(0);
            usleep(50000);/*��ͣ0.05��*/
            sum++;
            for(i = y; i>(WIN_Y2-giHight-2); i--)
            {
                memcpy(&block_screen[i][1],&block_screen[i-1][1],sizeof(int)*WIDTH);
            }
            DrawScreen(1);
            giHight--;
            y++;/*����һ�к��������ƣ���y�����겻��*/
        }
    }
    switch(sum)
    {
    case 1:
        score = 1;
        break;
    case 2:
        score = 3;
        break;
    case 3:
        score = 5;
        break;
    case 4:
        score = 10;
        break;
    default:
        break;
    }

    giScore+= score;/*�жϵ÷�*/

    giLevel = giScore/50;/*�ٶ� ����*/
}
/*
void JudgeHight(ZzBody block)
{

}*/

static int zJudgeNew(ZzBody block,int dir)/*�ж��ƶ���ķ����Ƿ��ײ���߽����ײ�����еķ���*/
{
    int i,j,height;
    if(dir == DOWN)/*�Ƿ�ײ���±߽�  ��Ҫ�����µķ���*/
    {
        for(i=0; i<4; i++)
        {
            if(block.ZBody[i][1] >= (WIN_Y2-1))
            {
                for(j=0; j<4; j++)
                {
                    height = WIN_Y2-block.ZBody[j][1];/*���¸߶�*/
                    if(height > giHight)
                        giHight = height;
                    block_screen[block.ZBody[j][1]-1][block.ZBody[j][0]-1] = 1;/*���鲻���ƶ� �̶���ĳ��λ��*/
                }
                //JudgeHight(block);
                if(giHight >= 20)/*������ ������Ϸ*/
                    return -2;
                return -1;
            }
            if(block_screen[block.ZBody[i][1]][block.ZBody[i][0]-1] == 1)
            {
                for(j=0; j<4; j++)
                {
                    height = WIN_Y2-block.ZBody[j][1];
                    if(height > giHight)
                        giHight = height;
                    block_screen[block.ZBody[j][1]-1][block.ZBody[j][0]-1] = 1;
                }
                //JudgeHight(block);
                if(giHight >= 20)
                    return -2;
                return -1;
            }
        }
    }
    else if(dir == LEFT)/*�Ƿ�������߽�*/
    {
        for(i=0; i<4; i++)
        {
            if(block.ZBody[i][0] <= (WIN_X1+1))
                return 0;
            if(block_screen[block.ZBody[i][1]-1][block.ZBody[i][0]-2] == 1)
                return 0;
        }
    }
    else if(dir == RIGHT)/*�Ƿ������ұ߽�*/
    {
        for(i=0; i<4; i++)
        {
            if(block.ZBody[i][0] >= (WIN_X2-1))
                return 0;
            if(block_screen[block.ZBody[i][1]-1][block.ZBody[i][0]] == 1)
                return 0;
        }
    }
    return 1;
}

static void JudgeNext(ZzBody block)/*�ж���ת��ķ����Ƿ��ײ���߽����ײ�����еķ���*/
{
    int i;
    for(i=0; i<4; i++)
    {
        if(block_screen[block.ZBody[i][1]-1][block.ZBody[i][0]-1] == 1)/*ײ�����еķ���*/
        {
            giMoveErr = 1;
            return;
        }
        if((block.ZBody[i][0] >= WIN_X2) || (block.ZBody[i][0] <= WIN_X1) || (block.ZBody[i][1] >= WIN_Y2))/*ײ���߽�*/
        {
            giMoveErr = 1;
            return;
        }
    }
}
void *Send_Down_Cmd()
{
    int dw;
    int time;
    char ch[2];
    ch[1] = 1;

    while(1)
    {
        time = MAX_TIME - (MAX_TIME-MIN_TIME)/MAX_LEVEL*giLevel;
        usleep(time);
        write(giPipeFd,ch,sizeof(ch));
    }
}


void InitThread()
{
    pthread_t thread;
    mkfifo(PIPE_NAME,0600);

    giPipeFd = open(PIPE_NAME,O_RDWR);

    if(pthread_create(&thread,0,(void*)Send_Down_Cmd,(void*)0) == 0)
    {
        printf("Error:pthread_create\n");
        return;
    }
    sleep(1);
}
/*************************************************
main ������
**************************************************/
int main()
{
    int i,j,zDir,flag;
    ZzBody hh,hh_bak;
    int numb=0;


    InitThread();/*add 2014-1029*/
    ZInitScreen();
    ZDrawScope();

    while(1)
    {
        if(giNewBlock == 0)/*��Ҫ�����µ�*/
        {
            /*			if(numb)
            				goto LOOP;*/
            JudgeScreen();
            printf("\033[%dm\033[40;%dm\033[%d;%dH%s%d",1,1*3+30,24,1,"The Score:",giScore);
            ZInitBlock(&hh,0);
            giNewBlock = 1;
            //numb++;
        }
        if(!giMoveErr)
        {
            memcpy(&hh_bak,&hh,sizeof(ZzBody));
            zDrawBlock(hh_bak,1);
        }
        else
        {
            memcpy(&hh,&hh_bak,sizeof(ZzBody));
        }
        giMoveErr = 0;
        zDir = ZGetDir();
        printf("\033[%dm\033[40;%dm\033[%d;%dH%s%d",1,1*3+30,23,1,"The giHeight:",giHight);
#if 1
        flag = zJudgeNew(hh,zDir);
        if(flag == -2)/*�ж����ҡ��µ��ƶ�*/
        {
            printf("\033[%dm\033[40;%dm\033[%d;%dH%s",1,1*3+30,13,3,OVER);
            printf("\033[%dm\033[40;%dm\033[%d;%dH%s%d",1,1*3+30,14,3,"Your score:",giScore);
            goto LOOP;
        }
        else if(flag == -1)
        {
            giNewBlock = 0;
            continue;
        }
        else if(flag == 0)
        {
            continue;
        }
#endif

        fflush(stdout);

        zNextBlock(&hh,zDir);

        JudgeNext(hh);

        if(!giMoveErr)
            zDrawBlock(hh_bak,0);
    }
LOOP:
    sleep(1);
    ZRestoreScreen();
#if 0
    for(i = 0; i<WIN_Y2; i++)
    {
        for(j=0; j<WIN_X2; j++)
        {
            printf(" %d",block_screen[i][j]);
        }
        printf("\n");
    }
#endif
    return 0;
    /*
    	for(i=0;i<4;i++)
    	{
    		printf(" [%d,%d]",hh.ZBody[i][0],hh.ZBody[i][1]);
    	}
    	printf("\n");
    */
}
