// Ŭnicode please 
#pragma once
#include "renderer.h"
#include <array>

namespace haruna {;
namespace gl {
	class FrameBuffer;
}
}

typedef enum {
	kEffectNo,
	kEffectGray,
	kEffectSepia,
	kEffectEmboss,
	kEffectEdgeDetection,
	kEffectCount,
} EffectType;


class Texture2DMapping : public AbstractLogic {
public:
	Texture2DMapping(float width, float height);
	virtual ~Texture2DMapping();

	virtual bool Init();
	virtual bool Update(float dt);
	virtual void Draw();

private:
	haruna::gl::ShaderProgram *CreatePostEffect(const char *effect_file);
	std::array<haruna::gl::ShaderProgram*, kEffectCount> effect_prog_list_;
	bool DrawPostEffect(haruna::gl::ShaderProgram &prog);

private:
	std::unique_ptr<haruna::gl::ShaderProgram> prog_;
	
	std::unique_ptr<haruna::gl::Texture2D> left_tex_;
	std::unique_ptr<haruna::gl::Texture2D> right_tex_;

	std::unique_ptr<haruna::gl::FrameBuffer> fb_;
};

