#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UBOColor {
	vec4 uniformColor; 
};

void main() {
    outColor = uniformColor;
}
