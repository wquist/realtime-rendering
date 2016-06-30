#include "scene.h"
glm::uvec2 scene::window_size   = glm::uvec2(1024, 768);
const char scene::window_name[] = "Sponza";

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../common/exception.h"
#include "../input/keyboard.h"
#include "../render/window.h"
#include "../render/vertex_array.h"
#include "../render/texture.h"
#include "../render/shader.h"
#include "../render/camera2.h"
#include "../render/camera3.h"
#include "../render/frame_buffer.h"
#include "../resource/obj_model.h"
#include "../resource/tga_image.h"

#include "../common/frame_counter.h"

#include "../draw/shadow_map.h"
#include "../draw/cube_map.h"
#include "../draw/font.h"

struct mesh_info
{
	vertex_array data;
	std::string material;
};

struct material_info
{
	std::unique_ptr<texture> diffuse;
	std::unique_ptr<texture> alpha;
	std::unique_ptr<texture> normal;
	std::unique_ptr<texture> specular;
};

// Configuration options:
const glm::uvec2 g_resolution = scene::window_size; //< Scaled rendering
const glm::uvec2 g_shadowsize = glm::uvec2(2048); //< Resolution of shadow map
const glm::uvec3 g_lpvsize    = glm::uvec3(32); //< LPV 3D grid size
const glm::uvec2 g_vplcount   = glm::uvec2(1024); //< Resolution of VPL samples
const size_t     g_iterations = 8; //< Passes for LPV
const size_t     g_dustcount  = 32768; //< Number of GPU simulated dust particles

std::unique_ptr<window> g_window;

// Base rendering objects
std::vector<mesh_info> g_model;
std::map<std::string, material_info> g_textures;
std::unique_ptr<texture> g_whitetex;
std::unique_ptr<texture> g_graytex;
std::unique_ptr<texture> g_blacktex;

// First pass, directional lighting
std::unique_ptr<shader> g_input;
glm::vec3 g_light;

// View parameters
std::unique_ptr<camera3> g_camera;
glm::mat4 g_world;

// Light propagation volumes objects
std::unique_ptr<frame_buffer> g_rsm;
std::unique_ptr<shader> g_shadow;
aabb<glm::vec3> g_bounds;
std::unique_ptr<frame_buffer> g_lpv;
std::unique_ptr<shader> g_inject;
std::unique_ptr<vertex_array> g_vplcloud;
std::unique_ptr<frame_buffer> g_bounce[2];
size_t g_pingpong;
std::unique_ptr<shader> g_normalize;
std::unique_ptr<shader> g_propagate;
std::unique_ptr<vertex_array> g_volume;

// Deferred shading and post-process objects
std::unique_ptr<vertex_array> g_quad;
std::unique_ptr<frame_buffer> g_gbuffer;
std::unique_ptr<shader> g_postprocess;
std::unique_ptr<frame_buffer> g_accum;
std::unique_ptr<shader> g_output;

// Dust particles and participating media objects
std::unique_ptr<vertex_array> g_particles;
std::unique_ptr<frame_buffer> g_air;
std::unique_ptr<shader> g_dust;

std::unique_ptr<cube_map> g_skybox;

// Performance information objects
std::unique_ptr<frame_counter> g_fps;
std::unique_ptr<camera2> g_hud;
std::unique_ptr<font> g_font;
std::unique_ptr<font::text> g_text;

// Parameters that can be set pressing special keys during program run
struct
{
	int  mode;
	bool normal_map;
	bool shadow_map;
	bool ssao;
	bool ssr;
	bool gi;
	bool rays;
	int  layer;
} g_options;

void scene::create(GLFWwindow* wnd) try
{
	g_window = std::make_unique<window>(wnd);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_3D);

	glClearColor(0.f, 0.f, 0.f, 0.f);

	// Set up empty textures for missing sponza channels
	std::vector<unsigned char> wdata = { 255 };
	texture::source wsrc(wdata, GL_RED);
	texture::target wdst(GL_RED);

	g_whitetex = std::make_unique<texture>(GL_TEXTURE_2D);
	g_whitetex->set_data(wsrc, wdst, glm::uvec2(1));

	std::vector<unsigned char> gdata = { 127,127,255 };
	texture::source gsrc(gdata, GL_RGB);
	texture::target gdst(GL_RGB);

	g_graytex = std::make_unique<texture>(GL_TEXTURE_2D);
	g_graytex->set_data(gsrc, gdst, glm::uvec2(1));

	std::vector<unsigned char> bdata = { 31 };
	texture::source bsrc(bdata, GL_RED);
	texture::target bdst(GL_RED);

	g_blacktex = std::make_unique<texture>(GL_TEXTURE_2D);
	g_blacktex->set_data(bsrc, bdst, glm::uvec2(1));

	// Create the deffered shader
	g_input = std::make_unique<shader>();
	g_input->compile("data/shader/gbuffer.vs", GL_VERTEX_SHADER);
	g_input->compile("data/shader/gbuffer.fs", GL_FRAGMENT_SHADER);
	g_input->link();

	g_input->declare_uniform("M");
	g_input->declare_uniform("VP");

	g_input->declare_uniform("Color");
	g_input->declare_uniform("Alpha");
	g_input->declare_uniform("Normal");
	g_input->declare_uniform("Specular");

	g_input->declare_uniform("NormalMap");

	g_light = glm::normalize(glm::vec3(0.5f, -1.f, 0.f));

	g_camera = std::make_unique<camera3>(*g_window, 60.f, 30.f);
	g_camera->move_to(glm::vec3(0.f, 1.f, 0.f));
	g_camera->look_at(glm::vec3(1.f, 1.f, 0.f));

	g_world = glm::scale(glm::mat4(1.f), glm::vec3(0.005f));

	// Define shadow map related shaders + buffers
	g_rsm = std::make_unique<frame_buffer>(g_shadowsize);
	g_rsm->attach_color(0, GL_RGB);
	g_rsm->attach_color(1, GL_RGB);
	g_rsm->attach_depth(GL_DEPTH_COMPONENT24);
	g_rsm->finish();

	g_shadow = std::make_unique<shader>();
	g_shadow->compile("data/shader/rsm.vs", GL_VERTEX_SHADER);
	g_shadow->compile("data/shader/rsm.fs", GL_FRAGMENT_SHADER);
	g_shadow->link();

	g_shadow->declare_uniform("M");
	g_shadow->declare_uniform("VP");

	g_shadow->declare_uniform("Color");

	// Make the light propagation volume textures
	g_lpv = std::make_unique<frame_buffer>(g_lpvsize.x, g_lpvsize.y);
	for (auto i = 0; i != 3; ++i)
	{
		texture::source src(GL_RGBA);
		texture::target dst(GL_RGBA16F);

		texture tex(GL_TEXTURE_3D);
		tex.set_data(src, dst, g_lpvsize);
		tex.set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		tex.set_filtering(GL_NEAREST, GL_NEAREST);

		g_lpv->attach_color(i, std::move(tex));
	}

	texture::source csrc(GL_RED);
	texture::target cdst(GL_R16);

	texture ctex(GL_TEXTURE_3D);
	ctex.set_data(csrc, cdst, g_lpvsize);
	ctex.set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	ctex.set_filtering(GL_NEAREST, GL_NEAREST);

	g_lpv->attach_color(3, std::move(ctex));
	g_lpv->finish();

	// The inject phase of LPV
	g_inject = std::make_unique<shader>();
	g_inject->compile("data/shader/lpv_inject.vs", GL_VERTEX_SHADER);
	g_inject->compile("data/shader/lpv_inject.gs", GL_GEOMETRY_SHADER);
	g_inject->compile("data/shader/lpv_inject.fs", GL_FRAGMENT_SHADER);
	g_inject->link();

	g_inject->declare_uniform("InvLightVP");
	g_inject->declare_uniform("Clip");

	g_inject->declare_uniform("Color");
	g_inject->declare_uniform("Normal");
	g_inject->declare_uniform("Depth");

	std::vector<char> vpl_null(g_vplcount.x * g_vplcount.y);

	// Vertex arrays apparently need at least one buffer attached
	g_vplcloud = std::make_unique<vertex_array>();
	g_vplcloud->attach("null", vpl_null, 0);

	// Create textures for LPV propagation phase
	std::vector<texture> atex;
	for (auto i = 0; i != 3; ++i)
	{
		texture::source src(GL_RGBA);
		texture::target dst(GL_RGBA16F);

		texture tex(GL_TEXTURE_3D);
		tex.set_data(src, dst, g_lpvsize);
		tex.set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		tex.set_filtering(GL_LINEAR, GL_LINEAR);

		atex.emplace_back(std::move(tex));
	}

	for (auto i = 0; i != 2; ++i)
	{
		g_bounce[i] = std::make_unique<frame_buffer>(g_lpvsize.x, g_lpvsize.y);
		for (auto j = 0; j != 3; ++j)
		{
			texture::source src(GL_RGBA);
			texture::target dst(GL_RGBA16F);

			texture tex(GL_TEXTURE_3D);
			tex.set_data(src, dst, g_lpvsize);
			tex.set_wrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
			tex.set_filtering(GL_NEAREST, GL_NEAREST);

			g_bounce[i]->attach_color(j, std::move(tex));
		}

		for (auto j = 0; j != 3; ++j)
		{
			texture tex = atex[j];
			g_bounce[i]->attach_color(j+3, std::move(tex));
		}

		g_bounce[i]->finish();
	}

	// Use ping-ponging to reduce number of unique buffers needed
	g_pingpong = 0;

	// Add a normalization phase to even out LPV light intensity
	g_normalize = std::make_unique<shader>();
	g_normalize->compile("data/shader/lpv_propagate.vs", GL_VERTEX_SHADER);
	g_normalize->compile("data/shader/lpv_propagate.gs", GL_GEOMETRY_SHADER);
	g_normalize->compile("data/shader/lpv_normalize.fs", GL_FRAGMENT_SHADER);
	g_normalize->link();

	g_normalize->declare_uniform("RCoeff");
	g_normalize->declare_uniform("GCoeff");
	g_normalize->declare_uniform("BCoeff");
	g_normalize->declare_uniform("InjectCount");

	g_propagate = std::make_unique<shader>();
	g_propagate->compile("data/shader/lpv_propagate.vs", GL_VERTEX_SHADER);
	g_propagate->compile("data/shader/lpv_propagate.gs", GL_GEOMETRY_SHADER);
	g_propagate->compile("data/shader/lpv_propagate.fs", GL_FRAGMENT_SHADER);
	g_propagate->link();

	g_propagate->declare_uniform("RCoeff");
	g_propagate->declare_uniform("GCoeff");
	g_propagate->declare_uniform("BCoeff");

	std::vector<char> vol_null(g_lpvsize.x * g_lpvsize.y * g_lpvsize.z);

	g_volume = std::make_unique<vertex_array>();
	g_volume->attach("null", vol_null, 0);

	// The screen covering quad is used for deferred rendering
	std::vector<glm::vec2> quad_verts =
	{
		{ -1.f,  1.f },
		{  1.f, -1.f },
		{ -1.f, -1.f },
		{ -1.f,  1.f },
		{  1.f,  1.f },
		{  1.f, -1.f }
	};

	g_quad = std::make_unique<vertex_array>();
	g_quad->attach("position", quad_verts, 0);

	g_gbuffer = std::make_unique<frame_buffer>(g_resolution);
	g_gbuffer->attach_color(0, GL_RGBA);
	g_gbuffer->attach_color(1, GL_RGBA);
	g_gbuffer->attach_depth(GL_DEPTH_COMPONENT24);
	g_gbuffer->finish();

	// All effects are applied in the post process shader
	g_postprocess = std::make_unique<shader>();
	g_postprocess->compile("data/shader/postprocess.vs", GL_VERTEX_SHADER);
	g_postprocess->compile("data/shader/postprocess.fs", GL_FRAGMENT_SHADER);
	g_postprocess->link();

	g_postprocess->declare_uniform("VP");
	g_postprocess->declare_uniform("InvVP");
	g_postprocess->declare_uniform("Clip");
	g_postprocess->declare_uniform("CameraPos");
	g_postprocess->declare_uniform("LightVP");
	g_postprocess->declare_uniform("LightDir");

	g_postprocess->declare_uniform("Albedo");
	g_postprocess->declare_uniform("Normal");
	g_postprocess->declare_uniform("Depth");
	g_postprocess->declare_uniform("Shadow");
	g_postprocess->declare_uniform("LPV");
	g_postprocess->declare_uniform("GIR");
	g_postprocess->declare_uniform("GIG");
	g_postprocess->declare_uniform("GIB");
	g_postprocess->declare_uniform("Dust");
	g_postprocess->declare_uniform("Air");

	g_postprocess->declare_uniform("Mode");
	g_postprocess->declare_uniform("Shadows");
	g_postprocess->declare_uniform("Occlusion");
	g_postprocess->declare_uniform("Reflections");
	g_postprocess->declare_uniform("Indirect");
	g_postprocess->declare_uniform("Godrays");
	g_postprocess->declare_uniform("Layer");

	g_accum = std::make_unique<frame_buffer>(g_resolution);
	g_accum->attach_color(0, GL_RGB, GL_RGB16F);
	g_accum->attach_depth(GL_DEPTH_COMPONENT24);
	g_accum->finish();

	g_output = std::make_unique<shader>();
	g_output->compile("data/shader/composite.vs", GL_VERTEX_SHADER);
	g_output->compile("data/shader/composite.fs", GL_FRAGMENT_SHADER);
	g_output->link();

	g_output->declare_uniform("Color");

	// Set up the skybox
	// Skybox images from:
	// http://www.learnopengl.com/#!Advanced-OpenGL/Cubemaps
	std::vector<image> skytexes =
	{
		tga_image("data/texture/sky/right.tga"),
		tga_image("data/texture/sky/left.tga"),
		tga_image("data/texture/sky/top.tga"),
		tga_image("data/texture/sky/bottom.tga"),
		tga_image("data/texture/sky/back.tga"),
		tga_image("data/texture/sky/front.tga")
	};

	g_skybox = std::make_unique<cube_map>();
	g_skybox->attach(skytexes);

	g_fps = std::make_unique<frame_counter>();

	g_hud = std::make_unique<camera2>(*g_window, scene::window_size.y / 2);
	g_font = std::make_unique<font>("basic", 256);

	g_text = g_font->create_text(-8.f, 0.f);
	g_text->update("FRAME TIME: ?ms");

	// Load the models last - if there are any errors above you dont wait as long
	// Sponza model from:
	// http://graphics.cs.williams.edu/data/meshes.xml
	auto model = obj_model("data/model/sponza.obj", true);
	for (const auto& mesh : model.meshes())
	{
		auto info = mesh_info();

		info.data.attach("position", mesh.positions,  0);
		info.data.attach("texcoord", mesh.texcoords,  1);
		info.data.attach("normal",   mesh.normals,    2);

		info.material = mesh.material;

		g_model.emplace_back(std::move(info));
	}

	model.load_materials("data/model/sponza.mtl");
	for (const auto& material : model.materials())
	{
		auto get_texture = [](const std::string& name, GLenum fmt)
		{
			auto img = tga_image("data/texture/sponza/" + name);
			auto tex = std::make_unique<texture>(GL_TEXTURE_2D);

			texture::source src(img.pixels(), img.format());
			texture::target dst(fmt);

			tex->set_data(src, dst, img.size());
			return tex;
		};

		material_info info;
		info.diffuse = get_texture(material.second.diffuse, GL_RGB);
		info.diffuse->set_filtering(GL_LINEAR, GL_LINEAR, GL_LINEAR);

		if (material.second.has_alpha())
			info.alpha = get_texture(material.second.alpha, GL_RED);
		if (material.second.has_normal())
			info.normal = get_texture(material.second.normal, GL_RGB);
		if (material.second.has_specular())
			info.specular = get_texture(material.second.specular, GL_RED);

		info.diffuse->bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);

		g_textures.emplace(material.first, std::move(info));
	}

	g_bounds = model.calculate_bounds();

	// Create the GPU particles at random positions, then interpolate on time
	std::vector<glm::vec4> dpos(g_dustcount);
	for (auto i = 0; i != g_dustcount; ++i)
	{
		float rx = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		float ry = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		float rz = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

		glm::vec3 range = g_bounds.max() - g_bounds.min();
		dpos[i] = glm::vec4(glm::vec3(rx, ry, rz) * range - (range * 0.5f), rand());
	}

	g_particles = std::make_unique<vertex_array>();
	g_particles->attach("position", dpos, 0);

	g_air = std::make_unique<frame_buffer>(g_resolution);
	g_air->attach_color(0, GL_RGB);
	g_air->attach_depth(GL_DEPTH_COMPONENT24);
	g_air->finish();

	g_dust = std::make_unique<shader>();
	g_dust->compile("data/shader/dust.vs", GL_VERTEX_SHADER);
	g_dust->compile("data/shader/dust.fs", GL_FRAGMENT_SHADER);
	g_dust->link();

	g_dust->declare_uniform("M");
	g_dust->declare_uniform("VP");
	g_dust->declare_uniform("Time");
	g_dust->declare_uniform("Min");
	g_dust->declare_uniform("Max");

	// Default everything on
	g_options.mode = 0;
	g_options.normal_map = true;
	g_options.shadow_map = true;
	g_options.ssao       = true;
	g_options.ssr        = true;
	g_options.gi         = true;
	g_options.rays       = true;
	g_options.layer      = 0;

	keyboard::listen(*g_window, keyboard::event::down, [](int key)
	{
		switch (key)
		{
			case GLFW_KEY_0: g_options.mode = 0; break; // Final
			case GLFW_KEY_1: g_options.mode = 1; break; // Raw color
			case GLFW_KEY_2: g_options.mode = 2; break; // Specular
			case GLFW_KEY_3: g_options.mode = 3; break; // Position
			case GLFW_KEY_4: g_options.mode = 4; break; // Normals
			case GLFW_KEY_5: g_options.mode = 5; break; // Ambient occlusion
			case GLFW_KEY_6: g_options.mode = 6; break; // Reflections
			case GLFW_KEY_7: g_options.mode = 7; break; // Shadow map
			case GLFW_KEY_8: g_options.mode = 8; break; // LPV initial injection
			case GLFW_KEY_9: g_options.mode = 9; break; // LPV final contribution
			case GLFW_KEY_B: g_options.ssr        = !g_options.ssr;         break; // Reflections
			case GLFW_KEY_G: g_options.gi         = !g_options.gi;          break; // LPV
			case GLFW_KEY_U: g_options.layer      = (g_options.layer+1)%32; break; // Select LPV slice to view
			case GLFW_KEY_M: g_options.shadow_map = !g_options.shadow_map;  break; // Shadow mapping
			case GLFW_KEY_N: g_options.normal_map = !g_options.normal_map;  break; // Normal mapping
			case GLFW_KEY_O: g_options.ssao       = !g_options.ssao;        break; // Ambient occlusion
			case GLFW_KEY_R: g_options.rays       = !g_options.rays;        break; // Godrays
		}
	});
}
catch (exception& e)
{
	std::cout << e.source() << " error in " << e.where() << ":" << std::endl;
	std::cout << e.what() << std::endl;

	exit(1);
}

void scene::destroy() {}

void scene::update()
{
	static glm::vec2 angle;
	if (keyboard::get(*g_window, GLFW_KEY_A))
		angle.x += 0.02f;
	if (keyboard::get(*g_window, GLFW_KEY_D))
		angle.x -= 0.02f;
	if (keyboard::get(*g_window, GLFW_KEY_W))
		angle.y += 0.02f;
	if (keyboard::get(*g_window, GLFW_KEY_S))
		angle.y -= 0.02f;

	auto rotp = glm::rotate(glm::mat4(1.f), angle.y, glm::vec3(0, 0, 1));
	auto roty = glm::rotate(glm::mat4(1.f), angle.x, glm::vec3(0, 1, 0));
	auto ray  = glm::vec3(roty * rotp * glm::vec4(1.f, 0.f, 0.f, 1.f));

	g_camera->look_at(g_camera->position() + ray);

	float dir = 0.f;
	if (keyboard::get(*g_window, GLFW_KEY_UP))
		dir += 0.05f;
	if (keyboard::get(*g_window, GLFW_KEY_DOWN))
		dir -= 0.05f;

	auto delta = glm::normalize(ray) * dir;
	g_camera->move_to(g_camera->position() + delta);

	glm::vec3 phase = glm::vec3();
	if (keyboard::get(*g_window, GLFW_KEY_J))
		phase.x += 0.01f;
	if (keyboard::get(*g_window, GLFW_KEY_L))
		phase.x -= 0.01f;
	if (keyboard::get(*g_window, GLFW_KEY_I))
		phase.z += 0.01f;
	if (keyboard::get(*g_window, GLFW_KEY_K))
		phase.z -= 0.01f;

	g_light = glm::rotate(g_light, phase.x, glm::vec3(1, 0, 0));
	g_light = glm::rotate(g_light, phase.z, glm::vec3(0, 0, 1));

	g_fps->update();
	if (g_fps->dirty())
	{
		auto tms = std::to_string(g_fps->frame_time()) + "ms";
		g_text->update("FRAME TIME: " + tms);
	}
}

void scene::render()
{
	// First render the (reflective) shadow map

	auto shadow_vp = shadow_map::directional(*g_camera, g_light);

	auto diag = glm::abs(g_bounds.max() - g_bounds.min());
	auto unit = glm::scale(glm::mat4(1.f), glm::vec3(1.f)/diag);
	auto clip = unit * glm::translate(glm::mat4(1.f), -g_bounds.min());

	glViewport(0.f, 0.f, g_shadowsize.x, g_shadowsize.y);

	g_rsm->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_shadow->use();
	g_shadow->set_uniform("M",  g_world);
	g_shadow->set_uniform("VP", shadow_vp);

	g_shadow->set_uniform("Color", 0);

	// Only draw with diffuse (color needed for RSM)
	for (const auto& info : g_model)
	{
		if (!g_textures.count(info.material))
			continue;

		info.data.bind();
		g_textures.at(info.material).diffuse->bind(0);

		glDrawArrays(GL_TRIANGLES, 0, info.data.size());
	}

	// Inject the RSM into the LPVs

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glViewport(0.f, 0.f, g_lpvsize.x, g_lpvsize.y);

	g_lpv->bind();
	glClear(GL_COLOR_BUFFER_BIT);

	g_inject->use();
	g_inject->set_uniform("InvLightVP", glm::inverse(shadow_vp));
	g_inject->set_uniform("Clip", clip * glm::inverse(g_world));

	g_inject->set_uniform("Color",  0);
	g_inject->set_uniform("Normal", 1);
	g_inject->set_uniform("Depth",  2);

	g_rsm->bind_color(0, 0);
	g_rsm->bind_color(1, 1);
	g_rsm->bind_depth(2);

	g_vplcloud->bind();
	glDrawArrays(GL_POINTS, 0, g_vplcloud->size());

	g_pingpong = 0;

	g_bounce[g_pingpong]->bind();
	g_bounce[g_pingpong]->write_to_all();
	glClear(GL_COLOR_BUFFER_BIT);

	// Normalize contributions to prevent bright spots

	g_normalize->use();
	g_normalize->set_uniform("RCoeff", 0);
	g_normalize->set_uniform("GCoeff", 1);
	g_normalize->set_uniform("BCoeff", 2);
	g_normalize->set_uniform("InjectCount", 3);

	g_lpv->bind_color(0, 0);
	g_lpv->bind_color(1, 1);
	g_lpv->bind_color(2, 2);
	g_lpv->bind_color(3, 3);

	g_volume->bind();
	glDrawArrays(GL_POINTS, 0, g_volume->size());

	// Calculate LPV propagations

	for (auto i = 0; i != g_iterations; ++i)
	{
		auto next = (g_pingpong + 1) % 2;

		g_bounce[next]->bind();
		g_bounce[next]->write_to({ 0, 1, 2 });
		glClear(GL_COLOR_BUFFER_BIT);
		g_bounce[next]->write_to_all();

		g_propagate->use();
		g_propagate->set_uniform("RCoeff", 0);
		g_propagate->set_uniform("GCoeff", 1);
		g_propagate->set_uniform("BCoeff", 2);

		g_bounce[g_pingpong]->bind_color(0, 0);
		g_bounce[g_pingpong]->bind_color(1, 1);
		g_bounce[g_pingpong]->bind_color(2, 2);

		g_volume->bind();
		glDrawArrays(GL_POINTS, 0, g_volume->size());

		g_pingpong = next;
	}

	glDisable(GL_BLEND);

	// Render the scene normally

	glViewport(0.f, 0.f, g_resolution.x, g_resolution.y);

	g_gbuffer->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_input->use();
	g_input->set_uniform("M",  g_world);
	g_input->set_uniform("VP", g_camera->matrix());

	g_input->set_uniform("Color",    0);
	g_input->set_uniform("Alpha",    1);
	g_input->set_uniform("Normal",   2);
	g_input->set_uniform("Specular", 3);

	g_input->set_uniform("NormalMap", (int)g_options.normal_map);

	for (const auto& info : g_model)
	{
		if (!g_textures.count(info.material))
			continue;

		info.data.bind();
		const auto& material = g_textures.at(info.material);

		material.diffuse->bind(0);

		// If a channel does not exist, use a default color appropriate for that channel

		if (material.alpha)
			material.alpha->bind(1);
		else
			g_whitetex->bind(1);

		if (material.normal)
			material.normal->bind(2);
		else
			g_graytex->bind(2);

		if (material.specular)
			material.specular->bind(3);
		else
			g_blacktex->bind(3);

		glDrawArrays(GL_TRIANGLES, 0, info.data.size());
	}

	glViewport(0.f, 0.f, g_resolution.x, g_resolution.y);

	// Render participating media

	g_air->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_dust->use();
	g_dust->set_uniform("M", g_world);
	g_dust->set_uniform("VP", g_camera->matrix());

	static float t = 0.f; t += 0.3f;
	g_dust->set_uniform("Time", t);
	g_dust->set_uniform("Min", g_bounds.min());
	g_dust->set_uniform("Max", g_bounds.max());

	g_particles->bind();
	glDrawArrays(GL_POINTS, 0, g_particles->size());

	// Accumlate the final image by applying all effects

	glViewport(0.f, 0.f, g_resolution.x, g_resolution.y);

	g_accum->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_postprocess->use();
	g_postprocess->set_uniform("VP", g_camera->matrix());
	g_postprocess->set_uniform("InvVP", glm::inverse(g_camera->matrix()));
	g_postprocess->set_uniform("Clip", clip * glm::inverse(g_world));
	g_postprocess->set_uniform("CameraPos", g_camera->position());
	g_postprocess->set_uniform("LightVP", shadow_vp);
	g_postprocess->set_uniform("LightDir", g_light);

	g_postprocess->set_uniform("Albedo", 0);
	g_postprocess->set_uniform("Normal", 1);
	g_postprocess->set_uniform("Depth",  2);
	g_postprocess->set_uniform("Shadow", 3);
	g_postprocess->set_uniform("LPV",    4);
	g_postprocess->set_uniform("GIR",    5);
	g_postprocess->set_uniform("GIG",    6);
	g_postprocess->set_uniform("GIB",    7);
	g_postprocess->set_uniform("Dust",   8);
	g_postprocess->set_uniform("Air",    9);

	g_postprocess->set_uniform("Mode", g_options.mode);
	g_postprocess->set_uniform("Shadows", (int)g_options.shadow_map);
	g_postprocess->set_uniform("Occlusion", (int)g_options.ssao);
	g_postprocess->set_uniform("Reflections", (int)g_options.ssr);
	g_postprocess->set_uniform("Indirect", (int)g_options.gi);
	g_postprocess->set_uniform("Godrays", (int)g_options.rays);
	g_postprocess->set_uniform("Layer", g_options.layer);

	g_gbuffer->bind_color(0, 0);
	g_gbuffer->bind_color(1, 1);
	g_gbuffer->bind_depth(2);
	g_rsm->bind_depth(3);
	g_lpv->bind_color(0, 4);
	g_bounce[g_pingpong]->bind_color(3, 5);
	g_bounce[g_pingpong]->bind_color(4, 6);
	g_bounce[g_pingpong]->bind_color(5, 7);
	g_air->bind_color(0, 8);
	g_air->bind_depth(9);

	g_quad->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	g_skybox->draw(*g_camera);

	glViewport(0.f, 0.f, g_window->render_width(), g_window->render_height());

	g_accum->bind(false);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_output->use();
	g_output->set_uniform("Color", 0);

	g_accum->bind_color(0, 0);

	g_quad->bind();
	glDrawArrays(GL_TRIANGLES, 0, 6);

	auto px = static_cast<float>(scene::window_size.x) / 2.f - 10.f;
	auto py = static_cast<float>(scene::window_size.y) / 2.f - 10.f;
	auto hud_model = glm::translate(glm::mat4(1.f), glm::vec3(-px, py, 0.f));

	g_font->draw(*g_text, *g_hud, hud_model);
}
