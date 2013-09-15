attribute float pointSize;

void main() {
    gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_PointSize  = pointSize;
    gl_FrontColor = gl_Color;
}