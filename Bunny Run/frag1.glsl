#version 120

uniform float offset;

varying vec4 fragPos;

vec3 color(){
	//bool x = (int) ((fragPos.x + offset) * scale) % 2;

	bool x = bool(int(mod(((fragPos.x + 0) * 0.5),2)));
	bool y = bool(int(mod(((fragPos.y + 0) * 0.5),2)));
	bool z = bool(int(mod(((fragPos.z - offset) * 0.5),2)));

	bool xorXY = x != y;
	if (xorXY != z)
		return vec3(255, 255, 255);
	else
		return vec3(0, 0, 0);
}

void main(void)
{
	vec3 myColor = color();
    gl_FragColor = vec4(myColor,1); // interpolated color computed by the rasterizer
}