#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "Shader.h"
#include "Texture.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"


struct ShaderProgramSource
{
	std::string VertexSource;
	std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1, VERTEX = 0, FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos)
			{
				//set mode to vertex
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				//set mode to fragment
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}
	return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	//const char* src = &source[0];
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, NULL);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	//if(result == GL_FALSE)
	if (!result)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile" <<
			(type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	//设置刷新率
	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "Error" << std::endl;

	std::cout << glGetString(GL_VERSION) << std::endl;
	{
		//float positions[] =
		//{
		//	-0.5f, -0.5f,  0.0f, 0.0f, 0.0f,
		//	 0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
		//	 0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
		//	-0.5f,  0.5f,  0.0f, 0.0f, 1.0f
		//};

		float positions[] =
		{
			100.0f, 100.0f,  0.0f, 0.0f, 0.0f,
			200.0f, 100.0f,  0.0f, 1.0f, 0.0f,
			200.0f, 200.0f,  0.0f, 1.0f, 1.0f,
			100.0f, 200.0f,  0.0f, 0.0f, 1.0f
		};

		unsigned int indices[] =
		{
			0, 1, 2,
			2, 3, 0,
		};

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		VertexArray va;

		//定义顶点缓冲区（Vertex Buffer）,第一个4指4个顶点，第二个4指每个顶点4个float
		//VertexBuffer vb(positions, 4 * 4 * sizeof(float));
		VertexBuffer vb(positions, sizeof(positions));

		VertexBufferLayout layout;
		layout.Push<float>(3);
		layout.Push<float>(2);
		va.AddBuffer(vb, layout);

		IndexBuffer ib(indices, 6);

		//glm::mat4 proj = glm::ortho(2.0f, -2.0f, -1.5f, 1.5f, -1.0f, 1.0f);
		glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);
		glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(-100, 0, 0));



#if 0
		std::string vertexShader =
			"#version 330 core\n"
			"\n"
			"layout(location = 0) in vec4 position;"
			"\n"
			"void main()\n"
			"{"
			"	gl_Position = position;\n"
			"}\n"
			;

		std::string fragmentShader =
			"#version 330 core\n"
			"\n"
			"layout(location = 0) out vec4 color;"
			"\n"
			"void main()\n"
			"{"
			"	color = vec4(1.0, 0.0, 0.0, 1.0);\n"
			"}\n"
			;
		unsigned int shader = CreateShader(vertexShader, fragmentShader);
		glUseProgram(shader);
#endif // 原始着色器编写方式

#if 1


		Shader shader("res/shaders/Basic.shader");
		shader.Bind();
		shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);

		Texture tt("res/textures/ChernoLogo.png");
		//Texture tt("res/textures/RedHeart.jpg");
		tt.Bind();
		shader.SetUniform1i("u_Texture", 0);

		va.Unbind();
		vb.Unbind();
		ib.Unbind();
		shader.Unbind();

		Renderer renderer;

		// Setup ImGui binding
		ImGui::CreateContext();
		ImGui_ImplGlfwGL3_Init(window, true);
		// Setup style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		glm::vec3 translation(200, 200, 0);


		float r = 0.f;
		float increment = 0.05f;

#endif // 解析文件的方式编写着色器

		//glViewport(0, 0, 640, 480);

		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			renderer.clear();

			ImGui_ImplGlfwGL3_NewFrame();

			glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);

			glm::mat4 mvp = proj * view * model;



			shader.Bind();
			shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);
			shader.SetUniformMat4f("u_MVP", mvp);

			renderer.Draw(va, ib, shader);

			if (r > 1.0f)
				increment = -0.05f;
			else if (r < 0.0f)
				increment = 0.05f;
			r += increment;

			{
				ImGui::SliderFloat3("Translation", &translation.y, 0.0f, 960.0f);            // Edit 1 float using a slider from 0.0f to 1.0f  
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}

			// ImGui Rendering
			ImGui::Render();
			ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());


			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
		}
	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}