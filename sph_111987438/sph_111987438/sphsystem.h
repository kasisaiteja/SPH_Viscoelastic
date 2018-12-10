#pragma once

#include "Particle.h"

#include "helper.h"
#include "misc.h"
#include "vector3D.h"

#include <vector>

// The flow of the sphsystem:
//  1. new SPHSystem;
//	2. call init();
//	3. call run();
//	4. call delete sph to deconstruct.

class SPHSystem {
public:

	SPHSystem();
	~SPHSystem();

	void init();

	// animation, system run
	void run(); 
	inline void setRunning() { sysRunning = true; }
	inline void stopRunning() { sysRunning = false; }
	inline bool getRunningState() { return sysRunning; }

	// get partical status for drawing
	Particle* particles;
	inline unsigned int getpNum() { return pNum; }

	// get other variable might needed
	inline Vector3D getWorldSize() { return worldSize; }


private:
	void update(); // advection, updates every particles' velocity & position 
	void buildTable(); // build_table, build the hash table of cells
	void calcDensPress(); // calculate density and pressure
	void calcForceAdv(); 

	unsigned int calCellHash(int3 pos); 


	unsigned int pNum; // number of particles
	unsigned int pNumMax; // maximum

	Vector3D worldSize;
	double cellSize;
	uint3 gridSize;
	unsigned int cellNum; // total cell
	Particle** cell;

	double h; // radius of kernel
	double h2; // kernel_2: kernel * kernel
	double mass;
	double kDens; 
	double kColorLapl;

	double timeStep;
	Vector3D gravity;

	
	double wallDamping; // wall_damping
	double restDens; //rest_density
	int test_density = 0;
	double restDens1; //rest_density
	double restDens2; //rest_density
	double restDens3; //rest_density
	double R; // gas_constant
	double viscosity;
	double surfNorm; // surf_norm
	double surfCoe; // surf_coe

	double poly6; // poly6_value
	double spiky; // spiky_value
	double visco; // visco_value

	double poly6Grad; // gradient grad_poly6
	double poly6Lapl; // laplace lplc_poly6

	bool sysRunning;

};


