#version 330 core
out vec3 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;
uniform float sunTimeReference;

uniform sampler2D sunTex;
uniform sampler2D sunTex2;

void main()
{
//vec3 n = normalize(vertex_normal);
//vec3 lp=lightPos;
//vec3 ld = normalize(lp - vertex_pos);
//float diffuse = dot(n,ld);

// map for mercury
color = texture(sunTex, sunTimeReference*0.003*vec2(1, 1) + vec2(vertex_tex.x, 1. - vertex_tex.y)).rgb;
color += texture(sunTex, sunTimeReference*0.003*vec2(1, 1) + vec2(-vertex_tex.x*0.8, (1. - vertex_tex.y)*0.6)).rgb;
color -= texture(sunTex2, vec2(vertex_tex.x, 1. - vertex_tex.y)).rgb;
//color += texture(venusAtmos, vec2(vertex_tex.x, 1. - vertex_tex.y)).rgb;
//color *= diffuse*0.8;

//vec3 cd = normalize(vertex_pos - campos);
//vec3 h = normalize(cd+ld);
//float spec = dot(n,h);
//spec = clamp(spec,0,1);
//spec = pow(spec,40);
// specular map for earth
//color += vec3(1,1,1)*spec*texture(tex2, vec2(vertex_tex.x, 1. - vertex_tex.y)).rgb;
}
