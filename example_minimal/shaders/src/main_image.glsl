//base on my shader https://www.shadertoy.com/view/lttfR8

// added debugdraw and pause effect, space pause, 1(keyboard) debugdraw
// 2(key) enable/diable fps lock(nvidia bug)



#define MD(a) mat2(cos(a), -sin(a), sin(a), cos(a))

#define PI (4.0 * atan(1.0))
#define TWO_PI PI*2.

const vec3 green_ = vec3(0x58, 0x8b, 0x8c) / float(0xff);
const vec3 green2 = vec3(0x22, 0x2e, 0x2a) / float(0xff);
const vec3 white_ = vec3(0xe3, 0xe4, 0xdf) / float(0xff);
const vec3 black_ = vec3(0x84, 0x82, 0x85) / float(0xff);

vec3 background_col(vec2 p){
    if(p.y>0.35) return smoothstep(0.55,0.4,p.y)*green_;
    return white_;
}

vec3 background_black(vec2 p){
    return smoothstep(0.,-0.5,p.y)*black_;;
}

float xRandom(float x) {
    return fract(dot(sin(x * 591.32 ), cos(x * 391.32 )));
}

//using http://www.iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm
float sdLine( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

float sdCircle( vec2 p, float r )
{
  return length(p) - r;
}

float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,vec2(0))) + min(max(d.x,d.y),0.0);
}

//layer 1
float l1(vec2 p){
    vec2 res = vec2(16./9.,1.);
    vec2 op=p;
    p=vec2((mod(floor(op.y/(1.))*.689+op.x,res.x/2.)-res.x/1.9),mod(op.y,1.)-1./2.);
    float d;
    d=smoothstep(0.005,0.01,sdLine(p,vec2(-res.x/2.,-0.5),vec2(-res.x/2.+0.2,0.5)));
    d=min(d,smoothstep(0.005,0.01,sdLine(p,vec2(-res.x/2.+0.3,-0.5),vec2(-res.x/2.+0.5,0.5))));
    d=min(d,smoothstep(0.0,0.008,sdLine(p,vec2(-res.x/2.+0.325,-0.5),vec2(-res.x/2.+0.525,0.5))));
    p=vec2(mod(op.x,res.x),mod(floor(op.x/(res.x))*0.25+op.y,0.4)-0.4*1.5);
    d=min(d,smoothstep(0.005,0.01,sdLine(p,vec2(-res.x,-0.),vec2(res.x,-0.5))));
    return d;
}

//layer 2
float l2(vec2 p){
    vec2 op=p;
    float d;
    p.x=mod(p.x,0.5)-0.5/2.;
    p.y+=-0.051;
    d=smoothstep(0.025,0.03,sdLine(p,vec2(0.,0.3),vec2(-0.4,0.6)));
    d=min(d,smoothstep(0.015,0.02,sdLine(p,vec2(0.,0.3),vec2(0.4,0.6))));
    d=min(d,smoothstep(0.015,0.02,sdLine(p,vec2(-0.02,0.34),vec2(0.38,0.64))));
    p=op;
    d*=step(0.4,p.y);
    
    p.x=mod(p.x+0.35,0.7)-0.7/2.;
    d=max(d,smoothstep(0.01,0.005,sdCircle(p,0.28)));
    
    //p=op;
    
    d=min(d,smoothstep(0.005,0.01,sdCircle(p,0.123)));
    
    vec2 res = iResolution.xy / iResolution.y;
    //fan speed
    //p*=MD(mod(iTime*8.*(0.5-xRandom(floor((op.x+(0.35))/0.7))),TWO_PI));
    p*=MD(mod(iTime*3.*((1.+0.2*floor(mod(((op.x+(0.35))/(0.7)),3.)))*.5-
                        (1.-0.2*floor(mod(((op.x+(0.35))/(0.7)),5.)))*
                        min(clamp(floor(mod(((op.x+(0.35*3.))/(0.7*3.)),2.))-
                                     floor(mod(((op.x+(0.35))/(0.7)),2.)),-1.,1.),
                            floor(mod(((op.x+(0.35*3.))/(0.7*3.)),2.)))),
              TWO_PI));
    float a = atan(p.x,p.y);
    float r = length(p);
    p = vec2(a/(TWO_PI),r);
 	p = (1.0 * p) - vec2(0.5,0.42);
    p.xy=p.yx;
    p.y=mod(p.y,0.142)-0.142/2.;
    d=min(d,smoothstep(0.007,0.01,
                       sdBox(p+vec2(0.142+0.068,0.), //circle radius 0.123+fan borders=0.142
                             vec2(0.068,0.025))));
    d=min(d,smoothstep(0.005,0.01,sdBox(p+vec2(0.242+0.068,0.),vec2(0.068,0.0125))));
    
    return d;
}

//layer 3
float l3(vec2 p){
    float d;
    d=smoothstep(0.005,0.0,abs(p.y+0.38));
    p.x=mod(p.x,0.025)-0.025/2.;
    if(p.y<-0.38)
    d=max(d,smoothstep(0.005,0.0,abs(p.x)));
    return d;
}

vec3 map_bg(vec2 p){
    p.x+=iTime/15.; //scroll speed
    vec2 op=p;
    float d=l1(p);
    vec3 col;
    p=(op+1.)*MD(-.32);
    d=min(d,l1(p));
    p=op;
    col=d*background_col(p);
    d=min(d,l2(p));
    col=mix(col,background_black(p),1.-d);
    p=op;
    p.x+=iTime/60.; //scroll speed 2
    d=l3(p);
    col=mix(col,green2,d);
    return col;
}

void maindraw( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 res = iResolution.xy / iResolution.y;
    vec2 uv = (fragCoord.xy) / iResolution.y - res/2.0;
    //uv*=2.0;
    vec3 col;
    
    col = map_bg(uv);

    // Output to screen
    fragColor = vec4(col,1.0);
}


int PrintInt( in vec2 uv, in int value, const int maxDigits );

void debugdraw( out vec4 fragColor, in vec2 fragCoord )
{
    fragColor=vec4(0.);
    vec2 res=(iResolution.xy / iResolution.y);
    vec2 uv = (fragCoord.xy) / iResolution.y - res/2.0;
    vec2 ouv=uv;
    uv.x+=0.6;
    uv.y+=-0.3;
    
    int d=0;
    uv*=12.;
    d+=PrintInt(uv,int(iResolution.x),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(iResolution.y),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(iMouse.x),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(iMouse.y),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,(int(iMouse.z)<0?9000+abs(int(iMouse.z)):int(iMouse.z)),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,(int(iMouse.w)<0?9000+abs(int(iMouse.w)):int(iMouse.w)),4);
    uv.y+=0.2*7.;
    
    uv=ouv;
    uv.x+=0.2;
    uv.y+=-0.3;
    
    uv*=12.;
    d+=PrintInt(uv,int(iDate.x),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(iDate.y),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(iDate.z),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(iDate.w/3600.),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(mod(iDate.w/60.,60.)),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(mod(iDate.w,60.)),4);
    uv.y+=0.2*7.;
    fragColor=vec4(vec3(d),1.);
    
    uv=ouv;
    uv.x+=-0.2;
    uv.y+=-0.3;
    
    uv*=12.;
    d+=PrintInt(uv,iFrame,4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(iTime*10.),4);
    uv.y+=0.2*7.;
    d+=PrintInt(uv,int(1./iTimeDelta),4);
    uv.y+=0.2*7.;
    fragColor=vec4(vec3(d),1.);

}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec4 ocol=vec4(0.);
	maindraw(ocol,fragCoord);
	if(is_debugdraw){
		vec4 tcol=vec4(0.);
		debugdraw(tcol,fragCoord);
		ocol=mix(ocol,tcol,tcol.r*0.85);
	}
	if(is_pause){
		ocol=vec4(vec3(dot(ocol.rgb,vec3(1.))/3.),1.);
		ocol.r*=1.5;
	}
	fragColor=ocol;
}

//https://www.shadertoy.com/view/ldsyz4
// The MIT License
// Copyright © 2017 Inigo Quilez
// Digit data by P_Malin (https://www.shadertoy.com/view/4sf3RN)
const int[] font = int[](0x75557, 0x22222, 0x74717, 0x74747, 0x11574, 0x71747, 0x71757, 0x74444, 0x75757, 0x75747);
const int[] powers = int[](1, 10, 100, 1000, 10000, 100000, 1000000);

int PrintInt( in vec2 uv, in int value, const int maxDigits )
{
    if( abs(uv.y-0.5)<0.5 )
    {
        int iu = int(floor(uv.x));
        if( iu>=0 && iu<maxDigits )
        {
            int n = (value/powers[maxDigits-iu-1]) % 10;
            uv.x = fract(uv.x);//(uv.x-float(iu)); 
            ivec2 p = ivec2(floor(uv*vec2(4.0,5.0)));
            return (font[n] >> (p.x+p.y*4)) & 1;
        }
    }
    return 0;
}
