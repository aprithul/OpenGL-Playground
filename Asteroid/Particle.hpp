#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include "MathUtil.hpp"
#include "Entity.hpp"
#include "glew.h"
struct Particle
{
public:
	Transform transform;
	Vec3f start_velocity;
	Vec4f start_color;
	Vec4f end_color;
	Float_32 start_life_time;
	Float_32 life_time;
	bool is_alive;

	void start(Vec3f position, Vec3f start_velocity, Vec4f start_color, Vec4f end_color, Float_32 start_life_time);
};


struct ParticleEmitter : public Entity
{
public:

	static const int max_particle_count = 1024;
	GLuint vbo;
	ParticleEmitter(
		Vec3f position,
		Vec3f rotation,
		Float_32 start_speed,
		Vec4f start_color,
		Float_32 start_life_time,
		Int_32 rate,
		const char* texture);

	ParticleEmitter(
		Vec3f position,
		Vec3f rotation,
		Float_32 start_speed,
		Float_32 velocity_rand_factor,
		Vec4f start_color,
		Vec4f end_color,
		Float_32 color_rand_factor,
		Float_32 start_life_time,
		Float_32 life_time_rand_factor,
		int rate,
		const char* texture);

	void set_rate(int count);
	float get_start_life_time() const;
	void set_emission_cone_angle(Float_32 f);
	void reset();
	void start();
	void resume();
	void update(Float_32 dt);
	void pause();
	int get_count();
	const Particle* get_particle_array();

private:
	Float_32 start_speed;
	Float_32 emission_cone_angle;
	Vec4f start_color;
	Vec4f end_color;
	Float_32 color_rand_factor;
	Float_32 dt_accumulator;
	Float_32 interval;
	Float_32 start_life_time;
	Float_32 life_time_rand_factor;
	bool running;

	
	Particle particles[max_particle_count];
	Int_32 free_particles[max_particle_count];
	Int_32 next_free_particle;
	
	// rendering data
};


#endif