#define SS(x, y, z) smoothstep(x, y, z)
#define MD(a) mat2(cos(a), -sin(a), sin(a), cos(a))
#define PI (4.0 * atan(1.0))
#define TAU (2.*PI)
#define E exp(1.)
#define res (iResolution.xy / iResolution.y)

int PrintInt( in vec2 uv, in int value, const int maxDigits );

float angle2d(vec2 c, vec2 e) {
    float theta = atan(e.y-c.y, e.x-c.x);
    return theta;
}

// Created by Danil (2019+) https://github.com/danilw
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

//types
#define background 0
#define box 1
#define character 2
#define debug 3

#define spawn 0
#define walk 1
#define att 2

float sdBox( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,vec2(0))) + min(max(d.x,d.y),0.0);
}

float sdCircle( vec2 p, float r )
{
  return length(p) - r;
}

float zv;
vec2 res_g;
vec2 draw_pos=vec2(0.,0.);


vec4 mifgergg(vec2 uv )
{
	float f = 0.0;
		float s = sin(iTime*3.0 + 2. * 0.0031415926) * 0.8;
		float c = cos(iTime*2.0+2. *0.0031415926)*0.8;
		f = 0.011/abs(length(uv)-2./1000.);
    vec3 destColor = vec3(.70,0.24,.20)+0.2+0.2*vec3(s,0.,c);
        vec3 col = destColor*f;
        f=(clamp(f,0.,1.))*SS(0.03,0.0,sdCircle(uv,0.014));
    return vec4(col,f);
}

vec4 draw_box(vec2 p){
    vec4 col=vec4(0.);
    p+=draw_pos;
    col=mifgergg(p);
    return col;
}



float draw_circle(vec2 p){
    float d=0.;
    p+=draw_pos;
    float radius=0.1;
    d=max(d,SS(zv,-zv-0.1,sdCircle(p,radius)));
    return d;
}

float draw_debug(vec2 p){
    float d=0.;
    p+=draw_pos;
    p.x+=0.1;
    if(p.y<0.05){
    ivec2 ires=ivec2(iResolution.xy);
    ivec2 size=ivec2((ires.x/1000>=1)?4:3,(ires.y/1000>=1)?4:3);
    d=max(d,float(PrintInt(p*35.,ires.x,size.x)));
    p.x+=-0.15;
    d=max(d,float(PrintInt(p*35.,ires.y,size.y)));
    }else
    if(p.y<0.05*2.){
    p.y+=-0.05;
    ivec2 im=ivec2(iMouse.xy);
    ivec2 ires=ivec2(iResolution.xy);
    ivec2 size=ivec2((ires.x/1000>=1)?4:3,(ires.y/1000>=1)?4:3);
    d=max(d,float(PrintInt(p*35.,im.x,size.x)));
    p.x+=-0.15;
    d=max(d,float(PrintInt(p*35.,im.y,size.y)));
    }
    else if(p.y<0.05*3.){
    p.y+=-0.05*2.;
    int fps=int(FPS);
    int sizefps=(fps/100>=1)?3:2;
    d=max(d,float(PrintInt(p*35.,fps,sizefps))); 
    }
    return d;
}





mat2 mm2(in float a){float cx = cos(a), s = sin(a);return mat2(cx,-s,s,cx);}
float tri(in float x){return abs(fract(x)-.5);}
vec2 tri2(in vec2 p){return vec2(tri(p.x+tri(p.y*2.)),tri(p.y+tri(p.x*2.)));}
mat2 m22 = mat2( 0.970,  0.242, -0.242,  0.970 );
float triangleNoise(in vec2 p)
{
    float z=1.5;
    float z2=1.5;
	float rz = 0.;
    vec2 bp = p;
	for (float i=0.; i<=3.; i++ )
	{
        vec2 dg = tri2(bp*2.)*.8;
        dg *= mm2(iTime*.52);
        p += dg/z2;
        bp *= 1.6;
        z2 *= .6;
		z *= 1.8;
		p *= 1.2;
        p*= m22;
        rz+= (tri(p.x+tri(p.y)))/z;
	}
	return rz;
}


float sdTriangleIsosceles( in vec2 q, in vec2 p )
{
    p.y -= 0.5;
    p.x = abs(p.x);
    
	vec2 a = p - q*clamp( dot(p,q)/dot(q,q), 0.0, 1.0 );
    vec2 b = p - q*vec2( clamp( p.x/q.x, 0.0, 1.0 ), 1.0 );
    
    float s = -sign( q.y );

    vec2 d = min( vec2( dot( a, a ), s*(p.x*q.y-p.y*q.x) ),
                  vec2( dot( b, b ), s*(p.y-q.y)  ));

	return -sqrt(d.x)*sign(d.y);
}

float sdEllipse( in vec2 p, in vec2 ab )
{
    p = abs(p); if( p.x > p.y ) {p=p.yx;ab=ab.yx;}
    float l = ab.y*ab.y - ab.x*ab.x;
	
    float m = ab.x*p.x/l;      float m2 = m*m; 
    float n = ab.y*p.y/l;      float n2 = n*n; 
    float c = (m2+n2-1.0)/3.0; float c3 = c*c*c;
	
    float q = c3 + m2*n2*2.0;
    float d = c3 + m2*n2;
    float g = m + m*n2;

    float co;
    if( d < 0.0 )
    {
        float h = acos(q/c3)/3.0;
        float s = cos(h);
        float t = sin(h)*sqrt(3.0);
        float rx = sqrt( -c*(s + t + 2.0) + m2 );
        float ry = sqrt( -c*(s - t + 2.0) + m2 );
        co = (ry+sign(l)*rx+abs(g)/(rx*ry)- m)/2.0;
    }
    else
    {
        float h = 2.0*m*n*sqrt( d );
        float s = sign(q+h)*pow(abs(q+h), 1.0/3.0);
        float u = sign(q-h)*pow(abs(q-h), 1.0/3.0);
        float rx = -s - u - c*4.0 + 2.0*m2;
        float ry = (s - u)*sqrt(3.0);
        float rm = sqrt( rx*rx + ry*ry );
        co = (ry/sqrt(rm-rx)+2.0*g/rm-m)/2.0;
    }

    vec2 r = ab * vec2(co, sqrt(1.0-co*co));
    return length(r-p) * sign(p.y-r.y);
}

float cro(in vec2 a, in vec2 b ) { return a.x*b.y - a.y*b.x; }
float sdUnevenCapsule( in vec2 p, in vec2 pa, in vec2 pb, in float ra, in float rb )
{
    p  -= pa;
    pb -= pa;
    float h = dot(pb,pb);
    vec2  q = vec2( dot(p,vec2(pb.y,-pb.x)), dot(p,pb) )/h;
    
    //-----------
    
    q.x = abs(q.x);
    
    float b = ra-rb;
    vec2  c = vec2(sqrt(h-b*b),b);
    
    float k = cro(c,q);
    float m = dot(c,q);
    float n = dot(q,q);
    
         if( k < 0.0 ) return sqrt(h*(n            )) - ra;
    else if( k > c.x ) return sqrt(h*(n+1.0-2.0*q.y)) - rb;
                       return m                       - ra;
}

float sdTriangle( in vec2 p0, in vec2 p1, in vec2 p2, in vec2 p )
{
	vec2 e0 = p1 - p0;
	vec2 e1 = p2 - p1;
	vec2 e2 = p0 - p2;

	vec2 v0 = p - p0;
	vec2 v1 = p - p1;
	vec2 v2 = p - p2;

	vec2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
	vec2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
	vec2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
    
    float s = sign( e0.x*e2.y - e0.y*e2.x );
    vec2 d = min( min( vec2( dot( pq0, pq0 ), s*(v0.x*e0.y-v0.y*e0.x) ),
                       vec2( dot( pq1, pq1 ), s*(v1.x*e1.y-v1.y*e1.x) )),
                       vec2( dot( pq2, pq2 ), s*(v2.x*e2.y-v2.y*e2.x) ));

	return -sqrt(d.x)*sign(d.y);
}


float linearstep(float begin, float end, float t) {
    return clamp((t - begin) / (end - begin), 0.0, 1.0);
}

float easeInOutExpo(float t) {
    if (t == 0.0 || t == 1.0) {
        return t;
    }
    if ((t *= 2.0) < 1.0) {
        return 0.5 * pow(2.0, 10.0 * (t - 1.0));
    } else {
        return 0.5 * (-pow(2.0, -10.0 * (t - 1.0)) + 2.0);
    }
}

float timey(float t){
    float t4 = linearstep(0., 0.5, t);
    float t5 = linearstep(0.5, 1.0, t);
    return t4 - t5;
}

float timex(float t){
    float t4 = linearstep(0.2, 0.5, t);
    float t5 = linearstep(0.2, .8, t);
    return t4 - t5;
}

float timex2(float t){
    float t4 = linearstep(0.1, 0.5, t);
    float t5 = linearstep(0.2, .8, t);
    return t4 - t5;
}

float lx_s(vec2 p,float ez,float atime){
    float d=1.;
    vec2 op=p;
    p.y+=0.3;
    p*=ez;
    vec2 v1 = vec2(0.,0.);
	vec2 v2 = vec2(0.,-01.0);
    float r1 = 0.25;
    float r2 = 0.013;
    float anim_timer=atime; 
    vec2 tpp=vec2(-0.15,-0.15+0.15*linearstep(0., .3, anim_timer));
    vec2 tp=(tpp*ez+p)*MD(-PI/5.+PI/5.*linearstep(.8, .4, anim_timer));
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    d = min(d,sdCircle((vec2(tpp.x*linearstep(.8, .35, anim_timer),-0.015+0.015*linearstep(.3, .35, anim_timer)+0.1*linearstep(.8, .35, anim_timer)-0.05*linearstep(.8, .35, anim_timer)+tpp.y+0.02)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(tpp.x*1.*linearstep(.8, .35, anim_timer)+0.008+0.12,-0.05+0.05*linearstep(.3, .4, anim_timer)+0.1*linearstep(.8, .35, anim_timer)-0.05*linearstep(.8, .35, anim_timer)+tpp.y-0.12)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(-0.2,-0.05+0.05*linearstep(.3, .4, anim_timer)+0.1*linearstep(.8, .35, anim_timer)-0.05*linearstep(.8, .35, anim_timer)+tpp.y+0.12)+op)*ez,0.02*ez));
    //anim_timer=1.-anim_timer;
    tpp=vec2(-0.15+.3*linearstep(.3, .8, anim_timer),-0.25+0.15*linearstep(0., .3, anim_timer)+0.1*linearstep(0.3, .8, anim_timer));
    tp=(tpp*ez+p)*MD(-PI/5.+2.*PI/5.-PI/5.*linearstep(.8, .4, anim_timer));
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    return d;
}

float lx(vec2 p,float ez,float atime){
    float d=1.;
    vec2 op=p;
    p.y+=0.3;
    p*=ez;
    vec2 v1 = vec2(0.,0.);
	vec2 v2 = vec2(0.,-01.0);
    float r1 = 0.25;
    float r2 = 0.013;
    float anim_timer=atime; 
    vec2 tpp=vec2(-0.15+.3*anim_timer,-0.15*timey(anim_timer));
    vec2 tp=(tpp*ez+p)*MD(-PI/5.+2.*PI/5.*anim_timer);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    d = min(d,sdCircle((vec2(0.02*sin(PI*2.*anim_timer),tpp.y/3.+0.02)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(0.008*cos(PI*2.*anim_timer)+0.12,tpp.y/6.-0.12)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(0.012*sin(PI*2.*anim_timer)-0.2,(-0.1*timex2(anim_timer))/3.5+0.12)+op)*ez,0.02*ez));
    anim_timer=1.-anim_timer;
    tpp=vec2(-0.15+.3*anim_timer,0.);
    tp=(tpp*ez+p)*MD(-PI/5.+2.*PI/5.*anim_timer);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    return d;
}

float lx_a(vec2 p,float ez,float atime){
    float d=1.;
    vec2 op=p;
    p.y+=0.3;
    p*=ez;
    vec2 v1 = vec2(0.,0.);
	vec2 v2 = vec2(0.,-01.0);
    float r1 = 0.25;
    float r2 = 0.013;
    vec2 tpp=vec2(-0.15,-0.);
    vec2 tp=(tpp*ez+p)*MD(-PI/5.);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    d = min(d,sdCircle((vec2(0.,0.02)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(0.008+0.12+0.122*linearstep(0.08, .7, atime),0.09*linearstep(0.08, .7, atime)-0.12)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(-0.2+0.5*linearstep(0.05, .7, atime)-0.05*linearstep(0.3, .7, atime),0.12-0.15*(linearstep(0.15, .6, atime)*linearstep(0.15, .8, atime)))+op)*ez,0.02*ez));
    tpp=vec2(0.15,0.);
    tp=(tpp*ez+p)*MD(PI/5.);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    return d;
}

float lx_a2(vec2 p,float ez,float atime){
    float d=1.;
    vec2 op=p;
    p.y+=0.3;
    p*=ez;
    vec2 v1 = vec2(0.,0.);
	vec2 v2 = vec2(0.,-01.0);
    float r1 = 0.25;
    float r2 = 0.013;
    vec2 tpp=vec2(-0.15,-0.);
    vec2 tp=(tpp*ez+p)*MD(-PI/5.);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    d = min(d,sdCircle((vec2(0.,0.02)+op)*ez,0.02*ez));
    float timer=linearstep(0.0,0.6,atime);
    d = min(d,sdCircle((vec2(-0.4*timer+0.008+0.12+0.122*linearstep(0.08, .7, 1.),0.09*linearstep(0.08, .7, 1.)-0.12)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(-0.2+0.5*linearstep(0.05, .7, 1.)-0.05*linearstep(0.3, .7, 1.),0.12-0.15*(linearstep(0.15, .6, 1.)*linearstep(0.15, .8, 1.)))+op)*ez,0.02*ez));
    tpp=vec2(0.15,0.);
    tp=(tpp*ez+p)*MD(PI/5.);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    return d;
}

float lx_a22(vec2 p,float ez,float atime){
    float d=1.;
    vec2 op=p;
    //atime=1.;
    p.y+=0.3;
    p*=ez;
    vec2 v1 = vec2(0.,0.);
	vec2 v2 = vec2(0.,-01.0);
    float r1 = 0.25;
    float r2 = 0.013;
    vec2 tpp=vec2(-0.15,-0.);
    vec2 tp=(tpp*ez+p)*MD(-PI/5.);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    d = min(d,sdCircle((vec2(0.,0.02)+op)*ez,0.02*ez));
    float timer=linearstep(0.0,0.6,1.);
    float timer2=linearstep(0.0,1.,atime);
    d = min(d,sdCircle((vec2(-0.05*timer2-0.4*timer+0.008+0.12+0.122*linearstep(0.08, .7, 1.),0.15*timer2+0.09*linearstep(0.08, .7, 1.)-0.12)+op)*ez,0.02*ez));
    d = min(d,sdCircle((vec2(-0.122*timer2-0.2+0.5*linearstep(0.05, .7, 1.)-0.05*linearstep(0.3, .7, 1.),-0.09*timer2+0.12-0.15*(linearstep(0.15, .6, 1.)*linearstep(0.15, .8, 1.)))+op)*ez,0.02*ez));
    tpp=vec2(0.15,0.);
    tp=(tpp*ez+p)*MD(PI/5.);
    d = min(d,sdUnevenCapsule( tp, v1, v2, r1, r2 ));
    return d;
}

float lx_bow(vec2 p,float atime){
    float d=1.;
    p.y+=-0.03;
    p.y=abs(p.y);
    p+=vec2(0.33,.062);
    p*=MD(-0.2*linearstep(0.6,0.0,atime));
    d = min(d,sdTriangle(vec2(0.01,0.07),vec2(0.07,0.12),vec2(0.13,0.1),p));
    d = min(d,sdTriangle(vec2(0.2,0.35),vec2(0.06,0.11),vec2(0.11,0.12),p));
    p+=vec2(0.02,0.02);
    d = min(d,sdTriangle(vec2(0.25,0.4),vec2(0.15,0.25),vec2(0.165,0.32),p));
    return d;
}

float draw_char_s(vec2 p,float atime){
    float d=0.;
    float ez=25.;
    d=max(d,SS(zv*ez,-zv*ez,lx_s(p,ez,atime)));
    return d;
}

float draw_char_a(vec2 p,float atime){
    float d=0.;
    float ez=25.;
    d=max(d,SS(zv*ez,-zv*ez,lx_a(p,ez,atime)));
    return d;
}

float draw_char_a2(vec2 p,float atime){
    float d=0.;
    float ez=25.;
    d=max(d,SS(zv*ez,-zv*ez,lx_a2(p,ez,atime)));
    return d;
}

float draw_char_a22(vec2 p,float atime){
    float d=0.;
    float ez=25.;
    d=max(d,SS(zv*ez,-zv*ez,lx_a22(p,ez,atime)));
    return d;
}

float draw_char(vec2 p,float atime){
    float d=0.;
    float ez=25.;
    d=max(d,SS(zv*ez,-zv*ez,lx(p,ez,atime)));
    return d;
}

float draw_char_hh(vec2 p,float atime){
    float d=0.;
    float anim_timer=atime; 
    vec2 tpp=vec2(0.022*sin(PI*2.*anim_timer),-0.15*timex(anim_timer));
    p+=vec2(tpp.x,tpp.y/3.);
    d=max(d,SS(zv+0.,-zv-0.056,sdCircle(p+vec2(0.,-0.3),0.055)));
    
    return d;
}

float draw_char_bow(vec2 p,float atime){
    float d=0.;
    vec2 tpp=vec2(0.);
    p+=vec2(tpp.x,tpp.y/3.);
    d=max(d,SS(zv,-zv-0.016,lx_bow(p,atime)));
    float timer4=linearstep(2.5,03.,atime);
    d*=SS(linearstep(0.,0.4,atime)-timer4,linearstep(0.,0.4,atime)-timer4-0.1,2.*abs(p.y-0.03));
    
    return d;
}

float draw_char_hh_s(vec2 p,float atime){
    float d=0.;
    float anim_timer=atime; 
    vec2 tpp=vec2(-0.1+0.1*linearstep(0.35, .8, anim_timer),-0.15+0.15*linearstep(0., .3, anim_timer)-0.05*linearstep(.35, .75, anim_timer)+0.05*linearstep(.3, .45, anim_timer));
    p+=vec2(tpp.x,tpp.y);
    d=max(d,SS(zv+0.,-zv-0.056,sdCircle(p+vec2(0.,-0.3),0.055)));
    
    return d;
}

float draw_char_sh(vec2 p,bool ia){
    float d=0.;
    p+=vec2(0.008,-0.1);
    if(p.y<0.)p.y*=0.8;
    d=max(d,SS(zv,-zv-0.0155,-0.015+(sdEllipse(p*MD(0.23),2.*vec2(0.1,0.2)))));
    d*=SS(-0.15+.25*SS(0.,.25,iTime-u_dataxx[1])*SS(.75,.25,iTime-u_dataxx[1]),-0.2,(p*MD(0.2)).x);
    return d;
}

vec3 ch_col=vec3(1.);
vec3 ch_h_col=vec3(.15,.37,1.);
vec3 ch_sh_col=vec3(.5,1.,1.);

vec4 draw_char_ss(vec2 p, float atime){
    float d=0.;
    float anim_timer=atime; 
    vec2 tpp=vec2(0.022*sin(PI*2.*anim_timer),-0.1*timex2(anim_timer));
    p+=vec2(0.012*sin(PI*2.*anim_timer)-0.2,tpp.y/3.5+0.12);
    p=p*MD(2.-0.37*tpp.y);
    d=max(d,SS(zv-0.05+0.1*SS(0.,.15,u_dataxx.x)*SS(2.,1.75,u_dataxx.x),-zv-0.08,-0.05+sdTriangleIsosceles(vec2(0.05,-0.65),(p+vec2(-0.00,-0.23)))));
    if(p.y<0.115)
    d=max(d,SS(zv-0.05,-zv-0.08,-0.05+sdTriangleIsosceles(vec2(0.05,0.17),+vec2(-0.0,0.53)+p)));
    vec3 col=vec3(0.);
    if(d<1.){
        vec3 ch_ss_col= (0.4 + 0.35*cos(iTime*1.5+15.*p.xyx+vec3(0,1,2)));
        col=ch_ss_col.rgb/(1.-d);
    }else if(d>=1.)col=vec3(10.);
    col=min(vec3(10.),col);
    col.gb*=1.-0.85*SS(0.,.15,u_dataxx.x)*SS(2.,1.75,u_dataxx.x);
    return vec4(col,min(d,max(max(col.r,col.g),col.b)));
}

vec4 char_col_w(vec2 p,float atime){
    vec4 col=vec4(0.);
    float d=0.;
    d=draw_char(p,atime);
    col=vec4(ch_col,d);
    d=draw_char_hh(p,atime);
    
    if(d<1.){
    vec3 tc=ch_h_col.rgb/(1.-d);
    vec4 tcc=vec4(tc,d);
    col=mix(col,tcc,1.-col.a);
    col.a=clamp(col.a,0.,1.);
    }
    d=draw_char_sh(p,true); //hit anim
    if(d<1.){
    ch_sh_col = 01. + 0.5*cos(iTime*PI/1.5+vec3(0,2,4));
    vec3 tc=ch_sh_col.rgb/(1.-d);
    float ns=8.*triangleNoise(p*1.5+vec2(-iTime*0.05,iTime*0.05));
    vec4 tcc=vec4(tc,d*ns);
    col=mix(col,tcc,tcc.a);
    col.a=clamp(col.a,0.,1.);
    }
    vec4 tcc=draw_char_ss(p,atime);
    col=mix(mix(col,tcc,tcc.a),col,col.a);
    col.a=clamp(col.a,0.,1.);
    
    return col;
}

float bx_s(vec2 p,float atime){
    float d=0.;
    d=SS(zv+0.05,-zv,sdBox(p+vec2(0.3*linearstep(0.,0.2,atime)-0.3,0.),vec2(0.3*linearstep(0.,0.2,atime),0.5)));//linearstep
    return d;
}

vec4 char_col_s(vec2 p,float atime){
    vec4 col=vec4(0.);
    float d=0.;
    d=bx_s(p,atime);
    float odd=floor(d);
    
    if(d<1.){
        d*=linearstep(0.9,0.4,atime);
    vec3 tc=2.*ch_h_col.rgb*d;
    vec4 tcc=vec4(tc,d);
    col=mix(col,tcc,1.-col.a);
    col.a=clamp(col.a,0.,1.);
    }else if(d>=1.)col=vec4(vec3(0.),d*linearstep(0.9,0.4,atime));
    
    d=draw_char_s(p,atime)*odd;
    if(d>0.){
    col=vec4(ch_col,d);
    }
    d=draw_char_hh_s(p,atime)*odd;
    if(d>0.){
    vec3 tc=ch_h_col.rgb/(1.-d);
    vec4 tcc=vec4(tc,d);
    col=mix(col,tcc,1.);
    col.a=clamp(col.a,0.,1.);
    }
    d=draw_char_sh(p,true); //hit anim
    if(d<1.){
    ch_sh_col = 01. + 0.5*cos(iTime*PI/1.5+vec3(0,2,4));
    vec3 tc=ch_sh_col.rgb/(1.-d);
    float ns=3.*triangleNoise(p*1.5+vec2(-iTime*0.05,iTime*0.05));
    vec4 tcc=vec4(tc,d*ns);
    tcc*=linearstep(0.8,1.,atime);
    col=mix(col,tcc,tcc.a);
    col.a=clamp(col.a,0.,1.);
    }
    vec4 tcc=draw_char_ss(p,0.);
    tcc*=linearstep(0.8,1.,atime);
    col=mix(mix(col,tcc,tcc.a),col,col.a);
    col.a=clamp(col.a,0.,1.);
    col*=linearstep(0.,.1,atime);
    
    return col;
}

vec4 char_col_aa(vec2 p,float atime){
    vec4 col=vec4(0.);
    float d=0.;
    d=draw_char_a(p,atime);
    col=vec4(ch_col,d);
    d=draw_char_hh(p,0.);
    
    if(d<1.){
    vec3 tc=ch_h_col.rgb/(1.-d);
    vec4 tcc=vec4(tc,d);
    col=mix(col,tcc,1.-col.a);
    col.a=clamp(col.a,0.,1.);
    }
    d=draw_char_sh(p,true); //hit anim
    if(d<1.){
    ch_sh_col = 01. + 0.5*cos(iTime*PI/1.5+vec3(0,2,4));
    vec3 tc=ch_sh_col.rgb/(1.-d);
    float ns=8.*triangleNoise(p*1.5+vec2(-iTime*0.05,iTime*0.05));
    vec4 tcc=vec4(tc,d*ns);
    tcc*=linearstep(0.3,0.,atime);
    col=mix(col,tcc,tcc.a);
    col.a=clamp(col.a,0.,1.);
    }
    vec4 tcc=draw_char_ss((vec2(0.0,-0.-0.45*(linearstep(0.8, .1, atime)*linearstep(0.1, .8, atime)))+p)*
                          MD(min(.87*PI,1.*PI*linearstep(0.05, .9, atime))),0.);
    
    col=mix(mix(col,tcc,tcc.a),col,col.a);
    col.a=clamp(col.a,0.,1.);
    
    return col;
}

vec4 char_col_aa2(vec2 p,float atime){
    vec4 col=vec4(0.);
    float d=0.;
    d=draw_char_a2(p,atime);
    col=vec4(ch_col,d);
    d=draw_char_hh(p,0.);
    
    if(d<1.){
    vec3 tc=ch_h_col.rgb/(1.-d);
    vec4 tcc=vec4(tc,d);
    col=mix(col,tcc,1.-col.a);
    col.a=clamp(col.a,0.,1.);
    }
    vec4 tcc=draw_char_ss((vec2(0.0,-0.-0.45*(linearstep(0.8, .1, 1.)*linearstep(0.1, .8, 1.)))+p)*
                          MD(min(.87*PI,1.*PI*linearstep(0.05, .9, 1.))),0.);
    tcc*=SS(2.*linearstep(0.5,0.,atime),-0.01+linearstep(0.5,0.,atime),-p.x);
    col=mix(mix(col,tcc,tcc.a),col,col.a);
    col.a=clamp(col.a,0.,1.);
    
    float timer=linearstep(0.6,0.0,atime);
    float timer2=linearstep(0.2,0.6,atime);
    float timer3=linearstep(1.,1.15,atime);
    d=SS(0.017+0.05*(timer2),-0.001,abs(p.y-0.030))*SS(-0.7,-0.6,p.x-0.4)*SS(-0.2,-0.3,p.x-0.4+0.4*timer+0.4*timer3);
    if(d<1.){
     	vec3 ch_ss_col= (0.4 + 0.35*cos(1.*1.5+15.*p.xyx+vec3(0,1,2)));
	    //vec3 ch_ss_col = vec3( 0.05+max(-2.*(p.x+0.2),0.),0.3,1. );
        tcc=vec4(ch_ss_col/(1.-d),d);
        col=mix(mix(col,tcc,tcc.a),col,col.a);
        col.a=clamp(col.a,0.,1.);
    }
    d=draw_char_bow(p,atime);
    if(d<1.){
    vec3 tcy=vec3(0x25,0xe9,0xff)/float(0xff);
    vec3 tcx=vec3(0xfa,0x30,0x21)/float(0xff);
    vec3 tc=mix(tcx,tcy,linearstep(0.8,.4,atime))/(1.-d);
    vec4 tcc=vec4(tc,d);
    col=mix(col,tcc,tcc.a);
    col.a=clamp(col.a,0.,1.);
    }else if(d>=1.)col=vec4(vec3(10.),d);
    
    return col;
}



vec4 char_col_aa3(vec2 p,float atime){
    vec4 col=vec4(0.);
    col=char_col_aa2(p,1.+atime);
    vec2 v1 = vec2(0.,0.);
	vec2 v2 = vec2(-010.,0.0);
    float timer4=linearstep(1.3,1.9,atime);
    float timer=linearstep(0.,0.35,atime)-timer4;
    float timer2=linearstep(0.5,03.,atime);
    
    float r1 = 0.45*timer+0.0015*timer*sin(iTime*300.);
    float r2 = 0.4*timer+0.0015*timer*sin(iTime*300.);
    float d = sdUnevenCapsule( p-vec2(-0.65+0.4*(1.-timer),0.03), v1, v2, r1, r2 );
    d=SS(zv+0.2,-zv-0.21,d);
    d*=linearstep(1.9,1.3,atime);
    if(d<1.){
        vec3 ch_ss_col= (0.4 + 0.35*cos(1.*1.5+1.*p.xyx+vec3(0,1,2)));
        vec4 tcc=vec4(ch_ss_col/(1.-d),d);
        col=mix(mix(col,tcc,tcc.a),col,col.a);
        col.a=clamp(col.a,0.,1.);
    }else if(d>=1.)col=vec4(vec3(10.),d);
    return col;
}

vec4 char_col_aa22(vec2 p,float atime){
    vec4 col=vec4(0.);
    float d=0.;
    d=draw_char_a22(p,atime);
    col=vec4(ch_col,d);
    d=draw_char_hh(p,0.);
    
    if(d<1.){
    vec3 tc=ch_h_col.rgb/(1.-d);
    vec4 tcc=vec4(tc,d);
    col=mix(col,tcc,1.-col.a);
    col.a=clamp(col.a,0.,1.);
    }
    d=draw_char_sh(p,true); //hit anim
    if(d<1.){
    ch_sh_col = 01. + 0.5*cos(iTime*PI/1.5+vec3(0,2,4));
    vec3 tc=ch_sh_col.rgb/(1.-d);
    float ns=8.*triangleNoise(p*1.5+vec2(-iTime*0.05,iTime*0.05));
    vec4 tcc=vec4(tc,d*ns);
    tcc*=linearstep(0.8,1.,atime);
    col=mix(col,tcc,tcc.a);
    col.a=clamp(col.a,0.,1.);
    }
    vec4 tcc=draw_char_ss(p,0.);
    tcc*=linearstep(0.8,1.,atime);
    col=mix(mix(col,tcc,tcc.a),col,col.a);
    col.a=clamp(col.a,0.,1.);
    
    return col;
}

vec4 char_col_aa4(vec2 p,float atime){
    vec4 col=vec4(0.);
    col=char_col_aa22(p,atime);
    //col.ra+=0.5*char_col_w(p,1.).ra;
    return col;
}


vec4 char_col_a(vec2 p,float atime){
    vec4 col=vec4(0.);
    //atime*=9.; // 9 sec
    if(atime<=1.)col=char_col_aa(p,atime);
    else if(atime<=2.) col=char_col_aa2(p,atime-1.);
    else if(atime<=5.) col=char_col_aa3(p,atime-2.);
    else if(atime<=6.)col=char_col_aa4(p,(atime-5.)*2.);
    else col=char_col_w(p,linearstep(1.,3.,atime-6.)*linearstep(1.,3.,atime-6.));
    return col;
}

vec4 char_col(vec2 p,float atime,int id){
    p+=draw_pos;
    p*=2.5;
    p.x=-p.x;
    vec4 col=vec4(0.);
    switch (id){
        case spawn:col=char_col_s(p,atime);break;
        case walk:col=char_col_w(p,atime);break;
        case att:col=char_col_a(p,atime);break; //2 sec
    }
    return col;
}

vec4 test_char_anim(vec2 p){
    vec4 rc;
    float iTime=u_valt.x;
    int uidxf=int(u_valt.y);
    if(uidxf==att)rc=char_col(p,iTime,uidxf);
    else
    if(iTime<=1.)
    rc=char_col(p,linearstep(0.,1.,iTime)*linearstep(0.,1.,iTime),uidxf);
    else
    if(iTime<=3.)
    rc=char_col(p,linearstep(1.,3.,iTime)*linearstep(1.,3.,iTime),uidxf);
    else
    if(iTime<=6.)
    rc=char_col(p,mod(iTime,1.),uidxf);
    /*else
    if(iTime<=15.)
    rc=char_col(p,iTime-6.,att);*/
    else
    rc=char_col(p,mod(iTime,1.),uidxf);
    return rc;
}

const float Power = 5.059;
const float Dumping = 10.0;
vec3 colx=vec3(2.0, 0.525, 0.08);



vec3 hash3(vec3 p) {
	p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
			dot(p, vec3(269.5, 183.3, 246.1)),
			dot(p, vec3(113.5, 271.9, 124.6)));

	return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float noise(vec3 p) {
	vec3 i = floor(p);
	vec3 f = fract(p);

	vec3 u = f * f * (3.0 - 2.0 * f);

	float n0 = dot(hash3(i + vec3(0.0, 0.0, 0.0)), f - vec3(0.0, 0.0, 0.0));
	float n1 = dot(hash3(i + vec3(1.0, 0.0, 0.0)), f - vec3(1.0, 0.0, 0.0));
	float n2 = dot(hash3(i + vec3(0.0, 1.0, 0.0)), f - vec3(0.0, 1.0, 0.0));
	float n3 = dot(hash3(i + vec3(1.0, 1.0, 0.0)), f - vec3(1.0, 1.0, 0.0));
	float n4 = dot(hash3(i + vec3(0.0, 0.0, 1.0)), f - vec3(0.0, 0.0, 1.0));
	float n5 = dot(hash3(i + vec3(1.0, 0.0, 1.0)), f - vec3(1.0, 0.0, 1.0));
	float n6 = dot(hash3(i + vec3(0.0, 1.0, 1.0)), f - vec3(0.0, 1.0, 1.0));
	float n7 = dot(hash3(i + vec3(1.0, 1.0, 1.0)), f - vec3(1.0, 1.0, 1.0));

	float ix0 = mix(n0, n1, u.x);
	float ix1 = mix(n2, n3, u.x);
	float ix2 = mix(n4, n5, u.x);
	float ix3 = mix(n6, n7, u.x);

	float ret = mix(mix(ix0, ix1, u.y), mix(ix2, ix3, u.y), u.z) * 0.5 + 0.5;
	return ret * 2.0 - 1.0;
}

#define DOT_FREQ    .285
#define DOT_RADIUS .051

float sdf_disk(vec2 uv, float radius, vec2 center,bool x){
    if(x)
    return distance(uv, center) - radius;
    else return distance(uv.xx, center.xx) - radius;
}

float IDX=hpdata.x;
float IDX2=hpdata.y;
float IDX3=hpdata.z;
bool tvx=false;
float draw_dotgrid(vec2 uv,bool x){
    if(iTime<6.5){
        IDX=floor(7.-7.*smoothstep(5.4,5.9,iTime));
        IDX2=floor(6.-6.*smoothstep(5.6,6.2,iTime));
        IDX3=floor(6.-6.*smoothstep(5.8,6.4,iTime));
    }
    if(x&&(uv.x-DOT_FREQ/2.>DOT_FREQ*(5.+IDX))){
    vec2 uv_repeat  = mod(uv - .5 * DOT_FREQ, DOT_FREQ) - .5 * DOT_FREQ;

    float dot_aa  =0.338;
    float dot_radius = DOT_RADIUS;  
        float ret= smoothstep(dot_aa, 0., sdf_disk(uv_repeat, dot_radius, vec2(0),x));
        return ret;
    }
    else
    {uv=uv*MD(0.85);
     uv.x+=DOT_FREQ*.35;
     float IDXU=0.;
     if(tvx)IDXU=IDX2;
     else IDXU=IDX3;
        if((!x)&&(uv.x-DOT_FREQ/2.>DOT_FREQ*(0.+IDXU))&&(uv.x-DOT_FREQ/2.<DOT_FREQ*(6.))){
            
        vec2 uv_repeat  = mod(uv - .5 * DOT_FREQ, DOT_FREQ) - .5 * DOT_FREQ;

    float dot_aa  =01.38;
    float dot_radius = DOT_RADIUS;  
        float ret= smoothstep(dot_aa, 0., sdf_disk(uv_repeat, dot_radius, vec2(0),x));
        return ret;}return 0.;
    }
        return 0.;
}

float tv=0.;
float tv2=0.;
float tv3=0.;
float dotsx(vec2 uv){
    uv.x+=0.0;
    uv*=5.5;
	float dotgrid = draw_dotgrid(uv,true);
    dotgrid*=1.-step(DOT_RADIUS*3.,abs(uv.y));
    tv=dotgrid;
    float dotgrid2 = draw_dotgrid(uv*2.,false);
    dotgrid2*=smoothstep(DOT_RADIUS*8.5,DOT_RADIUS*3.,abs(uv.y));
    dotgrid2*=smoothstep(-0.1,DOT_RADIUS/2.5,abs(uv.y));
    if(tvx)tv2=dotgrid2;
    else tv3=dotgrid2;
    dotgrid=max(dotgrid,dotgrid2);
    
   
   return dotgrid;
}



float line( vec2 uv, float r1, float r2, float an,float lenx,float z,vec2 ab)
{
    vec2 ouv=uv*MD(an/2.);
    float w=0.;
    w = 2.0/iResolution.y;
    if((ouv.x)>0.){if((ouv.x)<lenx){uv=uv*MD(an); }
                   else{uv.y+=2.*lenx*sin(an/2.);}}
 
    uv*=z;
    //uv*=0.31;
    float t = r1-r2;
    float r = r1;    
    return smoothstep(ab.x-w/1.0, ab.y+w/1.0, abs((uv.y) - r) - t/8.0);    
   
}

float hpbar( vec2 uv,vec2 ab){
    
	uv.x-=0.5;
    uv.y+=-0.385;
    if(uv.y>0.)tvx=true;
    uv.y=abs(uv.y);
    float lin=1.0-line(uv,0.,-0.045,-0.65,0.052,2.,ab);
    uv.x+=0.25;
    lin=max(lin,1.0-line(uv,0.,-0.048,-0.65,0.052,2.,ab));
    float vb=smoothstep(0.,-0.071,uv.x);
    lin*=1.0-vb;
    lin=max(dotsx(uv)/1.0051,lin);
    return 1.-lin;
    
}

vec4 color_hhhp(vec2 p) {
    vec3 coord = vec3(p*19., iTime * 0.35);
	float nx = abs(noise(coord));
	nx += 0.5 * abs(noise(coord * 2.0));
	nx += 0.25 * abs(noise(coord * 4.0));
	nx += 0.125 * abs(noise(coord * 8.0));

    float vx=hpbar(p,vec2(-0.02,0.25));
	float n = max(0.5,0.5+nx);
	n *= (100.001 - Power);
	float dist = vx;
	n *= dist / pow(1.001 , Dumping);
    colx=mix(colx,vec3(.20,.825, 0.08)/nx,smoothstep(0.852,1.051,tv));
    colx=mix(colx,vec3(.820,.5825, 0.08)/nx,smoothstep(0.952,1.051,tv2));
    colx=mix(colx,vec3(.20,.25, 0.808)/nx,smoothstep(0.952,1.051,tv3));
	vec3 col = colx / n;
	col= pow(col, vec3(2.0))*(0.15-vx);
    col = max(vec3(0.),pow(col, vec3(0.545)));
    return vec4(col,(col.r+col.b+col.g)/3.);
}

vec4 mihgtngerg(vec2 fragCoord) {
	vec2 resxx = iResolution.xy / iResolution.y;
    vec2 uv = (fragCoord.xy) / iResolution.y - resxx/2.0;
    uv.x=-uv.x-.29;
    uv.y+=.705;
    uv*=1.5;
	vec4 col = color_hhhp(uv);

	return vec4(col);
}


float myround(float v)
{
	if(v - floor(v) >= .5) return floor(v)+1.0;
	else return floor(v);

}

vec2 myround(vec2 v)
{
	vec2 ret = vec2(0.0);
	if(v.x - floor(v.x) >= 0.5) ret.x = floor(v.x)+1.0;
	else ret.x = floor(v.x);
	if(v.y - floor(v.y) >= 0.5) ret.y = floor(v.y)+1.0;
	else ret.y = floor(v.y);
	return ret;
}

float triwave(float x)
{
	return 1.0-4.0*abs(0.5-fract(0.5*x + 0.25));
}

float rand(vec2 co){
	float t = mod(iTime,64.0);
    return fract(sin(dot(co.xy ,vec2(1.9898,7.233))) * t*t);
}
float rands(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float BarrelPower = 1.085;
vec2 Distort(vec2 p)
{
    float theta  = atan(p.y, p.x);
    float radius = length(p);
    radius = pow(radius, BarrelPower);
    p.x = radius * cos(theta);
    p.y = radius * sin(theta);
    return 0.5 * (p + 1.0);
}

vec3 shader( vec2 p, vec2 resolution ,float ghf) {

	vec2 position = ( p / resolution.xy ) ;
	float t = iTime/2.;
	float b = 0.0;
	float g = 0.0;
	float r = 0.0;
	
	float yorigin = 1.0 + 0.1*((0.3+0.7*ghf)*tan(sin(position.x*40.0+1.0*(fract(t / 2.0)) * 2.5 * 20.)));
	
	float dist = ( 20.0*abs(yorigin - position.y));
	
	b = (0.02 + 0.2  + 0.4)/dist;
	g = (0.02 + 0.0013*(1000.))/dist;
	r = (0.02 + .005 *(1000.))/dist + sin(position.x - 1.0 );
	return vec3( r, g, b);
//-------------------
}




vec4 migfgd( vec2 fragCoord ,float ghf)
{
    float iTime=max(iTime-16.,0.);
    vec2 position =(fragCoord.xy) / iResolution.y ;
    vec2 op=position-res_g/2.;
    position.x+=02.86;
    position.y+=0.03;
	position*=6.;
    position =position*iResolution.y+vec2(-1./80.)*4./1.135;
	vec3 color = vec3(0.0);
    float pixelsize = 8.*4.0*iResolution.y/450.;
	
	vec2 dposition = Distort(position/iResolution.y-0.5)*(iResolution.y*2.0);
	
	vec2 rposition = myround(((dposition-(pixelsize/2.0))/pixelsize));
	
	
    float tvvx=smoothstep(16.,18.,mod(iTime,36.))*smoothstep(036.,34.,mod(iTime,36.));
	color = vec3(shader(rposition,(iResolution.xy*tvvx+iResolution.yy*(1.-tvvx))/pixelsize,ghf));
	
	//color = clamp(color,0.0625,1.0);
	
	color *= (rand(rposition)*0.5+0.5);
	
	color *= abs(sin(rposition.y*8.0+(iTime*0.0))*0.25+0.75);
	
	color *= vec3(clamp( abs(triwave(dposition.x/pixelsize))*3.0 , 0.0 , 1.0 ));
	color *= vec3(clamp( abs(triwave(dposition.y/pixelsize))*3.0 , 0.0 , 1.0 ));
	
	float darkness = 1.;
	darkness*=1.-smoothstep(0.2,0.28,(op.y+0.67));
    darkness*=smoothstep(0.19,0.26,(op.y+0.69));
    darkness*=smoothstep(0.13,0.05,abs(op.x));
	color *= vec3(clamp( darkness*4.0 ,0.0 ,1.0 ));
	
	 return vec4( color, 1.0 );
}



#define DECAY		.974
#define EXPOSURE	.24

 #define SAMPLES	16
 #define DENSITY	.93
 #define WEIGHT		.536


#define fra(u) (u-.5*iResolution.xy)/iResolution.y

#define iterBayerMat 1
#define bayer2x2(a) (4-(a).x-((a).y<<1))%4

float GetBayerFromCoordLevel(vec2 pixelpos)
{ivec2 p=ivec2(pixelpos);int a=0
;for(int i=0; i<iterBayerMat; i++
){a+=bayer2x2(p>>(iterBayerMat-1-i)&1)<<(2*i);
}return float(a)/float(2<<(iterBayerMat*2-1));}

float bayer2  (vec2 a){a=floor(a);return fract(dot(a,vec2(.5, a.y*.75)));}
float bayer4  (vec2 a){return bayer2 (  .5*a)*.25    +bayer2(a);}
float bayer8  (vec2 a){return bayer4 (  .5*a)*.25    +bayer2(a);}
float bayer16 (vec2 a){return bayer4 ( .25*a)*.0625  +bayer4(a);}
#define dither2(p)   (bayer2(  p)-.375      )
#define dither4(p)   (bayer4(  p)-.46875    )
#define dither8(p)   (bayer8(  p)-.4921875  )
#define dither16(p)  (bayer16( p)-.498046875)

float iib(vec2 u){
 return dither16(u);
 return GetBayerFromCoordLevel(u*999.);
}



vec3 sp( vec2 uv ,float iTime) {	
    vec2 p=vec2(-3.+06.*fract(iTime/2.), -0.);
    vec3 resxx;
    float di = distance(uv, p);
    resxx.x =  di <= .3333 ? sqrt(1. - di*3.) : 0.;
    resxx.yz = p;
    resxx.y /= (iResolution.x / iResolution.y);
    resxx.yz = (resxx.yz+1.)*.5;
    return resxx;}


#define SSxx 2./min(iResolution.x,iResolution.y)

float pattern_bg(vec2 p){
    float d=0.;
    p=vec2(mod(p.x+0.01*(floor((mod(p.y,0.04)-0.02)/0.02)),0.02)-0.01,mod(p.y,0.02)-0.01);
    d=smoothstep(-0.001,0.001,sdBox(p,vec2(0.0035)));
    return d;
}

float mapxx(vec2 uv,float tt){
    uv*=1.25;
        uv.x=mod(uv.x,0.4)-0.2;
    
	float col = (smoothstep(-.0,0.12,abs(uv.x))* smoothstep(0.2,0.0,abs(uv.y)-0.8))+0.001;
	return col*pattern_bg(uv*0.2*MD(iTime));
}


vec4 BA(in vec2 uv ,float iTime){  
    uv=uv*2.-1.;
    float aspect = iResolution.x / iResolution.y;
   uv.x *= aspect;
    
    float occluders=mapxx(uv,iTime);
    vec3 light=min(sp(uv,iTime),1.);
    float col = max(light.x - occluders, 0.);
    return vec4(col,occluders,light.yz); 
}



vec4 mi2dfg(in vec2 fragCoord ,float tt,vec3 colx){
	vec2 uv = fragCoord.xy / iResolution.xy;
    vec2 coord = uv;
    vec4 ic=BA(uv,tt);
    vec2 lightpos = ic.zw;   	
    float occ = ic.x; 
    float obj = ic.y; 
    float dither = iib(fragCoord);
    vec2 dtc = (coord - lightpos)*(1./float(SAMPLES)*DENSITY);
    float illumdecay = 1.;
    
    for(int i=0; i<SAMPLES; i++)    {
        coord -= dtc;
        	float s = BA(coord+(dtc*dither),tt).x;
        s *= illumdecay * WEIGHT;
        occ += s;
        illumdecay *= DECAY;
    }
        
	return vec4(vec3(0.)*obj/3.+colx*occ*EXPOSURE,1.-occ);
}


float map(vec2 p) {
    p.x=mod(p.x,0.4)-0.2;
	float col = (smoothstep(-.0,0.2,abs(p.x))* smoothstep(0.2,0.0,abs(p.y)-0.8))+0.001;
	return col;
}

vec3 calcNormal(vec2 p) {
	vec2 e = vec2(0.001, 0.00);
	vec3 nor = vec3(
		map(p + e.xy) - map(p - e.xy),
		map(p + e.yx) - map(p - e.yx),
		map(p) * 0.3
	);
        return normalize(nor);
}


float lineSegToBrightness(vec2 U, vec2 P0, vec2 P1)
{
    P0 -= U; P1 -= U;
    float a = mod ( ( atan(P0.y,P0.x) - atan(P1.y,P1.x) ) / PI, 2.);  
    return min( a, 2.-a );
}


float noise2dxx(vec2 n) {
    const vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n);
    vec2 f = mix(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rands(b), rands(b + d.yx), f.x), mix(rands(b + d.xy), rands(b + d.yy), f.x), f.y);
}

vec3 ramp(float t) {
	return t <= .5 ? vec3( .4 - t * .4, .2, 1.05 ).rgg / t : vec3( .3* (1. - t) * 2., 5.2, 10.505 ) / t;
}

float fire(vec2 n) {
    return noise2dxx(n) + noise2dxx(n * 2.1) * .6 + noise2dxx(n * 5.4) * .42;
}

vec4 mivgls(in vec2 fragCoord ) {
	
    float t = iTime;
    vec2 uv = fragCoord / iResolution.y;
    uv.x+=-res_g.x/2.+.9;
    vec2 ouv=uv;
    uv.y+=0.4;
    uv.x += uv.y < .5 ? 23.0 + t * .35*0.5: -11.0 + t * .3*0.5;    
    uv.y = abs(uv.y - .5);
    uv *= 55.0;
    
    float q = fire(uv - t * .013) / 2.0;
    vec2 r = vec2(fire(uv + q / 2.0 + t - uv.x - uv.y), fire(uv + q - t));
    vec3 color = vec3(1.0 / (pow(vec3(0.5, 0.0, .1) + 1.61, vec3(4.0))));
    
    float grad = pow((r.y + r.y) * max(.25, uv.y+2.*smoothstep(.4,.8,ouv.x)) + .1, 4.0)*smoothstep(-.01,.4,ouv.x);
    color = ramp(grad);
    color /= (2.50 + max(vec3(0), color));
    color=clamp(color,vec3(0.),vec3(1.));
    return vec4(color, 1.0);
}



vec3 c1=vec3(0xf5,0x97,0x70)/float(0xff);
vec3 c2=vec3(0x47,0x97,0xeb)/float(0xff);
vec3 c3=vec3(0x42,0xe6,0x9d)/float(0xff);

vec4 ggnergne( vec2 p ) 
{
    p.x+=-01.6;
	float sx = 0.23 * (p.y*p.y*.5 - 0.87) * cos( 45.0 * p.y*SS(2.,1.,u_dataxx.x) - iTime*25.)*SS(0.,.25,u_dataxx.x)*SS(2.,1.75,u_dataxx.x);
	float dy = 9./ ( 423. * abs(p.x - sx));
	dy += 11./ (200. * max(-0.01+length(p.yx - vec2(p.y, 0.0)),0.0001));
	return vec4( (p.y + 0.2) * dy, 0.3 * dy, dy, 1. )*SS(0.,.25,u_dataxx.x)*SS(2.,1.75,u_dataxx.x);

}

vec4 mi_bgx(vec2 fragCoord )
{
    vec3 col = vec3(0.);
    vec3 c=vec3(0.8);
    float ttxx=iTime;
    float iTime=max(iTime-16.,0.);
    float tt=0.;
    float vv=mod(iTime/4.,18.);
    if(vv<2.)
    tt=mod(iTime/4.,2.);
    else if((vv>6.)&&(vv<8.))
    tt=mod(iTime/4.,2.)+2.;
    else if((vv>10.)&&(vv<12.))
    tt=mod(iTime/4.,2.)+4.;
    else if((vv>14.)&&(vv<16.))
    tt=mod(iTime/4.,2.)+6.;
    iTime=tt;
    float valx=smoothstep(03.,1.,-3.+06.*fract(iTime/2.))*smoothstep(-03.,-01.,-3.+06.*fract(iTime/2.));
    vec2 p = fragCoord.xy / iResolution.xy;
	p = 2.0 * p - 1.0;
	p.x *= iResolution.x / iResolution.y;
    p*=1.25;
    vec3 elc=vec3(lineSegToBrightness(vec2(abs(p.x),p.y)+vec2(-1.,-1.1),vec2(0.),vec2(1.,0.)))*smoothstep(0.5,1.5,ttxx);
	vec3 rd = normalize(vec3(0.0, 0.0, -1.0));
	vec3 nor = calcNormal(p);
    {
    vec3 lig = normalize(vec3(p.x, p.y, 0.0) - vec3(-1.5, 01.1, -1.0));
	float dif = clamp(dot(nor, lig), 0.0, 1.0);
	float spe = pow(clamp(dot(reflect(rd, nor), lig), 0.0, 1.0), 64.0);
	float fre = 1.0 - dot(-rd, nor);
    vec3 c=1.25*elc;
	col =  (1.-0.28*valx)*(c * dif + spe*smoothstep(0.5,1.5,ttxx) + fre * 0.2);
    }
    {
    vec3 lig = normalize(vec3(p.x, p.y, 0.0) - vec3(1.5, 01.1, -1.0));
	float dif = clamp(dot(nor, lig), 0.0, 1.0);
	float spe = pow(clamp(dot(reflect(rd, nor), lig), 0.0, 1.0), 64.0);
	float fre = 1.0 - dot(-rd, nor);
    vec3 c=1.25*elc;
	col =  max(col,(1.-0.28*valx)*(c * dif + spe*smoothstep(0.5,1.5,ttxx) + fre * 0.2));
    }
    vec3 evalx=vec3(0.);
    if(step(sdBox(p+vec2(0.,1.125),vec2(0.35,0.125)),0.)>0.5)
       evalx=(0.85+valx*0.15)*clamp(migfgd(fragCoord,valx).rgb,vec3(0.),vec3(1.))*smoothstep(0.2,1.5,ttxx);
    {
    vec3 lig = normalize(vec3(p.x, p.y, 0.0) - vec3(0., -01.1, -1.0));
	float dif = clamp(dot(nor, lig), 0.0, 1.0);
	float spe = pow(clamp(dot(reflect(rd, nor), lig), 0.0, 1.0), 48.0);
	float fre = 1.0 - dot(-rd, nor);
    vec3 c=vec3(0xd5,0x5b,0x0a)/float(0xff)*smoothstep(0.2,.7,ttxx);
	col =  max(col,(1.-sdCircle(vec2(p.x*.5,p.y)+vec2(0., 01.1),0.5))*(.6-0.5*valx)*c*c*( dif + spe*(0.65-0.5*evalx.r) + fre * 0.52));
    }
    
    if(int(u_valt.y)==att){
    vec3 lig = normalize(vec3(p.x, p.y, 0.0) - vec3(-0.9, -0.5, -.750));
	float dif = clamp(dot(nor, lig), 0.0, 1.0);
	float spe = pow(clamp(dot(reflect(rd, nor), lig), 0.0, 1.0), 128.0);
	float fre = 1.0 - dot(-rd, nor);
    vec3 c=vec3(0x1a,0x9f,0xbf)/float(0xff);
	col =  max(col,c*( dif + spe + fre * 0.2)*SS(2.,2.35,u_valt.x)*SS(4.5,2.5,u_valt.x)*(0.98+.0015*sin(ttxx*300.)));
    }
    
    {
    vec3 lig = normalize(vec3(p.x, p.y, 0.0) - vec3(-3.+06.*fract(iTime/2.), -0.0, -1.0));
	float dif = clamp(dot(nor, lig), 0.0, 1.0);
	float spe = pow(clamp(dot(reflect(rd, nor), lig), 0.0, 1.0), 128.0);
	float fre = 1.0 - dot(-rd, nor);
    c=vec3(0.8);
    if(mod(iTime,8.)<2.)
    c=.75*c1;
    else if(mod(iTime,8.)<4.)
    c=.75*c2;
    else if(mod(iTime,8.)<6.)
    c=.75*c3;
	col =  (col+c*valx * dif + spe*valx + fre * 0.2*valx);
    }
    if(valx>0.)
    {
    vec4 tccxx=mi2dfg(fragCoord,iTime,1.5*c);
        col=col+tccxx.rgb*valx;
    }
    if((abs(p.y+1.)<0.2)&&(p.x<-0.))
       col+=mivgls(fragCoord).rgb*step(p.x,-2.5+2.5*smoothstep(5.,5.5,ttxx));;
       col=col+evalx;
    
    col += (rands(p)-.5)*.057;
    if(step(sdBox(p+vec2(1.9,1.125),vec2(0.85,0.125)),0.)>0.5){
        vec4 tgh=mihgtngerg(fragCoord)*step(p.x,-2.5+2.5*smoothstep(5.,5.5,ttxx));
        col=col+tgh.rgb;
    }
    
    col+=ggnergne(p).rgb;
    
    col=clamp(col,vec3(0.),vec3(1.));
    

	return vec4(col*col, 1.0);
}

vec4 main_c(vec2 fragCoord){
    vec2 p = (fragCoord.xy) / iResolution.y - res_g/2.0;
    vec4 col=vec4(0.);
    float d=0.;
    int id=int(draw_id.x);
    switch (id){
        case background:col=mi_bgx(fragCoord);break;
        case box:col=draw_box(p);break;
        case character:col=test_char_anim(p);break;
        case debug:d=max(d,draw_debug(p));col.g=0.75;col.a=d;break;
    }
    
    return vec4(col);
}

float zoom_calc(float zv) {
    float ex = 0.0025 * ((1080. * zv) / (iResolution.y));
    return ex/2.;
}

void init_globals(vec2 fragCoord) {
    zv = zoom_calc(1.);
    res_g = res;
    draw_pos=draw_pos_g.xy;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{

    
    init_globals(fragCoord);
    
    vec4 ret_col=main_c(fragCoord);
    if(hpdata.w>=1.)ret_col.rgb=vec3(max(max(ret_col.r,ret_col.g),ret_col.b));
    fragColor = ret_col;
    
}

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

