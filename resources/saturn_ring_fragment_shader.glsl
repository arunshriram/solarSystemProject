#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform float planet;
uniform vec3 campos;
uniform vec3 lightPos;

uniform sampler2D satRingTex;

void main()
{

// map for saturn ring textures
vec3 n = normalize(vertex_normal);
vec3 ld = normalize(vertex_pos - lightPos);
float diffuse = dot(n,ld);

color = texture(satRingTex, vertex_tex);

if(planet == 0)
{
    vec3 cd = normalize(vertex_pos - campos);
    vec3 h = normalize(cd + ld);
    float spec = dot(n, h);
    spec = clamp(spec, 0, 1);
    spec = pow(spec, 20);

    color += vec4(1, 1, 1, 1)*spec;
}

else
{
    float avg = (color.r + color.g + color.b)/3.;
    if((color.r < 0.75) &&
    (color.g < 0.75) &&
    (color.b < 0.75) && avg > 0.4)
    {
        color = vec4(color.r + 0.15, color.g + 0.15, color.b + 0.15, 0.7);
    }
    if(color.r < 0.8 || color.g < 0.8)
    {
        color = vec4(0, 0, 0, 0);
    }
    
    color = vec4(color.r, color.g, color.b, color.a - 0.6);
    
}

}
