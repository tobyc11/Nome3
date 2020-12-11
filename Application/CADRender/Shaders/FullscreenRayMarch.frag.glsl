#version 330 core
#define MAX_RAYMARCH_DEPTH 30
#define EPS 0.02
#define MAX_T 1000.0

in vec2 QuadUV;

out vec4 FragColor;

uniform vec3 CameraWorldPos;
uniform mat4 V2WMat;

// Camera
vec3 CastRay(vec2 uv, float fovY, float aspect)
{
    vec2 angles = vec2(fovY / 2, -fovY / 2);
    vec2 cuv = 2 * tan(angles);
    vec2 xy = cuv * (uv - 0.5);
    return vec3(xy.x * aspect, xy.y, -1);
}

// Scene SDF
float IntersectSDF(float distA, float distB)
{
    return max(distA, distB);
}

float UnionSDF(float distA, float distB)
{
    return min(distA, distB);
}

float DifferenceSDF(float distA, float distB)
{
    return max(distA, -distB);
}

float SphereSDF(vec3 p, vec3 origin, float radius)
{
    return length(p - origin) - radius;
}

float BoxSDF(vec3 p, vec3 b)
{
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float SceneSDF(vec3 p)
{
    return UnionSDF(SphereSDF(p, vec3(3, 1, -1), 1), BoxSDF(p - vec3(-3, 1, -1), vec3(1, 1, 1)));
}

// Ray Marching
vec3 EstimateNormal(vec3 p)
{
    return normalize(vec3(SceneSDF(vec3(p.x + EPS, p.y, p.z)) - SceneSDF(vec3(p.x - EPS, p.y, p.z)),
                          SceneSDF(vec3(p.x, p.y + EPS, p.z)) - SceneSDF(vec3(p.x, p.y - EPS, p.z)),
                          SceneSDF(vec3(p.x, p.y, p.z + EPS)) - SceneSDF(vec3(p.x, p.y, p.z - EPS))));
}

float RayMarch(vec3 origin, vec3 unitD)
{
    float t = 0;
    for (int i = 0; i < MAX_RAYMARCH_DEPTH; i++)
    {
        float radius = SceneSDF(origin + t * unitD);
        if (radius < EPS)
        {
            return t;
        }
        t += radius;
        if (t > MAX_T)
            return MAX_T;
    }
    return MAX_T;
}

void main()
{
    vec3 origin = CameraWorldPos;
    vec4 targetWorld = V2WMat * vec4(CastRay(QuadUV, 0.959931f, 1280.0f / 720.0f), 1);
    vec3 dir = targetWorld.xyz / targetWorld.w - origin;
    vec3 unitDir = normalize(dir);
    float t = RayMarch(origin, unitDir);
    vec3 normal = EstimateNormal(origin + t * unitDir);
    vec3 rgbOutput = normal;
    FragColor = vec4(rgbOutput, 1);
}
