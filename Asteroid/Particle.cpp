#include "Particle.hpp"
#include "Rendering.hpp"
#include "Utils.hpp"
#include <assert.h>
#include <cstdlib>


Mat4x4 make_transform_from_rotation(float x, float y, float z)
{
	Mat3x3 rot_x = { 1, 0, 0,
					0, cosf(x), -sinf(x),
					0, sinf(x), cosf(x) };

	Mat3x3 rot_y = { cosf(y), 0, sinf(y),
				0, 1, 0,
				-sinf(y), 0,	cosf(y) };

	Mat3x3 rot_z = { cosf(z), -sinf(z), 0,
					sinf(z), cosf(z), 0,
					0,	0,	1 };

	return rot_z * rot_y * rot_x;
}

void Particle::	start(Vec3f position, Vec3f start_velocity, Vec4f start_color, Vec4f end_color, Float_32 start_life_time)
{
	this->transform.position = position;
	//float max_rot = PI / 16.f;
	//Mat4x4 rotator = make_transform_from_rotation(max_rot*get_rand(), max_rot*get_rand(), max_rot*get_rand());
	this->start_velocity = start_velocity;// rotator * Vec4f{ velocity.x, velocity.y, velocity.z, 0 };
	this->start_color = start_color;
	this->end_color = end_color;
	this->start_life_time = start_life_time;// *(get_rand() + 1.f);
	this->life_time = start_life_time;// *(get_rand() + 1.f);
	this->is_alive = true;
}



ParticleEmitter::ParticleEmitter(
Vec3f position,
Vec3f rotation,
Float_32 start_speed,
Vec4f start_color,
Float_32 start_life_time,
Int_32 rate,
const char* texture):
	start_speed(start_speed), emission_cone_angle(0),
		start_color(start_color), end_color(start_color), color_rand_factor(0), start_life_time(start_life_time),
			life_time_rand_factor(0) {
	this->transform.position = position;
	this->transform.rotation = rotation;
	//vao_count = 1;
	set_rate(rate);
	// set up a quad mesh for drawing particles with instancing
	make_particle_mesh(*this);
	make_texture(texture, &texture_diffuse, GL_SRGB_ALPHA, true);
}


ParticleEmitter::ParticleEmitter(
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
	const char* texture) :
	start_speed(start_speed),emission_cone_angle(velocity_rand_factor),
		start_color(start_color), end_color(end_color), color_rand_factor(color_rand_factor),start_life_time(start_life_time), 
			life_time_rand_factor(life_time_rand_factor)
{
	this->transform.position = position;
	this->transform.rotation = rotation;
	//vao_count = 1;
	set_rate(rate);
	// set up a quad mesh for drawing particles with instancing
	make_particle_mesh(*this);
	make_texture(texture, &texture_diffuse, GL_SRGB_ALPHA, true);
}


int ParticleEmitter::get_count()
{
	return max_particle_count - (next_free_particle+1);
}

void ParticleEmitter::reset()
{
	
	for (auto p : particles)
	{
		p.is_alive = false;
	}

	// all particles are available
	for (int i = 0; i < max_particle_count; i++)
	{
		free_particles[i] = i;
	}
	next_free_particle = max_particle_count-1;
	dt_accumulator = 0;
}


void ParticleEmitter::resume()
{
	running = true;
}

void ParticleEmitter::start()
{
	reset();
	running = true;
}


float clamp_color(float v)
{
	if (v < 0.f)
		v = 1.f + v;
	else if (v > 1.f)
		v = (v - 1.f);

	return v;
}

void ParticleEmitter::update(Float_32 dt)
{

	if (running)
	{	
		// update particles that are instanced
		for (int i = 0; i < max_particle_count; i++)
		{
			Particle& p = particles[i];
			if (p.is_alive)
			{
				p.life_time -= dt;
				p.transform.position = p.transform.position + ((p.start_velocity + Vec3f(get_rand(-0.1f,0.1f),0, get_rand(-0.1f, 0.1f))) *dt);

				if (p.life_time <= 0.f)
				{
					p.is_alive = false;
					next_free_particle++;
					free_particles[next_free_particle] = i;
				}
			}
		}

		// create new particle if we've gone past interval
		dt_accumulator += dt;
		if (dt_accumulator >= interval)
		{
			dt_accumulator -= interval;
			Uint_32 free_particle = free_particles[next_free_particle];
			next_free_particle--;
			assert(next_free_particle >= 0);
			//Vec3f velocity = transform.transform_direction(start_velocity);

			// calculate parameters with random deviation for particle
			Particle& _particle = particles[free_particle];
			// velocity
			float max_rot = PI;
			//Mat4x4 velocity_rotator = make_transform_from_rotation(max_rot*velocity_rand_factor*get_rand(), max_rot*velocity_rand_factor*get_rand(), max_rot*velocity_rand_factor*get_rand());
			//Mat4x4 velocity_rotator = make_transform_from_rotation(get_rand()*max_rot*velocity_rand_factor, get_rand()*max_rot*velocity_rand_factor, 0);// get_rand()*max_rot*velocity_rand_factor);// *velocity_rand_factor*get_rand());
			//Vec3f _velocity = velocity_rotator * Vec4f{ velocity.x, velocity.y, velocity.z, 0 };

			// get a random rotation within the cone of emission
			Float_32 theta = get_rand(0.f, 2 * PI);
			Float_32 _z = get_rand(cosf(emission_cone_angle), 1.f);
			Vec3f _velocity = { cosf(theta) * sqrtf(1.f - (_z*_z)), _z , sinf(theta) * sqrtf(1.f - (_z*_z))};
			_velocity = transform.transform_direction(_velocity) * start_speed;

			// life time
			Float_32 _life_time = start_life_time * (get_rand(-1.f,1.f)*life_time_rand_factor + 1.f);
			// color
			float r = start_color.x + get_rand(-1,1)*color_rand_factor;//*start_color.x;
			r = clamp(r, 0, 1.f);
			float g = start_color.y + get_rand(-1, 1)*color_rand_factor;//*start_color.y;
			g = clamp(g, 0, 1.f);
			float b = start_color.z + get_rand(-1, 1)*color_rand_factor;//*start_color.z;
			b = clamp(b, 0, 1.f);
			float a = start_color.w;


			Vec4f _color = {r,g,b,a};

			particles[free_particle].start(transform.position, _velocity, _color, end_color, _life_time);
		}
	}
}

void ParticleEmitter::set_emission_cone_angle(Float_32 f)
{
	this->emission_cone_angle = f;
}

void ParticleEmitter::pause()
{
	running = false;
}

void ParticleEmitter::set_rate(int count)
{
	assert(count > 0);
	interval = 1.f / count;

	assert(max_particle_count * interval > start_life_time);	// otherwise birth rate is higher than death rate
}

const Particle* ParticleEmitter::get_particle_array()
{
	return particles;
}

float ParticleEmitter::get_start_life_time() const
{
	return start_life_time;
}




