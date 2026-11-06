#include "Renderer.h"

#include "Model.h"

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace
{
	// Four two-pixel-wide rectangles form a crosshair with a small center gap.
	constexpr std::array<float, 48> CrosshairVertices = {
		-11.0f, -1.0f,  -3.0f, -1.0f,  -3.0f,  1.0f,
		 -3.0f,  1.0f, -11.0f,  1.0f, -11.0f, -1.0f,

		  3.0f, -1.0f,  11.0f, -1.0f,  11.0f,  1.0f,
		 11.0f,  1.0f,   3.0f,  1.0f,   3.0f, -1.0f,

		 -1.0f, -11.0f,  1.0f, -11.0f,  1.0f, -3.0f,
		  1.0f,  -3.0f, -1.0f,  -3.0f, -1.0f, -11.0f,

		 -1.0f,  3.0f,   1.0f,  3.0f,   1.0f, 11.0f,
		  1.0f, 11.0f,  -1.0f, 11.0f,  -1.0f,  3.0f,
	};

	unsigned int compileShader(unsigned int shaderType, const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary);
		if (!file)
		{
			std::cerr << "Failed to open shader file: " << path << '\n';
			return 0;
		}

		std::ostringstream sourceStream;
		sourceStream << file.rdbuf();
		if (!sourceStream || file.bad())
		{
			std::cerr << "Failed to read shader file (unreadable or empty): " << path << '\n';
			return 0;
		}

		const std::string source = sourceStream.str();
		const char* sourceData = source.c_str();

		const unsigned int shader = glCreateShader(shaderType);
		glShaderSource(shader, 1, &sourceData, nullptr);
		glCompileShader(shader);

		int success = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			std::array<char, 1024> infoLog{};
			glGetShaderInfoLog(shader, static_cast<int>(infoLog.size()), nullptr, infoLog.data());
			std::cerr << "Shader compilation failed: " << path << '\n' << infoLog.data() << '\n';
			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	unsigned int createShaderProgram(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath)
	{
		// Vertex and fragment shaders are compiled independently, then linked into
		// one program that represents the programmable part of our pipeline.
		const unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexPath);
		if (vertexShader == 0)
		{
			return 0;
		}

		const unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentPath);
		if (fragmentShader == 0)
		{
			glDeleteShader(vertexShader);
			return 0;
		}

		const unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		// Linking copies the compiled shader code into the program, so the separate
		// shader objects are no longer needed after this point.
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		int success = 0;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			std::array<char, 1024> infoLog{};
			glGetProgramInfoLog(shaderProgram, static_cast<int>(infoLog.size()), nullptr, infoLog.data());
			std::cerr << "Shader linking failed: " << vertexPath << " and " << fragmentPath
				<< '\n' << infoLog.data() << '\n';
			glDeleteProgram(shaderProgram);
			return 0;
		}

		return shaderProgram;
	}
}

Renderer::~Renderer()
{
	cleanup();
}

bool Renderer::initialize()
{
	// Keep the fragment closest to the camera when triangles overlap. Without
	// a depth buffer test, later triangles would always paint over earlier ones.
	glEnable(GL_DEPTH_TEST);
	return createResources();
}

bool Renderer::createResources()
{
	const std::filesystem::path shaderDirectory =
		std::filesystem::path(SIMPLE_ENGINE_ASSET_DIR) / "shaders";

	// These objects live on the GPU and only need to be created once. Each frame
	// can then reuse the same shader program and cube buffers.
	m_shaderProgram = createShaderProgram(shaderDirectory / "mesh.vert", shaderDirectory / "mesh.frag");
	if (m_shaderProgram == 0)
	{
		return false;
	}

	m_crosshairShaderProgram = createShaderProgram(shaderDirectory / "crosshair.vert", shaderDirectory / "crosshair.frag");
	if (m_crosshairShaderProgram == 0)
	{
		return false;
	}

	m_lineShaderProgram = createShaderProgram(shaderDirectory / "line.vert", shaderDirectory / "line.frag");
	if (m_lineShaderProgram == 0)
	{
		return false;
	}

	glGenVertexArrays(1, &m_crosshairVertexArray);
	glGenBuffers(1, &m_crosshairVertexBuffer);

	glBindVertexArray(m_crosshairVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_crosshairVertexBuffer);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(CrosshairVertices.size() * sizeof(float)),
		CrosshairVertices.data(),
		GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &m_lineVertexArray);
	glGenBuffers(1, &m_lineVertexBuffer);

	glBindVertexArray(m_lineVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	m_cube = Mesh::createCube();
	return m_cube.isValid()
		&& m_crosshairVertexArray != 0
		&& m_crosshairVertexBuffer != 0
		&& m_lineVertexArray != 0
		&& m_lineVertexBuffer != 0;
}

void Renderer::beginFrame(int framebufferWidth, int framebufferHeight, const glm::mat4& view)
{
	// Clear both last frame's color and its depth information before drawing the
	// new frame. The clear color becomes the window background.
	glClearColor(0.08f, 0.10f, 0.12f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// The projection needs the framebuffer's aspect ratio so the cube does not
	// stretch when the window changes shape. Height can be zero while minimized.
	const float aspectRatio = framebufferHeight > 0 ? static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight) : 1.0f;

	// Projection adds perspective: distant geometry appears smaller. The last
	// two values are the near and far visible clipping distances.
	const glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

	// Select the shader program, then upload this frame's transforms to its
	// uniform variables. GL_FALSE tells OpenGL not to transpose GLM's matrices.
	glUseProgram(m_shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));

	// The direction points from each surface toward the light. Slightly cool
	// ambient light keeps shadowed faces readable while the warm key light
	// gives the model enough contrast to reveal its shape.
	const glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.4f, 1.0f, 0.6f));
	const glm::vec3 lightColor(0.95f, 0.90f, 0.80f);
	const glm::vec3 ambientColor(0.16f, 0.19f, 0.24f);
	glUniform3fv(glGetUniformLocation(m_shaderProgram, "uLightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(m_shaderProgram, "uLightColor"), 1, glm::value_ptr(lightColor));
	glUniform3fv(glGetUniformLocation(m_shaderProgram, "uAmbientColor"), 1, glm::value_ptr(ambientColor));

	// Lines share the same camera matrices as meshes, but use a separate
	// shader because they have a single fading color instead of vertex colors.
	glUseProgram(m_lineShaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(m_lineShaderProgram, "uView"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(m_lineShaderProgram, "uProjection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void Renderer::drawCube(const glm::mat4& model)
{
	drawMesh(m_cube, model, glm::vec3(0.25f, 0.65f, 0.9f));
}

void Renderer::drawMesh(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color)
{
	// The model matrix changes for each object, while the view and projection
	// set by beginFrame are shared by every object in the frame.
	glUseProgram(m_shaderProgram);
	glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uModel"), 1, GL_FALSE, glm::value_ptr(model));
	const glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(model));
	glUniformMatrix3fv(glGetUniformLocation(m_shaderProgram, "uNormalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	glUniform3fv(glGetUniformLocation(m_shaderProgram, "uColor"), 1, glm::value_ptr(color));

	// Mesh::draw binds the mesh's vertex layout and asks OpenGL to draw its
	// indexed triangles with glDrawElements.
	mesh.draw();
}

void Renderer::drawModel(const Model& model, const glm::mat4& transform, const glm::vec3& color)
{
	for (const ModelPart& part : model.getParts())
	{
		drawMesh(part.mesh, transform * part.localTransform, color);
	}
}

void Renderer::drawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color)
{
	const std::array<float, 6> vertices = {
		start.x, start.y, start.z,
		end.x, end.y, end.z,
	};

	glBindBuffer(GL_ARRAY_BUFFER, m_lineVertexBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertices.size() * sizeof(float)), vertices.data());

	glUseProgram(m_lineShaderProgram);
	glUniform4fv(glGetUniformLocation(m_lineShaderProgram, "uColor"), 1, glm::value_ptr(color));

	// Alpha blending makes the line fade as Game reduces its remaining life.
	// Keep depth testing so the tracer still belongs to the 3D scene.
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	glLineWidth(2.0f);

	glBindVertexArray(m_lineVertexArray);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);

	glLineWidth(1.0f);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
}

void Renderer::drawCrosshair(int framebufferWidth, int framebufferHeight)
{
	if (framebufferWidth <= 0 || framebufferHeight <= 0)
	{
		return;
	}

	// The crosshair is a screen-space overlay, so it should not be hidden by
	// depth values written while drawing the 3D scene.
	glDisable(GL_DEPTH_TEST);
	glUseProgram(m_crosshairShaderProgram);
	glUniform2f(
		glGetUniformLocation(m_crosshairShaderProgram, "uFramebufferSize"),
		static_cast<float>(framebufferWidth),
		static_cast<float>(framebufferHeight));

	glBindVertexArray(m_crosshairVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, 24);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
}

void Renderer::cleanup()
{
	// GPU resources must be released while the OpenGL context still exists.
	m_cube.cleanup();

	if (m_crosshairVertexBuffer != 0)
	{
		glDeleteBuffers(1, &m_crosshairVertexBuffer);
		m_crosshairVertexBuffer = 0;
	}

	if (m_crosshairVertexArray != 0)
	{
		glDeleteVertexArrays(1, &m_crosshairVertexArray);
		m_crosshairVertexArray = 0;
	}

	if (m_crosshairShaderProgram != 0)
	{
		glDeleteProgram(m_crosshairShaderProgram);
		m_crosshairShaderProgram = 0;
	}

	if (m_lineVertexBuffer != 0)
	{
		glDeleteBuffers(1, &m_lineVertexBuffer);
		m_lineVertexBuffer = 0;
	}

	if (m_lineVertexArray != 0)
	{
		glDeleteVertexArrays(1, &m_lineVertexArray);
		m_lineVertexArray = 0;
	}

	if (m_lineShaderProgram != 0)
	{
		glDeleteProgram(m_lineShaderProgram);
		m_lineShaderProgram = 0;
	}

	if (m_shaderProgram != 0)
	{
		glDeleteProgram(m_shaderProgram);
		m_shaderProgram = 0;
	}
}
