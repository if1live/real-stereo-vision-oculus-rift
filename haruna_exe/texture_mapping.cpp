// Ŭnicode please 
#include "texture_mapping.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "main.h"

#include <vector>

#include "sora/OVR_Types.h"
#include "sora/filesystem.h"
#include "sora/low_level_c_file.h"
#include "haruna/gl/shader.h"
#include "haruna/gl/texture.h"
#include "haruna/gl/gl_env.h"
#include "haruna/gl/frame_buffer.h"

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"



//init camera
cv::VideoCapture left_cap;
cv::VideoCapture right_cap;
cv::Mat left_frame;
cv::Mat right_frame;
cv::Mat left_image;
cv::Mat right_image;

EffectType effect_type = kEffectNo;


struct HMDDescriptor {
	int m_iResolutionH;
	int m_iResolutionV;
	float m_fSizeH;
	float m_fSizeV;
	float m_fInterpupillaryDistance;
	float m_fLensSeparationDistance;
	float m_fEyeToScreenDistance;
	float m_fDistortionK[4];

	HMDDescriptor() {
		m_iResolutionH = 1280;
		m_iResolutionV = 800;
		m_fSizeH = 0.14976f;
		m_fSizeV = 0.0936f;
		m_fInterpupillaryDistance = 0.064f;
		m_fLensSeparationDistance = 0.064f;
		m_fEyeToScreenDistance = 0.041f;
		m_fDistortionK[0] = 1.0f;
		m_fDistortionK[1] = 0.22f;
		m_fDistortionK[2] = 0.24f;
		m_fDistortionK[3] = 0.0f;
	}
};

Texture2DMapping::Texture2DMapping(float width, float height)
	: AbstractLogic(width, height)
{
}

Texture2DMapping::~Texture2DMapping()
{
	left_tex_->Deinit();
	right_tex_->Deinit();
	prog_->Deinit();
	fb_->Deinit();

	for(auto prog : effect_prog_list_) {
		prog->Deinit();
		delete(prog);
	}
}

bool Texture2DMapping::Init()
{
	//쉐이더 
	std::string vs_path = sora::Filesystem::GetAppPath("assets/shader/simple.vs");
	std::string fs_path = sora::Filesystem::GetAppPath("assets/shader/stereo.fs");
	sora::ReadonlyCFile fs_file = sora::ReadonlyCFile(fs_path);
	sora::ReadonlyCFile vs_file = sora::ReadonlyCFile(vs_path);
	bool fs_open_result = fs_file.Open();
	bool vs_open_result = vs_file.Open();
	if(!fs_open_result) {
		return false; 
	}
	if(!vs_open_result) {
		return false;
	}
	
	std::string fs_src(static_cast<const char*>(fs_file.GetBuffer()));
	std::string vs_src(static_cast<const char*>(vs_file.GetBuffer()));
	
	haruna::gl::VertexShader vs(vs_src);
	haruna::gl::FragmentShader fs(fs_src);

	prog_.reset(new haruna::gl::ShaderProgram(vs, fs));
	bool prog_result = prog_->Init();
	if(!prog_result) {
		return false;
	}

	//fb
	fb_.reset(new haruna::gl::FrameBuffer());
	bool fb_init_result = fb_->Init((int)width(), (int)height());
	if(fb_init_result == false) {
		return false;
	}

	effect_prog_list_[kEffectNo] = CreatePostEffect("assets/shader/no_effect.fs");
	effect_prog_list_[kEffectGray] = CreatePostEffect("assets/shader/gray_scale.fs");
	effect_prog_list_[kEffectSepia] = CreatePostEffect("assets/shader/sepia.fs");
	effect_prog_list_[kEffectEdgeDetection] = CreatePostEffect("assets/shader/edge_detection.fs");
	effect_prog_list_[kEffectEmboss] = CreatePostEffect("assets/shader/emboss.fs");
	for(auto effect : effect_prog_list_) {
		if(effect == nullptr) {
			return false;
		}
	}

    //opencv
    left_cap.open(0);
	right_cap.open(1);
	int frame_width = 640;
	int frame_height = 480;
	left_cap.set(CV_CAP_PROP_FRAME_WIDTH, frame_width);
	left_cap.set(CV_CAP_PROP_FRAME_HEIGHT, frame_height);
	right_cap.set(CV_CAP_PROP_FRAME_WIDTH, frame_width);
	right_cap.set(CV_CAP_PROP_FRAME_HEIGHT, frame_height);

	//create texture
	for(int i = 0 ; i < 2 ; ++i) {
		GLuint tex_id = 0;
		glGenTextures(1, &tex_id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture(GL_TEXTURE_2D, tex_id);

		cv::Mat *frame = nullptr;
		if(i == 0) {
			left_cap >> left_frame;
			frame = &left_frame;
		} else {
			right_cap >> right_frame;
			frame = &right_frame;
		}

		glBindTexture(GL_TEXTURE_2D, tex_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		bool tex_init_result = false;
		if(i == 0) {
			left_tex_.reset(new haruna::gl::Texture2D());
			tex_init_result = left_tex_->Init(tex_id, frame->cols, frame->rows);
		} else {
			right_tex_.reset(new haruna::gl::Texture2D());
			tex_init_result = right_tex_->Init(tex_id, frame->cols, frame->rows);
		}
		if(!tex_init_result) {
			return false;
		}
	}

	return true;
}
bool Texture2DMapping::Update(float dt)
{
	if(glfwGetKey(g_window, '1') == GLFW_PRESS) {
		effect_type = kEffectNo;
		printf("Current : No Effect\n");
	}
	if(glfwGetKey(g_window, '2') == GLFW_PRESS) {
		effect_type = kEffectGray;
		printf("Current : Gray Scale\n");
	}
	if(glfwGetKey(g_window, '3') == GLFW_PRESS) {
		effect_type = kEffectSepia;
		printf("Current : Sepia\n");
	}
	if(glfwGetKey(g_window, '4') == GLFW_PRESS) {
		effect_type = kEffectEmboss;
		printf("Current : Emboss\n");
	}
	if(glfwGetKey(g_window, '5') == GLFW_PRESS) {
		effect_type = kEffectEdgeDetection;
		printf("Current : Edge Detection\n");
	}


	left_cap >> left_frame;
	if(!left_frame.empty()) {
		left_frame.copyTo(left_image);
	}
	right_cap >> right_frame;
	if(!right_frame.empty()) {
		right_frame.copyTo(right_image);
	}

    //캠에서 얻은 정보를 기반으로 텍스쳐 생성 + 내용 갱신하기
    glBindTexture(GL_TEXTURE_2D, left_tex_->tex());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, left_image.cols, left_image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, left_image.data);

	glBindTexture(GL_TEXTURE_2D, right_tex_->tex());
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, right_image.cols, right_image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, right_image.data);

	//y_rot_ += 4.0f * dt;
	bool running = !glfwGetKey(g_window, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(g_window);
	return running;
}
void Texture2DMapping::Draw()
{
	fb_->Bind();
	{
		prog_->Use();
		haruna::gl::ShaderLocation pos_loc = prog_->GetAttribLocation("a_position");
		haruna::gl::ShaderLocation texcoord_loc = prog_->GetAttribLocation("a_texcoord");
		haruna::gl::ShaderLocation tex_loc = prog_->GetUniformLocation("s_tex");

		haruna::gl::ShaderLocation scale_loc = prog_->GetUniformLocation("scale");
		haruna::gl::ShaderLocation scale_in_loc = prog_->GetUniformLocation("scaleIn");
		haruna::gl::ShaderLocation lens_center_loc = prog_->GetUniformLocation("lensCenter");
		haruna::gl::ShaderLocation hmd_wrap_param_loc = prog_->GetUniformLocation("hmdWarpParam");

		float m_fScale[2];
		float m_fScaleIn[2];
		float m_fLensCenter[2];
		float m_fHmdWarpParam[4];
		{
			//hmd로 연결되는 변수

			// Init shader parameters
			m_fScale     [0] = 1.0f; m_fScale     [1] = 1.0f;
			m_fScaleIn   [0] = 1.0f; m_fScaleIn   [1] = 1.0f;
			m_fLensCenter[0] = 0.0f; m_fLensCenter[1] = 0.0f;

			m_fHmdWarpParam[0] = 1.0f;
			m_fHmdWarpParam[1] = 0.0f;
			m_fHmdWarpParam[2] = 0.0f;
			m_fHmdWarpParam[3] = 0.0f;

			HMDDescriptor m_cHMD;

			// Compute aspect ratio and FOV
			float l_fAspect    = m_cHMD.m_iResolutionH / (2.0f * m_cHMD.m_iResolutionV);
			float l_fR         = -1.0f - (4.0f * (m_cHMD.m_fSizeH / 4.0f - m_cHMD.m_fLensSeparationDistance / 2.0f) / m_cHMD.m_fSizeH);
			float l_fDistScale = (m_cHMD.m_fDistortionK[0] + m_cHMD.m_fDistortionK[1] * pow(l_fR,2) + m_cHMD.m_fDistortionK[2] * pow(l_fR,4) + m_cHMD.m_fDistortionK[3] * pow(l_fR,6));
			float l_fFov       = 2.0f * atan2(m_cHMD.m_fSizeV * l_fDistScale, 2.0f * m_cHMD.m_fEyeToScreenDistance);

			m_fScale[0] = 1.0f            / l_fDistScale;
			m_fScale[1] = 1.0f * l_fAspect/ l_fDistScale;

			m_fScaleIn[0] = 1.0f;
			m_fScaleIn[1] = 1.0f / l_fAspect;

			m_fHmdWarpParam[0] = m_cHMD.m_fDistortionK[0];
			m_fHmdWarpParam[1] = m_cHMD.m_fDistortionK[1];
			m_fHmdWarpParam[2] = m_cHMD.m_fDistortionK[2];
			m_fHmdWarpParam[3] = m_cHMD.m_fDistortionK[3];
		}

		float left_vertex_data[] = {
			-1, -1,
			0, -1,
			0, 1,
			-1, 1
		};
		float right_vertex_data[] = {
			0, -1,
			1, -1,
			1, 1,
			0, 1
		};
		float texcoord_data[] = {
			0, 1,
			1, 1,
			1, 0,
			0, 0
		};
		
		//draw
		glViewport(0, 0, (int)width(), (int)height());
		glClearColor(0, 0, 0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnableVertexAttribArray(pos_loc.handle());
		glEnableVertexAttribArray(texcoord_loc.handle());


		glUniform2fv(scale_loc.handle(), 1, m_fScale);
		glUniform2fv(scale_in_loc.handle(), 1, m_fScaleIn);
		glUniform2fv(lens_center_loc.handle(), 1, m_fLensCenter);
		glUniform4fv(hmd_wrap_param_loc.handle(), 1, m_fHmdWarpParam);

		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, left_tex_->tex());
			glUniform1i(tex_loc.handle(), 0);

			glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, left_vertex_data);
			glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 0, texcoord_data);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, right_tex_->tex());
			glUniform1i(tex_loc.handle(), 0);

			glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, right_vertex_data);
			glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 0, texcoord_data);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
	}
	haruna::gl::GLEnv::CheckError("End FB render");
	fb_->Unbind();

	auto effect = effect_prog_list_[effect_type];
	DrawPostEffect(*effect);

	haruna::gl::GLEnv::CheckError("End Frame");
}

haruna::gl::ShaderProgram *Texture2DMapping::CreatePostEffect(const char *effect_file)
{
	std::string vs_path = sora::Filesystem::GetAppPath("assets/shader/simple.vs");
	std::string fs_path = sora::Filesystem::GetAppPath(effect_file);
	sora::ReadonlyCFile vs_file = sora::ReadonlyCFile(vs_path);
	sora::ReadonlyCFile fs_file = sora::ReadonlyCFile(fs_path);
	if(!vs_file.Open()) {
		return false;
	}
	if(!fs_file.Open()) {
		return false;
	}
	
	std::string vs_src(static_cast<const char*>(vs_file.GetBuffer()));
	std::string fs_src(static_cast<const char*>(fs_file.GetBuffer()));
	
	haruna::gl::VertexShader vs(vs_src);
	haruna::gl::FragmentShader fs(fs_src);

	auto prog = new haruna::gl::ShaderProgram(vs, fs);
	bool prog_result = prog->Init();
	if(!prog_result) {
		delete(prog);
		return nullptr;
	}
	return prog;
}

bool Texture2DMapping::DrawPostEffect(haruna::gl::ShaderProgram &prog)
{
	prog.Use();

	glViewport(0, 0, (int)width(), (int)height());
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	haruna::gl::ShaderLocation pos_loc = prog.GetAttribLocation("a_position");
	haruna::gl::ShaderLocation texcoord_los = prog.GetAttribLocation("a_texcoord");
	haruna::gl::ShaderLocation scene_los = prog.GetUniformLocation("s_scene");
	haruna::gl::ShaderLocation pixel_offset_loc = prog.GetUniformLocation("u_pixelOffset");

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb_->color_tex());
	glUniform1i(scene_los, 0);

	glEnableVertexAttribArray(pos_loc);
	glEnableVertexAttribArray(texcoord_los);

	if(pixel_offset_loc.IsValid()) {
		glm::vec2 pixel_offset(1.0f / width(), 1.0f / height());
		glUniform2fv(pixel_offset_loc, 1, glm::value_ptr(pixel_offset));
	}
		
	float vertex_data[] = {
		-1, -1,
		1, -1,
		1, 1,
		-1, 1
	};
	float texcoord_data[] = {
		0, 0,
		1, 0,
		1, 1,
		0, 1
	};
	glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, vertex_data);
	glVertexAttribPointer(texcoord_los, 2, GL_FLOAT, GL_FALSE, 0, texcoord_data);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	return true;
}