

void init_game_vals(){
    uniform_vals.u_hpdata[0]=0.; //7
    uniform_vals.u_hpdata[1]=0.; //6
    uniform_vals.u_hpdata[2]=0.; //6
    uniform_vals.u_dataxx[1]=-100;
}

//----------------------------
//objects
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
    if(m_left){
        m_left_c=true;
        m_left=false;
        uniform_vals.u_hpdata[1]=6.;
        cltime=uniform_vals.u_time;
    }
    if(m_right){
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

//----------------------------



//----------------------------
//sound 
//mainsound func
double * Sound( float time ){
    const double two_pi = 6.283185307179586476925286766559;
    double frequency = 440.0; // 261.626;  // middle C
    static double ret[2];
    
    double value = sin(two_pi*frequency*fmod(time,1.))*exp(-3.0*fmod(time,1.));
    
    ret[0]=0.2*value;
    ret[1]=0.2*value;
    return ret;
}

//save to file
void write_word(FILE *f,uint32_t val, size_t sz){
    for (; sz; --sz, val >>= 8){
        fprintf(f,"%c",(char)(val & 0xFF));
    }
}

void gen_sound() {
    FILE *f;
    printf("Saving sound...\n");
    f = fopen("background.wav", "w");
    if (f==NULL) printf("Can not save sound to file.\n");
    else
    {
    fprintf(f,"%s","RIFF----WAVEfmt ");     // (chunk size to be filled in later)
    write_word( f,     16, 4 );  // no extension data
    write_word( f,      1, 2 );  // PCM - integer samples
    write_word( f,      2, 2 );  // two channels (stereo file)
    write_word( f,  44100, 4 );  // samples per second (Hz)
    write_word( f, 176400, 4 );  // (Sample Rate * BitsPerSample * Channels) / 8
    write_word( f,      4, 2 );  // data block size (size of two integer samples, one for each channel, in bytes)
    write_word( f,     16, 2 );  // number of bits per sample (use a multiple of 8)
    size_t data_chunk_pos = ftell(f);
    fprintf(f,"%s","data----");
    const double max_amplitude = 32760;  // "volume"
    double hz        = 44100;    // samples per second
    double seconds   = 60.5;      // time
    int N = hz * seconds;  // total number of samples
    for (int n = 0; n < N; n++)
    {
        double * val=Sound(n/hz);
        write_word( f, (int)((max_amplitude ) * val[0]), 2 ); //left ear
        write_word( f, (int)((max_amplitude ) * val[1]), 2 ); //right ear
    }
    size_t file_length = ftell(f);
    fseek(f, data_chunk_pos + 4 ,SEEK_SET);
    write_word( f, file_length - data_chunk_pos + 8, 4 );
    fseek(f,  0 + 4 ,SEEK_SET);
    write_word( f, file_length - 8, 4 ); 
    fclose(f);
    }
}

#ifdef USE_OPENAL

//play sound using OpenAL
#include <AL/al.h>
#include <AL/alc.h>
ALCdevice  * openal_output_device;
ALCcontext * openal_output_context;

ALuint internal_buffer;
ALuint streaming_source[1];

bool sound_backgound_isplayed=false;

int al_check_error(const char * given_label) {

    ALenum al_error;
    al_error = alGetError();

    if(AL_NO_ERROR != al_error) {

        printf("ERROR - %s  (%s)\n", alGetString(al_error), given_label);
        return al_error;
    }
    return 0;
}

void MM_init_al() {

    const char * defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);

    openal_output_device  = alcOpenDevice(defname);
    openal_output_context = alcCreateContext(openal_output_device, NULL);
    alcMakeContextCurrent(openal_output_context);

    // setup buffer and source

    alGenBuffers(1, & internal_buffer);
    al_check_error("failed call to alGenBuffers");
}

void MM_exit_al() {
    if(sound_backgound_isplayed)
    {
    ALenum errorCode = 0;

    // Stop the sources
    alSourceStopv(1, & streaming_source[0]);        //      streaming_source
    int ii;
    for (ii = 0; ii < 1; ++ii) {
        alSourcei(streaming_source[ii], AL_BUFFER, 0);
    }
    // Clean-up
    alDeleteSources(1, &streaming_source[0]);
    alDeleteBuffers(16, &streaming_source[0]);
    errorCode = alGetError();
    alcMakeContextCurrent(NULL);
    errorCode = alGetError();
    alcDestroyContext(openal_output_context);
    alcCloseDevice(openal_output_device);
    sound_backgound_isplayed=false;
    }
}

void MM_check_state_al() {
    if(sound_backgound_isplayed)
    {
    ALenum current_playing_state;
    alGetSourcei(streaming_source[0], AL_SOURCE_STATE, & current_playing_state);
    al_check_error("alGetSourcei AL_SOURCE_STATE");

    if(AL_PLAYING != current_playing_state) {
        printf("Audio end.\n");
        /* Dealloc OpenAL */
        MM_exit_al();
    }
    }
}

void MM_render_one_buffer() {

    double hz        = 44100;    // samples per second
    double seconds   = 60.5;      // time
    const double max_amplitude = 32760;  // "volume"
    size_t buf_size = seconds * hz;

    // allocate PCM audio buffer        
    short * samples = malloc(sizeof(short) * buf_size);

    for(int i=0; i<buf_size; i++) {
        double * val=Sound(i/hz);
        samples[i]=(short)((max_amplitude ) * (val[0]+val[1])/2);
    }

    /* upload buffer to OpenAL */
    alBufferData( internal_buffer, AL_FORMAT_MONO16, samples, buf_size, hz);
    al_check_error("populating alBufferData");

    free(samples);

    alGenSources(1, & streaming_source[0]);
    alSourcei(streaming_source[0], AL_BUFFER, internal_buffer);
    alSourcePlay(streaming_source[0]);
    sound_backgound_isplayed=true;
}

void init_play_background() {
    printf("Using OpenAL for sound.\n");
    MM_init_al();
    MM_render_one_buffer();
}
#else
void init_play_background() {
    printf("Sound disabled.\n");
}
void MM_check_state_al() {}
void MM_exit_al() {}
#endif

//----------------------------
