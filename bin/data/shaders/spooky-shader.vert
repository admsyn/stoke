attribute float pointSize;
uniform float visibility;
uniform float colorMod;
//  gl_Color;

void main() {
    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_PointSize  = pointSize;	
	gl_FrontColor = vec4(0.1,
						 0.1 + (pointSize / 40.0) + (colorMod / 2.0),
						 0.4 + colorMod,
						 visibility);
}