// Danil, 2020 Vulkan shader launcher, self https://github.com/danilw/vulkan-shader-launcher

#include <math.h>

struct update_vals {
    float u_mouse[2];
    float p_pos[200][2];
    float p_time[200];
    bool p_isl[200];
    int p_id[200];
    float FPS;
    float u_time;
    float obj_size[4][2];
    float u_hpdata[4];
    float u_valt[2];
    float u_dataxx[4];
    float ishit;
};

bool m_left=false;
bool m_right=false;
bool m_left_c=false;
bool m_right_c=false;

static struct update_vals uniform_vals; //game data

void init_game_vals(){
    uniform_vals.u_hpdata[0]=0.; //7
    uniform_vals.u_hpdata[1]=0.; //6
    uniform_vals.u_hpdata[2]=0.; //6
    uniform_vals.u_dataxx[1]=-100;
}

//----------------------------
//objects
#define background 0
#define box 1
#define character 2
#define debug 3

#define spawn 0
#define walk 1
#define att 2
void init_size(){
    uniform_vals.obj_size[background][0]=16./9.;
    uniform_vals.obj_size[background][1]=1.;
    uniform_vals.obj_size[debug][0]=0.3;
    uniform_vals.obj_size[debug][1]=0.3;
    uniform_vals.obj_size[box][0]=.5;
    uniform_vals.obj_size[box][1]=.5;
    uniform_vals.obj_size[character][0]=10.;
    uniform_vals.obj_size[character][1]=10.;
}


//particles

void gen_pos(){
    for(int i=0;i<200;i++){
    uniform_vals.p_pos[i][0]=(float)i/200.;
    uniform_vals.p_pos[i][1]=(float)i/400.;
    uniform_vals.p_id[i]=box;
    uniform_vals.p_time[i]=0.;
    uniform_vals.p_isl[i]=false;
    }
} 

void update_mouse(bool left, bool right){
    m_left=left;
    m_right=right;
}

void update_time(float time){
    uniform_vals.u_time=time;
}

static bool ehx=false;
static bool ehx2=false;
static float lspwn=0.;
void update_pos(){
    float mpi=3.1415926;
    float timerx=(uniform_vals.u_time-4.)/50.>0.?(uniform_vals.u_time-4.)/50.:0.;
    timerx=timerx>1.?1.:timerx;
    float tevx=1.-0.9*timerx;
    for (int i=0.;i<20;i++){
    if(uniform_vals.p_isl[i]){
        if(ehx2)if((uniform_vals.p_pos[i][0]<-0.67+0.06)&&(uniform_vals.p_pos[i][0]>-0.67-0.06))uniform_vals.p_isl[i]=false;
        if(ehx)if(uniform_vals.p_pos[i][1]>-0.3+0.42)uniform_vals.p_isl[i]=false;
        if(uniform_vals.p_pos[i][1]>-0.3+0.5){if(uniform_vals.p_pos[i][0]<-0.67+0.06)
        uniform_vals.p_pos[i][0]=-0.67+0.165*sin(-mpi/2.+(uniform_vals.u_time-uniform_vals.p_time[i])*(2.5));
        else uniform_vals.p_pos[i][0]+=0.01;
        }
        else
        uniform_vals.p_pos[i][0]=-0.67+0.165*sin(-mpi/2.+(uniform_vals.u_time-uniform_vals.p_time[i])*2.5);
        if(uniform_vals.p_pos[i][1]<-0.3+0.5)
        uniform_vals.p_pos[i][1]=-0.04+0.26*sin(-mpi/2.+(uniform_vals.u_time-uniform_vals.p_time[i])*(.5+0.5*(1.-tevx)));
        
        if(uniform_vals.p_pos[i][0]>0.45){uniform_vals.p_isl[i]=false;uniform_vals.u_dataxx[1]=uniform_vals.u_time;uniform_vals.u_hpdata[0]+=1;if(uniform_vals.u_hpdata[0]>7.)uniform_vals.u_hpdata[0]=0.;}
    }else{
        if((uniform_vals.u_time-4.>0.)&&(!ehx2)){
            if((uniform_vals.u_time-4.)-lspwn>tevx*2.){
            lspwn=(uniform_vals.u_time-4.);
            uniform_vals.p_isl[i]=true;
            uniform_vals.p_pos[i][0]=-0.67-0.165;
            uniform_vals.p_pos[i][1]=-0.04-0.26;
            uniform_vals.p_time[i]=uniform_vals.u_time;
            }
        }
    }
    }
} 

static float cltime=0.;
static float cltime2=-100.;

void update_char_anim(){
    update_pos();
    uniform_vals.u_dataxx[0]=uniform_vals.u_time-cltime2;
    float itm=(uniform_vals.u_time-4.>0.?uniform_vals.u_time-4.:0.);
    if(itm<=3.)m_right=m_left=false;
    if(itm<=1.){
    uniform_vals.u_valt[0]=itm;
    uniform_vals.u_valt[1]=spawn;
    return;
    }
    if(itm<=3.){
    uniform_vals.u_valt[0]=itm;
    uniform_vals.u_valt[1]=walk;
    return;
    }
    if(m_left&&(!m_left_c)){
        m_left_c=true;
        m_left=false;
        uniform_vals.u_hpdata[1]=6.;
        cltime=uniform_vals.u_time;
    }
    if(m_right&&(!m_right_c)){
        m_right_c=true;
        m_right=false;
        uniform_vals.u_hpdata[2]=6.;
        cltime2=uniform_vals.u_time;
    }
    ehx2=false;
    if(m_right_c){
        uniform_vals.u_hpdata[2]=6.-(int)(((uniform_vals.u_time-ceil(cltime2)))*1.);
        if((uniform_vals.u_time-ceil(cltime2)>1.))ehx2=false;
        else ehx2=true;
        if((uniform_vals.u_time-ceil(cltime2)>6.)){
            uniform_vals.u_hpdata[2]=0.;
            m_right_c=false;
        }
    }
    if(m_left_c){
    bool tv=((uniform_vals.u_time-ceil(cltime))>0.);
    itm=((tv)&&(uniform_vals.u_time-ceil(cltime)<=9.))?uniform_vals.u_time-ceil(cltime):itm;
    uniform_vals.u_valt[0]=itm;
    uniform_vals.u_valt[1]=((tv)&&(itm<=9.))?att:walk;
    ehx=((tv)&&(itm<=3.8)&&(itm>=2.));
    if((tv)&&(uniform_vals.u_time-ceil(cltime)>8.)){
        uniform_vals.u_hpdata[1]=6.-(int)(((uniform_vals.u_time-ceil(cltime))-8.)*3.);
        if(uniform_vals.u_hpdata[1]<=0.){
        uniform_vals.u_hpdata[1]=0.;
        ehx=false;
        m_left_c=false;}
        }
    return;
    }
    uniform_vals.u_valt[0]=itm;
    uniform_vals.u_valt[1]=walk;
    return;
}

void engine_init(){
    init_size();
    init_game_vals();
    gen_pos();
}

//----------------------------
