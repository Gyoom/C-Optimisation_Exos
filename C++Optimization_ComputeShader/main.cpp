#include <iostream>
#include <glew.h>
#include <freeglut.h>
#include <chrono>
#include <vector>

const char* blur1D_source = R"(
#version 430

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer BufferA { int A[]; };

//shared int tmp[1024];

void main()
{
	uint i = gl_GlobalInvocationID.x;

	int tmp = 0;
	for(int j=-16;j<=16;++j)
    	tmp += A[i+j];
	tmp /= 33;
	barrier();
	A[i] = tmp;
}
)";

GLuint createComputeShader(const char* source) {
	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char log[512];
		glGetShaderInfoLog(shader, 512, nullptr, log);
		std::cerr << "Compute Shader Compilation Failed:\n" << log << std::endl;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, shader);
	glLinkProgram(program);
	glDeleteShader(shader);

	return program;
}

const int GROUP_SIZE = 1024;//32 * 32; //A REPORTER DANS LE CSHADER pour local_size_x
const int DATA_SIZE = 256 * 400000; //102 400 000
double minimum2 = 9999;
bool printed_once = false;
void display()
{
	using namespace std;

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);

	std::vector<int> A(DATA_SIZE, 1);

	GLuint buffers[1];
	glGenBuffers(1, buffers);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, DATA_SIZE * sizeof(int), A.data(), GL_DYNAMIC_DRAW);

	GLuint program = createComputeShader(blur1D_source);

	glUseProgram(program);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers[0]);

	chrono::high_resolution_clock::time_point start_chrono = chrono::high_resolution_clock::now();

	glDispatchCompute(DATA_SIZE / GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 1 * sizeof(int), A.data());
	//sus ^ de récup 1 seul int après le blur

	auto end_chrono = chrono::high_resolution_clock::now();

	double time_taken = chrono::duration_cast<chrono::duration<double>>(end_chrono - start_chrono).count();
	minimum2 = time_taken < minimum2 ? ((std::cout << (time_taken * 1000.0) << "ms, new minimum" << std::endl), time_taken) : minimum2;

	if (!printed_once)
		std::cout << (printed_once = true, A[1]) << endl;

	glDeleteProgram(program);
	glDeleteBuffers(1, buffers);

	glutSwapBuffers();
	//glFinish(); //glFlush();
}

int main(int argc, char** argv)
{
	using namespace std;

	//plein de boilerplate relou à copy-paste d'un projet à l'autre
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); //mode de la fenetre: double buffering & couleurs RGB (ça aurait pu être RGBA, monochrome, etc)
	glutInitWindowSize(800, 600); //taille de la fenêtre
	glutCreateWindow("Compute Shader Example"); //titre de la fenêtre
	glewInit(); //lancer le moteur d'extensions d'opengl
	//

	cout << "bjr 0w0" << endl;

	//on distribue notre équipage: quelle fonction prend quel poste
	//qui prend les flingues, les comms, le radar, le pilotage, etc
	glutDisplayFunc(display); //ici notre fonction display prend le poste Display, cf la doc pour tous les postes/events
	//on aurait pu appeler notre fonction bob_le_cameleon
	//on aurait call glutDisplayFunc(bob_le_cameleon), ct bon aussi
	//display() c'est une fonction qu'on a écrite plus haut ^, elle est à nous on l'écrit nous-mêmes
	//le type de retour et les params sont imposés selon le poste auquel on met la fonction, cf la doc
	//normalement c'est 1 func par poste, j'ai jamais try en mettre 2
	glutIdleFunc(display);

	glutMainLoop(); //ici on fait go, on lance la moulinette qui distribue les events à chaque poste

	int a;
	cin >> a;
	return 0;
}