//Got this from http://cirl.missouri.edu/gpu/glsl_lessons/glsl_geometry_shader/index.html
//Vertex shader
void main()
{
	//Transform the vertex (ModelViewProj matrix)
	gl_Position = ftransform();
}
