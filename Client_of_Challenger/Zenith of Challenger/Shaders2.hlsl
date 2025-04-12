cbuffer cbGameObjectInfo : register(b0)
{
	matrix		gmtxGameObject : packoffset(c0);
};

cbuffer cbCameraInfo : register(b1)
{
	matrix		gmtxView : packoffset(c0);
	matrix		gmtxProjection : packoffset(c4);
};

cbuffer cbFrameworkInfo : register(b2)
{
	float 		gfCurrentTime;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct VS_DIFFUSED_INPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VS_DIFFUSED_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

VS_DIFFUSED_OUTPUT VSPlayer(VS_DIFFUSED_INPUT input)
{
	VS_DIFFUSED_OUTPUT output;

	output.position = mul(mul(mul(float4(input.position, 1.0f), gmtxGameObject), gmtxView), gmtxProjection);
	output.color = input.color;

	return(output);
}

float4 PSPlayer(VS_DIFFUSED_OUTPUT input) : SV_TARGET
{
	return(input.color);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
Texture2D gtxtTexture : register(t0);
SamplerState gSamplerState : register(s0);

struct VS_TEXTURED_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_TEXTURED_OUTPUT VSTextureToScreen(uint nVertexID : SV_VertexID)
{
	VS_TEXTURED_OUTPUT output;
	if (nVertexID == 0) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 1) { output.position = float4(+1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 0.0f); }
	if (nVertexID == 2) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 3) { output.position = float4(-1.0f, +1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 0.0f); }
	if (nVertexID == 4) { output.position = float4(+1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(1.0f, 1.0f); }
	if (nVertexID == 5) { output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f); output.uv = float2(0.0f, 1.0f); }

	return(output);
}

#define _WITH_DEFORMED_TUNNEL

/*/
float4 PSTunnelEffect(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
	float u = input.uv.x - 0.5f;
	float v = input.uv.y - 0.5f;

#ifdef _WITH_DEFORMED_TUNNEL
    float a = atan(v / u); //Polar coordinates
    float b = atan(v / abs(u));
    float r = length(float2(u, v));
    
    float2 uvL = float2((1.0f / r) + gfCurrentTime, (a / 3.1415927f));
    float2 uvR = float2((1.0f / r) + gfCurrentTime, (b / 3.1415927f));

    float4 cColor = gtxtTexture.SampleGrad(gSamplerState, uvL, ddx(uvR), ddy(uvL));
#else
    float a = atan(v / u); //Polar coordinates
    float r = length(float2(u, v));
    float2 uv = float2((1.0f / r) + gfCurrentTime, a / 3.1415927f);

	float4 cColor = gtxtTexture.Sample(gSamplerState, uv);
#endif
	return(cColor);
}
*/

//#define _WITH_CYLINDRICAL_TUNNEL
//#define _WITH_NAIVE_IMPLEMENTATION

//*/
float4 PSTunnelEffect(VS_TEXTURED_OUTPUT input) : SV_TARGET
{
#ifdef _WITH_TEXTURE_COORD
	float2 p = input.uv.xy - float2(0.5f, 0.5f);
#else
	float2 p = ((2.0f * input.position.xy) - float2(640.0f, 480.0f)) / 480.0f; //y: (-1, +1)

//	float2 p = (input.position.xy - 0.5f * float2(640.0f, 480.0f)) / 480.0f;
#endif
	float a = atan(p.y / p.x);

#ifdef _WITH_CYLINDRICAL_TUNNEL
    float r = length(p);
#else
    float r = pow(pow(p.x * p.x, 4.0f) + pow(p.y * p.y, 4.0f), 1.0f / 8.0f);
#endif
//    float2 uv = float2((0.5f / r) + (0.2f * gfCurrentTime), (a / 3.1415927f));
    float2 uv = float2((0.3f / r) + (0.2f * gfCurrentTime), (a / 3.1415927f));

#ifdef _WITH_NAIVE_IMPLEMENTATION
	float4 cColor = gtxtTexture.Sample(gSamplerState, uv);
#else
    float2 uv2 = float2(uv.x, atan(p.y / abs(p.x)) / 3.1415927f);
    float4 cColor = gtxtTexture.SampleGrad(gSamplerState, uv, ddx(uv2), ddy(uv2));
#endif
    
    cColor = cColor * r;
    
	return(cColor);
}
//*/

/*
vec3 hash( vec3 p ) // replace this by something better. really. do
{
	p = vec3( dot(p,vec3(127.1,311.7, 74.7)),
			  dot(p,vec3(269.5,183.3,246.1)),
			  dot(p,vec3(113.5,271.9,124.6)));

	return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}

// return value noise (in x) and its derivatives (in yzw)
vec4 noised( in vec3 x )
{
	// grid
	vec3 p = floor(x);
	vec3 w = fract(x);

	#if 1
	// quintic interpolant
	vec3 u = w*w*w*(w*(w*6.0-15.0)+10.0);
	vec3 du = 30.0*w*w*(w*(w-2.0)+1.0);
	#else
	// cubic interpolant
	vec3 u = w*w*(3.0-2.0*w);
	vec3 du = 6.0*w*(1.0-w);
	#endif

	// gradients
	vec3 ga = hash( p+vec3(0.0,0.0,0.0) );
	vec3 gb = hash( p+vec3(1.0,0.0,0.0) );
	vec3 gc = hash( p+vec3(0.0,1.0,0.0) );
	vec3 gd = hash( p+vec3(1.0,1.0,0.0) );
	vec3 ge = hash( p+vec3(0.0,0.0,1.0) );
	vec3 gf = hash( p+vec3(1.0,0.0,1.0) );
	vec3 gg = hash( p+vec3(0.0,1.0,1.0) );
	vec3 gh = hash( p+vec3(1.0,1.0,1.0) );

	// projections
	float va = dot( ga, w-vec3(0.0,0.0,0.0) );
	float vb = dot( gb, w-vec3(1.0,0.0,0.0) );
	float vc = dot( gc, w-vec3(0.0,1.0,0.0) );
	float vd = dot( gd, w-vec3(1.0,1.0,0.0) );
	float ve = dot( ge, w-vec3(0.0,0.0,1.0) );
	float vf = dot( gf, w-vec3(1.0,0.0,1.0) );
	float vg = dot( gg, w-vec3(0.0,1.0,1.0) );
	float vh = dot( gh, w-vec3(1.0,1.0,1.0) );

	// interpolations
	return vec4( va + u.x*(vb-va) + u.y*(vc-va) + u.z*(ve-va) + u.x*u.y*(va-vb-vc+vd) + u.y*u.z*(va-vc-ve+vg) + u.z*u.x*(va-vb-ve+vf) + (-va+vb+vc-vd+ve-vf-vg+vh)*u.x*u.y*u.z,    // value
				 ga + u.x*(gb-ga) + u.y*(gc-ga) + u.z*(ge-ga) + u.x*u.y*(ga-gb-gc+gd) + u.y*u.z*(ga-gc-ge+gg) + u.z*u.x*(ga-gb-ge+gf) + (-ga+gb+gc-gd+ge-gf-gg+gh)*u.x*u.y*u.z +   // derivatives
				 du * (vec3(vb,vc,ve) - va + u.yzx*vec3(va-vb-vc+vd,va-vc-ve+vg,va-vb-ve+vf) + u.zxy*vec3(va-vb-ve+vf,va-vb-vc+vd,va-vc-ve+vg) + u.yzx*u.zxy*(-va+vb+vc-vd+ve-vf-vg+vh) ));
}

//===============================================================================================
//===============================================================================================
//===============================================================================================
//===============================================================================================
//===============================================================================================

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	vec2 p = (-iResolution.xy + 2.0*fragCoord) / iResolution.y;

	 // camera movement
	float an = 0.5*iTime;
	vec3 ro = vec3( 2.5*cos(an), 1.0, 2.5*sin(an) );
	vec3 ta = vec3( 0.0, 1.0, 0.0 );
	// camera matrix
	vec3 ww = normalize( ta - ro );
	vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
	vec3 vv = normalize( cross(uu,ww));
	// create view ray
	vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

	// sphere center
	vec3 sc = vec3(0.0,1.0,0.0);

	// raytrace
	float tmin = 10000.0;
	vec3  nor = vec3(0.0);
	float occ = 1.0;
	vec3  pos = vec3(0.0);

	// raytrace-plane
	float h = (0.0-ro.y)/rd.y;
	if( h>0.0 )
	{
		tmin = h;
		nor = vec3(0.0,1.0,0.0);
		pos = ro + h*rd;
		vec3 di = sc - pos;
		float l = length(di);
		occ = 1.0 - dot(nor,di/l)*1.0*1.0/(l*l);
	}

	// raytrace-sphere
	vec3  ce = ro - sc;
	float b = dot( rd, ce );
	float c = dot( ce, ce ) - 1.0;
	h = b*b - c;
	if( h>0.0 )
	{
		h = -b - sqrt(h);
		if( h<tmin )
		{
			tmin=h;
			nor = normalize(ro+h*rd-sc);
			occ = 0.5 + 0.5*nor.y;
		}
	}

	// shading/lighting
	vec3 col = vec3(0.9);
	if( tmin<100.0 )
	{
		pos = ro + tmin*rd;

		vec4 n = noised( 12.0*pos );
		col = 0.5 + 0.5*((p.x>0.0)?n.yzw:n.xxx);

		col = mix( col, vec3(0.9), 1.0-exp( -0.003*tmin*tmin ) );
	}


	fragColor = vec4( col, 1.0 );
}
*/