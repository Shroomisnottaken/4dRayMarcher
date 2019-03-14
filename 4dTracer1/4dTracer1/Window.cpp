#include "Window.h"

//general variables
int DrawAreaHeight = 400;
int WindowHeight = 400; //zoom a bit, since 3d tracing is expensive...
float ScreenRatio = (float)1 / 1;//16 / 9
float MouseSensitivity = 0.4f;
float zSensitivity = 2.0f;
float PlayerSpeed = 10.0f;
int DrawAreaWidth = 0;
int WindowWidth = 0;
bool ReadFirstMousePos = false;
MouseData lastMousePos = MouseData(0, 0);
CameraData camDir = CameraData(0, 0, 0);
PlayerData pd = PlayerData(Vector4(0, 0, 0, 0), Vector4(0, 0, 0, 0), Vector4(0, 0, 0, 0), Vector4(0, 0, 0, 0), Vector4(0, 0, 0, 0));
bool KeysDown[] = { false, false, false, false, false, false, false }; //wasd and mouse left / right and space for jumping

//only for fps measuring
double LastTimeM = 0.0;
double LastTime = 0.0;
int NumberFrames = 0;

//the actual frame length
double DeltaTime = 0.0;

//render variables
GLuint bufferID;
GLuint shaderProgramID;

//global shader variables
GLuint playerPos;
GLuint viewFwd;
GLuint viewRight;
GLuint viewUp;
GLuint viewWup;

//textures
const int TexNumber = 2;
GLuint Textures[TexNumber];

int main()
{
	
	//set drawarea
	DrawAreaWidth = ScreenRatio * DrawAreaHeight;
	WindowWidth = ScreenRatio * WindowHeight;
	//pd.Pos = Vector3(13245, 0, 13578);

	GLFWwindow* window;

	//Initialize the library
	if (!glfwInit())
		return -1;

	//set hints
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	
	//Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(WindowWidth, WindowHeight, "4d Ray Marching Example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	
	//Make the window's context current
	glfwMakeContextCurrent(window);

	//init glew
	GLenum glewError = glewInit();
	if (glewError != 0)
		return -1;

	if (!GLEW_VERSION_4_5)
	{
		printf("%s", "Es muss mindestens OpenGL 4.5 installiert sein");
		return -1;
	}
	//turn on off vScreenSizeFctync
	glfwSwapInterval(0);

	//initialize rendering and window callback and set shader and textures
	InitGL(window);

	//hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // to lock GLFW_CURSOR_DISABLED
	
	//set lasttime for deltatime at start
	LastTime = glfwGetTime();

	//Loop until the user closes the window
	while (!glfwWindowShouldClose(window))
	{
		//compute input
		SetPlayerViewDir(camDir, pd);
		MovePlayer(pd);

		//set shader variables
		glUniform4f(playerPos, pd.Pos.x, pd.Pos.y, pd.Pos.z, pd.Pos.w);
		glUniform4f(viewFwd, pd.viewDir.x, pd.viewDir.y, pd.viewDir.z, pd.viewDir.w);
		glUniform4f(viewRight, pd.viewRight.x, pd.viewRight.y, pd.viewRight.z, pd.viewRight.w);
		glUniform4f(viewUp, pd.viewUp.x, pd.viewUp.y, pd.viewUp.z, pd.viewUp.w);
		glUniform4f(viewWup, pd.viewWup.x, pd.viewWup.y, pd.viewWup.z, pd.viewWup.w);

		//clear window
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//activate textures
		/*for (int i = 0; i < TexNumber; i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, Textures[i]);
		}*/

		//draw quad for cuda texture and draw
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(0, 0, 0);

		glTexCoord2f(0, 1);
		glVertex3f(0, DrawAreaHeight, 0);

		glTexCoord2f(1 * ScreenRatio, 1);
		glVertex3f(DrawAreaWidth, DrawAreaHeight, 0);

		glTexCoord2f(1 * ScreenRatio, 0);
		glVertex3f(DrawAreaWidth, 0, 0);
		glEnd();

		//Getfps
		GetFPS(window);

		//Swap front and back buffers
		glfwSwapBuffers(window);

		//Poll for and process events
		glfwPollEvents();
	}
	
	glfwTerminate();
	return 0;
}
void InitGL(GLFWwindow* window)
{
	//set window callbacks
	glfwSetCursorPosCallback(window, MouseCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, scroll_callback);

	//init gl
	glViewport(0, 0, DrawAreaWidth, DrawAreaHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, DrawAreaWidth, 0, DrawAreaHeight, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	//init shader
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	//the fragment shader
	char *ShaderFileName = (char*)"GLSLfragmentShader.txt";
	string FragmentShader = readShader(ShaderFileName);
	const char *FragmentShaderSource = FragmentShader.c_str();

	//compile programm
	glShaderSource(fragmentShaderID, 1, &FragmentShaderSource, NULL);
	glCompileShader(fragmentShaderID);

	//Check for errors and create log
	GLint fCompileResult;
	GLint fErrorLogLength;
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &fCompileResult);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &fErrorLogLength);

	//create the compile error log
	GLchar *fErrorLog = new GLchar[fErrorLogLength];
	glGetShaderInfoLog(fragmentShaderID, fErrorLogLength, &fErrorLogLength, fErrorLog);

	//create gpu programm
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, fragmentShaderID);
	glLinkProgram(shaderProgramID);
	glDetachShader(shaderProgramID, fragmentShaderID);
	glDeleteShader(fragmentShaderID);
	glUseProgram(shaderProgramID);

	//get variables 
	playerPos = glGetUniformLocation(shaderProgramID, "playerPos");
	viewFwd = glGetUniformLocation(shaderProgramID, "playerFwd");
	viewRight = glGetUniformLocation(shaderProgramID, "playerRight");
	viewUp = glGetUniformLocation(shaderProgramID, "playerUp");
	viewWup = glGetUniformLocation(shaderProgramID, "playerWup");

	//set textures
	//SetTexture(terrainHFile, "TerrainHeigth", GL_REPEAT, GL_LINEAR, 0, GL_RED, GL_R8);
	//SetTexture(terrainNFile, "TerrainNormal", GL_REPEAT, GL_LINEAR, 1, GL_RGBA, GL_RGBA8);
}

void MouseCallback(GLFWwindow *window, double xpos, double ypos)
{
	//void needs to run once to become active
	if (ReadFirstMousePos == true)
	{
		//get the diff to last mousepos
		float xDiff = xpos - lastMousePos.xPos;
		float yDiff = ypos - lastMousePos.yPos;

		//set the cameradata values according to the frame speed
		camDir.xAng += xDiff * MouseSensitivity;
		camDir.yAng += yDiff * MouseSensitivity;

		//make coords overlap and set borders
		if (camDir.xAng >= 360) camDir.xAng -= 360;
		if (camDir.xAng < 0) camDir.xAng += 360;

		if (camDir.yAng >= 360) camDir.yAng -= 360;
		if (camDir.yAng < 0) camDir.yAng += 360;
	}
	else ReadFirstMousePos = true;

	//set lastmousepos
	lastMousePos = MouseData(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//set z ang
	camDir.zAng -= yoffset * zSensitivity;
	if (camDir.zAng >= 360) camDir.zAng -= 360;
	if (camDir.zAng < 0) camDir.zAng += 360;
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		//close
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//get the keys Down (wasd)
	if (key == GLFW_KEY_W && (action == GLFW_PRESS))
	{
		//y forward
		KeysDown[0] = true;
	}
	if (key == GLFW_KEY_S && (action == GLFW_PRESS))
	{
		//y back
		KeysDown[1] = true;
	}
	if (key == GLFW_KEY_A && (action == GLFW_PRESS))
	{
		//x left
		KeysDown[2] = true;
	}
	if (key == GLFW_KEY_D && (action == GLFW_PRESS))
	{
		//x right
		KeysDown[3] = true;
	}
	
	//get the keys up (wasd)
	if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		//y forward
		KeysDown[0] = false;
	}
	if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		//y back
		KeysDown[1] = false;
	}
	if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		//x left
		KeysDown[2] = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		//x right
		KeysDown[3] = false;
	}
}
void GetFPS(GLFWwindow *window)
{
	// Measure speed
	double CurrentTime = glfwGetTime();
	DeltaTime = CurrentTime - LastTime;
	LastTime = CurrentTime;
	NumberFrames++;

	//check for display every second
	if (CurrentTime - LastTimeM >= 1.0){ // If last frame measure was more than 1 sec ago
		//printf("%d", NumberFrames);
		NumberFrames = 0;
		LastTimeM += 1.0;
	}
}
string readShader(char *filePath) 
{
	string content;
	ifstream fileStream(filePath, ios::in);

	if (!fileStream.is_open()) {
		cerr << "Could not read file " << filePath << ". File does not exist." << endl;
		return "";
	}

	string line = "";
	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}
int ReadAllBytes(char const* FileName, vector<char>& File) //0 = no error, 1 = error (file does not exist)
{
	ifstream ifs(FileName, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();
	if ((int)ifs.tellg() == -1) return -1;

	File.resize(pos);

	ifs.seekg(0, ios::beg);
	ifs.read(&File[0], pos);

	return 0;
}
int WriteAllBytes(char const* FileName, vector<char>& File) // creates new file or ovverides
{
	ofstream ofs(FileName, ios::binary);
	ofstream::pos_type pos = File.size();

	ofs.seekp(0, ios::beg);
	ofs.write(&File[0], pos);

	return 0;
}
int LoadTexFromFile(char const* FileName, Texture& Tex)
{
	vector<char> Data;
	int Error = ReadAllBytes(FileName, Data);
	if (Error == -1) return - 1;
	short Width = (Data[1] << 8 | Data[0]);
	short Heigth = (Data[3] << 8 | Data[2]);
	Data.erase(Data.begin(), Data.begin() + 4);
	Tex.Width = Width;
	Tex.Heigth = Heigth;
	Tex.Data = new unsigned char[Data.size()];
	memcpy(Tex.Data, &Data[0], Data.size() * sizeof(char)); // the byte size
}
void SetTexture(const char* FileName, const char *TexName, GLint WarpMode, GLint InterplMode, int TextureChannel, GLenum ColorFormat, GLint BitFormat)
{
	Texture dTex = Texture(0, 0);
	int Error = LoadTexFromFile(FileName, dTex);

	glGenTextures(1, &Textures[TextureChannel]);
	glBindTexture(GL_TEXTURE_2D, Textures[TextureChannel]);

	//set texture params
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, InterplMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, InterplMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WarpMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WarpMode);
	
	glTexImage2D(GL_TEXTURE_2D, 0, BitFormat, dTex.Width, dTex.Heigth, 0, ColorFormat, GL_UNSIGNED_BYTE, dTex.Data);
	GLuint TexLoc = glGetUniformLocation(shaderProgramID, TexName);
	glUniform1i(TexLoc, TextureChannel);
}
void SetPlayerViewDir(CameraData camData, PlayerData& playerData)
{
	//ang control
	//x to y by xang
	//x,y to z by zang
	//x,y,z to w by yang

	float a1 = camData.xAng;
	float a2 = camData.yAng + 90;
	float a3 = camData.zAng + 90;

	//Vector3 Fwd = Normalize(Vector3(sin(DegToRad(camData.yAng)) * cos(DegToRad(camData.xAng)), sin(DegToRad(camData.yAng)) * sin(DegToRad(camData.xAng)), cos(DegToRad(camData.yAng))));
	Vector4 Fwd = Normalize(Vector4(cos(DegToRad(a1)) * sin(DegToRad(a2)) * sin(DegToRad(a3)), sin(DegToRad(a1)) * sin(DegToRad(a2)) * sin(DegToRad(a3)), cos(DegToRad(a2)) * sin(DegToRad(a3)), cos(DegToRad(a3))));

	Vector4 Right = Normalize(Vector4(cos(DegToRad(a1 + 90)), sin(DegToRad(a1 + 90)), 0, 0));

	Vector4 Up = Normalize(Vector4(cos(DegToRad(a1)) * sin(DegToRad(a2 - 90)), sin(DegToRad(a1)) * sin(DegToRad(a2 - 90)), cos(DegToRad(a2 - 90)), 0));
	
	//is there a better name for up un dim4 ???
	Vector4 Wup = Normalize(Vector4(cos(DegToRad(a1)) * sin(DegToRad(a2)) * sin(DegToRad(a3 - 90)), sin(DegToRad(a1)) * sin(DegToRad(a2)) * sin(DegToRad(a3 - 90)), cos(DegToRad(a2)) * sin(DegToRad(a3 - 90)), cos(DegToRad(a3 - 90))));

	//this needs to be 0, since the camera must be orthogonal in projection plane
	//printf("%f", Dot(Fwd, Right) + Dot(Fwd, Up) + Dot(Right, Up) + Dot(Fwd, Wup) + Dot(Right, Wup), Dot(Up, Wup));

	playerData.viewDir = Fwd;
	playerData.viewRight = Right;
	playerData.viewUp = Up;
	playerData.viewWup = Wup;
}
void MovePlayer(PlayerData& playerData)
{
	if (KeysDown[0] == true)
	{
		//forward
		pd.Pos = pd.Pos + playerData.viewDir * DeltaTime * PlayerSpeed; //Vector3(-(*pd).ViewRight.y, (*pd).ViewRight.x, 0) * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[1] == true)
	{
		//back
		pd.Pos = pd.Pos - playerData.viewDir * DeltaTime * PlayerSpeed;//(*PlayerPos) = (*PlayerPos) - Vector3(-(*pd).ViewRight.y, (*pd).ViewRight.x, 0) * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[2] == true)
	{
		//left
		pd.Pos = pd.Pos - playerData.viewRight * DeltaTime * PlayerSpeed;
	}
	if (KeysDown[3] == true)
	{
		//right
		pd.Pos = pd.Pos + playerData.viewRight * DeltaTime * PlayerSpeed;
	}

	//add up and wup direction????
	//qe and rf

}