//Got this from http://cirl.missouri.edu/gpu/glsl_lessons/glsl_geometry_shader/index.html
//Fragment shader
void main()
{
	//Yeah, yeah, yeah ... we just color the pixel
	// this example is showing off geometry shaders, not fragments!
	//Shade to blue!
	gl_FragColor = vec4(0.0,0.0,1.0,1.0);
}
