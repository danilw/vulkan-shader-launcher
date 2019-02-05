//vals for uniforms
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

bool drawdebug=false;
bool m_left=false;
bool m_right=false;
bool m_left_c=false;
bool m_right_c=false;

#define background 0
#define box 1
#define character 2
#define debug 3

#define spawn 0
#define walk 1
#define att 2

static struct update_vals uniform_vals;

#if defined(_WIN32)

#include <windows.h>
#include <time.h>
#define CLOCK_MONOTONIC_RAW 0
#define BILLION                             (1E9)

static BOOL g_first_time = 1;
static LARGE_INTEGER g_counts_per_sec;

int clock_gettime(int dummy, struct timespec *ct)
{
	LARGE_INTEGER count;

	if (g_first_time)
	{
		g_first_time = 0;

		if (0 == QueryPerformanceFrequency(&g_counts_per_sec))
		{
			g_counts_per_sec.QuadPart = 0;
		}
	}

	if ((NULL == ct) || (g_counts_per_sec.QuadPart <= 0) ||
		(0 == QueryPerformanceCounter(&count)))
	{
		return -1;
	}

	ct->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
	ct->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * BILLION) / g_counts_per_sec.QuadPart;

	return 0;
}

#else

#include <time.h>

#endif

//time in msec
long get_time_ticks(){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long ticks = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return ticks;
}


static long frames=0;
static long last_time=0;
static long shifttime=0;

//fps
void update_fps(){
    long ticks = get_time_ticks();
    if (frames == 0) {
        last_time = ticks;
    }
    frames++;
    if (ticks - last_time >= 1000) {
        uniform_vals.FPS = 1000.0 * frames / (ticks - last_time);
        //printf("FPS: %.2f\n", uniform_vals.FPS);
        frames = 0;
    }
}
static float pto=0.;
static long pppp=0.;

void pres_pause(struct demo *demo){
    long ticks = get_time_ticks();
    if(demo->pause)
    pppp=ticks;
    else
    pto+=(float)(ticks- pppp)/ 1000.0;
}

void update_time(bool pause){
    long ticks = get_time_ticks();
    if(!pause)
    uniform_vals.u_time=(float)(ticks - shifttime)/ 1000.0-pto;
}

// cross-platform sleep function
void sleep_ms(int milliseconds) 
{
#ifdef WIN32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&ts, NULL);
#endif
}

static long frame_time=0;

//60 fps lock
void update_60_fps(){
    long ticks = get_time_ticks();
    if(ticks-frame_time<(1./60.)*1000.)sleep_ms((1./60.)*1000.-(ticks-frame_time));
    frame_time=get_time_ticks();
}

void MM_check_state_al();
void update_char_anim();

static void uptate_my_vars(struct demo *demo){
    update_fps();
    update_60_fps();
    MM_check_state_al();
    update_time(demo->pause);
    update_char_anim();
}

void MM_exit_al();

static void clean_on_exit(){
    MM_exit_al();
}
void init_game_vals();
void init_play_background();
void gen_pos();
void init_size();

static void ex_init(){
    //sound file
    //gen_sound();
    
    //OpenAl
    init_play_background();
    init_size();
    init_game_vals();
    gen_pos();
    shifttime = get_time_ticks();
}

















